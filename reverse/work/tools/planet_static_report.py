#!/usr/bin/env python3
from __future__ import annotations

import csv
import json
import pathlib
import re
import struct
from collections import Counter

import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32


ROOT = pathlib.Path(__file__).resolve().parents[3]
BIN_PATH = ROOT / "reverse" / "baseline" / "cornball" / "PLANET.EXE"
OUT_DIR = ROOT / "reverse" / "work" / "exports"


KNOWN_STRINGS = [
    b"Planet Cornball",
    b"aab.xm",
    b"logotaus.tga",
    b"logo.tga",
    b"txt2.tga",
    b"txt1.tga",
    b"v2.tga",
    b"v1.tga",
    b"s2.tga",
    b"s1.tga",
    b"kaar128.tga",
    b"fla.tga",
    b"surf128.tga",
    b"SetPixelFormat failed",
    b"ChoosePixelFormat failed",
]


def read_pe() -> pefile.PE:
    return pefile.PE(str(BIN_PATH), fast_load=False)


def ascii_strings(data: bytes, min_len: int = 4) -> list[str]:
    found = re.findall(rb"[ -~]{%d,}" % min_len, data)
    seen: set[str] = set()
    out: list[str] = []
    for item in found:
        text = item.decode("latin1", errors="ignore")
        if text not in seen:
            seen.add(text)
            out.append(text)
    return out


def import_rows(pe: pefile.PE) -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    for entry in getattr(pe, "DIRECTORY_ENTRY_IMPORT", []):
        dll = entry.dll.decode(errors="ignore")
        for imp in entry.imports:
            name = imp.name.decode(errors="ignore") if imp.name else f"ordinal_{imp.ordinal}"
            rows.append(
                {
                    "dll": dll,
                    "name": name,
                    "iat_va": f"0x{imp.address:08x}",
                }
            )
    return rows


def section_rows(pe: pefile.PE) -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    for sec in pe.sections:
        rows.append(
            {
                "name": sec.Name.decode(errors="ignore").rstrip("\x00"),
                "virtual_address": f"0x{sec.VirtualAddress:08x}",
                "virtual_size": f"0x{sec.Misc_VirtualSize:08x}",
                "raw_size": f"0x{sec.SizeOfRawData:08x}",
                "entropy": f"{sec.get_entropy():.3f}",
                "characteristics": f"0x{sec.Characteristics:08x}",
            }
        )
    return rows


def instruction_profile(pe: pefile.PE) -> dict[str, object]:
    text = next(sec for sec in pe.sections if sec.Name.startswith(b".text"))
    code = text.get_data()
    base = pe.OPTIONAL_HEADER.ImageBase + text.VirtualAddress
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    counts: Counter[str] = Counter()
    for insn in md.disasm(code, base):
        if insn.mnemonic.startswith("f"):
            counts["x87"] += 1
            counts[f"x87:{insn.mnemonic}"] += 1
        if insn.mnemonic == "call":
            counts["call"] += 1
        if insn.mnemonic.startswith("j"):
            counts["branch"] += 1
        if insn.mnemonic in {"mov", "push", "pop", "lea", "cmp", "test"}:
            counts[insn.mnemonic] += 1
    top_x87 = sorted(
        ((k[4:], v) for k, v in counts.items() if k.startswith("x87:")),
        key=lambda kv: (-kv[1], kv[0]),
    )[:15]
    return {
        "summary": {k: counts[k] for k in ["call", "branch", "x87", "mov", "push", "pop", "lea", "cmp", "test"]},
        "top_x87": top_x87,
    }


def find_string_xrefs(pe: pefile.PE, data: bytes, label: bytes) -> dict[str, object] | None:
    offset = data.find(label)
    if offset < 0:
        return None
    rva = pe.get_rva_from_offset(offset)
    va = pe.OPTIONAL_HEADER.ImageBase + rva
    needle = struct.pack("<I", va)
    xrefs: list[str] = []
    start = 0
    while True:
        hit = data.find(needle, start)
        if hit < 0:
            break
        try:
            hit_rva = pe.get_rva_from_offset(hit)
        except pefile.PEFormatError:
            start = hit + 1
            continue
        xrefs.append(f"0x{pe.OPTIONAL_HEADER.ImageBase + hit_rva:08x}")
        start = hit + 1
    return {
        "label": label.decode("latin1", errors="ignore"),
        "string_va": f"0x{va:08x}",
        "xrefs": xrefs,
    }


