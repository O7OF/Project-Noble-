#include "TrooperMode.hpp"

#include "common/Logger.hpp"
#include "hooks/HookManager.hpp"

#include <shellapi.h>

#include <algorithm>
#include <atomic>
#include <cstring>
#include <string>

namespace noble {
namespace TrooperMode {
namespace {

// The cache step ElitePack uses only runs for extra caches, so Trooper mode works after the zone-set rebuild (map load,
// before the player spawns) on the live tag table. ElitePack only installs in multiplayer / Firefight / Forge / menu,
// Trooper mode only in campaign, so the two never hook the rebuild in the same process.
constexpr uintptr_t kZoneRebuild = 0x08EAF0;   // zone-set rebuild (ElitePack hooks it too, in its own modes)
constexpr uintptr_t kTagTable = 0xC1A600;      // pointer to the live combined tag table (same layout as the cache step's)
constexpr uintptr_t kPointerBases = 0x4E39F20;
constexpr uint32_t kBipd = 0x62697064, kMatg = 0x6D617467;

// Tag index of objects\characters\marine\marine and objects\characters\spartans\spartans in each stock MCC map
// (Reclaimer, tools rc_trooper.fsx, 2026-11-14). m70_a has no marine.
struct MapEntry { const char* map; int32_t marine; int32_t spartan; };
constexpr MapEntry kMaps[] = {
    {"m05", 5653, 4600},   {"m10", 5055, 12263},  {"m20", 3094, 13197},  {"m30", 9271, 11623},
    {"m35", 2320, 14181},  {"m45", 8768, 12265},  {"m50", 3430, 12739},  {"m52", 1290, 10979},
    {"m60", 3719, 14626},  {"m70", 10706, 13277}, {"m70_bonus", 1377, 1759},
};

uintptr_t s_base = 0;
const MapEntry* s_entry = nullptr;
std::atomic<bool> s_done{false};
std::atomic<bool> s_active{false};

using RebuildFn = void(__fastcall*)(uint8_t*);
RebuildFn s_original = nullptr;

uint8_t* TagAt(uint32_t packed) {
    const uint64_t* bases = reinterpret_cast<const uint64_t*>(s_base + kPointerBases);
    return reinterpret_cast<uint8_t*>(bases[packed >> 28] + (static_cast<uint64_t>(packed) << 2));
}

struct Table {
    uint8_t* t;
    int32_t Count() const { int32_t n = 0; std::memcpy(&n, t + 0x3FFF8, 4); return std::clamp(n, 0, 0x8000); }
    uint32_t Magic(int32_t i) const {
        int16_t g = 0; uint16_t salt = 0;
        std::memcpy(&g, t + i * 8, 2); std::memcpy(&salt, t + i * 8 + 2, 2);
        if (g < 0 || salt == 0xFFFF) return 0;
        uint32_t m = 0; std::memcpy(&m, t + 0x3FFFC + g * 16, 4); return m;
    }
    uint32_t Address(int32_t i) const { uint32_t a = 0; std::memcpy(&a, t + i * 8 + 4, 4); return a; }
    uint32_t Datum(int32_t i) const { uint16_t s = 0; std::memcpy(&s, t + i * 8 + 2, 2); return (uint32_t(s) << 16) | uint32_t(i); }
    int32_t First(uint32_t magic) const { for (int32_t i = 0; i < Count(); ++i) if (Magic(i) == magic) return i; return -1; }
};

// Returns the number of representations switched, or a negative reason.
uint32_t s_repBipeds[16] = {};
uint32_t s_repHead[16][4] = {};
int SwitchSEH(uint8_t* tags, uint32_t* marineOut, uint32_t* spartanOut, int32_t* repsOut, uint32_t* firstOut) {
    __try {
        Table tt{tags};
        if (s_entry->marine >= tt.Count() || tt.Magic(s_entry->marine) != kBipd) return -1;    // not loaded yet / wrong map
        if (s_entry->spartan >= tt.Count() || tt.Magic(s_entry->spartan) != kBipd) return -2;
        const int32_t g = tt.First(kMatg);
        if (g < 0) return -3;
        uint8_t* matg = TagAt(tt.Address(g));
        int32_t reps = 0; uint32_t repsPtr = 0;
        std::memcpy(&reps, matg + 0x474, 4);
        std::memcpy(&repsPtr, matg + 0x478, 4);
        *repsOut = reps;
        if (reps < 1 || !repsPtr) return -4;
        uint8_t* rep = TagAt(repsPtr);
        reps = std::min(reps, 16);
        const uint32_t marine = tt.Datum(s_entry->marine), spartan = tt.Datum(s_entry->spartan);
        *marineOut = marine; *spartanOut = spartan;
        std::memcpy(firstOut, rep + 0x40, 4);
        int n = 0;
        for (int32_t r = 0; r < reps; ++r) {
            uint32_t d = 0;
            std::memcpy(&d, rep + r * 0x6C + 0x40, 4);
            s_repBipeds[r] = d;
            std::memcpy(s_repHead[r], rep + r * 0x6C, 16);
            if (d != spartan) continue;
            std::memcpy(rep + r * 0x6C + 0x40, &marine, 4);
            ++n;
        }
        return n;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return -100;
    }
}

uint8_t* LiveTableSEH() {
    __try { return *reinterpret_cast<uint8_t**>(s_base + kTagTable); } __except (EXCEPTION_EXECUTE_HANDLER) { return nullptr; }
}

void __fastcall RebuildHook(uint8_t* arg) {
    s_original(arg);
    uint8_t* tags = LiveTableSEH();
    if (tags && s_entry && !s_done.load()) {
        uint32_t marine = 0, spartan = 0, first = 0;
        int32_t reps = 0;
        const int n = SwitchSEH(static_cast<uint8_t*>(tags), &marine, &spartan, &reps, &first);
        for (int r = 0; r < reps && r < 16; ++r)
            NOBLE_INFO("TrooperMode: representation {}: biped 0x{:08X}, first dwords {:08X} {:08X} {:08X} {:08X}", r, s_repBipeds[r],
                       s_repHead[r][0], s_repHead[r][1], s_repHead[r][2], s_repHead[r][3]);
        if (n > 0) {
            s_done = true;
            s_active = true;
            NOBLE_INFO("TrooperMode: {} player representation(s) of {} switched from the Spartan 0x{:08X} to the marine 0x{:08X}",
                       n, reps, spartan, marine);
        } else if (n == 0) {
            s_done = true;
            NOBLE_WARN("TrooperMode: no player representation uses the Spartan biped 0x{:08X} ({} representations, first 0x{:08X})",
                       spartan, reps, first);
        } else {
            NOBLE_INFO("TrooperMode: zone-set rebuild, not ready ({})", n);
        }
    }
}

} // namespace

bool Install(HMODULE engine, const std::string& map, const std::string& mode) {
    if (!engine || s_base) return false;
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    bool on = false;
    for (int i = 1; argv && i < argc; ++i) if (_wcsicmp(argv[i], L"-trooper") == 0) on = true;
    if (argv) LocalFree(argv);
    if (!on) return false;
    if (mode != "campaign") { NOBLE_INFO("TrooperMode: -trooper is campaign only for now (mode {})", mode); return false; }
    for (const auto& e : kMaps) if (_stricmp(e.map, map.c_str()) == 0) s_entry = &e;
    if (!s_entry) { NOBLE_WARN("TrooperMode: no marine known for map '{}'", map); return false; }
    s_base = reinterpret_cast<uintptr_t>(engine);
    static const uint8_t kPrologue[7] = {0x48, 0x8B, 0xC4, 0x48, 0x89, 0x58, 0x10};
    if (std::memcmp(reinterpret_cast<void*>(s_base + kZoneRebuild), kPrologue, 7) != 0) {
        NOBLE_WARN("TrooperMode: haloreach.dll doesn't look as expected; off");
        return false;
    }
    if (!HookManager::InstallHook(reinterpret_cast<void*>(s_base + kZoneRebuild), reinterpret_cast<void*>(&RebuildHook),
                                  reinterpret_cast<void**>(&s_original), "trooper_zone_rebuild")) {
        NOBLE_WARN("TrooperMode: hook failed; off");
        return false;
    }
    HookManager::EnableAll();
    NOBLE_INFO("TrooperMode: on for {} (marine tag {}, Spartan tag {})", map, s_entry->marine, s_entry->spartan);
    return true;
}

bool Active() { return s_active.load(); }

} // namespace TrooperMode
} // namespace noble
