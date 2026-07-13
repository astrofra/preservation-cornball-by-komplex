#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <GL/gl.h>

#include "cornball/capture.h"
#include "cornball/fla_gl.h"
#include "cornball/fla_scene.h"
#include "cornball/intro_gl.h"
#include "cornball/intro_scene.h"
#include "cornball/kaar_gl.h"
#include "cornball/kaar_scene.h"
#include "cornball/random.h"
#include "cornball/s_pair_gl.h"
#include "cornball/s_pair_scene.h"
#include "cornball/surf_gl.h"
#include "cornball/surf_scene.h"

typedef enum ReplayFamily {
    REPLAY_FAMILY_INTRO = 0,
    REPLAY_FAMILY_KAAR = 1,
    REPLAY_FAMILY_S_PAIR = 2,
    REPLAY_FAMILY_SURF = 3,
    REPLAY_FAMILY_FLA = 4,
    REPLAY_FAMILY_PLACEHOLDER = 5
} ReplayFamily;

typedef enum ReplayMusicMode {
    REPLAY_MUSIC_MODE_AUTO = -1,
    REPLAY_MUSIC_MODE_OFF = 0,
    REPLAY_MUSIC_MODE_ON = 1
} ReplayMusicMode;

typedef struct ReplaySegment {
    unsigned int original_scene_index;
    ReplayFamily family;
    double duration_positions;
} ReplaySegment;

typedef struct ReplaySelection {
    unsigned int segment_index;
    unsigned int original_scene_index;
    ReplayFamily family;
    double local_seconds;
    int valid;
} ReplaySelection;

typedef struct ReplayOptions {
    char asset_root[MAX_PATH];
    char capture_dir[MAX_PATH];
    unsigned int max_frames;
    unsigned int capture_every;
    unsigned int random_seed;
    int force_kaar_main_branch;
    int isolate_kaar_tube;
    int hidden;
    int window_width;
    int window_height;
    int music_mode;
    int log_scene_transitions;
    double demo_seconds;
    double position_seconds;
    double fixed_step_hz;
} ReplayOptions;

typedef struct MidasPlayStatus {
    unsigned int position;
    unsigned int pattern;
    unsigned int row;
    int sync_info;
} MidasPlayStatus;

typedef int (WINAPI *MidasStartupFn)(void);
typedef int (WINAPI *MidasInitFn)(void);
typedef int (WINAPI *MidasCloseFn)(void);
typedef void * (WINAPI *MidasLoadModuleFn)(char *file_name);
typedef int (WINAPI *MidasPlayModuleFn)(void *module, int num_effect_channels);
typedef int (WINAPI *MidasStopModuleFn)(void *module);
typedef int (WINAPI *MidasFreeModuleFn)(void *module);
typedef int (WINAPI *MidasStartBackgroundPlayFn)(DWORD poll_rate);
typedef int (WINAPI *MidasStopBackgroundPlayFn)(void);
typedef int (WINAPI *MidasGetPlayStatusFn)(MidasPlayStatus *status);
typedef int (WINAPI *MidasGetLastErrorFn)(void);
typedef char * (WINAPI *MidasGetErrorMessageFn)(int error_code);

typedef struct MidasAudioBackend {
    HMODULE dll_module;
    void *music_module;
    int active;
    int startup_done;
    int init_done;
    int background_started;
    int module_loaded;
    int playback_started;
    MidasStartupFn startup;
    MidasInitFn init;
    MidasCloseFn close;
    MidasLoadModuleFn load_module;
    MidasPlayModuleFn play_module;
    MidasStopModuleFn stop_module;
    MidasFreeModuleFn free_module;
    MidasStartBackgroundPlayFn start_background_play;
    MidasStopBackgroundPlayFn stop_background_play;
    MidasGetPlayStatusFn get_play_status;
    MidasGetLastErrorFn get_last_error;
    MidasGetErrorMessageFn get_error_message;
} MidasAudioBackend;

typedef struct ReplayApp {
    HWND hwnd;
    HDC hdc;
    HGLRC hglrc;
    int width;
    int height;
    int running;
    LARGE_INTEGER perf_frequency;
    LARGE_INTEGER last_tick;
    double elapsed_seconds;
    double accumulator_seconds;
    double scene_start_seconds;
    unsigned int presented_frames;
    unsigned int active_scene_index;
    ReplayOptions options;
    ReplaySelection selection;
    MidasAudioBackend music;
    MidasPlayStatus music_status;
    int music_active;
    CornballRandom random;
    CornballIntroScene intro_scene;
    CornballIntroFrame intro_frame;
    CornballIntroGlRenderer intro_renderer;
    CornballKaarScene kaar_scene;
    CornballKaarFrame kaar_frame;
    CornballKaarGlRenderer kaar_renderer;
    CornballSPairScene s_pair_scene;
    CornballSPairFrame s_pair_frame;
    CornballSPairGlRenderer s_pair_renderer;
    CornballSurfScene surf_scene;
    CornballSurfFrame surf_frame;
    CornballSurfGlRenderer surf_renderer;
    CornballFlaScene fla_scene;
    CornballFlaFrame fla_frame;
    CornballFlaGlRenderer fla_renderer;
} ReplayApp;

static const double kDefaultFixedStepHz = 60.0;
static const double kDefaultPositionSeconds = 1.0;
static const int kDefaultWindowWidth = 640;
static const int kDefaultWindowHeight = 400;
static const unsigned int kInvalidSceneIndex = UINT_MAX;

