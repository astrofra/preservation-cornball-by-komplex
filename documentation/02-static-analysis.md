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
13. Ran the fourth static pass on timing globals and MIDAS status layout:
   - [reverse/work/notes/cutter-pass-04.md](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/notes/cutter-pass-04.md)
14. Ran the fifth static pass on reusable geometry helpers:
   - [reverse/work/notes/cutter-pass-05.md](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/notes/cutter-pass-05.md)
15. Ran the sixth static pass on helper-owned state and indirect MIDAS-field usage:
   - [reverse/work/notes/cutter-pass-06.md](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/notes/cutter-pass-06.md)
16. Ran the seventh static pass on the `fla` particle-state blocks:
   - [reverse/work/notes/cutter-pass-07.md](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/notes/cutter-pass-07.md)

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
- [planet-geometry-helpers.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-geometry-helpers.csv)
- [planet-scene-state-blocks.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-scene-state-blocks.csv)
- [planet-timing-globals.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-timing-globals.csv)
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

- `draw_dual_texture_bipyramid` at `0x00403760`
- `draw_soft_blended_quad` at `0x00403c90`
- `draw_centered_textured_quad` at `0x00403d70`
- `draw_cached_tube_shell` at `0x00403e40`
- `draw_jittered_overlay_quad` at `0x00403b40`
- `draw_timed_fade_quad` at `0x004039d0`
- `lcg_rand15` at `0x004040b0`
- `fmod_x87_helper` at `0x004040e0`

These mappings are exported in [planet-scene-helper-map.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-scene-helper-map.csv).

### Scene-side interpretation

Current working interpretation:

- the intro/logo and `s` scenes share the same two-texture bipyramid primitive, but feed it different texture pairs
- the `kaar` and `surf` scenes both depend on a cached tube-shell helper, making them stronger candidates for geometry-heavy ribbon, ring, or tube effects
- the `fla` scene owns the most obvious per-element state loop and is currently the best particle-field candidate
- the finale scene reuses the shared quad art and ends with a dedicated time-based fade helper

### New scene-side anchors

Pass 03 also provided several stable helpers and state blocks for later source reconstruction:

- startup polar table builder at `0x00401000`
- basic quad texture loader at `0x00403400`
- tube-shell vertex caches around `0x005d7950` and `0x005d7c50`
- overlay jitter state around `0x005d7948`, `0x005d7f50`, and `0x005d7f54`

## Cutter Pass 04 Findings

Pass 04 resolved the timing globals around `0x005d1710` through `0x005d1764` and verified the older MIDAS status layout used by the demo.

### Verified MIDAS status structure

`PLANET.EXE` imports `_MIDASgetPlayStatus@4` from `midas06.dll`.

A local mirror of Housemarque Audio System `0.7 beta 1` under `reverse/work/vendor/` matches that exact single-argument signature and documents:

- `MIDASplayStatus.position`
- `MIDASplayStatus.pattern`
- `MIDASplayStatus.row`
- `MIDASplayStatus.syncInfo`

This lines up cleanly with the binary accesses:

- `0x005d1750` -> `g_midas_play_status_position`
- `0x005d1754` -> `g_midas_play_status_pattern`
- `0x005d1758` -> `g_midas_play_status_row`
- `0x005d175c` -> `g_midas_play_status_sync_info`

### Timing model

The main loop and dispatcher now resolve to a hybrid timing model:

- `MIDASplayStatus.position` selects the active scene family through the threshold table at `0x0040e068`
- `GetTickCount()` snapshots drive frame timing and scene-local elapsed time
- the scale constant at `0x0040d078` is exactly `0.001`

Resolved globals:

- `0x005d1730` -> `g_demo_start_tick_ms`
- `0x005d1734` -> `g_scene_start_tick_ms`
- `0x005d1728` -> `g_scene_elapsed_seconds`
- `0x005d1760` -> `g_current_scene_index`
- `0x005d1764` -> `g_previous_scene_index`
- `0x005d1718` -> `g_cached_music_position`
- `0x005d1738` -> `g_cached_music_row`
- `0x005d1710` -> `g_demo_elapsed_seconds`

The most important derived value is:

- `g_scene_elapsed_seconds = (current_tick_ms - g_scene_start_tick_ms) * 0.001`

That is the scene-facing timer reused across the renderer. The sibling global `g_demo_elapsed_seconds` is the same conversion measured from main-loop entry instead of scene entry. By contrast, `g_cached_music_position` and `g_cached_music_row` currently look like dispatcher-local caches rather than active scene inputs.

### Reconstruction impact

This reduces a key uncertainty for later source recovery:

- scene changes are music-position driven
- most intra-scene animation is wall-clock driven

In practice, that means a future C or C++ rewrite can model the stable interface as:

- `music_position`
- `scene_index`
- `scene_time_seconds`

without first rebuilding a tracker-row-exact runtime.

The detailed address map for this pass is exported in [planet-timing-globals.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-timing-globals.csv).

## Cutter Pass 05 Findings

Pass 05 resolved the two most reusable geometry helpers with enough detail to stop treating them as vague visual guesses.

### Corrected helper identities

Two helper names are now materially more precise:

- `0x00403760` -> `draw_dual_texture_bipyramid`
- `0x00403e40` -> `draw_cached_tube_shell`

The previous “panel pair” and “ring strip” labels are now obsolete.

### `draw_dual_texture_bipyramid`

This helper:

