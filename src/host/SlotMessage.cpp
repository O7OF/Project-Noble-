#include "SlotMessage.hpp"
#include "SlotPatch.hpp"

#include "common/Logger.hpp"

#include <windows.h>
#include <cstdio>
#include <cstring>
#include <intrin.h>
#include <iterator>

namespace noble::slots::message {

namespace {

struct Fix { uint8_t at; uint8_t kind; uint32_t target; };   // kind 0: rel32 to engine RVA, 1: peer shadow C, 2: player shadow C
struct Cave {
    uint32_t rva;
    uint8_t origLen;
    uint8_t orig[10];
    uint8_t caveLen;
    uint8_t cave[48];
    uint8_t fixCount;
    Fix fix[4];
};
struct Edit {
    uint32_t rva;
    uint8_t len;
    uint8_t orig[8];
    uint8_t patched[8];
    uint8_t alt[8];   // second accepted original (EnginePatches security fix), same as orig otherwise
};

#include "SlotMessageTable.inc"

const Cave kCaves[] = { NOBLE_MSG_CAVES };
const Edit kEdits[] = { NOBLE_MSG_EDITS };
const Cave kTestCaves[] = { NOBLE_MSG_TEST_CAVES };

// The producer 0x39DD4C(block, other, ..., payload in r9) is only called from these two sites in 0x396828. They are
// redirected to ProducerWithExtension, which builds the payload's extension entries, then runs the producer.
constexpr uint32_t kProducerRva = 0x39DD4C;
constexpr uint32_t kProducerCalls[] = { 0x396A28, 0x396A57 };
constexpr uint32_t kPeerEntryCtorRva = 0x396E44;     // payload peer entry (0x180) ctor, rcx = entry
constexpr uint32_t kPlayerEntryCtorRva = 0x396E60;   // payload player entry (0xB8) ctor
constexpr uint32_t kExtPeers = 15;                   // peers 17..31
constexpr uint32_t kExtPlayers = 16;                 // players 16..31
constexpr uint32_t kPeerEntry = 0x180, kPlayerEntry = 0xB8;
static_assert(kExtPeers * kPeerEntry + kExtPlayers * kPlayerEntry == kMsgExtBytes, "extension layout");

constexpr int32_t kPeerFirstShadow = 17, kPeerRecord = 0x158;
constexpr int32_t kPlayerFirstShadow = 16, kPlayerRecord = 0xB0;

using ProducerFn = uint64_t (__fastcall*)(void*, void*, void*, uint8_t*);
using CtorFn = void* (__fastcall*)(void*);
ProducerFn gProducer = nullptr;
CtorFn gPeerCtor = nullptr;
CtorFn gPlayerCtor = nullptr;

volatile long gProducerCalls = 0;
uintptr_t gBase = 0;
constexpr uintptr_t kMessageTableRva = 0x4E38D28;   // pointer to the message type table (50 x 0x40)
constexpr uint32_t kMembershipEntry = 0x680;        // message 0x1A: +0x14 min size, +0x18 max size, +0x20 encode, +0x28 decode

// The registration (0x427E8C) is patched to register kMsgSize; if the table was filled before the patch went in, fix
// the entry here (the channel refuses a send larger than the registered maximum before it reaches the encoder).
void CheckRegisteredSize() {
    static bool logged = false;
    auto* table = *reinterpret_cast<uint8_t**>(gBase + kMessageTableRva);
    if (!table) return;
    auto* minSize = reinterpret_cast<uint32_t*>(table + kMembershipEntry + 0x14);
    auto* maxSize = reinterpret_cast<uint32_t*>(table + kMembershipEntry + 0x18);
    if (!logged) {
        logged = true;
        NOBLE_INFO("SlotPatch: membership message registered size min 0x{:X} max 0x{:X} (want 0x{:X})", *minSize, *maxSize, kMsgSize);
    }
    if (*minSize == 0x2550 && *maxSize == 0x2550) {
        *minSize = kMsgSize;
        *maxSize = kMsgSize;
        NOBLE_WARN("SlotPatch: membership message table entry was 0x2550; set to 0x{:X}", kMsgSize);
    }
}

// High-slot data repair (2026-11-16, ghidra-delta J.6 P1): while peer 17 is being established, an uncached peer
// property write can land in the player 0/1 records and set a player record's slot dword to an invalid value (4).
// The wire player entry only has a 2-bit slot field and the engine's encoder does not mask it, so slot 4 leaked
// into the peer bit that precedes it: the host sent "peer 0 slot 4", the guest decoded "peer 1 slot 0", the
// membership checksum no longer matched and the guest reset the session. Before the engine producer runs, repair
// any invalid slot in the live membership block from the authoritative peer->player mapping table; the engine then
// encodes (and checksums) consistent data. 32-player mode only; 16-player sessions never have an out-of-range slot
// and stay byte-identical.
void SanitizeMembershipBlock(uint8_t* block) {
    if (!block || !slots::Players32Active()) return;
    __try {
        uint8_t* session = block - 0x38;   // producer's block pointer is session+0x38 (peer records at block+0x28)
        const uint32_t playerMask = *reinterpret_cast<const uint32_t*>(block + 0x1704);   // session+0x173C
        for (int i = 0; i < 16; ++i) {
            if ((playerMask & (1u << i)) == 0) continue;
            uint8_t* rec = block + 0x1708 + static_cast<size_t>(i) * 0xB0;   // session+0x1740 + i*0xB0
            const uint32_t slot = *reinterpret_cast<const uint32_t*>(rec + 0x10);
            if (slot <= 3) continue;
            const int16_t peer = *reinterpret_cast<const int16_t*>(rec + 0x0C);
            int fixed = 0;
            if (uint8_t* peerRec = slots::PeerRecord(session, peer)) {
                for (int j = 0; j < 4; ++j)
                    if (*reinterpret_cast<const int32_t*>(peerRec + 0x144 + static_cast<size_t>(j) * 4) == i) fixed = j;
            }
            NOBLE_WARN("SlotPatch: membership producer repaired player {} slot {} -> {} (peer {}); invalid slot cannot be "
                       "put on the wire", i, slot, fixed, peer);
            *reinterpret_cast<uint32_t*>(rec + 0x10) = static_cast<uint32_t>(fixed);
        }
    } __except (1) {
        NOBLE_WARN("SlotPatch: membership block sanitizer read fault; producer payload left as-is");
    }
}

uint64_t __fastcall ProducerWithExtension(void* a, void* b, void* c, uint8_t* payload) {
    CheckRegisteredSize();
    SanitizeMembershipBlock(static_cast<uint8_t*>(b));
    if (payload) {
        uint8_t* ext = payload + kMsgExtOffset;
        std::memset(ext, 0, kMsgExtBytes);
        for (uint32_t i = 0; i < kExtPeers; ++i) gPeerCtor(ext + i * kPeerEntry);
        for (uint32_t j = 0; j < kExtPlayers; ++j) gPlayerCtor(ext + kExtPeers * kPeerEntry + j * kPlayerEntry);
    }
    const uint64_t r = gProducer(a, b, c, payload);
    if (InterlockedIncrement(&gProducerCalls) <= 12 && payload) {   // diagnostics for the first runs
        wchar_t probe[8] = {};
        const bool slotProbe = GetEnvironmentVariableW(L"NOBLE_SLOT_PROBE", probe, 8) > 0 && probe[0] == L'1';
        char extra[768] = {};
        if (slotProbe && gBase) {
            // Which payload is this (P1/P2 call site) and what did the producer put in it: the header ids, the tail
            // checksum and the first peer/player entries. Needed to explain why a guest rejects an update.
            const auto u32 = [payload](uint32_t off) { uint32_t v = 0; std::memcpy(&v, payload + off, 4); return v; };
            const int peers = *reinterpret_cast<int16_t*>(payload + 0x14);
            const int players = *reinterpret_cast<int16_t*>(payload + 0x16);
            size_t n = std::snprintf(extra, sizeof(extra), " hdr %08X %08X %08X cks %08X", u32(0x08), u32(0x0C),
                                     u32(0x10), u32(0x2548));
            for (int i = 0; i < peers && i < 3 && n < sizeof(extra) - 200; ++i) {
                const uint8_t* e = payload + 0x18 + static_cast<size_t>(i) * 0x180;
                n += std::snprintf(extra + n, sizeof(extra) - n, " | peer[%d] %08X %08X %08X %08X f10 %02X", i,
                                   *reinterpret_cast<const uint32_t*>(e + 0x0), *reinterpret_cast<const uint32_t*>(e + 0x4),
                                   *reinterpret_cast<const uint32_t*>(e + 0x8), *reinterpret_cast<const uint32_t*>(e + 0xC),
                                   e[0x10]);
            }
            for (int j = 0; j < players && j < 3 && n < sizeof(extra) - 220; ++j) {
                const uint8_t* e = payload + 0x1998 + static_cast<size_t>(j) * 0xB8;
                n += std::snprintf(extra + n, sizeof(extra) - n,
                                   " | plr[%d] idx %d f4 %08X f8 %02X peer %d slot %d f18 %02X f1C %08X id %016llX", j,
                                   *reinterpret_cast<const int32_t*>(e + 0x0), *reinterpret_cast<const uint32_t*>(e + 0x4),
                                   e[0x8], *reinterpret_cast<const int16_t*>(e + 0x12), e[0x14], e[0x18],
                                   *reinterpret_cast<const uint32_t*>(e + 0x1C),
                                   static_cast<unsigned long long>(*reinterpret_cast<const uint64_t*>(e + 0x9)));
            }
        }
        NOBLE_INFO("SlotPatch: membership producer #{} caller hr+0x{:X} payload {:p}: {} peers, {} players{}", gProducerCalls,
                   gBase ? static_cast<uint32_t>(reinterpret_cast<uintptr_t>(_ReturnAddress()) - gBase) : 0,
                   static_cast<void*>(payload), *reinterpret_cast<int16_t*>(payload + 0x14),
                   *reinterpret_cast<int16_t*>(payload + 0x16), extra);
    }
    return r;
}

void* AllocateNearEngine(uintptr_t target, size_t size) {
    SYSTEM_INFO si{};
    GetSystemInfo(&si);
    const uintptr_t step = si.dwAllocationGranularity;
    for (uintptr_t delta = step; delta < 0x7FFF0000ull; delta += step) {
        const uintptr_t candidates[2] = { target + delta, target > delta ? target - delta : 0 };
        for (const uintptr_t candidate : candidates) {
            if (!candidate) continue;
            if (void* p = VirtualAlloc(reinterpret_cast<void*>(candidate & ~(step - 1)), size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE))
                return p;
        }
    }
    return nullptr;
}

void Write(uintptr_t at, const void* bytes, size_t n) {
    DWORD old = 0;
    VirtualProtect(reinterpret_cast<void*>(at), n, PAGE_EXECUTE_READWRITE, &old);
    std::memcpy(reinterpret_cast<void*>(at), bytes, n);
    VirtualProtect(reinterpret_cast<void*>(at), n, old, &old);
}

void Put32(uint8_t* p, int64_t v) { const auto x = static_cast<int32_t>(v); std::memcpy(p, &x, 4); }

bool CallTargets(uintptr_t base, uint32_t site, uint32_t target) {
    const auto* p = reinterpret_cast<const uint8_t*>(base + site);
    if (p[0] != 0xE8) return false;
    int32_t rel = 0;
    std::memcpy(&rel, p + 1, 4);
    return base + site + 5 + static_cast<intptr_t>(rel) == base + target;
}

constexpr size_t kCaveStride = 64;
constexpr size_t kThunkBytes = 16;

} // namespace

unsigned CaveCount() { return static_cast<unsigned>(std::size(kCaves)); }
unsigned EditCount() { return static_cast<unsigned>(std::size(kEdits)); }

bool Match(uintptr_t base) {
    bool ok = true;
    for (const Cave& c : kCaves)
        if (std::memcmp(reinterpret_cast<void*>(base + c.rva), c.orig, c.origLen) != 0) {
            NOBLE_WARN("SlotPatch: membership message site +0x{:X} does not match", c.rva);
            ok = false;
        }
    for (const Edit& e : kEdits) {
        const void* at = reinterpret_cast<void*>(base + e.rva);
        if (std::memcmp(at, e.orig, e.len) != 0 && std::memcmp(at, e.alt, e.len) != 0) {
            NOBLE_WARN("SlotPatch: membership message bytes at +0x{:X} do not match", e.rva);
            ok = false;
        }
    }
    for (const uint32_t site : kProducerCalls)
        if (!CallTargets(base, site, kProducerRva)) {
            NOBLE_WARN("SlotPatch: membership message producer call at +0x{:X} does not match", site);
            ok = false;
        }
    return ok;
}

bool Apply(uintptr_t base, int64_t shadowDelta) {
    const size_t bytes = std::size(kCaves) * kCaveStride + std::size(kProducerCalls) * kThunkBytes;
    auto* mem = static_cast<uint8_t*>(AllocateNearEngine(base, bytes));
    if (!mem) {
        NOBLE_ERROR("SlotPatch: no memory near the engine for the membership message caves");
        return false;
    }
    const int64_t peerShadow = shadowDelta - static_cast<int64_t>(kPeerFirstShadow) * kPeerRecord;
    const int64_t playerShadow = shadowDelta - static_cast<int64_t>(kPlayerFirstShadow) * kPlayerRecord;
    for (size_t i = 0; i < std::size(kCaves); ++i) {
        const Cave& c = kCaves[i];
        uint8_t* cave = mem + i * kCaveStride;
        std::memcpy(cave, c.cave, c.caveLen);
        for (uint8_t f = 0; f < c.fixCount; ++f) {
            const Fix& x = c.fix[f];
            if (x.kind == 0)
                Put32(cave + x.at, static_cast<int64_t>(base + x.target) - reinterpret_cast<intptr_t>(cave + x.at + 4));
            else
                Put32(cave + x.at, x.kind == 1 ? peerShadow : playerShadow);
        }
    }
    // Thunks for the producer call sites: mov rax, imm64 / jmp rax.
    uint8_t* thunks = mem + std::size(kCaves) * kCaveStride;
    gBase = base;
    gProducer = reinterpret_cast<ProducerFn>(base + kProducerRva);
    gPeerCtor = reinterpret_cast<CtorFn>(base + kPeerEntryCtorRva);
    gPlayerCtor = reinterpret_cast<CtorFn>(base + kPlayerEntryCtorRva);
    for (size_t i = 0; i < std::size(kProducerCalls); ++i) {
        uint8_t* t = thunks + i * kThunkBytes;
        t[0] = 0x48; t[1] = 0xB8;
        const auto target = reinterpret_cast<uint64_t>(&ProducerWithExtension);
        std::memcpy(t + 2, &target, 8);
        t[10] = 0xFF; t[11] = 0xE0;
    }
    DWORD old = 0;
    VirtualProtect(mem, bytes, PAGE_EXECUTE_READ, &old);
    FlushInstructionCache(GetCurrentProcess(), mem, bytes);

    for (size_t i = 0; i < std::size(kCaves); ++i) {
        const Cave& c = kCaves[i];
        uint8_t jump[10] = { 0xE9, 0, 0, 0, 0, 0x90, 0x90, 0x90, 0x90, 0x90 };
        Put32(jump + 1, reinterpret_cast<intptr_t>(mem + i * kCaveStride) - static_cast<intptr_t>(base + c.rva + 5));
        Write(base + c.rva, jump, c.origLen);
    }
    for (const Edit& e : kEdits) Write(base + e.rva, e.patched, e.len);
    for (size_t i = 0; i < std::size(kProducerCalls); ++i) {
        uint8_t call[5] = { 0xE8, 0, 0, 0, 0 };
        Put32(call + 1, reinterpret_cast<intptr_t>(thunks + i * kThunkBytes) - static_cast<intptr_t>(base + kProducerCalls[i] + 5));
        Write(base + kProducerCalls[i], call, sizeof(call));
    }
    FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void*>(base), 0x1000000);
    NOBLE_WARN("SlotPatch: membership message (#32) for 32 players: {} caves, {} byte edits, producer extension stubs; "
               "wire counts/peer indices 6 bits, player index 5 bits, payload 0x{:X} (extension at +0x{:X})",
               std::size(kCaves), std::size(kEdits), kMsgSize, kMsgExtOffset);
    return true;
}

