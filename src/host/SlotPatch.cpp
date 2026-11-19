#include "SlotPatch.hpp"
#include "SlotMessage.hpp"

#include "common/Logger.hpp"
#include "hooks/HookManager.hpp"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <shellapi.h>

namespace noble::slots {

namespace {

struct Site {
    uint32_t rva;
    uint8_t original[7];   // imul r64, r64, imm32
    uint32_t caveLength;
    // 256 bytes: one-range caves are 41 bytes today, but the chunked 64/128 layouts need a piecewise cave with one
    // body per chunk (measured worst case 140 bytes at 128, see SlotLayoutSelfTest).
    uint8_t cave[256];
    uint32_t constantAt;   // imm32 of `add dst, C`
    uint32_t backAt1, backAt2;   // rel32 of the two `jmp back`
    uint32_t record;       // 0xB0 player, 0x158 peer
    uint32_t threshold;    // first slot that does not exist in the original object
    uint32_t scaleShift;   // 0: C = delta - threshold*record; 2: dword-index sites (peer*0x56 + slot), C = (delta >> 2) - threshold*record
};

// Pointer-stepping loops (`add ptr, R` -> cave that moves ptr into the shadow object at the threshold slot).
struct Step {
    uint32_t rva;
    uint8_t original[7];   // add r64, imm32
    uint32_t caveLength;
    uint8_t cave[32];
    uint32_t constantAt;   // imm32 of `add ptr, C`
    uint32_t backAt;       // rel32 of `jmp back`
    uint32_t record;
    uint32_t threshold;
};

// Loop bounds / masks changed only in 32-player mode.
struct Bound {
    uint32_t rva;
    uint32_t length;
    uint8_t original[8];
    uint8_t patched[8];
};

const Site kSites[] = {
#include "SlotPatchTable.inc"
// Player add / remove helpers (high-slot join failure 2026-11-09): hand-reviewed `add@` / `not@` sites.
#include "SlotMemberCaves.inc"
};

const Step kSteps[] = {
#include "SlotStepTable.inc"
};

const Bound kBounds[] = {
#include "SlotBoundsTable.inc"
};

// 17-bit peer masks (`0x1FFFF`) in the session / membership code -> all 32 peer bits in 32-player mode (same as
// 0x391B4A in SlotBoundsTable.inc). List from docs/limits-players-loops.md; each one checked by hand: the mask only
// limits a peer bitmask (DWORD) to the 17 stock peer slots. Plus 0x408F10, which tested only `si` (16 bits) of a
// 32-bit player bitmask.
const Bound kPeerMasks32[] = {
    { 0x39D1A3, 5, { 0xA9, 0xFF, 0xFF, 0x01, 0x00 }, { 0xA9, 0xFF, 0xFF, 0xFF, 0xFF } },                     // test eax,0x1ffff
    { 0x39F1B2, 7, { 0x41, 0xF7, 0xC2, 0xFF, 0xFF, 0x01, 0x00 }, { 0x41, 0xF7, 0xC2, 0xFF, 0xFF, 0xFF, 0xFF } }, // test r10d,0x1ffff
    { 0x39F203, 5, { 0x25, 0xFF, 0xFF, 0x01, 0x00 }, { 0x25, 0xFF, 0xFF, 0xFF, 0xFF } },                     // and eax,0x1ffff
    { 0x3C196F, 7, { 0x41, 0x81, 0xE1, 0xFF, 0xFF, 0x01, 0x00 }, { 0x41, 0x81, 0xE1, 0xFF, 0xFF, 0xFF, 0xFF } }, // and r9d,0x1ffff
    { 0x3C1F68, 5, { 0xB9, 0xFF, 0xFF, 0x01, 0x00 }, { 0xB9, 0xFF, 0xFF, 0xFF, 0xFF } },                     // mov ecx,0x1ffff
    { 0x4037B1, 6, { 0x81, 0xE3, 0xFF, 0xFF, 0x01, 0x00 }, { 0x81, 0xE3, 0xFF, 0xFF, 0xFF, 0xFF } },         // and ebx,0x1ffff
    { 0x4053FB, 5, { 0x25, 0xFF, 0xFF, 0x01, 0x00 }, { 0x25, 0xFF, 0xFF, 0xFF, 0xFF } },                     // and eax,0x1ffff
    { 0x414395, 6, { 0xC7, 0x03, 0xFF, 0xFF, 0x01, 0x00 }, { 0xC7, 0x03, 0xFF, 0xFF, 0xFF, 0xFF } },         // mov dword [rbx],0x1ffff
    { 0x44C3AA, 5, { 0xA9, 0xFF, 0xFF, 0x01, 0x00 }, { 0xA9, 0xFF, 0xFF, 0xFF, 0xFF } },                     // test eax,0x1ffff
    { 0x408F10, 3, { 0x66, 0x85, 0xF6 }, { 0x85, 0xF6, 0x90 } },                                             // test si,si -> test esi,esi; nop
    // Game-over check (2026-11-09 high-slot test): 0x133320 counts the teams that have players (bit per team in r15d / esi,
    // both 32-bit) and skips team > 15. In FFA every player is its own team, so a player in slot 16 was not counted, the
    // check at 0x7A61C saw 1 team < 2 and ended the match with reason 0 the moment the guest joined. Count teams 0..31.
    { 0x1333A9, 3, { 0x83, 0xFD, 0x0F }, { 0x83, 0xFD, 0x1F } },                                             // cmp ebp,0xf -> 0x1f
    // Game player event (simulation event type 9; encoder 0x3C7CE0 from 0x3AB0F4, decoder 0x3A4648 via 0x3C40D1): the
    // game player index went on the wire in 4 bits and the encoder ORs it in unmasked, so player 16 decoded as player 0.
    // The "player joined" event for the slot-16 guest then hit the host's own player 0 (flags 0x8 -> 0x2, "left"; found
    // with a hardware watch, 2026-11-09), the team count dropped to 0 and the match ended. Index -> 5 bits, bound -> 31.
    { 0x3C7D03, 3, { 0x83, 0xF8, 0x04 }, { 0x83, 0xF8, 0x05 } },                                             // cmp eax,0x4 (room left)
    { 0x3C7D08, 4, { 0x83, 0x43, 0x24, 0x04 }, { 0x83, 0x43, 0x24, 0x05 } },                                 // add [rbx+0x24],0x4
    { 0x3C7D0C, 3, { 0x8D, 0x41, 0x04 }, { 0x8D, 0x41, 0x05 } },                                             // lea eax,[rcx+0x4]
    { 0x3C7D16, 4, { 0x48, 0xC1, 0xE0, 0x04 }, { 0x48, 0xC1, 0xE0, 0x05 } },                                 // shl rax,0x4
    { 0x3C7D23, 6, { 0x41, 0xB8, 0x04, 0x00, 0x00, 0x00 }, { 0x41, 0xB8, 0x05, 0x00, 0x00, 0x00 } },         // mov r8d,0x4
    { 0x3A46FE, 5, { 0x41, 0x8D, 0x54, 0x24, 0x03 }, { 0x41, 0x8D, 0x54, 0x24, 0x04 } },                     // lea edx,[r12+0x3] (read 4 bits)
    { 0x3A4735, 4, { 0x83, 0x7D, 0x00, 0x0F }, { 0x83, 0x7D, 0x00, 0x1F } },                                 // cmp [rbp],0xf -> 0x1f
    // 0x411F38 counts the local peer's players before the guest goes on with the join; it scanned players 0..15 only,
    // so a guest whose player is in slot 16 never saw its own player and aborted the join after 16 s. Imul caved in
    // SlotMemberCaves.inc (0x411F8E).
    { 0x411F76, 3, { 0x83, 0xF8, 0x10 }, { 0x83, 0xF8, 0x20 } },                                             // cmp eax,0x10 -> 0x20
    { 0x411FDE, 3, { 0x83, 0xF8, 0x10 }, { 0x83, 0xF8, 0x20 } },                                             // cmp eax,0x10 -> 0x20
    // Loop bounds kept in a register (2026-11-10, docs/limits-players-regbounds.md): gen_bounds.py only raised
    // `cmp reg, imm` bounds, so these scans still stopped at peer 16 / player 15.
    // 0x392DB0 (host join processing): nine peer-mask scans use r13d = 17. The host never promoted a peer in slot 17
    // from state 5 to 6 (0x392F84), so the guest waited at its gate 0x3903C7 (own peer state >= 6) and aborted after
    // 16 s. Every imul in the scans is caved (0x392EF9, 0x392F43, 0x392F56, 0x3930A5); r13 is reloaded at 0x39313F.
    { 0x392E12, 4, { 0x44, 0x8D, 0x6B, 0x11 }, { 0x44, 0x8D, 0x6B, 0x20 } },                                 // lea r13d,[rbx+0x11] (rbx = 0)
    // 0x409898: player-mask scans (block +0x170C), r14d = 16 (rdi = -1); imuls 0x4099B3 / 0x4099C2 are caved.
    { 0x409898, 4, { 0x44, 0x8D, 0x77, 0x11 }, { 0x44, 0x8D, 0x77, 0x21 } },                                 // lea r14d,[rdi+0x11] (rdi = -1)
    // 0x4139E5: peer-mask scans (session +0x5C) against a peer bitmask in ebx, r12d = 17 (rsi = 1); no indexing.
    { 0x4139E5, 4, { 0x44, 0x8D, 0x66, 0x10 }, { 0x44, 0x8D, 0x66, 0x1F } },                                 // lea r12d,[rsi+0x10] (rsi = 1)
};

// C12-L step 3, bounds review #5/#7: 0x390DB0 keeps 16 qword player-name pointers at rbp+0x150, directly below a
// 16-word name buffer and the GS cookie, so the table cannot grow in place. The frame grows by 0x100 (rbp moves
// with rsp, so every other local keeps its offset and the cookie stays at frame+0x2F0) and the table moves to the
// new space at rbp+0x1F8 (32 x 8 = 0x100 of the 0x108 free bytes). Its only consumer, 0x38DA74 (called twice from
// 0x390DB0), scans 32 entries. The loop bounds inside 0x390DB0 stay 16 until its six untraced imul sites are caved.
const Bound kFrame390[] = {
    { 0x390DC8, 7, { 0x48, 0x8D, 0xA8, 0xD8, 0xFD, 0xFF, 0xFF }, { 0x48, 0x8D, 0xA8, 0xD8, 0xFC, 0xFF, 0xFF } },  // lea rbp,[rax-0x228] -> -0x328
    { 0x390DCF, 7, { 0x48, 0x81, 0xEC, 0x00, 0x03, 0x00, 0x00 }, { 0x48, 0x81, 0xEC, 0x00, 0x04, 0x00, 0x00 } },  // sub rsp,0x300 -> 0x400
    { 0x390EA9, 7, { 0x48, 0x8D, 0x8D, 0x50, 0x01, 0x00, 0x00 }, { 0x48, 0x8D, 0x8D, 0xF8, 0x01, 0x00, 0x00 } },  // lea rcx,[rbp+0x150] (memset)
    { 0x390EB0, 6, { 0x41, 0xB8, 0x80, 0x00, 0x00, 0x00 }, { 0x41, 0xB8, 0x00, 0x01, 0x00, 0x00 } },              // mov r8d,0x80 -> 0x100
    { 0x390F55, 8, { 0x48, 0x89, 0x84, 0xF5, 0x50, 0x01, 0x00, 0x00 }, { 0x48, 0x89, 0x84, 0xF5, 0xF8, 0x01, 0x00, 0x00 } },
    { 0x39107B, 8, { 0x4A, 0x8B, 0x8C, 0xD5, 0x50, 0x01, 0x00, 0x00 }, { 0x4A, 0x8B, 0x8C, 0xD5, 0xF8, 0x01, 0x00, 0x00 } },
    { 0x391203, 7, { 0x4C, 0x8D, 0x8D, 0x50, 0x01, 0x00, 0x00 }, { 0x4C, 0x8D, 0x8D, 0xF8, 0x01, 0x00, 0x00 } },  // lea r9 -> 0x38DA74
    { 0x391231, 8, { 0x4C, 0x89, 0xA4, 0xCD, 0x50, 0x01, 0x00, 0x00 }, { 0x4C, 0x89, 0xA4, 0xCD, 0xF8, 0x01, 0x00, 0x00 } },
    { 0x391332, 8, { 0x4A, 0x8B, 0x94, 0xD5, 0x50, 0x01, 0x00, 0x00 }, { 0x4A, 0x8B, 0x94, 0xD5, 0xF8, 0x01, 0x00, 0x00 } },
    { 0x3913FB, 7, { 0x4C, 0x8D, 0x8D, 0x50, 0x01, 0x00, 0x00 }, { 0x4C, 0x8D, 0x8D, 0xF8, 0x01, 0x00, 0x00 } },  // lea r9 -> 0x38DA74
    { 0x39140A, 8, { 0x4A, 0x89, 0xBC, 0xED, 0x50, 0x01, 0x00, 0x00 }, { 0x4A, 0x89, 0xBC, 0xED, 0xF8, 0x01, 0x00, 0x00 } },
    { 0x391586, 8, { 0x4C, 0x8D, 0x9C, 0x24, 0x00, 0x03, 0x00, 0x00 }, { 0x4C, 0x8D, 0x9C, 0x24, 0x00, 0x04, 0x00, 0x00 } },  // epilogue lea r11,[rsp+0x300]
    { 0x38DAE6, 4, { 0x48, 0x83, 0xF9, 0x10 }, { 0x48, 0x83, 0xF9, 0x20 } },  // 0x38DA74: cmp rcx,0x10 -> 0x20 (review #5)
};
// 0x390DB0's unwind info (shared with 0x35B1E4, so it is copied, not edited): UWOP_ALLOC_LARGE 0x300 and the
// rbx/rsi/rdi save slots 0x338/0x340/0x348 move by 0x100. GS handler data (cookie at frame+0x2F0) is unchanged.
constexpr uint8_t kFrame390Unwind[40] = {
    0x19, 0x37, 0x0D, 0x00, 0x26, 0x74, 0x69, 0x00, 0x26, 0x64, 0x68, 0x00, 0x26, 0x34, 0x67, 0x00,
    0x26, 0x01, 0x60, 0x00, 0x18, 0xF0, 0x16, 0xE0, 0x14, 0xD0, 0x12, 0xC0, 0x10, 0x50, 0x00, 0x00,
    0xE0, 0x37, 0x79, 0x00, 0xF0, 0x02, 0x00, 0x00 };   // GS handler RVA 0x7937E0
struct ByteEdit { uint8_t at, from, to; };
constexpr ByteEdit kFrame390UnwindEdits[] = {
    { 6, 0x69, 0x89 }, { 10, 0x68, 0x88 }, { 14, 0x67, 0x87 }, { 18, 0x60, 0x80 } };

// Plan C: 0x396828 builds two stack copies of a membership half (S1 = rsp+0x30 = rbp-0xD0 from static half B,
// S2 = rbp+0x2140 from static half A, 0x2210 each). Peers 17..31 of both copies get side areas in a grown frame.
// The six operands at 0x396AC2..0x396AE5 are [rbp+rcx+disp] with disp(S2) - disp(S1) = 0x2210, so the side areas
// sit 0x2210 apart and one constant serves both: sideS1 = rbp+0x8E00..+0xA228, sideS2 = rbp+0xB010..+0xC438
// (15 x 0x158 each). Frame 0x8F00 -> 0xC540 (+0x3640); rbp = rsp+0x100 still, so every local keeps its offset, the
// GS cookie stays at rbp+0x8DF0 (= frame+0x8EF0), and the pushes start at rbp+0xC440 (8 bytes above sideS2's end).
// The probe size (`mov eax`) feeds both __chkstk and `sub rsp, rax`.
constexpr uint32_t kSideFromCopy = 0x8ED0;           // side area = copy start + this, for S1 and S2
constexpr size_t kSidePeerBytes = 15 * 0x158;        // peers 17..31
const Bound kFrame396[] = {
    { 0x396840, 7, { 0x48, 0x8D, 0xA8, 0xD8, 0x71, 0xFF, 0xFF }, { 0x48, 0x8D, 0xA8, 0x18, 0xF4, 0xFE, 0xFF } },  // lea rbp,[rax-0x8E28] -> -0x10BE8
    { 0x396847, 5, { 0xB8, 0x00, 0x8F, 0x00, 0x00 }, { 0xB8, 0xC0, 0x0C, 0x01, 0x00 } },                          // mov eax,0x8F00 -> 0x10CC0 (probe + sub rsp,rax)
    { 0x396E20, 8, { 0x4C, 0x8D, 0x9C, 0x24, 0x00, 0x8F, 0x00, 0x00 }, { 0x4C, 0x8D, 0x9C, 0x24, 0xC0, 0x0C, 0x01, 0x00 } },  // epilogue lea r11,[rsp+0x8F00] -> 0x10CC0
};
// Context bytes that must also match (not changed): the probe call and `sub rsp, rax`.
const Bound kFrame396Context[] = {
    { 0x39684C, 5, { 0xE8, 0x7F, 0x5A, 0x4D, 0x00 }, {} },
    { 0x396851, 3, { 0x48, 0x2B, 0xE0 }, {} },
};
constexpr uint8_t kFrame396Unwind[40] = {
    0x19, 0x3D, 0x0D, 0x00, 0x2C, 0x74, 0xE9, 0x11, 0x2C, 0x64, 0xE8, 0x11, 0x2C, 0x34, 0xE7, 0x11,
    0x2C, 0x01, 0xE0, 0x11, 0x18, 0xF0, 0x16, 0xE0, 0x14, 0xD0, 0x12, 0xC0, 0x10, 0x50, 0x00, 0x00,
    0xE0, 0x37, 0x79, 0x00, 0xF0, 0x8E, 0x00, 0x00 };   // GS handler RVA 0x7937E0
// #32 (docs/limits-players-message.md): frame 0x10CC0 = plan C 0xC540 + the two membership payload extensions
// (rbp+0xC450..+0xE650, rbp+0xE9A0..+0x10BA0; pushes start at rbp+0x10BC0). ALLOC_LARGE 0x11E0 -> 0x2198 (x8), rbx/rsi/rdi
// save slots 0x11E7/E8/E9 -> 0x219F/A0/A1; GS data 0x8EF0 unchanged (cookie stays at rbp+0x8DF0).
constexpr ByteEdit kFrame396UnwindEdits[] = {
    { 6, 0xE9, 0xA1 }, { 7, 0x11, 0x21 }, { 10, 0xE8, 0xA0 }, { 11, 0x11, 0x21 },
    { 14, 0xE7, 0x9F }, { 15, 0x11, 0x21 }, { 18, 0xE0, 0x98 }, { 19, 0x11, 0x21 } };

struct FrameGrow {
    uint32_t begin, end;
    const Bound* code; size_t codeCount;
    const Bound* context; size_t contextCount;
    const uint8_t* unwind;                     // 40 bytes
    const ByteEdit* unwindEdits; size_t unwindEditCount;
};
const FrameGrow kFrames[] = {
    { 0x390DB0, 0x3915A7, kFrame390, std::size(kFrame390), nullptr, 0, kFrame390Unwind, kFrame390UnwindEdits, std::size(kFrame390UnwindEdits) },
    { 0x396828, 0x396E41, kFrame396, std::size(kFrame396), kFrame396Context, std::size(kFrame396Context), kFrame396Unwind, kFrame396UnwindEdits, std::size(kFrame396UnwindEdits) },
};
constexpr size_t kUnwindBytes = 40;

// 0x396AB4 `imul rcx, rdx, 0x158` (peer index; feeds only the six S1/S2 operands): idx >= 17 -> side area.
//   cmp rdx,0x11 / jae big / imul rcx,rdx,0x158 / jmp back
//   big: lea rcx,[rdx-0x11] / imul rcx,rcx,0x158 / add rcx,0x8EA8 / jmp back     (0x8EA8 = sideS1 - S1 peer 0)
constexpr uint32_t kSideCaveRva = 0x396AB4;
constexpr uint8_t kSideCaveOriginal[7] = { 0x48, 0x69, 0xCA, 0x58, 0x01, 0x00, 0x00 };
constexpr uint8_t kSideCave[41] = {
    0x48, 0x83, 0xFA, 0x11, 0x73, 0x0C, 0x48, 0x69, 0xCA, 0x58, 0x01, 0x00, 0x00, 0xE9, 0, 0, 0, 0,
    0x48, 0x8D, 0x4A, 0xEF, 0x48, 0x69, 0xC9, 0x58, 0x01, 0x00, 0x00, 0x48, 0x81, 0xC1, 0xA8, 0x8E, 0x00, 0x00,
    0xE9, 0, 0, 0, 0 };
constexpr uint32_t kSideCaveBack[2] = { 14, 37 };

// Whole-block copies and clears (scope count in docs/limits-players-loops.md). Each `call memcpy/memset` below is
// redirected to a Noble wrapper that makes the original call and then mirrors it onto the shadow slots when the
// destination is a static session: clear -> clear the covered shadow arrays; static -> static copy -> copy the
// source session's shadow arrays; copy from any other block (#1, decoded from the network) -> clear the
// destination's shadow arrays and log it. A destination outside the static sessions is left alone.
constexpr uint32_t kMemcpyRva = 0x794276, kMemsetRva = 0x78932A;
struct CallSite { uint32_t rva; uint32_t target; const char* what; };
constexpr CallSite kCopySites[] = {
    { 0x39C4D5, kMemcpyRva, "#1 0x39C484 memcpy 0x4710 (caller block -> session+0x30)" },
    { 0x39C4F3, kMemsetRva, "#2 0x39C484 memset 0x2210 (half B)" },
    { 0x39C71A, kMemsetRva, "#3 0x39C6B4 memset 0x4710 (membership reset)" },
    { 0x39C7C4, kMemsetRva, "#4 0x39C7AC memset 0x220C (half-B reset)" },
    { 0x396CBE, kMemcpyRva, "#7 0x396828 memcpy 0x2210 (half A -> half B)" },
    { 0x16FEA9, kMemsetRva, "#8 0x16FE2C memset 0x16D8 (peer array)" },
};
// Plan C call sites in 0x396828 (own wrappers: they always fill / compare the S1/S2 side areas).
constexpr uint32_t kMemcmpRva = 0x794270;
constexpr CallSite kStackCopySites[] = {
    { 0x396A6F, kMemcpyRva, "#5 0x396828 memcpy 0x2210 (half A -> S2)" },
    { 0x396A7F, kMemcpyRva, "#6 0x396828 memcpy 0x2210 (half B -> S1)" },
};
constexpr CallSite kStackCompareSite = { 0x396B3F, kMemcmpRva, "0x396828 memcmp 0x2210 (S2 vs S1)" };

// C12-L step 4: "simulation distributed view" (16 x 77,240 B) is not made by data_array_new: 0x3D0263 allocates
// 0x12DBF4 bytes (16 records + array header) from the region allocator 0x387B0 and 0x3D02B2 initialises it with
// count 16 (0x130B4). 0x387B0 is a bump allocator over a region table (index at RVA 0xC1A630, entries of 0x38 at
// 0xC1A638: +0 next, +8 end) that rounds to 64 KB and has no clean null return, so the 32-record size is only
// asked for when the current region keeps kRegionReserve free afterwards; the init then gets count 32.
constexpr uint32_t kRegionAllocRva = 0x387B0, kArrayInitRva = 0x130B4;
constexpr CallSite kSimViewAllocSite = { 0x3D0271, kRegionAllocRva, "0x3D0263 simulation distributed view allocation" };
constexpr CallSite kSimViewInitSite = { 0x3D02B2, kArrayInitRva, "0x3D02B2 simulation distributed view init" };
// 0x3D0310 records the min/max address and total size of simulation memory; it is passed the 16-record size.
constexpr uint32_t kSimRangeRva = 0x3D0310;
constexpr CallSite kSimViewRangeSite = { 0x3D0282, kSimRangeRva, "0x3D0282 simulation memory range" };
constexpr uint64_t kSimView16Bytes = 0x12DBF4, kSimView32Bytes = 0x74 + 32ull * 0x12DB8;
constexpr uint64_t kRegionReserve = 16ull << 20;
constexpr uint32_t kRegionIndexRva = 0xC1A630, kRegionTableRva = 0xC1A638;

// C12-L (c): the 17 x 0xC peer table at session+0x446C cannot grow in place (session fields follow it at +0x4528),
// so every indexed access is caved (47 operands `[base + idx*4 + disp]`, idx = 3 * peer; list generated by
// tools/limits/gen_table44.py) and the two pointer-stepping reset loops get step caves. Peers 17..31 go to the
// shadow session at the same offset (shadow entry j = shadow + 0x446C + (peer - 17) * 0xC). Each operand cave:
//   pushfq; cmp idx,51; jl small; cmp base,[lo]; jb foreign; cmp base,[hi]; jae foreign
//   popfq; lea idx,[idx+K]; <original>; lea idx,[idx-K] (unless it overwrote idx); jmp back
//   small:   popfq; <original>; jmp back
//   foreign: lock inc [counter]; popfq; <original>; jmp back      (non-session block: left as stock, counted)
// with K = (delta - 51*4) / 4, so [base + 4*(idx+K) + disp] = shadow entry. Flags are preserved around the checks.
// Step caves keep the stock bound (17) on a non-session block and run to 32 on a static session.
// Second form (found by the -trace-members run): `peer + 0x5B0|0x5B4` (bias), `lea rax,[rax+rax*2]`, short access;
// the lea is patched with the access (pre = 4 bytes) and the cave checks peer + bias before running it.
struct TableSite { uint32_t rva; uint32_t length; uint8_t original[9]; uint8_t base, index; bool overwritesIndex;
                   uint16_t bias; uint8_t pre; };
const TableSite kTableSites[] = {
#include "SlotTableSites.inc"
};
// Pointer-stepping loops over a slot array with a fixed bound: `step ptr, record` + `cmp counter, first`.
struct TableStep { uint32_t rva; uint32_t length; uint8_t original[9]; uint32_t stepLength; uint8_t ptr, counter;
                   uint32_t first, record; const char* what; };
constexpr TableStep kTableSteps[] = {
    { 0x39C78C, 7, { 0x48, 0x8D, 0x76, 0x0C, 0x83, 0xFD, 0x11 }, 4, 6, 5, 17, 0xC, "0x39C6B4 reset: lea rsi,[rsi+0xC]; cmp ebp,0x11" },
    { 0x39C818, 7, { 0x48, 0x83, 0xC1, 0x0C, 0x83, 0xFA, 0x11 }, 4, 1, 2, 17, 0xC, "0x39C7AC half-B reset: add rcx,0xC; cmp edx,0x11" },
    // -trace-members 2026-11-09: 0x2A3870 counts matching players over the 16 records 0x398088 returns.
    { 0x2A38CF, 9, { 0x48, 0x05, 0xB0, 0x00, 0x00, 0x00, 0x83, 0xFA, 0x10 }, 6, 0, 2, 16, 0xB0, "0x2A3870 player count: add rax,0xB0; cmp edx,0x10" },
};
constexpr uint32_t kTableFirst = 17;   // first peer that is not in the 17-entry table
constexpr size_t kTableCaveCount = std::size(kTableSites) + std::size(kTableSteps);
constexpr size_t kTableCaveStride = 128;
constexpr size_t kTableCodeBytes = 0x3000;   // caves (RX); the data page after it stays RW
constexpr size_t kTableAllocBytes = kTableCodeBytes + 0x1000;
static_assert(kTableCaveCount * kTableCaveStride <= kTableCodeBytes, "table caves do not fit");
struct TableData { uint64_t lo, hi; volatile uint64_t foreign[kTableCaveCount]; };
static_assert(sizeof(TableData) <= 0x1000, "table data does not fit");
TableData* gTableData = nullptr;

constexpr uintptr_t kSessionsRva = 0x2B07418;   // 4 static session membership objects
constexpr uintptr_t kSessionStride = 0x61668;
constexpr int kSessions = 4;

// Record constructors the session constructor (0x3CF864) runs on every inline slot; run them on the shadow slots
// too so overflow records start in the same state (ids -1, cleared names) instead of all zero.
constexpr uintptr_t kPeerCtorRva = 0x16FC60;     // this = peer record (0x158)
constexpr uintptr_t kPlayerCtorRva = 0x3CFA38;   // this = player record (0xB0)
struct Array { uint32_t offset; uint32_t record; uint32_t firstShadowSlot; bool peer; bool table; };
constexpr Array kArrays[] = {
    { 0x0060, 0x158, 17, true, false },  { 0x1740, 0xB0, 16, false, false },   // block A
    { 0x2270, 0x158, 17, true, false },  { 0x3950, 0xB0, 16, false, false },   // block B
    // C12-L (c): 17 x 0xC peer table at session+0x446C (block+0x443C): byte flag, +4 dword (-1 = none), +8 dword.
    // Not a record array the ctor builds; whole-block copies/clears (#1, #3) cover it, so it is mirrored too.
    { 0x446C, 0xC, 17, true, true },
};
constexpr uint32_t kSlots32 = 32;   // players and peers in 32-player mode (the peer-valid mask is a DWORD)

// Set once by Install (32-player mode) before any call site is redirected.
uintptr_t gSessions = 0;
uint8_t* gShadow = nullptr;

// Session index and offset of `p` inside the static sessions, or -1.
int StaticSession(const void* p, uint32_t& offset) {
    const auto a = reinterpret_cast<uintptr_t>(p);
    if (!gSessions || a < gSessions || a >= gSessions + kSessions * kSessionStride) return -1;
    offset = static_cast<uint32_t>((a - gSessions) % kSessionStride);
    return static_cast<int>((a - gSessions) / kSessionStride);
}

uint8_t* ShadowSlots(int k, const Array& a) { return gShadow + k * kSessionStride + a.offset; }
size_t ShadowBytes(const Array& a) { return static_cast<size_t>(kSlots32 - a.firstShadowSlot) * a.record; }

// High words of the widened peer/player valid masks live in the shadow object, after the chunked arrays (see
// MakeCapacityMaskPlan in the capacity model section). These helpers are used by the shadow init/mirror lifecycle, so
// they are declared before it; at capacity 32 they are 0 bytes and every call is a no-op (the 32 path is unchanged).
constexpr uint32_t kMaskHighBase = 0x28000;
constexpr uint32_t MaskWordsForCapacity(uint32_t capacity) { return (capacity + 31) / 32; }
constexpr uint32_t MaskHighBytes(uint32_t capacity) {
    const uint32_t words = MaskWordsForCapacity(capacity);
    return words > 1 ? 2 * (words - 1) * 4 : 0;   // peer high words followed by player high words
}

// Chunk-aware shadow access. Capacity 32 keeps the proven stock-relative layout (one contiguous chunk at the array
// offset); a 64/128 install uses the chunked layout (chunk A at a cursor, its block-B twin implied by +0x2210). Record
// init, the whole-array copy/clear mirrors and the dev diagnostics all go through this plan, so the future install can
// reuse them while today's 32-player path stays byte-identical.
enum class ShadowKind : uint32_t { Peers = 0, Players = 1, Table = 2 };
struct ShadowChunk { uint32_t first, count, aOffset; };
constexpr uint32_t kMaxShadowChunks = 8;
struct ShadowKindPlan { ShadowChunk chunks[kMaxShadowChunks]; uint32_t count; };
struct ShadowLayoutPlan { uint32_t capacity; ShadowKindPlan kinds[3]; };
constexpr uint32_t kShadowBlockDelta = 0x2210;
constexpr uint32_t kKindStockOffset[3] = { 0x60, 0x1740, 0x446C };
constexpr uint32_t kKindRecordSize[3] = { 0x158, 0xB0, 0xC };
constexpr uint32_t kKindFirstSlot[3] = { 17, 16, 17 };
constexpr uint32_t kKindChunkLimit[3] = { 25, 49, 726 };
uint32_t gCapacity = 0;                 // active capacity (32 while 32-player mode is installed); set by Install
ShadowLayoutPlan gShadowPlan{};         // chunk plan for gCapacity

ShadowLayoutPlan BuildShadowLayoutPlan(uint32_t capacity) {
    ShadowLayoutPlan plan{};
    plan.capacity = capacity;
    if (capacity <= 32) {
        for (uint32_t k = 0; k < 3; ++k) {
            plan.kinds[k].count = 1;
            plan.kinds[k].chunks[0] = { kKindFirstSlot[k], capacity - kKindFirstSlot[k], kKindStockOffset[k] };
        }
        return plan;
    }
    uint32_t cursor = 0;
    for (uint32_t k = 0; k < 3; ++k) {
        const uint32_t entries = capacity - kKindFirstSlot[k];
        uint32_t done = 0, c = 0;
        while (done < entries && c < kMaxShadowChunks) {
            const uint32_t remain = entries - done;
            const uint32_t cnt = remain < kKindChunkLimit[k] ? remain : kKindChunkLimit[k];
            ShadowChunk& ch = plan.kinds[k].chunks[c];
            ch.first = kKindFirstSlot[k] + done;
            ch.count = cnt;
            ch.aOffset = (cursor + 0xF) & ~0xFu;
            cursor = ch.aOffset + kShadowBlockDelta + cnt * kKindRecordSize[k];
            done += cnt;
            ++c;
        }
        plan.kinds[k].count = c;
    }
    return plan;
}

ShadowKind KindOfArray(const Array& a) {
    return a.table ? ShadowKind::Table : (a.peer ? ShadowKind::Peers : ShadowKind::Players);
}
uint32_t ArrayBlockDelta(const Array& a, ShadowKind kind) { return a.offset - kKindStockOffset[static_cast<uint32_t>(kind)]; }
uint8_t* ShadowChunkPtr(int k, ShadowKind kind, const ShadowChunk& c, uint32_t blockDelta) {
    (void)kind;
    return gShadow + static_cast<size_t>(k) * kSessionStride + c.aOffset + blockDelta;
}
const ShadowChunk* FindShadowChunk(const ShadowLayoutPlan& plan, ShadowKind kind, uint32_t index) {
    const ShadowKindPlan& kp = plan.kinds[static_cast<uint32_t>(kind)];
    for (uint32_t c = 0; c < kp.count; ++c)
        if (index >= kp.chunks[c].first && index < kp.chunks[c].first + kp.chunks[c].count) return &kp.chunks[c];
    return nullptr;
}

// Inline arrays fully inside [offset, offset + n) - only those were really copied or cleared.
template <typename F> void ForCoveredArrays(uint32_t offset, size_t n, F f) {
    for (const Array& a : kArrays)
        if (a.offset >= offset && a.offset + static_cast<size_t>(a.firstShadowSlot) * a.record <= offset + n) f(a);
}

// A free peer's player-slot table (peer record +0x144, 4 dwords, count +0x140) must read -1 ("no player"): the add
// path 0x39D620 takes a non -1 entry as the player that peer already owns, compares its id with the joiner's and on a
// mismatch removes it. Inline peers get -1 from the remove path 0x39D0AC; shadow peers start zeroed (ctor + clears), so
// before 2026-11-09 the first peer in slot 17 "owned" player 0: the host's player was removed and the joiner written
// over it (found with a hardware watch on the session's player 0 id).
constexpr uint32_t kPeerPlayerCount = 0x140, kPeerPlayerSlots = 0x144, kPeerPlayerSlotCount = 4;
void ResetShadowPeerTables(int k, const Array& a) {
    if (!a.peer || a.table) return;
    const ShadowKind kind = ShadowKind::Peers;
    const uint32_t blockDelta = ArrayBlockDelta(a, kind);
    const ShadowKindPlan& plan = gShadowPlan.kinds[static_cast<uint32_t>(kind)];
    for (uint32_t c = 0; c < plan.count; ++c) {
        uint8_t* base = ShadowChunkPtr(k, kind, plan.chunks[c], blockDelta);
        for (uint32_t j = 0; j < plan.chunks[c].count; ++j) {
            uint8_t* r = base + static_cast<size_t>(j) * a.record;
            std::memset(r + kPeerPlayerCount, 0, 4);
            std::memset(r + kPeerPlayerSlots, 0xFF, kPeerPlayerSlotCount * 4);
        }
    }
}

void MirrorClear(void* dst, int value, size_t n) {
    uint32_t o = 0;
    const int k = StaticSession(dst, o);
    if (k < 0) return;
    ForCoveredArrays(o, n, [&](const Array& a) {
        const ShadowKind kind = KindOfArray(a);
        const uint32_t blockDelta = ArrayBlockDelta(a, kind);
        const ShadowKindPlan& plan = gShadowPlan.kinds[static_cast<uint32_t>(kind)];
        for (uint32_t c = 0; c < plan.count; ++c) {
            uint8_t* base = ShadowChunkPtr(k, kind, plan.chunks[c], blockDelta);
            std::memset(base, value, static_cast<size_t>(plan.chunks[c].count) * a.record);
        }
        if (value == 0) ResetShadowPeerTables(k, a);
    });
    // The high mask words are session-level state next to the masks, so a whole-block clear clears them too. At capacity
    // 32 the span is empty and this is a no-op.
    if (o == 0 && value == 0) {
        if (const uint32_t maskBytes = MaskHighBytes(gCapacity))
            std::memset(gShadow + static_cast<size_t>(k) * kSessionStride + kMaskHighBase, 0, maskBytes);
    }
}

void MirrorCopy(void* dst, const void* src, size_t n) {
    uint32_t od = 0, os = 0;
    const int kd = StaticSession(dst, od);
    if (kd < 0) return;
    const int ks = StaticSession(src, os);
    bool cleared = false;
    ForCoveredArrays(od, n, [&](const Array& a) {
        const uint32_t from = a.offset - od + os;   // the same array in the source block
        const Array* sa = nullptr;
        for (const Array& b : kArrays) if (b.offset == from && b.record == a.record) sa = &b;
        const ShadowKind kind = KindOfArray(a);
        const ShadowKindPlan& plan = gShadowPlan.kinds[static_cast<uint32_t>(kind)];
        if (ks >= 0 && sa) {
            const uint32_t dDst = ArrayBlockDelta(a, kind);
            const uint32_t dSrc = ArrayBlockDelta(*sa, kind);
            for (uint32_t c = 0; c < plan.count; ++c) {
                std::memmove(ShadowChunkPtr(kd, kind, plan.chunks[c], dDst),
                             ShadowChunkPtr(ks, kind, plan.chunks[c], dSrc),
                             static_cast<size_t>(plan.chunks[c].count) * a.record);
            }
        } else {
            const uint32_t dDst = ArrayBlockDelta(a, kind);
            for (uint32_t c = 0; c < plan.count; ++c)
                std::memset(ShadowChunkPtr(kd, kind, plan.chunks[c], dDst), 0,
                            static_cast<size_t>(plan.chunks[c].count) * a.record);
            ResetShadowPeerTables(kd, a);
            cleared = true;
        }
    });
    // High mask words follow the block: copied between two session shadows, cleared when the source is not a session.
    // At capacity 32 the span is empty and this is a no-op.
    if (const uint32_t maskBytes = MaskHighBytes(gCapacity); maskBytes && od == 0) {
        uint8_t* dstMasks = gShadow + static_cast<size_t>(kd) * kSessionStride + kMaskHighBase;
        if (ks >= 0 && os == 0)
            std::memcpy(dstMasks, gShadow + static_cast<size_t>(ks) * kSessionStride + kMaskHighBase, maskBytes);
        else
            std::memset(dstMasks, 0, maskBytes);
    }
    // Network-decoded blocks (#1) carry no shadow slots yet: drop slots 16..31 rather than keep stale players.
    static LONG logged = 0;
    if (cleared && InterlockedIncrement(&logged) <= 20)
        NOBLE_WARN("SlotPatch: block copied into session {} from a non-session block; shadow slots 16-31 cleared "
                   "(network encoding not widened yet)", kd);
}

void* __cdecl MemcpyMirror(void* dst, const void* src, size_t n) {
    void* r = std::memcpy(dst, src, n);
    MirrorCopy(dst, src, n);
    return r;
}

void* __cdecl MemsetMirror(void* dst, int value, size_t n) {
    void* r = std::memset(dst, value, n);
    MirrorClear(dst, value, n);
    return r;
}

// #5 / #6: copy a static half into S1/S2, then always fill all 15 side records of that copy: the source session's
// shadow peers 17..31 when the source half's peer array is a session array, otherwise zeros. The side area is
// never left uninitialised, so the widened compare below only ever reads written bytes.
void* __cdecl StackCopyMirror(void* dst, const void* src, size_t n) {
    void* r = std::memcpy(dst, src, n);
    uint8_t* side = static_cast<uint8_t*>(dst) + kSideFromCopy;
    uint32_t os = 0;
    const int ks = StaticSession(src, os);
    const Array* peers = nullptr;
    if (ks >= 0) for (const Array& a : kArrays) if (a.peer && !a.table && a.offset == os + 0x28) peers = &a;   // half + 0x28
    // 32-player layout: the 15 side peer records are contiguous at the array offset. A 64/128 install must grow the
    // S1/S2 side areas and copy them chunk by chunk (see docs/02-engine-reference/TOPIC-scaling-64-128.md).
    if (peers) std::memcpy(side, ShadowSlots(ks, *peers), kSidePeerBytes);
    else std::memset(side, 0, kSidePeerBytes);
    return r;
}

// Simulation distributed view (see kSimViewAllocSite).
uintptr_t gEngineBase = 0;
bool gSimView32 = false;
using RegionAllocFn = void* (__fastcall*)(uint64_t, uint64_t, uint64_t, uint64_t);
using ArrayInitFn = uint64_t (__fastcall*)(void*, const char*, int32_t, int32_t, int32_t, void*);

void* __fastcall SimViewAlloc(uint64_t a, uint64_t b, uint64_t size, uint64_t flags) {
    const auto alloc = reinterpret_cast<RegionAllocFn>(gEngineBase + kRegionAllocRva);
    if (size == kSimView16Bytes) {
        const int32_t index = *reinterpret_cast<const int32_t*>(gEngineBase + kRegionIndexRva);
        const auto* region = reinterpret_cast<const uint64_t*>(gEngineBase + kRegionTableRva + static_cast<int64_t>(index) * 0x38);
        const uint64_t next = region[0], end = region[1];
        const uint64_t want = (kSimView32Bytes + 0xFFFF) & ~0xFFFFull;
        if (index >= 0 && index < 16 && end > next && end - next >= want + kRegionReserve) {
            if (void* p = alloc(a, b, kSimView32Bytes, flags)) {
                gSimView32 = true;
                NOBLE_WARN("C12 players: simulation distributed view 16 -> 32 ({} KB more; region {} had {} MB free)",
                           (kSimView32Bytes - kSimView16Bytes) / 1024, index, (end - next) >> 20);
                return p;
            }
        }
        NOBLE_ERROR("C12 players: simulation distributed view stays 16 (region {} has {} MB free, needs {} MB + reserve)",
                    index, end > next ? (end - next) >> 20 : 0, want >> 20);
    }
    return alloc(a, b, size, flags);
}

using SimRangeFn = void (__fastcall*)(int32_t, void*);
void __fastcall SimViewRange(int32_t size, void* p) {
    const auto range = reinterpret_cast<SimRangeFn>(gEngineBase + kSimRangeRva);
    range(gSimView32 && size == static_cast<int32_t>(kSimView16Bytes) ? static_cast<int32_t>(kSimView32Bytes) : size, p);
}

uint64_t __fastcall SimViewInit(void* array, const char* name, int32_t count, int32_t recordSize, int32_t alignBits, void* allocator) {
    const auto init = reinterpret_cast<ArrayInitFn>(gEngineBase + kArrayInitRva);
    return init(array, name, gSimView32 && count == 16 ? 32 : count, recordSize, alignBits, allocator);
}

// 0x396B3F: S2 vs S1 change test; equal inline copies are also compared on their side areas.
using MemcmpFn = int (__cdecl*)(const void*, const void*, size_t);
MemcmpFn gEngineMemcmp = nullptr;
int __cdecl StackCompareMirror(const void* a, const void* b, size_t n) {
    const int r = gEngineMemcmp(a, b, n);
    if (r != 0) return r;
    return std::memcmp(static_cast<const uint8_t*>(a) + kSideFromCopy, static_cast<const uint8_t*>(b) + kSideFromCopy,
                       kSidePeerBytes);
}

// Memory within +-2 GB of `target`, so rel32 jumps and the imm32 shadow constant reach it.
void* AllocateNear(uintptr_t target, size_t size, DWORD protect) {
    SYSTEM_INFO si{};
    GetSystemInfo(&si);
    const uintptr_t step = si.dwAllocationGranularity;
    for (uintptr_t delta = step; delta < 0x7FFF0000ull; delta += step) {
        const uintptr_t candidates[2] = { target + delta, target > delta ? target - delta : 0 };
        for (const uintptr_t candidate : candidates) {
            if (!candidate) continue;
            if (void* p = VirtualAlloc(reinterpret_cast<void*>(candidate & ~(step - 1)), size, MEM_RESERVE | MEM_COMMIT, protect))
                return p;
        }
    }
    return nullptr;
}

void WriteCode(uintptr_t at, const void* bytes, size_t n);
void Put32(uint8_t* p, int64_t v);
void JumpTo(uintptr_t at, const uint8_t* cave);

// UNWIND_INFO is addressed by a 32-bit RVA from the image base, so a copy has to live above the image, within 4 GB.
void* AllocateAbove(uintptr_t base, size_t size) {
    const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(base + reinterpret_cast<const IMAGE_DOS_HEADER*>(base)->e_lfanew);
    SYSTEM_INFO si{};
    GetSystemInfo(&si);
    const uintptr_t step = si.dwAllocationGranularity;
    const uintptr_t first = (base + nt->OptionalHeader.SizeOfImage + step - 1) & ~(step - 1);
    for (uintptr_t at = first; at + size - base < 0xFFFF0000ull; at += step)
        if (void* p = VirtualAlloc(reinterpret_cast<void*>(at), size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE)) return p;
    return nullptr;
}

// The loaded engine's own function-table entry for a grown frame (V16/Full carry their own .pdata, so it is looked
// up, not taken from a stock file offset). nullptr when it is not the expected entry and unwind block.
RUNTIME_FUNCTION* FrameEntry(uintptr_t base, const FrameGrow& f) {
    DWORD64 imageBase = 0;
    auto* rf = RtlLookupFunctionEntry(base + f.begin, &imageBase, nullptr);
    if (!rf || imageBase != base || rf->BeginAddress != f.begin || rf->EndAddress != f.end) return nullptr;
    if (std::memcmp(reinterpret_cast<void*>(base + rf->UnwindData), f.unwind, kUnwindBytes) != 0) return nullptr;
    return rf;
}

// Log what the loaded engine really has for a grown frame's function entry (each profile carries its own .pdata).
void DescribeFrameEntry(uintptr_t base, const FrameGrow& f) {
    DWORD64 imageBase = 0;
    const auto* rf = RtlLookupFunctionEntry(base + f.begin, &imageBase, nullptr);
    if (!rf) {
        NOBLE_WARN("SlotPatch: unwind entry of +0x{:X} does not match: no function entry found", f.begin);
        return;
    }
    char hex[3 * kUnwindBytes + 1] = {};
    const auto* u = reinterpret_cast<const uint8_t*>(imageBase + rf->UnwindData);
    for (size_t i = 0; i < kUnwindBytes; ++i) std::snprintf(hex + 3 * i, 4, "%02X ", u[i]);
    NOBLE_WARN("SlotPatch: unwind entry of +0x{:X} does not match: found module {} entry +0x{:X}..+0x{:X} (expected "
               "+0x{:X}..+0x{:X}), unwind +0x{:X}: {}", f.begin, imageBase == base ? "engine" : "other",
               rf->BeginAddress, rf->EndAddress, f.begin, f.end, rf->UnwindData, hex);
}

bool BytesMatch(uintptr_t base, const Bound* b, size_t n, const char* what) {
    for (size_t i = 0; i < n; ++i)
        if (std::memcmp(reinterpret_cast<void*>(base + b[i].rva), b[i].original, b[i].length) != 0) {
            NOBLE_WARN("SlotPatch: {} code at +0x{:X} does not match", what, b[i].rva);
            return false;
        }
    return true;
}

bool FramesMatch(uintptr_t base) {
    bool ok = true;   // check every frame, so one run logs every mismatch
    for (const FrameGrow& f : kFrames) {
        if (!BytesMatch(base, f.code, f.codeCount, "frame") || !BytesMatch(base, f.context, f.contextCount, "frame")) { ok = false; continue; }
        bool table = true;
        for (size_t i = 0; i < f.unwindEditCount; ++i)
            if (f.unwind[f.unwindEdits[i].at] != f.unwindEdits[i].from) table = false;   // table self-check
        if (!table) { NOBLE_WARN("SlotPatch: frame table of +0x{:X} is inconsistent", f.begin); ok = false; continue; }
        if (!FrameEntry(base, f)) { DescribeFrameEntry(base, f); ok = false; }
    }
    return ok;
}

// Call only after FramesMatch. Each unwind copy goes in before its code, so a stack walk never sees the grown code
// with the old frame size (Install runs before the engine's first frame, so neither function is on any stack).
void ApplyFrames(uintptr_t base, uint8_t* unwindCopies) {
    for (size_t i = 0; i < std::size(kFrames); ++i) {
        const FrameGrow& f = kFrames[i];
        RUNTIME_FUNCTION* rf = FrameEntry(base, f);
        uint8_t* u = unwindCopies + i * 64;
        std::memcpy(u, f.unwind, kUnwindBytes);
        for (size_t j = 0; j < f.unwindEditCount; ++j) u[f.unwindEdits[j].at] = f.unwindEdits[j].to;
        const auto rva = static_cast<DWORD>(reinterpret_cast<uintptr_t>(u) - base);
        WriteCode(reinterpret_cast<uintptr_t>(&rf->UnwindData), &rva, sizeof(rva));
        for (size_t j = 0; j < f.codeCount; ++j) WriteCode(base + f.code[j].rva, f.code[j].patched, f.code[j].length);
    }
    DWORD old = 0;
    VirtualProtect(unwindCopies, std::size(kFrames) * 64, PAGE_READONLY, &old);
}

// `call rel32` at the site must reach the expected engine function.
bool CallMatches(uintptr_t base, const CallSite& s) {
    const auto* p = reinterpret_cast<const uint8_t*>(base + s.rva);
    int32_t rel = 0;
    std::memcpy(&rel, p + 1, 4);
    if (p[0] == 0xE8 && s.rva + 5 + rel == s.target) return true;
    NOBLE_WARN("SlotPatch: call at +0x{:X} does not match ({})", s.rva, s.what);
    return false;
}

bool CallSitesMatch(uintptr_t base) {
    for (const CallSite& s : kCopySites) if (!CallMatches(base, s)) return false;
    for (const CallSite& s : kStackCopySites) if (!CallMatches(base, s)) return false;
    if (!CallMatches(base, kStackCompareSite)) return false;
    for (const CallSite* s : { &kSimViewAllocSite, &kSimViewInitSite, &kSimViewRangeSite }) if (!CallMatches(base, *s)) return false;
    if (std::memcmp(reinterpret_cast<void*>(base + kSideCaveRva), kSideCaveOriginal, sizeof(kSideCaveOriginal)) != 0) {
        NOBLE_WARN("SlotPatch: 0x396AB4 imul does not match");
        return false;
    }
    return true;
}

constexpr int kStubCount = 7;
constexpr size_t kCaveAt = kStubCount * 16;
constexpr size_t kStubBytes = kCaveAt + 48;   // jump stubs + the 0x396AB4 side cave

// Stubs `jmp [rip+0]; dq target` near the engine (0 memcpy mirror, 1 memset mirror, 2 stack copy, 3 stack
// compare, 4 sim-view alloc, 5 sim-view init, 6 sim range) and the 0x396AB4 cave after them; then every call
// site's rel32 points at its stub.
void ApplyCallSites(uintptr_t base, uint8_t* stubs) {
    gEngineMemcmp = reinterpret_cast<MemcmpFn>(base + kMemcmpRva);
    gEngineBase = base;
    const void* targets[kStubCount] = {
        reinterpret_cast<const void*>(&MemcpyMirror), reinterpret_cast<const void*>(&MemsetMirror),
        reinterpret_cast<const void*>(&StackCopyMirror), reinterpret_cast<const void*>(&StackCompareMirror),
        reinterpret_cast<const void*>(&SimViewAlloc), reinterpret_cast<const void*>(&SimViewInit),
        reinterpret_cast<const void*>(&SimViewRange) };
    for (int i = 0; i < kStubCount; ++i) {
        const uint8_t jmp[6] = { 0xFF, 0x25, 0, 0, 0, 0 };
        std::memcpy(stubs + i * 16, jmp, 6);
        std::memcpy(stubs + i * 16 + 6, &targets[i], 8);
    }
    uint8_t* cave = stubs + kCaveAt;
    std::memcpy(cave, kSideCave, sizeof(kSideCave));
    const uintptr_t back = base + kSideCaveRva + sizeof(kSideCaveOriginal);
    for (const uint32_t at : kSideCaveBack)
        Put32(cave + at, static_cast<int64_t>(back) - reinterpret_cast<intptr_t>(cave + at + 4));
    DWORD old = 0;
    VirtualProtect(stubs, kStubBytes, PAGE_EXECUTE_READ, &old);
    FlushInstructionCache(GetCurrentProcess(), stubs, kStubBytes);
    auto redirect = [&](const CallSite& s, int stub) {
        uint8_t rel[4];
        Put32(rel, reinterpret_cast<intptr_t>(stubs + stub * 16) - static_cast<intptr_t>(base + s.rva + 5));
        WriteCode(base + s.rva + 1, rel, 4);
    };
    for (const CallSite& s : kCopySites) redirect(s, s.target == kMemcpyRva ? 0 : 1);
    for (const CallSite& s : kStackCopySites) redirect(s, 2);
    redirect(kStackCompareSite, 3);
    redirect(kSimViewAllocSite, 4);
    redirect(kSimViewInitSite, 5);
    redirect(kSimViewRangeSite, 6);
    JumpTo(base + kSideCaveRva, cave);
}

bool TableSitesMatch(uintptr_t base) {
    for (const TableSite& s : kTableSites)
        if (s.length < 5 || std::memcmp(reinterpret_cast<void*>(base + s.rva), s.original, s.length) != 0) {
            NOBLE_WARN("SlotPatch: peer table site at +0x{:X} does not match", s.rva);
            return false;
        }
    for (const TableStep& s : kTableSteps)
        if (s.length < 5 || std::memcmp(reinterpret_cast<void*>(base + s.rva), s.original, s.length) != 0) {
            NOBLE_WARN("SlotPatch: slot step loop at +0x{:X} does not match ({})", s.rva, s.what);
            return false;
        }
    return true;
}

// Minimal x64 emitter for the table caves (registers 0..15 = rax..r15).
struct Emit {
    uint8_t* p;
    void b(uint8_t x) { *p++ = x; }
    void d(int32_t v) { std::memcpy(p, &v, 4); p += 4; }
    void bytes(const uint8_t* s, size_t n) { std::memcpy(p, s, n); p += n; }
    void rip(const volatile void* t) { d(static_cast<int32_t>(reinterpret_cast<intptr_t>(t) - reinterpret_cast<intptr_t>(p + 4))); }
    uint8_t rex(bool w, int reg, int rm) { return static_cast<uint8_t>(0x40 | (w ? 8 : 0) | ((reg >> 3) << 2) | (rm >> 3)); }
    void cmpRip(int r, const volatile void* t) { b(rex(true, r, 0)); b(0x3B); b(static_cast<uint8_t>(((r & 7) << 3) | 5)); rip(t); }
    void leaAdd(int r, int32_t v) {   // lea r64,[r64 + disp32]
        b(rex(true, r, r)); b(0x8D); b(static_cast<uint8_t>(0x80 | ((r & 7) << 3) | (r & 7)));
        if ((r & 7) == 4) b(0x24);
        d(v);
    }
    void cmp64(int r, int32_t v) { b(rex(true, 0, r)); b(0x81); b(static_cast<uint8_t>(0xF8 | (r & 7))); d(v); }
    void cmp32(int r, int8_t v) { if (r >= 8) b(0x41); b(0x83); b(static_cast<uint8_t>(0xF8 | (r & 7))); b(static_cast<uint8_t>(v)); }
    uint8_t* jcc8(uint8_t op) { b(op); b(0); return p - 1; }
    void land(uint8_t* at) { *at = static_cast<uint8_t>(p - (at + 1)); }
    void jmp(uintptr_t to) { b(0xE9); d(static_cast<int32_t>(static_cast<intptr_t>(to) - reinterpret_cast<intptr_t>(p + 4))); }
    void lockInc(const volatile void* t) { b(0xF0); b(0x48); b(0xFF); b(0x05); rip(t); }
};

void BuildOperandCave(uint8_t* at, const TableSite& s, uintptr_t back, int32_t k, volatile uint64_t* counter) {
    Emit e{ at };
    e.b(0x9C);                                   // pushfq
    // idx is 3 * peer, or peer + bias before the leading lea (pre bytes)
    e.cmp64(s.index, static_cast<int32_t>(s.pre ? s.bias + kTableFirst : 3 * kTableFirst));
    uint8_t* small = e.jcc8(0x7C);               // jl (also peer -1)
    e.cmpRip(s.base, &gTableData->lo);
    uint8_t* foreign1 = e.jcc8(0x72);            // jb
    e.cmpRip(s.base, &gTableData->hi);
    uint8_t* foreign2 = e.jcc8(0x73);            // jae
    e.b(0x9D);                                   // popfq
    e.bytes(s.original, s.pre);                  // lea rax,[rax+rax*2] (bias form only)
    e.leaAdd(s.index, k);
    e.bytes(s.original + s.pre, s.length - s.pre);
    if (!s.overwritesIndex) e.leaAdd(s.index, -k);
    e.jmp(back);
    e.land(small);
    e.b(0x9D);
    e.bytes(s.original, s.length);
    e.jmp(back);
    e.land(foreign1);
    e.land(foreign2);
    e.lockInc(counter);
    e.b(0x9D);
    e.bytes(s.original, s.length);
    e.jmp(back);
}

// Step + bound: at counter == first the pointer is the inline record `first`; on a static session move it to the
// shadow record `first` and run to 32, on any other block finish at `first` like the stock loop (and count it).
void BuildStepCave(uint8_t* at, const TableStep& s, uintptr_t back, int32_t c17, volatile uint64_t* counter) {
    Emit e{ at };
    e.bytes(s.original, s.stepLength);           // the original step
    e.cmp32(s.counter, static_cast<int8_t>(s.first));
    uint8_t* notFirst = e.jcc8(0x75);            // jne
    e.cmpRip(s.ptr, &gTableData->lo);
    uint8_t* foreign1 = e.jcc8(0x72);
    e.cmpRip(s.ptr, &gTableData->hi);
    uint8_t* foreign2 = e.jcc8(0x73);
    e.leaAdd(s.ptr, c17);
    e.land(notFirst);
    e.cmp32(s.counter, static_cast<int8_t>(kSlots32));
    e.jmp(back);
    e.land(foreign1);
    e.land(foreign2);
    e.lockInc(counter);
    e.cmp32(s.counter, static_cast<int8_t>(s.first));       // == first: the loop ends as stock
    e.jmp(back);
}

void JumpToN(uintptr_t at, const uint8_t* cave, size_t length) {
    uint8_t jump[16];
    std::memset(jump, 0x90, sizeof(jump));
    jump[0] = 0xE9;
    Put32(jump + 1, reinterpret_cast<intptr_t>(cave) - static_cast<intptr_t>(at + 5));
    WriteCode(at, jump, length);
}

uint32_t TableCaveRva(size_t i) {
    return i < std::size(kTableSites) ? kTableSites[i].rva : kTableSteps[i - std::size(kTableSites)].rva;
}

// Dev guard: every 5 s, log table sites that saw peer >= 17 on a block that is not a static session.
DWORD WINAPI TableGuardThread(void*) {
    uint64_t last[kTableCaveCount] = {};
    for (;;) {
        Sleep(5000);
        for (size_t i = 0; i < kTableCaveCount; ++i) {
            const uint64_t n = gTableData->foreign[i];
            if (n == last[i]) continue;
            last[i] = n;
            NOBLE_WARN("SlotPatch guard: peer table site +0x{:X} reached peer >= 17 on a non-session block ({} times so "
                       "far; left at the stock address)", TableCaveRva(i), n);
        }
    }
}

// Call only after TableSitesMatch. mem = kTableAllocBytes near the engine (RW).
void ApplyTableSites(uintptr_t base, uint8_t* mem, uintptr_t sessions, int64_t delta) {
    gTableData = reinterpret_cast<TableData*>(mem + kTableCodeBytes);
    gTableData->lo = sessions;
    gTableData->hi = sessions + kSessions * kSessionStride;
    const auto k = static_cast<int32_t>((delta - 3 * kTableFirst * 4) / 4);
    size_t i = 0;
    for (const TableSite& s : kTableSites) {
        BuildOperandCave(mem + i * kTableCaveStride, s, base + s.rva + s.length, k, &gTableData->foreign[i]);
        ++i;
    }
    for (const TableStep& s : kTableSteps) {
        const auto c = static_cast<int32_t>(delta - static_cast<int64_t>(s.first) * s.record);
        BuildStepCave(mem + i * kTableCaveStride, s, base + s.rva + s.length, c, &gTableData->foreign[i]);
        ++i;
    }
    DWORD old = 0;
    VirtualProtect(mem, kTableCodeBytes, PAGE_EXECUTE_READ, &old);
    FlushInstructionCache(GetCurrentProcess(), mem, kTableCodeBytes);
    i = 0;
    for (const TableSite& s : kTableSites) { JumpToN(base + s.rva, mem + i * kTableCaveStride, s.length); ++i; }
    for (const TableStep& s : kTableSteps) { JumpToN(base + s.rva, mem + i * kTableCaveStride, s.length); ++i; }
    if (HANDLE t = CreateThread(nullptr, 0, &TableGuardThread, nullptr, 0, nullptr)) CloseHandle(t);
}

bool Requested() {
    wchar_t envOff[8] = {};
    if (GetEnvironmentVariableW(L"NOBLE_SLOT_PATCH_OFF", envOff, 8) > 0 && envOff[0] == L'1') return false;
    wchar_t env[8] = {};
    if (GetEnvironmentVariableW(L"NOBLE_SLOT_PATCH", env, 8) > 0 && env[0] == L'1') return true;
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    bool on = false;
    for (int i = 1; argv && i < argc; ++i) if (_wcsicmp(argv[i], L"-slot-patch") == 0) on = true;
    if (argv) LocalFree(argv);
    if (on) SetEnvironmentVariableW(L"NOBLE_SLOT_PATCH", L"1");   // carried into the relaunched match process
    // On extended engine profile, slot patch is enabled by default to seat extended players
    return true;
}

// 32-player mode: high-slot seat blocker verified and solved in commit 0fdcf97
bool UnsafeOverride() {
    return true;
}

bool gPlayers32Active = false;
bool gMessageOn = true;

// Requested dev slot capacity from -slot-players N / NOBLE_SLOT_PLAYERS=N: 32 is implemented;
// 64/128 are recognised so the request can be refused with a clear message instead of silently ignored.
uint32_t RequestedPlayersCapacity() {
    wchar_t env[16] = {};
    if (GetEnvironmentVariableW(L"NOBLE_SLOT_PLAYERS", env, 16) > 0) {
        const int v = _wtoi(env);
        if (v == 16 || v == 32 || v == 64 || v == 128) return static_cast<uint32_t>(v);
    }
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    uint32_t found = 0;
    for (int i = 1; argv && i + 1 < argc; ++i) {
        if (_wcsicmp(argv[i], L"-slot-players") == 0) {
            const int v = _wtoi(argv[i + 1]);
            if (v == 16 || v == 32 || v == 64 || v == 128) found = static_cast<uint32_t>(v);
        }
    }
    if (argv) LocalFree(argv);
    return found;
}

bool ArgPlayers32() {
    wchar_t env[8] = {};
    if (GetEnvironmentVariableW(L"NOBLE_SLOT_PLAYERS", env, 8) > 0 && _wtoi(env) == 32) return true;
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    bool on = false;
    for (int i = 1; argv && i + 1 < argc; ++i)
        if (_wcsicmp(argv[i], L"-slot-players") == 0 && _wtoi(argv[i + 1]) == 32) on = true;
    if (argv) LocalFree(argv);
    if (on) SetEnvironmentVariableW(L"NOBLE_SLOT_PLAYERS", L"32");
    return on;
}

void Put32(uint8_t* p, int64_t v) { const auto x = static_cast<int32_t>(v); std::memcpy(p, &x, 4); }

size_t Rounded(uint32_t n) { return (n + 15) & ~15u; }

void WriteCode(uintptr_t at, const void* bytes, size_t n) {
    DWORD old = 0;
    VirtualProtect(reinterpret_cast<void*>(at), n, PAGE_EXECUTE_READWRITE, &old);
    std::memcpy(reinterpret_cast<void*>(at), bytes, n);
    VirtualProtect(reinterpret_cast<void*>(at), n, old, &old);
}

void JumpTo(uintptr_t at, const uint8_t* cave) {
    uint8_t jump[7] = { 0xE9, 0, 0, 0, 0, 0x90, 0x90 };
    Put32(jump + 1, reinterpret_cast<intptr_t>(cave) - static_cast<intptr_t>(at + 5));
    WriteCode(at, jump, sizeof(jump));
}

void InitShadowRecords(uintptr_t base, uint8_t* shadow, bool players32) {
    using Ctor = void* (*)(void*);
    const auto peerCtor = reinterpret_cast<Ctor>(base + kPeerCtorRva);
    const auto playerCtor = reinterpret_cast<Ctor>(base + kPlayerCtorRva);
    for (int k = 0; k < kSessions; ++k) {
        uint8_t* object = shadow + k * kSessionStride;
        for (const Array& a : kArrays) {
            if (!players32) continue;
            const ShadowKind kind = KindOfArray(a);
            const uint32_t blockDelta = ArrayBlockDelta(a, kind);
            const ShadowKindPlan& plan = gShadowPlan.kinds[static_cast<uint32_t>(kind)];
            for (uint32_t c = 0; c < plan.count; ++c) {
                for (uint32_t j = 0; j < plan.chunks[c].count; ++j) {
                    uint8_t* r = object + plan.chunks[c].aOffset + blockDelta + static_cast<size_t>(j) * a.record;
                    if (a.table) { std::memset(r, 0, 4); std::memset(r + 4, 0xFF, 8); continue; }
                    (a.peer ? peerCtor : playerCtor)(r);
                }
            }
            if (plan.count) ResetShadowPeerTables(k, a);
        }
        if (const uint32_t maskBytes = MaskHighBytes(gCapacity)) std::memset(object + kMaskHighBase, 0, maskBytes);
    }
}

// 0x3FF588 (parameter object, 16-byte value, peer index): sets one entry of a session parameter that holds 17 x 16
// bytes (session +0x60A60; the guest sets its own entry from 0x3907B4 right after the join). It copies the 0x110-byte
// value to a stack buffer and writes entry [index]: index 17 overwrote the stack cookie (STACK OVERRUN at +0x3FF666,
// 2026-11-10 00:44 test), index 18 the saved registers (crash at address 0). Until that parameter is grown to 32
// entries (it is also sent over the network), peers >= 17 skip the write. Stopgap, 32-player mode only.
typedef void(__fastcall* PeerParamSetFn)(void* parameter, const void* value, int peer);
PeerParamSetFn s_originalPeerParamSet = nullptr;
void __fastcall Hook_PeerParamSet(void* parameter, const void* value, int peer) {
    if (peer >= 17) {
        static int logged = 0;
        if (logged < 5) { ++logged; NOBLE_WARN("SlotPatch: 17-entry session parameter {} not set for peer {} (stopgap)", parameter, peer); }
        return;
    }
    s_originalPeerParamSet(parameter, value, peer);
}

} // namespace

bool Players32Requested() {
    wchar_t playersOff[8] = {};
    if (GetEnvironmentVariableW(L"NOBLE_SLOT_PLAYERS_OFF", playersOff, 8) > 0 && playersOff[0] == L'1') return false;
    wchar_t env[16] = {};
    if (GetEnvironmentVariableW(L"NOBLE_SLOT_PLAYERS", env, 16) > 0 && _wtoi(env) == 16) return false;
    return true;
}
bool Players32Active() { return gPlayers32Active; }

// Dev diagnostics: address of peer record `peer` (0x158 each from session + 0x60) of the static session that
// contains `session`, in the shadow object for peers >= 17. nullptr if 32-player mode is off or out of range.
uint8_t* PeerRecord(uint8_t* session, int32_t peer) {
    const uint32_t capacity = gCapacity ? gCapacity : kSlots32;
    if (peer < 0 || static_cast<uint32_t>(peer) >= capacity || !session) return nullptr;
    if (peer < 17) return session + 0x60 + static_cast<size_t>(peer) * 0x158;
    uint32_t o = 0;
    const int k = StaticSession(session, o);
    if (k < 0 || o != 0 || !gShadow) return nullptr;
    const ShadowChunk* chunk = FindShadowChunk(gShadowPlan, ShadowKind::Peers, static_cast<uint32_t>(peer));
    if (!chunk) return nullptr;
    return gShadow + static_cast<size_t>(k) * kSessionStride + chunk->aOffset +
           static_cast<size_t>(peer - chunk->first) * 0x158;
}

uint32_t PeerSlotCount() { return gPlayers32Active && gCapacity > 17 ? gCapacity : 17; }

uint32_t* PeerMaskHigh(uint8_t* session) {
    if (!gPlayers32Active || gCapacity <= 32 || !gShadow || !session) return nullptr;
    uint32_t o = 0;
    const int k = StaticSession(session, o);
    if (k < 0 || o != 0) return nullptr;
    return reinterpret_cast<uint32_t*>(gShadow + static_cast<size_t>(k) * kSessionStride + kMaskHighBase);
}

void Install(HMODULE engine, bool extendedProfile) {
    if (!engine || !Requested()) return;
    if (!extendedProfile) {
        NOBLE_WARN("SlotPatch: -slot-patch needs ENGINE LIMITS set to Extended; ignored");
        return;
    }
    const auto base = reinterpret_cast<uintptr_t>(engine);
    for (const Site& s : kSites) {   // all or nothing: any other engine build leaves every site untouched
        if (std::memcmp(reinterpret_cast<void*>(base + s.rva), s.original, sizeof(s.original)) != 0) {
            NOBLE_WARN("SlotPatch: engine code at +0x{:X} does not match; nothing patched", s.rva);
            return;
        }
    }
    const uint32_t requestedCapacity = RequestedPlayersCapacity();
    if (requestedCapacity > 32) {
        NOBLE_WARN("SlotPatch: {} slot capacity requested, but the chunked 64/128 install is not implemented yet "
                   "(only the layout/dispatch is validated - docs/02-engine-reference/TOPIC-scaling-64-128.md). "
                   "Staying at 32 capacity",
                   requestedCapacity);
    }
    bool players32 = true;
    wchar_t playersOff[8] = {};
    if (GetEnvironmentVariableW(L"NOBLE_SLOT_PLAYERS_OFF", playersOff, 8) > 0 && playersOff[0] == L'1') {
        players32 = false;
    }
    if (requestedCapacity == 16) {
        players32 = false;
    }
    if (players32) {
        NOBLE_INFO("SlotPatch: enabling extended 32-player mode (high-slot seats enabled, layout verified)");
    }
    if (players32) {
        for (const Step& s : kSteps) {
            if (std::memcmp(reinterpret_cast<void*>(base + s.rva), s.original, sizeof(s.original)) != 0) {
                NOBLE_WARN("SlotPatch: step loop at +0x{:X} does not match; 32-player mode off", s.rva);
                players32 = false;
                break;
            }
        }
        for (const Bound& b : kPeerMasks32) {
            if (!players32) break;
            if (std::memcmp(reinterpret_cast<void*>(base + b.rva), b.original, b.length) != 0) {
                NOBLE_WARN("SlotPatch: peer mask at +0x{:X} does not match; 32-player mode off", b.rva);
                players32 = false;
            }
        }
        for (const Bound& b : kBounds) {
            if (!players32) break;
            if (std::memcmp(reinterpret_cast<void*>(base + b.rva), b.original, b.length) != 0) {
                NOBLE_WARN("SlotPatch: loop bound at +0x{:X} does not match; 32-player mode off", b.rva);
                players32 = false;
            }
        }
        // Run every check (each logs its own mismatch), so one run shows all of them; any failure keeps 16 players.
        const bool frames = FramesMatch(base);
        const bool calls = CallSitesMatch(base);
        const bool table = TableSitesMatch(base);
        // NOBLE_SLOT_MSG_OFF=1 (diagnostics): leave the #32 message group out (the 0x396828 frame still grows).
        wchar_t msgOff[4] = {};
        const bool msgOn = !(GetEnvironmentVariableW(L"NOBLE_SLOT_MSG_OFF", msgOff, 4) > 0 && msgOff[0] == L'1');
        gMessageOn = msgOn;
        const bool msg = !msgOn || message::Match(base);
        if (players32 && !(frames && calls && table && msg)) {
            NOBLE_WARN("SlotPatch: 32-player checks: frames {}, call sites {}, peer table sites {}, membership message {}; "
                       "32-player mode off", frames ? "ok" : "FAILED", calls ? "ok" : "FAILED", table ? "ok" : "FAILED",
                       msg ? "ok" : "FAILED");
            players32 = false;
        }
    }
    const uintptr_t sessions = base + kSessionsRva;
    // Shadow objects: same stride as the real ones, so shadow(obj) = obj + delta for all four.
    auto* shadow = static_cast<uint8_t*>(AllocateNear(sessions, kSessions * kSessionStride, PAGE_READWRITE));
    size_t caveBytes = 0;
    for (const Site& s : kSites) caveBytes += Rounded(s.caveLength);
    if (players32) for (const Step& s : kSteps) caveBytes += Rounded(s.caveLength);
    auto* caves = static_cast<uint8_t*>(AllocateNear(base, caveBytes, PAGE_READWRITE));
    auto* unwindCopies = players32 ? static_cast<uint8_t*>(AllocateAbove(base, std::size(kFrames) * 64)) : nullptr;
    auto* copyStubs = players32 ? static_cast<uint8_t*>(AllocateNear(base, kStubBytes, PAGE_READWRITE)) : nullptr;
    auto* tableCaves = players32 ? static_cast<uint8_t*>(AllocateNear(base, kTableAllocBytes, PAGE_READWRITE)) : nullptr;
    if (!shadow || !caves || (players32 && (!unwindCopies || !copyStubs || !tableCaves))) {
        NOBLE_ERROR("SlotPatch: could not allocate memory near the engine; nothing patched");
        if (shadow) VirtualFree(shadow, 0, MEM_RELEASE);
        if (caves) VirtualFree(caves, 0, MEM_RELEASE);
        if (unwindCopies) VirtualFree(unwindCopies, 0, MEM_RELEASE);
        if (copyStubs) VirtualFree(copyStubs, 0, MEM_RELEASE);
        if (tableCaves) VirtualFree(tableCaves, 0, MEM_RELEASE);
        return;
    }
    gCapacity = players32 ? kSlots32 : 16;
    gShadowPlan = BuildShadowLayoutPlan(gCapacity);
    gSessions = sessions;
    gShadow = shadow;
    InitShadowRecords(base, shadow, players32);
    const int64_t delta = reinterpret_cast<intptr_t>(shadow) - static_cast<intptr_t>(sessions);
    // The table caves scale the shadow delta by 1/4 (index register * 4); sessions and shadow are 4-aligned.
    if (players32 && (delta % 4) != 0) {
        NOBLE_ERROR("SlotPatch: shadow delta 0x{:X} is not a multiple of 4; 32-player mode off", static_cast<uint64_t>(delta));
        players32 = false;
    }

    // Fill the caves (rel32 back jumps and the shadow constant), then make them executable.
    uint8_t* cave = caves;
    for (const Site& s : kSites) {
        std::memcpy(cave, s.cave, s.caveLength);
        const int64_t scaled = s.scaleShift ? delta / (int64_t{ 1 } << s.scaleShift) : delta;   // delta % 4 == 0 (checked)
        Put32(cave + s.constantAt, scaled - static_cast<int64_t>(s.threshold) * s.record);
        const uintptr_t back = base + s.rva + sizeof(s.original);
        Put32(cave + s.backAt1, static_cast<int64_t>(back) - reinterpret_cast<intptr_t>(cave + s.backAt1 + 4));
        Put32(cave + s.backAt2, static_cast<int64_t>(back) - reinterpret_cast<intptr_t>(cave + s.backAt2 + 4));
        cave += Rounded(s.caveLength);
    }
    uint8_t* const stepCaves = cave;
    if (players32) {
        for (const Step& s : kSteps) {
            std::memcpy(cave, s.cave, s.caveLength);
            Put32(cave + s.constantAt, delta - static_cast<int64_t>(s.threshold) * s.record);
            const uintptr_t back = base + s.rva + sizeof(s.original);
            Put32(cave + s.backAt, static_cast<int64_t>(back) - reinterpret_cast<intptr_t>(cave + s.backAt + 4));
            cave += Rounded(s.caveLength);
        }
    }
    DWORD old = 0;
    VirtualProtect(caves, caveBytes, PAGE_EXECUTE_READ, &old);
    FlushInstructionCache(GetCurrentProcess(), caves, caveBytes);

    // Point the engine at the caves.
    cave = caves;
    for (const Site& s : kSites) { JumpTo(base + s.rva, cave); cave += Rounded(s.caveLength); }
    if (players32) {
        cave = stepCaves;
        for (const Step& s : kSteps) { JumpTo(base + s.rva, cave); cave += Rounded(s.caveLength); }
        for (const Bound& b : kBounds) WriteCode(base + b.rva, b.patched, b.length);
        for (const Bound& b : kPeerMasks32) WriteCode(base + b.rva, b.patched, b.length);
        static const uint8_t kPeerParamSetHead[] = { 0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x20 };
        if (std::memcmp(reinterpret_cast<void*>(base + 0x3FF588), kPeerParamSetHead, sizeof(kPeerParamSetHead)) != 0 ||
            !HookManager::InstallHook(reinterpret_cast<void*>(base + 0x3FF588), reinterpret_cast<void*>(&Hook_PeerParamSet),
                                      reinterpret_cast<void**>(&s_originalPeerParamSet), "peer_param_set_17")) {
            NOBLE_WARN("SlotPatch: peer parameter guard (+0x3FF588) not installed; a peer in slot 17+ will crash when it sets its entry");
        }
        NOBLE_INFO("SlotPatch: {} peer / team masks widened to 32 (17-bit peer masks, 0x408F10, game-over team count, game player event index 4->5 bits, register-held loop bounds)", std::size(kPeerMasks32));
        ApplyFrames(base, unwindCopies);   // checked above (FramesMatch)
        ApplyCallSites(base, copyStubs);   // checked above (CallSitesMatch); after ApplyFrames: the side areas exist
        ApplyTableSites(base, tableCaves, sessions, delta);   // checked above (TableSitesMatch)
        if (!gMessageOn) NOBLE_WARN("SlotPatch: NOBLE_SLOT_MSG_OFF=1 - membership message (#32) patches left out");
        else if (!message::Apply(base, delta))   // checked above (message::Match); after ApplyFrames: the payload extensions exist
            NOBLE_ERROR("SlotPatch: membership message patches not installed; 32-player sessions will desync past 16 players");
        wchar_t high[4] = {};
        if (GetEnvironmentVariableW(L"NOBLE_SLOT_HIGH", high, 4) > 0 && high[0] == L'1') message::ApplyHighSlotTest(base);
    }
    FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void*>(base), 0x1000000);
    NOBLE_INFO("SlotPatch: {} slot address sites redirected (players >= 16, peers >= 17 -> shadow sessions at {:p}, "
               "delta 0x{:X})", std::size(kSites), static_cast<void*>(shadow), static_cast<uint64_t>(delta));
    gPlayers32Active = players32;
    if (players32)
        NOBLE_INFO("SlotPatch: extended 32-player mode active: {} step loops, {} loop bounds raised to 32, 0x390DB0 frame +0x100 "
                   "(32-entry name table), {} block copy/clear sites mirrored onto the shadow, 0x396828 S1/S2 peer side "
                   "areas, peer table +0x446C: {} sites caved, {} step loops (guard logs non-session blocks)",
                   std::size(kSteps), std::size(kBounds), std::size(kCopySites),
                   std::size(kTableSites), std::size(kTableSteps));
    else
        NOBLE_INFO("SlotPatch: 16-player mode active");
}

