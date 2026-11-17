#include "ElitePack.hpp"

#include <shellapi.h>

#include "common/Logger.hpp"
#include "common/SaveCodec.hpp"
#include "common/ArmoryVisor.hpp"
#include "common/ArmorEffects.hpp"
#include "ParticleTint.hpp"
#include "hooks/HookManager.hpp"
#include "engine/SharedTagCache.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <atomic>
#include <climits>
#include <intrin.h>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cwctype>
#include <fstream>
#include <iterator>
#include <mutex>
#include <vector>

// Engine addresses (RVAs in the supported haloreach.dll, SHA-256 738dd2d2...). Found by reading how
// the reference host loads its Elite pack (the notes).
//   0x0A38A3  call to the match map loader 0x0141C0 (request: name, extra cache count, extra cache names)
//   0x031395  call to the extra cache loader 0x031078 inside the cache set loader (first extra list)
//   0x0AA634  map header slot (index 0xFFFF, name, mode 2), the main map gets it at 0x0A37DC
//   0x030698  per-cache load step (rebases the cache's own 32-bit fixups, then registers its tags)
//   0x030859  "mov edx,edx" there drops the top half of a 64-bit load delta; patched to a 2-byte nop
//   0x688F3C  packs a 64-bit tag address into a 32-bit packed pointer (table at 0x4E39F20)

