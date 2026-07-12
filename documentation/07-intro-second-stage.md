# Step 7 - Intro Second-Stage Lift

Date: 2026-07-12
Status: implemented, visually incomplete

## Goal

Extend the earlier `intro/logo` first-screen subset so the next visible opening stage is also buildable:

- restore the full two-texture `V1` / `V2` bipyramid usage
- emit the timed `LOGOTAUS` and `LOGO` soft-quad layers
- keep using deterministic capture so the result can be compared against the cropped VHS reference

## Code Changes

Updated the intro reconstruction under `reverse/work/reconstruction/`:

- [include/cornball/intro_scene.h](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/include/cornball/intro_scene.h)
- [src/cornball_intro_scene.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_intro_scene.c)
- [src/cornball_intro_gl.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_intro_gl.c)
- [tests/intro_scene_smoketest.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/tests/intro_scene_smoketest.c)
- [README.md](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/README.md)

Implemented behavior:

- the bipyramid now uses both texture slots confirmed by the scene-family lift: `V1` on the front half and `V2` on the back half
- `LOGOTAUS` is emitted as a timed soft quad with inset `0.01 .. 0.99` texture coordinates
- `LOGO` is emitted as a second timed soft quad, with a small positive Y translation to move it toward the region where the cropped reference begins to show the faint center wordmark
- the GL path now composites these soft quads with `GL_SRC_ALPHA, GL_ONE` before the jitter overlay pass
- the overlay quad remains deterministic and still switches from `TXT2` to `TXT1` at `5.0 s`

One small pragmatic adjustment was made for comparison quality:

- the intro overlay tint was reduced from a hard white to `0.85` so the soft-quad stage and the `V` underlayer do not disappear completely under the `TXT2` composite

This is a reconstruction-driven approximation, not yet a verified lift of the exact original caller-side constants.

## Verification

Build and test:

```powershell
cmake --build reverse/work/reconstruction/build --config Release
Push-Location reverse/work/reconstruction/build
ctest -C Release --output-on-failure
Pop-Location
```

The intro smoke test was extended to check:

- `V1` / `V2` bipyramid slot selection
- absence of the soft quads at `t = 0.0 s`
- presence of both soft quads at `t = 2.5 s`
- the logo-stage half extent, alpha, and Y translation
- switch to `TXT1` and soft-quad removal at `t = 5.0 s`

Note:

- during one all-target rebuild, `fla_scene_preview.exe` was temporarily locked by the host environment and failed to relink
- the intro targets and the full `ctest` pass still completed successfully after the intro-target rebuild, so the reconstruction state for this step is verified

## Comparison Artifacts

Artifacts created for this step:

- [intro_frame_2p4s.png](assets/intro-second-stage/intro_frame_2p4s.png)
- [intro_second_stage_contact_sheet.png](assets/intro-second-stage/intro_second_stage_contact_sheet.png)
- [reference_vs_intro_2p4s.png](assets/intro-second-stage/reference_vs_intro_2p4s.png)

Raw captures:

- `reverse/work/reconstruction/captures/intro-second-stage/`
- `reverse/work/reconstruction/captures/intro-second-stage-frame60/`

Reference target used for the direct still comparison:

- cropped reference frame `000060`, which corresponds to provisional scene-visible `t = 2.40 s`

Matching capture command:

```powershell
New-Item -ItemType Directory -Force -Path reverse/work/reconstruction/captures/intro-second-stage-frame60 | Out-Null
Push-Location reverse/work/reconstruction/build
.\Release\intro_scene_preview.exe `
  --hidden `
  --frames 2 `
  --capture-every 1 `
  --width 310 `
  --height 238 `
  --scene-seconds 2.4 `
  --capture-dir ..\captures\intro-second-stage-frame60
Pop-Location
```

## Result

This step succeeded at the **structural** level:

- the next intro stage is now represented in code instead of being left as a TODO
- the renderer can dump deterministic captures that include the `LOGOTAUS` / `LOGO` composition stage
- the intro-family frame description is now rich enough to express caller-side per-layer placement, not just centered quads

However, the `2.4 s` comparison is still visibly off:

- the reconstruction remains much cleaner and brighter than the VHS capture
- the `COWDEX` wordmark is still too weak in the composite
- the exact original caller-side transform constants for the soft quads are still unknown
- the seed and time alignment are still only approximate

So this pass should be treated as a **buildable second-stage approximation**, not a faithful final lift.

## Next Useful Target

The next quality-improving task is narrower now:

1. recover or infer the exact caller-side transforms for the two intro soft quads
2. tune the intro-layer timing against a small hand-picked reference strip around `t = 2.0 .. 3.0 s`
3. only after that, decide whether the overlay blend or tint still needs adjustment, or whether the remaining mismatch is mostly VHS capture loss