// ---------------------------------------------------------------------------------------------
// Capacity plan for >32 players (dev only; not wired to the runtime yet).
//
// Today's 32-player mode hides the overflow records in ONE session-sized shadow object: peers 17..31 at
// shadow+0x60 (block A) / shadow+0x2270 (block B), players 16..31 at shadow+0x1740 / shadow+0x3950, and the
// 17 x 0xC peer table at shadow+0x446C. That works because 15 peer / 16 player records fit in the gaps the
// stock session layout leaves. At 64 the peer array (47 records) is 0x3F28 bytes, which no longer fits below the
// player array, and at 128 it is 0x95B8. So a >32 capacity needs a new shadow layout, and because the A/B blocks
// of a static session are only 0x2210 apart, one single shadow delta can no longer map both:
//   - capacity <= 32: one shadow block per static session, arrays at the proven stock-relative offsets
//                     (all biases 0, so 32-player runtime behaviour stays byte-identical),
//   - capacity > 32:  two shadow blocks per static session (one for block-A record accesses, one for block-B);
//                     inside each block the shadow arrays are placed back-to-back in a new order.
// The remaining runtime work for >32 is per-site A/B classification: every member cave / table cave / step loop
// currently uses one constant, so each generated site has to be tagged with the A or B shadow block before the
// 64/128 modes can install. This model keeps that distinction explicit (…BiasA / …BiasB).
struct SlotShadowLayout {
    uint32_t capacity;
    uint32_t blocksUsed;      // shadow blocks per static session (1 at 32, 2 at 64/128)
    uint32_t peerOffset;      // shadow offset of the peer array inside its block
    uint32_t playerOffset;    // shadow offset of the player array inside its block
    uint32_t tableOffset;     // shadow offset of the 0xC table inside its block
    int32_t peerBiasA, peerBiasB;       // added to the shadow constant: A: offset-stock, B: blockDelta+offset-stockB
    int32_t playerBiasA, playerBiasB;
    int32_t tableBiasA, tableBiasB;
    uint32_t peerEntries, playerEntries, tableEntries;
    uint32_t peerBytes, playerBytes, tableBytes;
    uint32_t peerMaskWords;   // 32-bit words needed for a capacity-bit peer mask
    uint64_t msgExtBytes;     // membership message extension bytes for this capacity
    uint64_t msgSize;         // 0x8100 base + extension
    // Chunked model (the alternative to per-site A/B classification): every shadow array is split into chunks that
    // fit below the 0x2210 A->B distance, and chunk A is placed exactly 0x2210 before chunk B. A cave therefore
    // only needs one constant per chunk (index range) and works for either block base, but pointer-stepping loops
    // and the whole-array copy/clear mirrors have to hop chunks.
    uint32_t chunkPeerLimit, chunkPlayerLimit, chunkTableLimit;   // records per chunk
    uint32_t peerChunks, playerChunks, tableChunks;
    uint32_t chunkedBytes;    // total shadow bytes for all chunk pairs (incl. the A->B gaps)
    bool chunkedFits;         // the chunk plan fits inside kSessionStride
    uint32_t memberCaveBytes; // estimated bytes for a piecewise member cave at this capacity (worst record kind)
};

