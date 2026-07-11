# Cutter Project Area

Store Cutter project files and session outputs here.

Current reusable artifacts:

- `planet-pass-01.rzdb` once saved from the first scripted Rizin pass
- `planet_first_pass.rz` for replayable naming and label recovery
- `save-rizin-project.ps1` to rebuild `planet-pass-01.rzdb` from the binary

Recommended usage:

```powershell
pwsh reverse/work/tools/launch-cutter.ps1
```

If `pwsh` is not available, use:

```powershell
powershell -ExecutionPolicy Bypass -File reverse/work/tools/launch-cutter.ps1
```

To rebuild the saved Rizin database first:

```powershell
powershell -ExecutionPolicy Bypass -File reverse/work/tools/save-rizin-project.ps1
```
