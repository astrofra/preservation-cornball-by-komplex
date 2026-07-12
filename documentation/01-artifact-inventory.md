# Step 1 - Artifact Inventory And Preservation Baseline

Date: 2026-07-11
Scope: `original/cornball` and `original/cornball.zip`

## Goal

Freeze the original release artifacts, create a separate reverse-engineering workspace, and record enough metadata to detect accidental drift later.

## Completed Actions

1. Left `original/` untouched.
2. Created a read-only baseline copy under `reverse/baseline/`.
3. Created a working area under `reverse/work/` for all derived artifacts.
4. Computed SHA-256 hashes for the zip and every unpacked file.
5. Verified that every file in `reverse/baseline/` is hash-identical to its counterpart in `original/`.
6. Stored the machine-readable checksum manifest in [reverse/baseline/SHA256SUMS.txt](/C:/works/projects/preservation-cornball-by-komplex/reverse/baseline/SHA256SUMS.txt).

## Baseline Layout

- Original archival source inside the repo:
  - `original/cornball.zip`
  - `original/cornball/`
- Read-only working copy:
  - `reverse/baseline/cornball.zip`
  - `reverse/baseline/cornball/`
- Derived reverse-engineering outputs:
  - `reverse/work/`

## Inventory Summary

- `1` zip archive
- `19` unpacked files
- Unpacked payload size: `3,861,515` bytes
- Hash algorithm: `SHA-256`

File classes:

- `1` executable: `PLANET.EXE`
- `2` DLLs: `MIDAS06.DLL`, `OPENGL32.DLL`
- `1` XM module: `AAB.XM`
- `11` TGAs
- `3` NFO files
- `1` DIZ file

## Observations

- `original/cornball.zip` has a repository/import timestamp of `2026-07-11T17:21:08Z`. It should not be treated as a historical build timestamp.
- The unpacked files preserve historical timestamps from `1997` and `1998`, which are more useful for chronology.
- `Takeover.nfo` and `TO98.nfo` are byte-identical and share the same SHA-256 hash.
- This baseline is suitable for later integrity checks before and after reverse-engineering work.

## Full Manifest

| Path | Size | Last Write UTC | SHA-256 |
| --- | ---: | --- | --- |
| `original/cornball.zip` | 1334519 | `2026-07-11T17:21:08Z` | `4d2b65e7051477faced24a7c73fd14cd2ecb2093a9a24522133730d13e29b0c2` |
| `original/cornball/AAB.XM` | 667087 | `1997-11-03T23:13:58Z` | `0f07f57ac079b84de526c5b40594920098d41a2343c042186ddfdc9c98e195b3` |
| `original/cornball/file_id.diz` | 36 | `1998-04-08T09:19:02Z` | `de5fb6138d13b0167f3d3b7335d1498065c951e09d9dcc7bca8370960f8af7bd` |
| `original/cornball/FLA.TGA` | 263707 | `1998-03-03T02:27:28Z` | `ceb38ea84e2c04645808bd9391f53b389d0444dc111bca4439ec8fd48b0b8872` |
| `original/cornball/KAAR128.TGA` | 263707 | `1998-03-03T03:32:46Z` | `a55158ca14f2569a56ae3246990af950b9f7c5cd4271c7646ea3088b0c63c551` |
| `original/cornball/LOGO.TGA` | 263707 | `1998-03-01T21:24:38Z` | `9b895480e158137c23def574e4c183f84a53440029062f7b67abd3fe3b1d6cd5` |
| `original/cornball/LOGOTAUS.TGA` | 263707 | `1998-03-01T21:24:34Z` | `d3c18ac6f7c192ba05a63d9556d371e97ba86d7b3968de4e880eba4e88253755` |
| `original/cornball/MIDAS06.DLL` | 96768 | `1997-01-26T20:49:54Z` | `b55b4c7c37b3b10ffe02a47a50feb8e4062eb1cc5c71fe122f2f79c03ef18fd9` |
| `original/cornball/OPENGL32.DLL` | 126464 | `1997-09-26T16:16:02Z` | `20af9538b59fba2e2956756c6be5131c24a6f462f42811ed9327c1ff56026f3e` |
| `original/cornball/PLANET.EXE` | 66048 | `1998-04-08T09:16:32Z` | `c6bf4cf57251a16fa2a5664e9809d16558b12365c18bda53777d03ac00d3c86a` |
| `original/cornball/S1.TGA` | 263707 | `1998-03-03T03:00:30Z` | `5b625080f6e9d4e55e1c703ca63104bd031bff0a84ec215fd4bf5b208e1af978` |
| `original/cornball/S2.TGA` | 263707 | `1998-03-03T03:00:26Z` | `107f04b51c4403293235a1ac5f8cf7e0eb84d2a628ee79789ac70fbff73749ea` |
| `original/cornball/Skynet.nfo` | 2091 | `1998-02-22T20:39:50Z` | `a75c02a0d0c53776c0c06e9acd168cd9942fe54f6a238344d7bada4e43e75d15` |
| `original/cornball/SURF128.TGA` | 263707 | `1998-03-03T03:31:42Z` | `42e9dbea4f02028f88aece4dcd5b3d84d1b704566146c0b755945f767a0b0f8c` |
| `original/cornball/Takeover.nfo` | 1122 | `1998-04-12T22:24:20Z` | `bd3e59bf5d0bb4341eb32cf2306b5125ec5b0217bb558cf3c85a227a0d9c2aba` |
| `original/cornball/TO98.nfo` | 1122 | `1998-04-12T21:48:06Z` | `bd3e59bf5d0bb4341eb32cf2306b5125ec5b0217bb558cf3c85a227a0d9c2aba` |
| `original/cornball/TXT1.TGA` | 263707 | `1998-03-01T16:45:24Z` | `62e3e5e689b9c2a3cbb33413239904c3fe78f462575fc6f1b8ad2c2d9892ca98` |
| `original/cornball/TXT2.TGA` | 263707 | `1998-03-01T19:23:20Z` | `bd1af0878d3c2f62669b032d1d125ecee5498581e07572a17f93c8b283b6c503` |
| `original/cornball/V1.TGA` | 263707 | `1998-03-03T00:32:22Z` | `b96e287fb1de3014123e96e721876e4271e6384a147a4470f74064b054d5b36e` |
| `original/cornball/V2.TGA` | 263707 | `1998-03-03T00:32:24Z` | `cda9d8f5826b464376948669547544438988e3d9d8f3917c10971d0b3080fdae` |

## Verification

All `20` baseline artifacts matched their original counterparts by SHA-256.

## Next Step

Proceed with static reverse engineering from the read-only baseline copy and store all generated material under `reverse/work/`.

## Addendum - Supplementary Video Capture

Date: 2026-07-12

A supplementary MPEG capture was identified later at `original/video/TG98_3DFX_Complex_PlanetCornball.mpeg`.

- It was not part of the original Step 1 hash baseline.
- It is treated as an external reference artifact rather than part of the release payload in `original/cornball/`.
- Its crop-box analysis, timing convention, and derived PNG sequence are documented in [documentation/04-video-reference.md](04-video-reference.md).
