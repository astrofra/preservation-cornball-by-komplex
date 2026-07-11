# Cutter Pass 04

Date: 2026-07-12
Tooling: Cutter package `2.5.0`, using the bundled `rizin.exe`
Binary: `reverse/baseline/cornball/PLANET.EXE`

Related artifacts:

- [planet_first_pass.rz](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/tools/planet_first_pass.rz)
- [planet-timing-globals.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-timing-globals.csv)
- [planet-scene-dispatch.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-scene-dispatch.csv)

Reference material pulled for signature verification:

- [reverse/work/vendor/midas-has/downloads.md](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/vendor/midas-has/downloads.md)
- [reverse/work/vendor/midas-07/include/midasdll.h](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/vendor/midas-07/include/midasdll.h)
- [reverse/work/vendor/midas-07/doc/html/apiref/node15.html](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/vendor/midas-07/doc/html/apiref/node15.html)
- [reverse/work/vendor/midas-07/doc/html/apiref/node16.html](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/vendor/midas-07/doc/html/apiref/node16.html)

## Goal

Resolve the timing globals around `0x005d1710` through `0x005d1764` with enough confidence to promote them into the replayable naming script and the static-analysis report.

## Verified MIDAS Status Layout

`PLANET.EXE` imports `_MIDASgetPlayStatus@4` from `midas06.dll`.

The local Housemarque Audio System `0.7 beta 1` package uses the matching prototype:

- `BOOL MIDASgetPlayStatus(MIDASplayStatus *status)`

and the matching structure layout:

- `position`
- `pattern`
- `row`
- `syncInfo`

This is the important compatibility point for the demo: the binary touches `+0` and `+8` inside the status buffer, which line up exactly with `position` and `row` in the published `0.7` header.

## Timing-Global Map

| Address | Working name | Confidence | Notes |
| --- | --- | --- | --- |
| `0x005d1710` | `g_demo_elapsed_seconds` | high | Stores `(current_tick_ms - g_demo_start_tick_ms) * 0.001`. No downstream absolute reads found yet. |
| `0x005d1718` | `g_cached_music_position` | medium | Per-frame copy of `MIDASplayStatus.position`. |
| `0x005d1728` | `g_scene_elapsed_seconds` | high | Stores `(current_tick_ms - g_scene_start_tick_ms) * 0.001` and is the scene-facing timer used across the renderer. |
| `0x005d1730` | `g_demo_start_tick_ms` | high | Initial `GetTickCount()` snapshot written once when the main loop starts. |
| `0x005d1734` | `g_scene_start_tick_ms` | high | Initialized at loop start, then reset only when the scene index changes. |
| `0x005d1738` | `g_cached_music_row` | medium | Per-frame copy of `MIDASplayStatus.row`. |
| `0x005d1750` | `g_midas_play_status_position` | high | Read by the dispatcher and compared against the scene threshold table. |
| `0x005d1754` | `g_midas_play_status_pattern` | medium | Struct field confirmed from the published `0.7` header, but no binary reads found yet. |
| `0x005d1758` | `g_midas_play_status_row` | high | Read by the dispatcher and copied into `g_cached_music_row`. |
| `0x005d175c` | `g_midas_play_status_sync_info` | medium | Struct field confirmed from the published `0.7` header, but no binary reads found yet. |
| `0x005d1760` | `g_current_scene_index` | high | Active scene-family index used for jump-table dispatch. |
| `0x005d1764` | `g_previous_scene_index` | high | Snapshot of the scene-family index before any change is applied. |

## Key Control-Flow Findings

### `run_main_loop`

Before entering the repeating message/render loop at `0x00401459`, the code calls `GetTickCount()` once and writes the result to both:

- `g_demo_start_tick_ms`
- `g_scene_start_tick_ms`

That seeds both the global demo clock and the first scene-entry timestamp.

Later in the same loop, the code pushes the buffer at `0x005d1750` to `_MIDASgetPlayStatus@4`, then renders and swaps buffers.

### `dispatch_scene_by_music_position`

The dispatcher begins by reading `g_midas_play_status_position` and comparing it against the threshold table at `0x0040e068`.

When the resolved scene index changes:

- the old index is copied to `g_previous_scene_index`
- `GetTickCount()` is called again
- `g_scene_start_tick_ms` is refreshed
- `g_current_scene_index` is updated

The dispatcher then derives two values with the scale constant at `0x0040d078`:

- `g_demo_elapsed_seconds = (current_tick_ms - g_demo_start_tick_ms) * 0.001`
- `g_scene_elapsed_seconds = (current_tick_ms - g_scene_start_tick_ms) * 0.001`

The constant is exactly `0.001`, so the second expression is a direct milliseconds-to-seconds conversion.

## Interpretation

The demo uses a hybrid timing model:

- tracker `position` drives coarse scene changes
- wall-clock `GetTickCount()` deltas drive most intra-scene animation

This matters for reconstruction because the renderer does not need to be rebuilt as a tracker-row-accurate simulation first. A practical C or C++ rewrite can expose:

- a music-position input for scene selection
- a `scene_time_seconds` input for effect-local animation

and still preserve the current architecture closely.

## Current Unknowns

- `g_demo_elapsed_seconds` is written every dispatch, but no current consumer has been confirmed
- `pattern` and `syncInfo` exist in the imported MIDAS status structure, but the demo has no direct reads from those fields yet
- `row` is currently only observed as a cached copy, not as an active scene input
