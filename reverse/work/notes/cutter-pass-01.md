# Cutter Pass 01

Date: 2026-07-11
Tooling: Cutter package `2.5.0`, using the bundled `rizin.exe`
Binary: `reverse/baseline/cornball/PLANET.EXE`

Related artifacts:

- [planet_first_pass.rz](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/tools/planet_first_pass.rz)
- [save-rizin-project.ps1](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/tools/save-rizin-project.ps1)
- [planet-function-map.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-function-map.csv)
- [planet-texture-slots.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-texture-slots.csv)
- [planet-scene-dispatch.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-scene-dispatch.csv)
- [planet-pass-01.rzdb](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/projects/cutter/planet-pass-01.rzdb)

## Main Structural Corrections

The first automated pass underestimated the top-level structure. The Cutter/Rizin pass clarifies it:

- `0x00401510` is the recovered `main`, not just a window creation helper.
- `0x00401430` is the main Win32 message/render loop.
- `0x004012e0` is a scene dispatcher driven by music playback state and timeline thresholds.
- `0x00401680` is the window procedure address passed to `RegisterClassA`, even though it was not auto-promoted to a function boundary in this pass.

## Confirmed Top-Level Flow

### `main` at `0x00401510`

This function:

- registers the window class
- creates the main window
- initializes MIDAS
- starts background playback
- loads `aab.xm`
- shows and updates the window
- runs the main loop
- shuts audio down on exit

### `run_main_loop` at `0x00401430`

This function:

- records tick counters
- uses `PeekMessageA`
- drains the queue with `GetMessageA`, `TranslateMessage`, and `DispatchMessageA`
- polls `MIDASgetPlayStatus`
- clears the frame
- resets the matrix state
- calls the scene dispatcher
- sleeps for `10` ms
- finishes GL work and swaps buffers
- loops while the dispatcher returns non-zero

This is the core real-time loop of the demo.

### `dispatch_scene_by_music_position` at `0x004012e0`

This function:

- compares the current music/playback value against a threshold table at `0x0040e068`
- computes timing deltas
- stores normalized timing values into globals used by renderers
- dispatches through a 10-case jump table at `0x004013fc`

Threshold table values:

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

## Scene Dispatch Map

| Scene index | Threshold | Target |
| ---: | --- | --- |
| `0` | `0x0000000c` | `render_scene_case_0_7` at `0x00401930` |
| `1` | `0x00000012` | `render_scene_case_1_8` at `0x00402330` |
| `2` | `0x00000016` | `render_scene_case_2_4` at `0x00401fc0` |
| `3` | `0x00000019` | `render_scene_case_3_6` at `0x00402d70` |
| `4` | `0x0000001b` | `render_scene_case_2_4` at `0x00401fc0` |
| `5` | `0x0000001e` | `render_scene_case_5` at `0x004027c0` |
| `6` | `0x00000022` | `render_scene_case_3_6` at `0x00402d70` |
| `7` | `0x00000023` | `render_scene_case_0_7` at `0x00401930` |
| `8` | `0x00000026` | `render_scene_case_1_8` at `0x00402330` |
| `9` | `0x00000029` | `render_scene_case_9` at `0x00403320` |

This means the demo uses a small number of renderer families reused across multiple timeline slots.

## Texture Slot Table

The texture cache base is:

- `texture_slot_table` at `0x005d7548`

`ensure_texture_loaded` at `0x004036a0` uses:

- `slot_address = 0x005d7548 + slot * 4`

Confirmed slot mapping:

| Slot | Address | Asset | First loader |
| ---: | --- | --- | --- |
| `1` | `0x005d754c` | `v1.tga` | `load_intro_textures` |
| `2` | `0x005d7550` | `v2.tga` | `load_intro_textures` |
| `3` | `0x005d7554` | `txt1.tga` | `load_intro_textures` |
| `4` | `0x005d7558` | `txt2.tga` | `load_intro_textures` |
| `5` | `0x005d755c` | `logo.tga` | `load_intro_textures` |
| `6` | `0x005d7560` | `logotaus.tga` | `load_intro_textures` |
| `9` | `0x005d756c` | `kaar128.tga` | `load_kaar_textures` |
| `10` | `0x005d7570` | `surf128.tga` | `load_surf_texture_group` |
| `11` | `0x005d7574` | `fla.tga` | `load_fla_texture_group` |
| `12` | `0x005d7578` | `s1.tga` | `load_scene_s_textures` |
| `13` | `0x005d757c` | `s2.tga` | `load_scene_s_textures` |

This is a strong structural anchor for continued reconstruction.

## Initialization Flags

The scene-specific texture loaders are guarded by one-shot flags:

- `0x005d1768` for `load_intro_textures`
- `0x005d176c` for `load_scene_s_textures`
- `0x005d1770` for `load_kaar_textures`
- `0x005d753c` for `load_fla_texture_group`
- `0x005d7540` for `load_surf_texture_group`

## Window Procedure

The callback at `0x00401680` clearly handles:

- creation / first GL setup
- resize via `GetClientRect` and GL resize helpers
- teardown via `wglDeleteContext`, `ReleaseDC`, and `PostQuitMessage`

It should be treated as the main window procedure in future naming work.

## Reusable Script

The first-pass naming script is:

- [planet_first_pass.rz](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/tools/planet_first_pass.rz)
- [save-rizin-project.ps1](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/tools/save-rizin-project.ps1)

Intended usage:

```powershell
powershell -ExecutionPolicy Bypass -File reverse/work/tools/save-rizin-project.ps1
```

Notes:

- the `.rz` file is the naming source of truth
- the PowerShell wrapper replays it after analysis and saves `planet-pass-01.rzdb`
- direct `rizin -A -i ...` replay was not stable enough in this Cutter/Rizin build

## Next Focus

The next Cutter pass should target:

- function boundary recovery around `0x00401680`
- explicit naming of the resize helpers around `0x00401090` and `0x004010f0`
- scene renderer refinement for `0x00401930`, `0x00401fc0`, `0x00402330`, `0x004027c0`, `0x00402d70`, and `0x00403320`
- global timing variables around `0x005d1710` through `0x005d1738`
