#include "ReachArmory.hpp"
#include "ui/UIManager.hpp"
#include "ui/ArmorCustomizer.hpp"
#include "common/Logger.hpp"

#include <windows.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <cmath>
#include <cstring>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace noble {

namespace {

// -----------------------------------------------------------------------------
// Low-level SEH memory helpers (no C++ objects with destructors in SEH frames)
// -----------------------------------------------------------------------------
static bool SafeReadU32(uintptr_t addr, uint32_t* out) {
    if (addr <= 0x10000 || !out) return false;
    __try {
        *out = *reinterpret_cast<const uint32_t*>(addr);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static bool SafeReadI32(uintptr_t addr, int32_t* out) {
    if (addr <= 0x10000 || !out) return false;
    __try {
        *out = *reinterpret_cast<const int32_t*>(addr);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static bool SafeReadPtr(uintptr_t addr, uintptr_t* out) {
    if (addr <= 0x10000 || !out) return false;
    __try {
        *out = *reinterpret_cast<const uintptr_t*>(addr);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static bool SafeWriteU32(uintptr_t addr, uint32_t val) {
    if (addr <= 0x10000) return false;
    __try {
        *reinterpret_cast<uint32_t*>(addr) = val;
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static bool SafeWriteU8(uintptr_t addr, uint8_t val) {
    if (addr <= 0x10000) return false;
    __try {
        *reinterpret_cast<uint8_t*>(addr) = val;
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static bool SafeCopyMem(void* dst, const void* src, size_t len) {
    if (reinterpret_cast<uintptr_t>(dst) <= 0x10000 || reinterpret_cast<uintptr_t>(src) <= 0x10000 || len == 0) {
        return false;
    }
    __try {
        std::memcpy(dst, src, len);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static bool SafeWriteFloats(uintptr_t addr, const float* vals, size_t count) {
    if (addr <= 0x10000 || !vals || count == 0) return false;
    __try {
        std::memcpy(reinterpret_cast<void*>(addr), vals, count * sizeof(float));
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static std::string ToLowerAscii(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return s;
}

// -----------------------------------------------------------------------------
// State
// -----------------------------------------------------------------------------
static std::filesystem::path s_dataDir;
static std::atomic<bool> s_menuMode{false};
static std::atomic<bool> s_sceneReady{false};
static std::atomic<bool> s_scanning{false};
static std::atomic<bool> s_scanLaunched{false};
static std::atomic<uint64_t> s_frameCounter{0};
static std::atomic<int32_t> s_lastScene{-1};
static std::atomic<float> s_yawOffset{0.0f};
static std::atomic<uint32_t> s_rotationBaseBits{0xFFFFFFFFu};
static std::atomic<uint32_t> s_lastYawBits{0};

static std::mutex s_sceneMutex;
static std::string s_targetScene = "armory";

static std::mutex s_dataMutex;
static bool s_stringIdsLoaded = false;
static std::unordered_map<std::string, uint32_t> s_stringIds;

// Spartan mulg table address and stride (in bytes, typically 12 = 3 uint32_t's)
static std::atomic<uintptr_t> s_spartanTableBase{0};
static std::atomic<size_t> s_spartanStride{12};
static std::atomic<uintptr_t> s_bodyTypeAddr{0};
static std::atomic<uintptr_t> s_effectBaseAddr{0};
static uint32_t s_effectTags[7] = { 0 }; // cached +4 tag handle for each effect 0..6

// Elite hlmt 6-region table
struct ElitePermEntry {
    uint8_t bytes[0x18];
};
struct EliteRegionCache {
    uintptr_t permArrayAddr = 0;
    std::unordered_map<uint32_t, ElitePermEntry> permsByStringId;
};
static bool s_eliteScanned = false;
static std::array<EliteRegionCache, 6> s_eliteRegions;

// Color records & Suit shader constants
static std::vector<uintptr_t> s_colorRecords;
static std::vector<uintptr_t> s_suitRecords;
static std::array<float, 9> s_currentLinearColors = {
    0.15294118f, 0.15294118f, 0.15294118f, // Primary
    0.15294118f, 0.15294118f, 0.15294118f, // Secondary
    1.0f, 0.45f, 0.05f                      // Visor
};

// -----------------------------------------------------------------------------
// Load string_ids from data/menumaps/mainmenu_all.string_ids.csv
// -----------------------------------------------------------------------------
static void EnsureStringIdsLoaded() {
    std::lock_guard<std::mutex> lock(s_dataMutex);
    if (s_stringIdsLoaded) return;
    s_stringIdsLoaded = true;

    std::vector<std::filesystem::path> candidates;
    if (!s_dataDir.empty()) {
        candidates.push_back(s_dataDir / "menumaps" / "mainmenu_all.string_ids.csv");
        candidates.push_back(s_dataDir / "mainmenu_all.string_ids.csv");
    }
    wchar_t exeBuf[MAX_PATH] = {};
    if (GetModuleFileNameW(nullptr, exeBuf, MAX_PATH)) {
        std::filesystem::path exeDir = std::filesystem::path(exeBuf).parent_path();
        candidates.push_back(exeDir / "data" / "menumaps" / "mainmenu_all.string_ids.csv");
        candidates.push_back(exeDir / ".." / "data" / "menumaps" / "mainmenu_all.string_ids.csv");
        candidates.push_back(exeDir / ".." / ".." / "data" / "menumaps" / "mainmenu_all.string_ids.csv");
    }

    for (const auto& path : candidates) {
        std::error_code ec;
        if (!std::filesystem::exists(path, ec)) continue;
        std::ifstream in(path);
        if (!in) continue;

        std::string line;
        size_t count = 0;
        while (std::getline(in, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            size_t comma = line.find(',');
            if (comma == std::string::npos) continue;
            std::string hexPart = line.substr(0, comma);
            std::string namePart = line.substr(comma + 1);
            if (hexPart.size() > 2 && (hexPart[0] == '0') && (hexPart[1] == 'x' || hexPart[1] == 'X')) {
                uint32_t id = static_cast<uint32_t>(std::strtoul(hexPart.c_str() + 2, nullptr, 16));
                if (id != 0 && !namePart.empty()) {
                    s_stringIds.emplace(ToLowerAscii(namePart), id);
                    ++count;
                }
            }
        }
        NOBLE_INFO("ReachArmory: Loaded {} string_ids from '{}'", count, path.string());
        return;
    }
    NOBLE_WARN("ReachArmory: Could not locate mainmenu_all.string_ids.csv!");
}

static uint32_t LookupStringId(const std::string& name) {
    std::lock_guard<std::mutex> lock(s_dataMutex);
    auto it = s_stringIds.find(ToLowerAscii(name));
    return (it != s_stringIds.end()) ? it->second : 0;
}

// -----------------------------------------------------------------------------
// Scene Switching (ported from host::armory::switch_scene @ 0x1401b3fb0)
// -----------------------------------------------------------------------------
static int32_t ResolveSceneIndex(const std::string& scene) {
    const std::string s = ToLowerAscii(scene);
    if (s == "campaign")        return 0x00;
    if (s == "matchmaking")     return 0x01;
    if (s == "custom_games")    return 0x02;
    if (s == "forge")           return 0x03;
    if (s == "theater")         return 0x04;
    if (s == "firefight")       return 0x05;
    if (s == "armory")          return 0x0A;
    if (s == "armory_elite")    return 0x0B;
    if (s == "armory_full")     return 0x0C;
    if (s == "armory_head")     return 0x0D;
    if (s == "armory_chest")    return 0x0F;
    if (s == "armory_arm")      return 0x10;
    if (s == "armory_leg")      return 0x11;
    if (s == "armory_knee")     return 0x12;
    if (s == "armory_shoulder") return 0x13;
    if (s == "armory_colors")   return 0x14;
    return 0x0A;
}

static void SwitchSceneOnRenderThread(int32_t targetIdx) {
    HMODULE hReach = GetModuleHandleW(L"haloreach.dll");
    if (!hReach) return;
    const uintptr_t reachBase = reinterpret_cast<uintptr_t>(hReach);

    uint32_t tlsIdx = 0;
    if (!SafeReadU32(reachBase + 0xC19CE8, &tlsIdx) || tlsIdx > 0x43F) return;

    const uintptr_t tlsBase = reinterpret_cast<uintptr_t>(TlsGetValue(tlsIdx));
    if (tlsBase <= 0x10000) return;

    uintptr_t directorPtr = 0;
    if (!SafeReadPtr(tlsBase + 0x22D0, &directorPtr) || directorPtr <= 0x10000) return;

    uintptr_t sceneStateAddr = 0;
    if (!SafeReadPtr(directorPtr + 0x5C, &sceneStateAddr) || sceneStateAddr <= 0x10000) return;

    // Validate the 21-entry scene table at sceneStateAddr + 0x60 (stride 0x68)
    for (uint32_t i = 0; i < 21; ++i) {
        uint8_t entry[0x68];
        if (!SafeCopyMem(entry, reinterpret_cast<const void*>(sceneStateAddr + 0x60 + i * 0x68), sizeof(entry))) {
            return;
        }
        const uint32_t idx  = *reinterpret_cast<const uint32_t*>(entry + 0x00);
        const uint32_t tag1 = *reinterpret_cast<const uint32_t*>(entry + 0x0C);
        const uint32_t tag2 = *reinterpret_cast<const uint32_t*>(entry + 0x18);
        if (idx != i || tag1 == 0xFFFFFFFFu || tag2 == 0xFFFFFFFFu) {
            return;
        }
    }

    if (!s_sceneReady.exchange(true)) {
        NOBLE_INFO("ReachArmory: Director scene table ready at 0x{:X} (tlsBase=0x{:X})", sceneStateAddr, tlsBase);
    }

    int32_t currentScene = -1;
    SafeReadI32(sceneStateAddr, &currentScene);

    const int32_t prevScene = s_lastScene.load();
    if (prevScene != targetIdx || currentScene != targetIdx) {
        const uintptr_t srcEntry = sceneStateAddr + 0x60 + static_cast<uintptr_t>(targetIdx) * 0x68;
        if (SafeCopyMem(reinterpret_cast<void*>(sceneStateAddr + 0x04),
                        reinterpret_cast<const void*>(srcEntry + 0x0C), 0x5C)) {
            SafeWriteU32(sceneStateAddr, static_cast<uint32_t>(targetIdx));
            if (prevScene != targetIdx) {
                s_lastScene.store(targetIdx);
                NOBLE_INFO("ReachArmory: Switched menu camera scene to index 0x{:X}", targetIdx);
            }
        }
    }

    // Toggle Spartan (+0x2084) vs Elite (+0x2085) biped visibility on the Armory pedestal
    const bool isSpartanScene = (targetIdx == 0x0A) ||
                                (targetIdx >= 0x0C && targetIdx <= 0x14 && targetIdx != 0x0E);
    const bool isEliteScene   = (targetIdx == 0x0B);

    if (isSpartanScene) {
        SafeWriteU8(tlsBase + 0x2084, 1);
        SafeWriteU8(tlsBase + 0x2085, 0);
    } else if (isEliteScene) {
        SafeWriteU8(tlsBase + 0x2084, 0);
        SafeWriteU8(tlsBase + 0x2085, 1);
    } else {
        SafeWriteU8(tlsBase + 0x2084, 0);
        SafeWriteU8(tlsBase + 0x2085, 0);
    }
}

// -----------------------------------------------------------------------------
// Memory Scanners (ported from host::armory::scan, find_all, scan_suit, scan_elite)
// -----------------------------------------------------------------------------
static void ScanSpartanTable() {
    if (s_spartanTableBase.load() != 0) return;

    const uint32_t headId    = LookupStringId("head");
    const uint32_t defaultId = LookupStringId("default");
    if (headId == 0 || defaultId == 0) return;

    static constexpr size_t kStrides[6] = { 4, 8, 12, 16, 24, 32 };

    uintptr_t addr = 0;
    MEMORY_BASIC_INFORMATION mbi{};
    while (VirtualQuery(reinterpret_cast<LPCVOID>(addr), &mbi, sizeof(mbi)) != 0) {
        const uintptr_t base = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
        const size_t size = mbi.RegionSize;
        const uintptr_t nextAddr = base + size;

        if (mbi.State == MEM_COMMIT && mbi.Type == MEM_PRIVATE && mbi.Protect == PAGE_READWRITE &&
            size >= 0x100 && size <= 0x20000000) {
            std::vector<uint8_t> buf(size);
            if (SafeCopyMem(buf.data(), reinterpret_cast<const void*>(base), size)) {
                for (size_t off = 0; off + 0xC0 <= size; off += 4) {
                    const uint32_t v0 = *reinterpret_cast<const uint32_t*>(buf.data() + off);
                    if (v0 != 0x03BB6FEDu) continue;

                    for (size_t stride : kStrides) {
                        if (off + stride * 6 + 4 > size) continue;
                        const uint32_t v1 = *reinterpret_cast<const uint32_t*>(buf.data() + off + stride);
                        const uint32_t v4 = *reinterpret_cast<const uint32_t*>(buf.data() + off + stride * 4);
                        const uint32_t v5 = *reinterpret_cast<const uint32_t*>(buf.data() + off + stride * 5);
                        if (v1 == 0x03BAB653u && v4 == headId && v5 == defaultId) {
                            const uintptr_t foundBase = base + off;
                            s_spartanStride.store(stride);
                            s_spartanTableBase.store(foundBase);
                            NOBLE_INFO("ReachArmory: Found Spartan mulg armor table at 0x{:X} (stride={})", foundBase, stride);
                            return;
                        }
                    }
                }
            }
        }
        if (nextAddr <= addr) break;
        addr = nextAddr;
    }
}

static void ScanColorRecords() {
    // 16-byte pattern: [0.15294118f, 0.15294118f, 0.15294118f, 0.0f]
    static constexpr uint32_t kPattern[4] = { 0x3E1C9C9Du, 0x3E1C9C9Du, 0x3E1C9C9Du, 0x00000000u };

    std::vector<uintptr_t> found;
    uintptr_t addr = 0;
    MEMORY_BASIC_INFORMATION mbi{};
    while (VirtualQuery(reinterpret_cast<LPCVOID>(addr), &mbi, sizeof(mbi)) != 0) {
        const uintptr_t base = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
        const size_t size = mbi.RegionSize;
        const uintptr_t nextAddr = base + size;

        if (mbi.State == MEM_COMMIT && mbi.Type == MEM_PRIVATE && mbi.Protect == PAGE_READWRITE &&
            size >= 0x60 && size <= 0x20000000) {
            std::vector<uint8_t> buf(size);
            if (SafeCopyMem(buf.data(), reinterpret_cast<const void*>(base), size)) {
                for (size_t off = 0; off + 0x50 <= size; off += 4) {
                    if (std::memcmp(buf.data() + off, kPattern, sizeof(kPattern)) == 0) {
                        found.push_back(base + off);
                    }
                }
            }
        }
        if (nextAddr <= addr) break;
        addr = nextAddr;
    }

    if (!found.empty()) {
        std::lock_guard<std::mutex> lock(s_dataMutex);
        for (uintptr_t rec : found) {
            if (std::find(s_colorRecords.begin(), s_colorRecords.end(), rec) == s_colorRecords.end()) {
                s_colorRecords.push_back(rec);
            }
        }
        NOBLE_INFO("ReachArmory: Tracked {} live armor color record(s)", s_colorRecords.size());
    }
}

static void ScanSuitRecords() {
    // Scan for 9-float suit constant blocks where each float is in [0.005f, 1.0f] and matches current or default steel
    float cur[9];
    {
        std::lock_guard<std::mutex> lock(s_dataMutex);
        std::memcpy(cur, s_currentLinearColors.data(), sizeof(cur));
    }

    std::vector<uintptr_t> found;
    uintptr_t addr = 0;
    MEMORY_BASIC_INFORMATION mbi{};
    while (VirtualQuery(reinterpret_cast<LPCVOID>(addr), &mbi, sizeof(mbi)) != 0) {
        const uintptr_t base = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
        const size_t size = mbi.RegionSize;
        const uintptr_t nextAddr = base + size;

        if (mbi.State == MEM_COMMIT && mbi.Type == MEM_PRIVATE && mbi.Protect == PAGE_READWRITE &&
            size >= 0x40 && size <= 0x20000000) {
            std::vector<uint8_t> buf(size);
            if (SafeCopyMem(buf.data(), reinterpret_cast<const void*>(base), size)) {
                for (size_t off = 0; off + 36 <= size; off += 4) {
                    const float* f = reinterpret_cast<const float*>(buf.data() + off);
                    bool match = true;
                    for (int i = 0; i < 9; ++i) {
                        if (!(f[i] >= 0.005f && f[i] <= 1.0f) || std::fabs(f[i] - cur[i]) >= 0.002f) {
                            match = false;
                            break;
                        }
                    }
                    if (match) {
                        found.push_back(base + off);
                    }
                }
            }
        }
        if (nextAddr <= addr) break;
        addr = nextAddr;
    }

    if (!found.empty()) {
        std::lock_guard<std::mutex> lock(s_dataMutex);
        for (uintptr_t rec : found) {
            if (std::find(s_suitRecords.begin(), s_suitRecords.end(), rec) == s_suitRecords.end()) {
                s_suitRecords.push_back(rec);
            }
        }
    }
}

static void ScanEliteTable() {
    {
        std::lock_guard<std::mutex> lock(s_dataMutex);
        if (s_eliteScanned) return;
    }

    const uint32_t expectedRegions[6] = {
        LookupStringId("head"),
        LookupStringId("chest"),
        LookupStringId("left_shoulder"),
        LookupStringId("right_shoulder"),
        LookupStringId("arms"),
        LookupStringId("legs")
    };
    for (uint32_t id : expectedRegions) {
        if (id == 0) return;
    }
    const uint32_t minorId = LookupStringId("minor");
    const uint32_t specOpsId = LookupStringId("spec_ops");
    if (minorId == 0 || specOpsId == 0) return;

    uintptr_t addr = 0;
    MEMORY_BASIC_INFORMATION mbi{};
    while (VirtualQuery(reinterpret_cast<LPCVOID>(addr), &mbi, sizeof(mbi)) != 0) {
        const uintptr_t base = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
        const size_t size = mbi.RegionSize;
        const uintptr_t nextAddr = base + size;

        if (mbi.State == MEM_COMMIT && mbi.Type == MEM_PRIVATE && mbi.Protect == PAGE_READWRITE &&
            size >= 0x90 && size <= 0x20000000) {
            std::vector<uint8_t> buf(size);
            if (SafeCopyMem(buf.data(), reinterpret_cast<const void*>(base), size)) {
                for (size_t off = 0; off + 0x90 <= size; off += 4) {
                    bool match = true;
                    for (size_t r = 0; r < 6; ++r) {
                        const uint8_t* hdr = buf.data() + off + r * 0x18;
                        const uint32_t regionNameId = *reinterpret_cast<const uint32_t*>(hdr + 0x00);
                        const uint32_t permCount    = *reinterpret_cast<const uint32_t*>(hdr + 0x08);
                        const uintptr_t permPtr     = *reinterpret_cast<const uintptr_t*>(hdr + 0x10);
                        if (regionNameId != expectedRegions[r] || permCount == 0 || permCount > 31 || permPtr <= 0x10000) {
                            match = false;
                            break;
                        }
                    }
                    if (!match) continue;

                    // Verify region 0 contains minor and spec_ops permutations
                    const uint8_t* hdr0 = buf.data() + off;
                    const uint32_t count0 = *reinterpret_cast<const uint32_t*>(hdr0 + 0x08);
                    const uintptr_t ptr0  = *reinterpret_cast<const uintptr_t*>(hdr0 + 0x10);
                    std::vector<ElitePermEntry> entries0(count0);
                    if (!SafeCopyMem(entries0.data(), reinterpret_cast<const void*>(ptr0), count0 * sizeof(ElitePermEntry))) {
                        continue;
                    }
                    bool hasMinor = false, hasSpecOps = false;
                    for (uint32_t p = 0; p < count0; ++p) {
                        const uint32_t pId = *reinterpret_cast<const uint32_t*>(entries0[p].bytes);
                        if (pId == minorId) hasMinor = true;
                        if (pId == specOpsId) hasSpecOps = true;
                    }
                    if (!hasMinor || !hasSpecOps) continue;

                    // Cache all 6 regions
                    std::array<EliteRegionCache, 6> cached{};
                    bool allOk = true;
                    for (size_t r = 0; r < 6; ++r) {
                        const uint8_t* hdr = buf.data() + off + r * 0x18;
                        const uint32_t count = *reinterpret_cast<const uint32_t*>(hdr + 0x08);
                        const uintptr_t ptr  = *reinterpret_cast<const uintptr_t*>(hdr + 0x10);
                        cached[r].permArrayAddr = ptr;
                        std::vector<ElitePermEntry> perms(count);
                        if (!SafeCopyMem(perms.data(), reinterpret_cast<const void*>(ptr), count * sizeof(ElitePermEntry))) {
                            allOk = false;
                            break;
                        }
                        for (uint32_t p = 0; p < count; ++p) {
                            const uint32_t pId = *reinterpret_cast<const uint32_t*>(perms[p].bytes);
                            cached[r].permsByStringId[pId] = perms[p];
                        }
                    }
                    if (allOk) {
                        std::lock_guard<std::mutex> lock(s_dataMutex);
                        s_eliteRegions = std::move(cached);
                        s_eliteScanned = true;
                        NOBLE_INFO("ReachArmory: Found Elite hlmt 6-region table at 0x{:X}", base + off);
                        return;
                    }
                }
            }
        }
        if (nextAddr <= addr) break;
        addr = nextAddr;
    }
}

// -----------------------------------------------------------------------------
// Lookup tables from PlayerProfile indices to mainmenu_all.string_ids.csv names
// -----------------------------------------------------------------------------
static std::string GetHelmetPermName(int helmetIdx, int variantIdx) {
    struct HelmetVariants {
        const char* base;
        const char* v1;
        const char* v2;
    };
    static const HelmetVariants kHelmets[21] = {
        { "default",     "default_ua",        "default_ua_huli" },     // 0: MARK V[B]
        { "cqc",         "cqc_cbrn",          "cqc_ua_huli" },         // 1: CQC
        { "odst",        "odst_ua_cnm",       "odst_cbrn_huli" },      // 2: ODST
        { "hazop",       "hazop_cbrn",        "hazop_cnm_i" },         // 3: HAZOP
        { "eod",         "eod_cnm",           "eod_ua_huli" },         // 4: EOD
        { "operator",    "operator_ua_huli",  "operator_cnm" },        // 5: OPERATOR
        { "grenadier",   "grenadier_ua",      "grenadier_ua_fc" },     // 6: GRENADIER
        { "air_assault", "air_assault_ua_cnm","air_assault_fc_i" },    // 7: AIR ASSAULT
        { "scout",       "scout_huli",        "scout_cbrn_cnm" },      // 8: SCOUT
        { "eva",         "eva_cnm",           "eva_ua_huli3" },        // 9: EVA
        { "jfo",         "jfo_huli",          "jfo_ua" },              // 10: JFO
        { "commando",    "commando_cbrn_cnm", "commando_ua_fc_i2" },   // 11: COMMANDO
        { "emile",       "emile_cnm",         "emile_ua_huli" },       // 12: EVA[C]
        { "mark_v",      "mark_v_cnm",        "mark_v_ua" },           // 13: MJOLNIR MK. V
        { "pilot",       "pilot_huli",        "pilot_ua_huli" },       // 14: PILOT
        { "recon",       "recon_huli",        "recon_ua_huli3" },      // 15: RECON
        { "mark_vi",     "mark_vi_fc_i",      "mark_vi_ua_huli" },     // 16: MJOLNIR MK. VI
        { "cqb",         "cqb_huli_cnm",      "cqb_ua_huli" },         // 17: CQB
        { "mp",          "mp_cbrn_cnm",       "mp_huli_cnm" },         // 18: SECURITY
        { "gungnir",     "gungnir_huli",      "gungnir_cbrn" },        // 19: GUNGNIR
        { "skull",       "skull",             "skull" }                // 20: HAUNTED
    };
    const int h = std::clamp(helmetIdx, 0, 20);
    const int v = std::clamp(variantIdx, 0, 2);
    if (v == 1) return kHelmets[h].v1;
    if (v == 2) return kHelmets[h].v2;
    return kHelmets[h].base;
}

static std::string GetShoulderPermName(int shoulderIdx) {
    static const char* const kShoulders[18] = {
        "default",          // 0: DEFAULT
        "para_knees",       // 1: FJ/PARA
        "hazop",            // 2: HAZOP
        "jfo",              // 3: JFO
        "recon",            // 4: RECON
        "ua_multi_threat",  // 5: UA/MULTI-THREAT
        "jump_jet",         // 6: JUMP JET
        "eva",              // 7: EVA
        "gungnir",          // 8: GUNGNIR
        "odst",             // 9: ODST
        "ua_base_security", // 10: UA/BASE SECURITY
        "cqc",              // 11: CQC
        "operator",         // 12: OPERATOR
        "commando",         // 13: COMMANDO
        "grenadier",        // 14: GRENADIER
        "sniper",           // 15: SNIPER
        "mjolnir_mk_v",     // 16: MJOLNIR MK. V
        "security"          // 17: SECURITY
    };
    return kShoulders[std::clamp(shoulderIdx, 0, 17)];
}

static std::string GetChestPermName(int chestIdx, bool& outRobotHands) {
    outRobotHands = (chestIdx >= 14 && chestIdx <= 16);
    static const char* const kChests[17] = {
        "default",             // 0: DEFAULT
        "tactical_recon",      // 1: TACTICAL/RECON
        "hp_halo",             // 2: HP/HALO
        "ua_counterassault",   // 3: UA/COUNTERASSAULT
        "tactical_lrp",        // 4: TACTICAL/LRP
        "tactical_patrol",     // 5: TACTICAL/PATROL
        "collar_breacher",     // 6: COLLAR/BREACHER
        "assault_sapper",      // 7: ASSAULT/SAPPER
        "assault_commando",    // 8: ASSAULT/COMMANDO
        "hp_parafoil",         // 9: HP/PARAFOIL
        "collar_grenadier",    // 10: COLLAR/GRENADIER
        "ua_multi_threat",     // 11: UA/MULTI-THREAT
        "ua_base_security",    // 12: UA/BASE SECURITY
        "collar_grenadier_ua", // 13: COLLAR/GRENADIER [UA]
        "ua_multi_threat_w",   // 14: UA/MULTI-THREAT [W]
        "collar_breacher_r",   // 15: COLLAR/BREACHER [R]
        "hp_parafoil_r"        // 16: HP/PARAFOIL [R]
    };
    return kChests[std::clamp(chestIdx, 0, 16)];
}

static std::string GetWristPermName(int wristIdx) {
    static const char* const kWrists[6] = {
        "default",          // 0: DEFAULT
        "ua_buckler",       // 1: UA/BUCKLER
        "ua_bracer",        // 2: UA/BRACER
        "tactical_tacpad",  // 3: TACTICAL/TACPAD
        "tactical_ugps",    // 4: TACTICAL/UGPS
        "assault_breacher"  // 5: ASSAULT/BREACHER
    };
    return kWrists[std::clamp(wristIdx, 0, 5)];
}

static std::string GetUtilityPermName(int utilIdx) {
    static const char* const kUtilities[6] = {
        "default",             // 0: DEFAULT
        "ua_chobham",          // 1: UA/CHOBHAM
        "tactical_hard_case",  // 2: TACTICAL/HARD CASE
        "ua_nxra",             // 3: UA/NXRA
        "tactical_trauma_kit", // 4: TACTICAL/TRAUMA KIT
        "tactical_soft_case"   // 5: TACTICAL/SOFT CASE
    };
    return kUtilities[std::clamp(utilIdx, 0, 5)];
}

static std::string GetKneesPermName(int kneesIdx) {
    static const char* const kKnees[4] = {
        "default",    // 0: DEFAULT
        "para_knees", // 1: FJ/PARA
        "gungnir",    // 2: GUNGNIR
        "grenadier"   // 3: GRENADIER
    };
    return kKnees[std::clamp(kneesIdx, 0, 3)];
}

static std::string GetElitePermName(int eliteIdx) {
    static const char* const kElites[8] = {
        "minor",          // 0: MINOR
        "spec_ops",       // 1: SPEC OPS
        "space",          // 2: RANGER
        "ultra",          // 3: ULTRA
        "zealot",         // 4: ZEALOT
        "general",        // 5: GENERAL
        "field_marshall", // 6: FIELD MARSHAL
        "officer"         // 7: OFFICER
    };
    return kElites[std::clamp(eliteIdx, 0, 7)];
}

static void GetVisorLinearRgb(int visorIdx, float outRgb[3]) {
    // sRGB visor tints matching MCC Reach Visor palette, converted with pow(c, 2.2)
    static const float kVisorSrgb[5][3] = {
        { 1.00f, 0.62f, 0.12f }, // 0: DEFAULT (Amber/Gold-Orange)
        { 0.78f, 0.82f, 0.88f }, // 1: SILVER
        { 0.25f, 0.58f, 1.00f }, // 2: BLUE
        { 1.00f, 0.78f, 0.15f }, // 3: GOLD
        { 0.08f, 0.09f, 0.11f }  // 4: BLACK
    };
    const int idx = std::clamp(visorIdx, 0, 4);
    for (int c = 0; c < 3; ++c) {
        outRgb[c] = std::pow(kVisorSrgb[idx][c], 2.2f);
    }
}

// -----------------------------------------------------------------------------
// Apply Armor Permutations & Colors in Engine Memory
// -----------------------------------------------------------------------------
static void ApplyLivePermutationsAndColors() {
    const PlayerProfile& prof = UIManager::GetArmorCustomizer().GetProfile();

    // 1. Spartan Permutations
    const uintptr_t tableBase = s_spartanTableBase.load();
    const size_t stride = s_spartanStride.load();
    if (tableBase != 0 && stride != 0) {
        bool robotHands = false;
        const std::string chestPerm = GetChestPermName(prof.chestIndex, robotHands);
        const std::string slotNames[8] = {
            GetHelmetPermName(prof.helmetIndex, prof.helmetVariantIndex), // 0: head
            chestPerm,                                                    // 1: chest
            GetShoulderPermName(prof.leftShoulderIndex),                  // 2: left_shoulder
            GetShoulderPermName(prof.rightShoulderIndex),                 // 3: right_shoulder
            robotHands ? "robot" : "default",                             // 4: hands
            GetWristPermName(prof.wristIndex),                            // 5: wrist
            GetUtilityPermName(prof.utilityIndex),                        // 6: utility
            GetKneesPermName(prof.kneesIndex)                             // 7: knees
        };

        for (size_t slot = 0; slot < 8; ++slot) {
            uint32_t permId = LookupStringId(slotNames[slot]);
            if (permId == 0) permId = LookupStringId("default");
            if (permId != 0) {
                const uintptr_t slotAddr = tableBase + slot * 3 * stride;
                SafeWriteU32(slotAddr + stride * 5, permId); // Male permutation string_id
                SafeWriteU32(slotAddr + stride * 6, permId); // Female permutation string_id
            }
        }

        // Body Type (male / female at tableBase - 0x4C .. tableBase - 0x3C)
        const uint32_t maleId   = LookupStringId("male");
        const uint32_t femaleId = LookupStringId("female");
        if (maleId != 0 && femaleId != 0) {
            uintptr_t bodyAddr = s_bodyTypeAddr.load();
            if (bodyAddr == 0) {
                for (uintptr_t probe = tableBase - 0x4C; probe <= tableBase - 0x3C; probe += 4) {
                    uint32_t b0 = 0, b1 = 0;
                    if (SafeReadU32(probe, &b0) && SafeReadU32(probe + 4, &b1) &&
                        b0 == maleId && b1 == femaleId) {
                        bodyAddr = probe;
                        s_bodyTypeAddr.store(bodyAddr);
                        break;
                    }
                }
            }
            if (bodyAddr != 0) {
                const uint32_t targetBody = (prof.bodyType == 1) ? femaleId : maleId;
                SafeWriteU32(bodyAddr + 0, targetBody);
                SafeWriteU32(bodyAddr + 4, targetBody);
            }
        }

        // Armor Effects (none, birthday_party, heart_attack, pestilence, inclement_weather, flames, blue_flames)
        uintptr_t effectAddr = s_effectBaseAddr.load();
        if (effectAddr == 0) {
            const uint32_t noneId  = LookupStringId("none");
            const uint32_t bdayId  = LookupStringId("birthday_party");
            const uint32_t heartId = LookupStringId("heart_attack");
            if (noneId != 0 && bdayId != 0 && heartId != 0) {
                for (size_t off = 0; off < 0x1000; off += 4) {
                    uint32_t e0 = 0, e1 = 0, e2 = 0;
                    if (SafeReadU32(tableBase + off, &e0) &&
                        SafeReadU32(tableBase + off + 0x10, &e1) &&
                        SafeReadU32(tableBase + off + 0x20, &e2) &&
                        e0 == noneId && e1 == bdayId && e2 == heartId) {
                        effectAddr = tableBase + off;
                        for (size_t k = 0; k < 7; ++k) {
                            SafeReadU32(effectAddr + k * 0x10 + 4, &s_effectTags[k]);
                        }
                        s_effectBaseAddr.store(effectAddr);
                        break;
                    }
                }
            }
        }
        if (effectAddr != 0) {
            const int effIdx = std::clamp(prof.armorEffectIndex, 0, 6);
            SafeWriteU32(effectAddr + 4, s_effectTags[effIdx]);
        }
    }

    // 2. Elite Permutations
    {
        const uint32_t elitePermId = LookupStringId(GetElitePermName(prof.eliteArmorIndex));
        std::lock_guard<std::mutex> lock(s_dataMutex);
        if (s_eliteScanned && elitePermId != 0) {
            for (size_t r = 0; r < 6; ++r) {
                auto it = s_eliteRegions[r].permsByStringId.find(elitePermId);
                if (it != s_eliteRegions[r].permsByStringId.end() && s_eliteRegions[r].permArrayAddr != 0) {
                    SafeCopyMem(reinterpret_cast<void*>(s_eliteRegions[r].permArrayAddr),
                                it->second.bytes, sizeof(ElitePermEntry));
                }
            }
        }
    }

    // 3. Colors (Primary, Secondary, Visor)
    const ArmorColor& priCol = ArmorCustomizer::GetColorPalette()[std::clamp(prof.primaryColor, 0, 29)];
    const ArmorColor& secCol = ArmorCustomizer::GetColorPalette()[std::clamp(prof.secondaryColor, 0, 29)];
    float visorLinear[3] = {};
    GetVisorLinearRgb(prof.visorColorIndex, visorLinear);

    const float priLinear[4] = {
        std::pow(priCol.r, 2.2f),
        std::pow(priCol.g, 2.2f),
        std::pow(priCol.b, 2.2f),
        0.0f
    };
    const float secLinear[4] = {
        std::pow(secCol.r, 2.2f),
        std::pow(secCol.g, 2.2f),
        std::pow(secCol.b, 2.2f),
        0.0f
    };
    const float visLinear4[4] = {
        visorLinear[0],
        visorLinear[1],
        visorLinear[2],
        0.0f
    };

    const std::array<float, 9> newLinear9 = {
        priLinear[0], priLinear[1], priLinear[2],
        secLinear[0], secLinear[1], secLinear[2],
        visorLinear[0], visorLinear[1], visorLinear[2]
    };

    {
        std::lock_guard<std::mutex> lock(s_dataMutex);
        s_currentLinearColors = newLinear9;
        for (uintptr_t rec : s_colorRecords) {
            SafeWriteFloats(rec + 0x00, priLinear, 4);
            SafeWriteFloats(rec + 0x20, secLinear, 4);
            SafeWriteFloats(rec + 0x30, visLinear4, 4);
            SafeWriteFloats(rec + 0x40, visLinear4, 4);
        }
        for (uintptr_t suit : s_suitRecords) {
            SafeWriteFloats(suit, newLinear9.data(), 9);
        }
    }

    // 4. Pedestal Rotation (haloreach.dll + 0xBE5AD8)
    if (HMODULE hReach = GetModuleHandleW(L"haloreach.dll")) {
        const uintptr_t rotAddr = reinterpret_cast<uintptr_t>(hReach) + 0xBE5AD8;
        uint32_t baseBits = s_rotationBaseBits.load();
        if (baseBits == 0xFFFFFFFFu) {
            uint32_t curBits = 0;
            if (SafeReadU32(rotAddr, &curBits)) {
                float curFloat = 0.0f;
                std::memcpy(&curFloat, &curBits, sizeof(float));
                if (curFloat >= -6.29f && curFloat <= 6.29f) {
                    baseBits = curBits;
                    s_rotationBaseBits.store(baseBits);
                }
            }
        }
        float baseAngle = 0.0f;
        if (baseBits != 0xFFFFFFFFu) {
            std::memcpy(&baseAngle, &baseBits, sizeof(float));
        }
        const float targetAngle = baseAngle + s_yawOffset.load();
        uint32_t targetBits = 0;
        std::memcpy(&targetBits, &targetAngle, sizeof(float));
        if (targetBits != s_lastYawBits.load()) {
            if (SafeWriteU32(rotAddr, targetBits)) {
                s_lastYawBits.store(targetBits);
            }
        }
    }
}

} // namespace

void ReachArmory::Initialize(const std::filesystem::path& dataDir) {
    s_dataDir = dataDir;
    EnsureStringIdsLoaded();
}

void ReachArmory::SetMenuMode(bool menuMode) {
    s_menuMode.store(menuMode);
    if (!menuMode) {
        s_sceneReady.store(false);
        s_lastScene.store(-1);
    }
}

bool ReachArmory::IsMenuMode() {
    return s_menuMode.load();
}

void ReachArmory::SetTargetScene(const std::string& sceneName) {
    std::lock_guard<std::mutex> lock(s_sceneMutex);
    s_targetScene = sceneName;
}

std::string ReachArmory::GetTargetScene() {
    std::lock_guard<std::mutex> lock(s_sceneMutex);
    return s_targetScene;
}

void ReachArmory::AddYawRotation(float deltaRadians) {
    float cur = s_yawOffset.load();
    cur += deltaRadians;
    s_yawOffset.store(cur);
}

void ReachArmory::SetYawRotation(float radians) {
    s_yawOffset.store(radians);
}

float ReachArmory::GetYawRotation() {
    return s_yawOffset.load();
}

bool ReachArmory::IsSceneReady() {
    return s_sceneReady.load();
}

void ReachArmory::Tick() {
    if (!s_menuMode.load()) return;

    EnsureStringIdsLoaded();

    const uint64_t frame = ++s_frameCounter;

    // 1. Determine desired camera scene
    std::string sceneName = GetTargetScene();
    const int32_t sceneIdx = ResolveSceneIndex(sceneName);

    // 2. Switch scene and set Spartan/Elite visibility on the engine render thread
    SwitchSceneOnRenderThread(sceneIdx);

    // 3. Launch background memory scanner once mainmenu_all.map is ready (frame >= 60)
    if (s_sceneReady.load() && frame >= 60 && !s_scanLaunched.exchange(true)) {
        s_scanning.store(true);
        std::thread([]() {
            NOBLE_INFO("ReachArmory: Starting background memory scan for mainmenu_all.map armor tables...");
            ScanSpartanTable();
            ScanColorRecords();
            ScanSuitRecords();
            ScanEliteTable();
            s_scanning.store(false);

            // Follow-up scans for late-streamed color/suit/elite records (matches host-probe closure#0)
            std::this_thread::sleep_for(std::chrono::milliseconds(1500));
            ScanSpartanTable();
            ScanColorRecords();
            ScanSuitRecords();
            ScanEliteTable();

            std::this_thread::sleep_for(std::chrono::milliseconds(2500));
            ScanColorRecords();
            ScanSuitRecords();
            ScanEliteTable();
        }).detach();
    }

    // 4. Apply live permutations, colors, and pedestal rotation
    if (s_sceneReady.load()) {
        ApplyLivePermutationsAndColors();
    }
}

} // namespace noble

