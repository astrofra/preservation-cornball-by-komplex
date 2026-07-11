# Planet Cornball Decompilation Feasibility Study

Date: 2026-07-11
Scope: `original/cornball`
Method: static analysis only. The executable was not run on the host system.

## Objective

Assess whether the Windows demo in `original/cornball` can be reverse engineered into source code close to the original, ideally C or C++, with annotated assembly as a fallback, and whether that reconstructed source can eventually produce a Windows 10/11 binary.

## Executive Summary

This project is feasible.

The most realistic target is not recovery of the exact original source tree, but recovery of a structurally close and maintainable C or C++ codebase that reproduces the original behavior. Recovering the exact original identifiers, comments, source layout, and toolchain-specific details is not realistic from this binary alone.

The binary is favorable for reconstruction:

- `PLANET.EXE` is a small, unpacked 32-bit PE.
- It has no resource encryption, no packer, and no anti-analysis signs in the initial pass.
- Asset names, credits, window title, and runtime error strings are present in clear text.
- The rendering path uses a narrow subset of fixed-function OpenGL 1.x.
- The audio interface surface is very small and easy to replace.
- The texture loader logic is simple enough to reconstruct almost directly into C.

The main preservation risk is not decompilation itself. The main risks are:

- lack of symbols and original names
- x87-heavy math code that will need manual cleanup
- legacy runtime dependencies tied to 1998-era 3dfx MiniGL and MIDAS audio
- the gap between "decompiled pseudocode" and "clean, rebuildable source"

## Feasibility Rating

| Goal | Feasibility | Notes |
| --- | --- | --- |
| Recover exact original source | Low | Names, comments, original file split, and exact compiler artifacts are gone. |
| Recover close C/C++ source | High | Small code size, plain assets, and readable subsystem boundaries make this realistic. |
| Recover annotated assembly | Very high | Straightforward if C reconstruction hits ambiguity. |
| Produce a functionally equivalent Windows 10/11 build | High | Requires replacing or isolating legacy 3dfx and MIDAS dependencies. |
| Produce a byte-identical original binary | Very low | Missing exact toolchain, build flags, libraries, and source layout. |

## Static Findings

### Binary inventory

- Main executable: `original/cornball/PLANET.EXE`
- Size: 66,048 bytes
- SHA-256: `c6bf4cf57251a16fa2a5664e9809d16558b12365c18bda53777d03ac00d3c86a`
- Last write time on disk: 1998-04-08 11:16:32
- Pack status: not packed by UPX
- PE type: 32-bit x86 (`Machine = 0x14c`)
- Sections: `.text`, `.rdata`, `.data`, `.idata`
- Symbols/debug info: none found
- Resources: none found
- Relocations: none found
- Rich header: none found

### Code profile

- `.text` raw size is `0xB600`, which is modest and favorable for manual recovery.
- The code is math-heavy: a linear disassembly pass found 646 x87 instructions.
- Imports and startup patterns indicate a Microsoft Visual C++ era toolchain, likely late 1990s.
- I did not see convincing evidence of advanced C++ metadata such as RTTI. This looks more like C or C-with-very-light-C++ usage than class-heavy C++.

This last point is an inference from the binary, not a direct proof.

### Estimated original implementation language

My current estimate is:

- C: about 70%
- C++: about 30%

Important clarification:

- almost all of that 30% C++ probability is "procedural or C-style C++"
- the probability of class-heavy, idiomatic late-1990s MSVC C++ is low, roughly under 10%

Evidence pushing the estimate toward C:

- no MSVC RTTI markers were found
- no obvious C++ mangled names were found
- no obvious vtable blocks were found in `.rdata`
- no clear `thiscall` / object-method calling pattern stood out in the quick pass
- the visible program structure is very procedural: global state, asset tables, Win32 setup, OpenGL immediate-mode rendering, and small wrapper functions

Evidence preventing a stronger C-only claim:

- the binary was almost certainly linked with Microsoft tooling and static CRT components from the period
- the executable contains the standard `pure virtual function call` runtime string, even though no direct cross-reference to that string was found in the program logic
- in this era it was common to build mostly procedural demo code as `.cpp` while using very little actual C++ language machinery

So the best practical reading is:

- most likely written in C, or
- written in very light C++ that would reconstruct naturally back into C anyway

### Windowing and rendering

The executable imports from:

- `KERNEL32.dll`
- `USER32.dll`
- `GDI32.dll`
- `GLU32.dll`
- `OPENGL32.dll`
- `midas06.dll`

The rendering side is narrow and conventional:

- `ChoosePixelFormat`
- `SetPixelFormat`
- `SwapBuffers`
- `gluPerspective`
- fixed-function OpenGL calls such as `glTexImage2D`, `glBindTexture`, `glBegin`, `glVertex3f`, `glTexCoord2f`, `glRotatef`, `glTranslatef`, `glFog*`, `glClear`, `glEnable`, `glDisable`

