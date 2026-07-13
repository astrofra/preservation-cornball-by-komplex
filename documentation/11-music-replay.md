# Step 11 - Music Replay

Date: 2026-07-13
Status: implemented with Win32-only backend

## Goal

Add soundtrack playback to the reconstructed replay without introducing a large new dependency stack before the missing scene families are recovered.

## Implemented Path

The replay now has an optional music backend that reuses:

- `MIDAS06.DLL`
- `AAB.XM`

from the original demo asset directory.

This backend is loaded dynamically from the resolved asset root, so the reconstruction build does not need an import library for MIDAS.

Follow-up packaging improvement:

- the `Win32` replay build now copies `MIDAS06.DLL` next to `cornball_demo_replay.exe`
- the runtime probes the executable directory first for `MIDAS06.DLL`
- the runtime also probes the executable directory first for `AAB.XM`, then falls back to the resolved asset root

## Why This Is Win32-Only

The bundled `MIDAS06.DLL` is a 32-bit DLL.

That means:

- a `Win32` replay build can load it directly
- an `x64` replay build cannot load it at all

So the current implementation boundary is:

- `Win32/x86` replay: music available
- `x64` replay: silent fallback remains in place

This is a process-architecture constraint, not a replay bug.

## Runtime Behavior

When music startup is enabled and the backend initializes successfully, the replay now:

- calls `MIDASstartup`
- calls `MIDASinit`
- starts background playback with `MIDASstartBackgroundPlay(0)`
- loads `AAB.XM`
- starts module playback
- polls `MIDASgetPlayStatus`

The polled `position` field now drives scene dispatch directly.

## Scene Dispatch Change

Before this pass, the replay used a compressed synthetic timeline:

- `intro -> kaar -> surf -> fla -> surf -> intro -> kaar`

with missing scene families removed.

That compressed timeline is still kept as the fallback when music is not active.

When music is active, the replay instead follows the original scene slots:

1. scene `0` -> `intro`
2. scene `1` -> `kaar`
3. scene `2` -> placeholder
4. scene `3` -> `surf`
5. scene `4` -> placeholder
6. scene `5` -> `fla`
7. scene `6` -> `surf`
8. scene `7` -> `intro`
9. scene `8` -> `kaar`
10. scene `9` -> placeholder

Current placeholder policy:

- unreconstructed slots render as black frames

This is intentionally explicit so the music can stay on the original order grid without pretending that the missing families are already reconstructed.

## Controls

New replay flags:

- `--music`
- `--no-music`

Current default:

- visible interactive run: try to enable music automatically
- hidden / automated run: keep music off unless `--music` is requested explicitly

## Validation Notes

This pass is designed to preserve two workflows:

- deterministic hidden smoke tests without sound
- interactive Win32 playback with original music timing

The next preservation step after this one should be:

1. validate the Win32 music path against more reference captures
2. replace black placeholders by reconstructed `s-pair` and finale families
3. later replace the legacy MIDAS dependency with a modern open-source x64-capable XM backend
