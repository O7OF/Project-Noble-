#include "ReachArmory.hpp"
#include "common/Logger.hpp"

#include <windows.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstring>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
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

static bool SafeWriteU32(uintptr_t addr, uint32_t val) {
    if (addr <= 0x10000) return false;
    __try {
        *reinterpret_cast<uint32_t*>(addr) = val;
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static bool SafeWriteI32(uintptr_t addr, int32_t val) {
    if (addr <= 0x10000) return false;
    __try {
        *reinterpret_cast<int32_t*>(addr) = val;
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

static uint32_t LinearRgbToArgbU32(const std::array<float, 3>& linRgb) {
    const uint32_t r = static_cast<uint32_t>(std::clamp(std::lround(linRgb[0] * 255.0f), 0L, 255L));
    const uint32_t g = static_cast<uint32_t>(std::clamp(std::lround(linRgb[1] * 255.0f), 0L, 255L));
    const uint32_t b = static_cast<uint32_t>(std::clamp(std::lround(linRgb[2] * 255.0f), 0L, 255L));
    return 0xFF000000u | (r << 16) | (g << 8) | b;
}

// -----------------------------------------------------------------------------
// State
// -----------------------------------------------------------------------------
static std::filesystem::path s_dataDir;
static std::atomic<bool> s_menuMode{false};
static std::atomic<bool> s_sceneReady{false};
static std::atomic<bool> s_scanning{false};
static std::atomic<bool> s_rescanRequested{false};
static std::atomic<uint64_t> s_frameCounter{0};
static std::atomic<int32_t> s_lastScene{-1};
static std::atomic<int32_t> s_lastRemade{-1};
static std::atomic<float> s_yawOffset{0.0f};
static std::atomic<uint32_t> s_rotationBaseBits{0xFFFFFFFFu};
static std::atomic<uint32_t> s_lastYawBits{0};
static std::atomic<int32_t> s_lastTurnStep{-1};

static std::mutex s_sceneMutex;
static std::string s_targetScene = "armory";
static ReachArmorySelection s_selection;
static std::atomic<uint64_t> s_selectionGeneration{1};
static uint64_t s_appliedGeneration = 0;

static std::mutex s_dataMutex;
static bool s_stringIdsLoaded = false;
static std::unordered_map<std::string, uint32_t> s_stringIds;

// mainmenu_all.map HS GLOBALS table address and stride (in bytes, from {4, 8, 12, 16, 24, 32})
static std::atomic<uintptr_t> s_globalsBase{0};
static std::atomic<size_t> s_globalsStride{12};

// Elite hlmt 6-region permutation table (direct in-memory permutation swapper)
struct ElitePermEntry {
    uint8_t bytes[0x18];
};
struct EliteRegionCache {
    uintptr_t permArrayAddr = 0;
    std::unordered_map<uint32_t, ElitePermEntry> permsByStringId;
};
static bool s_eliteScanned = false;
static std::array<EliteRegionCache, 6> s_eliteRegions;

// Color records (uint32_t[4] ARGB) & Suit shader records (float[4] linear RGBA)
static std::vector<uintptr_t> s_colorRecords;
static std::vector<uintptr_t> s_suitRecords;
static std::array<uint32_t, 4> s_lastColorWords = {
    0xFF585858u, 0xFF595959u, 0xFF2FA0CBu, 0xFFC7A167u
};
static std::array<float, 4> s_lastSuitFloats = {
    0.9960785f, 0.9921569f, 0.98823535f, 1.0f
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
    EnsureStringIdsLoaded();
    std::lock_guard<std::mutex> lock(s_dataMutex);
    auto it = s_stringIds.find(ToLowerAscii(name));
    return (it != s_stringIds.end()) ? it->second : 0;
}

// -----------------------------------------------------------------------------
// Scene Index Resolution (ported from host::armory::switch_scene @ 0x1401b3fb0)
// -----------------------------------------------------------------------------
static int32_t ResolveSceneIndex(const std::string& scene) {
    const std::string s = ToLowerAscii(scene);
    if (s == "title" || s == "start_menu" || s == "main") return 0x00;
    if (s == "campaign" || s == "campaign_act1")          return 0x01;
    if (s == "campaign_act2")                             return 0x02;
    if (s == "campaign_act3")                             return 0x03;
    if (s == "matchmaking")                               return 0x04;
    if (s == "custom_games")                              return 0x05;
    if (s == "forge" || s == "editor")                    return 0x06;
    if (s == "theater")                                   return 0x07;
    if (s == "activities")                                return 0x08;
    if (s == "firefight" || s == "survival")              return 0x09;
    if (s == "armory")                                    return 0x0A;
    if (s == "armory_elite")                              return 0x0B;
    if (s == "c_full_spartan" || s == "armory_full" || s == "c_armory") return 0x0C;
    if (s == "c_helmet" || s == "armory_head")            return 0x0D;
    if (s == "roster")                                    return 0x0E;
    if (s == "c_chest" || s == "armory_chest")            return 0x0F;
    if (s == "c_wrist" || s == "armory_arm")              return 0x10;
    if (s == "c_utility" || s == "armory_leg")            return 0x11;
    if (s == "c_knees" || s == "armory_knee")             return 0x12;
    if (s == "c_left_shoulder" || s == "c_right_shoulder" || s == "armory_shoulder") return 0x13;
    if (s == "armory_colors")                             return 0x14;
    return 0x0A;
}

// -----------------------------------------------------------------------------
// Memory Scanners (ported from host::armory::scan, find_all, scan_suit, scan_elite)
// -----------------------------------------------------------------------------
// Committed private read/write memory, cut into 16 MB pieces that overlap by 4 KB so a
// record that straddles a cut is still seen whole. Reach keeps its tag data in regions far
// larger than 512 MB, which the old whole-region copy skipped (RR reads 1 MB at a time).
static std::vector<std::pair<uintptr_t, size_t>> ReadWriteChunks(size_t minSize) {
    constexpr size_t kChunk = 16u * 1024u * 1024u;
    constexpr size_t kOverlap = 0x1000;
    std::vector<std::pair<uintptr_t, size_t>> chunks;
    uintptr_t addr = 0;
    MEMORY_BASIC_INFORMATION mbi{};
    while (VirtualQuery(reinterpret_cast<LPCVOID>(addr), &mbi, sizeof(mbi)) != 0) {
        const uintptr_t base = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
        const size_t size = mbi.RegionSize;
        const uintptr_t nextAddr = base + size;
        if (mbi.State == MEM_COMMIT && mbi.Type == MEM_PRIVATE && mbi.Protect == PAGE_READWRITE && size >= minSize) {
            for (size_t off = 0; off < size; off += kChunk) {
                const size_t len = std::min(size - off, kChunk + kOverlap);
                chunks.emplace_back(base + off, len);
                if (off + len >= size) break;
            }
        }
        if (nextAddr <= addr) break;
        addr = nextAddr;
    }
    return chunks;
}

static std::atomic<uint64_t> s_globalsMarkerHits{0};

static bool ScanGlobalsTable() {
    const uintptr_t existing = s_globalsBase.load();
    if (existing != 0) {
        uint32_t m0 = 0, m1 = 0;
        const size_t st = s_globalsStride.load();
        if (SafeReadU32(existing, &m0) && SafeReadU32(existing + st, &m1) &&
            m0 == 0x03BB6FEDu && m1 == 0x03BAB653u) {
            return true;
        }
        s_globalsBase.store(0);
    }

    const uint32_t headId    = LookupStringId("head");
    const uint32_t defaultId = LookupStringId("default");
    if (headId == 0 || defaultId == 0) return false;

    static constexpr size_t kStrides[6] = { 4, 8, 12, 16, 24, 32 };

    for (const auto& chunk : ReadWriteChunks(0x100)) {
        const uintptr_t base = chunk.first;
        const size_t size = chunk.second;
        {
            std::vector<uint8_t> buf(size);
            {   // never scan our own copy buffer (RR skips its read buffer the same way)
                const uintptr_t self = reinterpret_cast<uintptr_t>(buf.data());
                if (base < self + size && self < base + size) continue;
            }
            if (SafeCopyMem(buf.data(), reinterpret_cast<const void*>(base), size)) {
                for (size_t off = 0; off + 0xC0 <= size; off += 4) {
                    const uint32_t v0 = *reinterpret_cast<const uint32_t*>(buf.data() + off);
                    if (v0 != 0x03BB6FEDu) continue;
                    ++s_globalsMarkerHits;

                    for (size_t stride : kStrides) {
                        if (off + stride * 0x34 > size) continue;
                        const uint32_t v1 = *reinterpret_cast<const uint32_t*>(buf.data() + off + stride);
                        const uint32_t v4 = *reinterpret_cast<const uint32_t*>(buf.data() + off + stride * 4);
                        const uint32_t v5 = *reinterpret_cast<const uint32_t*>(buf.data() + off + stride * 5);
                        if (v1 == 0x03BAB653u && (v4 == headId || v5 == defaultId)) {
                            const uintptr_t foundBase = base + off;
                            s_globalsStride.store(stride);
                            s_globalsBase.store(foundBase);
                            if (!s_sceneReady.exchange(true)) {
                                NOBLE_INFO("ReachArmory: Found mainmenu_all.map GLOBALS table at 0x{:X} (stride={})", foundBase, stride);
                            }
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

static void ScanColorRecords() {
    // Stock 16-byte ARGB color pattern in mainmenu_all.map: { 0xFF585858, 0xFF595959, 0xFF2FA0CB, 0xFFC7A167 }
    static constexpr uint32_t kStockColors[4] = { 0xFF585858u, 0xFF595959u, 0xFF2FA0CBu, 0xFFC7A167u };
    uint32_t prevColors[4];
    {
        std::lock_guard<std::mutex> lock(s_dataMutex);
        std::memcpy(prevColors, s_lastColorWords.data(), sizeof(prevColors));
    }

    std::vector<uintptr_t> found;
    for (const auto& chunk : ReadWriteChunks(0x40)) {
        const uintptr_t base = chunk.first;
        const size_t size = chunk.second;
        {
            std::vector<uint8_t> buf(size);
            {   // never scan our own copy buffer (RR skips its read buffer the same way)
                const uintptr_t self = reinterpret_cast<uintptr_t>(buf.data());
                if (base < self + size && self < base + size) continue;
            }
            if (SafeCopyMem(buf.data(), reinterpret_cast<const void*>(base), size)) {
                for (size_t off = 0; off + 16 <= size; off += 4) {
                    if (std::memcmp(buf.data() + off, kStockColors, 16) == 0 ||
                        std::memcmp(buf.data() + off, prevColors, 16) == 0) {
                        found.push_back(base + off);
                    }
                }
            }
        }
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
    // Stock 16-byte suit float[4] in mainmenu_all.map: { 0.9960785f, 0.9921569f, 0.98823535f, 1.0f }
    static constexpr float kStockSuit[4] = { 0.9960785f, 0.9921569f, 0.98823535f, 1.0f };
    float prevSuit[4];
    {
        std::lock_guard<std::mutex> lock(s_dataMutex);
        std::memcpy(prevSuit, s_lastSuitFloats.data(), sizeof(prevSuit));
    }

    std::vector<uintptr_t> found;
    for (const auto& chunk : ReadWriteChunks(0x40)) {
        const uintptr_t base = chunk.first;
        const size_t size = chunk.second;
        {
            std::vector<uint8_t> buf(size);
            {   // never scan our own copy buffer (RR skips its read buffer the same way)
                const uintptr_t self = reinterpret_cast<uintptr_t>(buf.data());
                if (base < self + size && self < base + size) continue;
            }
            if (SafeCopyMem(buf.data(), reinterpret_cast<const void*>(base), size)) {
                for (size_t off = 0; off + 16 <= size; off += 4) {
                    const float* f = reinterpret_cast<const float*>(buf.data() + off);
                    if (std::memcmp(f, kStockSuit, 16) == 0 ||
                        std::memcmp(f, prevSuit, 16) == 0) {
                        found.push_back(base + off);
                    }
                }
            }
        }
    }

    if (!found.empty()) {
        std::lock_guard<std::mutex> lock(s_dataMutex);
        for (uintptr_t rec : found) {
            if (std::find(s_suitRecords.begin(), s_suitRecords.end(), rec) == s_suitRecords.end()) {
                s_suitRecords.push_back(rec);
            }
        }
        NOBLE_INFO("ReachArmory: Tracked {} live suit shader record(s)", s_suitRecords.size());
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

    for (const auto& chunk : ReadWriteChunks(0x90)) {
        const uintptr_t base = chunk.first;
        const size_t size = chunk.second;
        {
            std::vector<uint8_t> buf(size);
            {   // never scan our own copy buffer (RR skips its read buffer the same way)
                const uintptr_t self = reinterpret_cast<uintptr_t>(buf.data());
                if (base < self + size && self < base + size) continue;
            }
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
    }
}

// -----------------------------------------------------------------------------
// Write Colors & Suit Shaders (ported from write_colors @ 0x1401b5ad0 & write_suit @ 0x1400468e0)
// -----------------------------------------------------------------------------
static void WriteColorsAndSuit(const ReachArmorySelection& sel) {
    const uint32_t priArgb = LinearRgbToArgbU32(sel.primaryRgb);
    const uint32_t secArgb = LinearRgbToArgbU32(sel.secondaryRgb);
    const uint32_t terArgb = 0xFF2FA0CBu;
    const uint32_t visArgb = (sel.visor == "default" || sel.visor.empty())
                           ? 0xFFC7A167u
                           : LinearRgbToArgbU32(sel.visorRgb);

    const uint32_t words[4] = { priArgb, secArgb, terArgb, visArgb };
    const float suitFloats[4] = {
        sel.primaryRgb[0] * 4.0f,
        sel.primaryRgb[1] * 4.0f,
        sel.primaryRgb[2] * 4.0f,
        1.0f
    };

    std::lock_guard<std::mutex> lock(s_dataMutex);
    s_lastColorWords = { words[0], words[1], words[2], words[3] };
    s_lastSuitFloats = { suitFloats[0], suitFloats[1], suitFloats[2], suitFloats[3] };

    s_colorRecords.erase(
        std::remove_if(s_colorRecords.begin(), s_colorRecords.end(), [&](uintptr_t rec) {
            uint32_t cur[4] = {};
            if (!SafeCopyMem(cur, reinterpret_cast<const void*>(rec), sizeof(cur))) return true;
            if ((cur[0] >> 24) != 0xFFu || (cur[1] >> 24) != 0xFFu ||
                (cur[2] >> 24) != 0xFFu || (cur[3] >> 24) != 0xFFu) {
                return true;
            }
            return !SafeCopyMem(reinterpret_cast<void*>(rec), words, sizeof(words));
        }),
        s_colorRecords.end()
    );

    s_suitRecords.erase(
        std::remove_if(s_suitRecords.begin(), s_suitRecords.end(), [&](uintptr_t suit) {
            float cur[4] = {};
            if (!SafeCopyMem(cur, reinterpret_cast<const void*>(suit), sizeof(cur))) return true;
            for (int i = 0; i < 4; ++i) {
                if (!std::isfinite(cur[i]) || cur[i] < 0.005f || cur[i] > 4.5f) return true;
            }
            return !SafeWriteFloats(suit, suitFloats, 4);
        }),
        s_suitRecords.end()
    );
}

// -----------------------------------------------------------------------------
// Apply Scene, Rotation, Armor Permutations & Colors via GLOBALS Table
// -----------------------------------------------------------------------------
static void ApplyGlobalsAndPermutations(int32_t targetSceneIdx) {
    const uintptr_t base = s_globalsBase.load();
    const size_t stride  = s_globalsStride.load();
    if (base == 0 || stride == 0) return;

    const ReachArmorySelection sel = ReachArmory::GetSelection();
    // The Elite pedestal (scene 0x0B, shows_elite) belongs to the Armory only. Outside the Armory the menu's own
    // scene and the Spartan line-up are used, otherwise the Armory's Elite (and its camera) stayed on screen behind
    // every other menu after leaving the Armory.
    const bool armoryScene = targetSceneIdx >= 0x0A && targetSceneIdx <= 0x14 && targetSceneIdx != 0x0E;
    const bool showElite = sel.previewElite && armoryScene;

    // 1. Camera Scene Index at GLOBALS[0x24] and shows_elite at GLOBALS[0x26]
    const int32_t effectiveSceneIdx = showElite ? 0x0B : targetSceneIdx;
    int32_t curScene = -1;
    SafeReadI32(base + 0x24 * stride, &curScene);
    uint32_t curElite = 0;
    SafeReadU32(base + 0x26 * stride, &curElite);

    const uint32_t wantElite = showElite ? 1u : 0u;
    if (curScene != effectiveSceneIdx || curElite != wantElite || s_lastScene.load() != effectiveSceneIdx) {
        SafeWriteU32(base + 0x26 * stride, wantElite);
        SafeWriteU32(base + 0x24 * stride, static_cast<uint32_t>(effectiveSceneIdx));
        if (s_lastScene.exchange(effectiveSceneIdx) != effectiveSceneIdx) {
            NOBLE_INFO("ReachArmory: Switched GLOBALS camera scene to 0x{:X} (shows_elite={})", effectiveSceneIdx, wantElite);
        }
    }

    // 2. Pedestal Turn Rotation at GLOBALS[0x27] (0..71 steps of 5 degrees) + haloreach.dll + 0xBE5AD8
    const float yawRad = s_yawOffset.load();
    const float yawDeg = yawRad * (180.0f / 3.14159265358979323846f);
    const int32_t turnStep = ((static_cast<int32_t>(std::lround(yawDeg / 5.0f)) % 72) + 72) % 72;
    if (s_lastTurnStep.exchange(turnStep) != turnStep) {
        SafeWriteI32(base + 0x27 * stride, turnStep);
    }

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
        const float targetAngle = baseAngle + yawRad;
        uint32_t targetBits = 0;
        std::memcpy(&targetBits, &targetAngle, sizeof(float));
        if (targetBits != s_lastYawBits.load()) {
            if (SafeWriteU32(rotAddr, targetBits)) {
                s_lastYawBits.store(targetBits);
            }
        }
    }

    // 3. Check if mainmenu_all.map script finished rebuilding biped (REMADE counter at GLOBALS[0x32])
    int32_t remade = 0;
    if (SafeReadI32(base + 0x32 * stride, &remade)) {
        const int32_t prevRemade = s_lastRemade.exchange(remade);
        if (prevRemade != -1 && prevRemade != remade) {
            s_rescanRequested.store(true);
        }
    }

    // 4. Write 16 (region_string_id, permutation_string_id) pairs to GLOBALS[0x04..0x23] and trigger rebuild
    const uint64_t gen = s_selectionGeneration.load();
    if (gen != s_appliedGeneration) {
        s_appliedGeneration = gen;

        std::vector<std::pair<uint32_t, uint32_t>> pairs;
        auto pushPair = [&](const std::string& regionName, const std::string& permName, const char* fallbackPerm = "default") {
            const uint32_t regId = LookupStringId(regionName);
            uint32_t permId = LookupStringId(permName);
            if (permId == 0 && fallbackPerm) {
                permId = LookupStringId(fallbackPerm);
            }
            if (regId != 0 && permId != 0) {
                pairs.emplace_back(regId, permId);
            }
        };

        if (showElite) {
            // The menu map's Elite model has its own regions (elite_*), with Reach's 8 sets plus Honor Guard,
            // Council, Ossoona, Schism, Combat and Halo 2 Anniversary pieces (mainmenu_all.map render model).
            pushPair("elite_helmet",         sel.eliteHelmet,        "minor");
            pushPair("elite_left_shoulder",  sel.eliteLeftShoulder,  "minor");
            pushPair("elite_right_shoulder", sel.eliteRightShoulder, "minor");
            pushPair("elite_chest",          sel.eliteChest,         "minor");
            pushPair("elite_forearms",       sel.eliteArms,          "default");
            pushPair("elite_legs",           sel.eliteLegs,          "minor");
            if (!sel.armorEffect.empty() && sel.armorEffect != "default") {
                pushPair("armor_effect", sel.armorEffect, "default");
            }
        } else {
            pushPair("head",           sel.helmet,        "default");
            pushPair("chest",          sel.chest,         "chest_blank");
            pushPair("left_shoulder",  sel.leftShoulder,  "left_shoulder_blank");
            pushPair("right_shoulder", sel.rightShoulder, "right_shoulder_blank");
            pushPair("hands",          sel.hands,         "default");
            pushPair("wrist",          sel.wrist,         "wrist_blank");
            pushPair("utility",        sel.utility,       "utility_blank");
            pushPair("knees",          sel.knees,         "knees_default");
            pushPair("armor_effect",   sel.armorEffect,   "default");
        }

        if (!pairs.empty()) {
            while (pairs.size() < 16) {
                pairs.push_back(pairs[0]);
            }
            for (size_t i = 0; i < 16; ++i) {
                SafeWriteU32(base + (0x04 + i * 2) * stride, pairs[i].first);
                SafeWriteU32(base + (0x05 + i * 2) * stride, pairs[i].second);
            }
            int32_t rebuildCounter = 0;
            SafeReadI32(base + 0x02 * stride, &rebuildCounter);
            SafeWriteI32(base + 0x02 * stride, rebuildCounter + 1);
            NOBLE_INFO("ReachArmory: Applied {} permutation slots to GLOBALS (rebuild={}, elite={})",
                       showElite ? "Elite" : "Spartan", rebuildCounter + 1, showElite);
        }

        // Also update Elite hlmt permutation table directly if cached
        if (showElite) {
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
    }

    // 5. Write live colors and suit shader constants every frame
    WriteColorsAndSuit(sel);
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
        s_globalsBase.store(0);
        s_lastScene.store(-1);
        s_lastRemade.store(-1);
        s_appliedGeneration = 0;
    } else {
        s_selectionGeneration.fetch_add(1);
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
    {
        std::lock_guard<std::mutex> lock(s_sceneMutex);
        s_selection = sel;
    }
    s_selectionGeneration.fetch_add(1);
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

uint32_t ReachArmory::ResolveStringId(const std::string& name) {
    return LookupStringId(name);
}

bool ReachArmory::IsSceneReady() {
    return s_sceneReady.load();
}

void ReachArmory::Tick() {
    if (!s_menuMode.load()) return;

    EnsureStringIdsLoaded();

    const uint64_t frame = ++s_frameCounter;

    // 1. Launch background scanner to locate mainmenu_all.map's GLOBALS table and color/suit records
    if ((s_globalsBase.load() == 0 || s_rescanRequested.load()) && frame >= 15 && !s_scanning.exchange(true)) {
        const bool isRescan = s_rescanRequested.exchange(false);
        std::thread([isRescan]() {
            if (!isRescan) {
                NOBLE_INFO("ReachArmory: Scanning process memory for mainmenu_all.map GLOBALS table...");
            }
            // The menu map can take a while to finish loading, so keep looking while the menu is up
            // (every 250 ms for the first 10 s, then once a second) instead of giving up after 10 s.
            for (int attempt = 0; s_menuMode.load(); ++attempt) {
                if (ScanGlobalsTable()) {
                    ScanColorRecords();
                    ScanSuitRecords();
                    ScanEliteTable();
                    break;
                }
                if (!isRescan && (attempt == 4 || attempt % 20 == 19)) {
                    size_t regions = 0, bytes = 0;
                    for (const auto& c : ReadWriteChunks(0x100)) { ++regions; bytes += c.second; }
                    NOBLE_INFO("ReachArmory: GLOBALS not found yet (pass {}, {} marker hit(s), {} chunk(s), {} MB scanned)",
                               attempt + 1, s_globalsMarkerHits.load(), regions, bytes >> 20);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(attempt < 40 ? 250 : 1000));
            }
            if (s_menuMode.load() && s_globalsBase.load() != 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(400));
                ScanColorRecords();
                ScanSuitRecords();
                ScanEliteTable();
            }
            s_scanning.store(false);
        }).detach();
    }

    // The Elite's region table only exists once the Elite model has loaded, so while the Elite is
    // shown and the table hasn't been found, look again about every 2 seconds.
    if (s_globalsBase.load() != 0 && GetSelection().previewElite && frame % 240 == 0 && !s_scanning.load()) {
        bool scanned;
        {
            std::lock_guard<std::mutex> lock(s_dataMutex);
            scanned = s_eliteScanned;
        }
        static int eliteTries = 0;
        if (!scanned && eliteTries < 10) {
            if (++eliteTries == 10) NOBLE_WARN("ReachArmory: Elite region table still not found after 10 looks; armor sets go through GLOBALS only");
            s_rescanRequested.store(true);
        }
    }

    // 2. Apply scene camera, rotation, and live armor/color updates if GLOBALS table is active
    if (s_globalsBase.load() != 0) {
        const std::string sceneName = GetTargetScene();
        const int32_t sceneIdx = ResolveSceneIndex(sceneName);
        ApplyGlobalsAndPermutations(sceneIdx);
    }
}

} // namespace noble