static const ReplaySegment kReplaySegments[] = {
    {0u, REPLAY_FAMILY_INTRO, 12.0},
    {1u, REPLAY_FAMILY_KAAR, 6.0},
    {2u, REPLAY_FAMILY_S_PAIR, 4.0},
    {3u, REPLAY_FAMILY_SURF, 3.0},
    {4u, REPLAY_FAMILY_S_PAIR, 2.0},
    {5u, REPLAY_FAMILY_FLA, 3.0},
    {6u, REPLAY_FAMILY_SURF, 4.0},
    {7u, REPLAY_FAMILY_INTRO, 1.0},
    {8u, REPLAY_FAMILY_KAAR, 3.0}
};

static const unsigned int kOriginalSceneThresholds[] = {
    0x0cu,
    0x12u,
    0x16u,
    0x19u,
    0x1bu,
    0x1eu,
    0x22u,
    0x23u,
    0x26u,
    0x29u
};

static const ReplayFamily kOriginalSceneFamilies[] = {
    REPLAY_FAMILY_INTRO,
    REPLAY_FAMILY_KAAR,
    REPLAY_FAMILY_S_PAIR,
    REPLAY_FAMILY_SURF,
    REPLAY_FAMILY_S_PAIR,
    REPLAY_FAMILY_FLA,
    REPLAY_FAMILY_SURF,
    REPLAY_FAMILY_INTRO,
    REPLAY_FAMILY_KAAR,
    REPLAY_FAMILY_PLACEHOLDER
};

static void copy_error_message(char *dst, size_t dst_size, const char *src)
{
    if ((dst == NULL) || (dst_size == 0u)) {
        return;
    }

    if (src == NULL) {
        dst[0] = '\0';
        return;
    }

    snprintf(dst, dst_size, "%s", src);
}

static FARPROC load_midas_symbol(HMODULE dll_module, const char *name)
{
    if ((dll_module == NULL) || (name == NULL)) {
        return NULL;
    }

    return GetProcAddress(dll_module, name);
}

static int midas_runtime_supported(void)
{
#if defined(_WIN64)
    return 0;
#else
    return sizeof(void *) == 4u;
#endif
}