This is good news. A fixed-function OpenGL 1.x demo from this era is much easier to reconstruct into readable C than a custom software renderer, self-modifying code, or a heavily abstracted engine.

The window creation code clearly embeds:

- title/class name: `Planet Cornball`
- size: `640 x 480`

### Bundled OpenGL DLL

The package includes `original/cornball/OPENGL32.DLL`.

Local inspection shows that this bundled DLL imports:

- `glide2x.dll`
- `KERNEL32.dll`
- `USER32.dll`

That means the shipped `OPENGL32.DLL` is a 3dfx-era MiniGL-style wrapper, not a modern system OpenGL runtime. This matters for preservation:

- it explains the `file_id.diz` text: `planet cornball. 3dfx minigl demo`
- it is not a good long-term runtime target for Windows 10/11
- the reconstructed source should target standard system `opengl32.dll` or a portability layer such as SDL2 or GLFW, not this bundled DLL

### Audio

`PLANET.EXE` imports only a very small set of MIDAS functions:

- `MIDASstartup`
- `MIDASinit`
- `MIDASloadModule`
- `MIDASplayModule`
- `MIDASstartBackgroundPlay`
- `MIDASgetPlayStatus`
- `MIDASstopModule`
- `MIDASstopBackgroundPlay`
- `MIDASfreeModule`
- `MIDASclose`

This is excellent for preservation. It means the original audio integration is narrow and can be replaced behind a small compatibility layer.

The music filename is present in clear text:

- `aab.xm`

### Assets and embedded strings

The binary contains in clear text:

- window title: `Planet Cornball`
- credits
- error messages for pixel-format setup
- texture filenames
- music filename

Recovered clear-text asset names include:

- `logotaus.tga`
- `logo.tga`
- `txt2.tga`
- `txt1.tga`
- `v2.tga`
- `v1.tga`
- `s2.tga`
- `s1.tga`
- `kaar128.tga`
- `fla.tga`
- `surf128.tga`
- `aab.xm`

This strongly suggests there is no asset-name obfuscation layer.

Sample credit strings embedded in the binary identify contributors and make later code annotation easier.

### Texture format and loader behavior

The included TGA files have standard headers indicating:

- image type 2
- 256 x 256
- 32 bits per pixel

The internal texture loader is one of the strongest positive signals in this study.

From disassembly, the loader:

- opens a file by name
- skips the 18-byte TGA header
- allocates `0x40000` bytes
- reads `0x40000` bytes of pixel data
- swaps color channels in-place before upload

That is exactly the kind of routine that can be rewritten into compact, readable C with very little ambiguity.

The upload paths then call `glTexImage2D` with 256 x 256 RGBA-style parameters. This gives us both file-format evidence and a direct behavioral validation target.

## What This Means For Source Recovery

### Best realistic outcome

The best realistic outcome is:

- clean reconstructed C or C++ source
- named subsystems
- reproducible scene order and timing
- functional replacement of legacy libraries where needed
- a Windows 10/11 build that reproduces the demo visually and audibly

### What will not come back automatically

The following are effectively lost unless another source archive appears:

- original variable names
- original function names
- comments
- exact source file boundaries
- exact compiler flags
- exact library versions
- exact original build scripts

### Why C or C++ is still realistic

Even without original names, the program shape is recoverable because the executable is small and the subsystem seams are visible:

- Win32 window/bootstrap
- WGL pixel format setup
- OpenGL state setup
- TGA loading and texture upload
- XM module playback
- main loop and scene/effect functions

This is enough to rebuild the program from behavior, not just from pseudocode.

If the original code was C++, it was probably close enough to C in structure that reconstructing it as C would still be a defensible preservation choice.

## Recommended Reconstruction Strategy

### Phase 1: preservation baseline

1. Freeze hashes for the original zip, executable, DLLs, XM, and TGA files.
2. Keep the original directory unmodified.
3. Work from copies in a separate `work/` or `reverse/` area.
4. Document every hash and every extracted observation.

### Phase 2: static reverse engineering

Recommended open-source tools:

- Ghidra for primary disassembly and decompilation
- Cutter or Rizin for fast cross-checking, string/xref work, and alternate analysis
- `pefile` and Capstone for small scripted checks

Tasks:

1. Import `PLANET.EXE` into Ghidra.
2. Label import thunks and the obvious wrappers first.
3. Rename all string-referenced functions.
4. Split code into subsystems:
   - startup
   - window creation
   - pixel format / WGL init
   - audio
   - asset loading
   - texture upload
   - scene/effect routines
   - main loop / timing
5. Recover global structures and arrays.

### Phase 3: dynamic analysis in isolation

Do this in a disposable VM, not on the preservation host.

Recommended tools:

- x64dbg for Windows user-mode debugging
- WinDbg if deeper Windows-specific runtime inspection is needed

