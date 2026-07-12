# Step 4 - Video Reference Capture

Date: 2026-07-12
Status: completed

## Goal

Document the old video capture of the original demo, derive a practical crop box for the overscanned image, define a provisional capture-side "timecode zero", and generate reusable QC/reference outputs.

## Source Artifacts

| Artifact | Path | Notes |
| --- | --- | --- |
| MPEG capture | `original/video/TG98_3DFX_Complex_PlanetCornball.mpeg` | Old external capture of the demo. Not part of the original release directory. |
| Screenshot with overscan | `C:/Users/fra/Pictures/vlcsnap-2026-07-12-07h58m59s097.png` | Manually supplied reference frame showing the black border around the active image. |

Reference hashes:

- MPEG SHA-256: `2a397d207defcb660827270ffe9af94ad3d9b83a2fbc981fa3011e6bfcfdac6d`
- Screenshot SHA-256: `16039eb3812e54bf34c2efb2de55c71e9d95429e8885857b10c6c6006386ce2c`

## Capture Metadata

- Video geometry: `352x288`
- Frame rate: `25 fps`
- Container duration: `00:02:14.86`
- Video codec: `mpeg1video`
- Audio codec: `mp2`

## Limitation

This capture is useful as a qualitative reference only.

- It was almost certainly made through an external video-grab path.
- The image contains an overscan border that was not part of the intended demo output.
- Compression and analog/noisy edges make pixel-perfect comparison against the reconstruction unreliable.

It is still valuable for:

- scene order
- broad timing alignment
- camera/effect recognition
- rough framing checks

## Crop-Box Decision

The supplied screenshot clearly shows a centered active image inside black borders. After a second pass on the row/column edge transitions, the chosen visible-content crop is:

- `x = 16`
- `y = 19`
- `width = 310`
- `height = 238`

Why this box was chosen:

- the active image begins abruptly at column `16` and row `19`
- the visible content falls back to near-black immediately after column `325` and row `256`
- this box matches the captured visible image rather than an inferred idealized render target

Measured edge transitions from the screenshot:

| Edge | Transition evidence | Chosen coordinate |
| --- | --- | --- |
| left | column mean jumps from `15.65` at `x=15` to `76.32` at `x=16` | `x=16` |
| top | row mean jumps from `12.36` at `y=18` to `63.42` at `y=19` | `y=19` |
| right | column mean is still `42.17` at `x=325`, then drops to `4.05` at `x=326` | `x2=325` |
| bottom | row mean is still `36.11` at `y=256`, then drops to `5.34` at `y=257` | `y2=256` |

Observed threshold-driven bounding boxes on the screenshot:

| Gray threshold | Bounding box | Size |
| --- | --- | --- |
| `> 20` | `x=12, y=16, x2=327, y2=256` | `316x241` |
| `> 24` | `x=15, y=16, x2=325, y2=256` | `311x241` |
| `> 32` | `x=15, y=19, x2=325, y2=256` | `311x238` |

These threshold-derived boxes describe only the brighter inner area. They were useful as a sanity check, but the final crop was taken from the stronger left/right/top/bottom transitions to black.

QC stills stored in the repository:

- [overscan-source.png](assets/video-reference/overscan-source.png)
- [overscan-bbox-red.png](assets/video-reference/overscan-bbox-red.png)

## Provisional Timecode Zero

Working convention:

- capture-side `t = 0` for preservation comparisons is defined as the first frame after the black screen whose cropped-frame average luma exceeds `0.1`

Measured result:

- `blackdetect=d=0.05:pix_th=0.10` reported a black section from about `00:00:02.72` to `00:00:03.44`
- on the cropped `310x238` area, `signalstats` reported `YAVG` around `23.2` to `23.5` from `00:00:03.00` through `00:00:03.40`
- `0.1` in normalized `0..1` luma corresponds to about `25.5` in `0..255` `YAVG`
- the first cropped frame above that threshold appears at `00:00:03.44`, with `YAVG = 123.924`

For the rest of the project, this step treats:

- `00:00:03.44` in the MPEG capture as provisional "timecode zero"

Stored first-frame reference:

- [timecode-zero-frame.png](assets/video-reference/timecode-zero-frame.png)

## Generated Frame Sequence

Generated with `ffmpeg` into:

- `reverse/work/reference-video/frames-cropped/`

Sequence properties:

- crop: `310x238`
- start point: first frame with `t >= 00:00:03.44`
- cadence: source cadence preserved at `25 fps`
- frame count: `3284`
- first frame: `frame_000000.png` -> capture time `00:00:03.44`
- last frame: `frame_003283.png`
- total size on disk: about `299.65 MiB`

Representative links:

- [frame_000000.png](../reverse/work/reference-video/frames-cropped/frame_000000.png)
- [frame_003283.png](../reverse/work/reference-video/frames-cropped/frame_003283.png)

## Commands Used

Video metadata:

```powershell
ffprobe -hide_banner original/video/TG98_3DFX_Complex_PlanetCornball.mpeg
```

Black-screen detection:

```powershell
ffmpeg -hide_banner -ss 0 -t 8 -i original/video/TG98_3DFX_Complex_PlanetCornball.mpeg -vf blackdetect=d=0.05:pix_th=0.10 -an -f null -
```

Frame-average luma on the cropped area:

```powershell
ffmpeg -hide_banner -i original/video/TG98_3DFX_Complex_PlanetCornball.mpeg -vf "select='between(t,3,4)',crop=310:238:16:19,signalstats,metadata=print:file=-" -an -f null -
```

QC images:

```powershell
ffmpeg -y -hide_banner -i "C:/Users/fra/Pictures/vlcsnap-2026-07-12-07h58m59s097.png" -frames:v 1 -update 1 documentation/assets/video-reference/overscan-source.png
ffmpeg -y -hide_banner -i "C:/Users/fra/Pictures/vlcsnap-2026-07-12-07h58m59s097.png" -vf "drawbox=x=16:y=19:w=310:h=238:color=red@1.0:thickness=2" -frames:v 1 -update 1 documentation/assets/video-reference/overscan-bbox-red.png
ffmpeg -y -hide_banner -i original/video/TG98_3DFX_Complex_PlanetCornball.mpeg -vf "select='gte(t,3.44)',crop=310:238:16:19" -frames:v 1 -update 1 documentation/assets/video-reference/timecode-zero-frame.png
```

Exact cropped PNG sequence:

```powershell
ffmpeg -y -hide_banner -i original/video/TG98_3DFX_Complex_PlanetCornball.mpeg -vf "select='gte(t,3.44)',crop=310:238:16:19" -fps_mode passthrough -start_number 0 reverse/work/reference-video/frames-cropped/frame_%06d.png
```

## Practical Use In Later Steps

Use this capture as a secondary reference only.

- Do not use it for pixel-accurate validation.
- Do use it to confirm scene ordering, gross timing, framing, and whether a reconstructed shot is recognizably correct.
- Prefer the cropped PNG sequence over the raw MPEG whenever side-by-side inspection is needed.