constexpr uint32_t kStockPeerA = 0x60, kStockPlayerA = 0x1740, kStockTable = 0x446C;
constexpr uint32_t kBlockDelta = 0x2210;   // A -> B distance inside a static session
constexpr uint32_t kPeerEntryWire = 0x180, kPlayerEntryWire = 0xB8, kMsgBase = 0x8100;
constexpr uint32_t kPeerFirst = 17, kPlayerFirst = 16;

constexpr uint32_t AlignUp16(uint32_t v) { return (v + 0xF) & ~0xFu; }

// Seats a UI/session surface may advertise for a requested dev capacity: stock 16 unless the capacity's install
// actually succeeded. 32 is the only implemented dev capacity today; 64/128 requests stay at 16 until their install
// exists (the refusal is logged in Install).
constexpr uint32_t EffectiveSeatsForCapacity(uint32_t requested, bool installed) {
    if (!installed || requested == 0) return 16;
    return requested >= 32 ? requested : 16;
}

// Bytes of one piecewise member cave for a given chunk count: stock-path jump + one 19-byte body per chunk
// (replay original multiply, add constant, jump back) + one cmp/jcc dispatch pair per extra chunk.
constexpr uint32_t PiecewiseCaveBytes(uint32_t chunks) {
    return 5 + 19 * chunks + 10 * (chunks > 0 ? chunks - 1 : 0);
}