namespace noble {
namespace ElitePack {
namespace {

constexpr uintptr_t kLoadCallSite = 0x0A38A3;
constexpr uintptr_t kMapLoad = 0x0141C0;
constexpr uintptr_t kExtraCallSite = 0x031395;
constexpr uintptr_t kExtraLoad = 0x031078;
constexpr uintptr_t kHeaderSlot = 0x0AA634;
constexpr uintptr_t kCacheStep = 0x030698;
constexpr uintptr_t kDeltaTruncation = 0x030859;
constexpr uintptr_t kEncodePointer = 0x688F3C;

struct LoadRequest {
    const char* mapName;        // "haloreach\maps\20_sword_slayer"
    int32_t extraCount;         // extra caches loaded with the map
    int32_t pad;
    const char* const* extraNames;
};

using MapLoadFn = uint8_t(__fastcall*)(void*, void*, void*, LoadRequest*);
using ExtraLoadFn = uint8_t(__fastcall*)(void*, const char*);
using HeaderSlotFn = uint8_t(__fastcall*)(uint32_t, const char*, uint32_t);
using CacheStepFn = uint8_t(__fastcall*)(void*, void*, uint32_t, void*, void*, void*);
using EncodeFn = uint32_t(__fastcall*)(uint64_t);

struct Reloc {
    std::vector<uint32_t> offsets;  // offsets of packed pointers inside the loaded pack
    uint64_t virtualBase = 0;       // the pack's own base address (cache +0x2F0 must match)
    uint64_t packedBase = 0;        // base the packed pointers were written against
    uint32_t sectionSize = 0;       // loaded size (cache +0x24 must match)
};

uintptr_t s_base = 0;
std::filesystem::path s_packPath;
std::string s_packStem;
std::string s_packEngineName;       // "haloreach\maps\<stem>", built from the match map's name
Reloc s_reloc;
bool s_relocPending = false;
std::atomic<bool> s_loading{false};
std::atomic<bool> s_loaded{false};
std::atomic<uint64_t> s_adaptedCalls{0};
bool s_ready = false;               // every hook is in place

MapLoadFn s_mapLoad = nullptr;
ExtraLoadFn s_extraLoad = nullptr;
CacheStepFn s_cacheStepOriginal = nullptr;
const char* s_extraList[1] = {nullptr};

template <typename T> T At(uintptr_t rva) { return reinterpret_cast<T>(s_base + rva); }

// Noble relocation sidecar: magic "NBRL" or "RRRL", version 1, the pack's packed-pointer offsets and bases.
bool ReadReloc(const std::filesystem::path& file, Reloc& out) {
    std::ifstream in(file, std::ios::binary);
    if (!in) return false;
    std::vector<uint8_t> d((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (d.size() < 0x20 || (std::memcmp(d.data(), "NBRL", 4) != 0 && std::memcmp(d.data(), "RRRL", 4) != 0)) {
        NOBLE_WARN("ElitePack: {} is not a Noble relocation sidecar", file.string());
        return false;
    }
    uint32_t version = 0, count = 0;
    std::memcpy(&version, &d[4], 4);
    std::memcpy(&count, &d[8], 4);
    std::memcpy(&out.virtualBase, &d[12], 8);
    std::memcpy(&out.packedBase, &d[20], 8);
    std::memcpy(&out.sectionSize, &d[28], 4);
    if (version != 1 || d.size() != 0x20 + static_cast<size_t>(count) * 4) {
        NOBLE_WARN("ElitePack: {} has version {} / {} records for {} bytes", file.string(), version, count, d.size());
        return false;
    }
    out.offsets.resize(count);
    std::memcpy(out.offsets.data(), &d[0x20], static_cast<size_t>(count) * 4);
    for (uint32_t off : out.offsets) {
        if (static_cast<uint64_t>(off) + 4 > out.sectionSize) {
            NOBLE_WARN("ElitePack: relocation record outside the pack's section");
            return false;
        }
    }
    return true;
}

// Writes bytes into the engine's code after checking what is there.
bool PatchCode(uintptr_t rva, const uint8_t* expect, const uint8_t* replace, size_t n) {
    uint8_t* p = At<uint8_t*>(rva);
    if (std::memcmp(p, replace, n) == 0) return true;
    if (std::memcmp(p, expect, n) != 0) return false;
    DWORD old = 0;
    if (!VirtualProtect(p, n, PAGE_EXECUTE_READWRITE, &old)) return false;
    std::memcpy(p, replace, n);
    VirtualProtect(p, n, old, &old);
    FlushInstructionCache(GetCurrentProcess(), p, n);
    return true;
}

// A jump stub within +-2 GB of the engine, so a rel32 call there can reach Noble's code.
void* NearStub(void* target) {
    SYSTEM_INFO si{};
    GetSystemInfo(&si);
    const uintptr_t gran = si.dwAllocationGranularity;
    for (uintptr_t delta = 0x10000000; delta < 0x7F000000; delta += gran * 16) {
        for (int sign : {-1, 1}) {
            const uintptr_t want = ((s_base + sign * static_cast<intptr_t>(delta)) / gran) * gran;
            void* mem = VirtualAlloc(reinterpret_cast<void*>(want), 4096, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
            if (!mem) continue;
            uint8_t* s = static_cast<uint8_t*>(mem);
            s[0] = 0xFF; s[1] = 0x25; std::memset(s + 2, 0, 4);       // jmp [rip+0]
            const uint64_t abs = reinterpret_cast<uint64_t>(target);
            std::memcpy(s + 6, &abs, 8);
            FlushInstructionCache(GetCurrentProcess(), s, 14);
            return mem;
        }
    }
    return nullptr;
}

// Points the call at `site` (E8 rel32 to `expect`) at `detour`. Returns the original target.
void* RetargetCall(uintptr_t site, uintptr_t expect, void* detour) {
    uint8_t* p = At<uint8_t*>(site);
    if (p[0] != 0xE8) return nullptr;
    int32_t rel = 0;
    std::memcpy(&rel, p + 1, 4);
    const uintptr_t target = s_base + site + 5 + rel;
    if (target != s_base + expect) return nullptr;
    void* stub = NearStub(detour);
    if (!stub) return nullptr;
    const int64_t newRel = reinterpret_cast<intptr_t>(stub) - static_cast<intptr_t>(s_base + site + 5);
    if (newRel < INT32_MIN || newRel > INT32_MAX) return nullptr;
    const int32_t r32 = static_cast<int32_t>(newRel);
    DWORD old = 0;
    if (!VirtualProtect(p + 1, 4, PAGE_EXECUTE_READWRITE, &old)) return nullptr;
    std::memcpy(p + 1, &r32, 4);
    VirtualProtect(p + 1, 4, old, &old);
    FlushInstructionCache(GetCurrentProcess(), p, 5);
    return reinterpret_cast<void*>(target);
}

uint8_t __fastcall MapLoadHook(void* a1, void* a2, void* a3, LoadRequest* req) {
    if (!s_ready || !req || !req->mapName || req->extraCount != 0) {
        if (req) NOBLE_INFO("ElitePack: not added (map {}, extra caches already {})",
                            req->mapName ? req->mapName : "?", req->extraCount);
        if (req) {
            const uint8_t result = s_mapLoad(a1, a2, a3, req);
            char line[224];
            std::snprintf(line, sizeof(line), "map load (passthrough): id 0x%X type 0x%X '%s' -> %u",
                          static_cast<unsigned>(reinterpret_cast<uintptr_t>(a2)),
                          static_cast<unsigned>(reinterpret_cast<uintptr_t>(a3)), req->mapName, result);
            NOBLE_INFO("{}", line);
            return result;
        }
        return s_mapLoad(a1, a2, a3, req);
    }
    // The loader (haloreach+0x141C0) rejects a map whose own header id/type disagrees with these two arguments
    // (it compares them against the loaded header at +0xC and +0x10, and accepts -1 as "any"). Log them, plus the
    // verdict, so a mod map that fails to load shows which side disagrees.
    {
        char line[192];
        std::snprintf(line, sizeof(line), "ElitePack: map load request (mapId 0x%X, type 0x%X) for '%s'",
                      static_cast<unsigned>(reinterpret_cast<uintptr_t>(a2)),
                      static_cast<unsigned>(reinterpret_cast<uintptr_t>(a3)), req->mapName);
        NOBLE_INFO("{}", line);
    }
    std::string name = req->mapName;
    const size_t slash = name.find_last_of("\\/");
    s_packEngineName = (slash == std::string::npos ? std::string() : name.substr(0, slash + 1)) + s_packStem;
    s_extraList[0] = s_packEngineName.c_str();
    req->extraCount = 1;
    req->extraNames = s_extraList;
    s_relocPending = true;
    NOBLE_INFO("ElitePack: loading {} with {}", s_packEngineName, name);
    s_loading = true;
    const uint8_t ok = s_mapLoad(a1, a2, a3, req);
    s_loading = false;
    s_relocPending = false;
    NOBLE_INFO("ElitePack: map load returned {} (pack {}, {} adapted resource calls so far)", ok,
               s_loaded ? "loaded" : "not loaded", s_adaptedCalls.load());
    if (ok == 0) {
        // The loader compares the requested id/type with the loaded map header's own values; log both sides so a
        // rejected map shows exactly which field disagrees.
        static const auto readMem = [](uintptr_t address, void* out, size_t size) -> bool {
            __try {
                std::memcpy(out, reinterpret_cast<const void*>(address), size);
                return true;
            } __except (EXCEPTION_EXECUTE_HANDLER) {}
            return false;
        };
        int slot = 0;
        uintptr_t base = 0;
        const bool gotSlot = readMem(s_base + 0xAFBE38, &slot, sizeof(slot));
        const bool gotBase = readMem(s_base + 0xC1A600, &base, sizeof(base));
        {
            char dbg[192];
            std::snprintf(dbg, sizeof(dbg), "ElitePack: header probe slot ok=%d (%d) base ok=%d (0x%llX)", gotSlot, slot,
                          gotBase, static_cast<unsigned long long>(base));
            NOBLE_WARN("{}", dbg);
        }
        if (gotSlot && gotBase && base) {
            uint32_t index = 0;
            uintptr_t table = 0;
            const bool gotIndex = readMem(base + 4 + static_cast<uint64_t>(slot & 0xffff) * 8, &index, sizeof(index));
            const bool gotTable = gotIndex &&
                readMem(s_base + 0x4E39F20 + static_cast<uint64_t>(index >> 28) * 8, &table, sizeof(table));
            {
                char dbg[192];
                std::snprintf(dbg, sizeof(dbg), "ElitePack: header probe index ok=%d (0x%X) table ok=%d (0x%llX)", gotIndex,
                              index, gotTable, static_cast<unsigned long long>(table));
                NOBLE_WARN("{}", dbg);
            }
            if (gotIndex && gotTable && table) {
                int fileId = 0;
                int fileType = 0;
                readMem(table + 0xC + static_cast<uint64_t>(index) * 4, &fileId, sizeof(fileId));
                readMem(table + 0x10 + static_cast<uint64_t>(index) * 4, &fileType, sizeof(fileType));
                char line[224];
                std::snprintf(line, sizeof(line),
                              "ElitePack: map header says id 0x%X type 0x%X (requested id 0x%X type 0x%X)", fileId,
                              fileType, static_cast<unsigned>(reinterpret_cast<uintptr_t>(a2)),
                              static_cast<unsigned>(reinterpret_cast<uintptr_t>(a3)));
                NOBLE_WARN("{}", line);
            }
        }
    }
    {
        wchar_t dumpEnv[8] = {};
        if (GetEnvironmentVariableW(L"NOBLE_SCENARIO_DUMP", dumpEnv, 8) > 0 && dumpEnv[0] == L'3') {
            SharedTagCache::DumpScenarioTag("after map load");
        }
    }
    return ok;
}

uint8_t __fastcall ExtraLoadHook(void* a1, const char* name) {
    const bool ours = s_loading && name && s_packEngineName == name;
    if (ours) {
        const uint8_t slot = At<HeaderSlotFn>(kHeaderSlot)(0xFFFF, name, 2);
        (void)slot;
        NOBLE_INFO("ElitePack: header slot for {}: {}", name, slot ? "ready" : "failed");
    }
    const uint8_t ok = s_extraLoad(a1, name);
    if (ours) NOBLE_INFO("ElitePack: extra cache {} load returned {}", name, ok);
    return ok;
}

// Rebases the pack's packed tag pointers before the engine processes the loaded cache.
void Relocate(uint8_t* cache) {
    uint64_t a = 0, b = 0, hdr = 0;
    uint32_t size = 0;
    std::memcpy(&a, cache + 0x000, 8);
    std::memcpy(&size, cache + 0x024, 4);
    std::memcpy(&b, cache + 0x2F0, 8);
    std::memcpy(&hdr, cache + 0x2F8, 8);
    if (!a || b != s_reloc.virtualBase || size != s_reloc.sectionSize) return;

    const auto encode = At<EncodeFn>(kEncodePointer);
    const uint64_t delta = a - b, end = b + size;
    size_t moved = 0, outside = 0;
    for (uint32_t off : s_reloc.offsets) {
        uint32_t packed = 0;
        std::memcpy(&packed, reinterpret_cast<void*>(a + off), 4);
        if (!packed) continue;
        const uint64_t addr = (static_cast<uint64_t>(packed) << 2) + s_reloc.packedBase;
        if (addr < b || addr >= end) { ++outside; continue; }
        const uint32_t rebased = encode(addr + delta);
        std::memcpy(reinterpret_cast<void*>(a + off), &rebased, 4);
        ++moved;
    }
    int tables = 0;
    for (uint64_t k : {0x08ull, 0x18ull, 0x28ull, 0x38ull}) {
        uint64_t* slot = reinterpret_cast<uint64_t*>(hdr + delta + k);
        if (*slot >= b && *slot < end) { *slot += delta; ++tables; }
    }
    s_relocPending = false;
    s_loaded = true;
    NOBLE_INFO("ElitePack: relocated {} packed pointers ({} outside the pack left alone) and {} table pointers from 0x{:X} to 0x{:X}",
               moved, outside, tables, b, a);
}

bool TryRelocate(uint8_t* cache) {
    __try {
        Relocate(cache);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

// ---- Stage A2: players use the pack's Elite ----------------------------------------------------
// The engine's combined tag table (4th argument of the cache step): 8-byte instances {i16 group
// index, u16 salt, u32 packed address} (count at +0x3FFF8, at most 0x8000), group magics at
// +0x3FFFC + group * 16. A datum is (salt << 16) | index. Packed address -> pointer:
// table[p >> 28] + (p << 2), table = 16 u64 at hr+0x4E39F20.
constexpr uintptr_t kPointerBases = 0x4E39F20;
constexpr uintptr_t kZoneRebuild = 0x08EAF0;   // zone-set rebuild; active-tag bits at arg +0x30B70

uint8_t* TagAt(uint32_t packed) {
    const uint64_t* bases = At<const uint64_t*>(kPointerBases);
    return reinterpret_cast<uint8_t*>(bases[packed >> 28] + (static_cast<uint64_t>(packed) << 2));
}

struct TagTable {
    uint8_t* t;
    int32_t Count() const {
        int32_t n = 0;
        std::memcpy(&n, t + 0x3FFF8, 4);
        return std::clamp(n, 0, 0x8000);
    }
    bool Valid(int32_t i, uint32_t magic) const {
        int16_t group = 0;
        uint16_t salt = 0;
        std::memcpy(&group, t + i * 8, 2);
        std::memcpy(&salt, t + i * 8 + 2, 2);
        if (group < 0 || salt == 0xFFFF) return false;
        uint32_t m = 0;
        std::memcpy(&m, t + 0x3FFFC + group * 16, 4);
        return m == magic;
    }
    uint32_t Address(int32_t i) const { uint32_t a = 0; std::memcpy(&a, t + i * 8 + 4, 4); return a; }
    uint32_t Magic(int32_t i) const {
        int16_t group = 0;
        std::memcpy(&group, t + i * 8, 2);
        if (group < 0) return 0;
        uint32_t m = 0;
        std::memcpy(&m, t + 0x3FFFC + group * 16, 4);
        return m;
    }
    void SetAddress(int32_t i, uint32_t a) { std::memcpy(t + i * 8 + 4, &a, 4); }
    uint32_t Datum(int32_t i) const { uint16_t salt = 0; std::memcpy(&salt, t + i * 8 + 2, 2); return (uint32_t(salt) << 16) | uint32_t(i); }
    int32_t First(uint32_t magic) const { for (int32_t i = 0; i < Count(); ++i) if (Valid(i, magic)) return i; return -1; }
    bool Is(uint32_t datum, uint32_t magic) const {
        if (datum == 0xFFFFFFFFu) return false;
        const int32_t i = datum & 0xFFFF;
        return i < Count() && Valid(i, magic) && Datum(i) == datum;
    }
};
constexpr uint32_t kBipd = 0x62697064, kMatg = 0x6D617467, kPmcg = 0x706D6367;

uint32_t s_activeDatums[3] = {0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu};
std::atomic<bool> s_haveActive{false};
using RebuildFn = void(__fastcall*)(uint8_t*);
RebuildFn s_rebuildOriginal = nullptr;

// Keeps the pack Elite's biped, model and render model active after every zone-set switch (the engine "rebuild").
void __fastcall RebuildHook(uint8_t* state) {
    s_rebuildOriginal(state);
    if (!state || !s_haveActive) return;
    for (uint32_t d : s_activeDatums) {
        if (d == 0xFFFFFFFFu) continue;
        uint32_t* word = reinterpret_cast<uint32_t*>(state + 0x30B70 + ((d >> 3) & 0x1FFC));
        *word |= 1u << (d & 31);
    }
}

bool s_keepPackHealth = false;
int32_t s_pmcgIndex = -1;
uint32_t s_pmcgAddress = 0;
std::vector<uint32_t> s_addressesBefore;   // every tag's address before the pack's step
bool s_keepPackGlobals = false;
bool s_keepDeathEffect = false;

void BeforePackStep(uint8_t* tags) {
    TagTable tt{tags};
    s_pmcgIndex = tt.First(kPmcg);
    s_pmcgAddress = s_pmcgIndex >= 0 ? tt.Address(s_pmcgIndex) : 0;
    s_addressesBefore.resize(static_cast<size_t>(tt.Count()));
    for (int32_t i = 0; i < tt.Count(); ++i) s_addressesBefore[static_cast<size_t>(i)] = tt.Address(i);
}

std::string fmt_hex(uint32_t v) {
    char b[16];
    std::snprintf(b, sizeof(b), " 0x%08X", v);
    return b;
}

std::string GroupText(uint32_t magic) {
    char g[5] = {static_cast<char>(magic >> 24), static_cast<char>(magic >> 16), static_cast<char>(magic >> 8),
                 static_cast<char>(magic), 0};
    for (char& ch : g) if (ch && (ch < 0x20 || ch > 0x7E)) ch = '?';
    return g;
}

// Besides the armour list, the pack carries its own copy of some of the map's one-of-a-kind tags (globals
// such as effect_globals) and loading it points those at the pack's copies. The pack's effect globals made
// Elites play Heart Attack and Grunt Birthday Party. Put back the map's own copy of every one-of-a-kind tag
// the pack replaced (tags of a group the pack has only one of). "-elite-pack-keep-globals" leaves them (test).
void RestoreMapGlobals(TagTable& tt, uint64_t packStart, uint64_t packEnd) {
    std::vector<std::pair<int32_t, uint32_t>> replaced;
    std::vector<uint32_t> packGroups;   // group magic of every tag stored inside the pack
    const int32_t n = std::min<int32_t>(tt.Count(), static_cast<int32_t>(s_addressesBefore.size()));
    for (int32_t i = 0; i < tt.Count(); ++i) {
        const uint32_t now = tt.Address(i);
        if (!now) continue;
        const uint64_t at = reinterpret_cast<uint64_t>(TagAt(now));
        if (at < packStart || at >= packEnd) continue;
        packGroups.push_back(tt.Magic(i));
        if (i < n && s_addressesBefore[static_cast<size_t>(i)] && s_addressesBefore[static_cast<size_t>(i)] != now)
            replaced.push_back({i, s_addressesBefore[static_cast<size_t>(i)]});
    }
    std::string restoredText, keptText;
    int restored = 0, kept = 0;
    for (const auto& [i, before] : replaced) {
        const uint32_t magic = tt.Magic(i);
        if (magic == kPmcg) continue;   // handled above (merged)
        const bool single = std::count(packGroups.begin(), packGroups.end(), magic) == 1;
        std::string& text = (single && !s_keepPackGlobals) ? restoredText : keptText;
        text += (text.empty() ? "" : ", ") + GroupText(magic) + fmt_hex(tt.Datum(i));
        if (single && !s_keepPackGlobals) { tt.SetAddress(i, before); ++restored; } else ++kept;
    }
    NOBLE_INFO("ElitePack: the pack replaced {} of the map's tags; map's copy put back for {} [{}]; pack's kept for {} [{}]{}",
               replaced.size(), restored, restoredText, kept, keptText, s_keepPackGlobals ? " (-elite-pack-keep-globals)" : "");
}

// Armour list (pmcg): 11 lists from +0x20, 12 bytes apart ({i32 count, u32 packed pointer, u32});
// the Elite armour sets are the list at +0x74. A selection is 16 bytes: +0 name (string id), +4 its
// changes {i32 count, u32 pointer, u32}; a change is 12 bytes: region, permutation (string ids), flags.
constexpr size_t kEliteSets = 0x74;

uint32_t EncodePointer(void* p) {
    const uint32_t packed = At<EncodeFn>(kEncodePointer)(reinterpret_cast<uint64_t>(p));
    return TagAt(packed) == p ? packed : 0;
}

// The appearance builder exits early for Elites when the map's Elite armour-effect list (+0x8C) is empty: fill it
// from the pack's list, else from the map's Spartan effects (+0x80).
void EnsureEliteEffects(uint8_t* mapList, uint8_t* packList) {
    auto count = [](const uint8_t* at) { int32_t c = 0; uint32_t p = 0; std::memcpy(&c, at, 4); std::memcpy(&p, at + 4, 4); return (c > 0 && c < 0x400 && p) ? c : 0; };
    if (count(mapList + 0x8C)) return;
    if (packList && count(packList + 0x8C)) {
        std::memcpy(mapList + 0x8C, packList + 0x8C, 12);
        NOBLE_INFO("ElitePack: Elite armour effects (+0x8C) taken from the pack ({} effects)", count(packList + 0x8C));
    } else if (count(mapList + 0x80)) {
        std::memcpy(mapList + 0x8C, mapList + 0x80, 12);
        NOBLE_INFO("ElitePack: Elite armour effects (+0x8C) taken from the map's Spartan effects ({} effects)", count(mapList + 0x80));
    }
}

void MatchStockDamage(const TagTable& tt, uint32_t packElite, uint32_t stockElite);

// ---- First-person Elite arms and body ---------------------------------------------------------
// The globals' player representation (matg +0x474, 0x6C each) names three models per character: +0x14 the
// first-person arms (render model objects\characters\elite\fp\fp, datum at +0x20), +0x24 the first-person body
// (objects\characters\elite\fp_body\fp_body, datum +0x30) and +0x34 the third-person biped (datum +0x40).
// Both first-person models have one armour region, "armor_pieces", with a permutation per Elite set (general,
// officer, space, spec_ops, ultra, story_elite, zealot, default = Minor) and no shoulders / Honor Guard / H2A.
// With the pack's per-piece Elite the first-person arms came out bare (no "armor_pieces" in the third-person
// model to follow). Override: every permutation of the region gets the mesh of the wanted set, so whatever the
// engine picks shows that set. The arms follow the Armory's Arms piece, the body follows the shoulders; pieces
// without a first-person version (Honor Guard, H2A, Council...) use the Armor Set. "-no-fp-override" turns it off.
// Render model: +0x0C regions {i32 count, u32 pointer, u32}, region 0x10 bytes {name, permutations block},
// permutation 0x18 bytes {+0 name, +4.. mesh data}. Set order (pmcg list +0x74): Minor, Spec Ops, Ranger,
// Ultra, Zealot, General, Field Marshal, Officer.
constexpr uint32_t kMode = 0x6D6F6465;
constexpr size_t kPermSize = 0x18;
struct FpRegion {
    uint8_t* perms = nullptr;          // permutation array of "armor_pieces"
    int32_t count = 0;
    std::vector<uint8_t> original;     // untouched copy, restored before every change
    int applied = -1;                  // set shown now (-1 = the model's own)
    const char* label = "";
};
FpRegion s_fp[2];                      // 0 = arms, 1 = body
uint32_t s_setPermutation[8] = {};     // permutation string id of each Elite set (from the map's pmcg list +0x74)
bool s_fpOverride = true;

void SetupFirstPerson(const TagTable& tt, const uint8_t* rep1, const uint8_t* mapPmcg) {
    if (!s_fpOverride || !rep1 || !mapPmcg) return;
    // armor_pieces region name and each set's permutation name, from the map's Elite set list.
    int32_t sets = 0;
    uint32_t setsPtr = 0;
    std::memcpy(&sets, mapPmcg + 0x74, 4);
    std::memcpy(&setsPtr, mapPmcg + 0x78, 4);
    if (sets < 8 || !setsPtr) { NOBLE_WARN("ElitePack: first-person override: Elite set list has {} entries", sets); return; }
    uint32_t region = 0;
    const uint8_t* sel = TagAt(setsPtr);
    for (int k = 0; k < 8; ++k) {
        int32_t changes = 0;
        uint32_t changesPtr = 0;
        std::memcpy(&changes, sel + k * 16 + 4, 4);
        std::memcpy(&changesPtr, sel + k * 16 + 8, 4);
        if (changes < 1 || !changesPtr) { NOBLE_WARN("ElitePack: first-person override: set {} has no change", k); return; }
        const uint8_t* ch = TagAt(changesPtr);
        std::memcpy(&region, ch, 4);
        std::memcpy(&s_setPermutation[k], ch + 4, 4);
    }
    const size_t datumAt[2] = {0x20, 0x30};
    const char* labels[2] = {"arms", "body"};
    for (int w = 0; w < 2; ++w) {
        FpRegion& fp = s_fp[w];
        fp = FpRegion{};
        fp.label = labels[w];
        uint32_t datum = 0xFFFFFFFFu;
        std::memcpy(&datum, rep1 + datumAt[w], 4);
        if (!tt.Is(datum, kMode)) { NOBLE_WARN("ElitePack: first-person {} model not found (0x{:X})", labels[w], datum); continue; }
        uint8_t* mode = TagAt(tt.Address(datum & 0xFFFF));
        int32_t regions = 0;
        uint32_t regionsPtr = 0;
        std::memcpy(&regions, mode + 0x0C, 4);
        std::memcpy(&regionsPtr, mode + 0x10, 4);
        if (regions <= 0 || regions > 64 || !regionsPtr) continue;
        uint8_t* r = TagAt(regionsPtr);
        for (int32_t i = 0; i < regions; ++i) {
            uint32_t name = 0;
            std::memcpy(&name, r + i * 0x10, 4);
            if (name != region) continue;
            int32_t perms = 0;
            uint32_t permsPtr = 0;
            std::memcpy(&perms, r + i * 0x10 + 4, 4);
            std::memcpy(&permsPtr, r + i * 0x10 + 8, 4);
            if (perms <= 0 || perms > 64 || !permsPtr) break;
            fp.perms = TagAt(permsPtr);
            fp.count = perms;
            fp.original.assign(fp.perms, fp.perms + size_t(perms) * kPermSize);
            break;
        }
        NOBLE_INFO("ElitePack: first-person {} model 0x{:X}: {} armour permutations{}", labels[w], datum, fp.count,
                   fp.perms ? "" : " (no armor_pieces region found)");
    }
}

// Shows Elite set 0..7 on first-person model w (arms / body).
void ApplyFirstPersonSet(int w, int set) {
    FpRegion& fp = s_fp[w];
    if (!fp.perms || set < 0 || set > 7 || set == fp.applied) return;
    std::memcpy(fp.perms, fp.original.data(), fp.original.size());
    const uint8_t* want = nullptr;
    for (int32_t i = 0; i < fp.count; ++i) {
        uint32_t name = 0;
        std::memcpy(&name, fp.original.data() + i * kPermSize, 4);
        if (name == s_setPermutation[set]) { want = fp.original.data() + i * kPermSize; break; }
    }
    if (!want) {
        NOBLE_WARN("ElitePack: first-person {} has no permutation for set {}", fp.label, set);
        fp.applied = set;
        return;
    }
    for (int32_t i = 0; i < fp.count; ++i) std::memcpy(fp.perms + i * kPermSize + 4, want + 4, kPermSize - 4);
    fp.applied = set;
    NOBLE_INFO("ElitePack: first-person {} now show Elite set {}", fp.label, set);
}
void DropDeathArmourEffect(const TagTable& tt, uint32_t packElite, uint32_t bodyName);
bool AddNoblePieces(const TagTable& tt, uint32_t render, uint8_t* mapList);
bool HasArg(const wchar_t* name);

void AfterPackStep(uint8_t* tags, uint8_t* cache) {
    TagTable tt{tags};
    uint64_t a = 0;
    uint32_t size = 0;
    std::memcpy(&a, cache, 8);
    std::memcpy(&size, cache + 0x24, 4);
    const uint64_t end = a + size;

    // The map's armour list (pmcg) stays the one in use (its Elite sets also dress the first-person arms); if the
    // pack replaced it, put the map's back. Noble's Elite pieces are added to it later (AddNoblePieces).
    if (s_pmcgIndex >= 0) {
        uint8_t* packList = tt.Address(s_pmcgIndex) != s_pmcgAddress ? TagAt(tt.Address(s_pmcgIndex)) : nullptr;
        if (packList) tt.SetAddress(s_pmcgIndex, s_pmcgAddress);
        if (s_pmcgAddress) EnsureEliteEffects(TagAt(s_pmcgAddress), packList);
    }
    RestoreMapGlobals(tt, a, end);

    // The pack's player Elite: the biped whose tag lies inside the pack's section.
    std::vector<uint32_t> packBipeds;
    for (int32_t i = 0; i < tt.Count(); ++i) {
        if (!tt.Valid(i, kBipd)) continue;
        const uint64_t at = reinterpret_cast<uint64_t>(TagAt(tt.Address(i)));
        if (at >= a && at < end) packBipeds.push_back(tt.Datum(i));
    }
    if (packBipeds.size() != 1) {
        // The pack carries the Spartan Armory biped too. Report enough to tell them apart: the model's
        // shield value and its render-model region count (Elite has a handful of elite_* regions).
        for (uint32_t datum : packBipeds) {
            uint32_t model = 0xFFFFFFFFu;
            std::memcpy(&model, TagAt(tt.Address(datum & 0xFFFF)) + 0x70, 4);
            float shield = 0.0f;
            int32_t regions = -1;
            if (model != 0xFFFFFFFFu) {
                uint8_t* m = TagAt(tt.Address(model & 0xFFFF));
                std::memcpy(&shield, m + 0xD0, 4);
                uint32_t rm = 0;
                std::memcpy(&rm, m + 0x0C, 4);
                if (rm != 0xFFFFFFFFu) std::memcpy(&regions, TagAt(tt.Address(rm & 0xFFFF)) + 0x0C, 4);
            }
            NOBLE_INFO("ElitePack: pack biped 0x{:X} model 0x{:X} shield {:.1f} regions {}", datum, model, shield, regions);
        }
        // Pick the Elite: the pack Elite's model carries the full shield value (210 in the current
        // pack) while the Spartan Armory biped's is much lower (70). Require a clear gap so a future
        // pack with two similar bipeds still bails instead of guessing.
        std::vector<std::pair<float, uint32_t>> ranked;
        for (uint32_t datum : packBipeds) {
            uint32_t model = 0xFFFFFFFFu;
            std::memcpy(&model, TagAt(tt.Address(datum & 0xFFFF)) + 0x70, 4);
            float shield = 0.0f;
            if (model != 0xFFFFFFFFu) std::memcpy(&shield, TagAt(tt.Address(model & 0xFFFF)) + 0xD0, 4);
            ranked.emplace_back(shield, datum);
        }
        std::sort(ranked.begin(), ranked.end(), [](const auto& a, const auto& b) { return a.first > b.first; });
        if (ranked.size() >= 2 && ranked[0].first > ranked[1].first * 1.25f) {
            NOBLE_INFO("ElitePack: pack Elite is biped 0x{:X} (shield {:.0f} vs {:.0f}); the other is the Spartan",
                       ranked[0].second, ranked[0].first, ranked[1].first);
            packBipeds = { ranked[0].second };
        } else {
            NOBLE_WARN("ElitePack: {} bipeds inside the pack, shields too close ({:.0f}/{:.0f}); players keep the map's Elite",
                       packBipeds.size(), ranked.empty() ? 0.0f : ranked[0].first, ranked.size() < 2 ? 0.0f : ranked[1].first);
            return;
        }
    }
    const uint32_t packElite = packBipeds[0];

    const int32_t globals = tt.First(kMatg);
    if (globals < 0) { NOBLE_WARN("ElitePack: no globals tag; players keep the map's Elite"); return; }
    uint8_t* matg = TagAt(tt.Address(globals));
    int32_t reps = 0;
    uint32_t repsPtr = 0;
    std::memcpy(&reps, matg + 0x474, 4);
    std::memcpy(&repsPtr, matg + 0x478, 4);
    if (reps < 2 || !repsPtr) { NOBLE_WARN("ElitePack: globals has {} player representations", reps); return; }
    uint8_t* rep = TagAt(repsPtr);
    reps = std::min(reps, 16);
    uint32_t stockElite = 0;
    std::memcpy(&stockElite, rep + 1 * 0x6C + 0x40, 4);
    if (!tt.Is(stockElite, kBipd)) { NOBLE_WARN("ElitePack: representation 1 isn't a biped (0x{:X})", stockElite); return; }
    std::string changed;
    for (int32_t r = 0; r < reps; ++r) {
        uint32_t d = 0;
        std::memcpy(&d, rep + r * 0x6C + 0x40, 4);
        if (d != stockElite) continue;
        std::memcpy(rep + r * 0x6C + 0x40, &packElite, 4);
        changed += (changed.empty() ? "" : ", ") + std::to_string(r);
    }

    // Keep the pack Elite's biped (+0x70 model) and the model's render model (+0x0C) active.
    MatchStockDamage(tt, packElite, stockElite);
    DropDeathArmourEffect(tt, packElite, 0xE2 /* "body" */);

    SetupFirstPerson(tt, rep + 1 * 0x6C, s_pmcgAddress ? TagAt(s_pmcgAddress) : nullptr);

    s_activeDatums[0] = packElite;
    uint32_t model = 0xFFFFFFFFu, render = 0xFFFFFFFFu;
    std::memcpy(&model, TagAt(tt.Address(packElite & 0xFFFF)) + 0x70, 4);
    if (model != 0xFFFFFFFFu && (model & 0xFFFF) < uint32_t(tt.Count()))
        std::memcpy(&render, TagAt(tt.Address(model & 0xFFFF)) + 0x0C, 4);
    s_activeDatums[1] = model;
    s_activeDatums[2] = render;
    AddNoblePieces(tt, render, s_pmcgAddress ? TagAt(s_pmcgAddress) : nullptr);
    s_haveActive = true;
    NOBLE_INFO("ElitePack: player representation {} -> the pack's player Elite biped 0x{:X} (AI Elites keep the map's 0x{:X}); "
               "kept active: 0x{:X}, 0x{:X}, 0x{:X}", changed, packElite, stockElite, s_activeDatums[0], model, render);
}

// Model (hlmt) +0xC0: "new damage info" block {i32 count, u32 packed pointer, u32}. Element: +0x00 flags,
// +0x04 global indirect material (string id), +0x08 indirect damage section (i16), body from +0x28 (maximum
// vitality), shield from +0x7C (maximum vitality, +0x80 shield material, +0x88 stun time, +0x8C recharge time),
// effect references up to +0xC8, then +0xCC the damage sections block (left alone: it lists the pack model's own).
constexpr uint32_t kHlmt = 0x686C6D74;
constexpr size_t kModelDamageInfo = 0xC0, kDamageCopyEnd = 0xCC;

constexpr size_t kModelShield = 0xD0;

// Model tag of a biped (biped +0x70); DamageInfo() has already checked it is an hlmt.
uint8_t* ModelOf(const TagTable& tt, uint32_t biped) {
    uint32_t model = 0xFFFFFFFFu;
    std::memcpy(&model, TagAt(tt.Address(biped & 0xFFFF)) + 0x70, 4);
    return TagAt(tt.Address(model & 0xFFFF));
}

uint8_t* DamageInfo(const TagTable& tt, uint32_t biped) {
    uint32_t model = 0xFFFFFFFFu;
    std::memcpy(&model, TagAt(tt.Address(biped & 0xFFFF)) + 0x70, 4);
    if (!tt.Is(model, kHlmt)) return nullptr;
    uint8_t* m = TagAt(tt.Address(model & 0xFFFF));
    int32_t count = 0;
    uint32_t packed = 0;
    std::memcpy(&count, m + kModelDamageInfo, 4);
    std::memcpy(&packed, m + kModelDamageInfo + 4, 4);
    return (count >= 1 && packed) ? TagAt(packed) : nullptr;
}

std::string VitalityText(const uint8_t* info) {
    float body = 0, shield = 0, stun = 0, recharge = 0;
    std::memcpy(&body, info + 0x28, 4);
    std::memcpy(&shield, info + 0x7C, 4);
    std::memcpy(&stun, info + 0x88, 4);
    std::memcpy(&recharge, info + 0x8C, 4);
    char text[128];
    std::snprintf(text, sizeof(text), "body %g, shield %g, shield stun %g s, recharge %g s", body, shield, stun, recharge);
    return text;
}

std::string Hex(const uint8_t* p, size_t n) {
    std::string out;
    char b[12];
    for (size_t i = 0; i < n; i += 4) {
        uint32_t v = 0;
        std::memcpy(&v, p + i, 4);
        std::snprintf(b, sizeof(b), "%s%08X", (i % 32 == 0 && i) ? " | " : " ", v);
        out += b;
    }
    return out;
}

// Diagnostic (Elite toughness): logs the model's damage-related data so the pack Elite and the stock Elite
// can be compared line by line. Raw dwords; " | " every 32 bytes.
void DumpDamageData(const char* label, const TagTable& tt, uint32_t biped) {
    (void)label;
    uint32_t model = 0xFFFFFFFFu;
    std::memcpy(&model, TagAt(tt.Address(biped & 0xFFFF)) + 0x70, 4);
    if (!tt.Is(model, kHlmt)) { NOBLE_WARN("ElitePack dump {}: no model", label); return; }
    const uint8_t* m = TagAt(tt.Address(model & 0xFFFF));
    NOBLE_INFO("ElitePack dump {}: biped 0x{:X} model 0x{:X} hlmt[0x00..0x1A0]:{}", label, biped, model, Hex(m, 0x1A0));
    auto block = [&](const uint8_t* base, size_t at, const char* name, size_t stride, int maxItems) {
        (void)name;
        (void)stride;
        int32_t count = 0;
        uint32_t packed = 0;
        std::memcpy(&count, base + at, 4);
        std::memcpy(&packed, base + at + 4, 4);
        if (count <= 0 || !packed) { NOBLE_INFO("ElitePack dump {}: {} (+0x{:X}) empty", label, name, at); return; }
        const uint8_t* e = TagAt(packed);
        (void)e;
        for (int i = 0; i < std::min(count, maxItems); ++i)
            NOBLE_INFO("ElitePack dump {}: {} (+0x{:X}) {}/{}:{}", label, name, at, i, count, Hex(e + i * stride, stride));
    };
    block(m, 0xA8, "hlmt block A8", 0x14, 4);
    block(m, 0xB4, "hlmt block B4", 0x14, 4);
    block(m, 0xC0, "damage info", 0x140, 1);
    const uint8_t* info = DamageInfo(tt, biped);
    if (info) {
        block(info, 0xCC, "damage sections", 0x44, 6);
        block(info, 0xD8, "damage info block D8", 0x10, 2);
        block(info, 0xE4, "damage info block E4", 0x10, 2);
    }
    uint32_t coll = 0xFFFFFFFFu;
    std::memcpy(&coll, m + 0x1C, 4);
    if (coll != 0xFFFFFFFFu && (coll & 0xFFFF) < uint32_t(tt.Count()))
        NOBLE_INFO("ElitePack dump {}: coll 0x{:X} [0x00..0x80]:{}", label, coll, Hex(TagAt(tt.Address(coll & 0xFFFF)), 0x80));
}

bool TryDump(const char* label, const TagTable& tt, uint32_t biped) {
    __try {
        DumpDamageData(label, tt, biped);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

// The pack's Elite comes with its own damage settings, which make it much tougher than MCC's multiplayer Elite
// (two grenades don't take its shields down). Give it the stock player Elite's settings instead.
// "-elite-pack-own-health" keeps the pack's (test).
void MatchStockDamage(const TagTable& tt, uint32_t packElite, uint32_t stockElite) {
    uint8_t* pack = DamageInfo(tt, packElite);
    const uint8_t* stock = DamageInfo(tt, stockElite);
    if (!pack || !stock) {
        NOBLE_WARN("ElitePack: damage info not found (pack {}, stock {}); the pack Elite keeps its own health",
                   pack != nullptr, stock != nullptr);
        return;
    }
    if (HasArg(L"-elite-pack-dump")) {
        if (!TryDump("stock", tt, stockElite)) NOBLE_WARN("ElitePack: stock Elite dump stopped early");
        if (!TryDump("pack", tt, packElite)) NOBLE_WARN("ElitePack: pack Elite dump stopped early");
    }
    const std::string before = VitalityText(pack);
    if (s_keepPackHealth) {
        NOBLE_INFO("ElitePack: pack Elite keeps its own health (-elite-pack-own-health): {}; stock Elite: {}",
                   before, VitalityText(stock));
        return;
    }
    std::memcpy(pack + 0x04, stock + 0x04, 4);
    std::memcpy(pack + 0x10, stock + 0x10, kDamageCopyEnd - 0x10);

    // The model's own shield strength (hlmt +0xD0, float; +0xCC is a count, 1): the stock multiplayer Elite
    // has 79 (its damage info's shield vitality), the pack's Elite 210, so two grenades never took its shields down.
    float* packShield = reinterpret_cast<float*>(ModelOf(tt, packElite) + kModelShield);
    const float stockShield = *reinterpret_cast<const float*>(ModelOf(tt, stockElite) + kModelShield);
    const float oldShield = *packShield;
    (void)oldShield;
    if (stockShield > 0.0f && stockShield < 10000.0f) *packShield = stockShield;
    NOBLE_INFO("ElitePack: pack Elite now uses the stock Elite's damage settings ({}; model shield {:g}, was {:g}; before: {})",
               VitalityText(pack), *packShield, oldShield, before);
}

// Death armour effect. The model's damage sections (hlmt +0xE0, {i32 count, u32 pointer, u32}, 0xC0 bytes each:
// +0x00 name, e.g. "body") carry what happens when a section is used up; the pack Elite's "body" section plays
// objects\characters\spartans\fx\armor_fx\armor_fx_death (in its blocks at +0x1C and +0x84). That effect picks
// the armour effect from the character's "armor_fx" model region, which the pack's Elite doesn't have, so every
// pack Elite played Heart Attack and Grunt Birthday Party. Drop it for the pack Elite only (Elites have no armour
// effect in Noble yet). "-elite-pack-death-fx" keeps it (test).
constexpr uint32_t kEffe = 0x65666665;
constexpr size_t kModelSections = 0xE0, kSectionSize = 0xC0;

void DropDeathArmourEffect(const TagTable& tt, uint32_t packElite, uint32_t bodyName) {
    uint8_t* m = ModelOf(tt, packElite);
    int32_t count = 0;
    uint32_t packed = 0;
    std::memcpy(&count, m + kModelSections, 4);
    std::memcpy(&packed, m + kModelSections + 4, 4);
    if (count <= 0 || count > 32 || !packed) { NOBLE_WARN("ElitePack: pack Elite has no damage sections"); return; }
    uint8_t* sections = TagAt(packed);
    int dropped = 0;
    std::string datums;
    for (int32_t i = 0; i < count; ++i) {
        uint8_t* sec = sections + i * kSectionSize;
        uint32_t name = 0;
        std::memcpy(&name, sec, 4);
        if (name != bodyName) continue;
        for (size_t blockAt : {size_t(0x1C), size_t(0x84)}) {
            int32_t n = 0;
            uint32_t p = 0;
            std::memcpy(&n, sec + blockAt, 4);
            std::memcpy(&p, sec + blockAt + 4, 4);
            if (n <= 0 || n > 8 || !p) continue;
            uint8_t* e = TagAt(p);
            for (size_t k = 0; k + 16 <= size_t(n) * 0x40; k += 4) {
                uint32_t group = 0, datum = 0;
                std::memcpy(&group, e + k, 4);
                if (group != kEffe) continue;
                std::memcpy(&datum, e + k + 12, 4);
                if (datum == 0xFFFFFFFFu) continue;
                if (s_keepDeathEffect) { datums += fmt_hex(datum); continue; }
                const uint32_t none = 0xFFFFFFFFu;
                std::memcpy(e + k + 12, &none, 4);
                datums += fmt_hex(datum);
                ++dropped;
            }
        }
    }
    if (s_keepDeathEffect)
        NOBLE_INFO("ElitePack: pack Elite keeps its death armour effect (-elite-pack-death-fx):{}", datums);
    else
        NOBLE_INFO("ElitePack: dropped {} death armour effect reference(s) from the pack Elite's body section:{}", dropped, datums);
}

bool TryAfterPackStep(uint8_t* tags, uint8_t* cache) {
    __try {
        AfterPackStep(tags, cache);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool TryBeforePackStep(uint8_t* tags) {
    __try {
        BeforePackStep(tags);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

uint8_t __fastcall CacheStepHook(void* a1, void* a2, uint32_t a3, void* tags, void* cache, void* a6) {
    // Diagnostic: does this map load take the same cache step as a multiplayer canvas? (First few calls only.)
    static std::atomic<int> s_stepLog{0};
    if (s_stepLog.fetch_add(1) < 6) {
        NOBLE_INFO("ElitePack: cache step call {} (a1 0x{:X}, a2 0x{:X}, a3 {}, tags 0x{:X}, cache 0x{:X})",
                   s_stepLog.load(), reinterpret_cast<uintptr_t>(a1), reinterpret_cast<uintptr_t>(a2), a3,
                   reinterpret_cast<uintptr_t>(tags), reinterpret_cast<uintptr_t>(cache));
    }
    bool ours = false;
    if (cache && s_relocPending) {
        if (!TryRelocate(static_cast<uint8_t*>(cache))) {
            s_relocPending = false;
            NOBLE_ERROR("ElitePack: relocation faulted; the pack is left as loaded");
        } else if (!s_relocPending && tags) {
            ours = TryBeforePackStep(static_cast<uint8_t*>(tags));   // Relocate cleared it: this is the pack
        }
    }
    const uint8_t ok = s_cacheStepOriginal(a1, a2, a3, tags, cache, a6);
    if (ours && ok && !TryAfterPackStep(static_cast<uint8_t*>(tags), static_cast<uint8_t*>(cache)))
        NOBLE_ERROR("ElitePack: switching players to the pack's Elite faulted");

    if (tags && ok) {
        noble::SharedTagCache::InjectScenarioPalette(static_cast<uint8_t*>(tags));
    }
    return ok;
}

// Resource adapters. A resource interface (vtable at 0x996978, 14 methods 0x1BB6C0..0x1BBDA0) can
// cover several ranges of resource indices, one sub-object per range; with the pack loaded, its
// resources form extra ranges. These methods expect an index inside a single range, so for an index
// in range k (lo != 0) the call is made on a one-range copy of the interface with the index rebased
// (index - lo), as the engine's resadapt does. Context: +0x10 range table (+0x18 ranges {lo, hi}, +0x20 count),
// +0x18 sub-object count, +0x20 sub-objects (16 bytes each).
using ResourceFn = uint64_t(__fastcall*)(uint8_t*, uint64_t, uint64_t, uint64_t);

struct AdapterSlot { uintptr_t rva; bool indexInR8; };
constexpr AdapterSlot kAdapters[14] = {
    {0x1BB6C0, true},  {0x1BB7B4, true},  {0x1BB830, false}, {0x1BBBC4, false}, {0x1BBC30, false},
    {0x1BBC9C, true},  {0x1BBD34, true},  {0x1BBDA0, true},  {0x1BB864, false}, {0x1BB898, false},
    {0x1BB8CC, false}, {0x1BB9B4, false}, {0x1BBA9C, false}, {0x1BBB30, false},
};
ResourceFn s_resourceOriginal[14] = {};

uint64_t Adapt(int slot, uint8_t* ctx, uint64_t rdx, uint64_t r8, uint64_t r9) {
    const ResourceFn original = s_resourceOriginal[slot];
    const bool inR8 = kAdapters[slot].indexInR8;
    const uint32_t index = static_cast<uint32_t>(inR8 ? r8 : rdx);
    uint8_t* table = nullptr;
    uint8_t* subs = nullptr;
    std::memcpy(&table, ctx + 0x10, 8);
    std::memcpy(&subs, ctx + 0x20, 8);
    if (index == 0xFFFFFFFFu || !table || !subs) return original(ctx, rdx, r8, r9);
    const int32_t* ranges = nullptr;
    int32_t count = 0;
    std::memcpy(&ranges, table + 0x18, 8);
    std::memcpy(&count, table + 0x20, 4);
    if (count <= 0 || !ranges) return original(ctx, rdx, r8, r9);
    const int32_t i = static_cast<int32_t>(index & 0xFFFF);
    uint32_t subCount = 0;
    std::memcpy(&subCount, ctx + 0x18, 4);
    for (int32_t k = 0; k < count; ++k) {
        const int32_t lo = ranges[k * 2], hi = ranges[k * 2 + 1];
        if (i - lo < 0 || i >= hi) continue;
        if (static_cast<uint32_t>(k) >= subCount || lo == 0) break;
        int32_t oneRange[2] = {0, hi - lo};
        alignas(16) uint8_t oneTable[0x28] = {};
        const int32_t* oneRangePtr = oneRange;
        const uint32_t one = 1;
        std::memcpy(oneTable + 0x18, &oneRangePtr, 8);
        std::memcpy(oneTable + 0x20, &one, 4);
        alignas(16) uint8_t sub[16];
        std::memcpy(sub, subs + static_cast<size_t>(k) * 16, 16);
        alignas(16) uint8_t copy[0x30];
        std::memcpy(copy, ctx, 0x30);
        uint8_t* tablePtr = oneTable;
        uint8_t* subPtr = sub;
        std::memcpy(copy + 0x10, &tablePtr, 8);
        std::memcpy(copy + 0x18, &one, 4);
        std::memcpy(copy + 0x20, &subPtr, 8);
        const uint64_t rebased = (index & 0xFFFF0000u) | static_cast<uint32_t>(i - lo);
        ++s_adaptedCalls;
        return inR8 ? original(copy, rdx, rebased, r9) : original(copy, rebased, r8, r9);
    }
    return original(ctx, rdx, r8, r9);
}

template <int Slot>
uint64_t __fastcall AdapterHook(uint8_t* ctx, uint64_t rdx, uint64_t r8, uint64_t r9) {
    return Adapt(Slot, ctx, rdx, r8, r9);
}

using AdapterHookFn = uint64_t(__fastcall*)(uint8_t*, uint64_t, uint64_t, uint64_t);
constexpr AdapterHookFn kAdapterHooks[14] = {
    &AdapterHook<0>, &AdapterHook<1>, &AdapterHook<2>, &AdapterHook<3>, &AdapterHook<4>, &AdapterHook<5>, &AdapterHook<6>,
    &AdapterHook<7>, &AdapterHook<8>, &AdapterHook<9>, &AdapterHook<10>, &AdapterHook<11>, &AdapterHook<12>, &AdapterHook<13>,
};

bool InstallAdapters() {
    for (int k = 0; k < 14; ++k) {
        // Each method must still be in the interface's vtable where this build has it.
        uint64_t inTable = 0;
        std::memcpy(&inTable, At<void*>(0x996978 + (k < 3 ? k * 8 : k * 8 + 8)), 8);
        if (inTable != s_base + kAdapters[k].rva) {
            NOBLE_WARN("ElitePack: resource method {} isn't at RVA 0x{:X}", k, kAdapters[k].rva);
            return false;
        }
        if (!HookManager::InstallHook(At<void*>(kAdapters[k].rva), reinterpret_cast<void*>(kAdapterHooks[k]),
                                      reinterpret_cast<void**>(&s_resourceOriginal[k]), "elitepack_resource")) return false;
    }
    return true;
}

// ---- Stage A3: the Armory's six Elite pieces -------------------------------------------------
// The appearance builder 0x1055EC(record, mode, unit, player) reads one byte per armour list from the
// appearance record: +0x10..+0x16 the Spartan piece lists (helmet, left shoulder, right shoulder, chest,
// wrist, utility, knees), +0x17 the Elite set, +0x18 / +0x19 effects. With the pack merged in, the
// pack's Elite pieces ride on the first six lists (helmet, shoulders, chest, forearms, legs), so for an
// Elite a copy of the record gets the six piece indices and knees 0xFF (the engine elitepicks). The Elite set
// byte is kept: the map's Elite set still dresses the first-person arms. Calls: spawn 0x4AC36, live change
// 0x447B0, another machine's player 0x3B184F, unit copy 0x524CD5.
constexpr uintptr_t kAppearanceBuilder = 0x1055EC;
constexpr uintptr_t kBuilderCalls[5] = {0x04AC36, 0x0447B0, 0x2C64DF, 0x3B184F, 0x524CD5};
using BuilderFn = uint64_t(__fastcall*)(uint8_t*, uint32_t, void*, void*);

// Armory presets, in ArmorCustomizer's kEliteSets order: 0..8 Reach's own + Honor Guard, 9 Counselor (10 = Custom).
constexpr int kArmorySets = 10;

// Appends one selection per permutation of the pack Elite's piece regions (discovered dynamically via the engine's
// string_id table so both 8-region elite_armor.map and 9-region noble_elite_pack.map layouts work) to the map's
// six piece lists (`0x20..0x5C`). A pick is then `s_nobleBase[slot] + permIndex`.
constexpr uintptr_t kStringIdName = 0x09D91C;   // const char* __fastcall string_id_get_string_const(uint32_t id)
using StringIdNameFn = const char*(__fastcall*)(uint32_t);
constexpr size_t kPieceLists[6] = {0x20, 0x2C, 0x38, 0x44, 0x50, 0x5C};

std::mutex s_packMutex;
std::atomic<int> s_nobleBase[6] = {-1, -1, -1, -1, -1, -1};
std::vector<std::string> s_packPerms[6];
std::atomic<int> s_nobleGen{0};

bool StringIdTableReady() {
    if (!s_base) return false;
    static constexpr uint8_t kPrologue[8] = {0x40, 0x53, 0x48, 0x83, 0xEC, 0x20, 0x83, 0x4C};
    return std::memcmp(At<const void*>(kStringIdName), kPrologue, sizeof(kPrologue)) == 0;
}

std::string LowerAscii(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

std::string StringIdLower(uint32_t id) {
    if (id == 0 || !StringIdTableReady()) return {};
    const char* raw = At<StringIdNameFn>(kStringIdName)(id);
    return raw ? LowerAscii(raw) : std::string();
}

int SlotForRegionName(const std::string& name, bool* preferred) {
    struct Rule {
        int slot;
        const char* name;
        bool pref;
    };
    static constexpr Rule kRules[] = {
        {0, "elite_helmet",         true},
        {0, "helmet",               false},
        {1, "elite_left_shoulder",  true},
        {1, "left_shoulder",        false},
        {2, "elite_right_shoulder", true},
        {2, "right_shoulder",       false},
        {3, "elite_chest",          true},
        {3, "chest",                false},
        {4, "elite_forearms",       true},
        {4, "elite_arms",           true},
        {4, "forearms",             false},
        {4, "arms",                 false},
        {5, "elite_legs",           true},
        {5, "legs",                 false},
    };
    for (const auto& r : kRules) {
        if (name == r.name) {
            *preferred = r.pref;
            return r.slot;
        }
    }
    *preferred = false;
    return -1;
}

bool NobleActive() { return s_nobleBase[0].load() >= 0; }

int NoblePick(int slot, std::string want) {
    if (slot < 0 || slot >= 6 || want.empty()) return -1;
    want = LowerAscii(std::move(want));
    std::lock_guard<std::mutex> lock(s_packMutex);
    const int base = s_nobleBase[slot].load();
    if (base < 0) return -1;
    const auto& perms = s_packPerms[slot];
    auto findExact = [&](const std::string& target) -> int {
        for (size_t j = 0; j < perms.size(); ++j)
            if (perms[j] == target) return base + static_cast<int>(j);
        return -1;
    };
    int idx = findExact(want);
    if (idx < 0 && want == "minor") idx = findExact("default");
    else if (idx < 0 && want == "default") idx = findExact("minor");
    return idx;
}

// Armory set (kEliteSets order) -> piece name per slot, for the pack.
std::string NobleSetPiece(int set, int slot) {
    static const char* kSet[9] = {"minor", "spec_ops", "space", "ultra", "zealot", "general", "story_elite", "officer",
                                  "honor_guard"};
    if (set == 9) {   // Counselor: Council helmet, Schism shoulders, Honor Guard chest and legs, Minor arms
        static const char* kC[6] = {"council", "schism", "schism", "honor_guard", "default", "honor_guard"};
        return kC[slot];
    }
    return kSet[(set >= 0 && set < 9) ? set : 0];
}

uint8_t SetPick(int set, int slot) {
    if (NobleActive()) {
        int idx = NoblePick(slot, NobleSetPiece(set, slot));
        if (idx < 0) idx = NoblePick(slot, "minor");
        if (idx < 0) idx = NoblePick(slot, "default");
        if (idx < 0) idx = s_nobleBase[slot].load();
        if (idx >= 0) return static_cast<uint8_t>(idx);
    }
    return 0;   // no Elite pieces added to pick
}

// Discovers the pack Elite's six armour regions on `render` by resolved string_id and appends one selection per
// permutation to the map's pmcg piece lists. Stages all six allocations before committing so failure leaves `pmcg` intact.
bool AddNoblePieces(const TagTable& tt, uint32_t render, uint8_t* mapList) {
    {
        std::lock_guard<std::mutex> lock(s_packMutex);
        for (int s = 0; s < 6; ++s) {
            s_nobleBase[s].store(-1);
            s_packPerms[s].clear();
        }
    }
    if (!mapList || !tt.Is(render, kMode)) return false;
    if (!StringIdTableReady()) {
        NOBLE_WARN("ElitePack: string_id lookup at 0x{:X} doesn't match expected prologue; can't index pack Elite pieces",
                   kStringIdName);
        return false;
    }
    uint8_t* mode = TagAt(tt.Address(render & 0xFFFF));
    int32_t regions = 0;
    uint32_t regionsPtr = 0;
    std::memcpy(&regions, mode + 0x0C, 4);
    std::memcpy(&regionsPtr, mode + 0x10, 4);
    if (regions <= 0 || regions > 64 || !regionsPtr) {
        NOBLE_WARN("ElitePack: pack render model 0x{:X} has invalid region count {}", render, regions);
        return false;
    }
    uint8_t* reg = TagAt(regionsPtr);
    int regionOf[6] = {-1, -1, -1, -1, -1, -1};
    bool regionPref[6] = {};
    std::string regionSummary;
    for (int32_t r = 0; r < regions; ++r) {
        uint32_t regionName = 0;
        int32_t permCount = 0;
        std::memcpy(&regionName, reg + r * 16, 4);
        std::memcpy(&permCount, reg + r * 16 + 4, 4);
        const std::string rname = StringIdLower(regionName);
        if (!regionSummary.empty()) regionSummary += ", ";
        regionSummary += "[" + std::to_string(r) + "] \"" + (rname.empty() ? "?" : rname) + "\"(" +
                         std::to_string(permCount) + ")";
        bool pref = false;
        const int slot = SlotForRegionName(rname, &pref);
        if (slot >= 0 && (regionOf[slot] < 0 || (pref && !regionPref[slot]))) {
            regionOf[slot] = r;
            regionPref[slot] = pref;
        }
    }
    for (int slot = 0; slot < 6; ++slot) {
        if (regionOf[slot] < 0) {
            NOBLE_WARN("ElitePack: pack render model 0x{:X} missing piece slot {} (matched {}/{}/{}/{}/{}/{}; regions: {})",
                       render, slot, regionOf[0], regionOf[1], regionOf[2], regionOf[3], regionOf[4], regionOf[5],
                       regionSummary);
            return false;
        }
    }

    struct StagedList {
        uint8_t* mem = nullptr;
        int32_t total = 0;
        uint32_t packed = 0;
        int base = -1;
        int32_t added = 0;
    };
    StagedList staged[6]{};
    std::vector<std::string> packPerms[6];
    auto cleanup = [&]() {
        for (auto& s : staged) {
            if (s.mem) VirtualFree(s.mem, 0, MEM_RELEASE);
        }
    };

    for (int slot = 0; slot < 6; ++slot) {
        uint8_t* region = reg + regionOf[slot] * 16;
        uint32_t regionName = 0, permsPtr = 0;
        int32_t n = 0;
        std::memcpy(&regionName, region, 4);
        std::memcpy(&n, region + 4, 4);
        std::memcpy(&permsPtr, region + 8, 4);
        if (n <= 0 || n > 64 || !permsPtr) {
            NOBLE_WARN("ElitePack: pack region {} (slot {}) has invalid permutations (count {})", regionOf[slot], slot, n);
            cleanup();
            return false;
        }
        const uint8_t* perms = TagAt(permsPtr);
        uint8_t* list = mapList + kPieceLists[slot];
        int32_t mc = 0;
        uint32_t msPtr = 0;
        std::memcpy(&mc, list, 4);
        std::memcpy(&msPtr, list + 4, 4);
        // AppearanceBuilder (0x1055EC) loads each byte index with movsx, so valid positive indices are 0..127.
        if (mc < 0 || mc > 0x7F || mc + n > 0x7F || (mc && !msPtr)) {
            NOBLE_WARN("ElitePack: armour list {} overflow ({} existing + {} pack pieces > 127 signed limit)",
                       slot, mc, n);
            cleanup();
            return false;
        }
        const size_t selBytes = static_cast<size_t>(mc + n) * 16;
        auto* mem = static_cast<uint8_t*>(VirtualAlloc(nullptr, selBytes + static_cast<size_t>(n) * 12,
                                                       MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
        if (!mem) {
            cleanup();
            return false;
        }
        staged[slot].mem = mem;
        if (mc) std::memcpy(mem, TagAt(msPtr), static_cast<size_t>(mc) * 16);
        uint8_t* changes = mem + selBytes;
        bool ok = true;
        packPerms[slot].reserve(static_cast<size_t>(n));
        for (int32_t j = 0; j < n && ok; ++j) {
            uint32_t permName = 0;
            std::memcpy(&permName, perms + j * kPermSize, 4);
            packPerms[slot].push_back(StringIdLower(permName));
            uint8_t* ch = changes + j * 12;
            const uint32_t zero = 0, one = 1;
            std::memcpy(ch, &regionName, 4);
            std::memcpy(ch + 4, &permName, 4);
            std::memcpy(ch + 8, &zero, 4);
            const uint32_t chPacked = EncodePointer(ch);
            uint8_t* sel = mem + static_cast<size_t>(mc + j) * 16;
            std::memcpy(sel, &permName, 4);
            std::memcpy(sel + 4, &one, 4);
            std::memcpy(sel + 8, &chPacked, 4);
            std::memcpy(sel + 12, &zero, 4);
            ok = chPacked != 0;
        }
        const uint32_t packed = ok ? EncodePointer(mem) : 0;
        if (!packed) {
            cleanup();
            NOBLE_WARN("ElitePack: couldn't address the pack Elite pieces (slot {})", slot);
            return false;
        }
        staged[slot].total = mc + n;
        staged[slot].packed = packed;
        staged[slot].base = mc;
        staged[slot].added = n;
    }

    for (int slot = 0; slot < 6; ++slot) {
        uint8_t* list = mapList + kPieceLists[slot];
        std::memcpy(list, &staged[slot].total, 4);
        std::memcpy(list + 4, &staged[slot].packed, 4);
    }
    {
        std::lock_guard<std::mutex> lock(s_packMutex);
        for (int slot = 0; slot < 6; ++slot) {
            s_nobleBase[slot].store(staged[slot].base);
            s_packPerms[slot] = std::move(packPerms[slot]);
        }
    }
    s_nobleGen.fetch_add(1);
    NOBLE_INFO("ElitePack: pack Elite pieces added to the armour lists (base helmet {}+{}, shoulders {}+{}/{}+{}, chest {}+{}, "
               "forearms {}+{}, legs {}+{})",
               staged[0].base, staged[0].added, staged[1].base, staged[1].added, staged[2].base, staged[2].added,
               staged[3].base, staged[3].added, staged[4].base, staged[4].added, staged[5].base, staged[5].added);
    return true;
}

// Armory piece ids -> index in the map's armour list (Noble's pieces, see AddNoblePieces), or -1.
int PieceIndex(int slot, const std::string& id) {
    return NobleActive() ? NoblePick(slot, id) : -1;
}

// Elite set (0..7, pmcg list +0x74 order) whose first-person version matches a picked piece, or -1 when the
// piece has none (Honor Guard, Council, Ossoona, Schism, Combat). slot: 1 left shoulder, 4 forearms.
int SetOfPiece(int slot, int index) {
    static const char* kSetIds[8] = {"minor", "spec_ops", "space", "ultra", "zealot", "general", "story_elite", "officer"};
    if (slot != 1 && slot != 4) return -1;
    std::lock_guard<std::mutex> lock(s_packMutex);
    const int base = s_nobleBase[slot].load();
    if (base < 0) return -1;
    const int j = index - base;
    if (j < 0 || j >= static_cast<int>(s_packPerms[slot].size())) return -1;
    std::string id = s_packPerms[slot][j];
    if (id == "default") id = "minor";
    for (int k = 0; k < 8; ++k) if (id == kSetIds[k]) return k;
    return -1;
}
std::filesystem::path s_appearancePath;
std::mutex s_picksMutex;
uint8_t s_localPicks[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t s_localSpartanEffect = 0;
uint8_t s_localEliteEffect = 0;
int s_localSet = -1;
uint8_t s_localColors[4] = {1, 10, 14, 14};
uint8_t s_localCustomFlags = 0;
float s_localCustomRgb[4][3] = {};
int s_localLightsBrightnessIdx = 1; // 0..130 -> 0.5f + idx * 0.5f (default 1 = x1.0)
uint8_t s_localSpartanColors[4] = {1, 10, 14, 14};
uint8_t s_localSpartanCustomFlags = 0;
float s_localSpartanCustomRgb[4][3] = {};
int s_localSpartanLightsBrightnessIdx = 1;
uint8_t s_localEliteColors[4] = {1, 10, 14, 14};
uint8_t s_localEliteCustomFlags = 0;
float s_localEliteCustomRgb[4][3] = {};
int s_localEliteLightsBrightnessIdx = 1;
uint8_t s_localSpartanVisorIndex = 0;
uint8_t s_localSpartanVisorItemIndex = 0; // 0..29 in kArmoryVisorItemIds / kArmoryVisorRgb order
bool s_localSpartanVisorCustom = false;
float s_localSpartanVisorRgb[3] = {0.85f, 0.53f, 0.14f};
bool s_localSpeciesElite = false;   // appearance.json "species" == 1 (the local player's preferred species)
static std::atomic<int> s_localFlameVariantIndex{-1};
std::atomic<int> s_localSettingsIndex{-1};   // settings index the engine last applied the local profile to
std::atomic<const uint8_t*> s_localUserSettingsPtr{nullptr};
std::filesystem::file_time_type s_picksStamp{};
std::atomic<int> s_picksLogged{0};

static uint8_t ParsePmcgEffect(const nlohmann::json& obj, int primaryColor, int& outFlameVar) {
    outFlameVar = -1;
    if (!obj.is_object()) return 0;
    int rawIdx = obj.value("armor_effect_index", 0);
    const std::string id = obj.value("armor_effect_id", std::string());
    if (rawIdx >= kArmorEffectVariantBase) {
        outFlameVar = rawIdx - kArmorEffectVariantBase;
        return 1;
    }
    if (rawIdx == kArmorEffectAutoIndex || id == "flames_primary_auto") {
        outFlameVar = std::clamp(primaryColor, 0, 29);
        return 1;
    }
    if (id.rfind("flame_pri_", 0) == 0) {
        const std::string suffix = id.substr(10);
        for (int i = 0; i < 30; ++i) {
            if (suffix == kPrimaryFlames[static_cast<size_t>(i)].id) {
                outFlameVar = i;
                break;
            }
        }
        return 1;
    }
    if (id == "flames" || rawIdx == 5) return 1;
    if (id == "blue_flames" || rawIdx == 6) return 2;
    if (id == "heart_attack" || rawIdx == 2) return 3;
    if (id == "pestilence" || rawIdx == 3) return 4;
    if (id == "inclement_weather" || rawIdx == 4) return 5;
    if (id == "birthday_party" || rawIdx == 1) return 6;
    return 0;
}

// Re-reads data\appearance.json when it changes (an Armory save mid-match reaches the next spawn).
void RefreshLocalPicks() {
    std::error_code ec;
    const auto stamp = std::filesystem::last_write_time(s_appearancePath, ec);
    static int s_seenNobleGen = -1;
    if (ec || (stamp == s_picksStamp && s_seenNobleGen == s_nobleGen.load())) return;
    s_picksStamp = stamp;
    s_seenNobleGen = s_nobleGen.load();
    nlohmann::json j;
    if (!SaveCodec::ReadJson(s_appearancePath, j)) return;
    try {
        s_localSpeciesElite = j.value("species", 0) == 1;
        const nlohmann::json emptyObj = nlohmann::json::object();
        const auto& el = (j.contains("elite") && j["elite"].is_object()) ? j["elite"] : emptyObj;
        const int rawSet = el.value("armor_set", 0);
        const int set = (rawSet >= 0 && rawSet < kArmorySets) ? rawSet : 0;
        static const char* kKeys[6] = {"helmet_id", "left_shoulder_id", "right_shoulder_id", "chest_id", "arms_id", "legs_id"};
        if (NobleActive()) {
            for (int k = 0; k < 6; ++k) {
                const int idx = PieceIndex(k, el.value(kKeys[k], std::string()));
                s_localPicks[k] = static_cast<uint8_t>(idx >= 0 ? idx : SetPick(set, k));
            }
        } else {
            std::fill(std::begin(s_localPicks), std::end(s_localPicks), uint8_t{0xFF});
        }
        s_localSet = set;

        auto parseColorsFrom = [&](const nlohmann::json& src, uint8_t colors[4], uint8_t& cFlags, float customRgb[4][3], int& bIdx) {
            colors[0] = static_cast<uint8_t>(std::clamp(src.value("primary_color", j.value("primary_color", 1)), 0, 29));
            colors[1] = static_cast<uint8_t>(std::clamp(src.value("secondary_color", j.value("secondary_color", 10)), 0, 29));
            colors[2] = static_cast<uint8_t>(std::clamp(src.value("accent_color", j.value("accent_color", 14)), 0, 29));
            colors[3] = static_cast<uint8_t>(std::clamp(src.value("lights_color", j.value("lights_color", 14)), 0, 29));

            const float rawB = std::clamp(src.value("lights_brightness", j.value("lights_brightness", 1.0f)), 0.5f, 65.5f);
            bIdx = std::clamp(static_cast<int>(std::lround((rawB - 0.5f) * 2.0f)), 0, 130);

            cFlags = 0;
            if (src.value("primary_custom", false))   cFlags |= (1 << 0);
            if (src.value("secondary_custom", false)) cFlags |= (1 << 1);
            if (src.value("accent_custom", false))    cFlags |= (1 << 2);
            if (src.value("lights_custom", false))    cFlags |= (1 << 3);

            auto readRgb = [&](const char* key, float out[3]) {
                if (src.contains(key) && src[key].is_array() && src[key].size() == 3) {
                    for (int i = 0; i < 3; ++i) out[i] = std::clamp(src[key][i].get<float>(), 0.0f, 1.0f);
                }
            };
            readRgb("primary_custom_rgb", customRgb[0]);
            readRgb("secondary_custom_rgb", customRgb[1]);
            readRgb("accent_custom_rgb", customRgb[2]);
            readRgb("lights_custom_rgb", customRgb[3]);
        };

        parseColorsFrom(j, s_localSpartanColors, s_localSpartanCustomFlags, s_localSpartanCustomRgb, s_localSpartanLightsBrightnessIdx);
        const auto& elColSrc = el.contains("primary_color") ? el : j;
        parseColorsFrom(elColSrc, s_localEliteColors, s_localEliteCustomFlags, s_localEliteCustomRgb, s_localEliteLightsBrightnessIdx);

        const bool useElColors = s_localSpeciesElite;
        std::memcpy(s_localColors, useElColors ? s_localEliteColors : s_localSpartanColors, sizeof(s_localColors));
        s_localCustomFlags = useElColors ? s_localEliteCustomFlags : s_localSpartanCustomFlags;
        std::memcpy(s_localCustomRgb, useElColors ? s_localEliteCustomRgb : s_localSpartanCustomRgb, sizeof(s_localCustomRgb));
        s_localLightsBrightnessIdx = useElColors ? s_localEliteLightsBrightnessIdx : s_localSpartanLightsBrightnessIdx;

        if (j.contains("spartan") && j["spartan"].is_object()) {
            const auto& sp = j["spartan"];
            int uiVisorIdx = std::clamp(sp.value("visor_color_index", 0), 0, 29);
            const std::string visorId = sp.value("visor_id", std::string());
            if (visorId == "black") uiVisorIdx = 3;
            else if (visorId == "gold") uiVisorIdx = 4;
            // Same resolution as ReachHost's profile path: resolve by string id (the Armory UI catalog order differs
            // from MCC's item order for ice/crimson/onyx/violet) and apply the engine's stock 3/4 Gold/Black swap.
            s_localSpartanVisorItemIndex = static_cast<uint8_t>(std::clamp(ArmoryVisorItemIndex(uiVisorIdx, visorId), 0, 29));
            s_localSpartanVisorIndex = static_cast<uint8_t>(ArmoryVisorEngineIndex(uiVisorIdx, visorId));
            s_localSpartanVisorCustom = (s_localSpartanVisorItemIndex > 0 && visorId != "default") || sp.value("visor_custom", false);
            s_localSpartanVisorRgb[0] = kArmoryVisorRgb[s_localSpartanVisorItemIndex][0];
            s_localSpartanVisorRgb[1] = kArmoryVisorRgb[s_localSpartanVisorItemIndex][1];
            s_localSpartanVisorRgb[2] = kArmoryVisorRgb[s_localSpartanVisorItemIndex][2];
            if (sp.contains("visor_custom_rgb") && sp["visor_custom_rgb"].is_array() && sp["visor_custom_rgb"].size() == 3) {
                for (int i = 0; i < 3; ++i) s_localSpartanVisorRgb[i] = std::clamp(sp["visor_custom_rgb"][i].get<float>(), 0.0f, 1.0f);
            }
        }

        // Parse Spartan & Elite effects and flame variants
        int spFlameVar = -1;
        const auto& sp = (j.contains("spartan") && j["spartan"].is_object()) ? j["spartan"] : emptyObj;
        s_localSpartanEffect = ParsePmcgEffect(sp, s_localSpartanColors[0], spFlameVar);

        int elFlameVar = -1;
        s_localEliteEffect = ParsePmcgEffect(el, s_localEliteColors[0], elFlameVar);

        const int flameVar = s_localSpeciesElite ? elFlameVar : spFlameVar;
        s_localFlameVariantIndex.store(flameVar);
        ParticleTint::SetVariantIndex(flameVar);

        NOBLE_INFO("ElitePack: Armory Elite pieces {} {} {} {} {} {} (set {}), colors {}/{}/{}/{}, lightsB x{:.1f}, visor {} (item {}), flameVar {}",
                   s_localPicks[0], s_localPicks[1], s_localPicks[2], s_localPicks[3],
                   s_localPicks[4], s_localPicks[5], set,
                   s_localColors[0], s_localColors[1], s_localColors[2], s_localColors[3],
                   0.5f + static_cast<float>(s_localLightsBrightnessIdx) * 0.5f,
                   s_localSpartanVisorIndex, s_localSpartanVisorItemIndex, flameVar);
    } catch (...) {
    }
}

// ---- Replicated Elite appearance ------------------------------------------------------------
// The engine's appearance record (player + 0xD8, 0x28 bytes) is also the per-player block it syncs
// between machines: 0x2C837C(player, block) copies a 0x20-byte block starting at record+0x10 into
// player+0xE8 and pushes it to the profile manager (0x3A9E4 -> settings +0x310), and 0x2C82F0 hands
// that block back for the settings sync. Layout, verified in the builder 0x1055EC:
//   +0x00 flags, +0x04 species, +0x10..+0x16 seven armour-list indices, +0x17 Elite armor set,
//   +0x18 Spartan armor effect, +0x19 Elite armor effect.
// For an Elite the seven list indices are the player's SPARTAN pieces and the pack reuses them for the
// six Elite piece indices (the pack merges Elite-region changes into the same-named selections), with
// 0xFF in +0x16 as a marker. Writing the marker + pieces into the local player's record makes the
// engine replicate the sender's own Elite pieces; the receiving machine then no longer has to guess
// with its own Armory picks (the old behaviour made every Elite look like the local player).
constexpr int kRecordPieceBase = 0x10;
constexpr int kRecordEliteSet = 0x17;
constexpr int kRecordEliteFx = 0x19;
constexpr uint8_t kElitePiecesMarker = 0xFF;
constexpr uintptr_t kPlayerRecordOffset = 0xD8;

// Return addresses of the 0x1055EC call sites. Only spawn (0x04AC36) and live change (0x0447B0) pass
// the local player's own record (player + 0xD8) straight from the player struct; 0x3B184F (another
// machine's player) and 0x524CD5 (unit copy) pass a network/copy record and must keep its bytes.
constexpr uintptr_t kBuilderReturnSpawn = 0x04AC3B;
constexpr uintptr_t kBuilderReturnLive  = 0x0447B5;

constexpr uintptr_t kApplyProfile = 0x2EDA08;   // void(int settingsIndex, uint8_t* player)
constexpr uintptr_t kBuildRecord  = 0x2C7A18;   // void(const uint8_t* userSettings, uint8_t* outRecord)
using ApplyProfileFn = void(__fastcall*)(int, uint8_t*);
using RecordSetFn = void(__fastcall*)(uint8_t*, uint8_t*);
using BuildRecordFn = void(__fastcall*)(const uint8_t*, uint8_t*);
static ApplyProfileFn s_applyProfileOriginal = nullptr;
static RecordSetFn s_recordSetOriginal = nullptr;
static BuildRecordFn s_buildRecordOriginal = nullptr;
constexpr uintptr_t kRecordSet = 0x2C837C;   // void(playerSettings*, uint8_t block[0x20])

BuilderFn s_builder = nullptr;

static bool IsLocalPlayerDatum(const uint8_t* player) {
    if (!player || !s_base) return false;
    __try {
        const uint32_t tlsIdx = *reinterpret_cast<const uint32_t*>(s_base + 0xC17B18);
        const uintptr_t tlsArray = *reinterpret_cast<const uintptr_t*>(
            __readgsqword(0x58) + static_cast<uintptr_t>(tlsIdx) * 8u);
        if (!tlsArray) return false;
        const uint8_t* pmg = *reinterpret_cast<const uint8_t* const*>(tlsArray + 0x158);
        if (!pmg) return false;
        const uint8_t* players = *reinterpret_cast<const uint8_t* const*>(tlsArray + 0x18);
        if (!players) return false;
        const uint8_t* playerEntries = *reinterpret_cast<const uint8_t* const*>(players + 0x50);
        if (!playerEntries) return false;
        const int32_t maxPlayers = std::clamp(*reinterpret_cast<const int32_t*>(players + 0x44), 0, 16);

        if (player >= playerEntries && player < playerEntries + static_cast<size_t>(maxPlayers) * 0x490u) {
            const size_t diff = player - playerEntries;
            if ((diff % 0x490u) == 0) {
                const int32_t playerIdx = static_cast<int32_t>(diff / 0x490u);
                const int32_t localUser = *reinterpret_cast<const int32_t*>(pmg + 0x74 + playerIdx * 4);
                return (localUser >= 0 && localUser < 4);
            }
        }
        return false;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

// Stamps the local player's extended Noble customization (Lights color, Lights brightness, Custom wheel RGB,
// Spartan visor finish 0..29, and Elite piece picks) into a 0x28-byte appearance record so 0x3A2500/0x3A296C
// replicates it across all peers into player+0xD8.
static void StampLocalAppearanceRecordLocked(uint8_t* rec, bool elite) {
    if (!rec) return;
    const uint8_t* cols = elite ? s_localEliteColors : s_localSpartanColors;
    const uint8_t cFlags = elite ? s_localEliteCustomFlags : s_localSpartanCustomFlags;
    const auto& cRgb = elite ? s_localEliteCustomRgb : s_localSpartanCustomRgb;

    const bool lightsCust = (cFlags & (1 << 3)) != 0;
    rec[0x1B] = static_cast<uint8_t>((lightsCust ? 0x60 : 0x40) | (cols[3] & 0x1F));
    if (lightsCust) {
        const int r6 = std::clamp(static_cast<int>(std::lround(cRgb[3][0] * 5.0f)), 0, 5);
        const int g6 = std::clamp(static_cast<int>(std::lround(cRgb[3][1] * 5.0f)), 0, 5);
        const int b6 = std::clamp(static_cast<int>(std::lround(cRgb[3][2] * 5.0f)), 0, 5);
        const uint16_t w26 = static_cast<uint16_t>(0xF2 + (r6 * 36 + g6 * 6 + b6));
        std::memcpy(rec + 0x26, &w26, sizeof(w26));
    }
    rec[0x01] = cols[0];
    rec[0x02] = cols[1];
    const int flameVar = s_localFlameVariantIndex.load();
    rec[0x1A] = (flameVar >= 0 && flameVar < 30) ? static_cast<uint8_t>(0x80 | (flameVar & 0x1F)) : 0;
    rec[0x18] = s_localSpartanEffect;
    rec[0x19] = s_localEliteEffect;

    if (elite) {
        if (NobleActive()) {
            const int set = (s_localSet >= 0 && s_localSet < kArmorySets) ? s_localSet : 0;
            for (int k = 0; k < 6; ++k)
                rec[kRecordPieceBase + k] = s_localPicks[k] != 0xFF ? s_localPicks[k] : SetPick(set, k);
            rec[kRecordPieceBase + 6] = kElitePiecesMarker;
            rec[kRecordEliteSet] = static_cast<uint8_t>(set);
        }
    } else {
        rec[0x03] = s_localSpartanVisorItemIndex;
    }
}

void __fastcall BuildRecordHook(const uint8_t* userSettings, uint8_t* outRecord) {
    if (s_buildRecordOriginal) s_buildRecordOriginal(userSettings, outRecord);
    if (!userSettings || !outRecord) return;
    const uintptr_t uAddr = reinterpret_cast<uintptr_t>(userSettings);
    // 0x2C7A18 is only called for local controller settings slots (0x2A03C58 + i*0xAB8) and the local
    // join-request override block (0x2A06910 used by 0x31EC34). Remote players arrive via 0x3A296C.
    const bool isLocalSettings = (userSettings == s_localUserSettingsPtr.load()) ||
                                 (s_base != 0 && uAddr >= s_base + 0x2A03C50u && uAddr < s_base + 0x2A07500u);
    if (!isLocalSettings) return;
    std::lock_guard<std::mutex> lock(s_picksMutex);
    RefreshLocalPicks();
    const bool elite = (outRecord[4] == 1) || s_localSpeciesElite;
    StampLocalAppearanceRecordLocked(outRecord, elite);
}

// Second chance for the settings sync: 0x2C837C(player, block) is the call the engine makes when it
// pushes a player's appearance block to the profile manager. ApplyProfileHook runs while the map is
// still loading (before the pack is in), so its push is usually too early; this hook catches the sync
// afterwards. The settings index recorded by ApplyProfileHook tells the local player's block apart from
// the other players' blocks, so a remote player's appearance is never overwritten with local picks.
void __fastcall RecordSetHook(uint8_t* player, uint8_t* block) {
    if (!player || !block) {
        if (s_recordSetOriginal) s_recordSetOriginal(player, block);
        return;
    }
    int32_t index = -1;
    std::memcpy(&index, player + 0x10, sizeof(index));
    const int storedLocal = s_localSettingsIndex.load();
    bool isLocal = false;
    if (storedLocal >= 0) {
        isLocal = (index == storedLocal);
    } else {
        isLocal = (player == s_localUserSettingsPtr.load());
    }
    if (!isLocal && storedLocal < 0 && index == 0 && (wcsstr(GetCommandLineW(), L"-connect") == nullptr)) {
        isLocal = true;
    }

    if (isLocal) {
        std::lock_guard<std::mutex> lock(s_picksMutex);
        RefreshLocalPicks();
        if (!s_localSpeciesElite) {
            *reinterpret_cast<uint32_t*>(player + 0x98) = s_localSpartanVisorItemIndex;
            player[0xD8 + 3] = s_localSpartanVisorItemIndex;
        }
        if (s_localSpeciesElite && NobleActive()) {
            const int set = (s_localSet >= 0 && s_localSet < kArmorySets)
                                ? s_localSet : 0;
            uint8_t picks[6];
            for (int k = 0; k < 6; ++k)
                picks[k] = s_localPicks[k] != 0xFF ? s_localPicks[k] : SetPick(set, k);
            std::memcpy(block, picks, 6);
            block[6] = kElitePiecesMarker;
            block[7] = static_cast<uint8_t>(set);
        }
        block[8] = s_localSpartanEffect;
        block[9] = s_localEliteEffect;

        const int flameVar = s_localFlameVariantIndex.load();
        block[23] = (flameVar >= 0 && flameVar < 30) ? static_cast<uint8_t>(0x80 | (flameVar & 0x1F)) : 0;
        block[10] = s_localColors[0];
        block[11] = s_localColors[1];
        block[12] = s_localColors[2];
        block[13] = s_localColors[3];
        block[14] = static_cast<uint8_t>(0xA0 | (s_localCustomFlags & 0x0F));
        block[15] = s_localSpartanVisorItemIndex;
        for (int c = 0; c < 4; ++c) {
            block[16 + c * 3 + 0] = static_cast<uint8_t>(std::clamp(std::lround(s_localCustomRgb[c][0] * 255.0f), 0L, 255L));
            block[16 + c * 3 + 1] = static_cast<uint8_t>(std::clamp(std::lround(s_localCustomRgb[c][1] * 255.0f), 0L, 255L));
            block[16 + c * 3 + 2] = static_cast<uint8_t>(std::clamp(std::lround(s_localCustomRgb[c][2] * 255.0f), 0L, 255L));
        }
        block[22] = static_cast<uint8_t>(std::clamp(s_localLightsBrightnessIdx, 0, 130));
        const float* lgtSrgb = (s_localCustomFlags & (1 << 3))
            ? s_localCustomRgb[3]
            : kArmoryReachPaletteSrgb[std::clamp<int>(s_localColors[3], 0, 29)];
        block[25] = static_cast<uint8_t>(std::clamp(std::lround(lgtSrgb[0] * 255.0f), 0L, 255L));
        block[26] = static_cast<uint8_t>(std::clamp(std::lround(lgtSrgb[1] * 255.0f), 0L, 255L));
        block[27] = static_cast<uint8_t>(std::clamp(std::lround(lgtSrgb[2] * 255.0f), 0L, 255L));
        block[28] = static_cast<uint8_t>(std::clamp(std::lround(s_localSpartanVisorRgb[0] * 255.0f), 0L, 255L));
        block[29] = static_cast<uint8_t>(std::clamp(std::lround(s_localSpartanVisorRgb[1] * 255.0f), 0L, 255L));
        block[30] = static_cast<uint8_t>(std::clamp(std::lround(s_localSpartanVisorRgb[2] * 255.0f), 0L, 255L));
        block[31] = s_localSpartanVisorCustom ? 0xFE : 0xFD;
        static std::atomic<int> s_synced{0};
        if (s_synced.fetch_add(1) < 6)
            NOBLE_INFO("ElitePack: synced appearance block (player index {}, elite {}, visor {}, lightsB x{:.1f}):{}",
                       index, s_localSpeciesElite, s_localSpartanVisorItemIndex,
                       0.5f + static_cast<float>(s_localLightsBrightnessIdx) * 0.5f, Hex(block, 0x20));
    }
    if (s_recordSetOriginal) s_recordSetOriginal(player, block);
}

// 0x2EDA08 (profile -> player copy) is called only for the local player and knows its settings index.
// The index is what tells the local player's appearance block apart from the other players' blocks in
// RecordSetHook; the block itself is rewritten there, after the pack has loaded.
void __fastcall ApplyProfileHook(int settingsIndex, uint8_t* player) {
    if (player) s_localUserSettingsPtr.store(player);
    if (s_applyProfileOriginal) s_applyProfileOriginal(settingsIndex, player);
    if (settingsIndex >= 0 && settingsIndex < 16) s_localSettingsIndex.store(settingsIndex);
    if (!player) return;
    std::lock_guard<std::mutex> lock(s_picksMutex);
    RefreshLocalPicks();
    const bool lightsCust = (s_localCustomFlags & (1 << 3)) != 0;
    player[0xB1] = static_cast<uint8_t>((lightsCust ? 0x60 : 0x40) | (s_localColors[3] & 0x1F));
    const int flameVar = s_localFlameVariantIndex.load();
    const uint8_t flameVal = (flameVar >= 0 && flameVar < 30) ? static_cast<uint8_t>(0x80 | (flameVar & 0x1F)) : 0;
    player[0xB0] = flameVal;
    player[0xD8 + 0x1A] = flameVal;
    if (lightsCust) {
        const int r6 = std::clamp(static_cast<int>(std::lround(s_localCustomRgb[3][0] * 5.0f)), 0, 5);
        const int g6 = std::clamp(static_cast<int>(std::lround(s_localCustomRgb[3][1] * 5.0f)), 0, 5);
        const int b6 = std::clamp(static_cast<int>(std::lround(s_localCustomRgb[3][2] * 5.0f)), 0, 5);
        const uint16_t w26 = static_cast<uint16_t>(0xF2 + (r6 * 36 + g6 * 6 + b6));
        std::memcpy(player + 0xCA, &w26, sizeof(w26));
    }
    if (!s_localSpeciesElite) {
        *reinterpret_cast<uint32_t*>(player + 0x98) = s_localSpartanVisorItemIndex;
        player[0xAD] = static_cast<uint8_t>(std::clamp(s_localLightsBrightnessIdx, 0, 127));
        player[0xAF] = static_cast<uint8_t>(0x40 | (s_localSpartanVisorItemIndex & 0x1F));
    } else {
        player[0xAE] = static_cast<uint8_t>(std::clamp(s_localLightsBrightnessIdx, 0, 127));
    }
}

// Inspects the player datum associated with an appearance record to verify if it belongs
// to the local machine's player. Separate function with no C++ objects with destructors
// so MSVC allows __try / __except under /EHsc.
static bool IsLocalPlayerRecord(const uint8_t* record, uintptr_t caller) {
    if (!record) return false;
    if (caller != kBuilderReturnLive && caller != kBuilderReturnSpawn) return false;
    const uint8_t* p = record - kPlayerRecordOffset;
    return IsLocalPlayerDatum(p);
}

uint64_t __fastcall BuilderHook(uint8_t* record, uint32_t mode, void* unit, void* player) {
    const uintptr_t caller = reinterpret_cast<uintptr_t>(_ReturnAddress()) - s_base;
    const bool localRecord = IsLocalPlayerRecord(record, caller);
    // Mirrors the builder: mode 0 forces Spartan, mode 1 forces Elite, 2 returns, otherwise record[4].
    const bool elite = record && mode != 2 && (mode == 1 || (mode != 0 && record[4] == 1));
    if (record && localRecord) {
        std::lock_guard<std::mutex> lock(s_picksMutex);
        RefreshLocalPicks();
        StampLocalAppearanceRecordLocked(record, elite);
        if (player && !elite) {
            *reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(player) + 0x98) = s_localSpartanVisorItemIndex;
        }
    }
    static std::atomic<int> s_recordsLogged{0};
    if (record && s_recordsLogged.fetch_add(1) < 12)
        NOBLE_INFO("ElitePack: appearance record (caller 0x{:X}, mode {}, elite {}, local {}):{}",
                   caller, mode, elite, localRecord, Hex(record, 0x28));
    if (!elite || !s_loaded || !s_haveActive || !NobleActive()) {
        if (record) {
            alignas(8) uint8_t spCopy[0x28];
            std::memcpy(spCopy, record, sizeof(spCopy));
            if (!elite) {
                spCopy[0x17] = 0;
                spCopy[0x19] = 0;
                if (localRecord && s_localSpartanEffect >= 1 && s_localSpartanEffect <= 6) {
                    spCopy[0x18] = s_localSpartanEffect;
                }
            } else {
                spCopy[0x18] = 0;
                if (localRecord && s_localEliteEffect >= 1 && s_localEliteEffect <= 6) {
                    spCopy[kRecordEliteFx] = s_localEliteEffect;
                }
            }
            return s_builder(spCopy, mode, unit, player);
        }
        return s_builder(record, mode, unit, player);
    }

    alignas(8) uint8_t copy[0x28];
    std::memcpy(copy, record, sizeof(copy));
    copy[0] &= ~2; // Clear bit 1 to ensure dil is 1 and armor effects are never suppressed on Elites

    uint8_t picks[6] = {0, 0, 0, 0, 0, 0};
    uint8_t eliteEffect = 0xFF;
    int effectiveSet = (record[kRecordEliteSet] < kArmorySets)
                           ? record[kRecordEliteSet] : 0;

    if (localRecord) {
        std::lock_guard<std::mutex> lock(s_picksMutex);
        RefreshLocalPicks();
        if (s_localSet >= 0 && s_localSet < kArmorySets) effectiveSet = s_localSet;
        for (int k = 0; k < 6; ++k)
            picks[k] = s_localPicks[k] != 0xFF ? s_localPicks[k] : SetPick(effectiveSet, k);
        eliteEffect = s_localEliteEffect;
        if ((eliteEffect < 1 || eliteEffect > 6) && record[kRecordEliteFx] >= 1 && record[kRecordEliteFx] <= 6)
            eliteEffect = record[kRecordEliteFx];
        StampLocalAppearanceRecordLocked(record, true);
    } else if (record[kRecordPieceBase + 6] == kElitePiecesMarker) {
        // The record came from its owner (another machine or a unit copy): use their pieces.
        std::memcpy(picks, record + kRecordPieceBase, 6);
        if (record[kRecordEliteSet] < kArmorySets) {
            effectiveSet = record[kRecordEliteSet];
        }
        eliteEffect = record[kRecordEliteFx];
    } else {
        // Old/unmodified record (no marker): fall back to the Elite armor set the engine replicated.
        for (int k = 0; k < 6; ++k) picks[k] = SetPick(effectiveSet, k);
        eliteEffect = record[kRecordEliteFx];
    }
    if (eliteEffect < 1 || eliteEffect > 6) eliteEffect = 0xFF;

    std::memcpy(copy + kRecordPieceBase, picks, 6);
    copy[kRecordPieceBase + 6] = kElitePiecesMarker;
    copy[kRecordEliteSet] = static_cast<uint8_t>(effectiveSet);
    copy[0x18] = 0;
    if (eliteEffect != 0xFF) copy[kRecordEliteFx] = eliteEffect;
    if (localRecord) {
        // first-person arms follow the forearms, the first-person body the shoulders (else the Armor Set)
        // ONLY update first-person meshes for the local machine's player!
        const int fpSet = effectiveSet < 8 ? effectiveSet : 0;
        const int arms = SetOfPiece(4, picks[4]), body = SetOfPiece(1, picks[1]);
        ApplyFirstPersonSet(0, arms >= 0 ? arms : fpSet);
        ApplyFirstPersonSet(1, body >= 0 ? body : fpSet);
    }
    if (s_picksLogged.fetch_add(1) < 12)
        NOBLE_INFO("ElitePack: Elite dressed with pieces {} {} {} {} {} {} (mode {}, set {}, fx {}, local {})",
                   picks[0], picks[1], picks[2], picks[3], picks[4], picks[5], mode, copy[kRecordEliteSet],
                   eliteEffect, localRecord);
    return s_builder(copy, mode, unit, player);
}

bool InstallPicks(const std::filesystem::path& dataDir) {
    s_appearancePath = dataDir / "appearance.dat";
    void* original = nullptr;
    for (uintptr_t site : kBuilderCalls) {
        void* t = RetargetCall(site, kAppearanceBuilder, reinterpret_cast<void*>(&BuilderHook));
        if (!t) { NOBLE_WARN("ElitePack: appearance call at 0x{:X} not redirected", site); continue; }
        original = t;
    }
    s_builder = reinterpret_cast<BuilderFn>(original);
    // The profile -> player copy (local player only, records the settings index).
    if (!HookManager::InstallHook(At<void*>(kApplyProfile), reinterpret_cast<void*>(&ApplyProfileHook),
                                  reinterpret_cast<void**>(&s_applyProfileOriginal), "elitepack_apply_profile"))
        NOBLE_WARN("ElitePack: profile hook (0x{:X}) not installed; other players keep the set only", kApplyProfile);
    if (!HookManager::InstallHook(At<void*>(kBuildRecord), reinterpret_cast<void*>(&BuildRecordHook),
                                  reinterpret_cast<void**>(&s_buildRecordOriginal), "elitepack_build_record"))
        NOBLE_WARN("ElitePack: build-record hook (0x{:X}) not installed", kBuildRecord);
    if (!HookManager::InstallHook(At<void*>(kRecordSet), reinterpret_cast<void*>(&RecordSetHook),
                                  reinterpret_cast<void**>(&s_recordSetOriginal), "elitepack_settings_sync"))
        NOBLE_WARN("ElitePack: settings-sync hook (0x{:X}) not installed; other players keep the set only", kRecordSet);
    return s_builder != nullptr;
}

bool HasArg(const wchar_t* name) {
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    bool found = false;
    for (int i = 1; argv && i < argc; ++i) if (_wcsicmp(argv[i], name) == 0) found = true;
    if (argv) LocalFree(argv);
    return found;
}

} // namespace

std::optional<std::filesystem::path> ResolveFile(const std::wstring& requested) {
    if (s_packPath.empty()) return std::nullopt;
    std::wstring want = std::filesystem::path(requested).filename().wstring();
    std::wstring have = s_packPath.filename().wstring();
    std::transform(want.begin(), want.end(), want.begin(), ::towlower);
    std::transform(have.begin(), have.end(), have.begin(), ::towlower);
    if (want == have) return s_packPath;
    return std::nullopt;
}

bool Loaded() { return s_loaded; }

void SetLocalFlameVariant(int variantIndex) {
    s_localFlameVariantIndex.store(variantIndex);
}

int GetLocalFlameVariant() {
    return s_localFlameVariantIndex.load();
}

static std::filesystem::path FindRelocSidecar(const std::filesystem::path& mapPath) {
    std::error_code ec;
    auto s1 = mapPath;
    s1 += ".noblereloc";
    if (std::filesystem::is_regular_file(s1, ec)) return s1;
    auto s2 = mapPath;
    s2 += ".rrreloc";
    if (std::filesystem::is_regular_file(s2, ec)) return s2;
    return {};
}

std::filesystem::path FindPackMap(const std::filesystem::path& dataDir) {
    std::error_code ec;
    const auto folder = dataDir / "armor";
    std::filesystem::path first;
    std::filesystem::path named;
    for (const auto& e : std::filesystem::directory_iterator(folder, ec)) {
        if (!e.is_regular_file(ec) || e.path().extension() != ".map") continue;
        if (FindRelocSidecar(e.path()).empty()) continue;
        std::string name = e.path().filename().string();
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (name == "noble_elite_pack.map" || name == "rr_elite_pack.map") return e.path();
        if (name == "elite_armor.map" && named.empty()) named = e.path();
        else if (named.empty() && (name.find("elite_pack") != std::string::npos || name.find("armory") != std::string::npos)) named = e.path();
        if (first.empty()) first = e.path();
    }
    if (!named.empty()) return named;
    return first;
}


void Install(HMODULE engine, const std::filesystem::path& dataDir, const std::string& mode) {
    if (!engine || s_base) return;
    std::string lowerMode = mode;
    std::transform(lowerMode.begin(), lowerMode.end(), lowerMode.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (lowerMode == "campaign") {
        s_base = reinterpret_cast<uintptr_t>(engine);
        InstallPicks(dataDir);
        return;
    }
    if (lowerMode != "multiplayer" && lowerMode != "firefight" && lowerMode != "forge" &&
        lowerMode != "mainmenu" && lowerMode != "menu") return;
    if (HasArg(L"-no-elite-pack")) {
        NOBLE_INFO("ElitePack: off (-no-elite-pack)");
        return;
    }
    const bool menuMode = (lowerMode == "mainmenu" || lowerMode == "menu");
    s_keepPackHealth = HasArg(L"-elite-pack-own-health");
    s_keepPackGlobals = HasArg(L"-elite-pack-keep-globals");
    s_keepDeathEffect = HasArg(L"-elite-pack-death-fx");
    s_fpOverride = !HasArg(L"-no-fp-override");
    s_packPath = FindPackMap(dataDir);
    if (s_packPath.empty()) {
        s_base = reinterpret_cast<uintptr_t>(engine);
        if (!menuMode) InstallPicks(dataDir);
        NOBLE_INFO("ElitePack: no pack in {} (players use the map's Elite; appearance sync active)", (dataDir / "armor").string());
        return;
    }
    auto sidecar = FindRelocSidecar(s_packPath);
    if (sidecar.empty() || !ReadReloc(sidecar, s_reloc)) {
        s_packPath.clear();
        s_base = reinterpret_cast<uintptr_t>(engine);
        if (!menuMode) InstallPicks(dataDir);
        return;
    }
    s_packStem = s_packPath.stem().string();
    s_base = reinterpret_cast<uintptr_t>(engine);

    static const uint8_t kMovEdx1[2] = {0x8B, 0xD2}, kMovEdx2[2] = {0x89, 0xD2}, kNop2[2] = {0x66, 0x90};
    const bool patched = PatchCode(kDeltaTruncation, kMovEdx1, kNop2, 2) || PatchCode(kDeltaTruncation, kMovEdx2, kNop2, 2);
    // Elite armour slots (the engine --elite-armor-slots): the appearance builder 0x1055EC applies only the Elite
    // set list (+0x74) to Elites ("cmp esi,7; sete r15b" at 0x105796). With the pack, Elites take their
    // pieces from the Spartan piece lists (which now carry the pack's Elite changes), so lists 0..7 apply
    // ("cmp esi,8; setb r15b"). Not 8+ (+0x80 Spartan armour effects, +0x8C Elite armour effects, +0x98): with
    // every list on (the engine's "mov r15b,1"), Elites played Heart Attack and Birthday Party at once from leftover bytes.
    static const uint8_t kSlotCheck[7] = {0x83, 0xFE, 0x07, 0x41, 0x0F, 0x94, 0xC7};
    static const uint8_t kSlotOld[7]   = {0x83, 0xFE, 0x08, 0x41, 0x0F, 0x92, 0xC7};
    static const uint8_t kSlotAll[7]   = {0x83, 0xFE, 0x08, 0x41, 0x0F, 0x95, 0xC7};
    const bool slots = PatchCode(0x105796, kSlotCheck, kSlotAll, 7) ||
                       PatchCode(0x105796, kSlotOld,   kSlotAll, 7);
    static const uint8_t kStepPrologue[7] = {0x48, 0x8B, 0xC4, 0x4C, 0x89, 0x48, 0x20};
    const bool stepOk = std::memcmp(At<void*>(kCacheStep), kStepPrologue, 7) == 0;
    if (!patched || !stepOk || !slots) {
        NOBLE_WARN("ElitePack: haloreach.dll doesn't look as expected (patch {}, step {}, slots {}); pack not used",
                   patched, stepOk, slots);
        s_packPath.clear();
        if (!menuMode) InstallPicks(dataDir);
        return;
    }
    // Order matters: the map-load detour only adds the pack once every other hook is in place (s_ready).
    const bool adapters = InstallAdapters();
    const bool stepHooked = adapters && HookManager::InstallHook(At<void*>(kCacheStep), reinterpret_cast<void*>(&CacheStepHook),
                                                     reinterpret_cast<void**>(&s_cacheStepOriginal), "elitepack_cache_step");
    static const uint8_t kRebuildPrologue[7] = {0x48, 0x8B, 0xC4, 0x48, 0x89, 0x58, 0x10};
    if (stepHooked && std::memcmp(At<void*>(kZoneRebuild), kRebuildPrologue, 7) == 0)
        HookManager::InstallHook(At<void*>(kZoneRebuild), reinterpret_cast<void*>(&RebuildHook),
                                 reinterpret_cast<void**>(&s_rebuildOriginal), "elitepack_zone_rebuild");
    if (stepHooked && !s_rebuildOriginal) NOBLE_WARN("ElitePack: zone-set rebuild hook not installed");
    if (stepHooked) s_extraLoad = reinterpret_cast<ExtraLoadFn>(RetargetCall(kExtraCallSite, kExtraLoad, reinterpret_cast<void*>(&ExtraLoadHook)));
    if (s_extraLoad) s_mapLoad = reinterpret_cast<MapLoadFn>(RetargetCall(kLoadCallSite, kMapLoad, reinterpret_cast<void*>(&MapLoadHook)));
    if (!stepHooked || !s_extraLoad || !s_mapLoad) {
        NOBLE_ERROR("ElitePack: hooks failed (adapters {}, step {}, extra {}, load {}); the pack is not used",
                    adapters, stepHooked, s_extraLoad != nullptr, s_mapLoad != nullptr);
        s_packPath.clear();
        if (!menuMode) InstallPicks(dataDir);
        return;
    }
    // The menu/Armory uses ReachArmory's GLOBALS pairs; the appearance-builder hooks are for matches
    // only (they apply the local profile's indices and would fight the Armory's own selection).
    if (!menuMode && !InstallPicks(dataDir)) NOBLE_WARN("ElitePack: Armory Elite pieces won't reach the game (calls not redirected)");
    s_ready = true;
    NOBLE_INFO("ElitePack: {} ({} packed pointers to relocate) will load with {} matches",
               s_packPath.string(), s_reloc.offsets.size(), mode);
}

} // namespace ElitePack
} // namespace noble
