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
5. Generated machine-readable exports from `PLANET.EXE`.
6. Wrote the first human-readable function map:
   - [reverse/work/notes/initial-function-map.md](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/notes/initial-function-map.md)

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

## Initial Findings

### Stable anchors

- Entry point: `0x004045c0`
- Window creation anchor: `0x00401510`
- Pixel format setup: `0x00401880`
- Raw TGA loader: `0x004036bb`
- MIDAS wrapper thunks: `0x00404014` to `0x0040402c`

### Program shape

The first static pass supports the following subsystem split:

- `startup`
- `win32_init`
- `opengl_init`
- `audio_wrappers`
- `audio_init`
- `assets_io`
- `assets_textures`
- `render_loop`

### String-driven naming

The following anchors were already strong enough for first-pass naming:

- `Planet Cornball`
- `aab.xm`
- `ChoosePixelFormat failed`
- `SetPixelFormat failed`
- `v1.tga`
- `s1.tga`
- `kaar128.tga`
- `fla.tga`
- `surf128.tga`

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

The workspace is now ready for interactive analysis.

The current function map is not final, but it is already useful enough to:

- avoid restarting from raw addresses
- focus the next pass on globals and scene dispatch
- keep naming decisions consistent across tools

## Next Step

Continue the static pass interactively in Cutter:

1. confirm function boundaries around the texture-loading group
2. map the global texture slot table around `0x5d7548`
3. identify the main message loop and scene dispatch chain
4. refine `render_scene_or_frame` into concrete effect functions