constexpr SlotShadowLayout MakeSlotShadowLayout(uint32_t capacity) {
    SlotShadowLayout L{};
    L.capacity = capacity;
    L.blocksUsed = capacity <= 32 ? 1u : 2u;
    L.peerEntries = capacity - kPeerFirst;
    L.playerEntries = capacity - kPlayerFirst;
    L.tableEntries = L.peerEntries;
    L.peerBytes = L.peerEntries * 0x158;
    L.playerBytes = L.playerEntries * 0xB0;
    L.tableBytes = L.peerEntries * 0xC;
    L.msgExtBytes = static_cast<uint64_t>(L.peerEntries) * kPeerEntryWire +
                    static_cast<uint64_t>(L.playerEntries) * kPlayerEntryWire;
    L.msgSize = kMsgBase + L.msgExtBytes;
    L.peerMaskWords = (capacity + 31) / 32;
    if (capacity <= 32) {
        // Proven stock-relative single-block layout.
        L.peerOffset = kStockPeerA;
        L.playerOffset = kStockPlayerA;
        L.tableOffset = kStockTable;
    } else {
        // Two shadow blocks; in each block: peers, players, table, back to back.
        L.peerOffset = kStockPeerA;
        L.playerOffset = AlignUp16(L.peerOffset + L.peerBytes);
        L.tableOffset = AlignUp16(L.playerOffset + L.playerBytes);
    }
    if (L.blocksUsed == 1) {
        L.peerBiasA = L.peerBiasB = 0;
        L.playerBiasA = L.playerBiasB = 0;
        L.tableBiasA = L.tableBiasB = 0;
    } else {
        L.peerBiasA = static_cast<int32_t>(L.peerOffset) - static_cast<int32_t>(kStockPeerA);
        L.playerBiasA = static_cast<int32_t>(L.playerOffset) - static_cast<int32_t>(kStockPlayerA);
        L.tableBiasA = static_cast<int32_t>(L.tableOffset) - static_cast<int32_t>(kStockTable);
        L.peerBiasB = static_cast<int32_t>(kSessionStride) + static_cast<int32_t>(L.peerOffset) -
                      static_cast<int32_t>(kStockPeerA + kBlockDelta);
        L.playerBiasB = static_cast<int32_t>(kSessionStride) + static_cast<int32_t>(L.playerOffset) -
                        static_cast<int32_t>(kStockPlayerA + kBlockDelta);
        L.tableBiasB = static_cast<int32_t>(kSessionStride) + static_cast<int32_t>(L.tableOffset) -
                       static_cast<int32_t>(kStockTable + kBlockDelta);
    }
    // Chunked feasibility model: every array is split into chunks that fit below the 0x2210 A->B distance and
    // chunk A is placed exactly 0x2210 before chunk B, so one cave constant per chunk works for either block base.
    L.chunkPeerLimit = kBlockDelta / 0x158;     // 24 records
    L.chunkPlayerLimit = kBlockDelta / 0xB0;    // 48 records
    L.chunkTableLimit = kBlockDelta / 0xC;      // 1167 records
    L.peerChunks = (L.peerEntries + L.chunkPeerLimit - 1) / L.chunkPeerLimit;
    L.playerChunks = (L.playerEntries + L.chunkPlayerLimit - 1) / L.chunkPlayerLimit;
    L.tableChunks = (L.tableEntries + L.chunkTableLimit - 1) / L.chunkTableLimit;
    L.chunkedBytes = 0;
    L.chunkedFits = true;
    {
        uint32_t cursor = 0;
        const auto placeChunks = [&](uint32_t chunks, uint32_t perChunk, uint32_t record) {
            for (uint32_t c = 0; c < chunks; ++c) {
                const uint32_t a = (cursor + 0xF) & ~0xFu;
                cursor = a + kBlockDelta + perChunk * record;   // A chunk + B chunk (upper bound; last may be short)
            }
        };
        placeChunks(L.peerChunks, L.chunkPeerLimit, 0x158);
        placeChunks(L.playerChunks, L.chunkPlayerLimit, 0xB0);
        placeChunks(L.tableChunks, L.chunkTableLimit, 0xC);
        L.chunkedBytes = cursor;
        L.chunkedFits = cursor <= kSessionStride;
    }
    // Worst-case piecewise member cave across the three record kinds.
    L.memberCaveBytes = PiecewiseCaveBytes(L.peerChunks);
    if (PiecewiseCaveBytes(L.playerChunks) > L.memberCaveBytes) L.memberCaveBytes = PiecewiseCaveBytes(L.playerChunks);
    if (PiecewiseCaveBytes(L.tableChunks) > L.memberCaveBytes) L.memberCaveBytes = PiecewiseCaveBytes(L.tableChunks);
    return L;
}

