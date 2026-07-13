# Global Demo Script

This note consolidates the recovered top-level script of `PLANET.EXE`, with priority given to the disassembly over the current fallback rendering state.

Primary evidence:

- dispatcher at `0x004012e0`
- threshold table at `0x0040e068`
- jump table at `0x004013fc`
- export [planet-scene-dispatch.csv](/C:/works/projects/preservation-cornball-by-komplex/reverse/work/exports/planet-scene-dispatch.csv)

## Dispatcher Contract

The original dispatcher reads `MIDASplayStatus.position` and selects the first slot whose threshold is strictly greater than the current position.

That yields this exact slot ownership:

- scene `0` for positions `0 .. 11`
- scene `1` starting at `12`
- scene `9` for positions `38 .. 40`
- scripted playback end at position `41` and above

On scene change, the binary:

- copies the previous scene index
- refreshes `g_scene_start_tick_ms` from `GetTickCount()`
- updates `g_current_scene_index`

It also recomputes every frame:

- `g_demo_elapsed_seconds = (now_ms - g_demo_start_tick_ms) * 0.001`
- `g_scene_elapsed_seconds = (now_ms - g_scene_start_tick_ms) * 0.001`

So the recovered model is:

- tracker position drives scene selection
- wall-clock time since the last scene change drives most intra-scene animation

## Recovered Slot Table

| Scene | Start | End threshold | Duration | Family | Target |
| --- | ---: | ---: | ---: | --- | --- |
| `0` | `0` | `12` | `12` | `intro` | `0x00401930` |
| `1` | `12` | `18` | `6` | `kaar` | `0x00402330` |
| `2` | `18` | `22` | `4` | `s-pair` | `0x00401fc0` |
| `3` | `22` | `25` | `3` | `surf` | `0x00402d70` |
| `4` | `25` | `27` | `2` | `s-pair` | `0x00401fc0` |
| `5` | `27` | `30` | `3` | `fla` | `0x004027c0` |
| `6` | `30` | `34` | `4` | `surf` | `0x00402d70` |
| `7` | `34` | `35` | `1` | `intro` | `0x00401930` |
| `8` | `35` | `38` | `3` | `kaar` | `0x00402330` |
| `9` | `38` | `41` | `3` | `finale` | `0x00403320` |

## Reconstruction Status

The replay host now preserves this script exactly at the selection layer.

Current status by family:

- `intro`: lifted and rendered
- `kaar`: lifted and rendered
- `s-pair`: lifted and rendered in current reconstruction form
- `surf`: lifted and rendered
- `fla`: lifted and rendered
- `finale`: not lifted yet, currently rendered as a black fallback frame

Important distinction:

- `finale` is an original disassembly-backed family identity
- black fallback is only the present reconstruction behavior for missing renderers

## Replay Host Implication

The replay host now applies the recovered script in two modes:

- synthetic mode: slot selection from `elapsed_seconds / position_seconds`
- music mode: slot selection directly from `MIDASgetPlayStatus.position`

The music-driven path now keeps tracker position authoritative for the whole frame step. It no longer reselects the active scene from the synthetic wall-clock path after polling MIDAS.
