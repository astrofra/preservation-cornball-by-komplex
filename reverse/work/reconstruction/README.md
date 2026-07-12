# Source Reconstruction Workspace

This directory contains hand-written source reconstruction work derived from the static analysis under `reverse/work/notes/`.

Current scope:

- exact PRNG reconstruction for `lcg_rand15`
- first C99 lift of `render_scene_fla_particle_family`
- shared `fla` layering helpers for the rotating `LOGOTAUS` quad and the `TXT1` jitter overlay
- first C99 lift of `render_scene_kaar_family`, including its fogged tube-shell pass and centered `TXT1` quad
- OpenGL renderer for the reconstructed `fla` scene
- OpenGL renderer for the reconstructed `kaar` scene
- Win32/WGL preview executable for Windows 10/11
- smoke test coverage for both deterministic scene reconstruction and the hidden OpenGL preview paths

Build with Visual Studio 2022 via CMake:

```powershell
cmake -S reverse/work/reconstruction -B reverse/work/reconstruction/build -G "Visual Studio 17 2022" -A x64
cmake --build reverse/work/reconstruction/build --config Release
Push-Location reverse/work/reconstruction/build
ctest -C Release --output-on-failure
.\Release\fla_scene_preview.exe
.\Release\kaar_scene_preview.exe
Pop-Location
```

Optional preview flags:

- `--hidden --frames 2` for a short automated smoke run
- `--asset-root <path>` to point at an alternate extracted asset directory

The current code still does not rebuild the original demo's full startup shell, audio path, scene dispatcher, or the remaining scene families.
