#include "SlotPatch.hpp"

#include "common/Logger.hpp"

#include <cstdint>
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
};

const Step kSteps[] = {
#include "SlotStepTable.inc"
};

const Bound kBounds[] = {
#include "SlotBoundsTable.inc"
};

constexpr uintptr_t kSessionsRva = 0x2B07418;   // 4 static session membership objects
constexpr uintptr_t kSessionStride = 0x61668;
constexpr int kSessions = 4;

// Record constructors the session constructor (0x3CF864) runs on every inline slot; run them on the shadow slots
// too so overflow records start in the same state (ids -1, cleared names) instead of all zero.
constexpr uintptr_t kPeerCtorRva = 0x16FC60;     // this = peer record (0x158)
constexpr uintptr_t kPlayerCtorRva = 0x3CFA38;   // this = player record (0xB0)
struct Array { uint32_t offset; uint32_t record; uint32_t firstShadowSlot; bool peer; };
constexpr Array kArrays[] = {
    { 0x0060, 0x158, 17, true },  { 0x1740, 0xB0, 16, false },   // block A
    { 0x2270, 0x158, 17, true },  { 0x3950, 0xB0, 16, false },   // block B
};
constexpr uint32_t kSlots32 = 32;   // players and peers in 32-player mode (the peer-valid mask is a DWORD)

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
bool Players32Requested() {
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
            for (uint32_t j = 0; j < count; ++j) (a.peer ? peerCtor : playerCtor)(object + a.offset + j * a.record);
        }
    }
}

} // namespace

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
    bool players32 = Players32Requested();
    if (players32) {
        for (const Step& s : kSteps) {
            if (std::memcmp(reinterpret_cast<void*>(base + s.rva), s.original, sizeof(s.original)) != 0) {
                NOBLE_WARN("SlotPatch: step loop at +0x{:X} does not match; 32-player mode off", s.rva);
                players32 = false;
                break;
            }
        }
        for (const Bound& b : kBounds) {
            if (!players32) break;
            if (std::memcmp(reinterpret_cast<void*>(base + b.rva), b.original, b.length) != 0) {
                NOBLE_WARN("SlotPatch: loop bound at +0x{:X} does not match; 32-player mode off", b.rva);
                players32 = false;
            }
        }
    }
    const uintptr_t sessions = base + kSessionsRva;
    // Shadow objects: same stride as the real ones, so shadow(obj) = obj + delta for all four.
    auto* shadow = static_cast<uint8_t*>(AllocateNear(sessions, kSessions * kSessionStride, PAGE_READWRITE));
    size_t caveBytes = 0;
    for (const Site& s : kSites) caveBytes += Rounded(s.caveLength);
    if (players32) for (const Step& s : kSteps) caveBytes += Rounded(s.caveLength);
    auto* caves = static_cast<uint8_t*>(AllocateNear(base, caveBytes, PAGE_READWRITE));
    if (!shadow || !caves) {
        NOBLE_ERROR("SlotPatch: could not allocate memory near the engine; nothing patched");
        if (shadow) VirtualFree(shadow, 0, MEM_RELEASE);
        if (caves) VirtualFree(caves, 0, MEM_RELEASE);
        return;
    }
    InitShadowRecords(base, shadow, players32);
    const int64_t delta = reinterpret_cast<intptr_t>(shadow) - static_cast<intptr_t>(sessions);

    // Fill the caves (rel32 back jumps and the shadow constant), then make them executable.
    uint8_t* cave = caves;
    for (const Site& s : kSites) {
        std::memcpy(cave, s.cave, s.caveLength);
        Put32(cave + s.constantAt, delta - static_cast<int64_t>(s.threshold) * s.record);
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
    }
    FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void*>(base), 0x1000000);
    NOBLE_INFO("SlotPatch: {} slot address sites redirected (players >= 16, peers >= 17 -> shadow sessions at {:p}, "
               "delta 0x{:X})", std::size(kSites), static_cast<void*>(shadow), static_cast<uint64_t>(delta));
    if (players32)
        NOBLE_WARN("SlotPatch: 32-player mode (dev): {} step loops, {} loop bounds raised to 32; tables listed in "
                   "docs/limits-players-loops.md are still 16/17 and the players pool is not raised here",
                   std::size(kSteps), std::size(kBounds));
    else
        NOBLE_INFO("SlotPatch: 16-player mode; inert until -slot-players 32");
}

} // namespace noble::slots
