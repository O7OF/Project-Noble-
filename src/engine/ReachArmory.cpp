#include "ReachArmory.hpp"
#include "ElitePack.hpp"
#include "SharedTagCache.hpp"
#include "common/Logger.hpp"
#include "common/ArmoryVisor.hpp"
#include "network/NetworkTransport.hpp"
#include "ui/FrontEndMenu.hpp"
#include "EmbeddedBlocks.hpp"
#include "hooks/HookManager.hpp"
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
#include <format>
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
static std::atomic<uint64_t> s_firstApplyMs{0};   // GetTickCount64() of the first apply since menu mode began
static std::atomic<uint64_t> s_holdStartMs{0};   // species switch: hold the Armory picture
static std::atomic<uint64_t> s_holdScriptDoneMs{0};   // the menu script finished that change (seq == done)
constexpr uint64_t kHoldAwaitingSwitch = ~0ull;   // s_holdScriptDoneMs: species picked, the switch itself not written yet
static std::atomic<bool> s_holdIsSpecies{false};   // only species switches hold the picture (see IsStageHeld)
// A colour/visor/armour change only updates the records the engine already tracks; the live model keeps its
// old shader data until the biped is re-instantiated. Set this whenever the visual part of the selection changes
// so the next apply forces the permutation write + REBUILD even when the piece list is identical.
static std::atomic<bool> s_previewDirty{false};

static std::recursive_mutex s_dataMutex;
using ArmoryLock = std::lock_guard<std::recursive_mutex>;
static bool s_stringIdsLoaded = false;
static std::unordered_map<std::string, uint32_t> s_stringIds;

// The armour pack map (data\armor\<name>.map + .noblereloc): its own string table and .rrpmcg are the
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
// A GLOBALS candidate whose script values never become real is a stale copy, not the live table (seen 2026-11-11:
// "scene done 1348969760" for 45 s while the game ran). Such bases are remembered and skipped by the next scan.
static std::atomic<uint64_t> s_globalsFoundMs{0};
static std::mutex s_rejectedBasesMutex;
static std::vector<uintptr_t> s_rejectedBases;
static bool IsRejectedBase(uintptr_t base) {
    std::lock_guard<std::mutex> lock(s_rejectedBasesMutex);
    return std::find(s_rejectedBases.begin(), s_rejectedBases.end(), base) != s_rejectedBases.end();
}
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

// First piece change after the menu map starts (2026-11-11): applying a helmet permutation while the menu map was
// still streaming in crashed the GPU driver (nvwgf2umx, ~1 s after the first rebuild) when the Armory was opened
// within a few seconds of launch; the same helmet was fine after the menu had idled 15-20 s. The first apply now
// waits this long after the GLOBALS table was first handled for the current menu map.
constexpr uint64_t kFirstApplyDelayMs = 500;   // after the menu script has started
static uint64_t s_settleStartMs = 0;
static uint64_t s_readySinceMs = 0;
static bool s_settleLogged = false;

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

// The shows_elite value the menu script reads: 0 Spartan, 1 Elite, 2 Marine, 3 Female Marine.
static const char* SpeciesName(uint32_t wantElite) {
    switch (wantElite) {
    case 1: return "Elite";
    case 2: return "Marine";
    case 3: return "Female Marine";
    default: return "Spartan";
    }
}
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
static std::vector<std::pair<uint32_t, uint32_t>> s_builtPairs[4];
static std::vector<std::pair<uint32_t, uint32_t>> s_builtLivePairs[4];
static uintptr_t s_builtBase[4] = { 0, 0, 0, 0 };
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
        // Noble's own menu map numbers its string_ids differently from RR's: prefer its table.
        std::ifstream loose(s_dataDir / "menumaps" / "noble_menu.string_ids.csv");
        if (!loose) loose.open(s_dataDir / "menumaps" / "mainmenu_all.string_ids.csv");
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