- begins `glBegin(GL_TRIANGLES)`
- binds one texture for the `+z` half and one for the `-z` half
- emits `8` triangles, `24` vertices total

Resolved geometry:

- shared base square at `z = 0`
- base corners at `(+/-60, +/-60, 0)`
- front apex at `(0, 0, 60)`
- back apex at `(0, 0, -60)`

So the helper draws a textured square bipyramid or octahedron-like sprite, not two flat panels.

### `draw_cached_tube_shell`

This helper:

- lazily builds two cached rings of `64` vertices each
- uses `radius = 6`
- places the rings at `z = -70` and `z = +70`
- draws `64` `GL_QUADS` between matching vertices

Resolved cache formula:

- `angle = i * 0.098125`
- `x = cos(angle) * 6`
- `y = sin(angle) * 6`
- `z = (ring - 0.5) * 140`

Resolved draw order:

- `ring0[i]`
- `ring0[next]`
- `ring1[next]`
- `ring1[i]`

The helper also applies a scrolled texture window derived from its `double phase` argument, using a `3.0` span across the tube length and `1/64` steps around the circumference.

### Geometry-state anchors

Pass 05 also sharpens the cached geometry labels:

- `0x005d7950` -> `tube_ring_vertices_neg_z`
- `0x005d7c50` -> `tube_ring_vertices_pos_z`
- `0x005d7f58` -> `g_tube_shell_cache_valid`

The geometry-specific export for this pass is [planet-geometry-helpers.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-geometry-helpers.csv).

## Cutter Pass 06 Findings

Pass 06 resolved the ownership of the remaining scene-side helper globals and checked whether the unused MIDAS fields leak back into rendering logic.

### MIDAS field usage

The current binary still shows no scene-side consumer for:

- `MIDASplayStatus.pattern`
- `MIDASplayStatus.syncInfo`

`MIDASplayStatus.row` is copied into `g_cached_music_row`, but no downstream renderer read has been found so far.

This keeps the effective renderer contract narrow:

- music `position` selects the scene family
- wall-clock deltas animate the active scene

### Helper-owned state blocks

The `0x005d79xx` / `0x005d7fxx` ranges are now resolved as shared helper state, not private `kaar` / `surf` family state.

`draw_jittered_overlay_quad` owns:

- `0x005d7948` -> `g_overlay_jitter_y`
- `0x005d7f50` -> `g_overlay_jitter_x`
- `0x005d7f54` -> `g_overlay_call_counter`

Confirmed behavior:

- the helper reseeds X/Y when `(g_overlay_call_counter & reseed_mask) == 0`
- all scene families call it with `reseed_mask = 1` except `surf`
- `surf` passes `0`, so it reseeds on every call

`draw_cached_tube_shell` owns:

- `0x005d7950` -> `tube_ring_vertices_neg_z`
- `0x005d7c50` -> `tube_ring_vertices_pos_z`
- `0x005d7f58` -> `g_tube_shell_cache_valid`

The shared-state export for this pass is [planet-scene-state-blocks.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-scene-state-blocks.csv).

## Cutter Pass 07 Findings

Pass 07 moved from shared helpers to the first real scene-owned state layout: the `fla` family.

### `fla` particle blocks

`load_fla_texture_group` zeroes three `0x1f40`-byte buffers before uploading textures:

- `0x005d1778` -> `fla_particle_velocity_block`
- `0x005d36b8` -> `fla_particle_accel_block`
- `0x005d55f8` -> `fla_particle_energy_position_block`

Each block is `500 * 16` bytes, which is consistent with a fixed `500`-particle effect using vec4-like records.

### Working reconstruction

The current working layout is:

- `brightness`, `pos_x`, `pos_y`, `pos_z`
- `unused0`, `vel_x`, `vel_y`, `vel_z`
- `unused0`, `accel_x`, `accel_y`, `accel_z`

The renderer updates each particle as:

- fade: `brightness *= 0.9`
- integrate position from velocity
- integrate velocity from acceleration
- respawn when `brightness <= 0.01`

Respawn ranges now resolved from constants:

- `vel_x` about `[-0.35, +0.35]`
- `vel_y` about `[0.7, 1.0]`
- `accel_y` about `[-0.16, -0.01]`

This is strong evidence for a fountain or spark-field particle system rather than a generic sprite cloud.

The draw path binds `fla.tga`, uses `brightness * 3.0` as a grayscale color scale, translates by `pos_x` / `pos_y`, and emits a small local quad for each live element.

## Current Deliverable Quality

The workspace is now ready for continued interactive analysis with a documented seven-pass Cutter workflow.

The current map is still not final, but it is already useful enough to:

- avoid restarting from raw addresses
- separate scene-family logic from reusable helper routines
- separate music-position scene switching from wall-clock scene timing
- describe the shared geometry helpers as concrete primitives
- separate helper-owned state from the first confirmed scene-owned particle blocks
- begin lifting a full scene family from stable data layouts instead of only from call graphs
- keep naming decisions consistent across tools

## Next Step

Continue the static pass interactively in Cutter:

1. lift `render_scene_fla_particle_family` into higher-level C-style pseudocode with explicit particle structs and loop boundaries
2. confirm whether the unused fourth float in the `fla` velocity and acceleration records is dead padding or a dormant field
3. start reconstructing one complete scene family around the now-stable helper library and particle data layout
4. continue looking for additional scene-owned blocks in the remaining families