def candidate_function_map() -> list[dict[str, str]]:
    return [
        {
            "va": "0x00401090",
            "proposed_name": "configure_viewport_and_projection",
            "subsystem": "opengl_view",
            "confidence": "high",
            "evidence": "Calls glViewport, switches to GL_PROJECTION, loads identity, and calls gluPerspective with width/height-derived aspect ratio.",
        },
        {
            "va": "0x004010f0",
            "proposed_name": "initialize_gl_state_and_resources",
            "subsystem": "opengl_init",
            "confidence": "medium",
            "evidence": "Sets core GL state, seeds five texture IDs and uploads 256x256 RGBA blocks, configures fog/culling, then calls the viewport/projection helper.",
        },
        {
            "va": "0x004012e0",
            "proposed_name": "dispatch_scene_by_music_position",
            "subsystem": "render_dispatch",
            "confidence": "high",
            "evidence": "Selects scene by threshold table at 0x40e068 and dispatches through the switch table at 0x4013fc.",
        },
        {
            "va": "0x00401430",
            "proposed_name": "run_main_loop",
            "subsystem": "main_loop",
            "confidence": "high",
            "evidence": "Uses PeekMessageA/GetMessageA/TranslateMessage/DispatchMessageA, then renders and swaps buffers.",
        },
        {
            "va": "0x00401510",
            "proposed_name": "main",
            "subsystem": "startup",
            "confidence": "high",
            "evidence": "Creates the window, initializes MIDAS, shows the window, primes GL, then calls the main loop.",
        },
        {
            "va": "0x00401680",
            "proposed_name": "main_wndproc",
            "subsystem": "win32_init",
            "confidence": "high",
            "evidence": "Window procedure handling WM_CREATE, WM_DESTROY, WM_SIZE, WM_PAINT, WM_CLOSE, WM_KEYDOWN, and WM_MOUSEMOVE.",
        },
        {
            "va": "0x00401860",
            "proposed_name": "setup_pixel_format",
            "subsystem": "opengl_init",
            "confidence": "high",
            "evidence": "Calls ChoosePixelFormat and SetPixelFormat, references setup error strings.",
        },
        {
            "va": "0x00401930",
            "proposed_name": "render_scene_case_0_7",
            "subsystem": "scene_render",
            "confidence": "medium",
            "evidence": "Dispatched by cases 0 and 7 from the main scene switch.",
        },
        {
            "va": "0x00401c00",
            "proposed_name": "load_intro_textures",
            "subsystem": "assets_textures",
            "confidence": "high",
            "evidence": "Loads v1/v2/txt1/txt2/logo/logotaus TGAs and uploads them via glTexImage2D.",
        },
        {
            "va": "0x00401fc0",
            "proposed_name": "render_scene_case_2_4",
            "subsystem": "scene_render",
            "confidence": "medium",
            "evidence": "Dispatched by cases 2 and 4 from the main scene switch.",
        },
        {
            "va": "0x00402130",
            "proposed_name": "load_scene_s_textures",
            "subsystem": "assets_textures",
            "confidence": "high",
            "evidence": "Loads s1/s2/txt2 textures and uploads them via glTexImage2D.",
        },
        {
            "va": "0x00402330",
            "proposed_name": "render_scene_case_1_8",
            "subsystem": "scene_render",
            "confidence": "medium",
            "evidence": "Dispatched by cases 1 and 8 from the main scene switch.",
        },
        {
            "va": "0x004025c0",
            "proposed_name": "load_kaar_textures",
            "subsystem": "assets_textures",
            "confidence": "high",
            "evidence": "Loads kaar128/txt1/txt2 textures and uploads them via glTexImage2D.",
        },
        {
            "va": "0x004027c0",
            "proposed_name": "render_scene_case_5",
            "subsystem": "scene_render",
            "confidence": "low",
            "evidence": "Dedicated scene renderer dispatched only by case 5.",
        },
        {
            "va": "0x00402b40",
            "proposed_name": "load_fla_texture_group",
            "subsystem": "assets_textures",
            "confidence": "high",
            "evidence": "Zeros effect buffers, loads fla/txt1/logotaus textures, and uploads them.",
        },
        {
            "va": "0x00402d70",
            "proposed_name": "render_scene_case_3_6",
            "subsystem": "scene_render",
            "confidence": "low",
            "evidence": "Shared scene renderer for cases 3 and 6.",
        },
        {
            "va": "0x00403120",
            "proposed_name": "load_surf_texture_group",
            "subsystem": "assets_textures",
            "confidence": "high",
            "evidence": "Loads surf128/fla/txt1 textures and uploads them via glTexImage2D.",
        },
        {
            "va": "0x00403320",
            "proposed_name": "render_scene_case_9",
            "subsystem": "scene_render",
            "confidence": "medium",
            "evidence": "Final scene renderer dispatched only by case 9.",
        },
        {
            "va": "0x004036a0",
            "proposed_name": "ensure_texture_loaded",
            "subsystem": "assets_textures",
            "confidence": "high",
            "evidence": "Caches texture data by index and delegates to the raw TGA loader.",
        },
        {
            "va": "0x004036bb",
            "proposed_name": "load_tga_256x256_rgba",
            "subsystem": "assets_io",
            "confidence": "high",
            "evidence": "Reads 18-byte TGA header, allocates 0x40000 bytes, swaps BGR(A) to RGB(A).",
        },
        {
            "va": "0x00404014",
            "proposed_name": "midas_play_module",
            "subsystem": "audio_wrappers",
            "confidence": "high",
            "evidence": "Import thunk to MIDASplayModule.",
        },
        {
            "va": "0x0040401a",
            "proposed_name": "midas_load_module",
            "subsystem": "audio_wrappers",
            "confidence": "high",
            "evidence": "Import thunk to MIDASloadModule.",
        },
        {
            "va": "0x00404020",
            "proposed_name": "midas_start_background_play",
            "subsystem": "audio_wrappers",
            "confidence": "high",
            "evidence": "Import thunk to MIDASstartBackgroundPlay.",
        },
        {
            "va": "0x00404026",
            "proposed_name": "midas_init",
            "subsystem": "audio_wrappers",
            "confidence": "high",
            "evidence": "Import thunk to MIDASinit.",
        },
        {
            "va": "0x0040402c",
            "proposed_name": "midas_startup",
            "subsystem": "audio_wrappers",
            "confidence": "high",
            "evidence": "Import thunk to MIDASstartup.",
        },
        {
            "va": "0x004015dd",
            "proposed_name": "load_music_module",
            "subsystem": "audio_init",
            "confidence": "high",
            "evidence": "References aab.xm and MIDAS load/play wrappers in window init path.",
        },
        {
            "va": "0x004045c0",
            "proposed_name": "crt_startup_main",
            "subsystem": "startup",
            "confidence": "high",
            "evidence": "MSVC CRT startup path leading into the recovered main function.",
        },
    ]


