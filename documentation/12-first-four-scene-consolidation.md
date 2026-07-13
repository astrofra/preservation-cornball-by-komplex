# Step 12 - First Four Scene Consolidation

Date: 2026-07-13
Status: in progress

## Goal

Consolidate the first four original scene slots against the cropped VHS reference by checking:

- scene order
- music-time anchoring
- scene-local state progression
- obvious caller-side transform mistakes

This pass treats the first four original scene slots as:

1. scene `0` -> `intro`
2. scene `1` -> `kaar`
3. scene `2` -> `s-pair`
4. scene `3` -> `surf`

## Music-Time Calibration

The Win32 replay was run with the original `MIDAS06.DLL` and `AAB.XM`, using scene-transition logging.

Measured early scene boundaries:

- scene `0` start: `0.000 s`, tracker position `0`
- scene `1` start: `38.200 s`, tracker position `12`
- scene `2` start: `57.433 s`, tracker position `18`
- scene `3` start: `70.250 s`, tracker position `22`
- scene `4` start: `79.867 s`, tracker position `25`

For silent deterministic comparison runs, this gives a practical synthetic rate near `3.19 s` per tracker-position unit.

The capture passes below used:

- `--position-seconds 3.183333`

as the first calibrated fallback. That value is good enough for the first four slots, but should still be treated as provisional rather than final.

## Iteration 1

One representative frame was captured for each of the first four slots.

Artifacts:

- [iteration 1 captures](../reverse/work/reconstruction/captures/scene-consolidation-iter1/)

Outcome:

- `scene0 / intro` already matched the VHS reference family well
- `scene3 / surf` also looked structurally correct
- `scene1 / kaar` was the main outlier
- `scene2 / s-pair` was plausible, but still noisy and not tightly anchored

## Iteration 2

Each of the first four slots was sampled at early, middle, and late points.

Artifacts:

- [comparison sheet](../reverse/work/reconstruction/captures/scene-consolidation-iter2/comparison-sheet.png)

Main observation:

- the early `scene1 / kaar` VHS frame at `43.008 s` (`frame_001075`) shows a cyan/green tube-like interior with a dark center
- the current reconstruction at the same anchor still shows a red `TXT1`-dominated branch instead

That mismatch was too strong to dismiss as VHS blur or minor timing drift.

## Iteration 3

To check whether this was only a one-frame phase issue, a short deterministic burst was captured around the early `kaar` anchor.

Artifacts:

- [scene1 burst contact sheet](../reverse/work/reconstruction/captures/scene-consolidation-iter3-scene1-burst/contact-sheet.png)

Result:

- nearby frames still alternated between dark and red-text states
- the expected tube-heavy image did not appear in that burst

So the problem was not just "one unlucky captured frame".

## Iteration 4

Two concrete reconstruction issues were then rechecked against the original binary.

### Confirmed Original LCG Seed

`lcg_rand15` at `0x004040b0` reads and writes `data.0040e21c`.

That global is initialized in the binary image to:

- `1`

So the current replay default of seed `1` is historically consistent. The early `kaar` mismatch is not explained by a wrong starting seed.

### Corrected Tube-Scene Translate Axes

Re-reading the original `glTranslatef` call sites showed that the reconstructed `x/y` assignments for the two tube scenes had been swapped.

Corrected formulas now used by the source lift:

- `kaar`: `x = cos(t * 0.2) * 3`, `y = cos(t * 0.3) * 3`
- `surf`: `x = cos(t * 0.3) * 3`, `y = cos(t * 0.2) * 3`

This correction was applied in:

- [reverse/work/reconstruction/src/cornball_kaar_scene.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_kaar_scene.c)
- [reverse/work/reconstruction/src/cornball_surf_scene.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_surf_scene.c)

### Fixed-Step Sweep

An analysis-only replay control was added:

- `--fixed-step-hz <hz>`

This makes it possible to vary the deterministic state-update cadence during hidden capture runs without changing the real-time music path.

Artifact:

