# Step 2 - Static Reverse Engineering Workspace

Date: 2026-07-11
Target binary: `reverse/baseline/cornball/PLANET.EXE`

## Goal

Set up a repeatable static-analysis workspace, install at least one open-source interactive reverse-engineering tool, and generate the first naming and subsystem map from the binary.

## Completed Actions

1. Created a structured static-analysis workspace under `reverse/work/`:
   - `tools/`
   - `exports/`
   - `notes/`
   - `projects/cutter/`
2. Installed Cutter `2.5.0` via `winget`.
3. Added a local launcher script for Cutter:
   - [reverse/work/tools/launch-cutter.ps1](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/tools/launch-cutter.ps1)
4. Added a repeatable local analysis script:
   - [reverse/work/tools/planet_static_report.py](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/tools/planet_static_report.py)
5. Added a replayable first-pass Rizin naming script:
   - [reverse/work/tools/planet_first_pass.rz](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/tools/planet_first_pass.rz)
6. Added a wrapper that rebuilds a saved Rizin project from the binary:
   - [reverse/work/tools/save-rizin-project.ps1](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/tools/save-rizin-project.ps1)
7. Generated machine-readable exports from `PLANET.EXE`.
8. Wrote the first human-readable function map:
   - [reverse/work/notes/initial-function-map.md](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/notes/initial-function-map.md)
9. Ran the first interactive Cutter/Rizin correction pass:
   - [reverse/work/notes/cutter-pass-01.md](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/notes/cutter-pass-01.md)
10. Ran the second static pass on the Win32/OpenGL bootstrap path:
   - [reverse/work/notes/cutter-pass-02.md](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/notes/cutter-pass-02.md)
11. Saved a reusable Rizin project database:
   - [reverse/work/projects/cutter/planet-pass-01.rzdb](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/projects/cutter/planet-pass-01.rzdb)
12. Ran the third static pass on the scene renderer families and shared helper routines:
   - [reverse/work/notes/cutter-pass-03.md](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/notes/cutter-pass-03.md)

## Cutter Installation

Installed package:

- `Rizin.Cutter` version `2.5.0`

Resolved executable path at install time:

- `C:\Users\fra\AppData\Local\Microsoft\WinGet\Packages\Rizin.Cutter_Microsoft.Winget.Source_8wekyb3d8bbwe\Cutter-v2.5.0-Windows-x86_64\cutter.exe`

Note:

- the command alias was added by the installer, but the current shell did not pick it up immediately
- the launcher script resolves the installed path directly and does not depend on shell refresh

## Generated Exports

Created under `reverse/work/exports/`:

- [planet-metadata.json](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-metadata.json)
- [planet-sections.json](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-sections.json)
- [planet-imports.json](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-imports.json)
- [planet-string-xrefs.json](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-string-xrefs.json)
- [planet-instruction-profile.json](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-instruction-profile.json)
- [planet-strings.txt](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-strings.txt)
- [planet-function-map.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-function-map.csv)
- [planet-texture-slots.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-texture-slots.csv)
- [planet-scene-dispatch.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-scene-dispatch.csv)
- [planet-scene-family-map.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-scene-family-map.csv)
- [planet-scene-helper-map.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-scene-helper-map.csv)
- [planet-wndproc-messages.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-wndproc-messages.csv)

## Cutter Pass 01 Findings

### Stable anchors

- Entry point: `0x004045c0`
- Recovered `main`: `0x00401510`
- Main message/render loop: `0x00401430`
- Scene dispatch by music position: `0x004012e0`
- Window procedure: `0x00401680`
- Pixel format setup: `0x00401860`
- Raw TGA loader: `0x004036bb`
- MIDAS wrapper thunks: `0x00404014` to `0x0040402c`

### Program shape

The corrected pass supports the following subsystem split:

- `startup`
- `main_loop`
- `render_dispatch`
- `scene_render`
- `win32_init`
- `opengl_init`
- `audio_wrappers`
- `audio_init`
- `assets_io`
- `assets_textures`

### Top-level flow

- `main` creates the window, initializes audio, starts playback, shows the window, and calls the main loop.
- `run_main_loop` owns the Win32 message pump, `MIDASgetPlayStatus` polling, frame clear/reset, dispatch call, and `SwapBuffers`.
- `dispatch_scene_by_music_position` uses the threshold table at `0x0040e068` and the jump table at `0x004013fc`.

### Win32 and GL bootstrap

Pass 02 recovered the bootstrap path in more detail:

- `main_wndproc` is now recovered as a real `stdcall` window procedure from `0x00401680` to `0x00401850`
- `configure_viewport_and_projection` at `0x00401090` owns `glViewport` plus the `gluPerspective` setup
- `initialize_gl_state_and_resources` at `0x004010f0` owns first-time GL state, fog/culling setup, staged texture uploads, and the initial projection sizing

