# Step 5 - Reference Frame Sampling

Date: 2026-07-12
Status: completed

## Goal

Take a small constant-interval sample from the cropped reference-video frame set and see how far a still-image-only pass can go in identifying which part of the demo each sample belongs to.

## Inputs

- Cropped frame sequence: `reverse/work/reference-video/frames-cropped/`
- Existing capture note: [04-video-reference.md](04-video-reference.md)
- Scene-dispatch data:
  - `reverse/work/exports/planet-scene-dispatch.csv`
  - `reverse/work/exports/planet-scene-family-map.csv`

## Sampling Rule

The cropped sequence contains:

- `3284` frames
- source cadence preserved at `25 fps`
- frame `000000` = provisional scene-visible `t = 0.00 s`

To pick exactly `10` frames with a constant integer interval and stay inside the sequence:

- first index = `0`
- interval = `floor((3284 - 1) / (10 - 1)) = 364`

Chosen frame indices:

- `0`
- `364`
- `728`
- `1092`
- `1456`
- `1820`
- `2184`
- `2548`
- `2912`
- `3276`

This leaves the last `7` frames unsampled, which is acceptable for this quick-look pass because the constant-step constraint was more important than exact end coverage.

QC contact sheet:

- [ten-sample-contact-sheet.png](assets/video-sampling/ten-sample-contact-sheet.png)

## Method

The classification pass used three signals together:

1. direct visual inspection of the `10` sampled PNG files
2. comparison against converted texture previews for `KAAR128`, `SURF128`, `FLA`, `TXT1`, `TXT2`, `V1`, `V2`, `S1`, `S2`, `LOGO`, and `LOGOTAUS`
3. the known scene-family order from the static-analysis exports

Useful texture cues during manual comparison:

- `KAAR128` reads as a brown/gold textured field
- `SURF128` reads as a bright gray cell lattice on black
- `FLA` reads as a red flare / particle glow
- `TXT1` reads as dense white text on black
- `TXT2` reads as pale black-on-white typographic collage with the barcode-like vertical strokes

Important limitation:

- this pass does **not** recover an exact mapping from cropped-video `t = 0` to tracker `MIDASplayStatus.position`
- the first visible frame after the black screen may already be later than music position zero
- therefore the scene-family order can guide interpretation, but it cannot be turned into exact time windows from these stills alone

## Sample Table

| Sample | Frame | Scene time | Capture time | Best-effort inference | Confidence | Why |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | [frame_000000.png](../reverse/work/reference-video/frames-cropped/frame_000000.png) | `0.00 s` | `3.44 s` | opening typographic composite, likely `intro/logo` family | high | Strong `TXT2` motif: barcode-like vertical strokes and the same pale text collage seen in `TXT2`. It is also the first visible post-black frame. |
| 2 | [frame_000364.png](../reverse/work/reference-video/frames-cropped/frame_000364.png) | `14.56 s` | `18.00 s` | early brown textured phase, likely `kaar` family | medium | The background is close to `KAAR128` in color and texture. The diagonal pale text looks more like a `TXT1` overlay than a `TXT2` panel composition. |
| 3 | [frame_000728.png](../reverse/work/reference-video/frames-cropped/frame_000728.png) | `29.12 s` | `32.56 s` | brown/halo transition, likely late `kaar` or early first `s`-pair | low | The brown field still feels `KAAR128`-like, but the centered pale ring/halo is not cleanly explained by the currently lifted `kaar` branch. |
| 4 | [frame_001092.png](../reverse/work/reference-video/frames-cropped/frame_001092.png) | `43.68 s` | `47.12 s` | dark red `TXT1`-driven phase, possibly first `surf` or another transition scene | low | The frame is dominated by dense red-on-black text, which matches tinted `TXT1` better than `TXT2`. No unique geometry survives the capture strongly enough to lock the family. |
| 5 | [frame_001456.png](../reverse/work/reference-video/frames-cropped/frame_001456.png) | `58.24 s` | `61.68 s` | bright typographic panel phase, likely an `s`-pair-style composite | medium | Clear `TXT2` phrases and vertical strokes reappear. That fits the text-heavy families better than `fla` or `surf`. |
| 6 | [frame_001820.png](../reverse/work/reference-video/frames-cropped/frame_001820.png) | `72.80 s` | `76.24 s` | gray text/curve transition, likely late second `s`-pair or pre-`fla` bridge | low | Still-image evidence is weak here: typography is visible, but the underlying geometry is too blurred to separate `s`-pair from a transition into the later effect scenes. |
| 7 | [frame_002184.png](../reverse/work/reference-video/frames-cropped/frame_002184.png) | `87.36 s` | `90.80 s` | particle/glow phase, likely `fla` family | high | Multiple red/pink flare blobs closely resemble repeated use of `FLA.TGA`. This is the clearest unique scene-family anchor in the sample set. |
| 8 | [frame_002548.png](../reverse/work/reference-video/frames-cropped/frame_002548.png) | `101.92 s` | `105.36 s` | flare-over-text composite, likely `surf` family | high | A centered red flare sits underneath a text overlay. That combination matches the known `surf` asset mix (`surf128` + `fla` + `txt1`) better than pure `fla`. |
| 9 | [frame_002912.png](../reverse/work/reference-video/frames-cropped/frame_002912.png) | `116.48 s` | `119.92 s` | late dark red text phase, likely second `kaar` or another late `TXT1` transition | low-medium | The image returns to a red `TXT1` look shortly before the ending. The fixed order says this region should be late-scene material, but the still itself is not distinctive enough for a unique family ID. |
| 10 | [frame_003276.png](../reverse/work/reference-video/frames-cropped/frame_003276.png) | `131.04 s` | `134.48 s` | terminal fade / end panel, likely `finale` family | high | Nearly blank white field with a hard teal edge. This reads as a closing/fade state rather than an effect scene. |

## What Worked

This pass did recover a few useful anchors:

- the opening frame is clearly part of the early `TXT2`-heavy typographic section
- the `FLA`-driven particle phase is easy to spot at `frame_002184`
- the later `FLA` + text composite at `frame_002548` is a plausible `surf` anchor
- the near-white closing frame is a strong finale marker

It also suggests that:

- `TXT2`-heavy stills cluster early and mid-demo
- `TXT1`-heavy dark red stills appear in multiple later phases, which makes them weak family identifiers on their own
- constant-interval still sampling is enough to find broad landmarks, but not enough to uniquely label every middle scene

## What Failed

The still-only method does **not** uniquely recover the full scene order from these `10` images.

Main reasons:

- the capture is soft and compressed
- text overlays often dominate the frame and hide the underlying geometry
- several families reuse the same text textures
- cropped-video `t = 0` is a visibility-based convention, not a verified music-position zero

The most ambiguous samples are:

- `frame_000728.png`
- `frame_001092.png`
- `frame_001820.png`
- `frame_002912.png`

Those frames can be described visually, but not assigned to a single family with high confidence from the stills alone.

## Conclusion

This sampling pass partially succeeded.

It is good enough to identify a few robust visual landmarks across the reference capture:

- opening text-composite material
- a `kaar`-like brown textured phase
- the `fla` particle/glow phase
- a later `surf`-like flare-over-text phase
- the closing finale fade

It is **not** good enough to turn the entire 10-frame sample into a reliable one-to-one scene-family timeline.

The next useful comparison step would be one of:

1. build short local clips around the ambiguous sample frames, for example `sample +/- 12` frames
2. render reconstruction previews for `fla`, `kaar`, and later `surf`, then compare them against the matching sampled stills
3. recover a better music-position-to-video-time alignment before trying to label the middle stills more precisely
