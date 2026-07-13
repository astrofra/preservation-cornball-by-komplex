# Step 3 - Source Reconstruction

Date: 2026-07-12
Status: in progress

## Goal

Start turning the recovered static-analysis model into buildable source, beginning with the first scene family whose private state layout is stable enough to reconstruct directly.

## Scope of this step

This first coding pass reconstructs:

- the exact local PRNG used by `lcg_rand15`
- the particle-state update path for `render_scene_fla_particle_family`
- the early multi-stage slice of `render_scene_intro_logo_family`
- a frame-description adapter that exposes the scene's layered `LOGOTAUS`, particle, and `TXT1` overlay passes
- the first source lift of `render_scene_kaar_family`, including its fogged tube-shell pass and centered `TXT1` quad
- the first source lift of `render_scene_surf_family`, including its inside-camera tube shell, additive `FLA` quad stack, rotating `SURF128` foreground layer, and `TXT1` overlay
- a thin OpenGL renderer that uploads `FLA.TGA`, `LOGOTAUS.TGA`, and `TXT1.TGA`
- a third OpenGL renderer that uploads `V1.TGA`, `V2.TGA`, `TXT1.TGA`, `TXT2.TGA`, `LOGO.TGA`, and `LOGOTAUS.TGA`
- a second OpenGL renderer that uploads `KAAR128.TGA`, `TXT1.TGA`, and `TXT2.TGA`
- a fourth OpenGL renderer that uploads `SURF128.TGA`, `FLA.TGA`, and `TXT1.TGA`
- a single Win32/WGL replay executable that chains the reconstructed families in original scene order
- a Win32/x86 music path that reuses the original `MIDAS06.DLL` and `AAB.XM`
- a reusable back-buffer frame-dump path for hidden preview captures

It does not yet reconstruct:

- the scene dispatcher
- the rest of the scene families
- the original MIDAS playback shell

## Added workspace

Created under `reverse/work/reconstruction/`:

- [CMakeLists.txt](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/CMakeLists.txt)
- [README.md](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/README.md)
- [include/cornball/random.h](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/include/cornball/random.h)
- [include/cornball/fla_scene.h](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/include/cornball/fla_scene.h)
- [include/cornball/fla_gl.h](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/include/cornball/fla_gl.h)
- [include/cornball/intro_scene.h](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/include/cornball/intro_scene.h)
- [include/cornball/intro_gl.h](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/include/cornball/intro_gl.h)
- [include/cornball/capture.h](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/include/cornball/capture.h)
- [src/cornball_random.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_random.c)
- [src/cornball_fla_scene.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_fla_scene.c)
- [src/cornball_fla_gl.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_fla_gl.c)
- [src/cornball_fla_preview_win32.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_fla_preview_win32.c)
- [src/cornball_intro_scene.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_intro_scene.c)
- [src/cornball_intro_gl.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_intro_gl.c)
- [src/cornball_intro_preview_win32.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_intro_preview_win32.c)
- [src/cornball_capture.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_capture.c)
- [include/cornball/kaar_scene.h](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/include/cornball/kaar_scene.h)
- [include/cornball/kaar_gl.h](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/include/cornball/kaar_gl.h)
- [src/cornball_kaar_scene.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_kaar_scene.c)
- [src/cornball_kaar_gl.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_kaar_gl.c)
- [src/cornball_kaar_preview_win32.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_kaar_preview_win32.c)
- [include/cornball/surf_scene.h](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/include/cornball/surf_scene.h)
- [include/cornball/surf_gl.h](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/include/cornball/surf_gl.h)
- [src/cornball_surf_scene.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_surf_scene.c)
- [src/cornball_surf_gl.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_surf_gl.c)
- [src/cornball_surf_preview_win32.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_surf_preview_win32.c)
- [src/cornball_demo_replay_win32.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_demo_replay_win32.c)
- [tests/fla_scene_smoketest.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/tests/fla_scene_smoketest.c)
- [tests/intro_scene_smoketest.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/tests/intro_scene_smoketest.c)
- [tests/kaar_scene_smoketest.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/tests/kaar_scene_smoketest.c)
- [tests/surf_scene_smoketest.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/tests/surf_scene_smoketest.c)

## Reconstruction choices

### Language

This pass uses C99 rather than C++:

- the recovered binary still looks more C-like than C++-like
- the current scene code is mostly flat state, arrays, globals, and helper functions
- keeping the first lift in C makes it easier to stay close to the original control flow

### Scene boundary

The `fla` family was chosen first because its private state is now clearer than the other scene families:

- fixed particle count: `500`
- three parallel `16`-byte records per particle
- direct respawn/update split
- stable numeric constants for fade, velocity, and acceleration

The follow-up `kaar` lift stays intentionally close to the scene/helper split from the binary:

- no private particle or mesh state block was found for the family itself
- the family consumes the shared tube-shell helper and the shared jitter overlay helper
- the scene-owned contract is therefore mostly frame gating, transform setup, and texture usage

### Source shape

The code intentionally separates:

- scene-state mutation
- deterministic PRNG access
- frame-description output

The OpenGL pass keeps that separation intact:

- the scene module still owns the recovered update logic
- the GL renderer only consumes frame data and asset paths
- the Win32 preview only owns timing, WGL setup, and the message loop

## Current behavior reproduced

The current C reconstruction models these points directly from the binary:

- `state = state * 214013 + 2531011`
- `rand15 = (state >> 16) & 0x7fff`
- `rand01 = rand15 / 32768.0`
- brightness decay threshold at `0.01`
- brightness decay factor at `0.9`
- respawned `vel_x = (rand01 - 0.5) * 0.7`
- respawned `vel_y = 0.7 + rand01 * 0.3`
- respawned `accel_y = -0.01 - rand01 * 0.15`
- frame rotation `= 180 - sin(scene_time * 0.72) * 19`
- draw grayscale `= brightness * 3.0`
- textured particle quads using the original mirrored texcoord layout
- `LOGOTAUS.TGA` soft-blended quad with inset `0.01 .. 0.99` texture coordinates
- `GL_ONE, GL_ONE_MINUS_SRC_COLOR` particle blending for the `fla` texture pass
- `TXT1.TGA` jitter overlay using a fixed centered quad and a pseudo-randomized texture window
- `intro` early-stage composition using the original `65 / 1 / 90` perspective envelope, both `V1` and `V2` bipyramid halves, the recovered caller-side `LOGOTAUS` / `LOGO` transforms and timing, and the original `TXT2` to `TXT1` overlay branch
- intro bipyramid tint `= (0.3 + 0.21r, 0.31 - 0.21r, 0.1 + 0.21r, 1.0)` with `r = rand15 / 32768.0`
- intro `LOGOTAUS` half-size `= 3.2 - 0.02 * t`, rotation `= 9 * t`, draw only while half-size `>= 1.0`
- intro `LOGO` intensity `= (r + 3.0) / (1.0 + 0.5 * t)`, rotation `= 180 + 3 * t`, size phase `= fmod(max(0, 0.3 + 0.06 * t), 2.0)`
- intro overlay intensity `= (r + 1.0) / (1.0 + 0.8 * t)`, with `TXT2` still selected at exactly `t = 5.0 s`
- wrapped texture sampling for the overlay helpers, which is necessary for the helper-owned `u/v` window to behave like a scrolling wrapped sample instead of a clamped crop
- `kaar` frame gate `= rand15 * 0.0000305185094759972`
- `kaar` main branch only when the gate is `<= 0.25`
- fogged `KAAR128.TGA` tube-shell pass with additive blending
- `kaar` uses the same global `65 / 1 / 90` perspective camera as the original demo bootstrap, not the earlier orthographic approximation
- tube-shell transform `x = cos(t * 0.2) * 3`, `y = cos(t * 0.3) * 3`
- tube-shell rotation `rx = sin(t * 0.3) * 190`, `ry = t * 2`, `rz = t`
- tube-shell texture phase `= t * 0.1`, with the opposite side sampled at `phase + 3.0`
- centered `TXT1.TGA` quad rotating at `t * 11` with `GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR`
- `surf` fogged `SURF128.TGA` tube-shell pass with the global `65 / 1 / 90` perspective camera still active
- `surf` tube-shell transform `x = cos(t * 0.3) * 3`, `y = cos(t * 0.2) * 3`, `rz_pre = t * 3`, `rx = sin(t * 0.5) * 30`, `rz_post = t * 32`, `phase = -0.3 * t`
- `surf` additive `FLA.TGA` stack of `32` quads, with base `z = t * 8` and per-layer step `-10`
- `surf` rotating `SURF128.TGA` foreground quad at `z = -1`, `u/v = 0 .. 2`, rotation `= t * 11`, blend `GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR`
- `surf` `TXT1.TGA` jitter overlay with reseed mask `0`, so it consumes two fresh random samples every frame
- replay ordering `= intro(0) -> kaar(1) -> surf(3) -> fla(5) -> surf(6) -> intro(7) -> kaar(8)`, matching the original dispatch order for the families reconstructed so far
- replay segment lengths are driven by the original music-position slot widths and exposed through a synthetic `--position-seconds` rate until the exact audio-driven timing is rebuilt