def texture_slot_rows() -> list[dict[str, str]]:
    return [
        {"slot": "1", "texture_va": "0x005d754c", "asset": "v1.tga", "first_loader": "0x00401c00"},
        {"slot": "2", "texture_va": "0x005d7550", "asset": "v2.tga", "first_loader": "0x00401c00"},
        {"slot": "3", "texture_va": "0x005d7554", "asset": "txt1.tga", "first_loader": "0x00401c00"},
        {"slot": "4", "texture_va": "0x005d7558", "asset": "txt2.tga", "first_loader": "0x00401c00"},
        {"slot": "5", "texture_va": "0x005d755c", "asset": "logo.tga", "first_loader": "0x00401c00"},
        {"slot": "6", "texture_va": "0x005d7560", "asset": "logotaus.tga", "first_loader": "0x00401c00"},
        {"slot": "9", "texture_va": "0x005d756c", "asset": "kaar128.tga", "first_loader": "0x004025c0"},
        {"slot": "10", "texture_va": "0x005d7570", "asset": "surf128.tga", "first_loader": "0x00403120"},
        {"slot": "11", "texture_va": "0x005d7574", "asset": "fla.tga", "first_loader": "0x00402b40"},
        {"slot": "12", "texture_va": "0x005d7578", "asset": "s1.tga", "first_loader": "0x00402130"},
        {"slot": "13", "texture_va": "0x005d757c", "asset": "s2.tga", "first_loader": "0x00402130"},
    ]


def scene_dispatch_rows() -> list[dict[str, str]]:
    return [
        {"scene_index": "0", "threshold": "0x0000000c", "target_va": "0x00401930", "target_name": "render_scene_case_0_7"},
        {"scene_index": "1", "threshold": "0x00000012", "target_va": "0x00402330", "target_name": "render_scene_case_1_8"},
        {"scene_index": "2", "threshold": "0x00000016", "target_va": "0x00401fc0", "target_name": "render_scene_case_2_4"},
        {"scene_index": "3", "threshold": "0x00000019", "target_va": "0x00402d70", "target_name": "render_scene_case_3_6"},
        {"scene_index": "4", "threshold": "0x0000001b", "target_va": "0x00401fc0", "target_name": "render_scene_case_2_4"},
        {"scene_index": "5", "threshold": "0x0000001e", "target_va": "0x004027c0", "target_name": "render_scene_case_5"},
        {"scene_index": "6", "threshold": "0x00000022", "target_va": "0x00402d70", "target_name": "render_scene_case_3_6"},
        {"scene_index": "7", "threshold": "0x00000023", "target_va": "0x00401930", "target_name": "render_scene_case_0_7"},
        {"scene_index": "8", "threshold": "0x00000026", "target_va": "0x00402330", "target_name": "render_scene_case_1_8"},
        {"scene_index": "9", "threshold": "0x00000029", "target_va": "0x00403320", "target_name": "render_scene_case_9"},
    ]


