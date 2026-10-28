#include "ReachArmory.hpp"
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
static ReachArmorySelection s_selection;

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
            // Format in mainmenu_all.string_ids.csv: Index,Total,ID,String (ID is decimal u32)
            // Also support 2-column 0xHEX,String format if present
            size_t c1 = line.find(',');
            if (c1 == std::string::npos) continue;
            size_t c2 = line.find(',', c1 + 1);
            if (c2 != std::string::npos) {
                size_t c3 = line.find(',', c2 + 1);
                if (c3 != std::string::npos) {
                    std::string idPart = line.substr(c2 + 1, c3 - (c2 + 1));
                    std::string namePart = line.substr(c3 + 1);
                    if (!idPart.empty() && std::isdigit(static_cast<unsigned char>(idPart[0]))) {
                        uint32_t id = static_cast<uint32_t>(std::strtoul(idPart.c_str(), nullptr, 10));
                        if (id != 0 && !namePart.empty()) {
                            s_stringIds.emplace(ToLowerAscii(namePart), id);
                            ++count;
                        }
                    }
                    continue;
                }
            }
            std::string hexPart = line.substr(0, c1);
            std::string namePart = line.substr(c1 + 1);
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
    if (s == "campaign")                         return 0x00;
    if (s == "matchmaking")                      return 0x01;
    if (s == "custom_games")                     return 0x02;
    if (s == "forge")                            return 0x03;
    if (s == "theater")                          return 0x04;
    if (s == "firefight")                        return 0x05;
    if (s == "start_menu" || s == "main")        return 0x06;
    if (s == "armory")                           return 0x0A;
    if (s == "armory_elite")                     return 0x0B;
    if (s == "c_armory")                         return 0x0C;
    if (s == "c_helmet" || s == "armory_head")   return 0x0D;
    if (s == "c_left_shoulder")                  return 0x0E;
    if (s == "c_right_shoulder")                 return 0x0F;
    if (s == "c_chest" || s == "armory_chest")   return 0x10;
    if (s == "c_wrist" || s == "armory_arm")     return 0x11;
    if (s == "c_utility" || s == "armory_leg")   return 0x12;
    if (s == "c_knees" || s == "armory_knee")    return 0x13;
    if (s == "c_full_spartan" || s == "armory_full") return 0x14;
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
    const bool isSpartanScene = (targetIdx == 0x0A) || (targetIdx >= 0x0C && targetIdx <= 0x14);
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

static int ResolveEffectIndex(const std::string& effectName) {
    const std::string e = ToLowerAscii(effectName);
    if (e == "birthday_party")    return 1;
    if (e == "heart_attack")      return 2;
    if (e == "pestilence")        return 3;
    if (e == "inclement_weather") return 4;
    if (e == "flames")            return 5;
    if (e == "blue_flames")       return 6;
    return 0;
}

// -----------------------------------------------------------------------------
// Apply Armor Permutations & Colors in Engine Memory
// -----------------------------------------------------------------------------
static void ApplyLivePermutationsAndColors() {
    const ReachArmorySelection sel = ReachArmory::GetSelection();

    // 1. Spartan Permutations
    const uintptr_t tableBase = s_spartanTableBase.load();
    const size_t stride = s_spartanStride.load();
    if (tableBase != 0 && stride != 0) {
        const std::string slotNames[8] = {
            sel.helmet,        // 0: head
            sel.chest,         // 1: chest
            sel.leftShoulder,  // 2: left_shoulder
            sel.rightShoulder, // 3: right_shoulder
            sel.hands,         // 4: hands ("default" or "robot")
            sel.wrist,         // 5: wrist
            sel.utility,       // 6: utility
            sel.knees          // 7: knees
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
                const uint32_t targetBody = sel.isFemale ? femaleId : maleId;
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
            const int effIdx = ResolveEffectIndex(sel.armorEffect);
            SafeWriteU32(effectAddr + 4, s_effectTags[effIdx]);
        }
    }

    // 2. Elite Permutations
    {
        const uint32_t elitePermId = LookupStringId(sel.eliteSuit);
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

    // 3. Colors (Primary, Secondary, Visor - already in linear RGB)
    const float priLinear[4] = { sel.primaryRgb[0], sel.primaryRgb[1], sel.primaryRgb[2], 0.0f };
    const float secLinear[4] = { sel.secondaryRgb[0], sel.secondaryRgb[1], sel.secondaryRgb[2], 0.0f };
    const float visLinear4[4] = { sel.visorRgb[0], sel.visorRgb[1], sel.visorRgb[2], 0.0f };

    const std::array<float, 9> newLinear9 = {
        sel.primaryRgb[0], sel.primaryRgb[1], sel.primaryRgb[2],
        sel.secondaryRgb[0], sel.secondaryRgb[1], sel.secondaryRgb[2],
        sel.visorRgb[0], sel.visorRgb[1], sel.visorRgb[2]
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

void ReachArmory::SetSelection(const ReachArmorySelection& sel) {
    std::lock_guard<std::mutex> lock(s_sceneMutex);
    s_selection = sel;
}

ReachArmorySelection ReachArmory::GetSelection() {
    std::lock_guard<std::mutex> lock(s_sceneMutex);
    return s_selection;
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
