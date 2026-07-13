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

- `kaar`: `x = cos(t * 0.3) * 3`, `y = cos(t * 0.2) * 190`
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

## Current Conclusion

What now looks solid:

- the first four slots are anchored to the correct early music-time windows
- `intro` and `surf` are the best-matching first-pass reconstructions in this range
- the original LCG seed really is `1`
- the tube-scene caller translations needed the `x/y` swap and have now been corrected
- the centered `TXT1` overlay was indeed hiding too much of `scene1`, and the cause was missing alpha reconstruction for the legacy TGAs rather than the recovered `glBlendFunc` call itself

What is still unresolved:

- the green/cyan flashing tube seen in the early VHS material almost certainly belongs to `scene1 / kaar`
- the current `kaar` reconstruction still does not expose that look at the calibrated early anchor
- fixed-step cadence alone does not explain the mismatch
- after fixing the texture-alpha path, the remaining gap now looks more like branch selection or scene-state timing than simple layer occlusion

So the next logical target is narrower than before:

1. add a forced or isolated `kaar` main-branch capture mode so the tube can be compared directly against the VHS frame
2. trace any remaining random-consumption or caller-timing mismatch now that the overlay no longer buries the background
3. only after that, revisit whether the remaining gap is a deeper helper/render-state issue

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