static int file_exists(const char *path)
{
    DWORD attributes = GetFileAttributesA(path);
    return (attributes != INVALID_FILE_ATTRIBUTES) && ((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0u);
}

static void join_path(char *dst, size_t dst_size, const char *left, const char *right)
{
    snprintf(dst, dst_size, "%s\\%s", left, right);
}

static int asset_root_looks_valid(const char *root)
{
    static const char *const kRequiredAssets[] = {
        "V1.TGA",
        "V2.TGA",
        "TXT1.TGA",
        "TXT2.TGA",
        "LOGO.TGA",
        "LOGOTAUS.TGA",
        "KAAR128.TGA",
        "S1.TGA",
        "S2.TGA",
        "SURF128.TGA",
        "FLA.TGA"
    };
    size_t index;
    char candidate[MAX_PATH];

    for (index = 0u; index < (sizeof(kRequiredAssets) / sizeof(kRequiredAssets[0])); ++index) {
        join_path(candidate, sizeof(candidate), root, kRequiredAssets[index]);
        if (!file_exists(candidate)) {
            return 0;
        }
    }

    return 1;
}

static void get_executable_directory(char *dst, size_t dst_size)
{
    DWORD length;

    length = GetModuleFileNameA(NULL, dst, (DWORD)dst_size);
    if ((length == 0u) || (length >= dst_size)) {
        dst[0] = '\0';
        return;
    }

    while ((length > 0u) && (dst[length - 1u] != '\\') && (dst[length - 1u] != '/')) {
        --length;
    }

    if (length > 0u) {
        dst[length - 1u] = '\0';
    } else {
        dst[0] = '\0';
    }
}

static double replay_total_positions(void)
{
    size_t index;
    double total = 0.0;

    for (index = 0u; index < (sizeof(kReplaySegments) / sizeof(kReplaySegments[0])); ++index) {
        total += kReplaySegments[index].duration_positions;
    }

    return total;
}

static double replay_total_seconds(double position_seconds)
{
    return replay_total_positions() * position_seconds;
}

static void log_scene_transition(
    const ReplayApp *app,
    const char *clock_label,
    unsigned int scene_index,
    unsigned int position
)
{
    if (app->options.log_scene_transitions == 0) {
        return;
    }

    printf(
        "[scene-transition] clock=%s elapsed=%0.3f scene=%u position=%u\n",
        clock_label,
        app->elapsed_seconds,
        scene_index,
        position
    );
    fflush(stdout);
}

static ReplaySelection replay_select(double elapsed_seconds, double position_seconds)
{
    size_t index;
    double current_positions;
    double start_positions = 0.0;
    ReplaySelection selection;

    memset(&selection, 0, sizeof(selection));

    if (position_seconds <= 0.0) {
        return selection;
    }

    current_positions = elapsed_seconds / position_seconds;

    for (index = 0u; index < (sizeof(kReplaySegments) / sizeof(kReplaySegments[0])); ++index) {
        double end_positions = start_positions + kReplaySegments[index].duration_positions;

        if (current_positions < end_positions) {
            selection.segment_index = (unsigned int)index;
            selection.original_scene_index = kReplaySegments[index].original_scene_index;
            selection.family = kReplaySegments[index].family;
            selection.local_seconds = (current_positions - start_positions) * position_seconds;
            selection.valid = 1;
            return selection;
        }

        start_positions = end_positions;
    }

    selection.segment_index = (unsigned int)((sizeof(kReplaySegments) / sizeof(kReplaySegments[0])) - 1u);
    selection.original_scene_index = kReplaySegments[selection.segment_index].original_scene_index;
    selection.family = kReplaySegments[selection.segment_index].family;
    selection.local_seconds = kReplaySegments[selection.segment_index].duration_positions * position_seconds;
    selection.valid = 1;
    return selection;
}

static ReplaySelection replay_select_from_music_position(unsigned int position)
{
    size_t index;
    ReplaySelection selection;

    memset(&selection, 0, sizeof(selection));

    for (index = 0u; index < (sizeof(kOriginalSceneThresholds) / sizeof(kOriginalSceneThresholds[0])); ++index) {
        if (position < kOriginalSceneThresholds[index]) {
            selection.segment_index = (unsigned int)index;
            selection.original_scene_index = (unsigned int)index;
            selection.family = kOriginalSceneFamilies[index];
            selection.valid = 1;
            return selection;
        }
    }

    selection.segment_index = (unsigned int)((sizeof(kOriginalSceneFamilies) / sizeof(kOriginalSceneFamilies[0])) - 1u);
    selection.original_scene_index = selection.segment_index;
    selection.family = kOriginalSceneFamilies[selection.segment_index];
    selection.valid = 1;
    return selection;
}

static double replay_step_seconds(const ReplayOptions *options)
{
    if ((options == NULL) || (options->fixed_step_hz <= 0.0)) {
        return 1.0 / kDefaultFixedStepHz;
    }

    return 1.0 / options->fixed_step_hz;
}

static int parse_options(int argc, char **argv, ReplayOptions *options)
{
    int i;

    memset(options, 0, sizeof(*options));
    options->random_seed = 1u;
    options->window_width = kDefaultWindowWidth;
    options->window_height = kDefaultWindowHeight;
    options->music_mode = REPLAY_MUSIC_MODE_AUTO;
    options->position_seconds = kDefaultPositionSeconds;
    options->fixed_step_hz = kDefaultFixedStepHz;

    for (i = 1; i < argc; ++i) {
        if ((strcmp(argv[i], "--asset-root") == 0) && ((i + 1) < argc)) {
            snprintf(options->asset_root, sizeof(options->asset_root), "%s", argv[i + 1]);
            ++i;
        } else if ((strcmp(argv[i], "--capture-dir") == 0) && ((i + 1) < argc)) {
            snprintf(options->capture_dir, sizeof(options->capture_dir), "%s", argv[i + 1]);
            ++i;
        } else if ((strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc)) {
            options->max_frames = (unsigned int)strtoul(argv[i + 1], NULL, 10);
            ++i;
        } else if ((strcmp(argv[i], "--capture-every") == 0) && ((i + 1) < argc)) {
            options->capture_every = (unsigned int)strtoul(argv[i + 1], NULL, 10);
            ++i;
        } else if ((strcmp(argv[i], "--seed") == 0) && ((i + 1) < argc)) {
            options->random_seed = (unsigned int)strtoul(argv[i + 1], NULL, 10);
            ++i;
        } else if ((strcmp(argv[i], "--width") == 0) && ((i + 1) < argc)) {
            options->window_width = (int)strtol(argv[i + 1], NULL, 10);
            ++i;
        } else if ((strcmp(argv[i], "--height") == 0) && ((i + 1) < argc)) {
            options->window_height = (int)strtol(argv[i + 1], NULL, 10);
            ++i;
        } else if (((strcmp(argv[i], "--demo-seconds") == 0) || (strcmp(argv[i], "--scene-seconds") == 0)) &&
                ((i + 1) < argc)) {
            options->demo_seconds = strtod(argv[i + 1], NULL);
            ++i;
        } else if ((strcmp(argv[i], "--position-seconds") == 0) && ((i + 1) < argc)) {
            options->position_seconds = strtod(argv[i + 1], NULL);
            ++i;
        } else if ((strcmp(argv[i], "--fixed-step-hz") == 0) && ((i + 1) < argc)) {
            options->fixed_step_hz = strtod(argv[i + 1], NULL);
            ++i;
        } else if (strcmp(argv[i], "--force-kaar-main-branch") == 0) {
            options->force_kaar_main_branch = 1;
        } else if (strcmp(argv[i], "--isolate-kaar-tube") == 0) {
            options->isolate_kaar_tube = 1;
        } else if (strcmp(argv[i], "--music") == 0) {
            options->music_mode = REPLAY_MUSIC_MODE_ON;
        } else if (strcmp(argv[i], "--no-music") == 0) {
            options->music_mode = REPLAY_MUSIC_MODE_OFF;
        } else if (strcmp(argv[i], "--log-scene-transitions") == 0) {
            options->log_scene_transitions = 1;
        } else if (strcmp(argv[i], "--hidden") == 0) {
            options->hidden = 1;
        } else {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            return 0;
        }
    }

    if (options->window_width <= 0) {
        options->window_width = kDefaultWindowWidth;
    }

    if (options->window_height <= 0) {
        options->window_height = kDefaultWindowHeight;
    }

    if (options->random_seed == 0u) {
        options->random_seed = 1u;
    }

    if (options->demo_seconds < 0.0) {
        options->demo_seconds = 0.0;
    }

    if (options->position_seconds <= 0.0) {
        options->position_seconds = kDefaultPositionSeconds;
    }

    if (options->fixed_step_hz <= 0.0) {
        options->fixed_step_hz = kDefaultFixedStepHz;
    }

    return 1;
}

static int resolve_asset_root(ReplayOptions *options)
{
    char executable_dir[MAX_PATH];
    char candidate[MAX_PATH];

    if ((options->asset_root[0] != '\0') && asset_root_looks_valid(options->asset_root)) {
        return 1;
    }

    if (asset_root_looks_valid("reverse\\baseline\\cornball")) {
        snprintf(options->asset_root, sizeof(options->asset_root), "%s", "reverse\\baseline\\cornball");
        return 1;
    }

    if (asset_root_looks_valid("original\\cornball")) {
        snprintf(options->asset_root, sizeof(options->asset_root), "%s", "original\\cornball");
        return 1;
    }

    get_executable_directory(executable_dir, sizeof(executable_dir));

    snprintf(candidate, sizeof(candidate), "%s\\..\\..\\..\\..\\baseline\\cornball", executable_dir);
    if (asset_root_looks_valid(candidate)) {
        snprintf(options->asset_root, sizeof(options->asset_root), "%s", candidate);
        return 1;
    }

    snprintf(candidate, sizeof(candidate), "%s\\..\\..\\..\\..\\..\\original\\cornball", executable_dir);
    if (asset_root_looks_valid(candidate)) {
        snprintf(options->asset_root, sizeof(options->asset_root), "%s", candidate);
        return 1;
    }

    return 0;
}

static void append_midas_error(
    const MidasAudioBackend *music,
    char *error_message,
    size_t error_message_size,
    const char *prefix
)
{
    int midas_error;
    const char *midas_message = NULL;

    if ((music != NULL) &&
            (music->get_last_error != NULL) &&
            (music->get_error_message != NULL)) {
        midas_error = music->get_last_error();
        midas_message = music->get_error_message(midas_error);
    }

    if ((midas_message != NULL) && (midas_message[0] != '\0')) {
        snprintf(error_message, error_message_size, "%s: %s", prefix, midas_message);
    } else {
        copy_error_message(error_message, error_message_size, prefix);
    }
}

static void shutdown_music_backend(MidasAudioBackend *music)
{
    if (music == NULL) {
        return;
    }

    if (music->playback_started && (music->stop_module != NULL) && (music->music_module != NULL)) {
        music->stop_module(music->music_module);
        music->playback_started = 0;
    }

    if (music->module_loaded && (music->free_module != NULL) && (music->music_module != NULL)) {
        music->free_module(music->music_module);
        music->music_module = NULL;
        music->module_loaded = 0;
    }

    if (music->background_started && (music->stop_background_play != NULL)) {
        music->stop_background_play();
        music->background_started = 0;
    }

    if (music->init_done && (music->close != NULL)) {
        music->close();
        music->init_done = 0;
    }

    if (music->dll_module != NULL) {
        FreeLibrary(music->dll_module);
        music->dll_module = NULL;
    }

    music->active = 0;
    music->startup_done = 0;
}

static int start_music_backend(
    ReplayApp *app,
    char *error_message,
    size_t error_message_size
)
{
    char executable_dir[MAX_PATH];
    char dll_path[MAX_PATH];
    char local_dll_path[MAX_PATH];
    char module_path[MAX_PATH];
    char local_module_path[MAX_PATH];
    MidasAudioBackend *music = &app->music;

    memset(music, 0, sizeof(*music));

    if (!midas_runtime_supported()) {
        copy_error_message(
            error_message,
            error_message_size,
            "Music replay currently requires a Win32/x86 build because the bundled MIDAS06.DLL is 32-bit."
        );
        return 0;
    }

    join_path(dll_path, sizeof(dll_path), app->options.asset_root, "MIDAS06.DLL");
    join_path(module_path, sizeof(module_path), app->options.asset_root, "AAB.XM");

    get_executable_directory(executable_dir, sizeof(executable_dir));
    if (executable_dir[0] != '\0') {
        join_path(local_dll_path, sizeof(local_dll_path), executable_dir, "MIDAS06.DLL");
        if (file_exists(local_dll_path)) {
            snprintf(dll_path, sizeof(dll_path), "%s", local_dll_path);
        }

        join_path(local_module_path, sizeof(local_module_path), executable_dir, "AAB.XM");
        if (file_exists(local_module_path)) {
            snprintf(module_path, sizeof(module_path), "%s", local_module_path);
        }
    }

    if (!file_exists(dll_path)) {
        copy_error_message(
            error_message,
            error_message_size,
            "MIDAS06.DLL was not found next to the executable or in the resolved asset root."
        );
        return 0;
    }

    if (!file_exists(module_path)) {
        copy_error_message(
            error_message,
            error_message_size,
            "AAB.XM was not found next to the executable or in the resolved asset root."
        );
        return 0;
    }

    music->dll_module = LoadLibraryA(dll_path);
    if (music->dll_module == NULL) {
        copy_error_message(error_message, error_message_size, "Failed to load MIDAS06.DLL.");
        shutdown_music_backend(music);
        return 0;
    }

    music->startup = (MidasStartupFn)load_midas_symbol(music->dll_module, "_MIDASstartup@0");
    music->init = (MidasInitFn)load_midas_symbol(music->dll_module, "_MIDASinit@0");
    music->close = (MidasCloseFn)load_midas_symbol(music->dll_module, "_MIDASclose@0");
    music->load_module = (MidasLoadModuleFn)load_midas_symbol(music->dll_module, "_MIDASloadModule@4");
    music->play_module = (MidasPlayModuleFn)load_midas_symbol(music->dll_module, "_MIDASplayModule@8");
    music->stop_module = (MidasStopModuleFn)load_midas_symbol(music->dll_module, "_MIDASstopModule@4");
    music->free_module = (MidasFreeModuleFn)load_midas_symbol(music->dll_module, "_MIDASfreeModule@4");
    music->start_background_play = (MidasStartBackgroundPlayFn)load_midas_symbol(
        music->dll_module,
        "_MIDASstartBackgroundPlay@4"
    );
    music->stop_background_play = (MidasStopBackgroundPlayFn)load_midas_symbol(
        music->dll_module,
        "_MIDASstopBackgroundPlay@0"
    );
    music->get_play_status = (MidasGetPlayStatusFn)load_midas_symbol(music->dll_module, "_MIDASgetPlayStatus@4");
    music->get_last_error = (MidasGetLastErrorFn)load_midas_symbol(music->dll_module, "_MIDASgetLastError@0");
    music->get_error_message = (MidasGetErrorMessageFn)load_midas_symbol(
        music->dll_module,
        "_MIDASgetErrorMessage@4"
    );

    if ((music->startup == NULL) ||
            (music->init == NULL) ||
            (music->close == NULL) ||
            (music->load_module == NULL) ||
            (music->play_module == NULL) ||
            (music->stop_module == NULL) ||
            (music->free_module == NULL) ||
            (music->start_background_play == NULL) ||
            (music->stop_background_play == NULL) ||
            (music->get_play_status == NULL) ||
            (music->get_last_error == NULL) ||
            (music->get_error_message == NULL)) {
        copy_error_message(
            error_message,
            error_message_size,
            "MIDAS06.DLL is missing one or more required exports for module playback."
        );
        shutdown_music_backend(music);
        return 0;
    }

    if (!music->startup()) {
        append_midas_error(music, error_message, error_message_size, "MIDASstartup failed");
        shutdown_music_backend(music);
        return 0;
    }
    music->startup_done = 1;

    if (!music->init()) {
        append_midas_error(music, error_message, error_message_size, "MIDASinit failed");
        shutdown_music_backend(music);
        return 0;
    }
    music->init_done = 1;

    if (!music->start_background_play(0u)) {
        append_midas_error(music, error_message, error_message_size, "MIDASstartBackgroundPlay failed");
        shutdown_music_backend(music);
        return 0;
    }
    music->background_started = 1;

    music->music_module = music->load_module(module_path);
    if (music->music_module == NULL) {
        append_midas_error(music, error_message, error_message_size, "MIDASloadModule failed");
        shutdown_music_backend(music);
        return 0;
    }
    music->module_loaded = 1;

    if (!music->play_module(music->music_module, 0)) {
        append_midas_error(music, error_message, error_message_size, "MIDASplayModule failed");
        shutdown_music_backend(music);
        return 0;
    }
    music->playback_started = 1;
    music->active = 1;
    app->music_active = 1;
    copy_error_message(error_message, error_message_size, "");
    return 1;
}

static int music_backend_get_status(
    const MidasAudioBackend *music,
    MidasPlayStatus *status
)
{
    if ((music == NULL) || (status == NULL) || !music->active || (music->get_play_status == NULL)) {
        return 0;
    }

    return music->get_play_status(status) != 0;
}

static int ensure_directory_exists(const char *path)
{
    if ((path == NULL) || (path[0] == '\0')) {
        return 0;
    }

    if (CreateDirectoryA(path, NULL) != 0) {
        return 1;
    }

    return GetLastError() == ERROR_ALREADY_EXISTS;
}

static int automated_mode_enabled(const ReplayApp *app)
{
    return (app->options.hidden != 0) ||
        (app->options.max_frames > 0u) ||
        (app->options.capture_every > 0u);
}

static int replay_should_enable_music(const ReplayApp *app)
{
    if (app->options.music_mode == REPLAY_MUSIC_MODE_ON) {
        return 1;
    }

    if (app->options.music_mode == REPLAY_MUSIC_MODE_OFF) {
        return 0;
    }

    return !automated_mode_enabled(app);
}

static void update_music_selection(ReplayApp *app)
{
    ReplaySelection next_selection;

    if (!music_backend_get_status(&app->music, &app->music_status)) {
        return;
    }

    next_selection = replay_select_from_music_position(app->music_status.position);

    if ((!app->selection.valid) || (next_selection.original_scene_index != app->active_scene_index)) {
        app->scene_start_seconds = app->elapsed_seconds;
        app->active_scene_index = next_selection.original_scene_index;
        log_scene_transition(app, "music", next_selection.original_scene_index, app->music_status.position);
    }

    next_selection.local_seconds = app->elapsed_seconds - app->scene_start_seconds;
    if (next_selection.local_seconds < 0.0) {
        next_selection.local_seconds = 0.0;
    }

    app->selection = next_selection;
}

static int capture_current_frame(ReplayApp *app, const char *prefix)
{
    char output_path[MAX_PATH];
    char error_message[256];
    unsigned char *pixels;
    size_t pixel_count;

    if ((app->options.capture_every == 0u) || (app->options.capture_dir[0] == '\0')) {
        return 1;
    }

    if ((app->presented_frames % app->options.capture_every) != 0u) {
        return 1;
    }

    if (!ensure_directory_exists(app->options.capture_dir)) {
        fprintf(stderr, "Failed to create capture directory: %s\n", app->options.capture_dir);
        return 0;
    }

    pixel_count = (size_t)app->width * (size_t)app->height * 3u;
    pixels = (unsigned char *)malloc(pixel_count);
    if (pixels == NULL) {
        fprintf(stderr, "Out of memory while capturing frame.\n");
        return 0;
    }

    glReadBuffer(GL_BACK);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, app->width, app->height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    snprintf(
        output_path,
        sizeof(output_path),
        "%s\\%s_frame_%06u.tga",
        app->options.capture_dir,
        prefix,
        app->presented_frames
    );

    if (!cornball_capture_write_tga24(
            output_path,
            app->width,
            app->height,
            pixels,
            error_message,
            sizeof(error_message))) {
        free(pixels);
        fprintf(stderr, "Capture write failed: %s\n", error_message);
        return 0;
    }

    free(pixels);
    return 1;
}

static void step_current_segment(ReplayApp *app)
{
    ReplaySelection next_selection;

    next_selection = replay_select(app->elapsed_seconds, app->options.position_seconds);
    if (next_selection.valid &&
            (!app->music_active) &&
            ((!app->selection.valid) || (next_selection.original_scene_index != app->active_scene_index))) {
        double current_positions = app->elapsed_seconds / app->options.position_seconds;

        app->active_scene_index = next_selection.original_scene_index;
        log_scene_transition(app, "synthetic", next_selection.original_scene_index, (unsigned int)current_positions);
    }

    app->selection = next_selection;

    if (!app->selection.valid) {
        return;
    }

    switch (app->selection.family) {
    case REPLAY_FAMILY_INTRO:
        cornball_intro_scene_step_frame(
            &app->intro_scene,
            &app->random,
            app->selection.local_seconds,
            &app->intro_frame
        );
        break;

    case REPLAY_FAMILY_KAAR:
        cornball_kaar_scene_step_frame(
            &app->kaar_scene,
            &app->random,
            app->selection.local_seconds,
            &app->kaar_frame
        );
        break;

    case REPLAY_FAMILY_S_PAIR:
        cornball_s_pair_scene_step_frame(
            &app->s_pair_scene,
            &app->random,
            app->selection.local_seconds,
            &app->s_pair_frame
        );
        break;

    case REPLAY_FAMILY_SURF:
        cornball_surf_scene_step_frame(
            &app->surf_scene,
            &app->random,
            app->selection.local_seconds,
            &app->surf_frame
        );
        break;

    case REPLAY_FAMILY_FLA:
        cornball_fla_scene_step_frame(
            &app->fla_scene,
            &app->random,
            app->selection.local_seconds,
            &app->fla_frame
        );
        break;

    case REPLAY_FAMILY_PLACEHOLDER:
        break;
    }
}

static void warmup_replay(ReplayApp *app)
{
    double step_seconds;
    double total_seconds;

    step_seconds = replay_step_seconds(&app->options);
    total_seconds = replay_total_seconds(app->options.position_seconds);
    if (app->options.demo_seconds > total_seconds) {
        app->options.demo_seconds = total_seconds;
    }

    app->elapsed_seconds = 0.0;
    step_current_segment(app);

    while ((app->elapsed_seconds + step_seconds) <= (app->options.demo_seconds + 1e-9)) {
        app->elapsed_seconds += step_seconds;
        step_current_segment(app);
    }
}

static int init_renderers(ReplayApp *app, char *error_message, size_t error_message_size)
{
    if (!cornball_intro_gl_init(&app->intro_renderer, app->options.asset_root, error_message, error_message_size)) {
        return 0;
    }

    if (!cornball_kaar_gl_init(&app->kaar_renderer, app->options.asset_root, error_message, error_message_size)) {
        cornball_intro_gl_shutdown(&app->intro_renderer);
        return 0;
    }

    if (!cornball_s_pair_gl_init(&app->s_pair_renderer, app->options.asset_root, error_message, error_message_size)) {
        cornball_kaar_gl_shutdown(&app->kaar_renderer);
        cornball_intro_gl_shutdown(&app->intro_renderer);
        return 0;
    }

    if (!cornball_surf_gl_init(&app->surf_renderer, app->options.asset_root, error_message, error_message_size)) {
        cornball_s_pair_gl_shutdown(&app->s_pair_renderer);
        cornball_kaar_gl_shutdown(&app->kaar_renderer);
        cornball_intro_gl_shutdown(&app->intro_renderer);
        return 0;
    }

    if (!cornball_fla_gl_init(&app->fla_renderer, app->options.asset_root, error_message, error_message_size)) {
        cornball_surf_gl_shutdown(&app->surf_renderer);
        cornball_s_pair_gl_shutdown(&app->s_pair_renderer);
        cornball_kaar_gl_shutdown(&app->kaar_renderer);
        cornball_intro_gl_shutdown(&app->intro_renderer);
        return 0;
    }

    return 1;
}

static void shutdown_renderers(ReplayApp *app)
{
    cornball_fla_gl_shutdown(&app->fla_renderer);
    cornball_surf_gl_shutdown(&app->surf_renderer);
    cornball_s_pair_gl_shutdown(&app->s_pair_renderer);
    cornball_kaar_gl_shutdown(&app->kaar_renderer);
    cornball_intro_gl_shutdown(&app->intro_renderer);
}

static LRESULT CALLBACK replay_wndproc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param)
{
    ReplayApp *app = (ReplayApp *)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

    switch (message) {
    case WM_NCCREATE:
        SetWindowLongPtrA(
            hwnd,
            GWLP_USERDATA,
            (LONG_PTR)((CREATESTRUCTA *)l_param)->lpCreateParams
        );
        return TRUE;

    case WM_SIZE:
        if (app != NULL) {
            app->width = LOWORD(l_param);
            app->height = HIWORD(l_param);
            if (app->hglrc != NULL) {
                cornball_intro_gl_resize(&app->intro_renderer, app->width, app->height);
                cornball_kaar_gl_resize(&app->kaar_renderer, app->width, app->height);
                cornball_s_pair_gl_resize(&app->s_pair_renderer, app->width, app->height);
                cornball_surf_gl_resize(&app->surf_renderer, app->width, app->height);
                cornball_fla_gl_resize(&app->fla_renderer, app->width, app->height);
            }
        }
        return 0;

    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        if (app != NULL) {
            app->running = 0;
        }
        PostQuitMessage(0);
        return 0;

    case WM_KEYDOWN:
        if (w_param == VK_ESCAPE) {
            DestroyWindow(hwnd);
            return 0;
        }
        break;

    default:
        break;
    }

    return DefWindowProcA(hwnd, message, w_param, l_param);
}

