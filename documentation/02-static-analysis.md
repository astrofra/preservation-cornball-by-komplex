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
10. Saved a reusable Rizin project database:
   - [reverse/work/projects/cutter/planet-pass-01.rzdb](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/projects/cutter/planet-pass-01.rzdb)

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

### Scene dispatch

The jump table currently resolves to these renderer families:

- scenes `0` and `7` -> `render_scene_case_0_7` at `0x00401930`
- scenes `1` and `8` -> `render_scene_case_1_8` at `0x00402330`
- scenes `2` and `4` -> `render_scene_case_2_4` at `0x00401fc0`
- scenes `3` and `6` -> `render_scene_case_3_6` at `0x00402d70`
- scene `5` -> `render_scene_case_5` at `0x004027c0`
- scene `9` -> `render_scene_case_9` at `0x00403320`

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

## Current Deliverable Quality

The workspace is now ready for continued interactive analysis with a documented first Cutter pass.

The current map is not final, but it is already useful enough to:

- avoid restarting from raw addresses
- focus the next pass on scene renderer internals and timing globals
- keep naming decisions consistent across tools

## Next Step

Continue the static pass interactively in Cutter:

1. recover a clean function boundary for `main_wndproc` at `0x00401680`
2. name the resize helpers around `0x00401090` and `0x004010f0`
3. refine the six scene renderer families into concrete effect routines
4. document the timing globals around `0x005d1710` through `0x005d1738`
