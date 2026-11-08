#!/usr/bin/env python3
"""Compile an engine-limit recipe into Noble's compact .nlp patch file.

An .nlp file holds only the differences between stock haloreach.dll and a patched build: a list of
overwrite rows (each with the stock bytes it expects and the bytes to write) plus the block appended
at the original end of file. Noble applies it at runtime to the player's own stock DLL and writes
the result to its own folder (see src/host/EngineLimits.cpp). The player's install is never changed.

Format NLP1 (all integers little-endian):
    char[4]  magic "NLP1"
    u32      format version (1)
    u32      profile id (1 = V16)
    u32      row count
    u32      stock length          u8[32] stock SHA-256
    u32      output length         u8[32] output SHA-256
    u32      append offset (== stock length)
    u32      append length
    rows     u32 offset, u32 length, u8[length] stock bytes, u8[length] new bytes   (sorted, no overlap)
    u8[append length] append block

Usage:
    python build_recipe.py v16 [--recipes tools/limits/recipes] [--out data/limits/v16.nlp]
                               [--verify-stock path\\to\\stock\\haloreach.dll]
                               [--verify-output path\\to\\patched\\haloreach.dll]

--verify-output alone also works: it rebuilds stock from a patched DLL using the rows' stock bytes and
checks both hashes, which proves the recipe round-trips without needing a stock copy.
"""
from __future__ import annotations

import argparse
import csv
import hashlib
import json
import struct
import sys
from pathlib import Path

MAGIC = b"NLP1"
FORMAT_VERSION = 1
PROFILES = {"v16": 1}


def load_v16(recipes: Path):
    d = recipes / "v16"
    layout = json.loads((d / "BYTE_LAYOUT.json").read_text(encoding="utf-8"))
    rows = []
    with open(d / "ALL_OVERWRITES.csv", encoding="utf-8-sig", newline="") as f:
        for r in csv.DictReader(f):
            off = int(r["File offset"], 16)
            stock = bytes.fromhex(r["Stock bytes"])
            new = bytes.fromhex(r["V16 bytes"])
            if len(stock) != len(new) or len(stock) != int(r["Byte count"]):
                raise ValueError(f"row 0x{off:X}: byte counts disagree")
            rows.append((off, stock, new))
    append = (d / "APPEND_AT_ORIGINAL_EOF.bin").read_bytes()
    meta = {
        "stock_len": layout["stock_file_bytes"],
        "stock_sha": bytes.fromhex(layout["stock_sha256"]),
        "out_len": layout["v16_file_bytes"],
        "out_sha": bytes.fromhex(layout["v16_sha256"]),
        "append_off": int(layout["append_file_offset"], 16),
        "append_sha": layout["append_sha256"],
    }
    if hashlib.sha256(append).hexdigest() != meta["append_sha"]:
        raise ValueError("APPEND_AT_ORIGINAL_EOF.bin hash does not match BYTE_LAYOUT.json")
    if len(rows) != layout["overwrite_rows"]:
        raise ValueError(f"expected {layout['overwrite_rows']} rows, found {len(rows)}")
    return rows, append, meta


def validate(rows, append, meta):
    rows.sort(key=lambda r: r[0])
    end = 0
    for off, stock, new in rows:
        if off < end:
            raise ValueError(f"row 0x{off:X} overlaps the previous row")
        if off + len(stock) > meta["stock_len"]:
            raise ValueError(f"row 0x{off:X} runs past the stock file")
        end = off + len(stock)
    if meta["append_off"] != meta["stock_len"]:
        raise ValueError("append must start at the stock end of file")
    if meta["stock_len"] + len(append) != meta["out_len"]:
        raise ValueError("stock length + append length != output length")


def apply(stock: bytes, rows, append) -> bytes:
    out = bytearray(stock)
    for off, old, new in rows:
        if out[off:off + len(old)] != old:
            raise ValueError(f"stock bytes differ at 0x{off:X}")
        out[off:off + len(new)] = new
    return bytes(out) + append


def unapply(patched: bytes, rows, meta) -> bytes:
    out = bytearray(patched[:meta["stock_len"]])
    for off, old, new in rows:
        if out[off:off + len(new)] != new:
            raise ValueError(f"patched bytes differ at 0x{off:X}")
        out[off:off + len(old)] = old
    return bytes(out)


def write_nlp(path: Path, profile_id: int, rows, append, meta):
    blob = bytearray()
    blob += MAGIC
    blob += struct.pack("<III", FORMAT_VERSION, profile_id, len(rows))
    blob += struct.pack("<I", meta["stock_len"]) + meta["stock_sha"]
    blob += struct.pack("<I", meta["out_len"]) + meta["out_sha"]
    blob += struct.pack("<II", meta["append_off"], len(append))
    for off, old, new in rows:
        blob += struct.pack("<II", off, len(old)) + old + new
    blob += append
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(blob)
    return len(blob)


def main() -> int:
    repo = Path(__file__).resolve().parents[2]
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("profile", choices=sorted(PROFILES))
    ap.add_argument("--recipes", type=Path, default=repo / "tools" / "limits" / "recipes")
    ap.add_argument("--out", type=Path, default=None)
    ap.add_argument("--verify-stock", type=Path, default=None)
    ap.add_argument("--verify-output", type=Path, default=None)
    a = ap.parse_args()

    rows, append, meta = load_v16(a.recipes)
    validate(rows, append, meta)
    out_path = a.out or (repo / "data" / "limits" / f"{a.profile}.nlp")
    size = write_nlp(out_path, PROFILES[a.profile], rows, append, meta)
    print(f"wrote {out_path} ({size:,} bytes, {len(rows)} rows, {len(append):,}-byte append)")

    if a.verify_output:
        patched = a.verify_output.read_bytes()
        if hashlib.sha256(patched).digest() != meta["out_sha"]:
            print("verify-output: patched DLL hash does not match the recipe", file=sys.stderr)
            return 1
        stock = unapply(patched, rows, meta)
        if hashlib.sha256(stock).digest() != meta["stock_sha"]:
            print("verify-output: rebuilt stock hash does not match", file=sys.stderr)
            return 1
        if apply(stock, rows, append) != patched:
            print("verify-output: forward apply did not reproduce the patched DLL", file=sys.stderr)
            return 1
        print("verify-output: OK (patched -> stock -> patched round trip, both hashes match)")
    if a.verify_stock:
        stock = a.verify_stock.read_bytes()
        if hashlib.sha256(stock).digest() != meta["stock_sha"]:
            print("verify-stock: this is not the stock DLL the recipe was made for", file=sys.stderr)
            return 1
        if hashlib.sha256(apply(stock, rows, append)).digest() != meta["out_sha"]:
            print("verify-stock: output hash mismatch", file=sys.stderr)
            return 1
        print("verify-stock: OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