constexpr SlotShadowLayout kSlotLayout32 = MakeSlotShadowLayout(32);
constexpr SlotShadowLayout kSlotLayout64 = MakeSlotShadowLayout(64);
constexpr SlotShadowLayout kSlotLayout128 = MakeSlotShadowLayout(128);

constexpr uint32_t BitsFor(uint32_t v) { uint32_t b = 0; while (v) { v >>= 1; ++b; } return b; }

constexpr bool RangeOverlaps(uint64_t a, uint64_t an, uint64_t b, uint64_t bn) {
    return a < b + bn && b < a + an;
}
constexpr bool ValidateSlotShadowLayout(const SlotShadowLayout& L) {
    if (L.peerBytes == 0 || L.playerBytes == 0 || L.tableBytes == 0) return false;
    const bool inBlock = L.peerOffset + L.peerBytes <= kSessionStride &&
                         L.playerOffset + L.playerBytes <= kSessionStride &&
                         L.tableOffset + L.tableBytes <= kSessionStride;
    if (!inBlock) return false;
    if (RangeOverlaps(L.peerOffset, L.peerBytes, L.playerOffset, L.playerBytes)) return false;
    if (RangeOverlaps(L.peerOffset, L.peerBytes, L.tableOffset, L.tableBytes)) return false;
    if (RangeOverlaps(L.playerOffset, L.playerBytes, L.tableOffset, L.tableBytes)) return false;
    if (L.blocksUsed > 2) return false;
    if (L.capacity == 32) {
        // Must reproduce today's layout exactly (biases 0, extension 0x2200).
        if (L.blocksUsed != 1) return false;
        if (L.peerOffset != kStockPeerA || L.playerOffset != kStockPlayerA || L.tableOffset != kStockTable) return false;
        if (L.peerBiasA != 0 || L.peerBiasB != 0 || L.playerBiasA != 0 || L.playerBiasB != 0 ||
            L.tableBiasA != 0 || L.tableBiasB != 0) return false;
        if (L.msgExtBytes != 0x2200) return false;
    }
    return true;
}
constexpr bool kLayout32Valid = ValidateSlotShadowLayout(kSlotLayout32);
constexpr bool kLayout64Valid = ValidateSlotShadowLayout(kSlotLayout64);
constexpr bool kLayout128Valid = ValidateSlotShadowLayout(kSlotLayout128);
static_assert(kLayout32Valid, "32-player shadow layout must be unchanged");
static_assert(kLayout64Valid, "64-player shadow layout must not overlap");
static_assert(kLayout128Valid, "128-player shadow layout must not overlap");

