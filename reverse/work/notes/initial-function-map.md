# Initial Function Map

Date: 2026-07-11
Binary: `reverse/baseline/cornball/PLANET.EXE`

> This note captures the pre-Cutter naming pass and contains addresses later corrected by interactive analysis.
> Use [cutter-pass-01.md](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/notes/cutter-pass-01.md) as the current reference.

Source data:

- [planet-function-map.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-function-map.csv)
- [planet-string-xrefs.json](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-string-xrefs.json)
- [planet-imports.json](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-imports.json)

## Current Naming Pass

These names are proposed working names, not final recovered names.

| VA | Working name | Subsystem | Confidence | Reason |
| --- | --- | --- | --- | --- |
| `0x00401510` | `create_main_window` | `win32_init` | High | Uses `Planet Cornball`, `RegisterClassA`, `CreateWindowExA`, `ShowWindow`. |
| `0x00401880` | `setup_pixel_format` | `opengl_init` | High | Calls `ChoosePixelFormat` and `SetPixelFormat`, references setup error strings. |
| `0x00401930` | `render_scene_or_frame` | `render_loop` | Medium | Starts with GL state setup and x87-heavy transforms. |
| `0x00401c10` | `load_intro_textures` | `assets_textures` | High | Loads `v1`, `v2`, `txt1`, `txt2`, `logo`, `logotaus`. |
| `0x00402140` | `load_scene_s_textures` | `assets_textures` | High | Loads `s1`, `s2`, `txt2`. |
| `0x004025d0` | `load_kaar_textures` | `assets_textures` | High | Loads `kaar128`, `txt1`, `txt2`. |
| `0x00402b70` | `load_fla_texture_group` | `assets_textures` | Medium | References `fla.tga` and shared texture IDs. |
| `0x00403130` | `load_surf_texture_group` | `assets_textures` | High | Loads `surf128`, `fla`, `txt1`. |
| `0x004036a0` | `ensure_texture_loaded` | `assets_textures` | High | Texture cache check by slot/index before raw load. |
| `0x004036bb` | `load_tga_256x256_rgba` | `assets_io` | High | Reads 18-byte header, allocates `0x40000`, swaps BGR(A) to RGB(A). |
| `0x00404014` | `midas_play_module` | `audio_wrappers` | High | Import thunk. |
| `0x0040401a` | `midas_load_module` | `audio_wrappers` | High | Import thunk. |
| `0x00404020` | `midas_start_background_play` | `audio_wrappers` | High | Import thunk. |
| `0x00404026` | `midas_init` | `audio_wrappers` | High | Import thunk. |
| `0x0040402c` | `midas_startup` | `audio_wrappers` | High | Import thunk. |
| `0x004045c0` | `crt_startup_main` | `startup` | High | MSVC CRT startup path. |
| `0x004015dd` | `load_music_module` | `audio_init` | High | References `aab.xm` and MIDAS module load/play flow. |

## Subsystem Split

Current subsystem buckets are:

- `startup`
- `win32_init`
- `opengl_init`
- `audio_wrappers`
- `audio_init`
- `assets_io`
- `assets_textures`
- `render_loop`

This split is strong enough to continue naming in Cutter or Ghidra without starting from raw addresses.

## Strong String Anchors

Useful anchor strings and first code references:

- `Planet Cornball` -> `0x0040151c`
- `aab.xm` -> `0x004015de`
- `ChoosePixelFormat failed` -> `0x004018bd`
- `SetPixelFormat failed` -> `0x0040190b`
- `v1.tga` -> `0x00401c16`
- `s1.tga` -> `0x00402146`
- `kaar128.tga` -> `0x004025d6`
- `fla.tga` -> `0x00402b7c`
- `surf128.tga` -> `0x00403136`

## Open Questions

- `0x00401930` likely belongs to the main frame renderer, but it still needs a cleaner scene/timing breakdown.
- `0x00402b70` should be rechecked interactively to see whether it is a loader, a scene initializer, or both.
- The global texture slot table around `0x5d7548` should be mapped next.
- The main message loop and effect dispatch chain still need an explicit call graph.