bool ApplyHighSlotTest(uintptr_t base) {
    for (const Cave& c : kTestCaves)
        if (std::memcmp(reinterpret_cast<void*>(base + c.rva), c.orig, c.origLen) != 0) {
            NOBLE_WARN("SlotPatch: high-slot test site +0x{:X} does not match; not installed", c.rva);
            return false;
        }
    const size_t bytes = std::size(kTestCaves) * kCaveStride;
    auto* mem = static_cast<uint8_t*>(AllocateNearEngine(base, bytes));
    if (!mem) return false;
    for (size_t i = 0; i < std::size(kTestCaves); ++i) {
        const Cave& c = kTestCaves[i];
        uint8_t* cave = mem + i * kCaveStride;
        std::memcpy(cave, c.cave, c.caveLen);
        for (uint8_t f = 0; f < c.fixCount; ++f)
            Put32(cave + c.fix[f].at, static_cast<int64_t>(base + c.fix[f].target) - reinterpret_cast<intptr_t>(cave + c.fix[f].at + 4));
    }
    DWORD old = 0;
    VirtualProtect(mem, bytes, PAGE_EXECUTE_READ, &old);
    FlushInstructionCache(GetCurrentProcess(), mem, bytes);
    for (size_t i = 0; i < std::size(kTestCaves); ++i) {
        uint8_t jump[10] = { 0xE9, 0, 0, 0, 0, 0x90, 0x90, 0x90, 0x90, 0x90 };
        Put32(jump + 1, reinterpret_cast<intptr_t>(mem + i * kCaveStride) - static_cast<intptr_t>(base + kTestCaves[i].rva + 5));
        Write(base + kTestCaves[i].rva, jump, kTestCaves[i].origLen);
    }
    FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void*>(base), 0x1000000);
    NOBLE_WARN("SlotPatch: NOBLE_SLOT_HIGH=1 (dev test) - joining peers take slots >= 17 and players >= 16");
    return true;
}

} // namespace noble::slots::message