- [scene1 early fixed-step sweep](../reverse/work/reconstruction/captures/scene-consolidation-step-hz/scene1-early-fixed-step-sheet.png)

Tested rates:

- `25 Hz`
- `30 Hz`
- `50 Hz`
- `60 Hz`

Result:

- changing the fixed-step cadence clearly changes the red-text branch layout
- none of the tested cadences reproduced the cyan/green early `kaar` tube frame by themselves

## Iteration 5

The next check narrowed the issue from generic scene timing to texture-alpha reconstruction.

### Legacy TGA Alpha Path

All shipped scene textures are stored as `32bpp` TGAs, but their headers report:

- descriptor `0x00`
- zero attribute bits
- zero-valued fourth-byte payload across the image body

So the previous GL upload path was too literal: it copied a dead fourth byte into alpha for every texture.

That made the centered `TXT1` pass in `kaar` behave like a darkening mask instead of a readable additive/screen-like overlay, because the recovered blend state really is:

- `glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR)`

The reconstruction was updated to synthesize alpha from the strongest RGB channel whenever a `32bpp` TGA declares zero alpha bits.

### Capture Outcome

Artifacts:

- [scene1 alpha check](../reverse/work/reconstruction/captures/scene1-alpha-check/replay_frame_000000.png)
- [scene1 alpha burst](../reverse/work/reconstruction/captures/scene1-alpha-burst/contact-sheet.png)

Observed change:

- the early `scene1` frame no longer collapses into a flat red `TXT1` wall
- the buried brown/black background layer is now visible through the text overlay
- the same alpha reconstruction did not obviously break the opening `intro` sanity capture

What did not change:

- the short burst around the same `43.008 s` anchor still does not expose the cyan/green tube frame seen in the VHS material

So this iteration resolves the compositing bug, but not the remaining branch/timing mismatch.

## Iteration 6

To compare the `kaar` tube branch directly against the VHS anchor, two analysis-only replay controls were added:

- `--force-kaar-main-branch`
- `--isolate-kaar-tube`

The first keeps `kaar` on its tube branch while preserving the original RNG consumption that would have happened on overlay frames. The second implies the tube branch and also hides the centered `TXT1` quad so the tube pass can be inspected by itself.

### Capture Outcome

Artifacts:

- [scene1 forced-main anchor](../reverse/work/reconstruction/captures/scene1-force-main-anchor/replay_frame_000000.png)
- [scene1 forced-main burst](../reverse/work/reconstruction/captures/scene1-force-main-burst/contact-sheet.png)
- [scene1 isolate-tube anchor](../reverse/work/reconstruction/captures/scene1-isolate-tube-anchor/replay_frame_000000.png)
- [scene1 isolate-tube burst](../reverse/work/reconstruction/captures/scene1-isolate-tube-burst/contact-sheet.png)

Observed change:

- forcing the main branch removes the red overlay wall and leaves the brown/black centered `TXT1` composition in front of the scene
- isolating the tube pass removes that centered quad entirely

Key finding:

- the isolated `kaar` tube pass is effectively black at the early `43.008 s` anchor
- the short isolated burst is also black throughout

So the remaining mismatch is now narrower than before. The current failure is no longer "the overlay hides the tube." It is "the lifted `kaar` tube pass itself is not producing the expected visible image at this anchor."

## Iteration 7

A caller-side recheck against `0x00402330` found that the first `kaar` lift had decoded `glTranslatef` with the wrong stack order.

Corrected transform:

- `x = cos(t * 0.2) * 3`
- `y = cos(t * 0.3) * 3`
- the `190` amplitude belongs only to `rx = sin(t * 0.3) * 190`

### Capture Outcome

Artifacts:

- [scene1 isolate-tube anchor after translation fix](../reverse/work/reconstruction/captures/scene1-isolate-tube-anchor-translate-fix/replay_frame_000000.png)
- [scene1 force-main anchor after translation fix](../reverse/work/reconstruction/captures/scene1-force-main-anchor-translate-fix/replay_frame_000000.png)
- [reference VHS anchor](../reverse/work/reference-video/frames-cropped/frame_001075.png)

