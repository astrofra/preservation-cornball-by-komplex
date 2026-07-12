# Source Reconstruction Workspace

This directory contains hand-written source reconstruction work derived from the static analysis under `reverse/work/notes/`.

Current scope:

- exact PRNG reconstruction for `lcg_rand15`
- first C99 lift of `render_scene_fla_particle_family`
- smoke test coverage for deterministic particle-state reconstruction

Build with Visual Studio 2022 via CMake:

```powershell
cmake -S reverse/work/reconstruction -B reverse/work/reconstruction/build -G "Visual Studio 17 2022" -A x64
cmake --build reverse/work/reconstruction/build --config Release
Push-Location reverse/work/reconstruction/build
ctest -C Release --output-on-failure
Pop-Location
```

The current code reconstructs scene logic and frame-description output only. It does not yet rebuild the original Win32, OpenGL, texture-upload, or MIDAS integration shell.
