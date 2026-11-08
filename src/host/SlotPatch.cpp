#include "SlotPatch.hpp"

#include "common/Logger.hpp"

#include <cstdint>
#include <cstring>
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

const Site kSites[] = {
#include "SlotPatchTable.inc"
};

constexpr uintptr_t kSessionsRva = 0x2B07418;   // 4 static session membership objects
constexpr uintptr_t kSessionStride = 0x61668;
constexpr int kSessions = 4;

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

void Put32(uint8_t* p, int64_t v) { const auto x = static_cast<int32_t>(v); std::memcpy(p, &x, 4); }

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
    const uintptr_t sessions = base + kSessionsRva;
    // Shadow objects: same stride as the real ones, so shadow(obj) = obj + delta for all four.
    auto* shadow = static_cast<uint8_t*>(AllocateNear(sessions, kSessions * kSessionStride, PAGE_READWRITE));
    size_t caveBytes = 0;
    for (const Site& s : kSites) caveBytes += (s.caveLength + 15) & ~15u;
    auto* caves = static_cast<uint8_t*>(AllocateNear(base, caveBytes, PAGE_READWRITE));
    if (!shadow || !caves) {
        NOBLE_ERROR("SlotPatch: could not allocate memory near the engine; nothing patched");
        if (shadow) VirtualFree(shadow, 0, MEM_RELEASE);
        if (caves) VirtualFree(caves, 0, MEM_RELEASE);
        return;
    }
    const int64_t delta = reinterpret_cast<intptr_t>(shadow) - static_cast<intptr_t>(sessions);
    uint8_t* cave = caves;
    for (const Site& s : kSites) {
        std::memcpy(cave, s.cave, s.caveLength);
        Put32(cave + s.constantAt, delta - static_cast<int64_t>(s.threshold) * s.record);
        const uintptr_t back = base + s.rva + sizeof(s.original);
        Put32(cave + s.backAt1, static_cast<int64_t>(back) - reinterpret_cast<intptr_t>(cave + s.backAt1 + 4));
        Put32(cave + s.backAt2, static_cast<int64_t>(back) - reinterpret_cast<intptr_t>(cave + s.backAt2 + 4));
        cave += (s.caveLength + 15) & ~15u;
    }
    DWORD old = 0;
    VirtualProtect(caves, caveBytes, PAGE_EXECUTE_READ, &old);
    FlushInstructionCache(GetCurrentProcess(), caves, caveBytes);
    cave = caves;
    for (const Site& s : kSites) {
        auto* at = reinterpret_cast<uint8_t*>(base + s.rva);
        uint8_t jump[7] = { 0xE9, 0, 0, 0, 0, 0x90, 0x90 };
        Put32(jump + 1, reinterpret_cast<intptr_t>(cave) - reinterpret_cast<intptr_t>(at + 5));
        VirtualProtect(at, sizeof(jump), PAGE_EXECUTE_READWRITE, &old);
        std::memcpy(at, jump, sizeof(jump));
        VirtualProtect(at, sizeof(jump), old, &old);
        cave += (s.caveLength + 15) & ~15u;
    }
    FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void*>(base), 0x1000000);
    NOBLE_INFO("SlotPatch: {} slot address sites redirected (players >= 16, peers >= 17 -> shadow sessions at {:p}, "
               "delta 0x{:X}); inert until the players pool is raised", std::size(kSites), static_cast<void*>(shadow),
               static_cast<uint64_t>(delta));
}

} // namespace noble::slots