Observed change:

- the isolated `kaar` anchor is no longer black
- it now exposes the expected cyan tube interior and a dark circular core, which was the missing visual cue from the VHS material
- the force-main anchor still shows the centered `TXT1` layer dominating most of the frame, so the remaining mismatch has moved from tube visibility to scene composition

Practical conclusion:

- the shared tube helper and `KAAR128` pass are now close enough to the original to serve as a visual anchor
- the next `kaar` target is the centered `TXT1` caller setup: exact quad placement, rotation, and how strongly it should sit in front of the tube at this moment

## Iteration 8

The tail of `render_scene_kaar_family` was rechecked directly against `0x00402531 .. 0x004025aa`.

Recovered caller-side `TXT1` details:

- `glColor4f(0.5, 0.3 + gate_tint, 0.2, 1.0)`
- `glBindTexture(GL_TEXTURE_2D, 3)`
- `glLoadIdentity()`
- `glRotatef(t * 11, 0, 0, 1)`
- `glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR)`
- `draw_centered_textured_quad(3.0, 3.0)`

The quad helper itself is not a perfectly symmetric `0.01 .. 0.99` UV rectangle. Its original per-vertex mapping is:

- `( +3, -3, -2 ) -> (0.99, 0.01)`
- `( +3, +3, -2 ) -> (0.99, 1.00)`
- `( -3, +3, -2 ) -> (0.01, 0.99)`
- `( -3, -3, -2 ) -> (0.01, 0.01)`

### Capture Outcome

Artifact:

- [scene1 force-main anchor after TXT1 tail pass](../reverse/work/reconstruction/captures/scene1-force-main-anchor-txt1-tail-pass/replay_frame_000000.png)

Observed change:

- the `TXT1` pass is now aligned with the original helper's exact UV layout
- the visible change at this anchor is small, which means the remaining mismatch is not explained by a gross caller-tail mistake anymore

Practical conclusion:

- the centered `TXT1` tail is now close to the original binary at the call/parameter level
- the remaining scene1 mismatch is more likely to live in the neighboring branch behavior or in the relative scene composition than in the `TXT1` tail helper itself

## Current Conclusion

What now looks solid:

- the first four slots are anchored to the correct early music-time windows
- `intro` and `surf` are the best-matching first-pass reconstructions in this range
- the original LCG seed really is `1`
- the tube-scene caller translations needed the `x/y` swap and have now been corrected
- the isolated `kaar` tube anchor is visible again and broadly matches the VHS cue of a cyan interior with a dark center
- the centered `TXT1` tail now matches the original caller-side color, rotation, blend mode, size, and helper UV layout closely
- the centered `TXT1` overlay was indeed hiding too much of `scene1`, and the cause was missing alpha reconstruction for the legacy TGAs rather than the recovered `glBlendFunc` call itself

What is still unresolved:

- the green/cyan flashing tube seen in the early VHS material almost certainly belongs to `scene1 / kaar`
- fixed-step cadence alone does not explain the mismatch
- after fixing the texture-alpha path, the `glTranslatef` stack-order bug, and the exact `TXT1` tail helper layout, the remaining gap now looks more like neighboring branch behavior or overall scene composition than a missing tube pass
- the dark core in the reconstructed tube still sits lower than in the VHS anchor, so some camera or caller-side quad nuance may still be off

So the next logical target is narrower than before:

1. recover the neighboring `kaar` branch-local details around the overlay path and the reused gate local, especially whether the red overlay uses `gate_tint` rather than `gate_unit`
2. compare the corrected force-main anchor against the VHS frame to tune why the dark core is vertically offset and why the foreground text layer still reads too strongly
3. only after that, revisit whether any remaining mismatch is random-consumption noise rather than caller-side composition

## Verification

After the texture-alpha reconstruction change:

```powershell
cmake --build reverse/work/reconstruction/build-win32 --config Release
Push-Location reverse/work/reconstruction/build-win32
ctest -C Release --output-on-failure
Pop-Location
```

Result:

- all `6` current tests passed