[[maybe_unused]] static bool AddrInStack(const StackRanges& stacks, uintptr_t addr) {
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
                            if (IsRejectedBase(foundBase)) continue;
                            s_globalsStride.store(stride);
                            s_globalsBase.store(foundBase);
                            s_globalsFoundMs.store(GetTickCount64());
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
                    if (IsRejectedBase(candidate)) continue;
                    s_globalsStride.store(stride);
                    s_globalsBase.store(candidate);
                    s_globalsFoundMs.store(GetTickCount64());
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
    int matchedVisorMaterials = 0;
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
                        ++matchedVisorMaterials;
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
                        ++matchedVisorMaterials;
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
        static std::atomic<int> s_visorPatchLogs{0};
        if (visorRgb && s_visorPatchLogs.fetch_add(1) < 5) {
            NOBLE_INFO("ReachArmory: visor shader tint applied to {} material(s) (rgb {:.2f},{:.2f},{:.2f})",
                       matchedVisorMaterials, visorRgb[0], visorRgb[1], visorRgb[2]);
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
    (void)words;
}


// The local player's emblem (2026-11-11). Reach keeps it in the per-user settings record (0xAB8 bytes each, user 0 at
// haloreach+0x2A03C58) at +0x9E: [0] foreground (< 0x7F), [1] background (< 0x7F), [2] flags (<= 7, bit 0 = toggled),
// [3..5] primary / secondary / background colour (<= 0x1F), [6..7] unused. The engine's own setter haloreach+0x2C7FF4
// (record, emblem, markDirty) validates it, stores it and marks the record dirty so the session sends it on. Stock
// Reach set it from its emblem screen and otherwise gives every record a random emblem (haloreach+0x2C8AE4), which is
// why the emblem on the armour never matched the Armory. Skipped when the record uses its override (dword bit 14).
//
// Main menu: objects that belong to no player (the Armory's characters) take the emblem from the global qword at
// haloreach+0x4E38F60 instead (haloreach+0x261B00, used while the game mode is the main menu). Stock Reach's own menu
// screens set it as a side effect of drawing an emblem (haloreach+0x504314); Noble's menu draws its own, so it stayed
// 0 = "Seventh Column" (the "7" on the Armory Spartan). It is set here as well.
static bool ApplyLocalEmblemSEH(const uint8_t want[8]) {
    HMODULE hReach = GetModuleHandleW(L"haloreach.dll");
    if (!hReach) return false;
    const uintptr_t base = reinterpret_cast<uintptr_t>(hReach);
    __try {
        uint8_t* menuEmblem = reinterpret_cast<uint8_t*>(base + 0x4E38F60);
        if (std::memcmp(menuEmblem, want, 8) != 0) std::memcpy(menuEmblem, want, 8);
        uint8_t* record = reinterpret_cast<uint8_t*>(base + 0x2A03C58);
        if ((*reinterpret_cast<const uint32_t*>(record) >> 14) & 1u) return false;
        if (std::memcmp(record + 0x9E, want, 8) == 0) return false;
        using SetEmblemFn = void(__fastcall*)(uint8_t*, const uint8_t*, uint8_t);
        reinterpret_cast<SetEmblemFn>(base + 0x2C7FF4)(record, want, 1);
        return std::memcmp(record + 0x9E, want, 8) == 0;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

// The armour's emblem comes from object "function" values (haloreach+0x4825FC, ids 0x250..0x258): for an object with a
// player they are read from that player's appearance (haloreach+0x45E70: player + 0xE0..0xE5, players are 0x490 bytes in
// the thread-local player table at +0x18). The record above only reaches that copy when the session next sends it,
// so the local player(s) get it directly too: every player in the menu, only the one driving `onlyUnit` in a match.
static int ApplyPlayerEmblemSEH(const uint8_t want[8], uint32_t onlyUnit) {
    int written = 0;
    HMODULE hReach = GetModuleHandleW(L"haloreach.dll");
    if (!hReach) return 0;
    const uintptr_t base = reinterpret_cast<uintptr_t>(hReach);
    __try {
        const uint32_t tlsIdx = *reinterpret_cast<const uint32_t*>(base + 0xC17B18);
        const uintptr_t tlsArray = *reinterpret_cast<const uintptr_t*>(__readgsqword(0x58) + static_cast<uintptr_t>(tlsIdx) * 8u);
        if (!tlsArray) return 0;
        uint8_t* players = *reinterpret_cast<uint8_t* const*>(tlsArray + 0x18);
        if (!players) return 0;
        const int32_t count = std::clamp(*reinterpret_cast<const int32_t*>(players + 0x48), 0, 16);
        uint8_t* entries = *reinterpret_cast<uint8_t* const*>(players + 0x50);
        if (!entries) return 0;
        for (int32_t i = 0; i < count; ++i) {
            uint8_t* p = entries + static_cast<size_t>(i) * 0x490;
            if (*reinterpret_cast<const uint16_t*>(p) == 0) continue;   // free slot
            if (onlyUnit != 0xFFFFFFFFu && *reinterpret_cast<const uint32_t*>(p + 0x28) != onlyUnit) continue;
            if (std::memcmp(p + 0xE0, want, 6) != 0) {
                std::memcpy(p + 0xE0, want, 6);
                ++written;
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
    return written;
}

// Dev test (2026-11-13, "-emblem-patch"): the armour-emblem extern (haloreach+0x261B00) only asks the host (slot 73/74)
// when the object is rendered through the normal object callback ([+0x4E38990] == +0x47A574); otherwise it shows the
// fallback emblem. NOP the "jne fallback" at +0x261C20 to see whether the Armory character then gets the host emblem.
static void PatchEmblemCallbackCheckOnce() {
    static bool s_done = false;
    if (s_done) return;
    s_done = true;
    if (!std::wcsstr(GetCommandLineW(), L"-emblem-patch")) return;
    HMODULE hReach = GetModuleHandleW(L"haloreach.dll");
    if (!hReach) { s_done = false; return; }
    uint8_t* p = reinterpret_cast<uint8_t*>(reinterpret_cast<uintptr_t>(hReach) + 0x261C20);
    if (p[0] != 0x0F || p[1] != 0x85) { NOBLE_WARN("ReachArmory: emblem patch skipped (bytes {:02X} {:02X})", p[0], p[1]); return; }
    DWORD old = 0;
    if (VirtualProtect(p, 6, PAGE_EXECUTE_READWRITE, &old)) {
        std::memset(p, 0x90, 6);
        VirtualProtect(p, 6, old, &old);
        FlushInstructionCache(GetCurrentProcess(), p, 6);
        NOBLE_INFO("ReachArmory: emblem callback check at +0x261C20 patched (test)");
    }
    // "-emblem-patch3": no render object -> take the "no player" branch (+0x261CBF: main-menu emblem, host key 0)
    // instead of the fallback emblem. je rel32 at +0x261C6F retargeted from +0x2620D8 to +0x261CBF.
    uint8_t* r = reinterpret_cast<uint8_t*>(reinterpret_cast<uintptr_t>(hReach) + 0x261C6F);
    if (std::wcsstr(GetCommandLineW(), L"-emblem-patch3") && r[0] == 0x0F && r[1] == 0x84 &&
        VirtualProtect(r, 6, PAGE_EXECUTE_READWRITE, &old)) {
        const int32_t rel = 0x261CBF - (0x261C6F + 6);
        std::memcpy(r + 2, &rel, 4);
        VirtualProtect(r, 6, old, &old);
        FlushInstructionCache(GetCurrentProcess(), r, 6);
        NOBLE_INFO("ReachArmory: emblem no-render-object branch at +0x261C6F retargeted (test)");
    }
    // "-emblem-patch2": also skip the default-bitmap check at +0x261C66 (je fallback).
    uint8_t* q = reinterpret_cast<uint8_t*>(reinterpret_cast<uintptr_t>(hReach) + 0x261C66);
    if (std::wcsstr(GetCommandLineW(), L"-emblem-patch2") && q[0] == 0x0F && q[1] == 0x84 &&
        VirtualProtect(q, 6, PAGE_EXECUTE_READWRITE, &old)) {
        std::memset(q, 0x90, 6);
        VirtualProtect(q, 6, old, &old);
        FlushInstructionCache(GetCurrentProcess(), q, 6);
        NOBLE_INFO("ReachArmory: emblem default-bitmap check at +0x261C66 patched (test)");
    }
}

// Dev trace (2026-11-13, "-emblem-trace"): log render-method extern ids 0x10..0x14 seen by haloreach+0x261B00 with the
// current render object (+0xB43B38) and render callback (+0x4E38990), to find how the menu character's emblem is set.
static uint8_t GameModeByteSEH(uintptr_t base) {
    __try {
        const uint32_t tlsIdx = *reinterpret_cast<const uint32_t*>(base + 0xC17B18);
        const uintptr_t tlsArray = *reinterpret_cast<const uintptr_t*>(__readgsqword(0x58) + static_cast<uintptr_t>(tlsIdx) * 8u);
        return *reinterpret_cast<const uint8_t*>(*reinterpret_cast<const uintptr_t*>(tlsArray + 0x48) + 0x10);
    } __except (EXCEPTION_EXECUTE_HANDLER) { return 0xEE; }
}
using ExternFn = void(__fastcall*)(int32_t, int32_t);
static ExternFn s_origExtern = nullptr;
static std::atomic<int> s_externLogs{0};
static void __fastcall ExternDetour(int32_t ext, int32_t arg) {
    if (ext >= 0x10 && ext <= 0x14 && s_externLogs.load() < 40) {
        const uintptr_t base = reinterpret_cast<uintptr_t>(GetModuleHandleW(L"haloreach.dll"));
        const int32_t obj = *reinterpret_cast<const int32_t*>(base + 0xB43B38);
        const uintptr_t cb = *reinterpret_cast<const uintptr_t*>(base + 0x4E38990);
        const uint8_t mode = GameModeByteSEH(base);
        static uint64_t s_seen[64] = {};
        const uint64_t key = (uint64_t(ext) << 48) ^ (uint64_t(uint32_t(obj)) << 16) ^ ((cb - base) & 0xFFFFFFF);
        bool dup = false;
        for (uint64_t k : s_seen) dup = dup || k == key;
        if (!dup) {
            s_seen[s_externLogs.fetch_add(1) % 64] = key;
            NOBLE_INFO("EmblemTrace: extern 0x{:X} arg {} render object 0x{:X} callback +0x{:X} mode {}", ext, arg, uint32_t(obj),
                       cb ? cb - base : 0, mode);
        }
    }
    s_origExtern(ext, arg);
}
static void InstallEmblemTraceOnce() {
    static bool s_done = false;
    if (s_done || !std::wcsstr(GetCommandLineW(), L"-emblem-trace")) return;
    HMODULE hReach = GetModuleHandleW(L"haloreach.dll");
    if (!hReach) return;
    s_done = true;
    void* target = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(hReach) + 0x261B00);
    if (HookManager::InstallHook(target, reinterpret_cast<void*>(&ExternDetour), reinterpret_cast<void**>(&s_origExtern), "EmblemTrace")) {
        HookManager::EnableAll();
        NOBLE_INFO("EmblemTrace: hooked +0x261B00");
    }
}

static void ApplyLocalEmblem(const ReachArmorySelection& sel, uint32_t onlyUnit = 0xFFFFFFFFu) {
    InstallEmblemTraceOnce();
    PatchEmblemCallbackCheckOnce();
    const uint8_t want[8] = {
        static_cast<uint8_t>(std::clamp(sel.emblemForeground, 0, 0x7E)),
        static_cast<uint8_t>(std::clamp(sel.emblemBackground, 0, 0x7E)),
        static_cast<uint8_t>(sel.emblemFlags & 7),
        static_cast<uint8_t>(std::clamp(sel.emblemColors[0], 0, 0x1F)),
        static_cast<uint8_t>(std::clamp(sel.emblemColors[1], 0, 0x1F)),
        static_cast<uint8_t>(std::clamp(sel.emblemColors[2], 0, 0x1F)),
        0, 0 };
    // Dev (2026-11-12): "-emblem-test fg,bg,c1,c2,c3" overrides the emblem (to tell it apart from the default).
    static int s_testEmblem[5] = { -2 };
    if (s_testEmblem[0] == -2) {
        s_testEmblem[0] = -1;
        if (const wchar_t* p = std::wcsstr(GetCommandLineW(), L"-emblem-test ")) {
            swscanf_s(p + 13, L"%d,%d,%d,%d,%d", &s_testEmblem[0], &s_testEmblem[1], &s_testEmblem[2], &s_testEmblem[3], &s_testEmblem[4]);
        }
    }
    uint8_t* w = const_cast<uint8_t*>(want);
    if (s_testEmblem[0] >= 0) {
        w[0] = static_cast<uint8_t>(s_testEmblem[0]); w[1] = static_cast<uint8_t>(s_testEmblem[1]);
        w[3] = static_cast<uint8_t>(s_testEmblem[2]); w[4] = static_cast<uint8_t>(s_testEmblem[3]); w[5] = static_cast<uint8_t>(s_testEmblem[4]);
    }
    if (const int players = ApplyPlayerEmblemSEH(want, onlyUnit)) {
        static int s_logs = 0;
        if (s_logs++ < 20) NOBLE_INFO("ReachArmory: emblem written to {} player appearance(s)", players);
    }
    if (ApplyLocalEmblemSEH(want)) {
        NOBLE_INFO("ReachArmory: emblem set (foreground {}, background {}, flags {}, colours {}/{}/{})",
                   want[0], want[1], want[2], want[3], want[4], want[5]);
    }
}

// The Elite's change colours 2 and 3 are fixed by the game, not picked by the player. haloreach+0x38DB0C (the
// player-colour builder used by haloreach+0x7EF6C / 0x44560) fills slots 0/1 from the colour palette for both
// species, slots 2/3 from the accent table for Spartans, and for Elites copies two constant colours from the
// multiplayer globals: [haloreach+0xC1A238] +0x4BC (slot 2) and +0x4C8 (slot 3), present when
// [haloreach+0xC1A230] is set. Noble used to write the hidden accent colour (stock teal) into both, and on the
// Elite those two slots cover most of the armour, so it stayed teal whatever the player picked (2026-11-11).
static bool EliteFixedChangeColorsSEH(float slot2[3], float slot3[3]) {
    HMODULE hReach = GetModuleHandleW(L"haloreach.dll");
    if (!hReach) return false;
    const uintptr_t base = reinterpret_cast<uintptr_t>(hReach);
    __try {
        const uintptr_t gate = *reinterpret_cast<const uintptr_t*>(base + 0xC1A230);
        const uint8_t* g = *reinterpret_cast<const uint8_t* const*>(base + 0xC1A238);
        if (!gate || !g) return false;
        const float* a = reinterpret_cast<const float*>(g + 0x4BC);
        const float* b = reinterpret_cast<const float*>(g + 0x4C8);
        for (int i = 0; i < 3; ++i) {
            if (!std::isfinite(a[i]) || !std::isfinite(b[i]) || a[i] < 0.f || a[i] > 4.f || b[i] < 0.f || b[i] > 4.f) return false;
        }
        slot2[0] = a[0]; slot2[1] = a[1]; slot2[2] = a[2];
        slot3[0] = b[0]; slot3[1] = b[1]; slot3[2] = b[2];
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static bool EliteFixedChangeColors(float slot2[3], float slot3[3]) {
    const bool ok = EliteFixedChangeColorsSEH(slot2, slot3);
    static bool s_logged = false;
    if (ok && !s_logged) {
        s_logged = true;
        NOBLE_INFO("ReachArmory: Elite fixed change colours 2 ({:.3f} {:.3f} {:.3f}) 3 ({:.3f} {:.3f} {:.3f})",
                   slot2[0], slot2[1], slot2[2], slot3[0], slot3[1], slot3[2]);
    }
    return ok;
}

static void SyncArmoryLiveBipedEngineTickSEH(bool previewElite,
                                             bool isFemale,
                                             int32_t remadeCounter,
                                             const float priRgb[3],
                                             const float secRgb[3],
                                             const float accRgb[3],
                                             const float litRgb[3],
                                             const float visRgb[3],
                                             const float* eliteSlot2,
                                             const float* eliteSlot3,
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
        // Elite slots 2/3: the game's fixed Elite colours (see EliteFixedChangeColorsSEH). If they couldn't be read,
        // repeat the primary and secondary rather than painting the hidden accent colour.
        const bool eliteFixed = previewElite && eliteSlot2 && eliteSlot3;
        const float slot2[3] = {
            eliteFixed ? eliteSlot2[0] : (previewElite ? priRgb[0] : accRgb[0]),
            eliteFixed ? eliteSlot2[1] : (previewElite ? priRgb[1] : accRgb[1]),
            eliteFixed ? eliteSlot2[2] : (previewElite ? priRgb[2] : accRgb[2]) };
        const float slot3[3] = {
            eliteFixed ? eliteSlot3[0] : (previewElite ? secRgb[0] : visRgb[0]),
            eliteFixed ? eliteSlot3[1] : (previewElite ? secRgb[1] : visRgb[1]),
            eliteFixed ? eliteSlot3[2] : (previewElite ? secRgb[2] : visRgb[2]) };
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
        // 2026-11-11: a rebuild (every armour-effect change) re-creates the character, and Reach gives regions the
        // model's variant doesn't name a RANDOM permutation. elite_noble's per-piece regions aren't in its variants,
        // so the re-created Elite showed random pieces when the 2-tick window had passed before the new object
        // existed. Keep applying for ~60 ticks, and restart the window whenever a new biped object shows up.
        static uint32_t s_permSeenObjects[16] = {};
        if (s_lastPermGen != permPairsGen || s_lastPermRemade != remadeCounter) {
            s_lastPermGen = permPairsGen;
            s_lastPermRemade = remadeCounter;
            s_permSyncTicksRemaining = 60;
        }
        for (int32_t i = 0; i < maxObjects; ++i) {
            const uint8_t* entry = entries + static_cast<size_t>(i) * 0x18;
            const uint16_t salt = *reinterpret_cast<const uint16_t*>(entry + 0x00);
            if (salt == 0 || entry[0x04] != 0 || !*reinterpret_cast<uint8_t* const*>(entry + 0x10)) continue;
            const uint32_t datum = (static_cast<uint32_t>(salt) << 16) | static_cast<uint32_t>(i);
            bool seen = false;
            for (uint32_t d : s_permSeenObjects) seen = seen || d == datum;
            if (seen) continue;
            std::memmove(s_permSeenObjects + 1, s_permSeenObjects, sizeof(s_permSeenObjects) - sizeof(uint32_t));
            s_permSeenObjects[0] = datum;
            s_permSyncTicksRemaining = std::max(s_permSyncTicksRemaining, 60);
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
    // Emissive lights (eyes, illumination cards, LEDs) are not set here (they live in the light shaders' tag data).
    // Elite [2]/[3] are the game's fixed Elite colours, not the accent (see EliteFixedChangeColorsSEH).
    float eliteSlot2[3] = {}, eliteSlot3[3] = {};
    const bool eliteFixed = sel.previewElite && EliteFixedChangeColors(eliteSlot2, eliteSlot3);
    const std::array<uint32_t, 4> words = sel.previewElite
        ? std::array<uint32_t, 4>{ priArgb, secArgb,
                                   eliteFixed ? LinearRgbToArgbU32(std::array<float, 3>{ eliteSlot2[0], eliteSlot2[1], eliteSlot2[2] }) : priArgb,
                                   eliteFixed ? LinearRgbToArgbU32(std::array<float, 3>{ eliteSlot3[0], eliteSlot3[1], eliteSlot3[2] }) : secArgb }
        : std::array<uint32_t, 4>{ priArgb, secArgb, terArgb, visArgb };

    // Serialises the change check below (WriteColorsAndSuit can run on the menu and the engine thread).
    ArmoryLock lock(s_dataMutex);
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

    // The armour colours themselves go through the game's SetChangeColor (SyncArmoryLiveBipedEngineTickSEH), which
    // also refreshes the game's own colour copies. The old ARGB "colour record" memory scans and writes are gone
    // (2026-11-11): they searched gigabytes of memory, rarely matched and could write into unrelated data.
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
                if (line.size() >= 3 && static_cast<uint8_t>(line[0]) == 0xEF && static_cast<uint8_t>(line[1]) == 0xBB &&
                    static_cast<uint8_t>(line[2]) == 0xBF) line.erase(0, 3);   // UTF-8 BOM on the first line
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
    if (const uint8_t* embPmcg = EmbeddedBlocks::GetBlock("spartan_armor_parts.txt", &embPmcgSize); embPmcg && embPmcgSize > 0) {
        std::stringstream in{ std::string(reinterpret_cast<const char*>(embPmcg), embPmcgSize) };
        std::string line, list, item;
        size_t items = 0;
        while (std::getline(in, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            // The embedded spartan_armor_parts.txt starts with a UTF-8 BOM (2026-11-12): without this the first line's
            // "L 20" was not recognised, every helmet landed under the key ":name", and the whole mapping was skipped.
            if (line.size() >= 3 && static_cast<uint8_t>(line[0]) == 0xEF && static_cast<uint8_t>(line[1]) == 0xBB &&
                static_cast<uint8_t>(line[2]) == 0xBF) line.erase(0, 3);
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
        NOBLE_INFO("ReachArmory: {} Armory items -> model regions from embedded 'spartan_armor_parts.txt'", items);
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
    const bool showMarine = sel.previewMarine && armoryScene;   // 2026-11-13: MARINE tab (rr_elite = 2)
    const bool showElite = sel.previewElite && armoryScene && !showMarine;
    const bool showFemale = showMarine && sel.marineFemale;   // female Marine (marine_f_armory, rr_elite = 3)

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

    const uint32_t wantElite = showFemale ? 3u : (showMarine ? 2u : (showElite ? 1u : 0u));

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
    // The settle wait exists because the Elite has its own pedestal/camera (0x0B): flipping the camera before the
    // character keeps 1-2 frames of Elite on screen (Tyler's 21:43 video). Marine / Female Marine / Spartan all
    // stand on the Spartan pedestal, so those flips can be applied at once - waiting on the Elite settle path is what
    // left the previous character (and its colours) on screen for seconds when switching Spartan <-> Marine.
    const bool eliteInvolved = ((curElite == 1u) != (wantElite == 1u));
    const bool speciesDeferred = speciesFlipWanted &&
                                 (rebuildBusy || (eliteInvolved && nowMs - s_speciesChangeMs.load() < kSpeciesSettleMs));
    s_speciesDeferred.store(speciesDeferred);
    if (speciesDeferred) {
        if (s_speciesWaitStartMs == 0) s_speciesWaitStartMs = nowMs;
    } else if (s_speciesWaitStartMs != 0) {
        if (speciesFlipWanted) {
            NOBLE_INFO("ReachArmory: species switch to {} applied after a {} ms wait", SpeciesName(wantElite),
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
            s_holdStartMs.store(GetTickCount64());
            s_holdScriptDoneMs.store(0);
            s_holdIsSpecies.store(true);
            ArmoryLock lock(s_dataMutex);
            // The character of this species is still in the scene: compare the next pairs with what it was
            // last built with, so an unchanged look is not rebuilt. Its live pairs are re-applied for a couple
            // of ticks (NotifyEngineTick) in case the map script re-created it on the flip.
            const int sp = static_cast<int>(std::min<uint32_t>(wantElite, 3u));
            s_lastPairs = s_builtPairs[sp];
            s_lastPairsBase = s_builtBase[sp];
            s_livePairs = s_builtLivePairs[sp];
            s_livePairsGen.fetch_add(1);
            s_rotationBaseBits.store(0xFFFFFFFFu);
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
        s_lastRemade.store(remade);
    }
    // DIAGNOSTIC (2026-11-11): when does the menu script itself finish a scene change / piece apply?
    int32_t sceneDoneNow = 0;   // rr_scene_done: garbage until the menu map's script has started
    {
        static int32_t s_diagSceneDone = -999, s_diagDone = -999, s_diagSeq = -999, s_diagScene = -999;
        int32_t sceneDone = 0, scriptDone = 0, scriptSeq = 0, sceneReq = 0;
        SafeReadI32(base + 0x25 * stride, &sceneDone);
        SafeReadI32(base + 0x03 * stride, &scriptDone);
        SafeReadI32(base + 0x02 * stride, &scriptSeq);
        SafeReadI32(base + 0x24 * stride, &sceneReq);
        sceneDoneNow = sceneDone;
        if (s_holdStartMs.load() != 0 && s_holdScriptDoneMs.load() == 0 && scriptSeq == scriptDone && sceneReq == sceneDone) {
            s_holdScriptDoneMs.store(GetTickCount64());
        }
        if (sceneDone != s_diagSceneDone || scriptDone != s_diagDone || scriptSeq != s_diagSeq || sceneReq != s_diagScene) {
            s_diagSceneDone = sceneDone; s_diagDone = scriptDone; s_diagSeq = scriptSeq; s_diagScene = sceneReq;
            NOBLE_INFO("ReachArmory: script state: scene requested {} done {} | armory seq {} done {}", sceneReq, sceneDone, scriptSeq, scriptDone);
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
        s_holdStartMs.store(GetTickCount64());
        s_holdScriptDoneMs.store(0);
        s_holdIsSpecies.store(false);
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
    const uint64_t settleNowMs = GetTickCount64();
    if (s_settleStartMs == 0) s_settleStartMs = settleNowMs;
    static uint64_t s_delayMs = kFirstApplyDelayMs;
    static bool s_delayRead = false;
    if (!s_delayRead) {   // TEMPORARY tuning: data\userdata\noble-test-delay.txt holds the wait in milliseconds
        s_delayRead = true;
        std::ifstream delayIn(s_dataDir / "userdata" / "noble-test-delay.txt");
        unsigned long long v = 0;
        if (delayIn && (delayIn >> v)) { s_delayMs = v; NOBLE_INFO("ReachArmory: TEST first-apply delay {} ms", v); }
    }
    // The GLOBALS table is found as soon as the map's memory exists, but its values are garbage until the map's
    // script starts (seen 2026-11-11: scene done 1243315504, armory seq 193, then 12 s later 3 / 0 / 0). Writing
    // pieces before that crashed the GPU driver. Ready = rr_scene_done holds a real scene number (-1..63).
    if (s_readySinceMs == 0 && sceneDoneNow >= -1 && sceneDoneNow <= 63) s_readySinceMs = settleNowMs;
    // No blind fallback any more: writing pieces into a table that never turns real is writing into the wrong memory.
    // If the script values are still garbage 20 s after the table was found (the script itself starts ~12 s after
    // the map loads), the table is a stale copy: drop it, remember it, and let the scanner find the live one.
    const uint64_t foundMs = s_globalsFoundMs.load();
    if (s_readySinceMs == 0 && foundMs != 0 && settleNowMs - foundMs >= 20000) {
        {
            std::lock_guard<std::mutex> lock(s_rejectedBasesMutex);
            s_rejectedBases.push_back(base);
        }
        NOBLE_WARN("ReachArmory: GLOBALS table at 0x{:X} never became live (scene done {}); rescanning for the real one",
                   base, sceneDoneNow);
        s_globalsFoundMs.store(0);
        s_sceneReady.store(false);
        s_globalsBase.store(0);
        s_settleStartMs = 0;
        return;
    }
    const bool startupSettled = s_readySinceMs != 0 && (settleNowMs - s_readySinceMs) >= s_delayMs;
    if (startupSettled && !s_settleLogged) {
        s_settleLogged = true;
        NOBLE_INFO("ReachArmory: menu map settled; piece changes may be applied");
    }
    if (gen != s_appliedGeneration && !rebuildInFlight && !speciesDeferred && startupSettled) {
        s_appliedGeneration = gen;
        s_previewDirty.store(false);
        if (s_firstApplyMs.load() == 0) s_firstApplyMs.store(settleNowMs);

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

        if (showFemale) {
            // Female Marine (2026-11-13): marine_f_armory takes these pairs (mainmenu.hsc, rr_elite = 3).
            static const std::pair<const char*, const char*> kFemaleDefaults[] = {
                { "helmet", "trooper_light" }, { "head", "default" }, { "flair_helmet_a", "trooper_light" },
                { "flair_shoulder_l", "trooper_light" }, { "flair_shoulder_r", "trooper_light" },
                { "flair_chest_a", "trooper_light" }, { "flair_backpack", "trooper_light" },
                { "flair_belt", "trooper_light" }, { "legs", "trooper_light" } };
            for (const auto& [region, def] : kFemaleDefaults) {
                std::string perm = def;
                for (const auto& [r, p] : sel.femalePieces) if (r == region) perm = p;
                pushPair(region, perm, def);
            }
        } else if (showMarine) {
            // MARINE tab (2026-11-13): marine_armory takes these pairs (mainmenu.hsc, rr_elite = 2). A slot left on MOD
            // DEFAULT shows the stock trooper piece.
            static const std::pair<const char*, const char*> kMarineDefaults[] = {
                { "helmet", "trooper_light" }, { "head", "default" }, { "flair_shoulder_l", "trooper_light" },
                { "flair_shoulder_r", "trooper_light" }, { "flair_chest_a", "trooper_light" },
                { "flair_backpack", "trooper_light" }, { "flair_belt", "trooper_light" }, { "flair_neck", "trooper_light" },
                { "legs", "trooper_light" } };
            for (const auto& [region, def] : kMarineDefaults) {
                std::string perm = def;
                for (const auto& [r, p] : sel.marinePieces) if (r == region) perm = p;
                pushPair(region, perm, def);
            }
        } else if (showElite) {
            // Noble's menu Elite (objects\characters\elite_noble, built from The Prophets' Hand's per-piece Elite by
            // Lachiebuzzx) has one region per piece: Reach's 8 sets plus Honor Guard, Council, Ossoona, Schism and
            // Combat. Pieces it lacks (H2A, Honor Guard robe) fall back to the default named here.
            pushPair("helmet",         sel.eliteHelmet,        "minor");
            pushPair("left_shoulder",  sel.eliteLeftShoulder,  "minor");
            pushPair("right_shoulder", sel.eliteRightShoulder, "minor");
            pushPair("chest",          sel.eliteChest,         "minor");
            pushPair("forearms",       sel.eliteArms,          "default");
            pushPair("legs",           sel.eliteLegs,          "minor");
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
            // TEMPORARY bisect switch (2026-11-11): data\userdata\noble-test-skip.txt holds pmcg list ids to skip,
            // separated by spaces (20 helmet+variant, 2c left shoulder, 38 right shoulder, 44 chest, 50 wrist,
            // 5c utility, 68 knees, 80 effect). Remove once the menu crash is understood.
            std::string testSkip;
            {
                std::ifstream skipIn(s_dataDir / "userdata" / "noble-test-skip.txt");
                if (skipIn) { std::stringstream skipSs; skipSs << skipIn.rdbuf(); testSkip = " " + skipSs.str() + " "; }
            }
            auto apply = [&](const char* list, const std::string& item) {
                if (!testSkip.empty() && testSkip.find(std::string(" ") + list + " ") != std::string::npos) return;
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
            // The pack maps the default knee guard (knees_default) to the female "kat" guard. On the male body it is
            // too small (gap under the D), so the male body gets the plain "default" knee guard instead.
            // 2026-11-12 (Tyler): "default" IS the FJ/PARA knee guard (pmcg knees_1), so DEFAULT and FJ/PARA looked
            // the same. Stock Reach uses "kat" for knees_default on both bodies; keep that (override removed).
            apply("80", PmcgEffectName(sel.armorEffect));
            // TEMPORARY bisect switch: data\userdata\noble-test-head.txt holds ONE head permutation name to force
            // (and drops the flair_head_* attachments), e.g. "default", "bungie", "player_sam".
            {
                std::ifstream headIn(s_dataDir / "userdata" / "noble-test-head.txt");
                std::string forced;
                if (headIn && std::getline(headIn, forced)) {
                    while (!forced.empty() && (forced.back() == '\r' || forced.back() == ' ')) forced.pop_back();
                    if (!forced.empty()) {
                        perm["head"] = forced;
                        perm.erase("flair_head_a"); perm.erase("flair_head_b"); perm.erase("flair_head_c");
                        NOBLE_INFO("ReachArmory: TEST head permutation forced to '{}'", forced);
                    }
                }
            }
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
            {
                static bool s_warnedNoPmcg = false;
                if (!s_warnedNoPmcg) {
                    s_warnedNoPmcg = true;
                    NOBLE_WARN("ReachArmory: pmcg list 20 has no 'default' item; Spartan pieces go to the model by item name only "
                               "(most helmets, attachments and effects won't show)");
                }
            }
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
                // 2026-11-13: two more cases are rebuilt. (1) The knife: mainmenu.hsc swaps the Armory Spartan to the
                // "noble_noknife" model variant only when it rebuilds, for chests whose flair_chest_a permutation
                // carries the knife (noble_nk in the script). A live change into or out of one of those chests kept
                // the old variant, so the knife came back (Tyler 2026-11-13). (2) AKIS: a live change to the AKIS
                // helmet (dlc2_helmet_hilands) does not show; a rebuild does (tests akh2 vs akh3, 2026-11-13).
                if (!needsRebuild && anyDiff) {
                    const uint32_t chestA = LookupStringId("flair_chest_a");
                    static const char* const kKnifeHiddenChests[] = { "player_joshua", "jun", "emile", "player_kelly", "player_jai" };
                    auto knifeHidden = [&](const std::vector<std::pair<uint32_t, uint32_t>>& v) {
                        for (const auto& pr : v) {
                            if (chestA == 0 || pr.first != chestA) continue;
                            for (const char* name : kKnifeHiddenChests) {
                                if (pr.second == LookupStringId(name)) return true;
                            }
                        }
                        return false;
                    };
                    if (knifeHidden(writtenPairs) != knifeHidden(s_lastPairs)) needsRebuild = true;
                    const uint32_t akis = LookupStringId("dlc2_helmet_hilands");
                    for (size_t i = 0; i < writtenPairs.size() && akis != 0; ++i) {
                        if (writtenPairs[i].second == akis && s_lastPairs[i].second != akis) needsRebuild = true;
                    }
                }
                liveChange = anyDiff && !needsRebuild;
            }
            if (liveChange) {
                {
                    ArmoryLock lock(s_dataMutex);
                    s_lastPairs = writtenPairs;
                    s_livePairs = livePairs;
                    s_livePairsGen.fetch_add(1);
                    const int sp = showFemale ? 3 : (showMarine ? 2 : (showElite ? 1 : 0));
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
                    const int sp = showFemale ? 3 : (showMarine ? 2 : (showElite ? 1 : 0));
                    s_builtPairs[sp] = writtenPairs;
                    s_builtLivePairs[sp] = livePairs;
                    s_builtBase[sp] = base;
                }
                s_lastPairsBase = base;
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
    j["emblem"] = nlohmann::json::array({ sel.emblemForeground, sel.emblemBackground, sel.emblemFlags,
                                          sel.emblemColors[0], sel.emblemColors[1], sel.emblemColors[2] });
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
        if (j.contains("emblem") && j["emblem"].is_array() && j["emblem"].size() == 6) {
            sel.emblemForeground = j["emblem"][0].get<int>();
            sel.emblemBackground = j["emblem"][1].get<int>();
            sel.emblemFlags = j["emblem"][2].get<int>();
            for (int i = 0; i < 3; ++i) sel.emblemColors[i] = j["emblem"][3 + i].get<int>();
        }
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
        s_dataDir / "menumaps" / "noble_menu.map",
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
        s_firstApplyMs.store(0);
        s_settleStartMs = 0;
        s_settleLogged = false;
        s_readySinceMs = 0;
        s_rebuildReadyFrame = 0;
        s_lastPairs.clear();
        s_livePairs.clear();
        s_lastPairsBase = 0;
        s_bumpAtFrame = 0;
        s_settlePending = false;
        s_previewDirty.store(false);
        s_speciesDeferred.store(false);
        s_speciesWaitStartMs = 0;
        for (int sp = 0; sp < 4; ++sp) {   // Spartan, Elite, Marine, female Marine
            s_builtPairs[sp].clear();
            s_builtLivePairs[sp].clear();
            s_builtBase[sp] = 0;
        }
        {
            ArmoryLock lock(s_sceneMutex);
            s_haveShownSelection = false;
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
        if (prev.previewElite != sel.previewElite || prev.previewMarine != sel.previewMarine || prev.marineFemale != sel.marineFemale) {
            const uint64_t now = GetTickCount64();
            s_speciesChangeMs.store(now);
            // Hold the Armory picture from the click, not only from the switch: between the click and the switch
            // (the species settle wait) the game already showed one frame of the Elite through the Spartan's camera
            // when going Elite -> Spartan (Tyler's 06:13 video, ~0.5 s before every Elite -> Spartan switch).
            s_holdStartMs.store(now);
            s_holdScriptDoneMs.store(kHoldAwaitingSwitch);
            s_holdIsSpecies.store(true);
        }
        pieceChanged =
            prev.previewElite != sel.previewElite || prev.previewMarine != sel.previewMarine || prev.isFemale != sel.isFemale ||
            prev.marinePieces != sel.marinePieces || prev.femalePieces != sel.femalePieces || prev.marineFemale != sel.marineFemale ||
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

namespace {
// After the first apply the engine still needs a few frames to swap the pieces in; showing the model before
// that is the "Elite jumps / default armour flashes" moment.
constexpr uint64_t kPresentSettleMs = 450;
}

bool ReachArmory::IsCharacterPresentable() {
    const uint64_t first = s_firstApplyMs.load();
    return s_sceneReady.load() && first != 0 && GetTickCount64() - first >= kPresentSettleMs;
}

void ReachArmory::HoldStageBriefly() {
    const uint64_t now = GetTickCount64();
    s_holdStartMs.store(now);
    s_holdScriptDoneMs.store(now);
}

bool ReachArmory::IsStageHeld() {
    // Effect/armour rebuilds: no hold (Tyler 2026-11-11: a short freeze read as stutter; the one-frame white-armour
    // flash is the lesser evil). Species switches: hold, because the game swaps the camera a few ticks after it
    // swaps the character, which showed one frame of the new character through the old camera (a "jump").
    if (!s_holdIsSpecies.load()) return false;
    const uint64_t start = s_holdStartMs.load();
    if (start == 0) return false;
    const uint64_t now = GetTickCount64();
    // Short on purpose: a longer hold read as stutter (2026-11-11, 60 fps capture). The bad picture is the first
    // frame or two of the new biped, right when the script finishes.
    if (now - start > 1200) return false;            // never freeze longer than this, whatever happens
    const uint64_t done = s_holdScriptDoneMs.load();
    if (done == kHoldAwaitingSwitch) return true;    // picked, waiting for the switch to be written
    return done == 0 || now - done < 160;            // until the script is done, plus the camera's tick delay
}

uint64_t ReachArmory::CharacterPresentableForMs() {
    const uint64_t first = s_firstApplyMs.load();
    if (!s_sceneReady.load() || first == 0) return 0;
    const uint64_t since = GetTickCount64() - first;
    return since >= kPresentSettleMs ? since - kPresentSettleMs : 0;
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

    // 1. Launch background scanner to locate mainmenu_all.map's GLOBALS table (and the Elite region table)
    if ((s_globalsBase.load() == 0 || s_rescanRequested.load()) && frame >= 15 && !s_scanning.exchange(true)) {
        const bool isRescan = s_rescanRequested.exchange(false);
        std::thread([isRescan]() {
          SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
          try {
            if (!isRescan) {
                NOBLE_INFO("ReachArmory: Scanning process memory for mainmenu_all.map GLOBALS table...");
            }
            // The menu map can take a while to finish loading, so keep looking while the menu is up
            // (every 250 ms for the first 10 s, then once a second) instead of giving up after 10 s.
            for (int attempt = 0; s_menuMode.load(); ++attempt) {
                // A match runs in its own process while this menu process waits. Re-scanning ~2.5 GB of this
                // process's memory every second then only competes with the match for CPU and memory, so wait.
                while (s_menuMode.load() && noble::FrontEndMenu::IsMatchRunning()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(250));
                }
                if (!s_menuMode.load()) break;
                if (ScanGlobalsTable()) {
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
                ScanEliteTable();
            }
          } catch (const std::exception& e) {
            NOBLE_ERROR("ReachArmory: memory scanner stopped: {}", e.what());
          }
            s_scanning.store(false);
        }).detach();
    }

    // 2. Apply scene camera, rotation, and live armor/color updates if GLOBALS table is active
    if (s_globalsBase.load() != 0) {
        const std::string sceneName = GetTargetScene();
        const int32_t sceneIdx = ResolveSceneIndex(sceneName);
        ApplyGlobalsAndPermutations(sceneIdx, sceneName);
        NotifyEngineTick();
    } else {
        // 2026-11-12 emblem test (-emblem-early): write the menu emblem as soon as the menu runs.
        static const bool s_earlyEmblem = std::wcsstr(GetCommandLineW(), L"-emblem-early") != nullptr;
        if (s_earlyEmblem) ApplyLocalEmblem(GetSelection());
    }
}

void ReachArmory::NotifyEngineTick() {
    // 2026-11-12 emblem test: write the menu emblem from the first menu tick, before the Armory scene exists, in
    // case the engine builds the armour emblem picture once when the map loads.
    if (s_menuMode.load() && s_globalsBase.load() == 0) {
        static const bool s_earlyEmblem = std::wcsstr(GetCommandLineW(), L"-emblem-early") != nullptr;
        if (s_earlyEmblem) ApplyLocalEmblem(GetSelection());
    }
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
    float eliteSlot2[3] = {}, eliteSlot3[3] = {};
    const bool haveEliteFixed = sel.previewElite && EliteFixedChangeColors(eliteSlot2, eliteSlot3);
    static const float kMarineGrey[3] = { 0.35f, 0.35f, 0.33f };
    SyncArmoryLiveBipedEngineTickSEH(
        sel.previewElite,
        sel.isFemale,
        s_lastRemade.load(),
        // MARINE tab: the Marine's own colours (or a neutral grey for MOD DEFAULT) on the preview.
        sel.previewMarine ? (sel.marineColors ? sel.marinePrimaryRgb.data() : kMarineGrey) : sel.primaryRgb.data(),
        sel.previewMarine ? (sel.marineColors ? sel.marineSecondaryRgb.data() : kMarineGrey) : sel.secondaryRgb.data(),
        // Marine slots 2/3 (random per spawn otherwise: the stray blue / blue ODST helmet, Tyler 05:51).
        sel.previewMarine ? (sel.marineColors ? sel.marineThirdRgb.data() : kMarineGrey) : sel.accentRgb.data(),
        sel.lightsRgb.data(),
        sel.previewMarine ? (sel.marineColors ? sel.marineFourthRgb.data() : kMarineGrey) : visFloats,
        haveEliteFixed ? eliteSlot2 : nullptr,
        haveEliteFixed ? eliteSlot3 : nullptr,
        permPairsFlat,
        permPairCount,
        permPairsGen);
    ApplyLocalEmblem(sel);
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

// Reads haloreach.dll's live game_globals ([TLS + 0x48]) to identify Campaign ([+0x10] == 1) and
// deterministic synchronous lockstep modes (Campaign, Firefight/Survival [+0x110EC] != 0, or Co-Op [+0x1EAB0] != 0).
static bool ReadEngineModeFlagsSEH(uintptr_t base, bool* outIsCampaign, bool* outIsSynchronousSim) {
    if (!base) return false;
    __try {
        const uint32_t tlsIdx = *reinterpret_cast<const uint32_t*>(base + 0xC17B18);
        const uintptr_t tlsArray = *reinterpret_cast<const uintptr_t*>(
            __readgsqword(0x58) + static_cast<uintptr_t>(tlsIdx) * 8u);
        if (!tlsArray) return false;
        const uint8_t* gameGlobals = *reinterpret_cast<const uint8_t* const*>(tlsArray + 0x48);
        if (!gameGlobals) return false;
        const uint8_t gameMode = gameGlobals[0x10];          // 1 = campaign, 2 = ui_shell, 3 = multiplayer
        const uint8_t isSurvival = gameGlobals[0x110EC];     // non-zero in Firefight / Survival
        const uint8_t isCooperative = gameGlobals[0x1EAB0];  // non-zero in Co-Op sessions
        if (outIsCampaign) *outIsCampaign = (gameMode == 1);
        if (outIsSynchronousSim) *outIsSynchronousSim = (gameMode != 3) || (isSurvival != 0) || (isCooperative != 0);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

void ReachArmory::SyncMatchLiveBipeds() {
    HMODULE hReach = GetModuleHandleW(L"haloreach.dll");
    if (!hReach) return;
    const uintptr_t base = reinterpret_cast<uintptr_t>(hReach);

    // NOTE: the visor/material rewrite below is map-wide. In Armory-style maps it tints every Spartan using those
    // materials; in campaign and normal match maps the scan currently finds no map-local rmsh materials (Spartan
    // materials are in the shared tag heap), so extended visor colours still fall back to the stock slot chosen by
    // ArmoryVisorStockIndex. See REFERENCE-LIBRARY-2026-11-15.md section 5a before changing this.
    bool liveSynchronousSim = false;
    const bool hasLiveFlags = ReadEngineModeFlagsSEH(base, nullptr, &liveSynchronousSim);

    const ReachArmorySelection sel = GetSelection();
    {   // every match, also without custom colours; only the local player's own appearance copy (0 = none yet)
        const uint32_t localUnit = LocalUnitDatum(base);
        ApplyLocalEmblem(sel, localUnit == 0xFFFFFFFFu ? 0u : localUnit);
    }
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
    // Extended finishes have no engine visor slot (only stock slots 0..4 exist; an invalid index renders white), so
    // their exact colour has to come from the map shader patch, exactly like the Armory preview does. This is map-wide:
    // every Spartan in the loaded map gets the local player's visor tint. That is the cost of custom finishes until the
    // tint is replicated per player.
    if (now - s_lastTagUpdate > 250) {
        s_lastTagUpdate = now;
        UpdateTagShaderConstantsSEH(
            sel.previewElite,
            words.data(),
            sel.accentCustom ? sel.accentRgb.data() : nullptr,
            nullptr,   // undersuit: never painted (see WriteColorsAndSuit)
            customVisor ? visFloats : nullptr);
    }

    static bool s_loggedVisorMode = false;
    if (customVisor && !s_loggedVisorMode) {
        s_loggedVisorMode = true;
        const int engineSlot = ArmoryVisorStockIndex(sel.visorColorIndex, sel.visor);
        NOBLE_INFO("ReachArmory: visor '{}' (catalog {}) -> stock engine slot {}; local change colour 3 ({:.2f},{:.2f},{:.2f}); visor shader tint on (map-wide)",
                   sel.visor, sel.visorColorIndex, engineSlot, visFloats[0], visFloats[1], visFloats[2]);
    }

    // Never mutate deterministic object-pool datums locally in networked synchronous lockstep modes
    // (Campaign / Firefight Co-Op), as asymmetric game_state object writes trigger CRC Out-Of-Sync.
    if (NetworkTransport::Get().IsActive() && (!hasLiveFlags || liveSynchronousSim)) {
        return;
    }

    const float slot0[3] = { sel.primaryRgb[0], sel.primaryRgb[1], sel.primaryRgb[2] };
    const float slot1[3] = { sel.secondaryRgb[0], sel.secondaryRgb[1], sel.secondaryRgb[2] };
    const float slot2[3] = { sel.accentRgb[0], sel.accentRgb[1], sel.accentRgb[2] };
    const float slot3[3] = { visFloats[0], visFloats[1], visFloats[2] };
    // Elite slots 2/3 are the game's fixed Elite colours (EliteFixedChangeColorsSEH); never force accent/visor there.
    const float* const wantSlots[4] = {
        sel.primaryCustom ? slot0 : nullptr,
        sel.secondaryCustom ? slot1 : nullptr,
        (sel.accentCustom && !sel.isElite) ? slot2 : nullptr,
        (customVisor && !sel.isElite) ? slot3 : nullptr
    };

    SyncMatchLiveBipedsSEH(base, wantSlots, LocalUnitDatum(base));
}

// =====================================================================================================================
// Marine / ODST pieces (2026-11-13). Mods such as Improved Trooper Firefight make the player a Marine
// (objects\characters\marine\marine). The Armory's Marine tab saves {region, permutation} picks; here they are put on
// the LOCAL player's biped with the engine's own set-permutation call, the way the menu does it for the Spartan.
// String ids are map-specific, so they are read from the scenario map file (header +0x30.. string tables, the same
// layout tools\map-slim.ps1 / sidcsv.py use for the menu map). Networked sessions are skipped: Firefight/campaign
// co-op are lockstep and a local-only change to the object pool would desync them.
// =====================================================================================================================
namespace {
std::mutex s_marineMutex;
std::string s_marineMapPath;                              // last scenario map resolved by slot 48
std::string s_marineSidMapPath;                           // map the table below was read from
std::unordered_map<std::string, uint32_t> s_marineSids;   // only the names the Marine code asks for
bool s_marineSidsTried = false;
}

void ReachArmory::NoteMapPath(const std::string& path) {
    std::string name = ToLowerAscii(std::filesystem::path(path).filename().string());
    if (name == "shared.map" || name == "campaign.map" || name == "mainmenu.map" || name == "noble_menu.map" ||
        name.find("elite_pack") != std::string::npos || name.find("mainmenu_all") != std::string::npos) return;
    std::lock_guard<std::mutex> lock(s_marineMutex);
    if (s_marineMapPath != path) {
        s_marineMapPath = path;
        NOBLE_INFO("MarineArmor: scenario map file {}", path);
    }
}

// Reads the string tables of `path` and keeps the ids of `wanted` (lower case). Header: +0x30 string count,
// +0x34 string data offset, +0x38 data size, +0x3C index offset, +0x40 namespace count, +0x44 namespace table offset.
// A string's id: namespace k (k>0) -> (k << 19) | (g - start of namespace k); the global block maps as in sidcsv.py.
static size_t ReadMapStringIds(const std::string& path, const std::vector<std::string>& wanted,
                               std::unordered_map<std::string, uint32_t>& out) {
    std::ifstream f(std::filesystem::u8path(path), std::ios::binary);
    if (!f) return 0;
    std::vector<char> h(0x48);
    if (!f.read(h.data(), h.size())) return 0;
    auto i32 = [&](size_t o) { int32_t v; std::memcpy(&v, h.data() + o, 4); return v; };
    const int32_t count = i32(0x30), dataOff = i32(0x34), dataLen = i32(0x38), idxOff = i32(0x3C);
    const int32_t nsCount = i32(0x40), nsOff = i32(0x44);
    if (count <= 0 || count > 400000 || dataLen <= 0 || dataLen > 64 * 1024 * 1024 || nsCount <= 0 || nsCount > 64) return 0;
    std::vector<int32_t> idx(static_cast<size_t>(count)), ns(static_cast<size_t>(nsCount));
    std::vector<char> data(static_cast<size_t>(dataLen));
    f.seekg(idxOff); f.read(reinterpret_cast<char*>(idx.data()), static_cast<std::streamsize>(idx.size() * 4));
    f.seekg(nsOff); f.read(reinterpret_cast<char*>(ns.data()), static_cast<std::streamsize>(ns.size() * 4));
    f.seekg(dataOff); f.read(data.data(), dataLen);
    if (!f) return 0;
    int64_t total = 0;
    for (int32_t n : ns) total += n;
    auto sidOf = [&](int64_t g) -> uint32_t {
        if (g < ns[0]) return static_cast<uint32_t>(g);
        if (g >= total) return static_cast<uint32_t>(g - total + ns[0]);
        int64_t base = ns[0];
        size_t k = 1;
        while (k < ns.size() && g >= base + ns[k]) { base += ns[k]; ++k; }
        return static_cast<uint32_t>((k << 19) | static_cast<uint64_t>(g - base));
    };
    size_t found = 0;
    for (int32_t g = 0; g < count; ++g) {
        const int32_t o = idx[static_cast<size_t>(g)];
        if (o < 0 || o >= dataLen) continue;
        const char* s = data.data() + o;
        const size_t maxLen = static_cast<size_t>(dataLen - o);
        const size_t len = strnlen(s, maxLen);
        const std::string name(s, len);
        if (std::find(wanted.begin(), wanted.end(), name) == wanted.end()) continue;
        if (out.emplace(name, sidOf(g)).second) ++found;
    }
    return found;
}

// Region index of `regionSid` on the object's model, or -1 (haloreach+0x1CBF0C, the lookup set-permutation uses).
static int32_t MarineRegionIndexSEH(uintptr_t base, uint32_t unitDatum, uint32_t regionSid) {
    __try {
        const uint32_t tlsIdx = *reinterpret_cast<const uint32_t*>(base + 0xC17B18);
        const uintptr_t tlsArray = *reinterpret_cast<const uintptr_t*>(__readgsqword(0x58) + static_cast<uintptr_t>(tlsIdx) * 8u);
        if (!tlsArray) return -1;
        const uint8_t* objTable = *reinterpret_cast<const uint8_t* const*>(tlsArray + 0x10);
        if (!objTable) return -1;
        const uint8_t* entries = *reinterpret_cast<const uint8_t* const*>(objTable + 0x50);
        const int32_t maxObjects = *reinterpret_cast<const int32_t*>(objTable + 0x44);
        const int32_t i = static_cast<int32_t>(unitDatum & 0xFFFFu);
        if (!entries || i < 0 || i >= maxObjects) return -1;
        const uint8_t* entry = entries + static_cast<size_t>(i) * 0x18;
        if (*reinterpret_cast<const uint16_t*>(entry) != static_cast<uint16_t>(unitDatum >> 16)) return -1;
        const uint8_t* obj = *reinterpret_cast<const uint8_t* const*>(entry + 0x10);
        if (!obj) return -1;
        const uint16_t defIdx = *reinterpret_cast<const uint16_t*>(obj);
        const uint8_t* tagInstances = *reinterpret_cast<const uint8_t* const*>(base + 0xC1A600);
        if (!tagInstances) return -1;
        const uint32_t packed = *reinterpret_cast<const uint32_t*>(tagInstances + static_cast<size_t>(defIdx) * 8 + 4);
        const uint64_t* pages = reinterpret_cast<const uint64_t*>(base + 0x4E39F20);
        const uint8_t* def = reinterpret_cast<const uint8_t*>(pages[packed >> 28] + static_cast<uint64_t>(packed) * 4);
        const uint32_t model = *reinterpret_cast<const uint32_t*>(def + 0x70);
        if (model == 0xFFFFFFFFu) return -1;
        using RegionIndexFn = int32_t(__fastcall*)(uint32_t, uint32_t);
        return reinterpret_cast<RegionIndexFn>(base + 0x1CBF0C)(model, regionSid);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return -1;
    }
}

static bool MarineApplySEH(uintptr_t base, uint32_t unit, const uint32_t* pairs, size_t pairCount, const float* pri, const float* sec, const float* c3, const float* c4, uint8_t* results, uint32_t variantSid) {
    __try {
        // haloreach+0x472B70 = object_set_variant(object, variant string id): what the menu script runs before its
        // object_set_permutation calls (hs evaluator haloreach+0x19C0B8, 2026-11-13).
        using SetVariantFn = void(__fastcall*)(uint32_t, uint32_t);
        if (variantSid) reinterpret_cast<SetVariantFn>(base + 0x472B70)(unit, variantSid);
        using SetRegionPermutationFn = uint8_t(__fastcall*)(uint32_t, uint32_t, uint32_t, uint8_t);
        using SetChangeColorFn = uint8_t(__fastcall*)(uint32_t, int32_t, const float*);
        auto SetRegionPermutation = reinterpret_cast<SetRegionPermutationFn>(base + 0x4729F4);
        auto SetChangeColor = reinterpret_cast<SetChangeColorFn>(base + 0x473858);
        for (size_t p = 0; p < pairCount; ++p) {
            const uint8_t rc = SetRegionPermutation(unit, pairs[p * 2], pairs[p * 2 + 1], 1);
            if (results && p < 16) results[p] = rc;
        }
        if (pri) SetChangeColor(unit, 0, pri);
        if (sec) SetChangeColor(unit, 1, sec);
        if (c3) SetChangeColor(unit, 2, c3);
        if (c4) SetChangeColor(unit, 3, c4);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

// 2026-11-13: Marine pieces must be set on the SIMULATION thread. Each frame haloreach+0x1CD2BC copies the whole game
// state from the update thread's buffer over the render thread's copy (proven with a write breakpoint: VCRUNTIME memcpy
// called from +0x1CD389 wrote the variant's piece back right after our render-thread call). SyncMarine queues the
// pairs; ReachArmory::NotifyMatchTick (FillLocalInput, simulation thread) applies them.
static std::mutex s_simMarineMutex;
static uint32_t s_simMarineUnit = 0xFFFFFFFFu;
static std::vector<uint32_t> s_simMarinePairs;
static std::atomic<int> s_simMarineTicks{0};
static std::atomic<uint64_t> s_simMarineApplied{0};

static uint32_t LocalUnitDatumNow(uintptr_t base) {
    using GetLocalUnitFn = int32_t(__fastcall*)(int32_t);
    return static_cast<uint32_t>(reinterpret_cast<GetLocalUnitFn>(base + 0x53EF8)(0));
}

static uint32_t LocalUnitDatumNowSEH(uintptr_t base) {
    __try { return LocalUnitDatumNow(base); } __except (EXCEPTION_EXECUTE_HANDLER) { return 0xFFFFFFFFu; }
}

// The object's definition (first dword of the object data = its biped tag datum), or 0xFFFFFFFF.
static uint32_t UnitDefinitionSEH(uintptr_t base, uint32_t unit) {
    __try {
        const uint32_t tlsIdx = *reinterpret_cast<const uint32_t*>(base + 0xC17B18);
        const uintptr_t tlsArray = *reinterpret_cast<const uintptr_t*>(__readgsqword(0x58) + static_cast<uintptr_t>(tlsIdx) * 8u);
        uint8_t* objTable = *reinterpret_cast<uint8_t* const*>(tlsArray + 0x10);
        uint8_t* entries = *reinterpret_cast<uint8_t* const*>(objTable + 0x50);
        const uint8_t* obj = *reinterpret_cast<uint8_t* const*>(entries + static_cast<size_t>(unit & 0xFFFFu) * 0x18 + 0x10);
        return obj ? *reinterpret_cast<const uint32_t*>(obj) : 0xFFFFFFFFu;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return 0xFFFFFFFFu;
    }
}

void ReachArmory::SyncMarine() {
    static ULONGLONG s_next = 0;
    const ULONGLONG now = GetTickCount64();
    if (now < s_next) return;
    s_next = now + 100;

    ReachArmorySelection sel = GetSelection();
    // Dev test (2026-11-13): env NOBLE_MARINE_TEST="helmet=odst_light;flair_backpack=odst_jetpack;color=1,0,0;color2=0,0,1"
    // replaces the saved Marine picks.
    if (const char* test = std::getenv("NOBLE_MARINE_TEST")) {
        sel.marinePieces.clear();
        std::stringstream ss(test);
        std::string item;
        while (std::getline(ss, item, ';')) {
            const size_t eq = item.find('=');
            if (eq == std::string::npos) continue;
            const std::string k = item.substr(0, eq), v = item.substr(eq + 1);
            float r = 0, g = 0, b = 0;
            if ((k == "color" || k == "color2") && sscanf_s(v.c_str(), "%f,%f,%f", &r, &g, &b) == 3) {
                sel.marineColors = true;
                (k == "color" ? sel.marinePrimaryRgb : sel.marineSecondaryRgb) = { r, g, b };
            } else {
                sel.marinePieces.emplace_back(k, v);
            }
        }
    }
    if (sel.marinePieces.empty() && sel.femalePieces.empty() && !sel.marineColors) return;
    // Solo Firefight still runs Noble's transport as a host with no peers; only skip when other players are in.
    if (NetworkTransport::Get().IsActive() &&
        (!NetworkTransport::Get().IsHost() || NetworkTransport::Get().GetGamePeerCount() > 0)) {
        static bool s_logged = false;
        if (!s_logged) { s_logged = true; NOBLE_INFO("MarineArmor: other players in the session - Marine pieces are not applied (lockstep safety)"); }
        return;
    }
    HMODULE hReach = GetModuleHandleW(L"haloreach.dll");
    if (!hReach) return;
    const uintptr_t base = reinterpret_cast<uintptr_t>(hReach);

    // String ids of every name we may need, read once per map.
    std::string mapPath;
    { std::lock_guard<std::mutex> lock(s_marineMutex); mapPath = s_marineMapPath; }
    if (mapPath.empty()) return;
    if (mapPath != s_marineSidMapPath) {
        s_marineSidMapPath = mapPath;
        s_marineSids.clear();
        s_marineSidsTried = false;
    }
    if (!s_marineSidsTried) {
        s_marineSidsTried = true;
        std::vector<std::string> wanted = { "helmet", "flair_backpack" };
        wanted.push_back("flair_helmet_a");
        for (const auto& [r, p] : sel.marinePieces) { wanted.push_back(r); wanted.push_back(p); }
        for (const auto& [r, p] : sel.femalePieces) { wanted.push_back(r); wanted.push_back(p); }
        const size_t n = ReadMapStringIds(mapPath, wanted, s_marineSids);
        NOBLE_INFO("MarineArmor: {} of {} names found in {}", n, wanted.size(), mapPath);
        {   // 2026-11-13: the ids themselves (compare with Reclaimer: tools\rc_meshidx.fsx)
            std::string ids;
            for (const auto& [k, v] : s_marineSids) { char buf[24]; snprintf(buf, sizeof(buf), "=0x%X", v); ids += " " + k + buf; }
            NOBLE_INFO("MarineArmor: string ids{}", ids);
        }
    }
    auto sid = [](const std::string& name) -> uint32_t {
        auto it = s_marineSids.find(name);
        return it == s_marineSids.end() ? 0u : it->second;
    };

    const uint32_t unit = LocalUnitDatumNowSEH(base);
    if (unit == 0xFFFFFFFFu) return;

    // Apply for ~3 s after a new body (spawn / respawn) or new picks, then leave it alone.
    static uint32_t s_lastUnit = 0xFFFFFFFFu;
    static std::vector<std::pair<std::string, std::string>> s_lastPieces, s_lastFemale;
    static std::array<float, 3> s_lastPri{}, s_lastSec{};
    static bool s_lastColors = false;
    static int s_ticksLeft = 0;
    static int s_isMarine = -1;
    if (unit != s_lastUnit || sel.marinePieces != s_lastPieces || sel.femalePieces != s_lastFemale || sel.marineColors != s_lastColors ||
        sel.marinePrimaryRgb != s_lastPri || sel.marineSecondaryRgb != s_lastSec) {
        if (unit != s_lastUnit) s_isMarine = -1;
        s_lastUnit = unit; s_lastPieces = sel.marinePieces; s_lastFemale = sel.femalePieces; s_lastColors = sel.marineColors;
        s_lastPri = sel.marinePrimaryRgb; s_lastSec = sel.marineSecondaryRgb;
        s_ticksLeft = 30;
    }
    if (s_ticksLeft <= 0) return;
    if (s_ticksLeft > 0) --s_ticksLeft;

    if (s_isMarine < 0) {
        const uint32_t helmetSid = sid("helmet"), backpackSid = sid("flair_backpack");
        s_isMarine = (helmetSid && backpackSid && MarineRegionIndexSEH(base, unit, helmetSid) >= 0 &&
                      MarineRegionIndexSEH(base, unit, backpackSid) >= 0) ? 1 : 0;
        // Female marine (objects\characters\marine_female): the only one with a flair_helmet_a region.
        const uint32_t helmetASid = sid("flair_helmet_a");
        if (s_isMarine && helmetASid && MarineRegionIndexSEH(base, unit, helmetASid) >= 0) s_isMarine = 2;
        NOBLE_INFO("MarineArmor: local unit 0x{:08X} (biped 0x{:08X}) is {}", unit, UnitDefinitionSEH(base, unit),
                   s_isMarine == 2 ? "a female Marine" : (s_isMarine ? "a Marine" : "not a Marine"));
    }
    if (s_isMarine != 1 && s_isMarine != 2) return;

    const auto& pieces = s_isMarine == 2 ? sel.femalePieces : sel.marinePieces;
    std::vector<uint32_t> pairs;
    for (const auto& [r, p] : pieces) {
        const uint32_t rs = sid(r), ps = sid(p);
        if (rs && ps) { pairs.push_back(rs); pairs.push_back(ps); }
    }
    const float pri[3] = { sel.marinePrimaryRgb[0], sel.marinePrimaryRgb[1], sel.marinePrimaryRgb[2] };
    const float sec[3] = { sel.marineSecondaryRgb[0], sel.marineSecondaryRgb[1], sel.marineSecondaryRgb[2] };
    const float c3[3] = { sel.marineThirdRgb[0], sel.marineThirdRgb[1], sel.marineThirdRgb[2] };
    const float c4[3] = { sel.marineFourthRgb[0], sel.marineFourthRgb[1], sel.marineFourthRgb[2] };
    {   // queue for the simulation thread (see s_simMarineMutex)
        std::lock_guard<std::mutex> lock(s_simMarineMutex);
        s_simMarineUnit = unit;
        s_simMarinePairs = pairs;
    }
    s_simMarineTicks.store(60);
    uint8_t results[16] = {};
    // Pieces: simulation thread (NotifyMatchTick). Here (render thread) only the colours.
    const bool ok = MarineApplySEH(base, unit, pairs.data(), 0, sel.marineColors ? pri : nullptr,
                                   sel.marineColors ? sec : nullptr, sel.marineColors ? c3 : nullptr, sel.marineColors ? c4 : nullptr, results, 0);
    if (s_ticksLeft == 29) {
        NOBLE_INFO("MarineArmor: applied {} of {} piece(s){} to 0x{:08X}{}", pairs.size() / 2, pieces.size(),
                   sel.marineColors ? " + colours" : "", unit, ok ? "" : " (FAILED)");
        // 2026-11-13: per-piece result of the engine call (helmet/chest/legs did not change in Trooper Firefight).
        std::string rcs;
        size_t i = 0;
        for (const auto& [r, p] : pieces) {
            if (i >= pairs.size() / 2 || i >= 16) break;
            if (!sid(r) || !sid(p)) { rcs += " " + r + "=" + p + ":nosid"; continue; }
            rcs += " " + r + "=" + p + ":" + std::to_string(results[i]); ++i;
        }
        NOBLE_INFO("MarineArmor: results{}", rcs);
    }
}

// haloreach+0x4F9184 = hs unit_open: object+0x3B8 = 1.0f, then +0x46C854(unit). hs unit_close (evaluator +0x19DD0C) does
// the same with 0. The flashlight lights are attached with function unit_open -> flashlight_new.
static bool FlashlightSetSEH(uintptr_t base, uint32_t unit, bool on, bool notify) {
    __try {
        const uint32_t tlsIdx = *reinterpret_cast<const uint32_t*>(base + 0xC17B18);
        const uintptr_t tlsArray = *reinterpret_cast<const uintptr_t*>(__readgsqword(0x58) + static_cast<uintptr_t>(tlsIdx) * 8u);
        if (!tlsArray) return false;
        uint8_t* objTable = *reinterpret_cast<uint8_t* const*>(tlsArray + 0x10);
        if (!objTable) return false;
        uint8_t* entries = *reinterpret_cast<uint8_t* const*>(objTable + 0x50);
        if (!entries) return false;
        uint8_t* obj = *reinterpret_cast<uint8_t* const*>(entries + static_cast<size_t>(unit & 0xFFFFu) * 0x18 + 0x10);
        if (!obj) return false;
        // The game resets this every update in matches (seen: 0.00 one second after a toggle), so while the light is
        // on it is written every frame; the change notify only on the toggle itself.
        *reinterpret_cast<float*>(obj + 0x3B8) = on ? 1.0f : 0.0f;
        if (notify) {
            using UnitOpenChangedFn = void(__fastcall*)(uint32_t);
            reinterpret_cast<UnitOpenChangedFn>(base + 0x46C854)(unit);
        }
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static float FlashlightValueSEH(uintptr_t base, uint32_t unit) {
    __try {
        const uint32_t tlsIdx = *reinterpret_cast<const uint32_t*>(base + 0xC17B18);
        const uintptr_t tlsArray = *reinterpret_cast<const uintptr_t*>(__readgsqword(0x58) + static_cast<uintptr_t>(tlsIdx) * 8u);
        uint8_t* objTable = *reinterpret_cast<uint8_t* const*>(tlsArray + 0x10);
        uint8_t* entries = *reinterpret_cast<uint8_t* const*>(objTable + 0x50);
        uint8_t* obj = *reinterpret_cast<uint8_t* const*>(entries + static_cast<size_t>(unit & 0xFFFFu) * 0x18 + 0x10);
        return obj ? *reinterpret_cast<const float*>(obj + 0x3B8) : -1.0f;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return -2.0f;
    }
}

void ReachArmory::TickFlashlight() {
    // Dev check: the open value one and three seconds after a toggle (does the game keep it?).
    static ULONGLONG s_checkAt[2] = { 0, 0 };
    static uint32_t s_checkUnit = 0xFFFFFFFFu;
    for (auto& at : s_checkAt) {
        if (at && GetTickCount64() >= at) {
            at = 0;
            HMODULE h = GetModuleHandleW(L"haloreach.dll");
            if (h) NOBLE_INFO("Flashlight: open value now {:.2f}", FlashlightValueSEH(reinterpret_cast<uintptr_t>(h), s_checkUnit));
        }
    }
    static bool s_on = false;
    static uint32_t s_onUnit = 0xFFFFFFFFu;
    static bool s_wasDown = false;
    const bool down = (GetAsyncKeyState('L') & 0x8000) != 0;
    const bool pressed = down && !s_wasDown;
    s_wasDown = down;
    if (!pressed) {
        if (s_on) {   // keep it on (new body after a respawn = light off again)
            HMODULE hR = GetModuleHandleW(L"haloreach.dll");
            const uintptr_t b = reinterpret_cast<uintptr_t>(hR);
            if (!hR || LocalUnitDatumNowSEH(b) != s_onUnit) { s_on = false; return; }
            FlashlightSetSEH(b, s_onUnit, true, false);
        }
        return;
    }
    // Only for this game window (not while typing elsewhere) and not in the menus (no local unit there).
    DWORD pid = 0;
    GetWindowThreadProcessId(GetForegroundWindow(), &pid);
    if (pid != GetCurrentProcessId()) return;
    // Same lockstep caution as the Marine pieces: a local-only state change, so not with other players in.
    if (NetworkTransport::Get().IsActive() &&
        (!NetworkTransport::Get().IsHost() || NetworkTransport::Get().GetGamePeerCount() > 0)) return;
    HMODULE hReach = GetModuleHandleW(L"haloreach.dll");
    if (!hReach) return;
    const uintptr_t base = reinterpret_cast<uintptr_t>(hReach);
    const uint32_t unit = LocalUnitDatumNowSEH(base);
    if (unit == 0xFFFFFFFFu) return;
    const bool on = !(s_on && s_onUnit == unit);
    if (FlashlightSetSEH(base, unit, on, true)) {
        s_on = on; s_onUnit = unit;
        NOBLE_INFO("Flashlight: {} (unit 0x{:08X})", on ? "on" : "off", unit);
        s_checkUnit = unit; s_checkAt[0] = GetTickCount64() + 1000; s_checkAt[1] = GetTickCount64() + 3000;
    }
}

void ReachArmory::NotifyMatchTick() {
    if (s_simMarineTicks.load(std::memory_order_relaxed) <= 0) return;
    s_simMarineTicks.fetch_sub(1);
    uint32_t unit; std::vector<uint32_t> pairs;
    {
        std::lock_guard<std::mutex> lock(s_simMarineMutex);
        unit = s_simMarineUnit; pairs = s_simMarinePairs;
    }
    if (unit == 0xFFFFFFFFu || pairs.empty()) return;
    HMODULE hReach = GetModuleHandleW(L"haloreach.dll");
    if (!hReach) return;
    MarineApplySEH(reinterpret_cast<uintptr_t>(hReach), unit, pairs.data(), pairs.size() / 2, nullptr, nullptr, nullptr, nullptr,
                   nullptr, 0);
    if (s_simMarineApplied.fetch_add(1) == 0) NOBLE_INFO("MarineArmor: pieces applied on the simulation thread (thread {})", GetCurrentThreadId());
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