static int create_gl_window(ReplayApp *app)
{
    const char *class_name = "CornballReplayWindow";
    WNDCLASSA wc;
    RECT rect;
    PIXELFORMATDESCRIPTOR pfd;
    int pixel_format;
    DWORD style = WS_OVERLAPPEDWINDOW;

    memset(&wc, 0, sizeof(wc));
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = replay_wndproc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = class_name;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (RegisterClassA(&wc) == 0u) {
        return 0;
    }

    rect.left = 0;
    rect.top = 0;
    rect.right = app->options.window_width;
    rect.bottom = app->options.window_height;
    AdjustWindowRect(&rect, style, FALSE);

    app->hwnd = CreateWindowExA(
        0,
        class_name,
        "Cornball Reconstruction Replay",
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL,
        NULL,
        GetModuleHandleA(NULL),
        app
    );

    if (app->hwnd == NULL) {
        return 0;
    }

    app->hdc = GetDC(app->hwnd);
    if (app->hdc == NULL) {
        return 0;
    }

    memset(&pfd, 0, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;

    pixel_format = ChoosePixelFormat(app->hdc, &pfd);
    if (pixel_format == 0) {
        return 0;
    }

    if (!SetPixelFormat(app->hdc, pixel_format, &pfd)) {
        return 0;
    }

    app->hglrc = wglCreateContext(app->hdc);
    if (app->hglrc == NULL) {
        return 0;
    }

    if (!wglMakeCurrent(app->hdc, app->hglrc)) {
        return 0;
    }

    ShowWindow(app->hwnd, app->options.hidden ? SW_HIDE : SW_SHOWNORMAL);
    UpdateWindow(app->hwnd);
    return 1;
}

static void destroy_gl_window(ReplayApp *app)
{
    if (app->hglrc != NULL) {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(app->hglrc);
        app->hglrc = NULL;
    }

    if ((app->hwnd != NULL) && (app->hdc != NULL)) {
        ReleaseDC(app->hwnd, app->hdc);
        app->hdc = NULL;
    }

    if (app->hwnd != NULL) {
        DestroyWindow(app->hwnd);
        app->hwnd = NULL;
    }
}

static void render_current_segment(ReplayApp *app)
{
    switch (app->selection.family) {
    case REPLAY_FAMILY_INTRO:
        cornball_intro_gl_resize(&app->intro_renderer, app->width, app->height);
        cornball_intro_gl_render(&app->intro_renderer, &app->intro_frame);
        break;

    case REPLAY_FAMILY_KAAR:
        cornball_kaar_gl_resize(&app->kaar_renderer, app->width, app->height);
        cornball_kaar_gl_render(&app->kaar_renderer, &app->kaar_frame);
        break;

    case REPLAY_FAMILY_S_PAIR:
        cornball_s_pair_gl_resize(&app->s_pair_renderer, app->width, app->height);
        cornball_s_pair_gl_render(&app->s_pair_renderer, &app->s_pair_frame);
        break;

    case REPLAY_FAMILY_SURF:
        cornball_surf_gl_resize(&app->surf_renderer, app->width, app->height);
        cornball_surf_gl_render(&app->surf_renderer, &app->surf_frame);
        break;

    case REPLAY_FAMILY_FLA:
        cornball_fla_gl_resize(&app->fla_renderer, app->width, app->height);
        cornball_fla_gl_render(&app->fla_renderer, &app->fla_frame);
        break;

    case REPLAY_FAMILY_PLACEHOLDER:
        glViewport(0, 0, app->width, app->height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        break;
    }
}

static int update_and_render(ReplayApp *app)
{
    LARGE_INTEGER now;
    double delta_seconds;
    double step_seconds;
    double total_seconds;
    RECT client_rect;

    QueryPerformanceCounter(&now);
    delta_seconds = (double)(now.QuadPart - app->last_tick.QuadPart) / (double)app->perf_frequency.QuadPart;
    app->last_tick = now;

    step_seconds = replay_step_seconds(&app->options);
    total_seconds = replay_total_seconds(app->options.position_seconds);

    if (automated_mode_enabled(app) && !app->music_active) {
        if (app->presented_frames > 0u) {
            app->elapsed_seconds += step_seconds;
            if (app->elapsed_seconds > total_seconds) {
                app->elapsed_seconds = total_seconds;
            }
            step_current_segment(app);
        }
    } else {
        if (delta_seconds > 0.25) {
            delta_seconds = 0.25;
        }

        app->accumulator_seconds += delta_seconds;

        while (app->accumulator_seconds >= step_seconds) {
            app->elapsed_seconds += step_seconds;
            if (!app->music_active && (app->elapsed_seconds > total_seconds)) {
                app->elapsed_seconds = total_seconds;
            }
            if (app->music_active) {
                update_music_selection(app);
            }
            step_current_segment(app);
            app->accumulator_seconds -= step_seconds;

            if (!app->music_active && (app->elapsed_seconds >= total_seconds)) {
                break;
            }
        }
    }

    GetClientRect(app->hwnd, &client_rect);
    app->width = client_rect.right - client_rect.left;
    app->height = client_rect.bottom - client_rect.top;

    render_current_segment(app);
    if (!capture_current_frame(app, "replay")) {
        return 0;
    }
    SwapBuffers(app->hdc);

    ++app->presented_frames;
    if ((app->options.max_frames > 0u) && (app->presented_frames >= app->options.max_frames)) {
        app->running = 0;
    }

    if (!app->music_active && (app->elapsed_seconds >= total_seconds)) {
        app->running = 0;
    }

    return 1;
}

int main(int argc, char **argv)
{
    ReplayApp app;
    MSG message;
    char error_message[256];

    memset(&app, 0, sizeof(app));

    if (!parse_options(argc, argv, &app.options)) {
        return 1;
    }

    if (!resolve_asset_root(&app.options)) {
        fprintf(
            stderr,
            "Could not resolve a valid asset root containing V1.TGA, V2.TGA, TXT1.TGA, TXT2.TGA, LOGO.TGA, LOGOTAUS.TGA, KAAR128.TGA, S1.TGA, S2.TGA, SURF128.TGA, and FLA.TGA.\n"
        );
        return 1;
    }

    cornball_random_seed(&app.random, app.options.random_seed);

    cornball_intro_scene_clear(&app.intro_scene);
    cornball_intro_scene_loader_pass(&app.intro_scene);
    cornball_kaar_scene_clear(&app.kaar_scene);
    cornball_kaar_scene_loader_pass(&app.kaar_scene);
    cornball_kaar_scene_set_force_main_branch(
        &app.kaar_scene,
        (uint32_t)(((app.options.force_kaar_main_branch != 0) || (app.options.isolate_kaar_tube != 0)) ? 1u : 0u)
    );
    cornball_kaar_scene_set_isolate_tube_pass(
        &app.kaar_scene,
        (uint32_t)((app.options.isolate_kaar_tube != 0) ? 1u : 0u)
    );
    cornball_s_pair_scene_clear(&app.s_pair_scene);
    cornball_s_pair_scene_loader_pass(&app.s_pair_scene);
    cornball_surf_scene_clear(&app.surf_scene);
    cornball_surf_scene_loader_pass(&app.surf_scene);
    cornball_fla_scene_clear(&app.fla_scene);
    cornball_fla_scene_loader_pass(&app.fla_scene);

    app.active_scene_index = kInvalidSceneIndex;

    if (replay_should_enable_music(&app)) {
        if (!start_music_backend(&app, error_message, sizeof(error_message))) {
            if (app.options.music_mode == REPLAY_MUSIC_MODE_ON) {
                fprintf(stderr, "Music backend init failed: %s\n", error_message);
                shutdown_music_backend(&app.music);
                return 1;
            }
        } else {
            update_music_selection(&app);
            step_current_segment(&app);
        }
    }

    if (!app.music_active) {
        warmup_replay(&app);
    }

    if (!create_gl_window(&app)) {
        fprintf(stderr, "Failed to create Win32 OpenGL replay window.\n");
        shutdown_music_backend(&app.music);
        destroy_gl_window(&app);
        return 1;
    }

    if (!QueryPerformanceFrequency(&app.perf_frequency)) {
        fprintf(stderr, "QueryPerformanceFrequency failed.\n");
        shutdown_music_backend(&app.music);
        destroy_gl_window(&app);
        return 1;
    }

    QueryPerformanceCounter(&app.last_tick);

    if (!init_renderers(&app, error_message, sizeof(error_message))) {
        fprintf(stderr, "OpenGL renderer init failed: %s\n", error_message);
        shutdown_music_backend(&app.music);
        destroy_gl_window(&app);
        return 1;
    }

    app.running = 1;

    while (app.running) {
        while (PeekMessageA(&message, NULL, 0u, 0u, PM_REMOVE)) {
            if (message.message == WM_QUIT) {
                app.running = 0;
                break;
            }

            TranslateMessage(&message);
            DispatchMessageA(&message);
        }

        if (!app.running) {
            break;
        }

        if (!update_and_render(&app)) {
            app.running = 0;
        }
    }

    shutdown_renderers(&app);
    shutdown_music_backend(&app.music);
    destroy_gl_window(&app);
    return 0;
}
