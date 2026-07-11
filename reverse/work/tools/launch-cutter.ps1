$ErrorActionPreference = "Stop"

$root = Resolve-Path (Join-Path $PSScriptRoot "..\\..\\..")
$binary = Join-Path $root "reverse\\baseline\\cornball\\PLANET.EXE"
$projectDir = Join-Path $root "reverse\\work\\projects\\cutter"
$packageRoot = Join-Path $env:LOCALAPPDATA "Microsoft\\WinGet\\Packages"

$candidate = Get-ChildItem -Path $packageRoot -Directory -Filter "Rizin.Cutter_*" |
    Sort-Object LastWriteTime -Descending |
    Select-Object -First 1

if (-not $candidate) {
    throw "Cutter package directory was not found under $packageRoot"
}

$cutterExe = Get-ChildItem -Path $candidate.FullName -Recurse -Filter "cutter.exe" |
    Select-Object -First 1 -ExpandProperty FullName

if (-not $cutterExe) {
    throw "cutter.exe was not found inside $($candidate.FullName)"
}

New-Item -ItemType Directory -Force -Path $projectDir | Out-Null

Write-Output "Launching Cutter:"
Write-Output "  EXE: $cutterExe"
Write-Output "  BIN: $binary"
Write-Output "  DIR: $projectDir"

Start-Process -FilePath $cutterExe -ArgumentList @($binary) -WorkingDirectory $projectDir