Recovered handled messages:

- `WM_CREATE`
- `WM_DESTROY`
- `WM_SIZE`
- `WM_PAINT`
- `WM_CLOSE`
- `WM_KEYDOWN`
- `WM_MOUSEMOVE`

### Scene dispatch

The jump table currently resolves to these renderer families:

- scenes `0` and `7` -> `render_scene_intro_logo_family` at `0x00401930`
- scenes `1` and `8` -> `render_scene_kaar_family` at `0x00402330`
- scenes `2` and `4` -> `render_scene_s_pair_family` at `0x00401fc0`
- scenes `3` and `6` -> `render_scene_surf_family` at `0x00402d70`
- scene `5` -> `render_scene_fla_particle_family` at `0x004027c0`
- scene `9` -> `render_scene_finale_family` at `0x00403320`

Threshold values exported in [planet-scene-dispatch.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-scene-dispatch.csv):

- `0x0c`
- `0x12`
- `0x16`
- `0x19`
- `0x1b`
- `0x1e`
- `0x22`
- `0x23`
- `0x26`
- `0x29`

### Texture cache mapping

The texture cache base is now anchored at `0x005d7548`, with confirmed slot-to-asset mappings exported in [planet-texture-slots.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-texture-slots.csv).

Confirmed assets include:

- `v1.tga`
- `v2.tga`
- `txt1.tga`
- `txt2.tga`
- `logo.tga`
- `logotaus.tga`
- `kaar128.tga`
- `surf128.tga`
- `fla.tga`
- `s1.tga`
- `s2.tga`

### Instruction profile

The machine-generated profile confirms a math-heavy renderer:

- `839` calls
- `806` branches
- `646` x87 instructions

Most common x87 instructions in the top group:

- `fld`
- `fstp`
- `fmul`
- `fxch`
- `fnstsw`

This is consistent with late-1990s procedural 3D demo code and reinforces the expectation that scene code will need manual cleanup even when decompiled.

## Cutter Pass 03 Findings

Pass 03 shifted from bootstrap analysis to the scene renderer itself.

### Refined scene families

The six dispatch targets can now be treated as effect families instead of anonymous case buckets:

- scenes `0` and `7` -> `render_scene_intro_logo_family`
- scenes `1` and `8` -> `render_scene_kaar_family`
- scenes `2` and `4` -> `render_scene_s_pair_family`
- scenes `3` and `6` -> `render_scene_surf_family`
- scene `5` -> `render_scene_fla_particle_family`
- scene `9` -> `render_scene_finale_family`

These mappings are exported in [planet-scene-family-map.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-scene-family-map.csv).

### Shared scene helpers

The scene families reuse a small support library:

- `draw_dual_texture_panel_pair` at `0x00403760`
- `draw_soft_blended_quad` at `0x00403c90`
- `draw_centered_textured_quad` at `0x00403d70`
- `draw_cached_ring_strip` at `0x00403e40`
- `draw_jittered_overlay_quad` at `0x00403b40`
- `draw_timed_fade_quad` at `0x004039d0`
- `lcg_rand15` at `0x004040b0`
- `fmod_x87_helper` at `0x004040e0`

These mappings are exported in [planet-scene-helper-map.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-scene-helper-map.csv).

### Scene-side interpretation

Current working interpretation:

- the intro/logo and `s` scenes share the same dual-panel primitive, but feed it different texture pairs
- the `kaar` and `surf` scenes both depend on a cached sin/cos strip helper, making them stronger candidates for geometry-heavy ribbon, ring, or tube effects
- the `fla` scene owns the most obvious per-element state loop and is currently the best particle-field candidate
- the finale scene reuses the shared quad art and ends with a dedicated time-based fade helper

### New scene-side anchors

Pass 03 also provided several stable helpers and state blocks for later source reconstruction:

- startup polar table builder at `0x00401000`
- basic quad texture loader at `0x00403400`
- ring-strip vertex caches around `0x005d7950` and `0x005d7c50`
- overlay jitter state around `0x005d7948`, `0x005d7f50`, and `0x005d7f54`

## Current Deliverable Quality

The workspace is now ready for continued interactive analysis with a documented three-pass Cutter workflow.

The current map is still not final, but it is already useful enough to:

- avoid restarting from raw addresses
- separate scene-family logic from reusable helper routines
- focus the next pass on timing globals and scene-local state
- keep naming decisions consistent across tools

## Next Step

Continue the static pass interactively in Cutter:

1. document the timing globals around `0x005d1710` through `0x005d1738`
2. resolve the exact geometry emitted by `draw_dual_texture_panel_pair` and `draw_cached_ring_strip`
3. continue naming scene-local state blocks in the `0x005d79xx` and `0x005d7fxx` ranges
4. begin drafting C-like pseudocode for the most stable scene helpers before tackling full-family reconstruction
