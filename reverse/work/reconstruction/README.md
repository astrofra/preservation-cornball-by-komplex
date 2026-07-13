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

Optional replay flags:

- `--hidden --frames 120 --position-seconds 0.05` for a short automated full-sequence smoke run
- `--asset-root <path>` to point at an alternate extracted asset directory
- `--width <pixels> --height <pixels>` to match a reference capture size
- `--seed <lcg-seed>` to explore deterministic overlay alignment
- `--demo-seconds <seconds>` to warm the chained replay before the first presented frame
- `--position-seconds <seconds>` to control how long one original music-position unit lasts in the synthetic replay
- `--capture-dir <path> --capture-every <n>` to dump back-buffer frames as `TGA`

The current code still does not rebuild the original demo's full startup shell, original audio path, the unreconstructed scene families, or the exact audio-driven duration mapping between music positions and wall-clock time.
