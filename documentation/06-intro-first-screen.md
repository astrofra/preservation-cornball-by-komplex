# Step 6 - Intro First-Screen Reconstruction

Date: 2026-07-12
Status: first pass completed

## Goal

Implement the first visible screen from the `intro/logo` family as a buildable reconstruction target, and add an internal frame-dump path so the result can be checked against the cropped VHS reference.

## Implemented Scope

This pass adds a new `intro` reconstruction slice under `reverse/work/reconstruction/`:

- [include/cornball/intro_scene.h](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/include/cornball/intro_scene.h)
- [include/cornball/intro_gl.h](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/include/cornball/intro_gl.h)
- [src/cornball_intro_scene.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_intro_scene.c)
- [src/cornball_intro_gl.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_intro_gl.c)
- [src/cornball_intro_preview_win32.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_intro_preview_win32.c)
- [tests/intro_scene_smoketest.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/tests/intro_scene_smoketest.c)

It also adds a reusable lossless capture writer:

- [include/cornball/capture.h](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/include/cornball/capture.h)
- [src/cornball_capture.c](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/reconstruction/src/cornball_capture.c)

The existing `fla` and `kaar` preview executables now expose the same capture-oriented preview flags.

## Reconstruction Choice

This is intentionally a **first-screen subset**, not a full lift of `render_scene_intro_logo_family`.

The current reconstruction models these parts:

- perspective projection matching the original bootstrap path: `65` degree vertical FOV, near `1`, far `90`
- the opening `TXT2` jitter-overlay behavior using the original `u/v = jitter - 1 .. jitter` window
- wrapped texture sampling for the overlay path, which was necessary to make the helper behave like a scrolling sample window instead of a clamped crop
- the rotating `V`-texture geometry as a first-pass bipyramid layer
- deterministic fixed-step preview stepping at `60 Hz` for automated capture runs

The current first-screen approximation deliberately defers or simplifies these points:

- exact `LOGO` / `LOGOTAUS` soft-quad timing
- exact bipyramid color modulation and front/back visibility
- exact original RNG seed and scene-start alignment
- VHS-era softness, analog bleed, and capture artifacts

## New Preview Capture Path

All current Win32 preview executables now support:

- `--hidden`
- `--frames <n>`
- `--width <pixels>`
- `--height <pixels>`
- `--seed <lcg-seed>`
- `--scene-seconds <seconds>`
- `--capture-dir <path>`
- `--capture-every <n>`

Captured frames are written as lossless uncompressed `24-bit` TGA files.

That format was chosen because:

- it is trivial to emit from the OpenGL back buffer without adding a heavyweight image dependency
- it is deterministic
- it can be converted to PNG later with `ffmpeg` when needed

## Commands Used

Build and verify:

```powershell
cmake -S reverse/work/reconstruction -B reverse/work/reconstruction/build -G "Visual Studio 17 2022" -A x64
cmake --build reverse/work/reconstruction/build --config Release
Push-Location reverse/work/reconstruction/build
ctest -C Release --output-on-failure
Pop-Location
```

Generate a 16-frame opening strip at the cropped-reference size:

```powershell
Push-Location reverse/work/reconstruction/build
.\Release\intro_scene_preview.exe `
  --hidden `
  --frames 16 `
  --capture-every 1 `
  --width 310 `
  --height 238 `
  --capture-dir ..\captures\intro-opening
Pop-Location
```

## Comparison Artifacts

Generated artifacts for this pass:

- [intro_frame_000000.png](assets/intro-comparison/intro_frame_000000.png)
- [intro_contact_sheet.png](assets/intro-comparison/intro_contact_sheet.png)
- [reference_frame_000000.png](assets/intro-comparison/reference_frame_000000.png)
- [reference_vs_intro_first_frame.png](assets/intro-comparison/reference_vs_intro_first_frame.png)

Local raw capture sequence:

- `reverse/work/reconstruction/captures/intro-opening/intro_frame_*.tga`

## Result

This first pass is **directionally correct**, but not yet faithful enough to treat as a final lift.

What now matches the VHS reference reasonably well:

- the opening family is clearly the right one
- the frame is dominated by `TXT2` typography rather than the later `fla` or `kaar` material
- the moving brown/black `V`-derived shape layer is present
- the automated preview can now generate a deterministic captured strip for iteration

What remains visibly wrong:

- the reconstruction is cleaner and more contrasty than the VHS material
- the `V` layer placement is only approximate
- the current subset still omits the later intro-family soft-quad stages
- the exact first visible seed/time alignment is still unresolved

## Next Useful Iteration

The next quality-improving target is to keep the same capture loop and tighten one of these:

1. recover the exact early intro draw order around the `LOGOTAUS` / `LOGO` soft quads
2. recover the exact original front/back bipyramid visibility instead of the current first-screen simplification
3. search seed and `scene_seconds` combinations against the reference opening frame now that automated frame dumps exist
