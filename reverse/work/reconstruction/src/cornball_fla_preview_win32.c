#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>

#include "cornball/fla_gl.h"
#include "cornball/fla_scene.h"
#include "cornball/random.h"

typedef struct PreviewOptions {
    char asset_root[MAX_PATH];
    unsigned int max_frames;
    int hidden;
} PreviewOptions;

typedef struct PreviewApp {
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
    PreviewOptions options;
    CornballRandom random;
    CornballFlaScene scene;
    CornballFlaFrame frame;
    CornballFlaGlRenderer renderer;
} PreviewApp;

static const double kSimulationStepSeconds = 1.0 / 60.0;

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
    char fla_path[MAX_PATH];
    char logo_path[MAX_PATH];

    join_path(fla_path, sizeof(fla_path), root, "FLA.TGA");
    join_path(logo_path, sizeof(logo_path), root, "LOGOTAUS.TGA");
    return file_exists(fla_path) && file_exists(logo_path);
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

static int parse_options(int argc, char **argv, PreviewOptions *options)
{
    int i;

    memset(options, 0, sizeof(*options));

    for (i = 1; i < argc; ++i) {
        if ((strcmp(argv[i], "--asset-root") == 0) && ((i + 1) < argc)) {
            snprintf(options->asset_root, sizeof(options->asset_root), "%s", argv[i + 1]);
            ++i;
        } else if ((strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc)) {
            options->max_frames = (unsigned int)strtoul(argv[i + 1], NULL, 10);
            ++i;
        } else if (strcmp(argv[i], "--hidden") == 0) {
            options->hidden = 1;
        } else {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            return 0;
        }
    }

    return 1;
}

static int resolve_asset_root(PreviewOptions *options)
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

static LRESULT CALLBACK preview_wndproc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param)
{
    PreviewApp *app = (PreviewApp *)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

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
                cornball_fla_gl_resize(&app->renderer, app->width, app->height);
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

static int create_gl_window(PreviewApp *app)
{
    const char *class_name = "CornballFlaPreviewWindow";
    WNDCLASSA wc;
    RECT rect;
    PIXELFORMATDESCRIPTOR pfd;
    int pixel_format;
    DWORD style = WS_OVERLAPPEDWINDOW;

    memset(&wc, 0, sizeof(wc));
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = preview_wndproc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = class_name;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (RegisterClassA(&wc) == 0u) {
        return 0;
    }

    rect.left = 0;
    rect.top = 0;
    rect.right = 1280;
    rect.bottom = 720;
    AdjustWindowRect(&rect, style, FALSE);

    app->hwnd = CreateWindowExA(
        0,
        class_name,
        "Cornball FLA Preview",
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

static void destroy_gl_window(PreviewApp *app)
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

static void update_and_render(PreviewApp *app)
{
    LARGE_INTEGER now;
    double delta_seconds;
    RECT client_rect;

    QueryPerformanceCounter(&now);
    delta_seconds = (double)(now.QuadPart - app->last_tick.QuadPart) / (double)app->perf_frequency.QuadPart;
    app->last_tick = now;

    if (delta_seconds > 0.25) {
        delta_seconds = 0.25;
    }

    app->elapsed_seconds += delta_seconds;
    app->accumulator_seconds += delta_seconds;

    while (app->accumulator_seconds >= kSimulationStepSeconds) {
        cornball_fla_scene_update(&app->scene, &app->random);
        app->accumulator_seconds -= kSimulationStepSeconds;
    }

    GetClientRect(app->hwnd, &client_rect);
    app->width = client_rect.right - client_rect.left;
    app->height = client_rect.bottom - client_rect.top;
    cornball_fla_gl_resize(&app->renderer, app->width, app->height);

    cornball_fla_scene_build_frame(&app->scene, app->elapsed_seconds, &app->frame);
    cornball_fla_gl_render(&app->renderer, &app->frame);
    SwapBuffers(app->hdc);

    ++app->presented_frames;
    if ((app->options.max_frames > 0u) && (app->presented_frames >= app->options.max_frames)) {
        app->running = 0;
    }
}

int main(int argc, char **argv)
{
    PreviewApp app;
    MSG message;
    char error_message[256];

    memset(&app, 0, sizeof(app));

    if (!parse_options(argc, argv, &app.options)) {
        return 1;
    }

    if (!resolve_asset_root(&app.options)) {
        fprintf(stderr, "Could not resolve a valid asset root containing FLA.TGA and LOGOTAUS.TGA.\n");
        return 1;
    }

    cornball_random_seed(&app.random, 1u);
    cornball_fla_scene_clear(&app.scene);
    cornball_fla_scene_loader_pass(&app.scene);

    if (!create_gl_window(&app)) {
        fprintf(stderr, "Failed to create Win32 OpenGL preview window.\n");
        destroy_gl_window(&app);
        return 1;
    }

    if (!QueryPerformanceFrequency(&app.perf_frequency)) {
        fprintf(stderr, "QueryPerformanceFrequency failed.\n");
        destroy_gl_window(&app);
        return 1;
    }

    QueryPerformanceCounter(&app.last_tick);

    if (!cornball_fla_gl_init(&app.renderer, app.options.asset_root, error_message, sizeof(error_message))) {
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

        update_and_render(&app);
    }

    cornball_fla_gl_shutdown(&app.renderer);
    destroy_gl_window(&app);
    return 0;
}
