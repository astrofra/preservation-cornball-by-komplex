# Cutter Pass 07

Date: 2026-07-12
Tooling: Cutter package `2.5.0`, using the bundled `rizin.exe`
Binary: `reverse/baseline/cornball/PLANET.EXE`

Related artifacts:

- [planet-scene-state-blocks.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-scene-state-blocks.csv)
- [planet-function-map.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-function-map.csv)
- [planet_first_pass.rz](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/tools/planet_first_pass.rz)

## Goal

Resolve the first real scene-owned state block by tracing `render_scene_fla_particle_family`.

## Loader-side ownership

`load_fla_texture_group` zeroes three contiguous `0x1f40`-byte buffers before uploading `fla.tga`, `txt1.tga`, and `logotaus.tga`.

Those buffers are:

- `0x005d1778` -> `fla_particle_velocity_block`
- `0x005d36b8` -> `fla_particle_accel_block`
- `0x005d55f8` -> `fla_particle_energy_position_block`

Each block is `500 * 16` bytes, which is consistent with `500` particles stored as `vec4`-like records.

## Particle update loop

`render_scene_fla_particle_family` walks these blocks in lockstep with `esi += 0x10` until `esi == 0x1f40`.

Current working record layout:

```c
struct FlaParticleState {
    float brightness;
    float pos_x;
    float pos_y;
    float pos_z;
};

struct FlaParticleVelocity {
    float unused0;
    float vel_x;
    float vel_y;
    float vel_z;
};

struct FlaParticleAccel {
    float unused0;
    float accel_x;
    float accel_y;
    float accel_z;
};
```

The update path does two things:

1. If `brightness > 0.01`, it:
   - multiplies `brightness` by `0.9`
   - integrates position from velocity
   - integrates velocity from acceleration
2. Otherwise, it respawns the particle with fresh pseudo-random values.

## Respawn ranges

The respawn path currently resolves to:

- `brightness = rand01`
- `pos_x = pos_y = pos_z = 0`
- `vel_x` in roughly `[-0.35, +0.35]`
- `vel_y` in roughly `[0.7, 1.0]`
- `vel_z = 0`
- `accel_x = 0`
- `accel_y` in roughly `[-0.16, -0.01]`
- `accel_z = 0`

This is a good match for a fountain or spark-field style particle system with per-particle fade and downward acceleration.

## Draw path

After the update loop, the renderer:

- binds texture slot `11` (`fla.tga`)
- draws `GL_QUADS`
- uses `brightness * 3.0` as a grayscale color scale
- translates each particle by `pos_x`, `pos_y`, and `0`
- draws a small `2 x 2` local quad

`pos_z` is integrated in the state block but is not consumed by the current draw loop.

## Outcome

Pass 07 provides the first scene-family data layout that is concrete enough to lift into hand-written C or C++:

- fixed particle count: `500`
- stable per-particle state arrays
- explicit update / respawn split
- initial numeric ranges for velocity, acceleration, and fade

That is enough to begin a source-level reconstruction of the `fla` family without waiting for full-program decompilation.
