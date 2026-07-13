#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>

#include "cornball/capture.h"
#include "cornball/fla_gl.h"
#include "cornball/fla_scene.h"
#include "cornball/intro_gl.h"
#include "cornball/intro_scene.h"
#include "cornball/kaar_gl.h"
#include "cornball/kaar_scene.h"
#include "cornball/random.h"
#include "cornball/surf_gl.h"
#include "cornball/surf_scene.h"

typedef enum ReplayFamily {
    REPLAY_FAMILY_INTRO = 0,
    REPLAY_FAMILY_KAAR = 1,
    REPLAY_FAMILY_SURF = 2,
    REPLAY_FAMILY_FLA = 3
} ReplayFamily;

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
    int hidden;
    int window_width;
    int window_height;
    double demo_seconds;
    double position_seconds;
} ReplayOptions;

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
    unsigned int presented_frames;
    ReplayOptions options;
    ReplaySelection selection;
    CornballRandom random;
    CornballIntroScene intro_scene;
    CornballIntroFrame intro_frame;
    CornballIntroGlRenderer intro_renderer;
    CornballKaarScene kaar_scene;
    CornballKaarFrame kaar_frame;
    CornballKaarGlRenderer kaar_renderer;
    CornballSurfScene surf_scene;
    CornballSurfFrame surf_frame;
    CornballSurfGlRenderer surf_renderer;
    CornballFlaScene fla_scene;
    CornballFlaFrame fla_frame;
    CornballFlaGlRenderer fla_renderer;
} ReplayApp;

static const double kSimulationStepSeconds = 1.0 / 60.0;
static const double kDefaultPositionSeconds = 1.0;
static const int kDefaultWindowWidth = 640;
static const int kDefaultWindowHeight = 400;

static const ReplaySegment kReplaySegments[] = {
    {0u, REPLAY_FAMILY_INTRO, 12.0},
    {1u, REPLAY_FAMILY_KAAR, 6.0},
    {3u, REPLAY_FAMILY_SURF, 3.0},
    {5u, REPLAY_FAMILY_FLA, 3.0},
    {6u, REPLAY_FAMILY_SURF, 4.0},
    {7u, REPLAY_FAMILY_INTRO, 1.0},
    {8u, REPLAY_FAMILY_KAAR, 3.0}
};

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

static int parse_options(int argc, char **argv, ReplayOptions *options)
{
    int i;

    memset(options, 0, sizeof(*options));
    options->random_seed = 1u;
    options->window_width = kDefaultWindowWidth;
    options->window_height = kDefaultWindowHeight;
    options->position_seconds = kDefaultPositionSeconds;

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
    app->selection = replay_select(app->elapsed_seconds, app->options.position_seconds);

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
    }
}

static void warmup_replay(ReplayApp *app)
{
    double total_seconds;

    total_seconds = replay_total_seconds(app->options.position_seconds);
    if (app->options.demo_seconds > total_seconds) {
        app->options.demo_seconds = total_seconds;
    }

    app->elapsed_seconds = 0.0;
    step_current_segment(app);

    while ((app->elapsed_seconds + kSimulationStepSeconds) <= (app->options.demo_seconds + 1e-9)) {
        app->elapsed_seconds += kSimulationStepSeconds;
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

    if (!cornball_surf_gl_init(&app->surf_renderer, app->options.asset_root, error_message, error_message_size)) {
        cornball_kaar_gl_shutdown(&app->kaar_renderer);
        cornball_intro_gl_shutdown(&app->intro_renderer);
        return 0;
    }

    if (!cornball_fla_gl_init(&app->fla_renderer, app->options.asset_root, error_message, error_message_size)) {
        cornball_surf_gl_shutdown(&app->surf_renderer);
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

    case REPLAY_FAMILY_SURF:
        cornball_surf_gl_resize(&app->surf_renderer, app->width, app->height);
        cornball_surf_gl_render(&app->surf_renderer, &app->surf_frame);
        break;

    case REPLAY_FAMILY_FLA:
        cornball_fla_gl_resize(&app->fla_renderer, app->width, app->height);
        cornball_fla_gl_render(&app->fla_renderer, &app->fla_frame);
        break;
    }
}

static int update_and_render(ReplayApp *app)
{
    LARGE_INTEGER now;
    double delta_seconds;
    double total_seconds;
    RECT client_rect;

    QueryPerformanceCounter(&now);
    delta_seconds = (double)(now.QuadPart - app->last_tick.QuadPart) / (double)app->perf_frequency.QuadPart;
    app->last_tick = now;

    total_seconds = replay_total_seconds(app->options.position_seconds);

    if (automated_mode_enabled(app)) {
        if (app->presented_frames > 0u) {
            app->elapsed_seconds += kSimulationStepSeconds;
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

        while (app->accumulator_seconds >= kSimulationStepSeconds) {
            app->elapsed_seconds += kSimulationStepSeconds;
            if (app->elapsed_seconds > total_seconds) {
                app->elapsed_seconds = total_seconds;
            }
            step_current_segment(app);
            app->accumulator_seconds -= kSimulationStepSeconds;

            if (app->elapsed_seconds >= total_seconds) {
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

    if (app->elapsed_seconds >= total_seconds) {
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
            "Could not resolve a valid asset root containing V1.TGA, V2.TGA, TXT1.TGA, TXT2.TGA, LOGO.TGA, LOGOTAUS.TGA, KAAR128.TGA, SURF128.TGA, and FLA.TGA.\n"
        );
        return 1;
    }

    cornball_random_seed(&app.random, app.options.random_seed);

    cornball_intro_scene_clear(&app.intro_scene);
    cornball_intro_scene_loader_pass(&app.intro_scene);
    cornball_kaar_scene_clear(&app.kaar_scene);
    cornball_kaar_scene_loader_pass(&app.kaar_scene);
    cornball_surf_scene_clear(&app.surf_scene);
    cornball_surf_scene_loader_pass(&app.surf_scene);
    cornball_fla_scene_clear(&app.fla_scene);
    cornball_fla_scene_loader_pass(&app.fla_scene);

    warmup_replay(&app);

    if (!create_gl_window(&app)) {
        fprintf(stderr, "Failed to create Win32 OpenGL replay window.\n");
        destroy_gl_window(&app);
        return 1;
    }

    if (!QueryPerformanceFrequency(&app.perf_frequency)) {
        fprintf(stderr, "QueryPerformanceFrequency failed.\n");
        destroy_gl_window(&app);
        return 1;
    }

    QueryPerformanceCounter(&app.last_tick);

    if (!init_renderers(&app, error_message, sizeof(error_message))) {
        fprintf(stderr, "OpenGL renderer init failed: %s\n", error_message);
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
    destroy_gl_window(&app);
    return 0;
}