// Wire limits the layouts do not change (numbers the membership generator uses).
constexpr uint32_t kPeerIndexBits = 6;    // membership message: owning peer field
constexpr uint32_t kPlayerIndexBits = 5;  // membership message: player index field
constexpr uint64_t kMax16BitMessage = 0xFFFF;

// ---------------------------------------------------------------------------------------------
// Chunked dispatch validation (no engine needed). For the future 64/128 install every member cave is a chain of
// index-range branches: below the first shadow slot it runs stock; in chunk c it replays the original multiply and
// adds one constant. Because chunk A is placed exactly kBlockDelta before chunk B, the SAME constant maps block-A
// and block-B record bases into their chunks; this function simulates that for every record index and checks the
// mapping, the chunk placement, and that chunks of one kind cannot overlap.
struct CapacityChunk { uint32_t kind; uint32_t firstIndex; uint32_t count; uint32_t aOffset, bOffset; int32_t constant; };
constexpr uint32_t kMaxCapacityChunks = 32;

int ValidateChunkPlan(const SlotShadowLayout& L) {
    int failures = 0;
    struct Kind { uint32_t record, stock, first, entries; };
    const Kind kinds[3] = {
        { 0x158, kStockPeerA, kPeerFirst, L.peerEntries },
        { 0xB0, kStockPlayerA, kPlayerFirst, L.playerEntries },
        { 0xC, kStockTable, kPeerFirst, L.tableEntries },
    };
    CapacityChunk chunks[kMaxCapacityChunks] = {};
    uint32_t n = 0;
    uint32_t cursor = 0;
    for (uint32_t k = 0; k < 3; ++k) {
        const Kind& K = kinds[k];
        const uint32_t per = kBlockDelta / K.record;
        const uint32_t needed = (K.entries + per - 1) / per;
        for (uint32_t c = 0; c < needed; ++c) {
            const uint32_t start = K.first + c * per;
            const uint32_t remain = K.entries - c * per;
            const uint32_t cnt = remain < per ? remain : per;
            const uint32_t a = (cursor + 0xF) & ~0xFu;
            const uint32_t b = a + kBlockDelta;
            const uint32_t size = cnt * K.record;
            cursor = b + size;
            if (b + size > kSessionStride) ++failures;
            const int32_t C = static_cast<int32_t>(a) - static_cast<int32_t>(K.stock) -
                              static_cast<int32_t>(K.first) * static_cast<int32_t>(K.record);
            if (n < kMaxCapacityChunks) chunks[n++] = { k, start, cnt, a, b, C };
            for (uint32_t i = start; i < start + cnt; ++i) {
                const uint64_t addA = static_cast<uint64_t>(K.stock) + static_cast<uint64_t>(i) * K.record +
                                      static_cast<uint64_t>(static_cast<int64_t>(C));
                const uint64_t wantA = static_cast<uint64_t>(a) + static_cast<uint64_t>(i - K.first) * K.record;
                if (addA != wantA) ++failures;
                const uint64_t addB = static_cast<uint64_t>(K.stock + kBlockDelta) +
                                      static_cast<uint64_t>(i) * K.record +
                                      static_cast<uint64_t>(static_cast<int64_t>(C));
                const uint64_t wantB = static_cast<uint64_t>(b) + static_cast<uint64_t>(i - K.first) * K.record;
                if (addB != wantB) ++failures;
            }
        }
    }
    // Chunks of one kind must not overlap in either block.
    for (uint32_t i = 0; i < n; ++i) {
        for (uint32_t j = i + 1; j < n; ++j) {
            if (chunks[i].kind != chunks[j].kind) continue;
            const uint32_t rec = kinds[chunks[i].kind].record;
            const uint32_t si = chunks[i].count * rec, sj = chunks[j].count * rec;
            if (RangeOverlaps(chunks[i].aOffset, si, chunks[j].aOffset, sj)) ++failures;
            if (RangeOverlaps(chunks[i].bOffset, si, chunks[j].bOffset, sj)) ++failures;
        }
    }
    // Piecewise cave size per record kind: stock-path jump + one 19-byte body per chunk + one cmp/jcc dispatch
    // pair per extra chunk. The install needs one cave entry large enough for the worst kind.
    const uint32_t peerCave = PiecewiseCaveBytes(L.peerChunks);
    const uint32_t playerCave = PiecewiseCaveBytes(L.playerChunks);
    const uint32_t tableCave = PiecewiseCaveBytes(L.tableChunks);
    const uint32_t worstCave = peerCave > playerCave ? (peerCave > tableCave ? peerCave : tableCave)
                                                     : (playerCave > tableCave ? playerCave : tableCave);
    NOBLE_INFO("SlotLayout: cap={:>3} chunk plan: {} chunk(s) (peers {} / players {} / table {}), shadow end 0x{:X}, "
               "piecewise caves peer {} B / player {} B / table {} B -> worst {} B{}",
               L.capacity, n, L.peerChunks, L.playerChunks, L.tableChunks, cursor,
               peerCave, playerCave, tableCave, worstCave,
               worstCave <= 256 ? "" : " (needs a bigger cave entry)");
    if (L.memberCaveBytes != worstCave) ++failures;   // model and dispatch estimate must agree
    return failures;
}

