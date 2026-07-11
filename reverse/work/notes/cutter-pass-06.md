# Cutter Pass 06

Date: 2026-07-12
Tooling: Cutter package `2.5.0`, using the bundled `rizin.exe`
Binary: `reverse/baseline/cornball/PLANET.EXE`

Related artifacts:

- [planet-scene-state-blocks.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-scene-state-blocks.csv)
- [planet-scene-helper-map.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-scene-helper-map.csv)
- [planet-timing-globals.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-timing-globals.csv)

## Goal

Determine whether the remaining MIDAS status fields are consumed by scene-side code and resolve ownership of the `0x005d79xx` / `0x005d7fxx` state ranges.

## MIDAS status usage

The binary still does not show any scene-side consumer for:

- `MIDASplayStatus.pattern`
- `MIDASplayStatus.syncInfo`

`MIDASplayStatus.row` is copied into `g_cached_music_row`, but no downstream renderer read has been found so far.

That leaves the current timing contract as:

- scene selection driven by `MIDASplayStatus.position`
- scene animation driven primarily by `GetTickCount()` deltas

## Helper-owned state, not family-owned state

Tracing all six scene families against the `0x005d79xx` and `0x005d7fxx` ranges shows that these globals belong to shared helpers rather than to `kaar` or `surf` directly.

### `draw_jittered_overlay_quad`

Resolved ownership:

- `0x005d7948` -> `g_overlay_jitter_y`
- `0x005d7f50` -> `g_overlay_jitter_x`
- `0x005d7f54` -> `g_overlay_call_counter`

Confirmed behavior:

- the helper tests `(g_overlay_call_counter & reseed_mask) == 0`
- if true, it reseeds X and Y from `lcg_rand15`
- it then increments the call counter
- it draws a small blended quad around the current pseudo-random offset

Caller-side masks:

- intro/logo: `1`
- `s` pair: `1`
- `kaar`: `1`
- `fla`: `1`
- finale: `1`
- `surf`: `0`

So `surf` forces a reseed on every helper call, while the other families reseed every other call.

### `draw_cached_tube_shell`

Resolved ownership:

- `0x005d7950` -> `tube_ring_vertices_neg_z`
- `0x005d7c50` -> `tube_ring_vertices_pos_z`
- `0x005d7f58` -> `g_tube_shell_cache_valid`

No direct `kaar` or `surf` state block was found in this range. Both families simply consume the helper-owned cache.

## Outcome

Pass 06 removes two scene-side ambiguities:

- the remaining MIDAS status fields are not yet part of the renderer contract
- the `0x005d79xx` / `0x005d7fxx` ranges are shared helper state, not private family state

This keeps the next scene-reconstruction pass focused on the actual scene-owned data blocks instead of chasing shared support globals.
