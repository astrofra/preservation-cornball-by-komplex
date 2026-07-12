# Step 8 - Intro Caller Recovery

Date: 2026-07-12
Status: implemented, binary-derived

## Goal

Replace the heuristic `intro/logo` caller reconstruction with formulas lifted directly from the original Windows binary, so the reconstructed source stays close to the real control flow and can be evolved into a faithful rebuild target.

## Binary Targets

Recovered from `reverse/baseline/cornball/PLANET.EXE`:

- `render_scene_intro_logo_family` at `0x00401930`
- `draw_jittered_overlay_quad` at `0x00403b40`
- `draw_soft_blended_quad` at `0x00403c90`

The disassembly work was done against the existing Cutter/Rizin project:

- `reverse/work/projects/cutter/planet-pass-01.rzdb`

## Exact Caller Model

One `rand15` sample is consumed at the start of every intro frame:

- `r = rand15 / 32768.0`

That same `r` drives all of these caller-side values for the frame:

### Bipyramid

- `glRotatef(90.0, 0, 0, 1)`
- `glRotatef(t * 2.2, 0, 1, 0)`
- `glRotatef(t * 10.0, 1, 0, 0)`
- front texture slot `= V1`
- back texture slot `= V2`
- color `= (0.3 + 0.21r, 0.31 - 0.21r, 0.1 + 0.21r, 1.0)`

### `LOGOTAUS`

- half-width `= 3.2 - 0.02t`
- half-height `= half-width * 0.8`
- draw only when half-width `>= 1.0`
- `glLoadIdentity()`
- color `= (0.5, 0.5, 0.5, 1.0)`
- `glRotatef(t * 9.0, 0, 0, 1)`
- texture slot `= LOGOTAUS`

### `LOGO`

- intensity `= (r + 3.0) / (1.0 + 0.5t)`
- draw only when intensity `> 0.01`
- `glLoadIdentity()`
- `glRotatef(180.0 + 3.0t, 0, 0, 1)`
- texture slot `= LOGO`
- color `= (intensity, intensity, intensity, 0.0)`
- phase `= fmod(max(0.0, 0.3 + 0.06t), 2.0)`
- half-width `= phase * 2.0`
- half-height `= phase * 0.5`

### Overlay

- intensity `= (r + 1.0) / (1.0 + 0.8t)`
- draw only when intensity `> 0.01`
- color `= (intensity, intensity, intensity, intensity)`
- texture slot `= TXT2` while `t <= 5.0`, otherwise `TXT1`
- helper call `draw_jittered_overlay_quad(1)`

## Helper Corrections

The caller lift forced several corrections to the earlier approximation:

- there is no caller-side Y translation for `LOGO`
- the soft quads are not driven by short fade-in/fade-out envelopes
- the intro bipyramid is not plain white; it is tinted from the per-frame random sample
- the `TXT2` to `TXT1` switch is inclusive at exactly `5.0 s`
- `draw_soft_blended_quad` uses `glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA)`, not additive `SRC_ALPHA, ONE`

The soft-quad helper also has a slightly asymmetric top edge in texture space:

- `(+x, -y) -> (0.99, 0.01)`
- `(+x, +y) -> (0.99, 1.00)`
- `(-x, +y) -> (0.01, 0.99)`
- `(-x, -y) -> (0.01, 0.01)`

That asymmetry is now represented explicitly in the reconstruction instead of being flattened to a simple `0.01 .. 0.99` rectangle.

## Mirror Note

No caller-side mirror transform was recovered from the intro scene.

Inference:

- the previously suspicious wordmark reading was not caused by a negative caller scale or mirrored scene rotation
- any remaining left/right ambiguity is more likely coming from the texture asset itself, TGA orientation, or VHS capture quality

## Source Updates

Implemented in:

- [reverse/work/reconstruction/include/cornball/intro_scene.h](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/include/cornball/intro_scene.h)
- [reverse/work/reconstruction/src/cornball_intro_scene.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_intro_scene.c)
- [reverse/work/reconstruction/src/cornball_intro_gl.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_intro_gl.c)
- [reverse/work/reconstruction/tests/intro_scene_smoketest.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/tests/intro_scene_smoketest.c)

## Verification

Validated on 2026-07-12 with:

```powershell
cmake --build reverse/work/reconstruction/build --config Release --target intro_scene_smoketest intro_scene_preview
Push-Location reverse/work/reconstruction/build
ctest -C Release --output-on-failure -R intro_scene
Pop-Location
```

Result:

- `intro_scene_smoketest` passed
- `intro_scene_preview_smoketest` passed

## Outcome

The intro reconstruction is no longer relying on guessed caller-side envelopes.

What remains for visual fidelity is narrower now:

- compare the updated intro against the cropped VHS reference
- resolve any remaining logo-orientation uncertainty from the asset side, not from the recovered caller transforms
- continue the next scene-family lift with the same binary-first workflow
