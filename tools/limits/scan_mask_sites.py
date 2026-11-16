#!/usr/bin/env python3
"""Static mask-site inventory for the 32->64/128 mask-widening work.

Scans haloreach-copy.dll for DWORD mask immediates (contiguous low-bit masks such as 0x1FFFF / 0xFFFF / 0xFFFFFFFF)
in the AND/TEST/MOV/OR/XOR forms the peer-valid and player-valid masks use, classifies the sites already covered by
kPeerMasks32 (SlotPatch.cpp) and writes a Markdown inventory. Heuristic byte scan, not a full disassembly: every hit
must be checked by hand before it is patched.

Usage: python tools/limits/scan_mask_sites.py [--dll PATH] [--out PATH]
"""
import argparse, re, struct
from pathlib import Path

DEFAULT_DLL = Path(r"C:\Users\tyler\Documents\deepseek-harness\default-workspace\haloreach-copy.dll")
REPO = Path(__file__).resolve().parents[2]

# Session/player regions (RVAs from the engine reference docs); used only to rank candidates, not to classify them.
REGIONS = [
    (0x038D000, 0x0393000, "0x38D-393 join/peer-mask code"),
    (0x039C000, 0x03A2000, "0x39C-3A2 player add/update + membership handlers"),
    (0x03AB000, 0x03AC000, "0x3AB game player event"),
    (0x03C1000, 0x03C8000, "0x3C1-3C8 peer masks + game player event encoder"),
    (0x0403000, 0x040A000, "0x403-40A player-mask scans / peer masks"),
    (0x0411000, 0x0415000, "0x411-415 local player count, peer scans"),
    (0x044B000, 0x0451000, "0x44B-451 membership message + session checks"),
    (0x0133000, 0x0134000, "0x133 team count / game-over check"),
]

def mask_like(imm):
    if imm == 0xFFFFFFFF:
        return True
    return imm >= 0x7FFF and (imm & (imm + 1)) == 0

def load_text(path):
    data = path.read_bytes()
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    nsec = struct.unpack_from("<H", data, pe + 6)[0]
    opt = struct.unpack_from("<H", data, pe + 20)[0]
    secs = []
    for i in range(nsec):
        off = pe + 24 + opt + i * 40
        name, vsize, va, rsize, raw = struct.unpack_from("<8sIIII", data, off)
        secs.append((name.split(b"\0")[0].decode(errors="replace"), vsize, va, rsize, raw))
    for name, vsize, va, rsize, raw in secs:
        if name == ".text":
            return data, va, raw, rsize
    raise SystemExit("no .text section")

def known_entries():
    """(rva, original_bytes) for kPeerMasks32 entries: the catalogued 32-player mask/bound patches."""
    text = (REPO / "src" / "host" / "SlotPatch.cpp").read_text(encoding="utf-8", errors="replace")
    m = re.search(r"const Bound kPeerMasks32\[\] = \{(.*?)\n\};", text, re.S)
    rows = []
    if m:
        for mm in re.finditer(r"^\s*\{\s*0x([0-9A-Fa-f]+),\s*(\d+),\s*\{([^}]*)\}", m.group(1), re.M):
            raw = bytes(int(x, 16) for x in re.findall(r"0x([0-9A-Fa-f]{2})", mm.group(3)))
            rows.append((int(mm.group(1), 16), raw))
    return rows

def scan(data, text_va, text_raw, text_size):
    hits = []
    end = text_raw + text_size
    def emit(rva, kind, imm, size):
        if mask_like(imm):
            hits.append({"rva": rva, "kind": kind, "imm": imm, "size": size})
    i = text_raw
    while i < end - 8:
        b = data[i]
        rva = text_va + (i - text_raw)
        if b == 0xA9:
            emit(rva, "test eax,imm32", struct.unpack_from("<I", data, i + 1)[0], 5)
        elif b == 0x25:
            emit(rva, "and eax,imm32", struct.unpack_from("<I", data, i + 1)[0], 5)
        elif 0xB8 <= b <= 0xBF:
            emit(rva, "mov r32,imm32", struct.unpack_from("<I", data, i + 1)[0], 5)
        elif b == 0x81:
            modrm = data[i + 1]
            if ((modrm >> 3) & 7) in (1, 4, 5, 6):
                emit(rva, "and/or/xor r/m32,imm32", struct.unpack_from("<I", data, i + 2)[0], 6)
        elif b == 0xF7:
            modrm = data[i + 1]
            if ((modrm >> 3) & 7) == 0 and modrm >= 0xC0:
                emit(rva, "test r/m32,imm32", struct.unpack_from("<I", data, i + 2)[0], 6)
        elif b == 0x41:
            if data[i + 1] == 0x81 and ((data[i + 2] >> 3) & 7) in (1, 4, 5, 6):
                emit(rva, "REX and/or/xor r/m32,imm32", struct.unpack_from("<I", data, i + 3)[0], 7)
            elif data[i + 1] == 0xF7 and ((data[i + 2] >> 3) & 7) == 0 and data[i + 2] >= 0xC0:
                emit(rva, "REX test r/m32,imm32", struct.unpack_from("<I", data, i + 3)[0], 7)
        elif b == 0x66 and data[i + 1] == 0x85 and (data[i + 2] & 0xC0) == 0xC0:
            emit(rva, "test r16,r16", 0xFFFF, 3)
        i += 1
    seen = {}
    for h in hits:
        seen.setdefault(h["rva"], h)
    return sorted(seen.values(), key=lambda h: h["rva"])

