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
    uint8_t cave[48];
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
void ResetShadowPeerPlayerTables(uint8_t* records, const Array& a) {
    if (!a.peer || a.table) return;
    for (uint32_t j = 0; j < kSlots32 - a.firstShadowSlot; ++j) {
        uint8_t* r = records + static_cast<size_t>(j) * a.record;
        std::memset(r + kPeerPlayerCount, 0, 4);
        std::memset(r + kPeerPlayerSlots, 0xFF, kPeerPlayerSlotCount * 4);
    }
}

void MirrorClear(void* dst, int value, size_t n) {
    uint32_t o = 0;
    const int k = StaticSession(dst, o);
    if (k < 0) return;
    ForCoveredArrays(o, n, [&](const Array& a) {
        std::memset(ShadowSlots(k, a), value, ShadowBytes(a));
        if (value == 0) ResetShadowPeerPlayerTables(ShadowSlots(k, a), a);
    });
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
        if (ks >= 0 && sa) std::memmove(ShadowSlots(kd, a), ShadowSlots(ks, *sa), ShadowBytes(a));
        else { std::memset(ShadowSlots(kd, a), 0, ShadowBytes(a)); ResetShadowPeerPlayerTables(ShadowSlots(kd, a), a); cleared = true; }
    });
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
    wchar_t env[8] = {};
    if (GetEnvironmentVariableW(L"NOBLE_SLOT_PATCH", env, 8) > 0 && env[0] == L'1') return true;
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    bool on = false;
    for (int i = 1; argv && i < argc; ++i) if (_wcsicmp(argv[i], L"-slot-patch") == 0) on = true;
    if (argv) LocalFree(argv);
    if (on) SetEnvironmentVariableW(L"NOBLE_SLOT_PATCH", L"1");   // carried into the relaunched match process
    return on;
}

// -slot-players 32 (NOBLE_SLOT_PLAYERS=32): also install the step caves and raise the loop bounds.
// Dev only until the fixed tables listed in docs/limits-players-loops.md are handled.
bool UnsafeOverride() {
    wchar_t unsafe[8] = {};
    return GetEnvironmentVariableW(L"NOBLE_SLOT_PLAYERS_UNSAFE", unsafe, 8) > 0 && unsafe[0] == L'1';
}

bool gPlayers32Active = false;
bool gMessageOn = true;

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
            // Shadow slot j sits at the same array offset as slot 0 (C = delta - threshold * record).
            const uint32_t count = players32 ? kSlots32 - a.firstShadowSlot : 0;
            for (uint32_t j = 0; j < count; ++j) {
                uint8_t* r = object + a.offset + j * a.record;
                if (a.table) { std::memset(r, 0, 4); std::memset(r + 4, 0xFF, 8); continue; }
                (a.peer ? peerCtor : playerCtor)(r);
            }
            if (count) ResetShadowPeerPlayerTables(object + a.offset, a);
        }
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

bool Players32Requested() { return ArgPlayers32() && UnsafeOverride(); }
bool Players32Active() { return gPlayers32Active; }

// Dev diagnostics: address of peer record `peer` (0x158 each from session + 0x60) of the static session that
// contains `session`, in the shadow object for peers >= 17. nullptr if 32-player mode is off or out of range.
uint8_t* PeerRecord(uint8_t* session, int32_t peer) {
    if (peer < 0 || peer >= 32 || !session) return nullptr;
    if (peer < 17) return session + 0x60 + static_cast<size_t>(peer) * 0x158;
    uint32_t o = 0;
    const int k = StaticSession(session, o);
    if (k < 0 || o != 0 || !gShadow) return nullptr;
    return gShadow + static_cast<size_t>(k) * kSessionStride + 0x60 + static_cast<size_t>(peer - 17) * 0x158;
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
    bool players32 = ArgPlayers32();
    // Safety gate: 32-player mode is known to be unsafe until the blockers in docs/limits-players-loops.md are
    // fixed (raised bounds already send slots >= 16 through uncaved code). Dev override only.
    if (players32) {
        if (!UnsafeOverride()) {
            NOBLE_WARN("SlotPatch: -slot-players 32 refused: not safe yet (169 untraced imul sites incl. 6 in 0x390DB0, "
                       "remaining step-3 tables, network encoding). "
                       "Dev override: NOBLE_SLOT_PLAYERS_UNSAFE=1. Staying in 16-player mode");
            players32 = false;
        } else {
            NOBLE_WARN("SlotPatch: NOBLE_SLOT_PLAYERS_UNSAFE=1 - enabling the known-unsafe 32-player mode (dev only)");
        }
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
        gSessions = sessions;
        gShadow = shadow;
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
        NOBLE_WARN("SlotPatch: 32-player mode (dev): {} step loops, {} loop bounds raised to 32, 0x390DB0 frame +0x100 "
                   "(32-entry name table), {} block copy/clear sites mirrored onto the shadow, 0x396828 S1/S2 peer side "
                   "areas, peer table +0x446C: {} sites caved, {} step loops (guard logs non-session blocks); other tables in "
                   "docs/limits-players-loops.md are still 16/17", std::size(kSteps), std::size(kBounds), std::size(kCopySites),
                   std::size(kTableSites), std::size(kTableSteps));
    else
        NOBLE_INFO("SlotPatch: 16-player mode; inert until -slot-players 32");
}

} // namespace noble::slots