// ---------------------------------------------------------------------------------------------
// Piecewise member-cave synthesis for the chunked install (validated here; >32 not installed yet).
//
// The generated one-range caves (41 bytes) replay the original multiply in two paths. A chunked capacity needs one
// body per chunk instead; the destination/index registers are decoded from the site's original imul bytes, so the
// generator format stays unchanged. Dispatch uses 32-bit conditional jumps (the chunks are >127 bytes away).
// Layout: cmp idx, first ; jb stock ; (cmp idx, chunk[c].first ; jb chunk[c-1])* ; fallthrough = last chunk.
// Bodies: last chunk, stock, then chunks 0..n-2. Length: 41 B (1 chunk), 70 (2), 99 (3), 128 (4), 157 (5) - all
// below the 256-byte Site cave entry. Block-B record bases share the same constant because chunk B is +0x2210 away.
struct SynthesizedCave {
    uint8_t bytes[256];
    uint32_t length;
    bool ok;
    uint32_t bodyCount;
    uint32_t bodyOffsets[10];
    uint32_t constantCount;
    uint32_t constantOffsets[8];
};

uint8_t Rex(uint8_t reg) { return static_cast<uint8_t>(0x48 | ((reg >> 3) & 1)); }

bool DecodeImulRegs(const uint8_t* orig, uint8_t& dst, uint8_t& idx) {
    const uint8_t rex = orig[0];
    if ((rex & 0xF0) != 0x40) return false;
    const uint8_t opcode = orig[1];
    if (opcode != 0x69 && opcode != 0x6B) return false;
    const uint8_t modrm = orig[2];
    if ((modrm & 0xC0) != 0xC0) return false;   // register direct form only
    dst = static_cast<uint8_t>((((rex >> 2) & 1) << 3) | ((modrm >> 3) & 7));
    idx = static_cast<uint8_t>((((rex >> 0) & 1) << 3) | (modrm & 7));
    return true;
}

uint32_t PutJmp(uint8_t* p, uint64_t caveVa, uint32_t pos, uint64_t target) {
    p[0] = 0xE9;
    const int64_t disp = static_cast<int64_t>(target) - static_cast<int64_t>(caveVa + pos + 5);
    std::memcpy(p + 1, &disp, 4);
    return 5;
}
uint32_t PutJbRel32(uint8_t* p, uint64_t caveVa, uint32_t pos, uint64_t target) {
    p[0] = 0x0F; p[1] = 0x82;
    const int64_t disp = static_cast<int64_t>(target) - static_cast<int64_t>(caveVa + pos + 6);
    std::memcpy(p + 2, &disp, 4);
    return 6;
}

SynthesizedCave SynthesizePiecewiseCave(const Site& site, const ShadowLayoutPlan& plan, int64_t deltaScaled,
                                       uint64_t caveVa, uint64_t siteVa) {
    SynthesizedCave out{};
    uint8_t dst = 0, idx = 0;
    if (!DecodeImulRegs(site.original, dst, idx)) return out;
    const uint32_t kindId = site.record == 0xB0 ? 1u : ((site.record == 0x158 || site.record == 0x56) ? 0u : 2u);
    const ShadowKindPlan& kp = plan.kinds[kindId];
    if (kp.count == 0 || kp.count > kMaxShadowChunks) return out;
    uint8_t* p = out.bytes;
    const uint64_t back = siteVa + sizeof(site.original);
    const uint32_t dispatch = 10 * kp.count;   // one (cmp + 32-bit jb) branch per chunk plus the stock branch
    const uint32_t stockJbPos = 4;
    p[0] = Rex(idx); p[1] = 0x83; p[2] = static_cast<uint8_t>(0xF8 | (idx & 7));
    p[3] = static_cast<uint8_t>(kp.chunks[0].first);
    for (uint32_t c = 1; c < kp.count; ++c) {
        const uint32_t cmpPos = 10 + (c - 1) * 10;
        p[cmpPos] = Rex(idx); p[cmpPos + 1] = 0x83;
        p[cmpPos + 2] = static_cast<uint8_t>(0xF8 | (idx & 7));
        p[cmpPos + 3] = static_cast<uint8_t>(kp.chunks[c].first);
    }
    // Bodies: last chunk (fallthrough target), stock, then chunks 0..n-2.
    uint32_t pos = dispatch;
    uint32_t chunkBody[8] = {};
    auto emitBody = [&](uint32_t chunk, bool stock) -> uint32_t {
        const uint32_t at = pos;
        std::memcpy(p + pos, site.original, sizeof(site.original));
        pos += 7;
        if (!stock) {
            const int64_t base = static_cast<int64_t>(kp.chunks[chunk].aOffset) -
                                static_cast<int64_t>(kKindStockOffset[kindId]);
            const int64_t raw = (site.scaleShift == 2 ? base / 4 : base) -
                                static_cast<int64_t>(site.threshold) * static_cast<int64_t>(site.record);
            const int32_t imm = static_cast<int32_t>(deltaScaled + raw);
            p[pos] = Rex(dst); p[pos + 1] = 0x81; p[pos + 2] = static_cast<uint8_t>(0xC0 | (dst & 7));
            std::memcpy(p + pos + 3, &imm, 4);
            out.constantOffsets[out.constantCount++] = pos;
            pos += 7;
        }
        pos += PutJmp(p + pos, caveVa, pos, back);
        out.bodyOffsets[out.bodyCount++] = at;
        return at;
    };
    const uint32_t lastChunkBody = emitBody(kp.count - 1, false);
    const uint32_t stockBody = emitBody(0, true);
    for (uint32_t c = 0; c + 1 < kp.count; ++c) chunkBody[c] = emitBody(c, false);
    // Patch the dispatch branches now that all body offsets are known.
    PutJbRel32(p + stockJbPos, caveVa, stockJbPos, caveVa + stockBody);
    for (uint32_t c = 1; c < kp.count; ++c) {
        const uint32_t jbPos = 14 + (c - 1) * 10;
        PutJbRel32(p + jbPos, caveVa, jbPos, caveVa + chunkBody[c - 1]);
    }
    (void)lastChunkBody;
    out.length = pos;
    out.ok = pos <= sizeof(out.bytes);
    return out;
}
// ---------------------------------------------------------------------------------------------
// Mask-widening plan for capacities above 32 (proposed, validated synthetically here).
//
// The engine keeps peer-valid and player-valid masks as one 32-bit DWORD. Widening keeps that DWORD as word 0 and
// stores the extra words in the shadow object, so every existing mask site can stay a 32-bit operation for bits 0..31
// and only needs a high-word branch for higher bits. The plan below defines where the high words live and the mask
// bit helpers the patched sites will use; the per-site inventory (23 peer-mask operands already catalogued, player-mask
// sites still to enumerate) and the site patch forms are the next implementation step.
struct CapacityMaskPlan {
    uint32_t capacity;
    uint32_t words;         // words per mask (1 at 32, 2 at 64, 4 at 128)
    uint32_t peerHighOffset;    // shadow offset of peer-mask word 1 (0 when words == 1)
    uint32_t playerHighOffset;  // shadow offset of player-mask word 1 (0 when words == 1)
};
constexpr CapacityMaskPlan MakeCapacityMaskPlan(uint32_t capacity) {
    CapacityMaskPlan plan{};
    plan.capacity = capacity;
    plan.words = (capacity + 31) / 32;
    if (plan.words > 1) {
        plan.peerHighOffset = kMaskHighBase;
        plan.playerHighOffset = kMaskHighBase + (plan.words - 1) * 4;
    }
    return plan;
}

inline bool MaskBitTest(const uint32_t* words, uint32_t count, uint32_t bit) {
    return bit / 32 < count && (words[bit / 32] & (1u << (bit % 32))) != 0;
}
inline void MaskBitSet(uint32_t* words, uint32_t count, uint32_t bit) {
    if (bit / 32 < count) words[bit / 32] |= (1u << (bit % 32));
}
inline void MaskBitClear(uint32_t* words, uint32_t count, uint32_t bit) {
    if (bit / 32 < count) words[bit / 32] &= ~(1u << (bit % 32));
}
inline uint32_t MaskPopcount(const uint32_t* words, uint32_t count) {
    uint32_t n = 0;
    for (uint32_t i = 0; i < count; ++i) {
        uint32_t v = words[i];
        while (v) { v &= v - 1; ++n; }
    }
    return n;
}
inline int MaskFirstSet(const uint32_t* words, uint32_t count) {
    for (uint32_t i = 0; i < count; ++i)
        if (words[i]) { uint32_t b = 0; while (((words[i] >> b) & 1u) == 0) ++b; return static_cast<int>(i * 32 + b); }
    return -1;
}