Goals:

1. Confirm scene order and timing.
2. Observe file access and load order.
3. Confirm whether removing the local `OPENGL32.DLL` lets the demo use the system OpenGL path.
4. Validate assumptions about texture IDs, frame progression, and audio state transitions.

### Phase 4: source reconstruction

Do not try to compile raw decompiler output.

Instead:

1. Treat decompiler output as a reference.
2. Rewrite each subsystem into new source files by hand.
3. Preserve the original control flow where it affects timing or visuals.
4. Keep assembly notes beside ambiguous math-heavy routines.
5. Add regression notes after each recovered function.

### Phase 5: modern rebuild target

For long-term rebuildability on Windows 10/11, use one of these strategies:

#### Option A: closest to original behavior

- Win32 window creation
- system `opengl32.dll`
- fixed-function OpenGL compatibility profile
- small wrapper replacing MIDAS with a modern module decoder/player

This is the best preservation target.

#### Option B: slightly more portable

- SDL2 or GLFW for window/context creation
- fixed-function-compatible OpenGL path
- `libopenmpt` or `libxmp` for XM playback

This is slightly less original in structure, but often better for long-term maintenance.

## Audio Library Recommendation

There are two realistic paths:

### Path 1: preserve behavior, replace implementation

Use a small compatibility layer that exposes the same narrow operations currently imported from MIDAS, but backs them with:

- `libopenmpt`, or
- `libxmp`

This is the most practical solution for a rebuildable modern codebase.

### Path 2: attempt historical MIDAS reuse

There appears to be a public mirror of Housemarque Digital Audio System source code and license material on GitHub. That may be useful for reference or compatibility work, but it should be treated carefully:

- it is not an official vendor distribution channel
- its license is not a standard modern permissive open-source license
- it appears aimed at free/non-commercial usage rather than unrestricted reuse

For preservation, Path 1 is cleaner unless exact historical behavior becomes critical.

## Major Risks

### 1. Decompiled C will not be buildable by default

This is the standard trap in binary preservation. Ghidra can help recover intent, but humans still need to rebuild types, names, and structure.

### 2. x87-heavy code will need manual interpretation

The binary uses substantial x87 math. Scene code may decompile into awkward floating-point expressions that need cleanup and validation.

### 3. Legacy runtime assumptions

The original package assumes:

- 3dfx-flavored MiniGL
- legacy Windows behavior
- legacy MIDAS audio runtime

Those assumptions are preservation problems, but they are also cleanly isolatable problems.

### 4. Exact original source parity is unattainable

Behavioral fidelity is realistic. Source identity is not.

## Documentation Plan For Each Step

I recommend documenting the process as a sequence of small Markdown files, for example:

- `documentation/01-artifact-inventory.md`
- `documentation/02-pe-analysis.md`
- `documentation/03-imports-and-dependencies.md`
- `documentation/04-function-map.md`
- `documentation/05-rendering-path.md`
- `documentation/06-audio-path.md`
- `documentation/07-reconstruction-notes.md`
- `documentation/08-modern-build-plan.md`
- `documentation/09-validation-log.md`

Each file should record:

- date
- tools used
- exact binary or asset hash
- what was learned
- what remains uncertain
- next action

## Practical Conclusion

This demo is a good candidate for reconstruction.

If the goal is "recover something extremely close to the original source and eventually ship a modern Windows 10/11 rebuild", the project is realistic.

If the goal is "recover the exact original source code as it existed on the author machine in 1998", the project is not realistic from this binary alone.

The best preservation target is:

- annotated assembly where needed
- reconstructed C or C++ for the main program
- original assets kept unchanged
- a modernized but behaviorally faithful OpenGL + XM playback runtime

In short:

- close source reconstruction: feasible
- annotated assembly recovery: very feasible
- exact source recovery: not feasible
- Windows 10/11 rebuild from reconstructed source: feasible

## Suggested Tooling

Open-source first:

- Ghidra: https://github.com/NationalSecurityAgency/ghidra
- Cutter: https://cutter.re/
- Rizin: https://rizin.re/
- x64dbg: https://x64dbg.com/
- libopenmpt: https://lib.openmpt.org/libopenmpt/
- libxmp: https://xmp.sourceforge.net/

Reference material:

- Pouet production page: https://www.pouet.net/prod.php?which=5493
- 3dfx MiniGL context: https://3dfx.retropc.se/minigl.html
- Housemarque audio system mirror: https://github.com/retrodump/midas-Housemarque-Audio-System

## Local Evidence Used

Artifacts inspected locally on 2026-07-11:

- `original/cornball/PLANET.EXE`
- `original/cornball/OPENGL32.DLL`
- `original/cornball/MIDAS06.DLL`
- all bundled `.TGA` files
- `original/cornball/AAB.XM`
- `original/cornball/file_id.diz`
- `README.md`
