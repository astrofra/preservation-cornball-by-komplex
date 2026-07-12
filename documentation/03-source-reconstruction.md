# Step 3 - Source Reconstruction

Date: 2026-07-12
Status: in progress

## Goal

Start turning the recovered static-analysis model into buildable source, beginning with the first scene family whose private state layout is stable enough to reconstruct directly.

## Scope of this step

This first coding pass reconstructs:

- the exact local PRNG used by `lcg_rand15`
- the particle-state update path for `render_scene_fla_particle_family`
- a frame-description adapter that exposes the scene's layered `LOGOTAUS`, particle, and `TXT1` overlay passes
- a thin OpenGL renderer that uploads `FLA.TGA`, `LOGOTAUS.TGA`, and `TXT1.TGA`
- a Win32/WGL preview executable for Windows 10/11

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
- [src/cornball_random.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_random.c)
- [src/cornball_fla_scene.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_fla_scene.c)
- [src/cornball_fla_gl.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_fla_gl.c)
- [src/cornball_fla_preview_win32.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_fla_preview_win32.c)
- [tests/fla_scene_smoketest.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/tests/fla_scene_smoketest.c)

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

One important correction from the first pass:

- `draw_jittered_overlay_quad` does not jitter the quad position
- it jitters the sampled `u/v` window on a fixed `4 x 4` quad at `z = -2`
- the helper-owned X/Y offsets therefore belong in the frame description as texture-coordinate bounds, not world-space translation

## Verification target

The smoke test checks:

- the first three PRNG outputs from seed `1`
- the first reconstructed particle state after one scene update
- frame rotation and draw-scale output
- a second update on a particle that stays in the live branch
- the helper-owned `TXT1` jitter overlay state emitted by the fixed-step frame tick
- hidden OpenGL preview startup for two rendered frames

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
- `fla_scene_preview_smoketest` passed

## Preview entrypoint

The new executable target is:

- `fla_scene_preview`

It currently:

- creates a raw Win32 window and WGL context
- discovers the extracted demo assets under `reverse/baseline/cornball` or `original/cornball`
- uploads `FLA.TGA`, `LOGOTAUS.TGA`, and `TXT1.TGA`
- runs the `fla` scene with a fixed `60 Hz` full-frame simulation step
- supports `--hidden --frames <n>` for automated smoke runs

The fixed-step choice is deliberate: the original `fla` routine is frame-based and consumes helper RNG state once per scene frame, so the preview needs an explicit host-side simulation cadence instead of tying particle respawns, overlay tint, and jitter state to an unrestricted modern refresh rate.

## Next step

Use this buildable `fla` module as the template for the next lift:

1. decide whether to keep the particle renderer on legacy immediate-mode OpenGL for preservation accuracy or start a second path that batches it into modern vertex buffers
2. continue with the next scene family, preferably `kaar` or `surf`, now that the shared tube-shell helper is already mapped
3. decide whether the shared helper state should stay scene-local in the reconstruction or be split into a reusable support module before more families are lifted
