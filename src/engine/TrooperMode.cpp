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

// ---- test-only model/permutation dump (NOBLE_MODEL_DUMP=1) -----------------------------------------------------

namespace {

uintptr_t EngineBaseForDump() {
    if (s_base) return s_base;
    return reinterpret_cast<uintptr_t>(GetModuleHandleW(L"haloreach.dll"));
}

const uint8_t* PagesAt(uintptr_t base, uint32_t packed) {
    const uint64_t* pages = reinterpret_cast<const uint64_t*>(base + 0x4E39F20);
    const uint64_t page = pages[packed >> 28];
    if (!page) return nullptr;
    return reinterpret_cast<const uint8_t*>(page + static_cast<uint64_t>(packed) * 4);
}

const uint8_t* TagDataAt(uintptr_t base, const uint8_t* tagInstances, uint32_t datum) {
    if (!tagInstances || datum == 0xFFFFFFFFu) return nullptr;
    const uint32_t packed = *reinterpret_cast<const uint32_t*>(tagInstances + static_cast<size_t>(datum & 0xFFFFu) * 8 + 4);
    return PagesAt(base, packed);
}

uint32_t LocalUnitDatumForDump(uintptr_t base) {
    using GetLocalUnitFn = int32_t(__fastcall*)(int32_t);
    return static_cast<uint32_t>(reinterpret_cast<GetLocalUnitFn>(base + 0x53EF8)(0));
}

const uint8_t* ObjectEntryForDump(uintptr_t base, uint32_t unit) {
    const uint32_t tlsIdx = *reinterpret_cast<const uint32_t*>(base + 0xC17B18);
    const uintptr_t tlsArray = *reinterpret_cast<const uintptr_t*>(__readgsqword(0x58) + static_cast<uintptr_t>(tlsIdx) * 8u);
    if (!tlsArray) return nullptr;
    const uint8_t* objTable = *reinterpret_cast<const uint8_t* const*>(tlsArray + 0x10);
    if (!objTable) return nullptr;
    const uint8_t* entries = *reinterpret_cast<const uint8_t* const*>(objTable + 0x50);
    if (!entries) return nullptr;
    return *reinterpret_cast<const uint8_t* const*>(entries + static_cast<size_t>(unit & 0xFFFFu) * 0x18 + 0x10);
}

void DumpRegionsSEH(uintptr_t base, uint32_t unit) {
    __try {
        const uint8_t* entry = ObjectEntryForDump(base, unit);
        if (!entry) { NOBLE_WARN("ModelDump: no object entry for unit 0x{:08X}", unit); return; }
        const uint32_t bipedDatum = *reinterpret_cast<const uint32_t*>(entry);
        const uint16_t variant = *reinterpret_cast<const uint16_t*>(entry + 0xEA);   // object_set_variant's field
        const uint8_t* tagInstances = *reinterpret_cast<const uint8_t* const*>(base + 0xC1A600);
        const uint8_t* biped = TagDataAt(base, tagInstances, bipedDatum);
        if (!biped) { NOBLE_WARN("ModelDump: no biped tag for unit 0x{:08X}", unit); return; }
        const uint32_t hlmtDatum = *reinterpret_cast<const uint32_t*>(biped + 0x70);
        const uint8_t* hlmt = TagDataAt(base, tagInstances, hlmtDatum);
        if (!hlmt) { NOBLE_WARN("ModelDump: no hlmt for biped 0x{:08X}", bipedDatum); return; }
        const uint32_t modeDatum = *reinterpret_cast<const uint32_t*>(hlmt + 0x0C);
        const uint8_t* mode = TagDataAt(base, tagInstances, modeDatum);
        if (!mode) { NOBLE_WARN("ModelDump: no model for hlmt 0x{:08X}", hlmtDatum); return; }
        const int32_t regionCount = *reinterpret_cast<const int32_t*>(mode + 0x0C);
        const uint32_t regionPacked = *reinterpret_cast<const uint32_t*>(mode + 0x10);
        const uint8_t* regions = PagesAt(base, regionPacked);

        // haloreach+0x473028: per-object region state (count, byte per region, region table).
        using RegionStateFn = void(__fastcall*)(uint16_t, int32_t*, int64_t*, int64_t*);
        int32_t stateCount = 0;
        int64_t statePtr = 0, tablePtr = 0;
        reinterpret_cast<RegionStateFn>(base + 0x473028)(static_cast<uint16_t>(unit), &stateCount, &statePtr, &tablePtr);
        const uint8_t* state = reinterpret_cast<const uint8_t*>(statePtr);

        NOBLE_INFO("ModelDump: unit 0x{:08X} biped 0x{:08X} hlmt 0x{:08X} model 0x{:08X} variant {} regions {} state {} table 0x{:X}",
                   unit, bipedDatum, hlmtDatum, modeDatum, variant, regionCount, stateCount, static_cast<uint64_t>(tablePtr));

        for (int32_t i = 0; i < regionCount && i < 64 && regions; ++i) {
            const uint8_t* region = regions + static_cast<size_t>(i) * 0x10;
            const uint32_t nameSid = *reinterpret_cast<const uint32_t*>(region);
            const int32_t permCount = *reinterpret_cast<const int32_t*>(region + 4);
            const uint32_t permPacked = *reinterpret_cast<const uint32_t*>(region + 8);
            const uint8_t* perms = PagesAt(base, permPacked);
            const int32_t current = (state && i < stateCount) ? state[i] : -1;
            uint32_t permSid = 0;
            if (perms && current >= 0 && current < permCount)
                permSid = *reinterpret_cast<const uint32_t*>(perms + static_cast<size_t>(current) * 0x18);
            NOBLE_INFO("ModelDump: region {:02} name 0x{:08X} perms {:2} current {:3} perm 0x{:08X}", i, nameSid,
                       permCount, current, permSid);
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        NOBLE_WARN("ModelDump: fault while reading the model");
    }
}

int EnvInt(const wchar_t* name, int fallback) {
    wchar_t value[32] = {};
    if (GetEnvironmentVariableW(name, value, 32) <= 0) return fallback;
    return _wtoi(value);
}

// Test-only: force region permutation(s) by index from the environment. NOBLE_TROOPER_FORCE_REGION=-1 forces every
// region; a non-negative value forces just that region (region order = the applied model variant's region list).
void ForceRegionPermSEH(uintptr_t base, uint32_t unit, int32_t regionIndex, int32_t permIndex) {
    __try {
        const uint8_t* entry = ObjectEntryForDump(base, unit);
        if (!entry) return;
        const uint32_t bipedDatum = *reinterpret_cast<const uint32_t*>(entry);
        const uint8_t* tagInstances = *reinterpret_cast<const uint8_t* const*>(base + 0xC1A600);
        const uint8_t* biped = TagDataAt(base, tagInstances, bipedDatum);
        if (!biped) return;
        const uint8_t* hlmt = TagDataAt(base, tagInstances, *reinterpret_cast<const uint32_t*>(biped + 0x70));
        if (!hlmt) return;
        const uint8_t* mode = TagDataAt(base, tagInstances, *reinterpret_cast<const uint32_t*>(hlmt + 0x0C));
        if (!mode) return;
        const int32_t regionCount = *reinterpret_cast<const int32_t*>(mode + 0x0C);
        const uint8_t* regions = PagesAt(base, *reinterpret_cast<const uint32_t*>(mode + 0x10));
        if (!regions) return;
        using SetRegionPermutationFn = uint8_t(__fastcall*)(uint32_t, uint32_t, uint32_t, uint8_t);
        auto setRegionPermutation = reinterpret_cast<SetRegionPermutationFn>(base + 0x4729F4);

        const int32_t first = (regionIndex >= 0) ? regionIndex : 0;
        const int32_t last = (regionIndex >= 0) ? regionIndex : (regionCount - 1);
        for (int32_t i = first; i <= last && i < regionCount; ++i) {
            const uint8_t* region = regions + static_cast<size_t>(i) * 0x10;
            const uint32_t regionSid = *reinterpret_cast<const uint32_t*>(region);
            const int32_t permCount = *reinterpret_cast<const int32_t*>(region + 4);
            const uint8_t* perms = PagesAt(base, *reinterpret_cast<const uint32_t*>(region + 8));
            if (!perms || permCount <= 0) continue;
            const int32_t use = (permIndex < permCount) ? permIndex : 0;
            const uint32_t permSid = *reinterpret_cast<const uint32_t*>(perms + static_cast<size_t>(use) * 0x18);
            const uint8_t rc = setRegionPermutation(unit, regionSid, permSid, 1);
            NOBLE_INFO("ModelForce: region {} sid 0x{:08X} -> perm {} sid 0x{:08X} rc {}", i, regionSid, use, permSid, rc);
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        NOBLE_WARN("ModelForce: fault");
    }
}

} // namespace

void DumpModelState() {
    static int mode = -1;   // bit 1 = dump, bit 2 = force
    static int forceRegion = -1, forcePerm = -1;
    if (mode < 0) {
        mode = 0;
        wchar_t value[8] = {};
        if (GetEnvironmentVariableW(L"NOBLE_MODEL_DUMP", value, 8) > 0) mode |= 1;
        forceRegion = EnvInt(L"NOBLE_TROOPER_FORCE_REGION", -1);
        forcePerm = EnvInt(L"NOBLE_TROOPER_FORCE_PERM", -1);
        if (forcePerm >= 0) mode |= 2;
    }
    if (!mode) return;
    static bool done = false;
    static int frames = 0;
    if (done) return;
    const uintptr_t base = EngineBaseForDump();
    if (!base) return;
    uint32_t unit;
    __try { unit = LocalUnitDatumForDump(base); } __except (EXCEPTION_EXECUTE_HANDLER) { return; }
    if (unit == 0xFFFFFFFFu) return;
    if (++frames < 120) return;   // about two seconds at 60 fps
    done = true;
    if (mode & 1) DumpRegionsSEH(base, unit);
    if (mode & 2) ForceRegionPermSEH(base, unit, forceRegion, forcePerm);
}

} // namespace TrooperMode
} // namespace noble