def region_of(rva):
    for lo, hi, label in REGIONS:
        if lo <= rva < hi:
            return label
    return ""

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--dll", type=Path, default=DEFAULT_DLL)
    ap.add_argument("--out", type=Path, default=REPO / "docs" / "02-engine-reference" / "MASK-SITE-INVENTORY.md")
    args = ap.parse_args()
    data, text_va, text_raw, text_size = load_text(args.dll)
    known = known_entries()
    hits = scan(data, text_va, text_raw, text_size)
    # A hit is "already catalogued" when its immediate lies inside a kPeerMasks32 patch (the patch RVA is the
    # instruction start, the mask immediate a few bytes later).
    known_cover = [(r, r + len(raw)) for r, raw in known]
    def is_known(rva):
        return any(lo <= rva < hi for lo, hi in known_cover)
    known_rvas = {h["rva"] for h in hits if is_known(h["rva"])}
    in_region = [h for h in hits if not is_known(h["rva"]) and region_of(h["rva"])]
    rest = [h for h in hits if not is_known(h["rva"]) and not region_of(h["rva"])]
    by_imm = {}
    for h in hits:
        by_imm[h["imm"]] = by_imm.get(h["imm"], 0) + 1
    L = []
    L.append("# Mask-site inventory (static scan of haloreach-copy.dll)")
    L.append("")
    L.append("Generated by `python tools/limits/scan_mask_sites.py`. Starting point for the multi-word mask widening in")
    L.append("`TOPIC-scaling-64-128.md` section 3.1: the engine peer-valid/player-valid masks are 32-bit DWORDs, so")
    L.append("capacities above 32 need every mask read/write/bit-scan site widened.")
    L.append("")
    L.append("**Heuristic byte scan, not a disassembly.** Hits are candidate AND/TEST/MOV/OR/XOR immediates that are")
    L.append("contiguous low-bit masks (0x7FFF and above) plus the stock 16-bit test r16,r16 form. Every candidate must be")
    L.append("checked by hand before patching.")
    L.append("")
    L.append("## Summary")
    L.append("")
    L.append("- mask-like immediate sites found in `.text`: **{}**".format(len(hits)))
    L.append("- catalogued kPeerMasks32 entries: **{}** ({} are mask-immediate patches; the rest are bounds/shift/-".format(len(known), sum(1 for r, raw in known if r in {h["rva"] for h in hits})))
    L.append("  field-width patches outside this scan)".format(len(known), sum(1 for r, raw in known if r in {h["rva"] for h in hits})))
    L.append("- candidates inside the known session/player regions (below): **{}**".format(len(in_region)))
    L.append("- candidates elsewhere (object/team/effect masks, mostly stay 32-bit): **{}**".format(len(rest)))
    L.append("")
    L.append("| mask immediate | sites |")
    L.append("|---|---|")
    for imm, n in sorted(by_imm.items(), key=lambda kv: -kv[1])[:15]:
        L.append("| 0x{:X} | {} |".format(imm, n))
    L.append("")
    L.append("## Candidates inside session/player code (classify first)")
    L.append("")
    if in_region:
        L.append("| RVA | region | kind | immediate |")
        L.append("|---|---|---|---|")
        for h in in_region:
            L.append("| 0x{:X} | {} | {} | 0x{:X} |".format(h["rva"], region_of(h["rva"]), h["kind"], h["imm"]))
    else:
        L.append("(none)")
    L.append("")
    L.append("## Already-catalogued peer-mask operands found by the scan")
    L.append("")
    L.append("| RVA | kind | immediate |")
    L.append("|---|---|---|")
    for h in hits:
        if h["rva"] in known_rvas:
            L.append("| 0x{:X} | {} | 0x{:X} |".format(h["rva"], h["kind"], h["imm"]))
    L.append("")
    L.append("## Next steps")
    L.append("")
    L.append("1. Classify the candidates above by reading the surrounding function (player-valid mask at block +0x170C,")
    L.append("   peer mask at session +0x5C/+0x40, team mask in game-over loops; object/team masks stay 32-bit).")
    L.append("2. Choose a patch form per confirmed site from `TOPIC-scaling-64-128.md` section 3.1 (high-word branch,")
    L.append("   split store, word-iterating bit scan) and validate the emitted bytes with the headless self-test.")
    L.append("3. Widen the membership encoder/decoder mask fields with the sites, so peers/players above bit 31 never")
    L.append("   reach the wire with a truncated mask.")
    L.append("")
    args.out.write_text("\n".join(L) + "\n", encoding="utf-8")
    print("scanned {} mask-like sites; {} inside session/player regions; {} elsewhere; {}/{} catalogued mask-immediate patched found; wrote {}".format(
        len(hits), len(in_region), len(rest), sum(1 for r, raw in known if r in {h["rva"] for h in hits}), len(known), args.out))

if __name__ == "__main__":
    main()