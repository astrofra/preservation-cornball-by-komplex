# Step 3 - Source Reconstruction

Date: 2026-07-12
Status: in progress

## Goal

Start turning the recovered static-analysis model into buildable source, beginning with the first scene family whose private state layout is stable enough to reconstruct directly.

## Scope of this step

This first coding pass reconstructs:

- the exact local PRNG used by `lcg_rand15`
- the particle-state update path for `render_scene_fla_particle_family`
- a frame-description adapter that exposes the scene's rotation and per-particle draw quads without depending on legacy OpenGL immediate mode yet

It does not yet reconstruct:

- the original Win32 startup shell
- texture upload helpers
- the scene dispatcher
- the rest of the scene families

## Added workspace

Created under `reverse/work/reconstruction/`:

- [CMakeLists.txt](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/CMakeLists.txt)
- [README.md](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/README.md)
- [include/cornball/random.h](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/include/cornball/random.h)
- [include/cornball/fla_scene.h](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/include/cornball/fla_scene.h)
- [src/cornball_random.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_random.c)
- [src/cornball_fla_scene.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_fla_scene.c)
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

That keeps the recovered logic testable before reintroducing real OpenGL calls.

## Current behavior reproduced

The current C reconstruction models these points directly from the binary:

- `state = state * 214013 + 2531011`
- `rand15 = (state >> 16) & 0x7fff`
- brightness decay threshold at `0.01`
- brightness decay factor at `0.9`
- respawned `vel_x = (rand01 - 0.5) * 0.7`
- respawned `vel_y = 0.7 + rand01 * 0.3`
- respawned `accel_y = -0.01 - rand01 * 0.15`
- frame rotation `= 180 + sin(scene_time * 0.72) * 19`
- draw grayscale `= brightness * 3.0`

## Verification target

The smoke test checks:

- the first three PRNG outputs from seed `1`
- the first reconstructed particle state after one scene update
- frame rotation and draw-scale output
- a second update on a particle that stays in the live branch

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

## Next step

Use this buildable `fla` module as the template for the next lift:

1. attach a thin OpenGL wrapper so this scene can render again on a modern host
2. replace the current frame-description adapter with real draw calls once the wrapper exists
3. continue with the next scene family, preferably `kaar` or `surf`, now that the shared tube-shell helper is already mapped
