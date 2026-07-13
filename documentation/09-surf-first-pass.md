# Step 9 - Surf First Pass

Date: 2026-07-12
Status: implemented, binary-derived

## Goal

Lift the next tube-based screen from `PLANET.EXE` into buildable source.

This scene is not the earlier `kaar` orbit view. It is the separate `surf` family:

- camera remains near the tube center
- the shell uses `SURF128`
- the red additive layer comes from repeated `FLA` quads
- the text bitmap layer is the shared `TXT1` jitter overlay

## Binary Target

Recovered from:

- `render_scene_surf_family` at `0x00402d70`

Shared helpers reused by the scene:

- `draw_cached_tube_shell` at `0x00403e40`
- `draw_jittered_overlay_quad` at `0x00403b40`
- `lcg_rand15` at `0x004040b0`

## Exact Caller Model

### One burned PRNG sample per frame

The caller consumes one `rand15` sample immediately after enabling textures:

- `burn = rand15 / 32768.0`

No direct visual use was found for that sample inside the caller itself.

Reconstruction choice:

- preserve the burn exactly to keep PRNG alignment with the original binary

### Fogged inside-tube pass

The scene configures:

- fog color `= (0, 0, 0, 1)`
- fog density `= 0.2`
- fog start `= 1.0`
- fog end `= 65.0`

Then it binds `SURF128` and draws the cached tube shell with additive blending:

- blend `= GL_ONE, GL_ONE`
- translate `x = cos(t * 0.3) * 3`
- translate `y = cos(t * 0.2) * 3`
- translate `z = 0`
- rotate `z_pre = t * 3`
- rotate `x = sin(t * 0.5) * 30`
- rotate `z_post = t * 32`
- shell phase `= -0.3 * t`

Important projection note:

- the scene does not override projection
- the global startup helper keeps the `gluPerspective(65, aspect, 1, 90)` camera active
- that perspective setup is necessary for the “inside the tube” feel

## Additional Texture Layers

### Repeated `FLA` stack

After the tube pass, the caller binds `FLA` and emits a stack of repeated additive quads:

- repeat count `= 32`
- blend `= GL_ONE, GL_ONE`
- base translation `z = t * 8`
- per-layer translation `z -= 10`
- quad extent `= 2 x 2`
- color `= (1, 1, 1, 0)`
- texture window `= 0 .. 1`

One compatibility issue appears in the original binary:

- it performs `glTranslatef(0, 0, -10)` inside an active `GL_QUADS` block

That is undefined OpenGL behavior on modern drivers.

Reconstruction choice:

- preserve the resulting stacked Z positions explicitly
- avoid reproducing the invalid matrix call inside `glBegin` / `glEnd`

### Rotating `SURF128` foreground quad

The next layer resets the modelview matrix and draws a small foreground quad:

- texture `= SURF128`
- color `= (1, 1, 1, 0.8)`
- blend `= GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR`
- quad depth `= -1`
- quad extent `= 1 x 1`
- texture window `= 0 .. 2`
- rotation `= t * 11`

### `TXT1` text overlay

The final scene-owned color before the helper overlay is:

- color `= (0.4, 0.4, 0.2, 1.0)`

Then the caller binds `TXT1` and invokes:

- `draw_jittered_overlay_quad(0)`

Because the reseed mask is `0`, the helper reseeds on every call:

- two fresh random samples are consumed every frame for jitter X/Y

## Reference Fit

This family matches the earlier reference-video sampling result well.

The strongest anchor is:

- [frame_002548.png](../reverse/work/reference-video/frames-cropped/frame_002548.png)

Why it matches:

- visible `TXT1` text clutter
- a centered red flare consistent with `FLA`
- darker tubular / cellular material consistent with `SURF128`

This is the same “flare-over-text composite” previously flagged as the most plausible `surf` still in [05-reference-frame-sampling.md](/C:/works/projects/preservation-cornball-by-komplex/documentation/05-reference-frame-sampling.md).

## Source Updates

Implemented in:

- [reverse/work/reconstruction/include/cornball/surf_scene.h](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/include/cornball/surf_scene.h)
- [reverse/work/reconstruction/include/cornball/surf_gl.h](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/include/cornball/surf_gl.h)
- [reverse/work/reconstruction/src/cornball_surf_scene.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_surf_scene.c)
- [reverse/work/reconstruction/src/cornball_surf_gl.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_surf_gl.c)
- [reverse/work/reconstruction/src/cornball_surf_preview_win32.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_surf_preview_win32.c)
- [reverse/work/reconstruction/tests/surf_scene_smoketest.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/tests/surf_scene_smoketest.c)

## Verification

Validated on 2026-07-12 with:

```powershell
cmake --build reverse/work/reconstruction/build --config Release --target surf_scene_smoketest surf_scene_preview
Push-Location reverse/work/reconstruction/build
ctest -C Release --output-on-failure -R surf_scene
Pop-Location
```

Result:

- `surf_scene_smoketest` passed
- `surf_scene_preview_smoketest` passed

Quick render sanity check:

- [surf_frame_000000.png](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/captures/surf-smoke/surf_frame_000000.png)

That single preview frame already shows the expected three-way composition:

- dark `SURF128` tube material
- centered red `FLA` flare stack
- overlaid `TXT1` typography

## Outcome

The next logical screen is now represented as code instead of only as a tube-helper hypothesis.

What is accurate in this pass:

- the caller-side timing constants
- the inside-camera tube transform
- the repeated `FLA` stack
- the rotating `SURF128` foreground layer
- the per-frame `TXT1` overlay reseed behavior

What still needs visual tuning:

- comparison against the VHS reference anchor
- possible texture orientation mismatches
- whether the earlier `kaar` approximation should now also move to the same global perspective camera
