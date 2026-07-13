# Source Reconstruction Workspace

This directory contains hand-written source reconstruction work derived from the static analysis under `reverse/work/notes/`.

Current scope:

- exact PRNG reconstruction for `lcg_rand15`
- exact caller-side lift of the opening `intro/logo` family using `V1`, `V2`, `TXT2`, `TXT1`, `LOGOTAUS`, and `LOGO`
- first C99 lift of `render_scene_fla_particle_family`
- shared `fla` layering helpers for the rotating `LOGOTAUS` quad and the `TXT1` jitter overlay
- first C99 lift of `render_scene_kaar_family`, including its fogged tube-shell pass and centered `TXT1` quad
- first C99 lift of `render_scene_surf_family`, including its inside-camera tube shell, stacked `FLA` flare quads, rotating `SURF128` foreground quad, and per-frame `TXT1` jitter overlay
- reusable lossless `TGA` frame dumping from the preview back buffer
- OpenGL renderer for the reconstructed `fla` scene
- OpenGL renderer for the reconstructed early `intro` stages
- OpenGL renderer for the reconstructed `kaar` scene
- OpenGL renderer for the reconstructed `surf` scene
- single Win32/WGL replay executable for Windows 10/11 that chains the reconstructed scene families in original order
- optional replay of the original `AAB.XM` soundtrack through the bundled `MIDAS06.DLL` when the replay is built as Win32/x86
- smoke test coverage for deterministic scene reconstruction plus a hidden end-to-end replay run

Build with Visual Studio 2022 via CMake:

```powershell
cmake -S reverse/work/reconstruction -B reverse/work/reconstruction/build -G "Visual Studio 17 2022" -A x64
cmake --build reverse/work/reconstruction/build --config Release
Push-Location reverse/work/reconstruction/build
ctest -C Release --output-on-failure
.\Release\cornball_demo_replay.exe
Pop-Location
```

To get music replay from the original `MIDAS06.DLL`, build the replay as `Win32` instead of `x64`:

```powershell
cmake -S reverse/work/reconstruction -B reverse/work/reconstruction/build-win32 -G "Visual Studio 17 2022" -A Win32
cmake --build reverse/work/reconstruction/build-win32 --config Release --target cornball_demo_replay
.\reverse\work\reconstruction\build-win32\Release\cornball_demo_replay.exe
```

In that `Win32` build:

- `MIDAS06.DLL` is copied automatically next to `cornball_demo_replay.exe`
- the replay looks for `MIDAS06.DLL` next to the executable before falling back to the resolved asset root
- `AAB.XM` is still loaded from the asset root by default, but if you also place it next to the executable, that local copy takes precedence

Default replay client size:

- `640x400`
- this is an intentional comparison preset for current VHS-ground-truth work, not a claim that the original Win32 shell stopped using `640x480`
- `--width` and `--height` still override it when a different capture target is needed

Optional replay flags:

- `--hidden --frames 120 --position-seconds 0.05` for a short automated full-sequence smoke run
- `--asset-root <path>` to point at an alternate extracted asset directory
- `--width <pixels> --height <pixels>` to match a reference capture size
- `--music` to force music startup when the backend is available
- `--no-music` to keep the replay silent even in Win32 interactive runs
- `--seed <lcg-seed>` to explore deterministic overlay alignment
- `--demo-seconds <seconds>` to warm the chained replay before the first presented frame
- `--position-seconds <seconds>` to control how long one original music-position unit lasts in the synthetic threshold-driven replay
- `--fixed-step-hz <hz>` to vary the deterministic state-update cadence during analysis captures
- `--force-kaar-main-branch` to keep `kaar` on its tube-branch while still preserving the original overlay-branch RNG consumption
- `--isolate-kaar-tube` to hide the centered `TXT1` quad during `kaar` and inspect the tube pass by itself
- `--capture-dir <path> --capture-every <n>` to dump back-buffer frames as `TGA`

Current music behavior:

- in Win32/x86 builds, visible replay runs try to start the original `AAB.XM` soundtrack through the bundled `MIDAS06.DLL`
- both the music-driven path and the synthetic fallback now follow the original `0x40e068` threshold table for all `10` scene slots
- scene slot `9` is preserved as the original unreconstructed `finale` family in the recovered script
- any family without a lifted renderer still falls back to a black frame
- x64 builds stay on the synthetic silent path because a 64-bit process cannot load the shipped 32-bit `MIDAS06.DLL`
- in auto mode, that x64 fallback is silent; use `--music` if you want a hard failure instead

The current code still does not rebuild the original demo's full startup shell, provide a modern open-source x64 XM backend, reconstruct the unrecovered scene families, or model the exact per-row visual timing inside those missing slots.
