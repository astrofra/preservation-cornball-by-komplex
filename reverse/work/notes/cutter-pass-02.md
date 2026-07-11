# Cutter Pass 02

Date: 2026-07-11
Tooling: Cutter package `2.5.0`, using the bundled `rizin.exe`
Binary: `reverse/baseline/cornball/PLANET.EXE`

Related artifacts:

- [planet_first_pass.rz](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/tools/planet_first_pass.rz)
- [save-rizin-project.ps1](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/tools/save-rizin-project.ps1)
- [planet-wndproc-messages.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-wndproc-messages.csv)
- [planet-function-map.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-function-map.csv)
- [planet-pass-01.rzdb](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/projects/cutter/planet-pass-01.rzdb)

## Goal

Pass 02 focused on the next structural target from pass 01:

- recover `main_wndproc` as a real analyzed function
- identify the helper at `0x00401090`
- identify the helper at `0x004010f0`

## Recovered `main_wndproc`

The callback at `0x00401680` can now be treated as a proper function:

- start: `0x00401680`
- end: `0x00401850`
- return shape: `ret 0x10`
- calling convention: `stdcall`

This is the expected Win32 `LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM)` shape.

The function was manually promoted with:

```text
s 0x00401680
af
afu 0x00401850
afn main_wndproc
afc stdcall
```

## Window Message Map

Recovered message handling:

| Message | Code | Handler | Effect |
| --- | --- | --- | --- |
| `WM_CREATE` | `0x0001` | `0x00401715` | Acquires the window DC, applies the pixel format, creates/makes current the WGL context, initializes GL state/resources, then sizes the viewport. |
| `WM_DESTROY` | `0x0002` | `0x004016da` | Deletes the WGL context, releases the DC, and posts quit. |
| `WM_SIZE` | `0x0005` | `0x004016ae` | Calls `GetClientRect`, then forwards `right` and `bottom` to the viewport/projection helper. |
| `WM_PAINT` | `0x000f` | `0x004017d1` | Executes only `BeginPaint` / `EndPaint`. |
| `WM_CLOSE` | `0x0010` | `0x00401809` | Deletes the GL context, releases the DC, clears globals, then destroys the window. |
| `WM_KEYDOWN` | `0x0100` | `0x004017f7` | Posts quit immediately. |
| `WM_MOUSEMOVE` | `0x0200` | `0x004017b7` | Stores mouse X/Y from `LOWORD/HIWORD(lParam)` into globals `0x005d173c` and `0x005d1740`. |
| default | `*` | `0x00401797` | Falls back to `DefWindowProcA`. |

This is exported in [planet-wndproc-messages.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-wndproc-messages.csv).

## `configure_viewport_and_projection` at `0x00401090`

The helper at `0x00401090` is now clearly identified as the resize/projection routine.

Observed behavior:

- calls `glViewport(0, 0, width, height)`
- switches to `GL_PROJECTION`
- loads the identity matrix
- calls `gluPerspective`
- switches back to `GL_MODELVIEW`

Recovered perspective parameters:

- field of view: `65.0`
- near plane: `1.0`
- far plane: `90.0`
- aspect ratio: `width / height` driven from the caller's client size values

Callers:

- `main_wndproc` on `WM_SIZE`
- `initialize_gl_state_and_resources`

## `initialize_gl_state_and_resources` at `0x004010f0`

The larger helper at `0x004010f0` is not just a resize helper. It performs first-time GL runtime setup.

Observed behavior:

- sets clear color to black
- sets clear depth to `1.0`
- sets depth function to `GL_LEQUAL`
- enables depth testing
- sets smooth shading
- configures pixel unpack alignment
- calls `0x00401000`, which appears to build a small procedural geometry or lookup table
- seeds a contiguous block of five texture IDs at `0x005510d0`
- uploads five `256x256x32` RGBA textures from contiguous memory starting at `0x004110d0`
- enables and configures fog
- enables back-face culling
- finishes by calling `configure_viewport_and_projection(width, height)`

This function is called from the `WM_CREATE` path of `main_wndproc`.

## New Named Globals

The pass also added two concrete interaction globals:

- `g_mouse_x` at `0x005d173c`
- `g_mouse_y` at `0x005d1740`

## Outcome

After pass 02, the Win32/OpenGL bootstrap path is much cleaner:

- startup enters `main`
- `main` registers `main_wndproc`
- `WM_CREATE` constructs the GL context and runtime resources
- `WM_SIZE` reconfigures the projection
- the main loop later dispatches scene rendering

This is a solid base for moving next into scene renderer refinement or timing-global naming.