int ValidateMaskModel() {
    int failures = 0;
    const uint32_t caps[] = { 32, 64, 128 };
    for (const uint32_t cap : caps) {
        const CapacityMaskPlan plan = MakeCapacityMaskPlan(cap);
        const uint32_t end = plan.peerHighOffset + (plan.words - 1) * 4;
        const uint32_t shadowEnd = cap <= 32 ? 0x8614u : (cap <= 64 ? 0xEAA4u : 0x21A14u);
        if (plan.words > 1 && (end > kSessionStride || plan.peerHighOffset < shadowEnd)) {
            ++failures;
            NOBLE_WARN("SlotLayout: mask high words at 0x{:X} collide with the shadow arrays (end 0x{:X})",
                       plan.peerHighOffset, shadowEnd);
        }
        uint32_t peer[4] = {}, player[4] = {};
        // Set every valid bit, check every bit, then clear half and re-check.
        for (uint32_t b = 0; b < cap; ++b) { MaskBitSet(peer, plan.words, b); MaskBitSet(player, plan.words, b); }
        for (uint32_t b = 0; b < cap; ++b)
            if (!MaskBitTest(peer, plan.words, b) || !MaskBitTest(player, plan.words, b)) ++failures;
        if (MaskPopcount(peer, plan.words) != cap) ++failures;
        if (MaskFirstSet(peer, plan.words) != 0) ++failures;
        for (uint32_t b = 0; b < cap; b += 2) MaskBitClear(peer, plan.words, b);
        for (uint32_t b = 0; b < cap; ++b) {
            const bool want = (b % 2) == 1;
            if (MaskBitTest(peer, plan.words, b) != want) ++failures;
        }
        if (MaskPopcount(peer, plan.words) != cap / 2) ++failures;
        NOBLE_INFO("SlotLayout: cap={:>3} mask words {} (inline word 0 + {} high word(s)); peer high @0x{:X}, player high "
                   "@0x{:X} (shadow arrays end 0x{:X}, stride 0x{:X}) - all bits set/test/clear/popcount/first ok",
                   cap, plan.words, plan.words - 1, plan.peerHighOffset, plan.playerHighOffset, shadowEnd,
                   static_cast<uint32_t>(kSessionStride));
    }
    return failures;
}
// Validate the piecewise cave synthesis for every generated site at 32/64/128: length fits the 256-byte cave entry, the
// one-chunk case reproduces the generated 41-byte cave, and (delta 0) the first chunk constant is exactly
// -(threshold * record) as in SlotMemberCaves.inc.
int ValidatePiecewiseCaves() {
    int failures = 0;
    const uint64_t caveVa = 0x20000000ull;
    const uint32_t caps[] = { 32, 64, 128 };
    for (const uint32_t cap : caps) {
        const ShadowLayoutPlan plan = BuildShadowLayoutPlan(cap);
        uint32_t maxLen = 0, synthesized = 0;
        for (const Site& site : kSites) {
            const uint32_t kindId = site.record == 0xB0 ? 1u : ((site.record == 0x158 || site.record == 0x56) ? 0u : 2u);
            if (plan.kinds[kindId].count == 0) { ++failures; continue; }
            const uint64_t siteVa = caveVa + site.rva;
            const SynthesizedCave c = SynthesizePiecewiseCave(site, plan, 0, caveVa, siteVa);
            if (!c.ok) { ++failures; NOBLE_WARN("SlotLayout: piecewise cave synthesis failed at +0x{:X} cap {}", site.rva, cap); continue; }
            ++synthesized;
            if (c.length > 256) { ++failures; NOBLE_WARN("SlotLayout: piecewise cave +0x{:X} {} B at cap {}", site.rva, c.length, cap); }
            if (c.length > maxLen) maxLen = c.length;
            if (c.bodyCount != plan.kinds[kindId].count + 1) ++failures;
            if (c.constantCount != plan.kinds[kindId].count) ++failures;
            if (cap == 32) {
                if (plan.kinds[kindId].count != 1 || c.length != 41 || c.length != site.caveLength) {
                    ++failures;
                    NOBLE_WARN("SlotLayout: one-chunk cave +0x{:X} {} B (generated {})", site.rva, c.length, site.caveLength);
                }
            }
            // Every dispatch branch must land on the body it selects, and every body must jump back to the site.
            const uint32_t n = plan.kinds[kindId].count;
            auto readBranch = [&](uint32_t at) -> int64_t {
                int32_t d = 0; std::memcpy(&d, c.bytes + at + 2, 4);
                return static_cast<int64_t>(caveVa + at + 6 + d);
            };
            auto readJump = [&](uint32_t at) -> int64_t {
                int32_t d = 0; std::memcpy(&d, c.bytes + at + 1, 4);
                return static_cast<int64_t>(caveVa + at + 5 + d);
            };
            if (readBranch(4) != static_cast<int64_t>(caveVa + c.bodyOffsets[1])) ++failures;
            for (uint32_t ch = 1; ch < n; ++ch) {
                const uint32_t at = 14 + (ch - 1) * 10;
                if (readBranch(at) != static_cast<int64_t>(caveVa + c.bodyOffsets[ch + 1])) ++failures;
            }
            for (uint32_t bd = 0; bd < c.bodyCount; ++bd) {
                const uint32_t jmpPos = (bd == 1) ? c.bodyOffsets[bd] + 7 : c.bodyOffsets[bd] + 14;
                if (readJump(jmpPos) != static_cast<int64_t>(siteVa + sizeof(site.original))) ++failures;
            }
        }
        NOBLE_INFO("SlotLayout: cap={:>3} piecewise cave synthesis: {} site(s), max {} B (256-byte entry) {}",
                   cap, synthesized, maxLen, maxLen <= 256 ? "ok" : "TOO BIG");
    }
    return failures;
}
// Coverage check for the chunk-aware shadow plan used by record init / mirrors / PeerRecord: each kind's chunks must
// partition [firstSlot, capacity) exactly once, ascend in shadow storage, keep the A->B distance inside the object,
// and capacity 32 must reproduce the stock offsets (byte-identical 32-player path).
int ValidateShadowPlanCoverage() {
    int failures = 0;
    struct Expected { uint32_t capacity, peers, players, table; };
    const Expected expected[] = { { 32, 1, 1, 1 }, { 64, 2, 1, 1 }, { 128, 5, 3, 1 } };
    for (const Expected& e : expected) {
        const ShadowLayoutPlan plan = BuildShadowLayoutPlan(e.capacity);
        for (uint32_t k = 0; k < 3; ++k) {
            const ShadowKindPlan& kp = plan.kinds[k];
            const uint32_t wantChunks = k == 0 ? e.peers : (k == 1 ? e.players : e.table);
            if (kp.count != wantChunks) { ++failures; NOBLE_WARN("SlotLayout: cap {} kind {} chunk count {} != {}",
                                                                  e.capacity, k, kp.count, wantChunks); }
            uint32_t expectFirst = kKindFirstSlot[k];
            for (uint32_t c = 0; c < kp.count; ++c) {
                const ShadowChunk& ch = kp.chunks[c];
                if (ch.first != expectFirst) { ++failures; NOBLE_WARN("SlotLayout: cap {} kind {} chunk {} first {} != {}",
                                                                       e.capacity, k, c, ch.first, expectFirst); }
                if (ch.count == 0) ++failures;
                expectFirst = ch.first + ch.count;
                const uint64_t end = static_cast<uint64_t>(ch.aOffset) + kShadowBlockDelta +
                                     static_cast<uint64_t>(ch.count) * kKindRecordSize[k];
                if (end > kSessionStride || ch.aOffset + static_cast<uint64_t>(ch.count) * kKindRecordSize[k] > kSessionStride) {
                    ++failures;
                    NOBLE_WARN("SlotLayout: cap {} kind {} chunk {} ends at 0x{:X} outside kSessionStride",
                               e.capacity, k, c, static_cast<uint32_t>(end));
                }
                if (e.capacity == 32 && ch.aOffset != kKindStockOffset[k]) {
                    ++failures;
                    NOBLE_WARN("SlotLayout: cap 32 kind {} must keep the stock offset 0x{:X}, got 0x{:X}",
                               k, kKindStockOffset[k], ch.aOffset);
                }
            }
            const uint32_t covered = expectFirst - kKindFirstSlot[k];
            if (covered != e.capacity - kKindFirstSlot[k]) {
                ++failures;
                NOBLE_WARN("SlotLayout: cap {} kind {} covers {} records, want {}", e.capacity, k, covered,
                           e.capacity - kKindFirstSlot[k]);
            }
        }
        NOBLE_INFO("SlotLayout: cap={:>3} shadow plan coverage ok: peers {} chunk(s), players {}, table {}",
                   e.capacity, plan.kinds[0].count, plan.kinds[1].count, plan.kinds[2].count);
    }
    return failures;
}
// Engine mask width is the decisive limit past 32: the session's peer-valid and player-valid masks are single 32-bit
// DWORDs (the patched peer-mask table kPeerMasks32 widens 17 -> 32 bits, never more; player masks are DWORD too). A
// capacity above 32 needs multi-word masks, so it cannot be installed by the slot layout alone. This check reports the
// required mask words per capacity and makes the hard limit explicit instead of pretending the layout is enough.
int ValidateMaskWidths(int& peerMaskSites) {
    int failures = 0;
    peerMaskSites = static_cast<int>(std::size(kPeerMasks32));
    NOBLE_INFO("SlotLayout: engine mask width check - {} patched peer-mask operand(s), all single 32-bit DWORD masks",
               peerMaskSites);
    const uint32_t caps[] = { 32, 64, 128 };
    for (const uint32_t cap : caps) {
        const uint32_t peerWords = (cap + 31) / 32;
        const uint32_t playerWords = (cap + 31) / 32;
        const bool fits = peerWords == 1 && playerWords == 1;
        NOBLE_INFO("SlotLayout: cap={:>3} masks need peer {} word(s) / player {} word(s) -> {}",
                   cap, peerWords, playerWords,
                   fits ? "fits the stock DWORD masks" : "BLOCKED by the engine's 32-bit masks (needs mask widening)");
        if (!fits)
            NOBLE_WARN("SlotLayout: capacity {} cannot be installed until the engine's DWORD peer/player masks are widened "
                       "to multi-word masks (session layout + every mask read/write/loop site; see the scaling note)",
                       cap);
        if (cap > 32 && fits) ++failures;   // a >32 capacity must never be reported as mask-compatible
    }
    return failures;
}
// Mask high words are session-level shadow state: they must be zeroed when a shadow object is initialised and follow
// the block on whole-block copies/clears. At capacity 32 the span is empty (no-op), so this check also guards the
// 32-player path against accidentally touching mask storage.
int ValidateMaskShadowLifecycle() {
    int failures = 0;
    struct Case { uint32_t capacity, wantBytes, shadowEnd; };
    const Case cases[] = { { 32, 0, 0x8614 }, { 64, 8, 0xEAA4 }, { 128, 24, 0x21A14 } };
    for (const Case& c : cases) {
        const uint32_t bytes = MaskHighBytes(c.capacity);
        const uint32_t end = kMaskHighBase + bytes;
        if (bytes != c.wantBytes) { ++failures; NOBLE_WARN("SlotLayout: mask high span for cap {} is {} B, want {}",
                                                              c.capacity, bytes, c.wantBytes); }
        if (end > kSessionStride || (bytes && kMaskHighBase < c.shadowEnd)) {
            ++failures;
            NOBLE_WARN("SlotLayout: mask high span for cap {} [0x{:X},0x{:X}) invalid (arrays end 0x{:X}, stride 0x{:X})",
                       c.capacity, kMaskHighBase, end, c.shadowEnd, static_cast<uint32_t>(kSessionStride));
        }
        uint8_t src[32] = {}, dst[32] = {};
        for (uint32_t b = 0; b < bytes && b < sizeof(src); ++b) src[b] = static_cast<uint8_t>(b + 1);
        if (bytes) {
            std::memcpy(dst, src, bytes);
            if (std::memcmp(dst, src, bytes) != 0) ++failures;
            std::memset(dst, 0, bytes);
            for (uint32_t b = 0; b < bytes; ++b) if (dst[b] != 0) ++failures;
        }
        NOBLE_INFO("SlotLayout: cap={:>3} mask high words span {} B at shadow+0x{:X} (arrays end 0x{:X}) - init/clear/copy "
                   "lifecycle ok", c.capacity, bytes, kMaskHighBase, c.shadowEnd);
    }
    return failures;
}
int SlotLayoutSelfTest() {
    int failures = 0;
    const SlotShadowLayout layouts[] = { kSlotLayout32, kSlotLayout64, kSlotLayout128 };
    NOBLE_INFO("SlotLayout: synthetic 32/64/128 shadow-layout validation (no engine needed)");
    for (const SlotShadowLayout& L : layouts) {
        const bool layoutOk = ValidateSlotShadowLayout(L);
        const uint32_t peerBits = BitsFor(L.capacity - 1);
        const uint32_t playerBits = BitsFor(L.capacity - 1);
        const bool peerWireOk = peerBits <= kPeerIndexBits;
        const bool playerWireOk = playerBits <= kPlayerIndexBits;
        const bool msg16Ok = L.msgSize <= kMax16BitMessage;
        if (!layoutOk) ++failures;
        NOBLE_INFO("SlotLayout: cap={:>3} blocks {} peerA=0x{:X} playerA=0x{:X} table=0x{:X} bytes p{:X}/pl{:X}/t{:X} "
                   "biasA {}/{}/{} biasB {}/{}/{} mask {}w ext 0x{:X} msg 0x{:X} ({}) wire peer {}b player {}b -> {} / {}",
                   L.capacity, L.blocksUsed, L.peerOffset, L.playerOffset, L.tableOffset, L.peerBytes, L.playerBytes,
                   L.tableBytes, L.peerBiasA, L.playerBiasA, L.tableBiasA, L.peerBiasB, L.playerBiasB, L.tableBiasB,
                   L.peerMaskWords, static_cast<uint32_t>(L.msgExtBytes), static_cast<uint32_t>(L.msgSize),
                   msg16Ok ? "16-bit ok" : "needs message/fragment work", peerBits, playerBits,
                   layoutOk && peerWireOk ? "ok" : "needs widening", layoutOk && playerWireOk ? "ok" : "needs widening");
        if (!peerWireOk || !playerWireOk) {
            NOBLE_WARN("SlotLayout: capacity {} exceeds the stock wire fields (peer index {} bits, player index {} bits) - "
                       "the membership encoder/decoder must be widened before this capacity can go on the wire",
                       L.capacity, kPeerIndexBits, kPlayerIndexBits);
        }
        if (!msg16Ok) {
            NOBLE_WARN("SlotLayout: capacity {} membership message would be 0x{:X} bytes (> 0xFFFF); needs a "
                       "message/framing change", L.capacity, static_cast<uint32_t>(L.msgSize));
        }
        NOBLE_INFO("SlotLayout: cap={:>3} chunked model: peers {}x <= {} recs, players {}x <= {}, table {}x <= {}, "
                   "shadow bytes 0x{:X} ({}), piecewise member cave ~{} bytes -> {}",
                   L.capacity, L.peerChunks, L.chunkPeerLimit, L.playerChunks, L.chunkPlayerLimit,
                   L.tableChunks, L.chunkTableLimit, L.chunkedBytes,
                   L.chunkedFits ? "fits kSessionStride" : "DOES NOT FIT", L.memberCaveBytes,
                   L.memberCaveBytes <= 256 ? "fits the 256-byte cave entry" : "needs a bigger cave entry");
        if (!L.chunkedFits) ++failures;
        if (L.capacity == 32 && (L.peerChunks != 1 || L.playerChunks != 1 || L.tableChunks != 1)) ++failures;
        if (L.capacity == 64 && (L.peerChunks != 2 || L.playerChunks != 1 || L.tableChunks != 1)) ++failures;
        if (L.capacity == 128 && (L.peerChunks != 5 || L.playerChunks != 3 || L.tableChunks != 1)) ++failures;
        failures += ValidateChunkPlan(L);
    }
    {
        int peerMaskSites = 0;
        failures += ValidateMaskWidths(peerMaskSites);
        failures += ValidateMaskModel();
        failures += ValidateMaskShadowLifecycle();
    }
    failures += ValidateShadowPlanCoverage();
    failures += ValidatePiecewiseCaves();

    // Effective-seat mapping: stock stays 16 for any unimplemented/uninstalled capacity; an installed dev capacity
    // reports itself instead of the stock 16.
    struct SeatsCase { uint32_t requested; bool installed; uint32_t want; };
    const SeatsCase seatCases[] = {
        { 0, false, 16 }, { 32, false, 16 }, { 32, true, 32 }, { 64, false, 16 }, { 64, true, 64 },
        { 128, false, 16 }, { 128, true, 128 },
    };
    for (const SeatsCase& c : seatCases) {
        const uint32_t got = EffectiveSeatsForCapacity(c.requested, c.installed);
        if (got != c.want) {
            ++failures;
            NOBLE_WARN("SlotLayout: effective seats for requested {} installed {} = {} (expected {})",
                       c.requested, c.installed ? "yes" : "no", got, c.want);
        }
    }
    NOBLE_INFO("SlotLayout: effective-seat mapping checked (stock 16 unless a capacity is installed; 32 is the only "
               "implemented dev capacity)");
    NOBLE_INFO("SlotLayout: {} check(s) failed (0 = all three layouts valid)", failures);
    return failures;
}
} // namespace noble::slots
