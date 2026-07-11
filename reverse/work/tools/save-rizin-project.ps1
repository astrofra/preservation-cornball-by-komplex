$ErrorActionPreference = "Stop"

$root = Resolve-Path (Join-Path $PSScriptRoot "..\\..\\..")
$binary = Join-Path $root "reverse\\baseline\\cornball\\PLANET.EXE"
$scriptPath = Join-Path $root "reverse\\work\\tools\\planet_first_pass.rz"
$projectDir = Join-Path $root "reverse\\work\\projects\\cutter"
$projectPath = Join-Path $projectDir "planet-pass-01.rzdb"
$packageRoot = Join-Path $env:LOCALAPPDATA "Microsoft\\WinGet\\Packages"

$candidate = Get-ChildItem -Path $packageRoot -Directory -Filter "Rizin.Cutter_*" |
    Sort-Object LastWriteTime -Descending |
    Select-Object -First 1

if (-not $candidate) {
    throw "Cutter package directory was not found under $packageRoot"
}

$rizinExe = Get-ChildItem -Path $candidate.FullName -Recurse -Filter "rizin.exe" |
    Select-Object -First 1 -ExpandProperty FullName

if (-not $rizinExe) {
    throw "rizin.exe was not found inside $($candidate.FullName)"
}

New-Item -ItemType Directory -Force -Path $projectDir | Out-Null

$commands = Get-Content $scriptPath |
    ForEach-Object { $_.Trim() } |
    Where-Object { $_ -and -not $_.StartsWith("#") }

$commandText = (($commands -join ";") + ";Ps " + $projectPath.Replace("\", "\\"))

Write-Output "Saving Rizin project:"
Write-Output "  EXE: $rizinExe"
Write-Output "  BIN: $binary"
Write-Output "  RZ : $scriptPath"
Write-Output "  DB : $projectPath"

& $rizinExe -Aqc $commandText $binary
