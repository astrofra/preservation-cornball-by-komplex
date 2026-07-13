# Step 10 - Replay Integration And Kaar Camera

Date: 2026-07-12
Status: implemented

## Goal

Do three cleanup-oriented integration tasks:

- stop shipping separate scene preview executables as the main runtime shape
- provide one replay executable that chains the reconstructed scene families in original order
- revisit `kaar` with the confirmed global perspective camera from the original binary

## Kaar Camera Correction

The earlier `kaar` preview still used an orthographic approximation.

That was no longer defensible after the static bootstrap pass confirmed:

- `configure_viewport_and_projection` at `0x00401090`
- global camera `= gluPerspective(65.0, aspect, 1.0, 90.0)`

No scene-local projection override was found for `render_scene_kaar_family`.

So the `kaar` renderer now uses the same perspective envelope as:

- `intro`
- `surf`
- the original demo bootstrap

Source updated:

- [reverse/work/reconstruction/src/cornball_kaar_gl.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_kaar_gl.c)

## Single Replay Executable

Added:

- [reverse/work/reconstruction/src/cornball_demo_replay_win32.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_demo_replay_win32.c)

New primary Windows runtime target:

- `cornball_demo_replay.exe`

The replay keeps:

- one global PRNG state
- one persistent scene-state block per reconstructed family
- one local scene timer that resets at each segment boundary

Follow-up adjustment on 2026-07-13:

- the replay default client size is now `640x400`
- this is a comparison preset chosen to bias back-buffer captures toward the current VHS reference workflow
- explicit `--width` and `--height` overrides still take precedence
- the static binary reading still says the original Win32 shell created a `640x480` window, so the new default should be treated as an analysis aid, not as a corrected historical fact

## Chained Order

The replay now follows the full original `10`-slot dispatch script from the threshold table:

1. scene `0` -> `intro`
2. scene `1` -> `kaar`
3. scene `2` -> `s-pair`
4. scene `3` -> `surf`
5. scene `4` -> `s-pair`
6. scene `5` -> `fla`
7. scene `6` -> `surf`
8. scene `7` -> `intro`
9. scene `8` -> `kaar`
10. scene `9` -> `finale`

Current limitation:

- any family without a lifted renderer still falls back to a black frame
- scene `9` is kept as `finale` in the recovered script instead of being collapsed into a generic placeholder

## Timing Model

The original binary switches scenes on music positions, not raw wall-clock thresholds.

Since the exact music-position-to-seconds mapping is not yet rebuilt, the replay uses:

- original slot widths in music-position units
- a synthetic conversion controlled by `--position-seconds`
- on the live music path, `MIDASgetPlayStatus.position` remains authoritative across the whole frame step

Default:

- `1.0` second per original music-position unit

So the replay order and slot boundaries are original, while the absolute duration is still an approximation.

## Build Cleanup

The CMake target graph was simplified:

- removed separate scene preview executables from the default Windows build
- added one replay smoke test:
  - `cornball_demo_replay_smoketest`

The scene-specific source files remain in the repository for reference, but they are no longer built as default runtime artifacts.

## Verification

Validated on 2026-07-12 with:

```powershell
cmake --build reverse/work/reconstruction/build --config Release --target cornball_demo_replay
Push-Location reverse/work/reconstruction/build
ctest -C Release --output-on-failure
Pop-Location
```

Result:

- `fla_scene_smoketest` passed
- `intro_scene_smoketest` passed
- `kaar_scene_smoketest` passed
- `surf_scene_smoketest` passed
- `cornball_demo_replay_smoketest` passed

## Visual Spot Check

Two quick replay captures were generated during the pass:

- [replay_frame_000000.png](../reverse/work/reconstruction/captures/replay-kaar/replay_frame_000000.png)
- [replay_frame_000000.png](../reverse/work/reconstruction/captures/replay-kaar-13/replay_frame_000000.png)

Observation:

- the first `kaar` boundary frame can legitimately land on a nearly black branch because the family gates between overlay and tube-shell content per frame
- a later sample at `demo_seconds = 13.0` shows the expected `TXT1`-driven branch under the replayed state history

That does not prove full `kaar` visual fidelity yet, but it confirms the replay path is active and the perspective-camera change did not break the renderer.

## Outcome

The build now produces one main executable instead of multiple scene preview executables, and `kaar` no longer uses the earlier camera approximation.

Next useful targets:

- compare replay captures against reference-video anchors
- decide whether shared helper state should now move out of the per-family wrappers
- lift the missing `finale` family and any remaining incomplete scene-family output so black fallback frames disappear