def wndproc_message_rows() -> list[dict[str, str]]:
    return [
        {
            "message_name": "WM_CREATE",
            "message_code": "0x0001",
            "handler_va": "0x00401715",
            "action": "Acquire HDC, set pixel format, create/make current WGL context, initialize GL state/resources, then size the viewport from GetClientRect.",
        },
        {
            "message_name": "WM_DESTROY",
            "message_code": "0x0002",
            "handler_va": "0x004016da",
            "action": "Delete WGL context, release the window DC, and call PostQuitMessage(0).",
        },
        {
            "message_name": "WM_SIZE",
            "message_code": "0x0005",
            "handler_va": "0x004016ae",
            "action": "Call GetClientRect and forward right/bottom to configure_viewport_and_projection.",
        },
        {
            "message_name": "WM_PAINT",
            "message_code": "0x000f",
            "handler_va": "0x004017d1",
            "action": "Wrap a minimal BeginPaint/EndPaint pair without extra rendering work.",
        },
        {
            "message_name": "WM_CLOSE",
            "message_code": "0x0010",
            "handler_va": "0x00401809",
            "action": "Delete the WGL context, release the DC, clear GL globals, and destroy the window.",
        },
        {
            "message_name": "WM_KEYDOWN",
            "message_code": "0x0100",
            "handler_va": "0x004017f7",
            "action": "Call PostQuitMessage(0), providing the demo's immediate exit-on-key behavior.",
        },
        {
            "message_name": "WM_MOUSEMOVE",
            "message_code": "0x0200",
            "handler_va": "0x004017b7",
            "action": "Store LOWORD/HIWORD(lParam) into globals at 0x005d173c and 0x005d1740.",
        },
        {
            "message_name": "default",
            "message_code": "*",
            "handler_va": "0x00401797",
            "action": "Fallback to DefWindowProcA for messages outside the explicitly handled set.",
        },
    ]


def write_csv(path: pathlib.Path, rows: list[dict[str, object]], fieldnames: list[str]) -> None:
    with path.open("w", newline="", encoding="utf-8") as fh:
        writer = csv.DictWriter(fh, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)


def main() -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    pe = read_pe()
    data = BIN_PATH.read_bytes()

    metadata = {
        "path": str(BIN_PATH.relative_to(ROOT)).replace("\\", "/"),
        "sha256": __import__("hashlib").sha256(data).hexdigest(),
        "machine": f"0x{pe.FILE_HEADER.Machine:04x}",
        "timestamp": pe.FILE_HEADER.TimeDateStamp,
        "image_base": f"0x{pe.OPTIONAL_HEADER.ImageBase:08x}",
        "entry_point_va": f"0x{pe.OPTIONAL_HEADER.ImageBase + pe.OPTIONAL_HEADER.AddressOfEntryPoint:08x}",
        "major_linker_version": pe.OPTIONAL_HEADER.MajorLinkerVersion,
        "minor_linker_version": pe.OPTIONAL_HEADER.MinorLinkerVersion,
        "size_of_image": f"0x{pe.OPTIONAL_HEADER.SizeOfImage:08x}",
    }

    string_xrefs = [item for item in (find_string_xrefs(pe, data, label) for label in KNOWN_STRINGS) if item]
    strings = ascii_strings(data)

    (OUT_DIR / "planet-metadata.json").write_text(json.dumps(metadata, indent=2), encoding="utf-8")
    (OUT_DIR / "planet-sections.json").write_text(json.dumps(section_rows(pe), indent=2), encoding="utf-8")
    (OUT_DIR / "planet-imports.json").write_text(json.dumps(import_rows(pe), indent=2), encoding="utf-8")
    (OUT_DIR / "planet-string-xrefs.json").write_text(json.dumps(string_xrefs, indent=2), encoding="utf-8")
    (OUT_DIR / "planet-instruction-profile.json").write_text(
        json.dumps(instruction_profile(pe), indent=2),
        encoding="utf-8",
    )
    (OUT_DIR / "planet-strings.txt").write_text("\n".join(strings) + "\n", encoding="utf-8")

    function_map = candidate_function_map()
    write_csv(
        OUT_DIR / "planet-function-map.csv",
        function_map,
        ["va", "proposed_name", "subsystem", "confidence", "evidence"],
    )
    write_csv(
        OUT_DIR / "planet-texture-slots.csv",
        texture_slot_rows(),
        ["slot", "texture_va", "asset", "first_loader"],
    )
    write_csv(
        OUT_DIR / "planet-scene-dispatch.csv",
        scene_dispatch_rows(),
        ["scene_index", "threshold", "target_va", "target_name"],
    )
    write_csv(
        OUT_DIR / "planet-wndproc-messages.csv",
        wndproc_message_rows(),
        ["message_name", "message_code", "handler_va", "action"],
    )


if __name__ == "__main__":
    main()
