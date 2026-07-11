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
            "va": "0x00401510",
            "proposed_name": "create_main_window",
            "subsystem": "win32_init",
            "confidence": "high",
            "evidence": "Uses Planet Cornball string, RegisterClassA, CreateWindowExA, ShowWindow.",
        },
        {
            "va": "0x00401880",
            "proposed_name": "setup_pixel_format",
            "subsystem": "opengl_init",
            "confidence": "high",
            "evidence": "Calls ChoosePixelFormat and SetPixelFormat, references setup error strings.",
        },
        {
            "va": "0x00401930",
            "proposed_name": "render_scene_or_frame",
            "subsystem": "render_loop",
            "confidence": "medium",
            "evidence": "Starts with GL state setup and x87 transforms; likely a frame render path.",
        },
        {
            "va": "0x00401c10",
            "proposed_name": "load_intro_textures",
            "subsystem": "assets_textures",
            "confidence": "high",
            "evidence": "Loads v1/v2/txt1/txt2/logo/logotaus TGAs and uploads them via glTexImage2D.",
        },
        {
            "va": "0x00402140",
            "proposed_name": "load_scene_s_textures",
            "subsystem": "assets_textures",
            "confidence": "high",
            "evidence": "Loads s1/s2/txt2 textures and uploads them via glTexImage2D.",
        },
        {
            "va": "0x004025d0",
            "proposed_name": "load_kaar_textures",
            "subsystem": "assets_textures",
            "confidence": "high",
            "evidence": "Loads kaar128/txt1/txt2 textures and uploads them via glTexImage2D.",
        },
        {
            "va": "0x00402b70",
            "proposed_name": "load_fla_texture_group",
            "subsystem": "assets_textures",
            "confidence": "medium",
            "evidence": "References fla.tga and intro texture IDs; likely a grouped texture setup routine.",
        },
        {
            "va": "0x00403130",
            "proposed_name": "load_surf_texture_group",
            "subsystem": "assets_textures",
            "confidence": "high",
            "evidence": "Loads surf128/fla/txt1 textures and uploads them via glTexImage2D.",
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
            "va": "0x004045c0",
            "proposed_name": "crt_startup_main",
            "subsystem": "startup",
            "confidence": "high",
            "evidence": "MSVC CRT startup path leading into program entry.",
        },
        {
            "va": "0x004015dd",
            "proposed_name": "load_music_module",
            "subsystem": "audio_init",
            "confidence": "high",
            "evidence": "References aab.xm and MIDAS load/play wrappers in window init path.",
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


if __name__ == "__main__":
    main()
