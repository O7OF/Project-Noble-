# Plan: 64-player lobbies and matches (2026-11-19)

Tyler restarted the >32 work on 2026-11-19 (goal: lobbies and matches that work at 64, built to be more stable and
better optimised than Reach Renaissance; real 64-player tests come later). Clean-room: everything below comes from our
own MCC decompile, the Xbox 360 symbol names and our own measurements. RR is used for ideas only; when a value matches
theirs it is because the engine dictates it, and our notes show how we found it.

Builds on `TOPIC-scaling-64-128.md` (layouts, chunked shadow model, mask storage model, message sizes) and the
committed 32-player `SlotPatch`.

## 1. What blocks 64 (inventory 2026-11-19)

| limit | where | size of the job |
|---|---|---|
| Peer-valid mask (32 bits) | session +0x5C | **100 instructions in 52 functions** (94 reads, 3 tests, 3 writes) — `MASK-ACCESS-INVENTORY-2026-11-19.md` |
| Player-valid mask (32 bits) | player block +0x170C (A and B blocks via block-relative pointers) | **23 instructions in 15 functions** (21 reads, 2 writes) |
| Stream/bandwidth manager | per-machine arrays of 34 entries (stride 0x124), loops `< 0x22`, already 64-bit masks | arrays must grow to 64+ machines; masks are already 64-bit (MCC widened them) |
| Session arrays | peers / players / peer table | chunked shadow layout for 64 is modelled and self-tested (`-test-slot-capacity`) |
| Membership message | 0xA300 at 32 | 0xEA00 at 64 — still fits the 16-bit length |
| Wire index fields | peer 6 bits (patched), player 5 bits (patched) | peer 6 bits is exactly 64; player needs 6 bits |
| Loop bounds | 139 bounds widened to 32 today | same tables, 64 |
| Pools | players / fire team / simulation views at 32 | raise to 64; measure memory |
| Host upload | was capped at 10 Mbit/s by the engine | **done 2026-11-19**: ceiling 100 Mbit/s, caps follow the engine's own % rule, `hostUploadKbps` floor for dedicated servers |

Tools (re-run after any MCC update): `tools/limits/scan_mask_access.py` (this inventory), `tools/limits/scan_mask_sites.py`
(immediates), `tools/check-addresses.py` (all addresses old vs new DLL).

## 2. Approach (how we do better than RR)

1. **Function-level replacements instead of instruction caves for the mask code.** The 67 mask functions are mostly
   short "for each valid peer/player" loops. Re-implementing each one in C++ against a 64-bit mask (low word = the
   engine's DWORD, high word in Noble's shadow block) is easier to read, test and keep correct than ~120 hand-made byte
   caves, and every replacement can be unit-tested against the original on recorded inputs (run both, compare).
2. **Capacity profiles.** Stock (≤16), 32, 64. Each profile installs all-or-nothing with byte checks; a lobby ≤16 never
   touches any of it.
3. **Membership over Noble's transport**, full + delta with a checksum and resync; validate and de-duplicate every
   machine record before the engine sees it (stops the "same user twice" boot loop seen at slot 17).
4. **Relevance-based sending** (near/in-view players at full rate, far players slower, roster-only for the rest) on top
   of the engine's priority system, so 64 players fit a sane host upload.
5. **Dedicated servers for 64** (host upload, no host advantage, no host frame drops).

## 3. Order of work

1. Mask high words live in the shadow block (model done) → replace the 52 peer-mask functions, then the 15 player-mask
   functions, behind the dev 64 profile. Each replacement checked against the original at 32 (must be identical).
2. Chunked shadow install for 64 (caves already synthesised and validated), S1/S2 side areas, step caves.
3. Stream manager arrays to 64 machines.
4. Wire fields (player index 6 bits), membership extension 0x6900, pools to 64, session slot count.
5. UI: lobby/roster/scoreboard/browser at 64 (dev-gated).

## 4. Testing without 64 players

- Headless: `noble-dev.exe -test-slot-capacity` (layouts, caves, masks, message sizes) extended with the replacement
  functions' self-tests.
- **Forced high slots**: 2–4 real clients seated at slots 33+ (`NOBLE_SLOT_HIGH`) prove the high-word path end to end on
  one PC.
- Fake players (`tools/fake-players.py`) for Noble's transport and membership at 64 machines.
- Then multi-PC (8–16), then a community night, then 64.

## 5. Progress
- 2026-11-19: `src/host/SlotMask.cpp` (dev, off by default). Replacement 1 of 67: hr+0x395728 "is any other peer still
  connecting" (pure query). Verify mode (`NOBLE_MASK64_VERIFY=1`), 3-player campaign: host 42,790 calls, guests 9,087 /
  9,183, **0 different from the engine**. Use mode (`NOBLE_MASK64=1`), 3-player campaign 120 s: clean, no crashes.
  SlotPatch gained `PeerSlotCount()` and `PeerMaskHigh()` for the 64-bit mask view.

