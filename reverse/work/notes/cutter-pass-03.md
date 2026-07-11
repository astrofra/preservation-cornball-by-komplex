# Cutter Pass 03

Date: 2026-07-12
Tooling: Cutter package `2.5.0`, using the bundled `rizin.exe`
Binary: `reverse/baseline/cornball/PLANET.EXE`

Related artifacts:

- [planet_first_pass.rz](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/tools/planet_first_pass.rz)
- [planet-function-map.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-function-map.csv)
- [planet-scene-dispatch.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-scene-dispatch.csv)
- [planet-scene-family-map.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-scene-family-map.csv)
- [planet-scene-helper-map.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-scene-helper-map.csv)

## Goal

Pass 03 focused on the scene side of the renderer:

- replace the generic `case_X_Y` labels with scene-family working names
- identify the reusable helper routines shared across scene families
- capture the current scene-side map in repeatable exports

## Scene Family Map

The jump-table families can now be described more concretely.

| Scene cases | Family VA | Working name | Loader | Primary assets | Notes |
| --- | --- | --- | --- | --- | --- |
| `0`, `7` | `0x00401930` | `render_scene_intro_logo_family` | `0x00401c00` | `v1`, `v2`, `txt1`, `txt2`, `logo`, `logotaus` | Intro/logo composition. Reuses the dual-panel helper, soft blended quads, and a timed fade overlay. |
| `1`, `8` | `0x00402330` | `render_scene_kaar_family` | `0x004025c0` | `kaar128`, `txt1`, `txt2` | Uses a cached strip/ring helper plus centered blended quads. This is the strongest current candidate for the `kaar` orbit/ribbon scene. |
| `2`, `4` | `0x00401fc0` | `render_scene_s_pair_family` | `0x00402130` | `s1`, `s2`, `txt2` | Reuses the same dual-panel helper as the intro family, but with the `s1/s2` textures. |
| `3`, `6` | `0x00402d70` | `render_scene_surf_family` | `0x00403120` | `surf128`, `fla`, `txt1` | Mixes repeated textured quads with the cached strip/ring helper. |
| `5` | `0x004027c0` | `render_scene_fla_particle_family` | `0x00402b40` | `fla`, `txt1`, `logotaus` | Dedicated particle or sprite-field family. Loops over per-element buffers and reseeds values through the local PRNG helper. |
| `9` | `0x00403320` | `render_scene_finale_family` | `0x00403400` | `v1`, `v2`, `txt1`, `txt2` | Finale or outro family. Reuses the intro panel helper and ends in a dedicated time-driven fade helper. |

## Reusable Scene Helpers

The main scene helpers now separate into a few consistent categories.

| Helper VA | Working name | Current interpretation | Key callers |
| --- | --- | --- | --- |
| `0x00403400` | `load_basic_quad_textures` | Dedicated four-texture loader for `v1`, `v2`, `txt1`, `txt2`. Separate from the larger intro loader. | `render_scene_finale_family` |
| `0x00403760` | `draw_dual_texture_panel_pair` | Draws two large textured panels from two texture slots. Implemented as triangle lists rather than `GL_QUADS`. | intro/logo, `s` pair, finale |
| `0x004039d0` | `draw_timed_fade_quad` | Time-driven blended fullscreen-style quad. Uses the x87 helper at `0x004040e0` to wrap or clamp a phase value before coloring. | finale |
| `0x00403b40` | `draw_jittered_overlay_quad` | Small blended overlay quad with alternating pseudo-random position jitter from `0x004040b0`. | all six scene families |
| `0x00403c90` | `draw_soft_blended_quad` | Centered blended quad with slightly inset texture coordinates, likely to avoid edge bleed. | intro/logo, fla particle |
| `0x00403d70` | `draw_centered_textured_quad` | Centered textured quad with two float parameters, likely width and height. | `kaar` family |
| `0x00403e40` | `draw_cached_ring_strip` | One-time build of a 64-step sin/cos table followed by a strip-style draw using `glVertex3fv`. Exact topology still needs confirmation. | `kaar`, `surf` |
| `0x004040b0` | `lcg_rand15` | Local linear-congruential generator returning a `0..0x7fff` style value. | scene families and `draw_jittered_overlay_quad` |
| `0x004040e0` | `fmod_x87_helper` | x87 remainder/wrap helper used to fold a phase value into a smaller range. | intro/logo, finale fade |

## Supporting Findings

### `draw_dual_texture_panel_pair` at `0x00403760`

Confirmed callers push texture-slot pairs:

- intro/logo family: `(1, 2)` -> `v1`, `v2`
- `s` family: `(12, 13)` -> `s1`, `s2`
- finale family: `(1, 2)` -> `v1`, `v2`

The helper begins with `glBegin(4)` and binds both textures in sequence, so this is not a single billboard helper. It is a reusable two-panel primitive shared by the scene families that present paired art assets.

### `draw_jittered_overlay_quad` at `0x00403b40`

The function is called by every scene family and keeps state in:

- `0x005d7f50`
- `0x005d7948`
- `0x005d7f54`

When the low bit of the call counter allows it, the helper reseeds two position values through `0x004040b0`, scales them, and then draws a small blended quad around `z = -2.0`.

This looks more like a shared flicker/jitter overlay than scene-specific geometry.

### `draw_cached_ring_strip` at `0x00403e40`

This helper has two phases:

1. on first use, build a 64-step sin/cos-derived table in the buffers around `0x005d7950` and `0x005d7c50`
2. on later use, draw the cached data using `glVertex3fv`

The exact primitive topology still needs a cleaner decompilation pass, but the function is clearly not just a flat quad helper.

### `load_basic_quad_textures` at `0x00403400`

This loader overlaps with the intro asset group but is not identical to `load_intro_textures`.

It loads only:

- `v1.tga`
- `v2.tga`
- `txt1.tga`
- `txt2.tga`

and is only reached from the finale family.

## Outcome

The scene side is now split into two useful layers:

- six scene families with asset-group identities
- a smaller helper library that can be targeted independently during source reconstruction

This is a better base for eventual C or C++ reconstruction than keeping the renderer as ten anonymous jump-table cases.

## Open Questions

- confirm the exact geometry produced by `draw_dual_texture_panel_pair`
- confirm the exact strip topology of `draw_cached_ring_strip`
- name the timing globals around `0x005d1710` through `0x005d1738` with higher confidence before they are promoted into the replay script