One important correction from the first pass:

- `draw_jittered_overlay_quad` does not jitter the quad position
- it jitters the sampled `u/v` window on a fixed `4 x 4` quad at `z = -2`
- the helper-owned X/Y offsets therefore belong in the frame description as texture-coordinate bounds, not world-space translation

One remaining ambiguity is now documented rather than hidden:

- the `kaar` family reads a slightly different random-to-float scale than the `fla` particle code
- the exact bit pattern lifted from `0x0040d1b0` is preserved in the current source instead of forcing it to `1 / 32768`
- that path should be revisited if later disassembly or visual comparison shows the constant was misidentified by the mixed `.rdata` layout

## Verification target

The smoke test checks:

- the first three PRNG outputs from seed `1`
- the first reconstructed particle state after one scene update
- frame rotation and draw-scale output
- a second update on a particle that stays in the live branch
- the helper-owned `TXT1` jitter overlay state emitted by the fixed-step frame tick
- the first two `kaar` frame gates from seed `1`
- both `kaar` render branches: fogged tube-shell path and jitter-overlay path
- hidden end-to-end replay startup across the full chained scene set

## Current verification status

Validated on 2026-07-12 with:

```powershell
cmake -S reverse/work/reconstruction -B reverse/work/reconstruction/build -G "Visual Studio 17 2022" -A x64
cmake --build reverse/work/reconstruction/build --config Release
Push-Location reverse/work/reconstruction/build
ctest -C Release --output-on-failure
Pop-Location
```

Result:

- configuration succeeded with Visual Studio 2022 / MSVC 19.41
- the static library built successfully
- `fla_scene_smoketest` passed
- `intro_scene_smoketest` passed
- `kaar_scene_smoketest` passed
- `surf_scene_smoketest` passed
- `cornball_demo_replay_smoketest` passed

## Replay entrypoint

The new executable target is:

- `cornball_demo_replay`

It currently:

- creates a raw Win32 window and WGL context
- discovers the extracted demo assets under `reverse/baseline/cornball` or `original/cornball`
- treats the shipped `32bpp` TGAs with descriptor `0x00` as color-only data and derives alpha from the strongest RGB channel instead of trusting the unused fourth byte
- uploads `FLA.TGA`, `LOGOTAUS.TGA`, and `TXT1.TGA`
- uploads `V1.TGA`, `V2.TGA`, `TXT1.TGA`, `TXT2.TGA`, `LOGO.TGA`, and `LOGOTAUS.TGA`
- uploads `KAAR128.TGA`, `TXT1.TGA`, and `TXT2.TGA`
- uploads `SURF128.TGA`, `FLA.TGA`, and `TXT1.TGA`
- runs the chained reconstructed sequence with a default fixed `60 Hz` scene-frame cadence, now overrideable with `--fixed-step-hz` during synthetic capture analysis
- resets scene-local time at each original scene-slot transition while keeping the global PRNG and family-owned state alive
- in Win32/x86 builds, can start the original `AAB.XM` replay through the bundled `MIDAS06.DLL`
- when that music path is active, follows the true tracker position for scene dispatch and still leaves unreconstructed scene slot `9` as black
- supports `--hidden --frames <n>` for automated smoke runs
- supports `--width`, `--height`, `--seed`, `--demo-seconds`, `--position-seconds`, `--fixed-step-hz`, `--force-kaar-main-branch`, `--isolate-kaar-tube`, `--capture-dir`, `--capture-every`, `--music`, and `--no-music` for scripted reference capture

Current comparison-oriented default:

- the replay now opens at `640x400` when no explicit size is provided
- this should be read as a VHS-comparison preset for the present preservation pass
- it does not replace the earlier static finding that the original shell still created a `640x480` Win32 window

Current music boundary:

- the shipped `MIDAS06.DLL` is 32-bit, so this music path is only available in a `Win32` replay build
- the default `x64` replay build still falls back to the synthetic silent timeline

The fixed-step choice is deliberate: the original `fla` routine is frame-based and consumes helper RNG state once per scene frame, so the replay needs an explicit host-side simulation cadence instead of tying particle respawns, overlay tint, and jitter state to an unrestricted modern refresh rate.

## Next step

Use this buildable `fla` / `intro` base as the template for the next lift:

1. compare the replayed `surf` and `kaar` captures against the reference-video anchors and tune any remaining projection or layer-order mismatch
2. decide whether the shared overlay and tube-shell helpers should now move into reusable support modules before the next family is added
3. lift the missing `s` pair and finale families so the dispatcher can stop compressing the unreconstructed gaps
