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
- a thin OpenGL renderer that uploads `FLA.TGA`, `LOGOTAUS.TGA`, and `TXT1.TGA`
- a third OpenGL renderer that uploads `V1.TGA`, `V2.TGA`, `TXT1.TGA`, `TXT2.TGA`, `LOGO.TGA`, and `LOGOTAUS.TGA`
- a second OpenGL renderer that uploads `KAAR128.TGA`, `TXT1.TGA`, and `TXT2.TGA`
- a Win32/WGL preview executable for Windows 10/11
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
- [tests/fla_scene_smoketest.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/tests/fla_scene_smoketest.c)
- [tests/intro_scene_smoketest.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/tests/intro_scene_smoketest.c)
- [tests/kaar_scene_smoketest.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/tests/kaar_scene_smoketest.c)

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
- `intro` early-stage composition using the original `65 / 1 / 90` perspective envelope, both `V1` and `V2` bipyramid halves, timed `LOGOTAUS` / `LOGO` soft quads, and a `TXT2` to `TXT1` jitter overlay switch
- wrapped texture sampling for the overlay helpers, which is necessary for the `u/v = jitter - 1 .. jitter` window to behave like a scrolling wrapped sample instead of a clamped crop
- `kaar` frame gate `= rand15 * 0.0000305185094759972`
- `kaar` main branch only when the gate is `<= 0.25`
- fogged `KAAR128.TGA` tube-shell pass with additive blending
- tube-shell transform `x = cos(t * 0.2) * 190`, `y = cos(t * 0.3) * 3`
- tube-shell rotation `rx = sin(t * 0.3) * 190`, `ry = t * 2`, `rz = t`
- tube-shell texture phase `= t * 0.1`, with the opposite side sampled at `phase + 3.0`
- centered `TXT1.TGA` quad rotating at `t * 11` with `GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR`

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
- hidden OpenGL preview startup for two rendered frames
- deterministic hidden preview capture for the intro family at `310 x 238`

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
- `fla_scene_preview_smoketest` passed
- `intro_scene_preview_smoketest` passed
- `kaar_scene_preview_smoketest` passed

## Preview entrypoint

The new executable target is:

- `fla_scene_preview`
- `intro_scene_preview`

It currently:

- creates a raw Win32 window and WGL context
- discovers the extracted demo assets under `reverse/baseline/cornball` or `original/cornball`
- uploads `FLA.TGA`, `LOGOTAUS.TGA`, and `TXT1.TGA`
- uploads `V1.TGA`, `V2.TGA`, `TXT1.TGA`, `TXT2.TGA`, `LOGO.TGA`, and `LOGOTAUS.TGA`
- uploads `KAAR128.TGA`, `TXT1.TGA`, and `TXT2.TGA`
- runs the `fla` scene with a fixed `60 Hz` full-frame simulation step
- runs the opening `intro` multi-stage slice with the same fixed `60 Hz` scene-frame cadence
- runs the `kaar` scene with the same fixed `60 Hz` scene-frame cadence
- supports `--hidden --frames <n>` for automated smoke runs
- supports `--width`, `--height`, `--seed`, `--scene-seconds`, `--capture-dir`, and `--capture-every` for scripted reference capture

The fixed-step choice is deliberate: the original `fla` routine is frame-based and consumes helper RNG state once per scene frame, so the preview needs an explicit host-side simulation cadence instead of tying particle respawns, overlay tint, and jitter state to an unrestricted modern refresh rate.

## Next step

Use this buildable `fla` / `intro` base as the template for the next lift:

1. recover the exact caller-side transforms, scales, and timing for the `intro/logo` soft-quad stages so the current buildable slice becomes visually faithful
2. lift `surf` next, since it reuses the same tube-shell helper but changes the overlay reseed mask and texture usage
3. decide whether the shared overlay and tube-shell helpers should now move into reusable support modules before the next family is added
