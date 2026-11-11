#include "ReachArmory.hpp"
#include "ElitePack.hpp"
#include "common/Logger.hpp"
#include "ui/FrontEndMenu.hpp"
#include "EmbeddedBlocks.hpp"
#include <sstream>

#include <windows.h>
#include <shellapi.h>
#include <tlhelp32.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <new>
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

static std::string ToLowerAscii(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return s;
}

static float LinearToSrgb(float lin) {
    if (lin <= 0.0031308f) return lin * 12.92f;
    return 1.055f * std::pow(std::clamp(lin, 0.0f, 1.0f), 1.0f / 2.4f) - 0.055f;
}

static uint32_t LinearRgbToArgbU32(const std::array<float, 3>& linRgb) {
    const float rS = LinearToSrgb(linRgb[0]);
    const float gS = LinearToSrgb(linRgb[1]);
    const float bS = LinearToSrgb(linRgb[2]);
    const uint32_t r = static_cast<uint32_t>(std::clamp(std::lround(rS * 255.0f), 0L, 255L));
    const uint32_t g = static_cast<uint32_t>(std::clamp(std::lround(gS * 255.0f), 0L, 255L));
    const uint32_t b = static_cast<uint32_t>(std::clamp(std::lround(bS * 255.0f), 0L, 255L));
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
static std::atomic<int32_t> s_mainScene{-1};   // camera the menu map opened on; "main" restores this
static std::atomic<int32_t> s_lastRemade{-1};
static std::atomic<float> s_yawOffset{0.0f};
static std::atomic<uint32_t> s_rotationBaseBits{0xFFFFFFFFu};
static std::atomic<uint32_t> s_lastYawBits{0};
static std::atomic<int32_t> s_lastTurnStep{-1};

static std::recursive_mutex s_sceneMutex;
static std::string s_targetScene = "armory";
static ReachArmorySelection s_selection;
static std::atomic<uint64_t> s_selectionGeneration{1};
static uint64_t s_appliedGeneration = 0;
// A colour/visor/armour change only updates the records the engine already tracks; the live model keeps its
// old shader data until the biped is re-instantiated. Set this whenever the visual part of the selection changes
// so the next apply forces the permutation write + REBUILD even when the piece list is identical.
static std::atomic<bool> s_previewDirty{false};

static std::recursive_mutex s_dataMutex;
using ArmoryLock = std::lock_guard<std::recursive_mutex>;
static bool s_stringIdsLoaded = false;
static std::unordered_map<std::string, uint32_t> s_stringIds;

// The armour pack map (data\armor\<name>.map + .rrreloc): its own string table and .rrpmcg are the
// authority for the Armory piece ids. Discovered once per process.
static std::filesystem::path s_armoryMapPath;
static bool s_armoryMapChecked = false;

// mainmenu_all.map HS GLOBALS table address and stride (in bytes, from {4, 8, 12, 16, 24, 32})
enum RrGlobalSlot : size_t {
    kRrMarkA     = 0x00, // [00 /  0] rr_mark_a      type=8  flags=0 init=-2146886507 (0x03BB6FED)
    kRrMarkB     = 0x01, // [01 /  1] rr_mark_b      type=8  flags=0 init=-2146558822 (0x03BAB653)
    kRrSeq       = 0x02, // [02 /  2] rr_seq         type=8  flags=0 init=-2146231137
    kRrDone      = 0x03, // [03 /  3] rr_done        type=8  flags=0 init=-2145903452
    kRrR0        = 0x04, // [04 /  4] rr_r0          type=11 flags=0 init=-2145575767
    kRrP0        = 0x05, // [05 /  5] rr_p0          type=11 flags=0 init=-2145248082
    kRrR1        = 0x06, // [06 /  6] rr_r1          type=11 flags=0 init=-2144920397
    kRrP1        = 0x07, // [07 /  7] rr_p1          type=11 flags=0 init=-2144592712
    kRrR2        = 0x08, // [08 /  8] rr_r2          type=11 flags=0 init=-2144265027
    kRrP2        = 0x09, // [09 /  9] rr_p2          type=11 flags=0 init=-2143937342
    kRrR3        = 0x0A, // [0a / 10] rr_r3          type=11 flags=0 init=-2143609657
    kRrP3        = 0x0B, // [0b / 11] rr_p3          type=11 flags=0 init=-2143281972
    kRrR4        = 0x0C, // [0c / 12] rr_r4          type=11 flags=0 init=-2142954287
    kRrP4        = 0x0D, // [0d / 13] rr_p4          type=11 flags=0 init=-2142626602
    kRrR5        = 0x0E, // [0e / 14] rr_r5          type=11 flags=0 init=-2142298917
    kRrP5        = 0x0F, // [0f / 15] rr_p5          type=11 flags=0 init=-2141971232
    kRrR6        = 0x10, // [10 / 16] rr_r6          type=11 flags=0 init=-2141643547
    kRrP6        = 0x11, // [11 / 17] rr_p6          type=11 flags=0 init=-2141315862
    kRrR7        = 0x12, // [12 / 18] rr_r7          type=11 flags=0 init=-2140988177
    kRrP7        = 0x13, // [13 / 19] rr_p7          type=11 flags=0 init=-2140660492
    kRrR8        = 0x14, // [14 / 20] rr_r8          type=11 flags=0 init=-2140332807
    kRrP8        = 0x15, // [15 / 21] rr_p8          type=11 flags=0 init=-2140005122
    kRrR9        = 0x16, // [16 / 22] rr_r9          type=11 flags=0 init=-2139677437
    kRrP9        = 0x17, // [17 / 23] rr_p9          type=11 flags=0 init=-2139349752
    kRrR10       = 0x18, // [18 / 24] rr_r10         type=11 flags=0 init=-2139022067
    kRrP10       = 0x19, // [19 / 25] rr_p10         type=11 flags=0 init=-2138694382
    kRrR11       = 0x1A, // [1a / 26] rr_r11         type=11 flags=0 init=-2138366697
    kRrP11       = 0x1B, // [1b / 27] rr_p11         type=11 flags=0 init=-2138039012
    kRrR12       = 0x1C, // [1c / 28] rr_r12         type=11 flags=0 init=-2137711327
    kRrP12       = 0x1D, // [1d / 29] rr_p12         type=11 flags=0 init=-2137383642
    kRrR13       = 0x1E, // [1e / 30] rr_r13         type=11 flags=0 init=-2137055957
    kRrP13       = 0x1F, // [1f / 31] rr_p13         type=11 flags=0 init=-2136728272
    kRrR14       = 0x20, // [20 / 32] rr_r14         type=11 flags=0 init=-2136400587
    kRrP14       = 0x21, // [21 / 33] rr_p14         type=11 flags=0 init=-2136072902
    kRrR15       = 0x22, // [22 / 34] rr_r15         type=11 flags=0 init=-2135745217
    kRrP15       = 0x23, // [23 / 35] rr_p15         type=11 flags=0 init=-2135417532
    kRrScene     = 0x24, // [24 / 36] rr_scene       type=8  flags=0 init=-2135089847
    kRrSceneDone = 0x25, // [25 / 37] rr_scene_done  type=8  flags=0 init=-2134762162
    kRrElite     = 0x26, // [26 / 38] rr_elite       type=8  flags=0 init=-2134434477
    kRrTurn      = 0x27, // [27 / 39] rr_turn        type=8  flags=0 init=-2134106792
    kRrTurnDone  = 0x28, // [28 / 40] rr_turn_done   type=8  flags=0 init=-2133779107
    kRrTurnD     = 0x29, // [29 / 41] rr_turn_d      type=8  flags=0 init=-2133451422
    kRrC         = 0x2A, // [2a / 42] rr_c           type=6  flags=0 init=-2133123737
    kRrS         = 0x2B, // [2b / 43] rr_s           type=6  flags=0 init=-2132796052
    kRrSpun      = 0x2C, // [2c / 44] rr_spun        type=8  flags=0 init=-2132468367
    kRrPose      = 0x2D, // [2d / 45] rr_pose        type=8  flags=0 init=-2132140682
    kRrPoseDone  = 0x2E, // [2e / 46] rr_pose_done   type=8  flags=0 init=-2131812997
    kRrSg        = 0x2F, // [2f / 47] rr_sg          type=5  flags=0 init=-2131485312
    kRrFxn       = 0x30, // [30 / 48] rr_fxn         type=11 flags=0 init=-2131157627
    kRrFxp       = 0x31, // [31 / 49] rr_fxp         type=11 flags=0 init=-2130829942
    kRrRemade    = 0x32, // [32 / 50] rr_remade      type=8  flags=0 init=-2130502257
    kRrCardDone  = 0x33, // [33 / 51] rr_card_done   type=8  flags=0 init=-2130174572
    kRrSlotCount = 0x34
};
static constexpr uint32_t kRrMarkAVal = 0x03BB6FEDu;
static constexpr uint32_t kRrMarkBVal = 0x03BAB653u;
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
static std::vector<uintptr_t> s_eliteColorRecords;
static std::vector<uintptr_t> s_suitRecords;
static std::array<uint32_t, 4> s_lastColorWords = {
    0xFF585858u, 0xFF595959u, 0xFF2FA0CBu, 0xFFC7A167u
};
static uint32_t s_lastElitePriArgb = 0;
static uint32_t s_lastEliteSecArgb = 0;
static uint32_t s_lastSpartanPriArgb = 0;
static uint32_t s_lastSpartanSecArgb = 0;
static uint32_t s_lastSpartanVisArgb = 0;
static std::array<float, 4> s_lastSuitFloats = {
    0.9960785f, 0.9921569f, 0.98823535f, 1.0f
};
// Memory regions the colour / suit records were found in. A rebuilt biped's new records land in the same
// regions, so after a rebuild only these are searched (a few ms) instead of all memory (the white flash).
static std::vector<std::pair<uintptr_t, size_t>> s_recordRegions;
static std::atomic<bool> s_quickScanning{false};
// Full-memory scans are expensive (the menu process holds ~7 GB; one full pass copies all of it). Rapid Spartan /
// Elite switching used to start one on every switch to the Elite while the menu scanner kept repeating its own full
// scans, which pinned the CPU, lagged the menu and ended in a std::bad_alloc on the scanner thread (2026-11-10 20:02).
// The quick rescans now search the remembered regions and fall back to a full scan at most once per this interval.
constexpr uint64_t kFullRescanIntervalMs = 10000;
static std::atomic<uint64_t> s_lastFullRescanMs{0};
static std::atomic<bool> s_eliteRecordsSeen{false};   // Elite colour records were found at least once this menu session
static std::atomic<uint64_t> s_lastRetryMs{0};        // Tick's slow retry while the Armory is missing records
constexpr uint64_t kRecordRetryIntervalMs = 3000;

static bool FullRescanAllowedNow() {
    const uint64_t now = GetTickCount64();
    uint64_t last = s_lastFullRescanMs.load();
    if (last != 0 && now - last < kFullRescanIntervalMs) return false;
    return s_lastFullRescanMs.compare_exchange_strong(last, now);
}

// Copy buffer for one scan chunk. A failed allocation skips that chunk instead of ending Noble: an uncaught
// std::bad_alloc on a detached scanner thread called std::terminate (2026-11-10 20:02:31). Logged, because a
// failure here can also mean a corrupted chunk size, which should stay visible.
static bool AllocScanBuffer(std::vector<uint8_t>& buf, size_t size) {
    try {
        buf.resize(size);
        return true;
    } catch (const std::bad_alloc&) {
        static std::atomic<int> s_logged{0};
        if (s_logged.fetch_add(1) < 5) {
            NOBLE_WARN("ReachArmory: could not allocate a {} KB scan buffer; skipping that memory chunk", size >> 10);
        }
        return false;
    }
}

// Last permutation pairs written (unchanged pairs = no rebuild, so colour-only changes don't rebuild the biped),
// and the rebuild: REBUILD is bumped a couple of frames after the pairs are written (and, with
// -armory-settle-rebuild, once more after the biped has been remade).
static std::vector<std::pair<uint32_t, uint32_t>> s_lastPairs;
static std::vector<std::pair<uint32_t, uint32_t>> s_livePairs;
static std::atomic<uint64_t> s_livePairsGen{0};
static uintptr_t s_lastPairsBase = 0;
static uint64_t s_bumpAtFrame = 0;       // 0 = none
static bool s_settlePending = false;
static uint64_t s_settleDeadline = 0;
static int32_t s_remadeAtBump = 0;
// Frames to let the map script finish recreating the biped before another rebuild may be requested. Rapid
// Armory changes (spam-clicking helmets, buying one after another) used to stack rebuild requests on top of a
// rebuild in flight, which crashed the engine; changes that arrive meanwhile are coalesced into one rebuild.
constexpr uint64_t kRebuildQuietFrames = 12;
static uint64_t s_rebuildReadyFrame = 0;
// Spartan / Elite switch wait (2026-11-10). The species (GLOBALS shows_elite + camera) used to flip on every press
// while the new species' pieces were only written once no rebuild was in flight, so during fast switching the menu
// script rebuilt the newly shown character with the other species' (region, permutation) pairs: wrong helmet /
// Elite pieces for several seconds, one DMR drop per rebuild. The flip now waits until the selection's species has
// been stable for kSpeciesSettleMs and no rebuild is running, then the camera, shows_elite and that species' pairs
// are written in the same frame (one rebuild). While it waits, the character on screen keeps the look it has.
constexpr uint64_t kSpeciesSettleMs = 350;
static std::atomic<uint64_t> s_speciesChangeMs{0};    // GetTickCount64 of the last selection whose species changed
static std::atomic<bool> s_speciesDeferred{false};    // a species flip is waiting (read by Tick / NotifyEngineTick)
static uint64_t s_speciesWaitStartMs = 0;              // first frame of the current wait (for the log line)
// Last selection applied to the species on screen. Guarded by s_sceneMutex: written by Tick (render thread) and
// read by NotifyEngineTick, which ReachHost also calls from the engine's input callback.
static ReachArmorySelection s_shownSelection;
static bool s_haveShownSelection = false;
// What each species' character was last built with (index 0 = Spartan, 1 = Elite). The menu map keeps the hidden
// species' character, so switching back to a species whose pieces did not change needs no rebuild: the forced
// rebuild on every species switch was the flicker (pose reset) and the DMR drop seen in Tyler's 21:00 video.
static std::vector<std::pair<uint32_t, uint32_t>> s_builtPairs[2];
static std::vector<std::pair<uint32_t, uint32_t>> s_builtLivePairs[2];
static uintptr_t s_builtBase[2] = { 0, 0 };
// Tick's colour-record retry gives up after a few tries per species shown (the records are rarely found and each
// try is a quick rescan); a species switch or a rebuild starts a new budget.
constexpr int kRecordRetryBudget = 3;
static std::atomic<int> s_recordRetriesLeft{kRecordRetryBudget};

// The pack map that carries the Armory models and the player_model_customization tag.
static const std::filesystem::path& ArmoryMapPath() {
    if (!s_armoryMapChecked) {
        s_armoryMapChecked = true;
        if (!s_dataDir.empty()) s_armoryMapPath = ElitePack::FindPackMap(s_dataDir);
    }
    return s_armoryMapPath;
}

// -----------------------------------------------------------------------------
// Load string_ids from data/menumaps/mainmenu_all.string_ids.csv
// -----------------------------------------------------------------------------
static void EnsureStringIdsLoaded() {
    ArmoryLock lock(s_dataMutex);
    if (s_stringIdsLoaded) return;
    s_stringIdsLoaded = true;

    auto parseCsvStream = [](auto& in) -> size_t {
        std::string line;
        size_t count = 0;
        while (std::getline(in, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
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
        return count;
    };

    if (!s_dataDir.empty()) {
        std::ifstream loose(s_dataDir / "menumaps" / "mainmenu_all.string_ids.csv");
        if (loose) {
            const size_t loaded = parseCsvStream(loose);
            static const char* const kExtraCsvs[] = { "mainmenu_armory.string_ids.csv", "mainmenu_armory_elite.string_ids.csv" };
            for (const char* extra : kExtraCsvs) {
                std::ifstream extraIn(s_dataDir / "menumaps" / extra);
                if (extraIn) parseCsvStream(extraIn);
            }
            NOBLE_INFO("ReachArmory: Loaded {} string_ids from loose mainmenu_all CSV", loaded);
            return;
        }
    }

    // 1. Try embedded mainmenu_all.string_ids.csv resource first
    size_t embeddedSize = 0;
    const uint8_t* embeddedData = EmbeddedBlocks::GetBlock("mainmenu_all.string_ids.csv", &embeddedSize);
    if (embeddedData && embeddedSize > 0) {
        std::string_view sv(reinterpret_cast<const char*>(embeddedData), embeddedSize);
        std::stringstream ss{ std::string(sv) };
        size_t loaded = parseCsvStream(ss);
        static const char* const kExtraCsvs[] = { "mainmenu_armory.string_ids.csv", "mainmenu_armory_elite.string_ids.csv" };
        for (const char* extra : kExtraCsvs) {
            size_t extraSize = 0;
            if (const uint8_t* extraData = EmbeddedBlocks::GetBlock(extra, &extraSize); extraData && extraSize > 0) {
                std::stringstream extraSs{ std::string(reinterpret_cast<const char*>(extraData), extraSize) };
                parseCsvStream(extraSs);
            }
        }
        NOBLE_INFO("ReachArmory: Loaded {} string_ids from embedded resource", loaded);
        return;
    }

    // 2. Fall back to disk files
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
        size_t count = parseCsvStream(in);
        NOBLE_INFO("ReachArmory: Loaded {} string_ids from '{}'", count, path.string());
        return;
    }
    NOBLE_WARN("ReachArmory: Could not locate mainmenu_all.string_ids.csv!");
}

// string_id -> name from the csv (for logging), or the hex value.
static std::string NameOfStringId(uint32_t id) {
    EnsureStringIdsLoaded();
    ArmoryLock lock(s_dataMutex);
    for (const auto& kv : s_stringIds)
        if (kv.second == id) return kv.first;
    char buf[16];
    std::snprintf(buf, sizeof(buf), "0x%X", id);
    return buf;
}

static uint32_t LookupStringId(const std::string& name) {
    EnsureStringIdsLoaded();
    ArmoryLock lock(s_dataMutex);
    auto it = s_stringIds.find(ToLowerAscii(name));
    return (it != s_stringIds.end()) ? it->second : 0;
}

// Item permutations are named differently in the pack's pmcg ("nicole") and the menu map's model
// regions ("player_nicole"). Resolve both, so a short name never silently drops the piece.
static uint32_t LookupPermutationId(const std::string& name) {
    const uint32_t exact = LookupStringId(name);
    if (exact != 0 || name.empty()) return exact;
    return LookupStringId("player_" + name);
}

// -----------------------------------------------------------------------------
// Scene Index Resolution (ported from host::armory::switch_scene @ 0x1401b3fb0)
// -----------------------------------------------------------------------------
static int32_t ResolveSceneIndex(const std::string& scene) {
    const std::string s = ToLowerAscii(scene);
    // "index:<n>" addresses a camera scene by number, so every camera in mainmenu_all.map can be tried without
    // naming it first (used to map the per-piece close-ups; see the 2026-10-31 handoff entry).
    if (s.rfind("index:", 0) == 0) {
        const char* text = s.c_str() + 6;
        const int parsed = (text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) ? static_cast<int>(std::strtol(text + 2, nullptr, 16))
                                                                                : static_cast<int>(std::strtol(text, nullptr, 10));
        return std::clamp(parsed, 0, 0x40);
    }
    // 0x00 is the title-screen cliff the menu map opens on; 0x01 is the New Alexandria campaign
    // backdrop. The main menu must stay on 0x00: switching to 0x01 a few seconds after startup pans
    // the camera onto the campaign scene and looks like the menu zooms out and tears (2026-11-02).
    if (s == "title" || s == "start_menu" || s == "main") return 0x00;
    if (s == "campaign" || s == "campaign_act1")         return 0x01;
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
// Thread stacks. The record scanners search this process's read/write memory for patterns that look like the
// Armory's colour and suit records, and the apply code then WRITES colours into every record it found. The
// patterns (four 0xFFrrggbb words, or four floats in 0..6) also appear in the local arrays the apply code and
// the scanners build on their own stacks, and the scanners used to skip only the stack of the thread they ran
// on. A record "found" on another thread's stack, or on a stack that was freed and reused, got colour floats
// written over live data (the crash dump showed a scan loop's vector pointer replaced by float values).
// Every thread's stack is now excluded, from the scan and from the write.
// -----------------------------------------------------------------------------
using StackRanges = std::vector<std::pair<uintptr_t, uintptr_t>>;

static bool SafeReadPtr(uintptr_t addr, uintptr_t* out) {
    if (addr <= 0x10000 || !out) return false;
    __try {
        *out = *reinterpret_cast<const uintptr_t*>(addr);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

struct NobleThreadBasicInfo {
    LONG exitStatus;
    PVOID tebBaseAddress;
    PVOID clientIdProcess;
    PVOID clientIdThread;
    ULONG_PTR affinityMask;
    LONG priority;
    LONG basePriority;
};

static StackRanges CollectThreadStackRangesNow() {
    StackRanges out;
    ULONG_PTR ownLow = 0, ownHigh = 0;
    GetCurrentThreadStackLimits(&ownLow, &ownHigh);
    if (ownLow != 0 && ownHigh != 0) out.emplace_back(ownLow, ownHigh);

    using NtQueryThreadFn = LONG(NTAPI*)(HANDLE, ULONG, PVOID, ULONG, PULONG);
    static const NtQueryThreadFn ntQueryThread = reinterpret_cast<NtQueryThreadFn>(
        GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtQueryInformationThread"));
    if (!ntQueryThread) return out;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return out;
    const DWORD pid = GetCurrentProcessId();
    THREADENTRY32 entry{};
    entry.dwSize = sizeof(entry);
    if (Thread32First(snapshot, &entry)) {
        do {
            if (entry.th32OwnerProcessID != pid) continue;
            HANDLE thread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, entry.th32ThreadID);
            if (!thread) continue;
            NobleThreadBasicInfo info{};
            if (ntQueryThread(thread, 0 /* ThreadBasicInformation */, &info, sizeof(info), nullptr) >= 0 &&
                info.tebBaseAddress) {
                // x64 TEB: NT_TIB.StackBase at +0x08, DeallocationStack (the lowest address of the stack
                // reservation) at +0x1478.
                uintptr_t stackBase = 0, deallocation = 0;
                const uintptr_t teb = reinterpret_cast<uintptr_t>(info.tebBaseAddress);
                if (SafeReadPtr(teb + 0x08, &stackBase) && SafeReadPtr(teb + 0x1478, &deallocation) &&
                    deallocation != 0 && deallocation < stackBase) {
                    out.emplace_back(deallocation, stackBase);
                }
            }
            CloseHandle(thread);
        } while (Thread32Next(snapshot, &entry));
    }
    CloseHandle(snapshot);
    return out;
}

// maxAgeMs 0 = look now (the scanners); larger = reuse a recent answer (the per-frame apply code).
static StackRanges ThreadStackRanges(ULONGLONG maxAgeMs) {
    static std::mutex s_stackMutex;
    static StackRanges s_cached;
    static ULONGLONG s_cachedAt = 0;
    std::lock_guard<std::mutex> lock(s_stackMutex);
    const ULONGLONG now = GetTickCount64();
    if (maxAgeMs == 0 || s_cached.empty() || now - s_cachedAt > maxAgeMs) {
        s_cached = CollectThreadStackRangesNow();
        s_cachedAt = now;
    }
    return s_cached;
}

static bool AddrInStack(const StackRanges& stacks, uintptr_t addr) {
    for (const auto& r : stacks) {
        if (addr >= r.first && addr < r.second) return true;
    }
    return false;
}

static bool RangeOverlapsStack(const StackRanges& stacks, uintptr_t begin, uintptr_t end) {
    for (const auto& r : stacks) {
        if (begin < r.second && end > r.first) return true;
    }
    return false;
}

// Drops candidate record addresses that sit inside a thread stack.
static void DropStackAddresses(std::vector<uintptr_t>& addrs, const char* what) {
    if (addrs.empty()) return;
    const StackRanges stacks = ThreadStackRanges(0);
    const size_t before = addrs.size();
    addrs.erase(std::remove_if(addrs.begin(), addrs.end(), [&](uintptr_t a) { return AddrInStack(stacks, a); }),
                addrs.end());
    if (addrs.size() != before) {
        static std::atomic<int> s_logged{0};
        if (s_logged.fetch_add(1) < 10) {
            NOBLE_WARN("ReachArmory: ignored {} {} record candidate(s) found inside a thread stack", before - addrs.size(), what);
        }
    }
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

    const StackRanges stacks = ThreadStackRanges(0);

    while (VirtualQuery(reinterpret_cast<LPCVOID>(addr), &mbi, sizeof(mbi)) != 0) {
        const uintptr_t base = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
        const size_t size = mbi.RegionSize;
        const uintptr_t nextAddr = base + size;
        if (mbi.State == MEM_COMMIT && mbi.Type == MEM_PRIVATE && mbi.Protect == PAGE_READWRITE && size >= minSize) {
            if (RangeOverlapsStack(stacks, base, nextAddr)) {
                // Skip thread stack memory (every thread's, not only this one's)
            } else {
                for (size_t off = 0; off < size; off += kChunk) {
                    const size_t len = std::min(size - off, kChunk + kOverlap);
                    chunks.emplace_back(base + off, len);
                    if (off + len >= size) break;
                }
            }
        }
        if (nextAddr <= addr) break;
        addr = nextAddr;
    }
    return chunks;
}

static std::atomic<uint64_t> s_globalsMarkerHits{0};

static bool IsNewMenuLayout() {
    EnsureStringIdsLoaded();
    ArmoryLock lock(s_dataMutex);
    return s_stringIds.size() < 32100;   // mainmenu_all.string_ids.csv has 32,213 names
}

static int GlobalsSlotFor(uintptr_t base, size_t stride, uint32_t id) {
    if (id == 0) return -1;
    for (size_t i = 0; i < 0x120; ++i) {
        uint32_t v = 0;
        if (SafeReadU32(base + i * stride, &v) && v == id) return static_cast<int>(i);
    }
    return -1;
}
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
    const uint32_t chestId   = LookupStringId("chest");
    const uint32_t flairHeadAId = LookupStringId("flair_head_a");
    if (headId == 0 || defaultId == 0) return false;

    static constexpr size_t kStrides[6] = { 4, 8, 12, 16, 24, 32 };

    for (const auto& chunk : ReadWriteChunks(0x100)) {
        const uintptr_t base = chunk.first;
        const size_t size = chunk.second;
        {
            std::vector<uint8_t> buf;
            if (!AllocScanBuffer(buf, size)) continue;
            {   // never scan our own copy buffer (RR skips its read buffer the same way)
                const uintptr_t self = reinterpret_cast<uintptr_t>(buf.data());
                if (base < self + size && self < base + size) continue;
            }
            if (SafeCopyMem(buf.data(), reinterpret_cast<const void*>(base), size)) {
                for (size_t off = 0; off + 0xC0 <= size; off += 4) {
                    const uint32_t v0 = *reinterpret_cast<const uint32_t*>(buf.data() + off);
                    if (v0 != kRrMarkAVal) continue;
                    // Skip the scenario hs_syntax_nodes initialization block (0xff82d40: 0x00090008, 0xffffffff,
                    // 0x0001ad2e, 0x03bb6fed, 0x00000311, 0xbaba0000, 0xbabababa...) where rr_mark_a..rr_card_done
                    // init expression nodes (-2146886507..-2130174572) sit 5 syntax nodes (120 bytes) apart.
                    if (off >= 12 && off + 16 <= size &&
                        *reinterpret_cast<const uint32_t*>(buf.data() + off - 12) == 0x00090008u &&
                        *reinterpret_cast<const uint32_t*>(buf.data() + off - 8)  == 0xFFFFFFFFu &&
                        *reinterpret_cast<const uint32_t*>(buf.data() + off + 8)  == 0xBABA0000u) {
                        continue;
                    }
                    ++s_globalsMarkerHits;

                    for (size_t stride : kStrides) {
                        if (off + stride * 0x34 > size) continue;
                        const uint32_t v1 = *reinterpret_cast<const uint32_t*>(buf.data() + off + stride);
                        const uint32_t v4 = *reinterpret_cast<const uint32_t*>(buf.data() + off + stride * 4);
                        const uint32_t v5 = *reinterpret_cast<const uint32_t*>(buf.data() + off + stride * 5);
                        if (v1 == kRrMarkBVal &&
                            (v4 == headId || v5 == defaultId || (v4 > 0 && v4 < 0x10000u && v5 > 0 && v5 < 0x10000u))) {
                            const uintptr_t foundBase = base + off;
                            s_globalsStride.store(stride);
                            s_globalsBase.store(foundBase);
                            if (!s_sceneReady.exchange(true)) {
                                NOBLE_INFO("ReachArmory: Found mainmenu_all.map GLOBALS table at 0x{:X} (stride={})", foundBase, stride);
                                // The menu script's own pairs before Noble writes any (which regions it sets itself).
                                std::string pairsText;
                                for (size_t i = 0; i < 16; ++i) {
                                    uint32_t r = 0, pm = 0;
                                    SafeReadU32(foundBase + (0x04 + i * 2) * stride, &r);
                                    SafeReadU32(foundBase + (0x05 + i * 2) * stride, &pm);
                                    pairsText += " " + NameOfStringId(r) + ":" + NameOfStringId(pm);
                                }
                                NOBLE_INFO("ReachArmory: menu script's starting pairs:{}", pairsText);
                            }
                            return true;
                        }
                    }
                }
            }
        }
    }

    // Imported mainmenu_all.map (RR's new armory map): no legacy marker, but the table still
    // starts with head/default and has chest at slot 2 plus the flair armour block at 20..40.
    // Never run this against the legacy map: its skeleton table also matches head/default and
    // would win before the real marker table appears.
    if (!IsNewMenuLayout()) return false;
    if (chestId != 0 && flairHeadAId != 0) {
        for (const auto& chunk : ReadWriteChunks(0x100)) {
            const uintptr_t chunkBase = chunk.first;
            const size_t size = chunk.second;
            std::vector<uint8_t> buf;
            if (!AllocScanBuffer(buf, size)) continue;
            const uintptr_t self = reinterpret_cast<uintptr_t>(buf.data());
            if (chunkBase < self + size && self < chunkBase + size) continue;
            if (!SafeCopyMem(buf.data(), reinterpret_cast<const void*>(chunkBase), size)) continue;
            for (size_t off = 0; off + 4 <= size; off += 4) {
                if (*reinterpret_cast<const uint32_t*>(buf.data() + off) != headId) continue;
                for (size_t stride : kStrides) {
                    if (off < 0x04 * stride) continue;
                    const uintptr_t candidate = chunkBase + off - 0x04 * stride;
                    uint32_t chestRegion = 0;
                    if (!SafeReadU32(candidate + 0x02 * stride, &chestRegion) || chestRegion != chestId) continue;
                    const int flairSlot = GlobalsSlotFor(candidate, stride, flairHeadAId);
                    if (flairSlot < 20 || flairSlot > 40) continue;
                    int32_t remade = 0;
                    if (!SafeReadI32(candidate + 0x32 * stride, &remade) || remade < -1 || remade > 500) continue;
                    s_globalsStride.store(stride);
                    s_globalsBase.store(candidate);
                    if (!s_sceneReady.exchange(true)) {
                        NOBLE_INFO("ReachArmory: Found imported mainmenu GLOBALS table at 0x{:X} (stride={}, flair slot {})",
                                   candidate, stride, flairSlot);
                    }
                    return true;
                }
            }
        }
    }
    return false;
}

static void RememberRegion(uintptr_t addr) {
    if (AddrInStack(ThreadStackRanges(1000), addr)) return;

    ArmoryLock lock(s_dataMutex);
    if (s_recordRegions.size() >= 16) return;
    MEMORY_BASIC_INFORMATION mbi{};
    if (VirtualQuery(reinterpret_cast<LPCVOID>(addr), &mbi, sizeof(mbi)) == 0) return;
    const auto region = std::make_pair(reinterpret_cast<uintptr_t>(mbi.BaseAddress), static_cast<size_t>(mbi.RegionSize));
    if (region.second > 128u * 1024u * 1024u) return;   // tag data blocks are huge; only the object memory is worth re-reading
    if (std::find(s_recordRegions.begin(), s_recordRegions.end(), region) == s_recordRegions.end()) s_recordRegions.push_back(region);
}

// The remembered regions, cut like ReadWriteChunks (only ones still committed read/write).
static std::vector<std::pair<uintptr_t, size_t>> RecordRegionChunks() {
    constexpr size_t kChunk = 16u * 1024u * 1024u;
    constexpr size_t kOverlap = 0x1000;
    std::vector<std::pair<uintptr_t, size_t>> regions;
    {
        ArmoryLock lock(s_dataMutex);
        regions = s_recordRegions;
    }
    const StackRanges stacks = ThreadStackRanges(1000);

    std::vector<std::pair<uintptr_t, size_t>> chunks;
    for (const auto& r : regions) {
        MEMORY_BASIC_INFORMATION mbi{};
        if (VirtualQuery(reinterpret_cast<LPCVOID>(r.first), &mbi, sizeof(mbi)) == 0) continue;
        if (mbi.State != MEM_COMMIT || mbi.Protect != PAGE_READWRITE) continue;
        const uintptr_t base = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
        const size_t size = std::min(r.second, static_cast<size_t>(mbi.RegionSize));
        if (RangeOverlapsStack(stacks, base, base + size)) continue;
        for (size_t off = 0; off < size; off += kChunk) {
            const size_t len = std::min(size - off, kChunk + kOverlap);
            chunks.emplace_back(r.first + off, len);
            if (off + len >= size) break;
        }
    }
    return chunks;
}

static size_t ScanColorRecordsIn(const std::vector<std::pair<uintptr_t, size_t>>& chunks, bool quiet);
static size_t ScanEliteColorRecordsIn(const std::vector<std::pair<uintptr_t, size_t>>& chunks, bool quiet);
// Kept for reference/diagnostics; no longer called since Noble stopped painting the undersuit (2026-11-10).
[[maybe_unused]] static size_t ScanSuitRecordsIn(const std::vector<std::pair<uintptr_t, size_t>>& chunks, bool quiet);

static void ScanColorRecords() { ScanColorRecordsIn(ReadWriteChunks(0x40), false); }
static void ScanEliteColorRecords() { ScanEliteColorRecordsIn(ReadWriteChunks(0x40), false); }
[[maybe_unused]] static void ScanSuitRecords() { ScanSuitRecordsIn(ReadWriteChunks(0x40), false); }

// After the biped is remade: look for its new colour / suit records in the remembered regions every few ms
// (up to ~1.5 s) so they get Noble's colours before the old full scan would have found them.
static void StartQuickRescan() {
    if (s_quickScanning.exchange(true)) return;
    std::thread([]() {
      try {
        size_t colours = 0, eliteColours = 0, suits = 0;
        {
            ArmoryLock lock(s_dataMutex);
            colours = s_colorRecords.size();
            eliteColours = s_eliteColorRecords.size();
            suits = s_suitRecords.size();
        }
        const auto start = std::chrono::steady_clock::now();
        for (int pass = 0; pass < 300 && s_menuMode.load(); ++pass) {
            auto chunks = RecordRegionChunks();
            const bool isElite = ReachArmory::GetSelection().previewElite;

            // Full scan only when nothing is remembered yet, or for the Elite until its records were found once,
            // and never more than once per kFullRescanIntervalMs (it used to run on every switch to the Elite).
            if ((chunks.empty() || (isElite && !s_eliteRecordsSeen.load() && pass == 0)) && FullRescanAllowedNow()) {
                chunks = ReadWriteChunks(0x40);
            }
            if (chunks.empty()) break;
            // Suit shader records are no longer scanned: Noble does not paint the undersuit any more.
            if (!isElite) {
                ScanColorRecordsIn(chunks, true);
            } else {
                ScanEliteColorRecordsIn(chunks, true);
            }


            size_t c = 0, ec = 0, sr = 0;
            {
                ArmoryLock lock(s_dataMutex);
                c = s_colorRecords.size();
                ec = s_eliteColorRecords.size();
                sr = s_suitRecords.size();
            }

            if (ec > 0) s_eliteRecordsSeen.store(true);
            bool foundReady = false;
            if (isElite) {
                foundReady = (ec > eliteColours || eliteColours == 0) && (ec > 0);
            } else {
                foundReady = (c > colours || colours == 0) && (c > 0);
            }
            if (foundReady) {
                const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
                NOBLE_INFO("ReachArmory: rebuilt {} biped's colour records found in {} ms ({} region chunk(s))", isElite ? "Elite" : "Spartan", ms, chunks.size());
                break;
            }
            colours = std::min(colours, c);
            eliteColours = std::min(eliteColours, ec);
            suits = std::min(suits, sr);
            if (std::chrono::steady_clock::now() - start > std::chrono::milliseconds(1500)) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(4));
        }
      } catch (const std::exception& e) {
        NOBLE_ERROR("ReachArmory: quick colour rescan stopped: {}", e.what());
      }
        s_quickScanning.store(false);
    }).detach();
}

static bool MatchEliteColorPattern(const uint32_t cur[4], uint32_t lastPri, uint32_t lastSec) {
    if ((cur[0] >> 24) != 0xFFu || (cur[1] >> 24) != 0xFFu ||
        (cur[2] >> 24) != 0xFFu || (cur[3] >> 24) != 0xFFu) {
        return false;
    }

    // Reach 11 stock Elite rank preset ARGB palettes (from host-probe.exe scan_elite @ 0x1407B6658)
    static constexpr uint32_t kStockElitePatterns[11][4] = {
        { 0xFF231831u, 0xFF2B122Cu, 0xFFEF5046u, 0xFF4D2947u }, // 00
        { 0xFF1F3E72u, 0xFF222D45u, 0xFFD1660Du, 0xFF5167A9u }, // 01 (Minor Blue)
        { 0xFFBDE3D6u, 0xFF97B5AAu, 0xFFFF663Bu, 0xFF9EB3ADu }, // 02 (Ultra)
        { 0xFFF69300u, 0xFFFFD400u, 0xFF1BD687u, 0xFFF47500u }, // 03 (Zealot)
        { 0xFF361224u, 0xFF2F1434u, 0xFFFF663Bu, 0xFF47162Fu }, // 04 (Spec Ops)
        { 0xFFEFEFEFu, 0xFF837F6Bu, 0xFFCB5140u, 0xFFC1F1E8u }, // 05
        { 0xFF361124u, 0xFF141414u, 0xFFCD915Eu, 0xFFABA8B4u }, // 06
        { 0xFF5E1313u, 0xFF953200u, 0xFF9DB02Cu, 0xFFBC1C17u }, // 07 (Officer)
        { 0xFF970000u, 0xFF953200u, 0xFF9DB02Cu, 0xFFBC1C17u }, // 08 (Field Marshal)
        { 0xFF1D161Fu, 0xFF161018u, 0xFF2C684Eu, 0xFF8F2D25u }, // 09 (Honor Guard)
        { 0xFF753904u, 0xFF741F04u, 0xFF440D0Du, 0xFFE9ED1Au }  // 10 (General)
    };

    auto chDiff = [](uint32_t a, uint32_t b) {
        int bd = std::abs(static_cast<int>(a & 0xFF) - static_cast<int>(b & 0xFF));
        int gd = std::abs(static_cast<int>((a >> 8) & 0xFF) - static_cast<int>((b >> 8) & 0xFF));
        int rd = std::abs(static_cast<int>((a >> 16) & 0xFF) - static_cast<int>((b >> 16) & 0xFF));
        return bd <= 8 && gd <= 8 && rd <= 8;
    };

    for (int p = 0; p < 11; ++p) {
        if (chDiff(cur[0], kStockElitePatterns[p][0]) &&
            chDiff(cur[1], kStockElitePatterns[p][1]) &&
            chDiff(cur[2], kStockElitePatterns[p][2]) &&
            chDiff(cur[3], kStockElitePatterns[p][3])) {
            return true;
        }
    }

    // Match previously applied custom colors: cur[0] or cur[3] is primary, cur[1] or cur[2] is secondary
    if (lastPri != 0 && (cur[0] == lastPri || cur[3] == lastPri)) {
        if (lastSec == 0 || cur[1] == lastSec || cur[2] == lastSec || cur[3] == lastSec) {
            return true;
        }
    }

    return false;
}

static bool MatchSpartanColorRecord(const uint32_t* cur, uint32_t lastPri, uint32_t lastSec, uint32_t lastVis, const uint32_t* prevColors, bool hasPrevColors) {
    if ((cur[0] >> 24) != 0xFFu || (cur[1] >> 24) != 0xFFu ||
        (cur[2] >> 24) != 0xFFu || (cur[3] >> 24) != 0xFFu) {
        return false;
    }
    if ((cur[0] & 0x00FFFFFFu) == 0 || (cur[1] & 0x00FFFFFFu) == 0 || (cur[3] & 0x00FFFFFFu) == 0) {
        return false;
    }

    auto chDiff = [](uint32_t a, uint32_t b) {
        if ((a >> 24) != 0xFFu || (b >> 24) != 0xFFu) return false;
        int bd = std::abs(static_cast<int>(a & 0xFF) - static_cast<int>(b & 0xFF));
        int gd = std::abs(static_cast<int>((a >> 8) & 0xFF) - static_cast<int>((b >> 8) & 0xFF));
        int rd = std::abs(static_cast<int>((a >> 16) & 0xFF) - static_cast<int>((b >> 16) & 0xFF));
        return bd <= 8 && gd <= 8 && rd <= 8;
    };

    // 1. Stock Reach spartan colors in mainmenu_all.map: { 0xFF585858, 0xFF595959, 0xFF2FA0CB, 0xFFC7A167 }
    static constexpr uint32_t kStockColors[4] = { 0xFF585858u, 0xFF595959u, 0xFF2FA0CBu, 0xFFC7A167u };
    if (chDiff(cur[0], kStockColors[0]) && chDiff(cur[1], kStockColors[1]) &&
        (chDiff(cur[2], kStockColors[2]) || chDiff(cur[3], kStockColors[3]))) {
        return true;
    }

    // 2. Tolerant match against previously applied color set
    if (hasPrevColors) {
        if (chDiff(cur[0], prevColors[0]) && chDiff(cur[1], prevColors[1]) &&
            (chDiff(cur[3], prevColors[3]) || chDiff(cur[2], prevColors[2]))) {
            return true;
        }
    }

    // 3. Match against known active Spartan primary/secondary/visor
    if (lastPri != 0 && chDiff(cur[0], lastPri)) {
        if (lastSec != 0 && chDiff(cur[1], lastSec)) {
            return true;
        }
        if (lastVis != 0 && chDiff(cur[3], lastVis)) {
            return true;
        }
    }

    return false;
}

static size_t ScanColorRecordsIn(const std::vector<std::pair<uintptr_t, size_t>>& chunks, bool quiet) {
    uint32_t prevColors[4] = {};
    uint32_t lastPri = 0, lastSec = 0, lastVis = 0;
    {
        ArmoryLock lock(s_dataMutex);
        std::memcpy(prevColors, s_lastColorWords.data(), sizeof(prevColors));
        lastPri = s_lastSpartanPriArgb;
        lastSec = s_lastSpartanSecArgb;
        lastVis = s_lastSpartanVisArgb;
    }
    const bool hasPrevColors = ((prevColors[0] >> 24) == 0xFFu &&
                                (prevColors[1] >> 24) == 0xFFu &&
                                (prevColors[2] >> 24) == 0xFFu &&
                                (prevColors[3] >> 24) == 0xFFu &&
                                (prevColors[0] & 0x00FFFFFFu) != 0);

    std::vector<uintptr_t> found;
    for (const auto& chunk : chunks) {
        const uintptr_t base = chunk.first;
        const size_t size = chunk.second;
        {
            std::vector<uint8_t> buf;
            if (!AllocScanBuffer(buf, size)) continue;
            const uintptr_t self = reinterpret_cast<uintptr_t>(buf.data());
            if (base < self + size && self < base + size) continue;
            if (SafeCopyMem(buf.data(), reinterpret_cast<const void*>(base), size)) {
                for (size_t off = 0; off + 16 <= size; off += 4) {
                    const uint32_t* cur = reinterpret_cast<const uint32_t*>(buf.data() + off);
                    if (MatchSpartanColorRecord(cur, lastPri, lastSec, lastVis, prevColors, hasPrevColors)) {
                        const uintptr_t recAddr = base + off;
                        if (recAddr >= 0x10000000ULL && recAddr <= 0x7FFFFFFF0000ULL) {
                            found.push_back(recAddr);
                            if (found.size() >= 16) break;
                        }
                    }
                }
            }
        }
        if (found.size() >= 16) break;
    }

    DropStackAddresses(found, "armor color");
    size_t added = 0;
    if (!found.empty()) {
        ArmoryLock lock(s_dataMutex);
        for (uintptr_t rec : found) {
            if (s_colorRecords.size() >= 16) break;
            if (std::find(s_colorRecords.begin(), s_colorRecords.end(), rec) == s_colorRecords.end()) {
                s_colorRecords.push_back(rec);
                RememberRegion(rec);
                ++added;
            }
        }
        if (!quiet || added) NOBLE_INFO("ReachArmory: Tracked {} live armor color record(s)", s_colorRecords.size());
    }
    return added;
}

static size_t ScanEliteColorRecordsIn(const std::vector<std::pair<uintptr_t, size_t>>& chunks, bool quiet) {
    uint32_t lastPri = 0, lastSec = 0;
    uint32_t prevColors[4] = {};
    {
        ArmoryLock lock(s_dataMutex);
        lastPri = s_lastElitePriArgb;
        lastSec = s_lastEliteSecArgb;
        std::memcpy(prevColors, s_lastColorWords.data(), sizeof(prevColors));
    }
    const bool hasPrevColors = ((prevColors[0] >> 24) == 0xFFu &&
                                (prevColors[1] >> 24) == 0xFFu &&
                                (prevColors[2] >> 24) == 0xFFu &&
                                (prevColors[3] >> 24) == 0xFFu &&
                                prevColors[0] != 0 && (prevColors[2] != 0 || prevColors[3] != 0));

    std::vector<uintptr_t> found;
    for (const auto& chunk : chunks) {
        const uintptr_t base = chunk.first;
        const size_t size = chunk.second;
        {
            std::vector<uint8_t> buf;
            if (!AllocScanBuffer(buf, size)) continue;
            const uintptr_t self = reinterpret_cast<uintptr_t>(buf.data());
            if (base < self + size && self < base + size) continue;
            if (SafeCopyMem(buf.data(), reinterpret_cast<const void*>(base), size)) {
                for (size_t off = 0; off + 16 <= size; off += 4) {
                    const uint32_t* cur = reinterpret_cast<const uint32_t*>(buf.data() + off);
                    if ((hasPrevColors && std::memcmp(cur, prevColors, 16) == 0) ||
                        MatchEliteColorPattern(cur, lastPri, lastSec)) {
                        const uintptr_t recAddr = base + off;
                        if (recAddr >= 0x10000000ULL && recAddr <= 0x7FFFFFFF0000ULL) {
                            found.push_back(recAddr);
                            if (found.size() >= 16) break;
                        }
                    }
                }
            }
        }
        if (found.size() >= 16) break;
    }

    DropStackAddresses(found, "Elite color");
    size_t added = 0;
    if (!found.empty()) {
        ArmoryLock lock(s_dataMutex);
        for (uintptr_t rec : found) {
            if (s_eliteColorRecords.size() >= 16) break;
            if (std::find(s_eliteColorRecords.begin(), s_eliteColorRecords.end(), rec) == s_eliteColorRecords.end()) {
                s_eliteColorRecords.push_back(rec);
                RememberRegion(rec);
                ++added;
            }
        }
        if (!quiet || added) NOBLE_INFO("ReachArmory: Tracked {} live Elite color record(s)", s_eliteColorRecords.size());
    }
    return added;
}

[[maybe_unused]] static size_t ScanSuitRecordsIn(const std::vector<std::pair<uintptr_t, size_t>>& chunks, bool quiet) {
    // Stock 16-byte suit float[4] in mainmenu_all.map: { 0.9960785f, 0.9921569f, 0.98823535f, 1.0f }
    static constexpr float kStockSuit[4] = { 0.9960785f, 0.9921569f, 0.98823535f, 1.0f };
    float prevSuit[4] = {};
    {
        ArmoryLock lock(s_dataMutex);
        std::memcpy(prevSuit, s_lastSuitFloats.data(), sizeof(prevSuit));
    }
    // Only match prevSuit if it is distinct from generic quaternions/vectors like (0,0,0,1)
    bool hasPrevSuit = false;
    if (std::isfinite(prevSuit[0]) && std::isfinite(prevSuit[1]) &&
        std::isfinite(prevSuit[2]) && std::isfinite(prevSuit[3]) &&
        std::abs(prevSuit[3] - 1.0f) < 0.01f) {
        if (prevSuit[0] > 0.02f || prevSuit[1] > 0.02f || prevSuit[2] > 0.02f) {
            hasPrevSuit = true;
        }
    }

    std::vector<uintptr_t> found;
    for (const auto& chunk : chunks) {
        const uintptr_t base = chunk.first;
        const size_t size = chunk.second;
        {
            std::vector<uint8_t> buf;
            if (!AllocScanBuffer(buf, size)) continue;
            const uintptr_t self = reinterpret_cast<uintptr_t>(buf.data());
            if (base < self + size && self < base + size) continue;
            if (SafeCopyMem(buf.data(), reinterpret_cast<const void*>(base), size)) {
                for (size_t off = 0; off + 16 <= size; off += 4) {
                    const float* f = reinterpret_cast<const float*>(buf.data() + off);
                    if (std::memcmp(f, kStockSuit, 16) == 0 ||
                        (hasPrevSuit && std::memcmp(f, prevSuit, 16) == 0)) {
                        const uintptr_t recAddr = base + off;
                        if (recAddr >= 0x10000000ULL && recAddr <= 0x7FFFFFFF0000ULL) {
                            found.push_back(recAddr);
                            if (found.size() >= 16) break;
                        }
                    }
                }
            }
        }
        if (found.size() >= 16) break;
    }

    DropStackAddresses(found, "suit shader");
    size_t added = 0;
    if (!found.empty()) {
        ArmoryLock lock(s_dataMutex);
        for (uintptr_t rec : found) {
            if (s_suitRecords.size() >= 16) break;
            if (std::find(s_suitRecords.begin(), s_suitRecords.end(), rec) == s_suitRecords.end()) {
                s_suitRecords.push_back(rec);
                RememberRegion(rec);
                ++added;
            }
        }
        if (!quiet || added) NOBLE_INFO("ReachArmory: Tracked {} live suit shader record(s)", s_suitRecords.size());
    }
    return added;
}

static void ScanEliteTable() {
    {
        ArmoryLock lock(s_dataMutex);
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
            std::vector<uint8_t> buf;
            if (!AllocScanBuffer(buf, size)) continue;
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
                        ArmoryLock lock(s_dataMutex);
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
static void UpdateTagShaderConstantsSEH(bool previewElite,
                                        const uint32_t words[4],
                                        const float accentRgb[3],
                                        const float undersuitRgb[3],
                                        const float visorRgb[3]) {
    HMODULE hReach = GetModuleHandleW(L"haloreach.dll");
    if (!hReach) return;
    const uintptr_t base = reinterpret_cast<uintptr_t>(hReach);
    __try {
        uint8_t* tagTable = *reinterpret_cast<uint8_t* const*>(base + 0xC1A600);
        if (!tagTable) return;
        const uint64_t* pointerBases = reinterpret_cast<const uint64_t*>(base + 0x4E39F20);
        auto unpackPtr = [&](uint32_t packed) -> uint8_t* {
            return packed ? reinterpret_cast<uint8_t*>(pointerBases[packed >> 28] + static_cast<uint64_t>(packed) * 4u) : nullptr;
        };

        const int32_t tagCount = std::clamp(*reinterpret_cast<const int32_t*>(tagTable + 0x3FFF8), 0, 0x8000);
        for (int32_t ti = 0; ti < tagCount; ++ti) {
            uint8_t* te = tagTable + static_cast<size_t>(ti) * 8u;
            const int16_t groupIdx = *reinterpret_cast<const int16_t*>(te + 0x00);
            const uint16_t salt = *reinterpret_cast<const uint16_t*>(te + 0x02);
            if (groupIdx < 0 || groupIdx >= 256 || salt == 0xFFFFu) continue;
            const uint32_t magic = *reinterpret_cast<const uint32_t*>(tagTable + 0x3FFFC + static_cast<size_t>(groupIdx) * 16u);
            if (magic != 0x726D7368u) continue; // 'rmsh'
            const uint32_t rmshPacked = *reinterpret_cast<const uint32_t*>(te + 0x04);
            uint8_t* rmsh = unpackPtr(rmshPacked);
            if (!rmsh) continue;
            const int32_t spCount = *reinterpret_cast<const int32_t*>(rmsh + 0x1C);
            const uint32_t spPacked = *reinterpret_cast<const uint32_t*>(rmsh + 0x20);
            if (spCount <= 0 || spCount > 8 || !spPacked) continue;
            uint8_t* spBase = unpackPtr(spPacked);
            if (!spBase) continue;
            for (int si = 0; si < spCount; ++si) {
                uint8_t* sp = spBase + static_cast<size_t>(si) * 0xAC;
                const int32_t f4Count = *reinterpret_cast<const int32_t*>(sp + 0x1C);
                const uint32_t f4Packed = *reinterpret_cast<const uint32_t*>(sp + 0x20);
                if (f4Count <= 0 || f4Count > 64 || !f4Packed) continue;
                float* f4 = reinterpret_cast<float*>(unpackPtr(f4Packed));
                if (!f4) continue;

                if (!previewElite) {
                    // 1) spartan_armor (26 float4s, f4[1][0] == 3.0f):
                    //    f4[22] (+0x160) = self_illum_color (Armor Lights on legs, back, and chest -> Colors > Accent)
                    if (accentRgb && f4Count == 26 && std::abs(f4[1 * 4 + 0] - 3.0f) < 0.1f && std::abs(f4[23 * 4 + 0] - 1.0f) < 0.1f) {
                        f4[22 * 4 + 0] = accentRgb[0] * 2.2f;
                        f4[22 * 4 + 1] = accentRgb[1] * 2.2f;
                        f4[22 * 4 + 2] = accentRgb[2] * 2.2f;
                        f4[22 * 4 + 3] = 1.0f;
                    }
                    // 2) spartan_rubber_suit (21 float4s, f4[1][0] == 18.0f):
                    //    f4[2] (+0x20) = albedo_color, f4[9] (+0x90) = material_color_0, f4[10] (+0xA0) = material_color_1
                    else if (undersuitRgb && f4Count == 21 && std::abs(f4[1 * 4 + 0] - 18.0f) < 0.1f) {
                        f4[2 * 4 + 0] = undersuitRgb[0];
                        f4[2 * 4 + 1] = undersuitRgb[1];
                        f4[2 * 4 + 2] = undersuitRgb[2];
                        f4[2 * 4 + 3] = 1.0f;
                        f4[9 * 4 + 0] = undersuitRgb[0] * 0.85f + 0.05f;
                        f4[9 * 4 + 1] = undersuitRgb[1] * 0.85f + 0.05f;
                        f4[9 * 4 + 2] = undersuitRgb[2] * 0.85f + 0.05f;
                        f4[9 * 4 + 3] = 1.0f;
                        f4[10 * 4 + 0] = undersuitRgb[0] * 0.45f + 0.02f;
                        f4[10 * 4 + 1] = undersuitRgb[1] * 0.45f + 0.02f;
                        f4[10 * 4 + 2] = undersuitRgb[2] * 0.45f + 0.02f;
                        f4[10 * 4 + 3] = 1.0f;
                    }
                    // 3) spartan_helmet_interior_visor (21 float4s, f4[1][0] == 3.0f, f4[8][0] == 1.0f):
                    //    f4[17] (+0x110) = visor specular/env tint, f4[6] (+0x60) & f4[7] (+0x70) = visor reflection tint
                    else if (visorRgb && f4Count == 21 && std::abs(f4[1 * 4 + 0] - 3.0f) < 0.1f && std::abs(f4[8 * 4 + 0] - 1.0f) < 0.1f) {
                        f4[6 * 4 + 0] = visorRgb[0];
                        f4[6 * 4 + 1] = visorRgb[1];
                        f4[6 * 4 + 2] = visorRgb[2];
                        f4[6 * 4 + 3] = 1.0f;
                        f4[7 * 4 + 0] = visorRgb[0];
                        f4[7 * 4 + 1] = visorRgb[1];
                        f4[7 * 4 + 2] = visorRgb[2];
                        f4[7 * 4 + 3] = 1.0f;
                        f4[17 * 4 + 0] = visorRgb[0];
                        f4[17 * 4 + 1] = visorRgb[1];
                        f4[17 * 4 + 2] = visorRgb[2];
                        f4[17 * 4 + 3] = 1.0f;
                    }
                    // 4) Secondary visor material (24 float4s, f4[1][0] == 10.0f):
                    //    f4[9] (+0x90) & f4[10] (+0xA0) = visor gold/reflection tint
                    else if (visorRgb && f4Count == 24 && std::abs(f4[1 * 4 + 0] - 10.0f) < 0.1f) {
                        f4[9 * 4 + 0] = visorRgb[0];
                        f4[9 * 4 + 1] = visorRgb[1];
                        f4[9 * 4 + 2] = visorRgb[2];
                        f4[9 * 4 + 3] = 1.0f;
                        f4[10 * 4 + 0] = visorRgb[0];
                        f4[10 * 4 + 1] = visorRgb[1];
                        f4[10 * 4 + 2] = visorRgb[2];
                        f4[10 * 4 + 3] = 1.0f;
                    }
                }
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
    (void)words;
}

static void SyncArmoryLiveBipedEngineTickSEH(bool previewElite,
                                             bool isFemale,
                                             int32_t remadeCounter,
                                             const float priRgb[3],
                                             const float secRgb[3],
                                             const float accRgb[3],
                                             const float litRgb[3],
                                             const float visRgb[3],
                                             int accentColorIndex,
                                             const uint32_t* permPairsFlat,
                                             size_t permPairCount,
                                             uint64_t permPairsGen) {
    HMODULE hReach = GetModuleHandleW(L"haloreach.dll");
    if (!hReach) return;
    const uintptr_t base = reinterpret_cast<uintptr_t>(hReach);
    __try {
        const uint32_t tlsIdx = *reinterpret_cast<const uint32_t*>(base + 0xC17B18);
        const uintptr_t tlsArray = *reinterpret_cast<const uintptr_t*>(__readgsqword(0x58) + static_cast<uintptr_t>(tlsIdx) * 8u);
        if (!tlsArray) return;
        uint8_t* objTable = *reinterpret_cast<uint8_t* const*>(tlsArray + 0x10);
        if (!objTable) return;

        using SetChangeColorFn = uint8_t(__fastcall*)(uint32_t, int32_t, const float*);
        using BipedSetGenderFn = void(__fastcall*)(uint32_t, uint8_t);
        using SetRegionPermutationFn = uint8_t(__fastcall*)(uint32_t, uint32_t, uint32_t, uint8_t);

        auto SetChangeColor = reinterpret_cast<SetChangeColorFn>(base + 0x473858);
        auto BipedSetGender = reinterpret_cast<BipedSetGenderFn>(base + 0x49BB84);
        auto SetRegionPermutation = reinterpret_cast<SetRegionPermutationFn>(base + 0x4729F4);

        const float slot0[3] = { priRgb[0], priRgb[1], priRgb[2] };
        const float slot1[3] = { secRgb[0], secRgb[1], secRgb[2] };
        // Change colour 2 is the third/detail ("accent") colour for both species. Matches take it from the
        // profile's +0xA0 accent colour (ReachHost ApplyArmoryAppearance, SyncMatchLiveBipeds); the Armory used
        // the hidden "lights" colour for the Spartan here, so the preview did not match the game (2026-11-10).
        (void)litRgb;
        const float slot2[3] = { accRgb[0], accRgb[1], accRgb[2] };
        const float slot3[3] = {
            previewElite ? (accentColorIndex != 0 ? accRgb[0] : priRgb[0]) : visRgb[0],
            previewElite ? (accentColorIndex != 0 ? accRgb[1] : priRgb[1]) : visRgb[1],
            previewElite ? (accentColorIndex != 0 ? accRgb[2] : priRgb[2]) : visRgb[2]
        };
        const float* const wantSlots[4] = { slot0, slot1, slot2, slot3 };

        const int32_t maxObjects = std::clamp(*reinterpret_cast<const int32_t*>(objTable + 0x44), 0, 2048);
        uint8_t* entries = *reinterpret_cast<uint8_t* const*>(objTable + 0x50);
        if (!entries) return;

        static uint32_t s_lastGenderBipeds[16] = {};
        static uint8_t s_lastGenderApplied = 0xFFu;
        static int32_t s_lastGenderRemade = -999999;
        static uint64_t s_lastPermGen = 0;
        static int32_t s_lastPermRemade = -999999;
        static int s_permSyncTicksRemaining = 0;

        const uint8_t wantFemale = isFemale ? 1u : 0u;
        const bool resetGenderCache = (s_lastGenderApplied != wantFemale || s_lastGenderRemade != remadeCounter);
        if (resetGenderCache) {
            std::memset(s_lastGenderBipeds, 0, sizeof(s_lastGenderBipeds));
            s_lastGenderApplied = wantFemale;
            s_lastGenderRemade = remadeCounter;
        }
        if (s_lastPermGen != permPairsGen || s_lastPermRemade != remadeCounter) {
            s_lastPermGen = permPairsGen;
            s_lastPermRemade = remadeCounter;
            s_permSyncTicksRemaining = 2;
        }
        const bool syncPermsThisTick = (s_permSyncTicksRemaining > 0);
        if (s_permSyncTicksRemaining > 0) --s_permSyncTicksRemaining;

        for (int32_t i = 0; i < maxObjects; ++i) {
            uint8_t* entry = entries + static_cast<size_t>(i) * 0x18;
            const uint16_t salt = *reinterpret_cast<const uint16_t*>(entry + 0x00);
            if (salt == 0) continue;
            const uint8_t objType = entry[0x04];
            if (objType != 0) continue; // 0 == biped
            uint8_t* objPtr = *reinterpret_cast<uint8_t* const*>(entry + 0x10);
            if (!objPtr) continue;
            const uint32_t objectDatum = (static_cast<uint32_t>(salt) << 16) | static_cast<uint32_t>(i);
            const uint16_t bipdIdx = *reinterpret_cast<const uint16_t*>(objPtr);
            const bool isEliteBiped = (bipdIdx == 0x7Eu);
            if (previewElite != isEliteBiped) continue;

            // Apply region permutations directly onto the active species' live bipeds
            if (syncPermsThisTick && permPairsFlat && permPairCount > 0) {
                for (size_t p = 0; p < permPairCount; ++p) {
                    const uint32_t regSid = permPairsFlat[p * 2 + 0];
                    const uint32_t permSid = permPairsFlat[p * 2 + 1];
                    if (regSid != 0) {
                        SetRegionPermutation(objectDatum, regSid, permSid, 1);
                    }
                }
            }

            // 1. Update live biped change_colors (base + active slots 0..3)
            const int16_t ccSize = *reinterpret_cast<const int16_t*>(objPtr + 0x188);
            const int16_t ccOffset = *reinterpret_cast<const int16_t*>(objPtr + 0x18A);
            if (ccSize >= 24 && ccOffset > 0 && ccOffset < 0x2000) {
                const int32_t slotCount = (ccSize / 12) / 2;
                float* ccBase = reinterpret_cast<float*>(objPtr + ccOffset);
                float* ccActive = ccBase + slotCount * 3;
                const int32_t nSlots = std::min<int32_t>(slotCount, 4);
                for (int32_t s = 0; s < nSlots; ++s) {
                    const float* w = wantSlots[s];
                    float* b = ccBase + s * 3;
                    float* a = ccActive + s * 3;
                    if (std::abs(b[0] - w[0]) > 1e-4f || std::abs(b[1] - w[1]) > 1e-4f || std::abs(b[2] - w[2]) > 1e-4f ||
                        std::abs(a[0] - w[0]) > 1e-4f || std::abs(a[1] - w[1]) > 1e-4f || std::abs(a[2] - w[2]) > 1e-4f) {
                        b[0] = w[0]; b[1] = w[1]; b[2] = w[2];
                        a[0] = w[0]; a[1] = w[1]; a[2] = w[2];
                        SetChangeColor(objectDatum, s, w);
                    }
                }
            }

            // 2. Update Male vs Female body type on Spartan bipeds
            if (!previewElite) {
                bool alreadySet = false;
                int freeSlot = -1;
                for (int k = 0; k < 16; ++k) {
                    if (s_lastGenderBipeds[k] == objectDatum) { alreadySet = true; break; }
                    if (s_lastGenderBipeds[k] == 0 && freeSlot < 0) freeSlot = k;
                }
                if (!alreadySet) {
                    if (freeSlot >= 0) s_lastGenderBipeds[freeSlot] = objectDatum;
                    BipedSetGender(objectDatum, wantFemale);
                }
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

static void WriteColorsAndSuit(const ReachArmorySelection& sel) {
    const uint32_t priArgb = LinearRgbToArgbU32(sel.primaryRgb);
    const uint32_t secArgb = LinearRgbToArgbU32(sel.secondaryRgb);
    const uint32_t terArgb = LinearRgbToArgbU32(sel.accentRgb);   // third/detail colour (was a fixed cyan)
    const uint32_t visArgb = (sel.visor == "default" || sel.visor.empty())
                           ? 0xFFC7A167u
                           : LinearRgbToArgbU32(sel.visorRgb);


    // Species-dependent 4-color layout:
    // Spartan (spartan_armor_template / spartan_visor_template):
    //   [0] = Primary base armor
    //   [1] = Secondary trim
    //   [2] = Accent detail
    //   [3] = Visor reflection tint (with 20 finishes)
    // Elite (elite_armor_template / elite_ultra_helmet):
    //   [0] = Primary base armor (priArgb)
    //   [1] = Secondary armor trim / markings (secArgb)
    //   [2] = Emissive armor lights (litArgb - glowing eyes, illumination cards, LEDs)
    //   [3] = Bodysuit / Techsuit / Accent detail or primary diffuse finish (terArgb / priArgb)
    // Species-dependent 4-color layout:
    // Spartan (spartan_armor_template / spartan_visor_template):
    //   [0] = Primary base armor
    //   [1] = Secondary trim
    //   [2] = Accent detail
    //   [3] = Visor reflection tint (with 20 finishes)
    // Elite (elite_armor_template / elite_ultra_helmet):
    //   [0] = Primary base armor (priArgb)
    //   [1] = Secondary armor trim / markings (secArgb)
    //   [2] = Undersuit / Bodysuit / Accent detail (terArgb)
    //   [3] = Primary diffuse albedo / base tint (terArgb if accent selected, else priArgb)
    // Emissive lights (eyes, illumination cards, LEDs) are driven by the float4 suit/illumination records below.
    const std::array<uint32_t, 4> words = sel.previewElite
        ? std::array<uint32_t, 4>{ priArgb, secArgb, terArgb, (sel.accentColorIndex != 0 ? terArgb : priArgb) }
        : std::array<uint32_t, 4>{ priArgb, secArgb, terArgb, visArgb };

    // Self-illumination / emissive suit shader float constant:
    // Spartan suit shader float4 (written to s_suitRecords)
    const float suitFloats[4] = {
        sel.lightsRgb[0] * 4.0f,
        sel.lightsRgb[1] * 4.0f,
        sel.lightsRgb[2] * 4.0f,
        1.0f
    };

    ArmoryLock lock(s_dataMutex);
    s_lastColorWords = words;
    s_lastElitePriArgb = priArgb;
    s_lastEliteSecArgb = secArgb;
    s_lastSpartanPriArgb = priArgb;
    s_lastSpartanSecArgb = secArgb;
    s_lastSpartanVisArgb = visArgb;

    if (!sel.previewElite) {
        s_lastSuitFloats = { sel.lightsRgb[0], sel.lightsRgb[1], sel.lightsRgb[2], 1.0f };
    } else {
        s_lastSuitFloats = { suitFloats[0], suitFloats[1], suitFloats[2], suitFloats[3] };
    }
    const float visFloats[3] = {
        (sel.visor == "default" || sel.visor.empty()) ? 0.78f : sel.visorRgb[0],
        (sel.visor == "default" || sel.visor.empty()) ? 0.63f : sel.visorRgb[1],
        (sel.visor == "default" || sel.visor.empty()) ? 0.40f : sel.visorRgb[2]
    };

    static std::array<uint32_t, 4> s_prevWords = {};
    static float s_prevVisFloats[3] = { -1.f, -1.f, -1.f };
    static float s_prevAccentRgb[3] = { -1.f, -1.f, -1.f };
    static float s_prevLightsRgb[3] = { -1.f, -1.f, -1.f };
    static bool s_prevPreviewElite = false;
    static int32_t s_prevRemadeTagUpdate = -999999;
    const int32_t currentRemade = s_lastRemade.load();

    const bool colorsDirty = (words != s_prevWords) ||
                             (sel.previewElite != s_prevPreviewElite) ||
                             (currentRemade != s_prevRemadeTagUpdate) ||
                             (std::memcmp(visFloats, s_prevVisFloats, sizeof(visFloats)) != 0) ||
                             (std::memcmp(sel.accentRgb.data(), s_prevAccentRgb, sizeof(float) * 3) != 0) ||
                             (std::memcmp(sel.lightsRgb.data(), s_prevLightsRgb, sizeof(float) * 3) != 0);

    if (colorsDirty) {
        s_prevWords = words;
        s_prevPreviewElite = sel.previewElite;
        s_prevRemadeTagUpdate = currentRemade;
        std::memcpy(s_prevVisFloats, visFloats, sizeof(visFloats));
        std::memcpy(s_prevAccentRgb, sel.accentRgb.data(), sizeof(float) * 3);
        std::memcpy(s_prevLightsRgb, sel.lightsRgb.data(), sizeof(float) * 3);
        // undersuitRgb = nullptr: Reach never lets players colour the undersuit and the Armory has no control for
        // it; Noble used to paint it with the hidden "lights" colour, which made it change (2026-11-10).
        UpdateTagShaderConstantsSEH(sel.previewElite, words.data(), sel.accentRgb.data(), nullptr, visFloats);
    }

    {
        // Never write into a thread stack, even if a stale record address now points into one.
        const StackRanges stacks = ThreadStackRanges(1000);
        const auto inStack = [&](uintptr_t a) { return AddrInStack(stacks, a); };
        s_colorRecords.erase(std::remove_if(s_colorRecords.begin(), s_colorRecords.end(), inStack), s_colorRecords.end());
        s_eliteColorRecords.erase(std::remove_if(s_eliteColorRecords.begin(), s_eliteColorRecords.end(), inStack),
                                  s_eliteColorRecords.end());
        s_suitRecords.erase(std::remove_if(s_suitRecords.begin(), s_suitRecords.end(), inStack), s_suitRecords.end());
    }

    if (!sel.previewElite) {
        s_colorRecords.erase(
            std::remove_if(s_colorRecords.begin(), s_colorRecords.end(), [&](uintptr_t rec) {
                uint32_t cur[4] = {};
                if (!SafeCopyMem(cur, reinterpret_cast<const void*>(rec), sizeof(cur))) return true;
                if ((cur[0] >> 24) == 0xFFu && (cur[1] >> 24) == 0xFFu &&
                    (cur[2] >> 24) == 0xFFu && (cur[3] >> 24) == 0xFFu) {
                    SafeCopyMem(reinterpret_cast<void*>(rec), words.data(), sizeof(uint32_t) * 4);
                }
                return false;
            }),
            s_colorRecords.end()
        );
    } else {
        s_eliteColorRecords.erase(
            std::remove_if(s_eliteColorRecords.begin(), s_eliteColorRecords.end(), [&](uintptr_t rec) {
                uint32_t cur[4] = {};
                if (!SafeCopyMem(cur, reinterpret_cast<const void*>(rec), sizeof(cur))) return true;
                if ((cur[0] >> 24) == 0xFFu && (cur[1] >> 24) == 0xFFu &&
                    (cur[2] >> 24) == 0xFFu && (cur[3] >> 24) == 0xFFu) {
                    SafeCopyMem(reinterpret_cast<void*>(rec), words.data(), sizeof(uint32_t) * 4);
                }
                return false;
            }),
            s_eliteColorRecords.end()
        );
    }

    // Suit shader records (undersuit / suit float4s) are no longer written. They were found by a byte pattern that
    // does not tell Spartan and Elite apart; the Elite path wrote its lights x4 into every one, and the Spartan path
    // also wrote 0x70/0x80 bytes past each match. That changed the Spartan's undersuit after an Elite visit and was
    // a blind write into memory Noble had not verified. The undersuit stays Reach's own.
    s_suitRecords.clear();
}

// -----------------------------------------------------------------------------
// Apply Scene, Rotation, Armor Permutations & Colors via GLOBALS Table
// -----------------------------------------------------------------------------
// "-armory-settle-rebuild": rebuild the biped a second time after each change (test switch; off by default).
static bool SingleRebuild() {
    static const bool settle = std::wcsstr(GetCommandLineW(), L"-armory-settle-rebuild") != nullptr;
    return !settle;
}

// -----------------------------------------------------------------------------
// Armory item -> model regions. An Armory item (e.g. helmet "kurt_a") isn't a permutation of the Spartan model:
// Reach's customization globals (pmcg) turn it into one or more (region, permutation) changes, e.g. kurt_a =
// head:player_kurt + flair_head_a:kat + flair_head_b:player_adrianna. The menu script takes (region, permutation)
// pairs, so the items are translated with that table, read from data\armor\*.rrpmcg (pmcg dumped from the
// armour pack; not in git). Lines: "L <list offset hex> <count>", "S <item>", "C <region> <permutation> <flags>".
// -----------------------------------------------------------------------------
struct PmcgChange {
    std::string region;
    std::string permutation;
    int flags = 0;   // arms: 1 = male body, 2 = female body
};
static std::unordered_map<std::string, std::vector<PmcgChange>> s_pmcg;   // "<list>:<item>" -> changes
static bool s_pmcgLoaded = false;

static void EnsurePmcgLoaded() {
    ArmoryLock lock(s_dataMutex);
    if (s_pmcgLoaded) return;
    s_pmcgLoaded = true;
    std::vector<std::filesystem::path> dirs;
    if (!s_dataDir.empty()) {
        dirs.push_back(s_dataDir / "armor");
        dirs.push_back(s_dataDir / "menumaps");
    }
    wchar_t exeBuf[MAX_PATH] = {};
    if (GetModuleFileNameW(nullptr, exeBuf, MAX_PATH)) {
        const std::filesystem::path exeDir = std::filesystem::path(exeBuf).parent_path();
        dirs.push_back(exeDir / "data" / "armor");
        dirs.push_back(exeDir / "data" / "menumaps");
    }
    for (const auto& dir : dirs) {
        std::error_code ec;
        if (!std::filesystem::is_directory(dir, ec)) continue;
        for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
            if (entry.path().extension() != ".rrpmcg") continue;
            std::ifstream in(entry.path());
            std::string line, list, item;
            size_t items = 0;
            while (std::getline(in, line)) {
                if (!line.empty() && line.back() == '\r') line.pop_back();
                char kind = line.empty() ? 0 : line[0];
                std::vector<std::string> f;
                {
                    size_t i = 0;
                    while (i < line.size()) {
                        while (i < line.size() && line[i] == ' ') ++i;
                        size_t j = i;
                        while (j < line.size() && line[j] != ' ') ++j;
                        if (j > i) f.push_back(line.substr(i, j - i));
                        i = j;
                    }
                }
                if (kind == 'L' && f.size() >= 2) { list = f[1]; }
                else if (kind == 'S' && f.size() >= 2) { item = list + ":" + f[1]; s_pmcg[item]; ++items; }
                else if (kind == 'C' && f.size() >= 3 && !item.empty()) {
                    s_pmcg[item].push_back({ f[1], f[2], f.size() >= 4 ? std::atoi(f[3].c_str()) : 0 });
                }
            }
            NOBLE_INFO("ReachArmory: {} Armory items -> model regions from '{}'", items, entry.path().string());
            return;
        }
    }
    size_t embPmcgSize = 0;
    if (const uint8_t* embPmcg = EmbeddedBlocks::GetBlock("spartan.rrpmcg", &embPmcgSize); embPmcg && embPmcgSize > 0) {
        std::stringstream in{ std::string(reinterpret_cast<const char*>(embPmcg), embPmcgSize) };
        std::string line, list, item;
        size_t items = 0;
        while (std::getline(in, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            char kind = line.empty() ? 0 : line[0];
            std::vector<std::string> f;
            size_t i = 0;
            while (i < line.size()) {
                while (i < line.size() && line[i] == ' ') ++i;
                size_t j = i;
                while (j < line.size() && line[j] != ' ') ++j;
                if (j > i) f.push_back(line.substr(i, j - i));
                i = j;
            }
            if (kind == 'L' && f.size() >= 2) { list = f[1]; }
            else if (kind == 'S' && f.size() >= 2) { item = list + ":" + f[1]; s_pmcg[item]; ++items; }
            else if (kind == 'C' && f.size() >= 3 && !item.empty()) {
                s_pmcg[item].push_back({ f[1], f[2], f.size() >= 4 ? std::atoi(f[3].c_str()) : 0 });
            }
        }
        NOBLE_INFO("ReachArmory: {} Armory items -> model regions from embedded 'spartan.rrpmcg'", items);
        return;
    }
    NOBLE_WARN("ReachArmory: no data\\armor\\*.rrpmcg; Armory items go to the 3D model by name only (attachments won't show)");
}

// The changes for an Armory item in a pmcg list ("20" helmets, "2c"/"38" shoulders, "44" chest, "50" wrist,
// "5c" utility, "68" knees, "80" Spartan effects). nullptr = not in the table.
static const std::vector<PmcgChange>* PmcgItem(const char* list, const std::string& item) {
    EnsurePmcgLoaded();
    ArmoryLock lock(s_dataMutex);
    auto it = s_pmcg.find(std::string(list) + ":" + item);
    return it == s_pmcg.end() ? nullptr : &it->second;
}

// Armory effect ids (kArmorEffects) -> pmcg names.
static std::string PmcgEffectName(const std::string& id) {
    if (id == "default" || id.empty()) return "default_armor";
    if (id == "flames") return "legendary_flame";
    if (id == "blue_flames") return "eternal_flame";
    if (id == "birthday_party") return "halo_bday";
    if (id == "inclement_weather") return "storm";
    return id;   // heart_attack, pestilence
}

static const char* PermutationForArmorEffect(const std::string& id) {
    if (id == "flames") return "armor_fx_00";
    if (id == "blue_flames") return "armor_fx_01";
    if (id == "heart_attack") return "armor_fx_02";
    if (id == "pestilence") return "armor_fx_03";
    if (id == "inclement_weather") return "armor_fx_04";
    if (id == "birthday_party") return "armor_fx_05";
    return nullptr;
}

static void ApplyGlobalsAndPermutations(int32_t targetSceneIdx, const std::string& targetSceneName) {
    const uintptr_t base = s_globalsBase.load();
    const size_t stride  = s_globalsStride.load();
    if (base == 0 || stride == 0) return;

    const ReachArmorySelection sel = ReachArmory::GetSelection();
    // The Elite pedestal (scene 0x0B, shows_elite) belongs to the Armory only. Outside the Armory the menu's own
    // scene and the Spartan line-up are used, otherwise the Armory's Elite (and its camera) stayed on screen behind
    // every other menu after leaving the Armory.
    // Whole-body Armory cameras (0x0A..0x14) and the per-slot close-ups ("c_helmet", "c_chest", ...), which
    // live outside that index range: the pedestal turn / model rotation has to work in all of them, so the
    // character can be turned around whatever the Armory is zoomed in on.
    const bool closeUpScene = targetSceneName.rfind("c_", 0) == 0 && targetSceneName != "c_full_spartan";
    const bool armoryScene = (targetSceneIdx >= 0x0A && targetSceneIdx <= 0x14 && targetSceneIdx != 0x0E) || closeUpScene;
    const bool showElite = sel.previewElite && armoryScene;

    // 1. Camera Scene Index at GLOBALS[0x24] and shows_elite at GLOBALS[0x26]
    int32_t curScene = -1;
    SafeReadI32(base + 0x24 * stride, &curScene);
    static std::atomic<int32_t> s_observedScene{-2};
    if (curScene != s_observedScene.exchange(curScene)) {
        NOBLE_INFO("ReachArmory: GLOBALS camera scene is now 0x{:X}", curScene);
    }
    uint32_t curElite = 0;
    SafeReadU32(base + 0x26 * stride, &curElite);

    // The menu map's opening camera is the one the player sees before the GLOBALS scan succeeds. It is
    // NOT necessarily 0x00: writing 0x00 about four seconds in is exactly the "main menu jumps to a
    // sliced/zoomed-out cliff" glitch. Remember whatever scene the map started on and restore that for
    // Main/start_menu instead of forcing a title camera.
    if (s_mainScene.load() < 0 && curScene >= 0) {
        s_mainScene.store(curScene);
        NOBLE_INFO("ReachArmory: main menu opening camera scene is 0x{:X}", curScene);
    }
    const bool mainScene = targetSceneName == "main" || targetSceneName == "start_menu" || targetSceneName == "title";
    const int32_t mainSceneIdx = s_mainScene.load();
    int32_t effectiveSceneIdx = showElite ? 0x0B : targetSceneIdx;
    if (mainScene && mainSceneIdx >= 0) effectiveSceneIdx = mainSceneIdx;
    // The Elite camera (0x0B) belongs to the Elite on screen. The Armory points its camera at the species the player
    // just pressed straight away, while the species itself waits (kSpeciesSettleMs) - so with the Spartan still shown,
    // the camera jumped to the Elite pedestal, where the Elite character still stands, and back: 1-2 frames of Elite
    // between Spartan frames (Tyler's 21:43 video; 112 "0xB (shows_elite=0)" switches in one session). Keep the camera
    // where it is (or the Spartan's full-body view) until the Elite is really shown.
    if (!showElite && effectiveSceneIdx == 0x0B) {
        effectiveSceneIdx = (curScene >= 0 && curScene != 0x0B) ? curScene : 0x0C;
    }

    const uint32_t wantElite = showElite ? 1u : 0u;

    // Species switch wait (see kSpeciesSettleMs). Only inside the Armory: leaving it switches back at once.
    // rebuildBusy mirrors rebuildInFlight below (pairs written / settle pending / quiet frames / REMADE not yet moved).
    const uint64_t frameEarly = s_frameCounter.load();
    int32_t remadeEarly = 0;
    SafeReadI32(base + 0x32 * stride, &remadeEarly);
    // rr_armory copies rr_seq into rr_done once it has applied the pairs: that, not only REMADE (which now moves only
    // when the script re-creates the character for an effect change), says a rebuild has been processed.
    int32_t seqEarly = 0, doneEarly = 0;
    SafeReadI32(base + kRrSeq * stride, &seqEarly);
    SafeReadI32(base + kRrDone * stride, &doneEarly);
    const bool scriptCaughtUp = (seqEarly == doneEarly);
    const bool rebuildBusy = (s_bumpAtFrame != 0) || s_settlePending || (frameEarly < s_rebuildReadyFrame) ||
                             (s_remadeAtBump != 0 && remadeEarly == s_remadeAtBump && !scriptCaughtUp &&
                              frameEarly < s_settleDeadline);
    const uint64_t nowMs = GetTickCount64();
    const bool speciesFlipWanted = armoryScene && (curElite != wantElite);
    const bool speciesDeferred = speciesFlipWanted &&
                                 (nowMs - s_speciesChangeMs.load() < kSpeciesSettleMs || rebuildBusy);
    s_speciesDeferred.store(speciesDeferred);
    if (speciesDeferred) {
        if (s_speciesWaitStartMs == 0) s_speciesWaitStartMs = nowMs;
    } else if (s_speciesWaitStartMs != 0) {
        if (speciesFlipWanted) {
            NOBLE_INFO("ReachArmory: species switch to {} applied after a {} ms wait", wantElite ? "Elite" : "Spartan",
                       nowMs - s_speciesWaitStartMs);
        }
        s_speciesWaitStartMs = 0;
    }

    if (!speciesDeferred && (curScene != effectiveSceneIdx || curElite != wantElite)) {
        const bool speciesChanged = (curElite != wantElite);
        SafeWriteU32(base + 0x26 * stride, wantElite);
        SafeWriteU32(base + 0x24 * stride, static_cast<uint32_t>(effectiveSceneIdx));
        if (s_lastScene.exchange(effectiveSceneIdx) != effectiveSceneIdx) {
            NOBLE_INFO("ReachArmory: Switched GLOBALS camera scene to 0x{:X} (shows_elite={})", effectiveSceneIdx, wantElite);
        }
        if (speciesChanged) {
            ArmoryLock lock(s_dataMutex);
            s_colorRecords.clear();
            s_eliteColorRecords.clear();
            s_suitRecords.clear();
            // The character of this species is still in the scene: compare the next pairs with what it was
            // last built with, so an unchanged look is not rebuilt. Its live pairs are re-applied for a couple
            // of ticks (NotifyEngineTick) in case the map script re-created it on the flip.
            const int sp = wantElite ? 1 : 0;
            s_lastPairs = s_builtPairs[sp];
            s_lastPairsBase = s_builtBase[sp];
            s_livePairs = s_builtLivePairs[sp];
            s_livePairsGen.fetch_add(1);
            s_rotationBaseBits.store(0xFFFFFFFFu);
            s_recordRetriesLeft.store(kRecordRetryBudget);
            StartQuickRescan();
        }
    }

    // Everything below moves/rebuilds the Armory pedestal and its biped. Applying it while the Main
    // menu is up made the engine re-instantiate the model a few seconds after startup, which also
    // reset the menu camera. Those writes are only needed while an Armory scene is actually shown.
    if (armoryScene) {
    // 2. Pedestal Turn Rotation at GLOBALS[0x27] (0..71 steps of 5 degrees) + haloreach.dll + 0xBE5AD8
    constexpr float kPi = 3.14159265358979323846f;
    constexpr float kTwoPi = 2.0f * kPi;
    float yawRad = std::fmod(s_yawOffset.load(), kTwoPi);
    if (yawRad < 0.0f) yawRad += kTwoPi;

    const float yawDeg = yawRad * (180.0f / kPi);
    const int32_t turnStep = ((static_cast<int32_t>(std::lround(yawDeg / 5.0f)) % 72) + 72) % 72;
    // Written every frame, not only when it changes: the menu map's own armory script re-writes these globals
    // during its tick, and a value we set once was immediately overwritten again - which is why the character
    // could not be turned with the per-slot close-up cameras (it only twitched by one frame's delta).
    if (s_lastTurnStep.exchange(turnStep) != turnStep) {
        NOBLE_DEBUG("ReachArmory: pedestal turn step {}", turnStep);
    }
    SafeWriteI32(base + 0x27 * stride, turnStep);

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
        float targetAngle = std::fmod(baseAngle + yawRad, kTwoPi);
        if (targetAngle > kPi) targetAngle -= kTwoPi;
        uint32_t targetBits = 0;
        std::memcpy(&targetBits, &targetAngle, sizeof(float));
        // Same as the pedestal: re-assert the model's yaw every frame so the script cannot snap it back while
        // a close-up camera is up. The cached bits only avoid the log/no-op bookkeeping.
        if (SafeWriteU32(rotAddr, targetBits)) {
            s_lastYawBits.store(targetBits);
        }
    }

    // 3. Check if mainmenu_all.map script finished rebuilding biped (REMADE counter at GLOBALS[0x32])
    int32_t remade = 0;
    if (SafeReadI32(base + 0x32 * stride, &remade)) {
        const int32_t prevRemade = s_lastRemade.exchange(remade);
        if (prevRemade != -1 && prevRemade != remade) {
            StartQuickRescan();
        }
    }
    const uint64_t frameNow = s_frameCounter.load();
    auto bumpRebuild = [&](const char* why) {
        // No longer invalidates rr_fxp (2026-11-10). mainmenu.hsc (decompiled with Assembly): on every rr_seq bump
        // rr_armory runs (object_set_variant "chief_armory" "default") - which clears all attachments - then applies
        // the 16 pairs and restarts the effect (rr_fx). It re-creates the character (object_create_anew, the DMR
        // throw / idle restart) only when the armour effect changed (rr_fxn != rr_fxp). Zeroing rr_fxp here forced
        // that re-creation on every Spartan piece change. The script still re-creates on a real effect change.
        (void)why;
        int32_t rebuildCounter = 0;
        SafeReadI32(base + 0x02 * stride, &rebuildCounter);
        SafeWriteI32(base + 0x02 * stride, rebuildCounter + 1);
        NOBLE_INFO("ReachArmory: rebuild {} ({})", rebuildCounter + 1, why);
    };
    // Step 2 of a change: the pairs went in a couple of frames ago; rebuild the biped now.
    if (s_bumpAtFrame != 0 && frameNow >= s_bumpAtFrame) {
        s_bumpAtFrame = 0;
        s_remadeAtBump = remade;
        bumpRebuild("new pieces");
        s_settlePending = !SingleRebuild();
        s_settleDeadline = frameNow + 45;
        s_rebuildReadyFrame = frameNow + kRebuildQuietFrames;
    } else if (s_settlePending && (remade != s_remadeAtBump || frameNow >= s_settleDeadline)) {
        // Step 3: once the biped has been remade, remake it once more so it shows the pieces just picked.
        s_settlePending = false;
        bumpRebuild("settle");
        s_rebuildReadyFrame = frameNow + kRebuildQuietFrames;
    }

    // 4. Write 16 (region_string_id, permutation_string_id) pairs to GLOBALS[0x04..0x23] and trigger rebuild.
    // One rebuild at a time: a request while the map script is still recreating the biped (see kRebuildQuietFrames,
    // the pending settle and the script's own REMADE counter) is what made rapid changes crash. The generation is
    // left unapplied in that case, so every change made meanwhile collapses into a single rebuild of the final look.
    const bool rebuildSettled = (s_remadeAtBump == 0) || (remade != s_remadeAtBump) || scriptCaughtUp ||
                                (frameNow >= s_settleDeadline);
    const bool rebuildInFlight = (s_bumpAtFrame != 0) || s_settlePending ||
                                 (frameNow < s_rebuildReadyFrame) || !rebuildSettled;
    const uint64_t gen = s_selectionGeneration.load();
    // While a species switch waits, nothing is applied: the pairs must go in together with the flip.
    if (gen != s_appliedGeneration && !rebuildInFlight && !speciesDeferred) {
        s_appliedGeneration = gen;
        s_previewDirty.store(false);

        std::vector<std::pair<uint32_t, uint32_t>> pairs;
        std::vector<std::pair<uint32_t, uint32_t>> livePairs;
        auto pushPair = [&](const std::string& regionName, const std::string& permName, const char* fallbackPerm = "default") {
            const uint32_t regId = LookupStringId(regionName);
            uint32_t permId = LookupPermutationId(permName);
            static std::vector<std::string> s_warned;   // each missing name is logged once
            if (permId == 0 && !permName.empty() &&
                std::find(s_warned.begin(), s_warned.end(), permName) == s_warned.end()) {
                s_warned.push_back(permName);
                NOBLE_WARN("ReachArmory: '{}' isn't in the menu map's string_ids; {} shows '{}'", permName, regionName,
                           fallbackPerm ? fallbackPerm : "nothing");
            }
            if (permId == 0 && fallbackPerm) {
                permId = LookupStringId(fallbackPerm);
            }
            if (regId != 0 && permId != 0) {
                pairs.emplace_back(regId, permId);
                livePairs.emplace_back(regId, permId);
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
            // Elite armor effects (pmcg list 80 / 8c, region armor_fx, permutations armor_fx_00..05)
            if (sel.eliteArmorEffect != "default" && !sel.eliteArmorEffect.empty()) {
                const std::string fxName = PmcgEffectName(sel.eliteArmorEffect);
                const std::vector<PmcgChange>* fxChanges = PmcgItem("80", fxName);
                if (!fxChanges || fxChanges->empty()) {
                    fxChanges = PmcgItem("8c", fxName);
                }
                if (fxChanges && !fxChanges->empty()) {
                    for (const auto& c : *fxChanges) pushPair(c.region, c.permutation, nullptr);
                } else {
                    const char* perm = PermutationForArmorEffect(sel.eliteArmorEffect);
                    if (perm) pushPair("armor_fx", perm, nullptr);
                }
            }
        } else if (PmcgItem("20", "default") != nullptr) {
            // Resolve each selected Armory item through the player_model_customization_globals (pmcg) table
            // into the actual Spartan hlmt/mode regions in mainmenu_all.map:
            //   head, body, flair_head_a..c, flair_left_shoulder, flair_right_shoulder,
            //   flair_chest_a..b, arms, flair_forearm, flair_thigh, flair_knees, armor_fx.
            static const char* const kRegions[] = { "head", "body", "flair_head_a", "flair_head_b", "flair_head_c",
                "flair_left_shoulder", "flair_right_shoulder", "flair_chest_a", "flair_chest_b", "arms",
                "flair_forearm", "flair_thigh", "flair_knees", "armor_fx" };
            std::unordered_map<std::string, std::string> perm;
            perm["body"] = sel.isFemale ? "female" : "default";
            perm["arms"] = sel.isFemale ? "female" : "default";
            auto apply = [&](const char* list, const std::string& item) {
                const std::vector<PmcgChange>* changes = PmcgItem(list, item);
                if (!changes) {
                    NOBLE_WARN("ReachArmory: Armory item '{}' isn't in pmcg list {}", item, list);
                    return;
                }
                for (const PmcgChange& c : *changes) {
                    // arms has a male (1) and a female (2) permutation for the prosthetic arm.
                    if (c.flags == 1 && sel.isFemale) continue;
                    if (c.flags == 2 && !sel.isFemale) continue;
                    perm[c.region] = c.permutation;
                }
            };
            apply("20", sel.helmet);
            apply("2c", sel.leftShoulder);
            apply("38", sel.rightShoulder);
            apply("44", sel.chest);
            // Prosthetic robotic arm: if the chest piece includes a cybernetic arm or the user has hands set to "robot",
            // resolve to the actual render model permutation ("kat" for female, "player_john117" for male).
            // Never assign the raw string "robot" to region "arms" (it causes crash 0xC0000005 at haloreach.dll+0x6950A9).
            if (sel.hands == "robot") {
                perm["arms"] = sel.isFemale ? "kat" : "player_john117";
            } else if (!sel.hands.empty() && sel.hands != "default" && sel.hands != "robot") {
                perm["arms"] = sel.hands;
            }
            apply("50", sel.wrist);
            apply("5c", sel.utility);
            apply("68", sel.knees);
            apply("80", PmcgEffectName(sel.armorEffect));
            for (const char* region : kRegions) {
                auto it = perm.find(region);
                if (it != perm.end()) {
                    pushPair(region, it->second, nullptr);
                } else {
                    // Regions the chosen items don't use are NOT sent. Tried 2026-11-10: filling them with "default"
                    // put visible parts on (for flair_head_* and flair_chest_b, "default" is a real attachment in the
                    // menu map's model - the visor strip, the chest knife). A region that disappears from the list
                    // makes the change a rebuild, which re-creates the character without it.
                    const uint32_t regId = LookupStringId(region);
                    if (regId != 0) livePairs.emplace_back(regId, 0u);
                }
            }
        } else {
            // No pmcg table: item names straight in (base helmets that share the model's name only).
            pushPair("head",           sel.helmet,        "default");
            pushPair("chest",          sel.chest,         "chest_blank");
            pushPair("left_shoulder",  sel.leftShoulder,  "left_shoulder_blank");
            pushPair("right_shoulder", sel.rightShoulder, "right_shoulder_blank");
            pushPair("arms",           (sel.hands == "robot") ? (sel.isFemale ? "kat" : "player_john117") : "default", "default");
            pushPair("wrist",          sel.wrist,         "wrist_blank");
            pushPair("utility",        sel.utility,       "utility_blank");
            pushPair("knees",          sel.knees,         "knees_default");
            pushPair("armor_effect",   sel.armorEffect,   "default");
        }

        // The imported RR mainmenu_all has a different GLOBALS layout: head at 4, chest at 2,
        // flair_head/chest at 24..34 and shoulders at 36/38. Resolve each region's own slot instead
        // of assuming the legacy 0x04, 0x06, ... layout.
        uint32_t m0 = 0, m1 = 0;
        const bool hasRrMarkers = SafeReadU32(base + kRrMarkA * stride, &m0) &&
                                  SafeReadU32(base + kRrMarkB * stride, &m1) &&
                                  m0 == kRrMarkAVal && m1 == kRrMarkBVal;
        const bool dynamicLayout = !hasRrMarkers && (GlobalsSlotFor(base, stride, LookupStringId("flair_head_a")) >= 20);
        std::vector<std::pair<uint32_t, uint32_t>> writtenPairs;
        std::vector<uint32_t> writtenSlots;
        if (!pairs.empty()) {
            if (dynamicLayout) {
                for (const auto& pair : pairs) {
                    int slot = GlobalsSlotFor(base, stride, pair.first);
                    if (slot < 0) {
                        const std::string region = NameOfStringId(pair.first);
                        if (region == "left_shoulder") slot = GlobalsSlotFor(base, stride, LookupStringId("flair_left_shoulder"));
                        else if (region == "right_shoulder") slot = GlobalsSlotFor(base, stride, LookupStringId("flair_right_shoulder"));
                        else if (region == "chest") slot = GlobalsSlotFor(base, stride, LookupStringId("flair_chest_a"));
                    }
                    if (slot < 0 || std::find(writtenSlots.begin(), writtenSlots.end(), static_cast<uint32_t>(slot)) != writtenSlots.end()) continue;
                    writtenSlots.push_back(static_cast<uint32_t>(slot));
                    writtenPairs.emplace_back(pair.first, pair.second);
                    SafeWriteU32(base + slot * stride, pair.first);
                    SafeWriteU32(base + (slot + 1) * stride, pair.second);
                }
            } else {
                // mainmenu_all.map rr_armory script loops i = 0..15 over GLOBALS[0x04 + i*2] (rr_r{i}) and
                // GLOBALS[0x05 + i*2] (rr_p{i}), calling (object_set_permutation chief_armory rr_r{i} rr_p{i})
                // for each pair and scanning rr_r0..rr_r15 for (= rr_r{i} "armor_fx").
                while (pairs.size() < 16) pairs.push_back(pairs[0]);
                writtenPairs = pairs;
                for (size_t i = 0; i < 16; ++i) {
                    SafeWriteU32(base + (0x04 + i * 2) * stride, writtenPairs[i].first);
                    SafeWriteU32(base + (0x05 + i * 2) * stride, writtenPairs[i].second);
                }
            }
            const bool piecesChanged = (writtenPairs != s_lastPairs || base != s_lastPairsBase);
            // Live piece change (2026-11-10). A rebuild re-creates the character, which restarts Reach's Armory idle
            // (the DMR throw) on every click. Male <-> female proved the live character takes new permutations without
            // being re-created: NotifyEngineTick applies the new pairs (SetRegionPermutation) for a couple of ticks,
            // and the engine's gender setter handles the body. So a change is applied live when the pairs cover the
            // same regions in the same order and only permutations differ. Still rebuilt: the first build of a
            // species, a change in which regions are used (a helmet adding or removing attachment regions, so no old
            // attachment can stay behind), and any armour effect change (the menu script only shows an effect on a
            // re-created character). Experiment: if pieces don't change on screen, narrow this back down.
            bool liveChange = false;
            if (piecesChanged && base == s_lastPairsBase && !writtenPairs.empty() && writtenPairs.size() == s_lastPairs.size()) {
                const uint32_t fxId = LookupStringId("armor_fx");
                // Safety net: an item that sets an attachment ("flair_*") region to "default" (the pmcg does that,
                // e.g. "flair_head_b default", "flair_knees default") is applied with a rebuild, not live - on the
                // live character a region set back to "default" kept the old part (Tyler's tests 22:19 / 22:29).
                // Non-attachment regions (head, body, arms) use "default" as a real permutation and stay live.
                const uint32_t defaultId = LookupStringId("default");
                static const char* const kFlairRegions[] = { "flair_head_a", "flair_head_b", "flair_head_c",
                    "flair_left_shoulder", "flair_right_shoulder", "flair_chest_a", "flair_chest_b",
                    "flair_forearm", "flair_thigh", "flair_knees" };
                auto isFlair = [&](uint32_t region) {
                    for (const char* name : kFlairRegions) {
                        if (region == LookupStringId(name)) return true;
                    }
                    return false;
                };
                bool anyDiff = false;
                bool needsRebuild = false;
                for (size_t i = 0; i < writtenPairs.size(); ++i) {
                    const uint32_t region = writtenPairs[i].first;
                    if (region != s_lastPairs[i].first || region == 0) { needsRebuild = true; break; }
                    if (writtenPairs[i].second == s_lastPairs[i].second) continue;
                    anyDiff = true;
                    if (fxId != 0 && region == fxId) needsRebuild = true;
                    if (defaultId != 0 && writtenPairs[i].second == defaultId && isFlair(region)) needsRebuild = true;   // attachment removed
                }
                liveChange = anyDiff && !needsRebuild;
            }
            if (liveChange) {
                {
                    ArmoryLock lock(s_dataMutex);
                    s_lastPairs = writtenPairs;
                    s_livePairs = livePairs;
                    s_livePairsGen.fetch_add(1);
                    const int sp = showElite ? 1 : 0;
                    s_builtPairs[sp] = writtenPairs;
                    s_builtLivePairs[sp] = livePairs;
                    s_builtBase[sp] = base;
                }
                if (showElite) {
                    NOBLE_INFO("ReachArmory: Elite pieces changed live (no rebuild): {} / {} / {} / {} / {} / {}", sel.eliteHelmet,
                               sel.eliteLeftShoulder, sel.eliteRightShoulder, sel.eliteChest, sel.eliteArms, sel.eliteLegs);
                } else {
                    NOBLE_INFO("ReachArmory: Spartan pieces changed live (no rebuild): head {}, chest {}, L {}, R {}, wrist {}, "
                               "utility {}, knees {}, body {}", sel.helmet, sel.chest, sel.leftShoulder, sel.rightShoulder,
                               sel.wrist, sel.utility, sel.knees, sel.isFemale ? "female" : "male");
                }
            } else if (!piecesChanged) {
                // Same pieces: no rebuild needed, colors and lights are written to live memory every frame.
            } else {
                {
                    ArmoryLock lock(s_dataMutex);
                    s_lastPairs = writtenPairs;
                    s_livePairs = livePairs;
                    s_livePairsGen.fetch_add(1);
                    const int sp = showElite ? 1 : 0;
                    s_builtPairs[sp] = writtenPairs;
                    s_builtLivePairs[sp] = livePairs;
                    s_builtBase[sp] = base;
                }
                s_lastPairsBase = base;
                s_recordRetriesLeft.store(kRecordRetryBudget);
                s_bumpAtFrame = frameNow + 2;
                s_settlePending = false;
                s_rebuildReadyFrame = frameNow + 2 + kRebuildQuietFrames;
                if (showElite) {
                    NOBLE_INFO("ReachArmory: Elite pieces {} / {} / {} / {} / {} / {}, effect {}", sel.eliteHelmet, sel.eliteLeftShoulder,
                               sel.eliteRightShoulder, sel.eliteChest, sel.eliteArms, sel.eliteLegs, sel.eliteArmorEffect);
                } else {
                    NOBLE_INFO("ReachArmory: Spartan pieces head {}, chest {}, L {}, R {}, wrist {}, utility {}, knees {}, effect {}",
                               sel.helmet, sel.chest, sel.leftShoulder, sel.rightShoulder, sel.wrist, sel.utility, sel.knees,
                               sel.armorEffect);
                }
            }
        }

        // Also update Elite hlmt permutation table directly if cached
        if (showElite) {
            const std::string* pieceNames[6] = {
                &sel.eliteHelmet, &sel.eliteChest, &sel.eliteLeftShoulder,
                &sel.eliteRightShoulder, &sel.eliteArms, &sel.eliteLegs
            };
            ArmoryLock lock(s_dataMutex);
            if (s_eliteScanned) {
                for (size_t r = 0; r < 6; ++r) {
                    const std::string& pieceName = !pieceNames[r]->empty() ? *pieceNames[r] : sel.eliteSuit;
                    const uint32_t permId = LookupStringId(pieceName);
                    if (permId != 0 && s_eliteRegions[r].permArrayAddr != 0) {
                        auto it = s_eliteRegions[r].permsByStringId.find(permId);
                        if (it == s_eliteRegions[r].permsByStringId.end()) {
                            const uint32_t suitId = LookupStringId(sel.eliteSuit);
                            it = s_eliteRegions[r].permsByStringId.find(suitId);
                        }
                        if (it != s_eliteRegions[r].permsByStringId.end()) {
                            SafeCopyMem(reinterpret_cast<void*>(s_eliteRegions[r].permArrayAddr),
                                        it->second.bytes, sizeof(ElitePermEntry));
                        }
                    }
                }
            }
        }
    }

    // 5. Write live colors and suit shader constants every frame - for the species actually on screen. During a
    // species wait the selection already names the other species; its colours must not go onto this character.
    if (!speciesDeferred) {
        {
            ArmoryLock lock(s_sceneMutex);
            s_shownSelection = sel;
            s_haveShownSelection = true;
        }
        WriteColorsAndSuit(sel);
    } else {
        ReachArmorySelection shown;
        bool haveShown = false;
        {
            ArmoryLock lock(s_sceneMutex);
            haveShown = s_haveShownSelection;
            if (haveShown) shown = s_shownSelection;
        }
        if (haveShown) WriteColorsAndSuit(shown);
    }
    }
}

} // namespace

namespace {
// The menu process resolves the Armory selection from the saved appearance (palette or wheel colours,
// visor, pieces); a match process never runs the Armory, so it learns the look from this file. Without it
// the in-game material sync used the DEFAULT selection, which is what painted every Spartan's visor amber
// (and the local suit/armour lights with default colours) no matter what the Armory had equipped.
std::filesystem::path SelectionFilePath() {
    return s_dataDir.empty() ? std::filesystem::path() : s_dataDir / "userdata" / "armory-selection.json";
}

void SaveSelectionFile(const ReachArmorySelection& sel) {
    const std::filesystem::path path = SelectionFilePath();
    if (path.empty()) return;
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    nlohmann::json j;
    j["visor"] = sel.visor;
    j["primary"] = sel.primaryRgb;
    j["secondary"] = sel.secondaryRgb;
    j["accent"] = sel.accentRgb;
    j["lights"] = sel.lightsRgb;
    j["visor_rgb"] = sel.visorRgb;
    j["custom"] = nlohmann::json::array({ sel.primaryCustom, sel.secondaryCustom, sel.accentCustom, sel.lightsCustom });
    j["primary_index"] = sel.primaryColorIndex;
    j["secondary_index"] = sel.secondaryColorIndex;
    j["accent_index"] = sel.accentColorIndex;
    j["lights_index"] = sel.lightsColorIndex;
    j["visor_index"] = sel.visorColorIndex;
    std::ofstream out(std::filesystem::path(path), std::ios::trunc | std::ios::binary);
    if (!out) return;
    out << j.dump();
}

void LoadSelectionFile(ReachArmorySelection& sel) {
    const std::filesystem::path path = SelectionFilePath();
    if (path.empty()) return;
    std::ifstream in(std::filesystem::path(path), std::ios::binary);
    if (!in) return;
    nlohmann::json j;
    try {
        in >> j;
        auto rgb = [&](const char* key, std::array<float, 3>& out) {
            if (!j.contains(key) || !j[key].is_array() || j[key].size() != 3) return;
            for (int i = 0; i < 3; ++i) out[i] = std::clamp(j[key][i].get<float>(), 0.0f, 4.0f);
        };
        rgb("primary", sel.primaryRgb);
        rgb("secondary", sel.secondaryRgb);
        rgb("accent", sel.accentRgb);
        rgb("lights", sel.lightsRgb);
        rgb("visor_rgb", sel.visorRgb);
        if (j.contains("visor") && j["visor"].is_string()) sel.visor = j["visor"].get<std::string>();
        if (j.contains("custom") && j["custom"].is_array() && j["custom"].size() == 4) {
            sel.primaryCustom = j["custom"][0].get<bool>();
            sel.secondaryCustom = j["custom"][1].get<bool>();
            sel.accentCustom = j["custom"][2].get<bool>();
            sel.lightsCustom = j["custom"][3].get<bool>();
        }
        if (j.contains("primary_index")) sel.primaryColorIndex = j["primary_index"].get<int>();
        if (j.contains("secondary_index")) sel.secondaryColorIndex = j["secondary_index"].get<int>();
        if (j.contains("accent_index")) sel.accentColorIndex = j["accent_index"].get<int>();
        if (j.contains("lights_index")) sel.lightsColorIndex = j["lights_index"].get<int>();
        if (j.contains("visor_index")) sel.visorColorIndex = j["visor_index"].get<int>();
        NOBLE_INFO("ReachArmory: loaded the saved Armory colours (visor '{}', custom {}{}{}{})", sel.visor,
                   sel.primaryCustom, sel.secondaryCustom, sel.accentCustom, sel.lightsCustom);
    } catch (const nlohmann::json::exception&) {
        // A truncated or hand-edited file must never take the game down: keep the defaults.
    }
}
} // namespace

void ReachArmory::Initialize(const std::filesystem::path& dataDir) {
    s_dataDir = dataDir;
    EnsureStringIdsLoaded();
    {
        ArmoryLock lock(s_sceneMutex);
        LoadSelectionFile(s_selection);
    }
}

bool ReachArmory::HasArmoryMap() {
    return HasCustomMenuMap() || !ArmoryMapPath().empty();
}

bool ReachArmory::HasCustomMenuMap() {
    if (s_dataDir.empty()) return false;
    std::error_code ec;
    const std::filesystem::path candidates[] = {
        s_dataDir / "menumaps" / "mainmenu_all.map",
        s_dataDir / "mainmenu_all.map",
    };
    for (const auto& path : candidates) {
        if (std::filesystem::is_regular_file(path, ec)) return true;
    }
    return false;
}

void ReachArmory::SetMenuMode(bool menuMode) {
    s_menuMode.store(menuMode);
    if (!menuMode) {
        s_sceneReady.store(false);
        s_globalsBase.store(0);
        s_lastScene.store(-1);
        s_mainScene.store(-1);
        s_lastRemade.store(-1);
        s_appliedGeneration = 0;
        s_rebuildReadyFrame = 0;
        s_lastPairs.clear();
        s_livePairs.clear();
        s_lastPairsBase = 0;
        s_bumpAtFrame = 0;
        s_settlePending = false;
        s_previewDirty.store(false);
        s_eliteRecordsSeen.store(false);
        s_lastFullRescanMs.store(0);
        s_speciesDeferred.store(false);
        s_speciesWaitStartMs = 0;
        for (int sp = 0; sp < 2; ++sp) {
            s_builtPairs[sp].clear();
            s_builtLivePairs[sp].clear();
            s_builtBase[sp] = 0;
        }
        s_recordRetriesLeft.store(kRecordRetryBudget);
        {
            ArmoryLock lock(s_sceneMutex);
            s_haveShownSelection = false;
        }
        {
            ArmoryLock lock(s_dataMutex);
            s_colorRecords.clear();
            s_eliteColorRecords.clear();
            s_suitRecords.clear();
            s_lastElitePriArgb = 0;
            s_lastEliteSecArgb = 0;
        }
    } else {
        s_selectionGeneration.fetch_add(1);
    }
}

bool ReachArmory::IsMenuMode() {
    return s_menuMode.load();
}

void ReachArmory::SetTargetScene(const std::string& sceneName) {
    ArmoryLock lock(s_sceneMutex);
    s_targetScene = sceneName;
}

std::string ReachArmory::GetTargetScene() {
    ArmoryLock lock(s_sceneMutex);
    return s_targetScene;
}

void ReachArmory::SetSelection(const ReachArmorySelection& sel) {
    bool pieceChanged = false;
    {
        ArmoryLock lock(s_sceneMutex);
        const ReachArmorySelection& prev = s_selection;
        if (prev.previewElite != sel.previewElite) s_speciesChangeMs.store(GetTickCount64());
        pieceChanged =
            prev.previewElite != sel.previewElite || prev.isFemale != sel.isFemale ||
            prev.helmet != sel.helmet || prev.chest != sel.chest ||
            prev.leftShoulder != sel.leftShoulder || prev.rightShoulder != sel.rightShoulder ||
            prev.hands != sel.hands || prev.wrist != sel.wrist || prev.utility != sel.utility ||
            prev.knees != sel.knees || prev.armorEffect != sel.armorEffect || prev.visor != sel.visor ||
            prev.eliteSuit != sel.eliteSuit || prev.eliteHelmet != sel.eliteHelmet ||
            prev.eliteLeftShoulder != sel.eliteLeftShoulder || prev.eliteRightShoulder != sel.eliteRightShoulder ||
            prev.eliteChest != sel.eliteChest || prev.eliteArms != sel.eliteArms || prev.eliteLegs != sel.eliteLegs ||
            prev.eliteArmorEffect != sel.eliteArmorEffect ||
            prev.eliteSuitIndex != sel.eliteSuitIndex;
        s_selection = sel;
    }
    if (pieceChanged) s_previewDirty.store(true);
    s_selectionGeneration.fetch_add(1);
    SaveSelectionFile(sel);   // a match process reads this instead of running the Armory
}

ReachArmorySelection ReachArmory::GetSelection() {
    ArmoryLock lock(s_sceneMutex);
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

bool ReachArmory::IsArmoryScene() {
    if (!s_menuMode.load() || !s_sceneReady.load()) return false;
    std::string scene;
    {
        ArmoryLock lock(s_dataMutex);
        scene = s_targetScene;
    }
    const int32_t idx = ResolveSceneIndex(scene);
    return (idx >= 0x0A && idx <= 0x14 && idx != 0x0E);
}

void ReachArmory::Tick() {
    if (!s_menuMode.load()) return;

    EnsureStringIdsLoaded();

    const uint64_t frame = ++s_frameCounter;

    // 1. Launch background scanner to locate mainmenu_all.map's GLOBALS table and color/suit records
    if ((s_globalsBase.load() == 0 || s_rescanRequested.load()) && frame >= 15 && !s_scanning.exchange(true)) {
        const bool isRescan = s_rescanRequested.exchange(false);
        std::thread([isRescan]() {
          try {
            if (!isRescan) {
                NOBLE_INFO("ReachArmory: Scanning process memory for mainmenu_all.map GLOBALS table...");
            }
            // The menu map can take a while to finish loading, so keep looking while the menu is up
            // (every 250 ms for the first 10 s, then once a second) instead of giving up after 10 s.
            // Once the GLOBALS table is found, the full colour-record scans run for a limited number of rounds:
            // a species switch clears the records, so "until the records exist" kept this thread repeating full
            // ~7 GB scans for as long as the player switched (CPU 30-70 %, menu lag, then std::bad_alloc). Records
            // still missing after that are left to the quick rescans (rebuilds, species switches, Tick's retry).
            int recordRounds = 0;
            for (int attempt = 0; s_menuMode.load(); ++attempt) {
                // A match runs in its own process while this menu process waits. Re-scanning ~2.5 GB of this
                // process's memory every second then only competes with the match for CPU and memory, so wait.
                while (s_menuMode.load() && noble::FrontEndMenu::IsMatchRunning()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(250));
                }
                if (!s_menuMode.load()) break;
                if (ScanGlobalsTable()) {
                    ScanColorRecords();
                    ScanEliteColorRecords();
                    ScanEliteTable();
                    bool hasRecords = false;
                    {
                        ArmoryLock lock(s_dataMutex);
                        const bool wantElite = GetSelection().previewElite;
                        hasRecords = wantElite ? !s_eliteColorRecords.empty()
                                               : !s_colorRecords.empty();
                    }
                    if (hasRecords) break;
                    if (++recordRounds >= 2) {
                        NOBLE_INFO("ReachArmory: colour records not all found after {} full scans; the Armory's quick rescans take over",
                                   recordRounds);
                        break;
                    }
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
                ScanEliteColorRecords();
                ScanEliteTable();
            }
          } catch (const std::exception& e) {
            NOBLE_ERROR("ReachArmory: memory scanner stopped: {}", e.what());
          }
            s_scanning.store(false);
        }).detach();
    }

    // Slow retry while an Armory scene is up and the shown species' colour records are still missing (the
    // safety net the menu scanner's endless loop used to provide). Every kRecordRetryIntervalMs, not every
    // 30 frames; the quick rescan itself limits full-memory scans.
    if (s_globalsBase.load() != 0 && !s_scanning.load() && !s_quickScanning.load() && !s_speciesDeferred.load()) {
        const int32_t idx = ResolveSceneIndex(GetTargetScene());
        const bool armoryShown = (idx >= 0x0A && idx <= 0x14 && idx != 0x0E);
        const bool wantElite = GetSelection().previewElite;
        bool missing = false;
        {
            ArmoryLock lock(s_dataMutex);
            missing = wantElite ? s_eliteColorRecords.empty() : s_colorRecords.empty();
        }
        const uint64_t now = GetTickCount64();
        if (armoryShown && missing && s_recordRetriesLeft.load() > 0 && now - s_lastRetryMs.load() >= kRecordRetryIntervalMs) {
            s_lastRetryMs.store(now);
            s_recordRetriesLeft.fetch_sub(1);
            StartQuickRescan();
        }
    }

    // 2. Apply scene camera, rotation, and live armor/color updates if GLOBALS table is active
    if (s_globalsBase.load() != 0) {
        const std::string sceneName = GetTargetScene();
        const int32_t sceneIdx = ResolveSceneIndex(sceneName);
        ApplyGlobalsAndPermutations(sceneIdx, sceneName);
        NotifyEngineTick();
    }
}

void ReachArmory::NotifyEngineTick() {
    if (!s_menuMode.load() || s_globalsBase.load() == 0) return;
    // During a species wait keep syncing the character on screen with its own selection, not the one it is waiting for.
    ReachArmorySelection sel;
    bool useShown = false;
    if (s_speciesDeferred.load()) {
        ArmoryLock lock(s_sceneMutex);
        if (s_haveShownSelection) {
            sel = s_shownSelection;
            useShown = true;
        }
    }
    if (!useShown) sel = GetSelection();
    const float visFloats[3] = {
        (sel.visor == "default" || sel.visor.empty()) ? 0.78f : sel.visorRgb[0],
        (sel.visor == "default" || sel.visor.empty()) ? 0.63f : sel.visorRgb[1],
        (sel.visor == "default" || sel.visor.empty()) ? 0.40f : sel.visorRgb[2]
    };
    uint32_t permPairsFlat[32] = {};
    size_t permPairCount = 0;
    const uint64_t permPairsGen = s_livePairsGen.load();
    {
        ArmoryLock lock(s_dataMutex);
        permPairCount = std::min<size_t>(s_livePairs.size(), 16u);
        for (size_t i = 0; i < permPairCount; ++i) {
            permPairsFlat[i * 2 + 0] = s_livePairs[i].first;
            permPairsFlat[i * 2 + 1] = s_livePairs[i].second;
        }
    }
    SyncArmoryLiveBipedEngineTickSEH(
        sel.previewElite,
        sel.isFemale,
        s_lastRemade.load(),
        sel.primaryRgb.data(),
        sel.secondaryRgb.data(),
        sel.accentRgb.data(),
        sel.lightsRgb.data(),
        visFloats,
        sel.accentColorIndex,
        permPairsFlat,
        permPairCount,
        permPairsGen);
}

// haloreach+0x53EF8 = player_mapping_get_unit_by_output_user(0): the local player's unit object datum.
// Resolved once, outside the __try block (a function containing __try may not need object unwinding, and a
// function-local static with a dynamic initialiser does).
static uint32_t LocalUnitDatum(uintptr_t base) {
    static uint32_t s_datum = 0xFFFFFFFFu;
    if (base && s_datum == 0xFFFFFFFFu) {
        using GetLocalUnitFn = int32_t(__fastcall*)(int32_t);
        s_datum = static_cast<uint32_t>(reinterpret_cast<GetLocalUnitFn>(base + 0x53EF8)(0));
    }
    return s_datum;
}

static void SyncMatchLiveBipedsSEH(uintptr_t base, const float* const wantSlots[4], uint32_t localUnitDatum) {
    __try {
        const uint32_t tlsIdx = *reinterpret_cast<const uint32_t*>(base + 0xC17B18);
        const uintptr_t tlsArray = *reinterpret_cast<const uintptr_t*>(__readgsqword(0x58) + static_cast<uintptr_t>(tlsIdx) * 8u);
        if (!tlsArray) return;
        uint8_t* objTable = *reinterpret_cast<uint8_t* const*>(tlsArray + 0x10);
        if (!objTable) return;

        using SetChangeColorFn = uint8_t(__fastcall*)(uint32_t, int32_t, const float*);
        auto SetChangeColor = reinterpret_cast<SetChangeColorFn>(base + 0x473858);

        const int32_t maxObjects = std::clamp(*reinterpret_cast<const int32_t*>(objTable + 0x44), 0, 2048);
        uint8_t* entries = *reinterpret_cast<uint8_t* const*>(objTable + 0x50);
        if (!entries) return;

        for (int32_t i = 0; i < maxObjects; ++i) {
            uint8_t* entry = entries + static_cast<size_t>(i) * 0x18;
            const uint16_t salt = *reinterpret_cast<const uint16_t*>(entry + 0x00);
            if (salt == 0) continue;
            const uint8_t objType = entry[0x04];
            if (objType != 0) continue; // 0 == biped
            uint8_t* objPtr = *reinterpret_cast<uint8_t* const*>(entry + 0x10);
            if (!objPtr) continue;

            const uint32_t objectDatum = (static_cast<uint32_t>(salt) << 16) | static_cast<uint32_t>(i);
            // Only the LOCAL player's own biped (haloreach+0x53EF8 = player_mapping_get_unit_by_output_user(0)).
            // Every other player's colours are replicated by the engine from that player's appearance record;
            // writing the local colours onto every player-controlled biped is what made every Spartan in a
            // match - including everyone else - look like the local player.
            if (localUnitDatum == 0xFFFFFFFFu || objectDatum != localUnitDatum) continue;
            const int16_t ccSize = *reinterpret_cast<const int16_t*>(objPtr + 0x188);
            const int16_t ccOffset = *reinterpret_cast<const int16_t*>(objPtr + 0x18A);
            if (ccSize >= 24 && ccOffset > 0 && ccOffset < 0x2000) {
                const int32_t slotCount = (ccSize / 12) / 2;
                float* ccBase = reinterpret_cast<float*>(objPtr + ccOffset);
                float* ccActive = ccBase + slotCount * 3;
                const int32_t nSlots = std::min<int32_t>(slotCount, 4);
                for (int32_t s = 0; s < nSlots; ++s) {
                    const float* w = wantSlots[s];
                    if (!w) continue;
                    float* b = ccBase + s * 3;
                    float* a = ccActive + s * 3;
                    if (std::abs(b[0] - w[0]) > 1e-4f || std::abs(b[1] - w[1]) > 1e-4f || std::abs(b[2] - w[2]) > 1e-4f ||
                        std::abs(a[0] - w[0]) > 1e-4f || std::abs(a[1] - w[1]) > 1e-4f || std::abs(a[2] - w[2]) > 1e-4f) {
                        b[0] = w[0]; b[1] = w[1]; b[2] = w[2];
                        a[0] = w[0]; a[1] = w[1]; a[2] = w[2];
                        SetChangeColor(objectDatum, s, w);
                    }
                }
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

// Campaign missions put AI Spartans (Noble Team, allied troopers) in the map next to the player. The
// shader rewrite below is map-wide: every 'rmsh' Spartan material in the loaded map receives the local
// player's undersuit, armour lights and visor colour, which painted the player's Armory colours onto every
// NPC Spartan as well. A campaign keeps the stock material constants; there the player's own biped still
// gets its colours from the profile record the engine applies, exactly as it did before this sync existed.
static bool CampaignKeepsStockMaterial() {
    static const bool s_campaign = [] {
        bool campaign = false;
        int argc = 0;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        for (int i = 1; argv && i + 1 < argc; ++i) {
            if (_wcsicmp(argv[i], L"-mode") == 0 && _wcsicmp(argv[i + 1], L"campaign") == 0) campaign = true;
        }
        if (argv) LocalFree(argv);
        return campaign;
    }();
    return s_campaign;
}

void ReachArmory::SyncMatchLiveBipeds() {
    if (CampaignKeepsStockMaterial()) return;
    HMODULE hReach = GetModuleHandleW(L"haloreach.dll");
    if (!hReach) return;
    const uintptr_t base = reinterpret_cast<uintptr_t>(hReach);

    const ReachArmorySelection sel = GetSelection();
    // Palette colours (and stock 0..4 visors) travel in the player's appearance record, and the engine
    // renders every player's own choice from it - other players' colours and visors included. Only the
    // Armory's wheel colours or extended visors (>= 5) need forcing into the local biped's materials.
    const bool customVisor = (sel.visorColorIndex >= 5);
    if (!sel.primaryCustom && !sel.secondaryCustom && !sel.accentCustom && !sel.lightsCustom && !customVisor) return;

    // Apply live visor reflection and suit constants to active map shader tags ('rmsh')
    const float visFloats[3] = {
        (sel.visor == "default" || sel.visor.empty()) ? 0.78f : sel.visorRgb[0],
        (sel.visor == "default" || sel.visor.empty()) ? 0.63f : sel.visorRgb[1],
        (sel.visor == "default" || sel.visor.empty()) ? 0.40f : sel.visorRgb[2]
    };
    const uint32_t priArgb = LinearRgbToArgbU32(sel.primaryRgb);
    const uint32_t secArgb = LinearRgbToArgbU32(sel.secondaryRgb);
    const uint32_t terArgb = LinearRgbToArgbU32(sel.accentRgb);
    const uint32_t visArgb = (sel.visor == "default" || sel.visor.empty())
                           ? 0xFFC7A167u
                           : LinearRgbToArgbU32(sel.visorRgb);
    const std::array<uint32_t, 4> words = sel.previewElite
        ? std::array<uint32_t, 4>{ priArgb, secArgb, terArgb, (sel.accentColorIndex != 0 ? terArgb : priArgb) }
        : std::array<uint32_t, 4>{ priArgb, secArgb, terArgb, visArgb };

    static ULONGLONG s_lastTagUpdate = 0;
    const ULONGLONG now = GetTickCount64();
    if (now - s_lastTagUpdate > 250) {
        s_lastTagUpdate = now;
        UpdateTagShaderConstantsSEH(
            sel.previewElite,
            words.data(),
            sel.accentCustom ? sel.accentRgb.data() : nullptr,
            nullptr,   // undersuit: never painted (see WriteColorsAndSuit)
            customVisor ? visFloats : nullptr);
    }

    const float slot0[3] = { sel.primaryRgb[0], sel.primaryRgb[1], sel.primaryRgb[2] };
    const float slot1[3] = { sel.secondaryRgb[0], sel.secondaryRgb[1], sel.secondaryRgb[2] };
    const float slot2[3] = { sel.accentRgb[0], sel.accentRgb[1], sel.accentRgb[2] };
    const float slot3[3] = { visFloats[0], visFloats[1], visFloats[2] };
    const float* const wantSlots[4] = {
        sel.primaryCustom ? slot0 : nullptr,
        sel.secondaryCustom ? slot1 : nullptr,
        sel.accentCustom ? slot2 : nullptr,
        customVisor ? slot3 : nullptr
    };

    SyncMatchLiveBipedsSEH(base, wantSlots, LocalUnitDatum(base));
}

std::vector<std::string> ReachArmory::MissingPermutationsFor(const std::string& list, const std::string& item) {
    std::vector<std::string> missing;
    const std::vector<PmcgChange>* changes = PmcgItem(list.c_str(), item);
    if (!changes) {
        missing.push_back("<item not in pmcg list " + list + ">");
        return missing;
    }
    for (const auto& change : *changes) {
        if (LookupPermutationId(change.permutation) == 0) {
            missing.push_back(change.region + "=" + change.permutation);
        }
    }
    return missing;
}

} // namespace noble
