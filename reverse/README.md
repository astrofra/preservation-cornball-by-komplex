# Reverse Engineering Area

This directory is the working area for preservation and reverse engineering.

## Layout

- `baseline/`: hash-verified copy of the original release artifacts. Treat this as read-only.
- `work/`: place for derived artifacts such as Ghidra projects, notes, screenshots, renamed exports, and reconstructed source.

## Rules

- Do not modify files under `baseline/`.
- Keep `original/` untouched as the archival source of truth inside this repository.
- Put generated or edited reverse-engineering material under `work/`.
