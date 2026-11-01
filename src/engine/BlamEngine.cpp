#include "BlamEngine.hpp"
#include "ReachHost.hpp"
#include "network/NetworkTransport.hpp"
#include "network/MasterServerClient.hpp"
#include "ui/LoadScreen.hpp"
#include "career/Career.hpp"
#include "vfs/VFS.hpp"
#include "hooks/HookManager.hpp"
#include "hooks/LobbyPatches.hpp"
#include "common/Logger.hpp"
#include <algorithm>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <fstream>
#include <filesystem>
#include <objbase.h>
#include <shellapi.h>
#pragma comment(lib, "ole32.lib")

namespace noble {
static void SafeCallVariantInit(HMODULE hModule, void* targetBuffer) {
    if (!hModule || !targetBuffer) return;
    __try {
        typedef void (*InitDefaultVariantFn)(void* buffer);
        auto pInitVariant = reinterpret_cast<InitDefaultVariantFn>(
            reinterpret_cast<uintptr_t>(hModule) + 0x6BFF8);
        pInitVariant(targetBuffer);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
    }
}

// Game variant decoder inside haloreach.dll (RVA 0x373A0, valid for the supported DLL build).
// Same call Reach Renaissance's set_game_variant makes: ok = decode(fileBytes, out[0xFC04], &status).
static bool SafeCallGameVariantDecode(HMODULE hModule, const void* fileData, void* outVariant, void* outStatus) {
    if (!hModule || !fileData || !outVariant || !outStatus) return false;
    __try {
        typedef char (*DecodeGameVariantFn)(const void* fileData, void* outVariant, void* outStatus);
        auto pDecode = reinterpret_cast<DecodeGameVariantFn>(
            reinterpret_cast<uintptr_t>(hModule) + 0x373A0);
        return pDecode(fileData, outVariant, outStatus) != 0;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static bool SafeCallMapVariantParse(HMODULE hModule, const void* fileData, size_t fileSize, void* outVariant, char* outStatus) {
    if (!hModule || !fileData || !outVariant) return false;
    __try {
        typedef char (*ParseMapVariantFn)(const void* fileData, size_t fileSize, void* outVariant, char* outStatus);
        auto pParse = reinterpret_cast<ParseMapVariantFn>(
            reinterpret_cast<uintptr_t>(hModule) + 0x17AF00);
        return pParse(fileData, fileSize, outVariant, outStatus) != 0;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

BlamEngine::BlamEngine() = default;

BlamEngine::~BlamEngine() {
    Shutdown();
}

bool BlamEngine::Initialize(const ReachInstallInfo& installInfo, HMODULE hModule, const ReachExports& exports, const EngineHostContext& context) {
    s_active = this;
    if (m_isInitialized) return true;

    m_hModule = hModule;
    m_exports = exports;
    m_context = context;

    NOBLE_INFO("Instantiating Blam! Reach Game Engine interface...");

    // 1. Pass Library Settings FIRST (as discovered in host-probe main.c lines 3439-3490)
    // haloreach.dll requires a 510-byte (0x1FE) structure with UTF-16 strings for paths
    if (m_exports.SetLibrarySettings) {
        NOBLE_INFO("Configuring haloreach.dll with SetLibrarySettings (3 x \"en-US\" locale strings)...");
        ReachLibrarySettings settings{};
        wcsncpy_s(settings.locale0, L"en-US", _TRUNCATE);
        wcsncpy_s(settings.locale1, L"en-US", _TRUNCATE);
        wcsncpy_s(settings.locale2, L"en-US", _TRUNCATE);

        m_exports.SetLibrarySettings(&settings);
        NOBLE_INFO("SetLibrarySettings initialized successfully.");
    }

    // 2. Create Game Engine Interface (CreateDataAccess removed per rr-launch-flow spec)
    if (!m_exports.CreateGameEngine) {
        NOBLE_ERROR("CreateGameEngine export not available in haloreach.dll!");
        return false;
    }

    typedef int32_t (*CreateGameEngineCall)(void** outEngine);
    auto pCreateGameEngine = reinterpret_cast<CreateGameEngineCall>(m_exports.CreateGameEngine);

    int32_t status = pCreateGameEngine(&m_engineInstance);
    NOBLE_INFO("CreateGameEngine returned status: 0x{:X}, Instance: 0x{:X}", 
        status, reinterpret_cast<uintptr_t>(m_engineInstance));

    if (!m_engineInstance) {
        NOBLE_ERROR("Failed to obtain valid c_game_engine instance from haloreach.dll!");
        return false;
    }

    // Resolve virtual method table
    void** vtable = *reinterpret_cast<void***>(m_engineInstance);
    if (vtable) {
        m_fnShutdown       = reinterpret_cast<EngineShutdownFn>(vtable[0]);
        m_fnInitialize     = reinterpret_cast<EngineInitFn>(vtable[1]);
        m_fnStart          = reinterpret_cast<EngineStartFn>(vtable[2]);
        m_fnProcessEvent   = reinterpret_cast<EngineProcessEventFn>(vtable[3]); // vtbl+0x18
        m_fnExecuteCommand = reinterpret_cast<EngineExecuteCommandFn>(vtable[9]);

        NOBLE_INFO("c_game_engine VTable bound successfully:");
        NOBLE_INFO(" - vfunc[0] (Shutdown):       0x{:X}", reinterpret_cast<uintptr_t>(m_fnShutdown));
        NOBLE_INFO(" - vfunc[1] (Initialize):     0x{:X}", reinterpret_cast<uintptr_t>(m_fnInitialize));
        NOBLE_INFO(" - vfunc[2] (Start):          0x{:X}", reinterpret_cast<uintptr_t>(m_fnStart));
        NOBLE_INFO(" - vfunc[3] (ProcessEvent):   0x{:X} (vtbl+0x18)", reinterpret_cast<uintptr_t>(m_fnProcessEvent));
        NOBLE_INFO(" - vfunc[9] (ExecuteCommand): 0x{:X}", reinterpret_cast<uintptr_t>(m_fnExecuteCommand));
    }

    // 3. Init(engine, device, 0, swap_chain, 0)
    if (m_fnInitialize) {
        NOBLE_INFO("Calling c_game_engine Init(engine, device:0x{:X}, 0, swap_chain:0x{:X}, 0)...",
            reinterpret_cast<uintptr_t>(m_context.pDevice),
            reinterpret_cast<uintptr_t>(m_context.pSwapChain));
        int32_t initStatus = m_fnInitialize(m_engineInstance, m_context.pDevice, nullptr, m_context.pSwapChain, nullptr);
        NOBLE_INFO("c_game_engine Init returned: 0x{:X}", initStatus);
    }

    // Configure Host object paths and renderer devices
    ReachHost::Get().SetPaths(installInfo.reachDirectory, installInfo.mapsDirectory, ReachHost::Get().GetDataDir());
    ReachHost::Get().SetDeviceAndContext(m_context.pDevice, m_context.pContext);
    // Pool SafeGuards: removed. The null atom arrays they guarded against came from the swap chain
    // missing DXGI_USAGE_SHADER_INPUT (fixed in D3D11Renderer.cpp) and host slot 32 not filling
    // the display-settings buffer (fixed in ReachHost.cpp).
    NOBLE_INFO("BlamEngine: Backbuffer pool set up via swap chain SHADER_INPUT; SafeGuard hooks removed.");

    m_isInitialized = true;
    NOBLE_INFO("BlamEngine host interface emulation initialized and ready for single-pass map launch.");
    return true;
}

// Multiplayer map table: launch index (written at +0x10/+0x14/+0x1D5C0) and map id
// (written at +0x1D334/+0x1D5BC). Values match the engine build Reach Renaissance supports
// (its host-probe set_map table); the main menu uses launch index -2 with map id 1035.
struct ReachMultiplayerMap {
    const char* scenario;
    const char* displayName;
    uint32_t launchIndex;
    uint32_t mapId;
};

static const ReachMultiplayerMap kMultiplayerMaps[] = {
    { "50_panopticon",     "boardwalk",     190, 1035 },
    { "70_boneyard",       "boneyard",      191, 1080 },
    { "45_launch_station", "countdown",     192, 1020 },
    { "30_settlement",     "powerhouse",    193, 1055 },
    { "52_ivory_tower",    "reflection",    194, 1150 },
    { "35_island",         "spire",         195, 1200 },
    { "20_sword_slayer",   "sword base",    196, 1000 },
    { "45_aftship",        "zealot",        197, 1040 },
    { "dlc_slayer",        "anchor 9",      198, 2001 },
    { "dlc_invasion",      "breakpoint",    199, 2002 },
    { "dlc_medium",        "tempest",       200, 2004 },
    { "condemned",         "condemned",     201, 1500 },
    { "trainingpreserve",  "highlands",     202, 1510 },
    { "cex_beaver_creek",  "battle canyon", 203, 10020 },
    { "cex_damnation",     "penance",       204, 10010 },
    { "cex_timberland",    "ridgeline",     205, 10030 },
    { "cex_prisoner",      "solitary",      206, 10070 },
    { "cex_hangemhigh",    "high noon",     207, 10060 },
    { "cex_headlong",      "breakneck",     208, 10050 },
    { "forge_halo",        "forge world",   209, 3006 },
};

static constexpr uint32_t kMainMenuLaunchIndex = static_cast<uint32_t>(-2);
static constexpr uint32_t kMainMenuMapId = 1035; // menu backdrop map id in RR's captured menu block

static uint32_t MapIdForLaunchIndex(uint32_t launchIndex) {
    if (launchIndex == kMainMenuLaunchIndex) return kMainMenuMapId;
    for (const auto& m : kMultiplayerMaps) {
        if (m.launchIndex == launchIndex) return m.mapId;
    }
    return 0;
}

// Resolve a requested game variant (absolute path, file name, or "game_variants\<file>")
// against the places Noble actually receives variants from.
static std::filesystem::path ResolveVariantFile(const std::string& requested) {
    if (requested.empty()) return {};
    std::error_code ec;
    const std::filesystem::path direct(requested);
    if (std::filesystem::is_regular_file(direct, ec)) return direct;

    const auto reachDir = ReachHost::Get().GetReachDir();
    const auto dataDir = ReachHost::Get().GetDataDir();
    const std::filesystem::path name = direct.filename();
    const std::filesystem::path candidates[] = {
        reachDir / "game_variants" / direct,
        reachDir / "game_variants" / name,
        dataDir / "game_variants" / direct,
        dataDir / "game_variants" / name,
        dataDir / "variants" / name,
        std::filesystem::current_path() / direct,
        std::filesystem::current_path() / name,
    };
    for (const auto& candidate : candidates) {
        if (std::filesystem::is_regular_file(candidate, ec)) return candidate;
    }
    return {};
}

// Some stock Reach variants are map-specific even though MCC also ships a generic file
// with the same game type. Invasion is the important case: 3nvasion_054.bin carries no
// compatibility with Boneyard/Spire/Breakpoint and the engine rejects it with
// "map and game incompatible" (EngineEvent reason 4). Substitute the matching stock
// map-specific Invasion variant instead of silently running Slayer.
static std::filesystem::path SelectMapSpecificVariant(const std::filesystem::path& requested, uint32_t mapId) {
    if (requested.empty()) return requested;
    const std::wstring stem = requested.stem().wstring();
    if (_wcsicmp(stem.c_str(), L"3nvasion_054") != 0) return requested;

    const wchar_t* replacement = nullptr;
    switch (mapId) {
        case 1080: replacement = L"3nvasion_boneyard_054.bin"; break; // Boneyard
        case 1200: replacement = L"3nvasion_spire_054.bin";    break; // Spire
        case 2002: replacement = L"3nvasion_dlc_054.bin";      break; // Breakpoint
        default: return requested;
    }

    const std::filesystem::path candidate = ReachHost::Get().GetReachDir() / "game_variants" / replacement;
    std::error_code ec;
    if (std::filesystem::is_regular_file(candidate, ec)) return candidate;
    return requested;
}

uint32_t BlamEngine::ResolveMapId(const std::string& mapName) {
    std::string lower = mapName;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    // Main Menu (-2 / 0xFFFFFFFE in authentic Reach Blam! engine)
    if (lower.find("mainmenu") != std::string::npos || lower == "0" || lower == "menu" || lower == "-2") {
        return static_cast<uint32_t>(-2);
    }

    // Firefight Maps (210 - 219)
    if (lower.find("beachhead") != std::string::npos || lower.find("ff10_prototype") != std::string::npos || lower == "210") return 210;
    if (lower.find("corvette") != std::string::npos || lower.find("ff45_corvette") != std::string::npos || lower == "211") return 211;
    if (lower.find("courtyard") != std::string::npos || lower.find("ff20_courtyard") != std::string::npos || lower == "212") return 212;
    if (lower.find("glacier") != std::string::npos || lower.find("ff60_icecave") != std::string::npos || lower == "213") return 213;
    if (lower.find("holdout") != std::string::npos || lower.find("ff70_holdout") != std::string::npos || lower == "214") return 214;
    if (lower.find("outpost") != std::string::npos || lower.find("ff60_airview") != std::string::npos || lower == "215") return 215;
    if (lower.find("overlook") != std::string::npos || lower.find("ff50_park") != std::string::npos || lower == "216") return 216;
    if (lower.find("waterfront") != std::string::npos || lower.find("ff30_waterfront") != std::string::npos || lower == "217") return 217;
    if (lower.find("unearthed") != std::string::npos || lower.find("ff_unearthed") != std::string::npos || lower == "218") return 218;
    if (lower.find("installation 04") != std::string::npos || lower.find("installation04") != std::string::npos || lower.find("cex_ff_halo") != std::string::npos || lower == "219") return 219;

    // Campaign Missions (178 - 189)
    if (lower.find("noble actual") != std::string::npos || lower.find("m05") != std::string::npos || lower == "178") return 178;
    if (lower.find("winter contingency") != std::string::npos || lower.find("m10") != std::string::npos || lower == "179") return 179;
    if (lower.find("oni: sword base") != std::string::npos || lower.find("m20") != std::string::npos || lower == "180") return 180;
    if (lower.find("nightfall") != std::string::npos || lower.find("m30") != std::string::npos || lower == "181") return 181;
    if (lower.find("tip of the spear") != std::string::npos || lower.find("m35") != std::string::npos || lower == "182") return 182;
    if (lower.find("long night of solace") != std::string::npos || lower.find("m45") != std::string::npos || lower == "183") return 183;
    if (lower.find("exodus") != std::string::npos || lower.find("m50") != std::string::npos || lower == "184") return 184;
    if (lower.find("new alexandria") != std::string::npos || lower.find("m52") != std::string::npos || lower == "185") return 185;
    if (lower.find("the package") != std::string::npos || lower.find("m60") != std::string::npos || lower == "186") return 186;
    if (lower.find("the pillar of autumn") != std::string::npos || lower.find("m70") != std::string::npos || lower == "187") return 187;
    if (lower.find("epilogue") != std::string::npos || lower.find("m70_a") != std::string::npos || lower == "188") return 188;
    if (lower.find("lone wolf") != std::string::npos || lower.find("m70_bonus") != std::string::npos || lower == "189") return 189;

    // Multiplayer / Forge maps (launch index 190-209)
    for (const auto& m : kMultiplayerMaps) {
        if (lower.find(m.scenario) != std::string::npos || lower.find(m.displayName) != std::string::npos ||
            lower == std::to_string(m.launchIndex)) {
            return m.launchIndex;
        }
    }

    return 0;
}

int BlamEngine::s_engineModeOverride = 0;
BlamEngine* BlamEngine::s_active = nullptr;
bool BlamEngine::s_guestPostStart = false;
std::optional<VariantFields> BlamEngine::s_variantOverrides;
std::string BlamEngine::s_variantPatch;

static void ApplyVariantPatch(const std::string& spec, VariantFields& fields) {
    size_t start = 0;
    while (start < spec.size()) {
        const size_t comma = spec.find(',', start);
        const std::string item = spec.substr(start, comma == std::string::npos ? std::string::npos : comma - start);
        start = comma == std::string::npos ? spec.size() : comma + 1;
        const size_t eq = item.find('=');
        if (eq == std::string::npos) continue;
        const std::string key = item.substr(0, eq);
        const int value = std::atoi(item.c_str() + eq + 1);
        auto byte = [&](uint8_t& field, int lo, int hi) {
            field = static_cast<uint8_t>(std::clamp(value, lo, hi));
        };
        if (key == "time") byte(fields.timeLimit, 0, 255);
        else if (key == "rounds") byte(fields.roundsToWin, 0, 15);
        else if (key == "roundlimit") byte(fields.roundLimit, 0, 31);
        else if (key == "sudden") byte(fields.suddenDeath, 0, 127);
        else if (key == "grace") byte(fields.gracePeriod, 0, 31);
        else if (key == "respawn") byte(fields.respawnTime, 0, 255);
        else if (key == "lives") byte(fields.livesPerRound, 0, 63);
        else if (key == "teamLives") byte(fields.teamLivesPerRound, 0, 127);
        else if (key == "traits") byte(fields.traitsDuration, 0, 63);
        else if (key == "speed") byte(fields.speed, 0, 31);
        else if (key == "radar") byte(fields.radarState, 0, 7);
        else if (key == "score") fields.scoreToWin = static_cast<uint16_t>(std::clamp(value, 0, 65535));
    }
}

std::vector<uint8_t> BlamEngine::BuildLaunchBlock(uint32_t mapId, uint32_t gameModeTag, int difficulty, uint64_t skulls,
                                                   const std::string& customMvarPath, const std::string& customVariantPath,
                                                   const std::string& connectAddress) {
    constexpr size_t LAUNCH_BLOCK_SIZE = 0x2BF30; // 180,016 bytes
    std::vector<uint8_t> block(LAUNCH_BLOCK_SIZE, 0);

    // 1. Try loading pre-built launch block from data/blocks/ or data/
    bool loadedPrebuilt = false;
    std::filesystem::path blockPath;

    std::filesystem::path dataRoot = ReachHost::Get().GetDataDir();
    if (!std::filesystem::exists(dataRoot / "launch_block_1.bin")) {
        if (std::filesystem::exists(std::filesystem::path("data") / "launch_block_1.bin")) {
            dataRoot = "data";
        }
    }

    int clampedDiff = (difficulty >= 0 && difficulty <= 3) ? difficulty : 1;

    bool isMainMenu = (mapId == kMainMenuLaunchIndex || mapId == 0 || gameModeTag == 0);
    const bool isGuest = !connectAddress.empty();
    if (!isMainMenu && !NetworkTransport::Get().IsActive()) {
        if (isGuest) {
            std::string tip = "127.0.0.1";
            uint16_t tport = 11774;
            size_t c = connectAddress.rfind(':');
            if (c != std::string::npos) {
                tip = connectAddress.substr(0, c);
                int p = std::atoi(connectAddress.substr(c + 1).c_str());
                if (p > 0 && p <= 65535) tport = static_cast<uint16_t>(p);
            } else {
                tip = connectAddress;
            }
            NetworkTransport::Get().Initialize(false, tip, tport);
        } else {
            NetworkTransport::Get().Initialize(true);
        }
    }

    std::string targetHostIp = "127.0.0.1";
    uint16_t targetHostPort = 11774;
    if (isGuest) {
        size_t colon = connectAddress.rfind(':');
        if (colon != std::string::npos) {
            targetHostIp = connectAddress.substr(0, colon);
            int p = std::atoi(connectAddress.substr(colon + 1).c_str());
            if (p > 0 && p <= 65535) targetHostPort = static_cast<uint16_t>(p);
        } else {
            targetHostIp = connectAddress;
        }
    }

    uint8_t targetOctets[4] = { 127, 0, 0, 1 };
    int to0 = 0, to1 = 0, to2 = 0, to3 = 0;
    if (sscanf_s(targetHostIp.c_str(), "%d.%d.%d.%d", &to0, &to1, &to2, &to3) == 4) {
        targetOctets[0] = static_cast<uint8_t>(to0);
        targetOctets[1] = static_cast<uint8_t>(to1);
        targetOctets[2] = static_cast<uint8_t>(to2);
        targetOctets[3] = static_cast<uint8_t>(to3);
    }

    // Campaign missions: launch index 178 (m05) .. 189 (m70_bonus).
    const bool isCampaign = (mapId >= 178 && mapId <= 189) || (gameModeTag == 1);
    // Firefight maps: launch index 210 .. 219.
    const bool isFirefight = (mapId >= 210 && mapId <= 219) || (gameModeTag == 1035);
    const bool isForge = (gameModeTag == 3);
    uint32_t effectiveMapId = isMainMenu ? static_cast<uint32_t>(-2) : mapId;

    if (isMainMenu) {
        // Authentic Reach Main Menu launch block
        if (std::filesystem::exists(dataRoot / "lb_m_c_1.bin")) {
            blockPath = dataRoot / "lb_m_c_1.bin";
        }
    } else if (mapId >= 210 && mapId <= 219) {
        // Firefight
        blockPath = dataRoot / "blocks" / ("firefight_" + std::to_string(mapId) + "_d" + std::to_string(clampedDiff) + ".bin");
        if (!std::filesystem::exists(blockPath)) {
            blockPath = dataRoot / "blocks" / ("firefight_" + std::to_string(mapId) + ".bin");
        }
    } else if (mapId >= 178 && mapId <= 189) {
        // Campaign
        blockPath = dataRoot / "blocks" / ("campaign_" + std::to_string(mapId) + "_d" + std::to_string(clampedDiff) + ".bin");
        if (!std::filesystem::exists(blockPath)) {
            blockPath = dataRoot / "blocks" / ("campaign_" + std::to_string(mapId) + ".bin");
        }
    }

    // Fallback: If prebuilt campaign or firefight block is missing, synthesize from lb_m_c_1.bin template
    if ((blockPath.empty() || !std::filesystem::exists(blockPath)) && (isCampaign || isFirefight)) {
        if (std::filesystem::exists(dataRoot / "lb_m_c_1.bin")) {
            blockPath = dataRoot / "lb_m_c_1.bin";
            NOBLE_INFO("BlamEngine: Synthesizing launch block from template '{}' for mode {}", blockPath.string(), isCampaign ? "Campaign" : "Firefight");
        }
    }

    if (!blockPath.empty() && std::filesystem::exists(blockPath)) {
        std::ifstream file(blockPath, std::ios::binary);
        if (file) {
            file.read(reinterpret_cast<char*>(block.data()), LAUNCH_BLOCK_SIZE);
            loadedPrebuilt = true;
            NOBLE_INFO("BlamEngine: Loaded authentic pre-built launch block from '{}'", blockPath.string());
        }
    }

    if (!loadedPrebuilt) {
        // Initial header markers (reversed from host-probe::new_launch_block)
        block[0] = isGuest ? 0x00 : 0x08;
        block[1] = 0x03;
        block[2] = 0x00;
        block[3] = 0x10;
        block[4] = 0x03;
        // +0x0C selects the engine class: 1 = campaign, 3 = multiplayer/Megalo (Forge is a
        // multiplayer mode), 5 = Firefight. 0x04 is the special map-editor path, NOT Forge:
        // the engine's mode dispatcher explicitly excludes 0x04 from the normal game-start
        // branch (see haloreach+0xDCCC), so launching Forge with it stalls at launch state 2.
        const int overrideMode = BlamEngine::EngineModeOverride();
        block[0x0C] = static_cast<uint8_t>(overrideMode > 0
                                               ? overrideMode
                                               : ((isMainMenu || isCampaign) ? 0x01 : (isFirefight ? 0x05 : 0x03)));
        if (overrideMode > 0) {
            NOBLE_WARN("BlamEngine: launch-block +0x0C forced to 0x{:X} by -engine-mode (research hook)", overrideMode);
        }
        block[0x10] = isMainMenu ? 0xFE : 0xBE;
        block[0x11] = isMainMenu ? 0xFF : 0x00;
        block[0x12] = isMainMenu ? 0xFF : 0x00;
        block[0x13] = isMainMenu ? 0xFF : 0x00;
        block[0x14] = isMainMenu ? 0xFE : 0xBE;
        block[0x15] = isMainMenu ? 0xFF : 0x00;
        block[0x16] = isMainMenu ? 0xFF : 0x00;
        block[0x17] = isMainMenu ? 0xFF : 0x00;
        block[0x1A] = 0x88;
        block[0x1B] = 0x88;
        block[0x34] = 0x01;
        block[0x44] = 0xFF;
        block[0x45] = 0xFF;

        // Set 16 player slot descriptors to unassigned (-1)
        for (size_t slot = 0; slot < 16; ++slot) {
            size_t offset = 0x100 + (slot * 0x20);
            block[offset + 0] = 0xFF;
            block[offset + 1] = 0xFF;
            block[offset + 2] = 0xFF;
            block[offset + 3] = 0xFF;
            for (size_t b = 8; b < 16; ++b) {
                block[offset + b] = 0xFF;
            }
        }

        uint64_t localMachineId = NetworkTransport::Get().GetLocalMachineId();
        uint64_t hostMachineId = NetworkTransport::Get().GetHostMachineId();

        if (isGuest) {
            // Guest mode: 1 local active player, target host machine at 0x2F8.
            // Note: +0xF0..+0xF7 is the 8-byte s_player_identifier copied by haloreach.dll (State 6,
            // RVA 0xE804) into 0x182A1E94C and sent in join-request. It MUST be unique per client so
            // c_network_session::can_accept_player_join_request (RVA 0x3973BC -> 0x39D59C) on the Host
            // does not see it colliding with Host Player 0 (0x000000003F59F3BE) and reject with 0x16.
            uint64_t guestPlayerId = localMachineId ^ 0x3F59F3BE00000000ULL;
            if (guestPlayerId == 0 || guestPlayerId == 0x000000003F59F3BEULL) {
                guestPlayerId = localMachineId | 0x504C520000000001ULL;
            }
            block[0] &= ~0x08; // bit 3 cleared for guest
            *reinterpret_cast<uint64_t*>(&block[0x60]) = 0; // Host machine id is 0 at 0x60 on guest
            block[0xE8] = 0x01; // 1 active local player
            *reinterpret_cast<uint64_t*>(&block[0xF0]) = guestPlayerId;
            *reinterpret_cast<uint64_t*>(&block[0xF8]) = localMachineId; // Local player 0 machine ID
            // Slot 0 is reserved for remote Host
            block[0x100] = 0xFF;
            block[0x101] = 0xFF;
            block[0x102] = 0xFF;
            block[0x103] = 0xFF;
            block[0x104] = 0x00;
            block[0x108] = 0xFF;
            block[0x109] = 0xFF;
            block[0x10A] = 0xFF;
            block[0x10B] = 0xFF;
            // Slot 1 is bound to local controller 0 for this Guest
            block[0x120] = 0x00;
            block[0x121] = 0x00;
            block[0x122] = 0x00;
            block[0x123] = 0x00;
            block[0x124] = 0x02;
            block[0x128] = 0x00;
            block[0x129] = 0x00;
            block[0x12A] = 0x00;
            block[0x12B] = 0x00;
            // Slots 2..15 are unassigned
            for (size_t s = 2; s < 16; ++s) {
                size_t off = 0x100 + (s * 0x20);
                block[off + 0] = 0xFF;
                block[off + 1] = 0xFF;
                block[off + 2] = 0xFF;
                block[off + 3] = 0xFF;
                block[off + 4] = 0x00;
                for (size_t b = 8; b < 16; ++b) block[off + b] = 0xFF;
            }
            block[0x2F0] = 0x00;
            *reinterpret_cast<uint32_t*>(&block[0x2F4]) = 0xFFFFFFFF;
            *reinterpret_cast<uint64_t*>(&block[0x2F8]) = hostMachineId; // uint64 Host Machine ID
            // Zero variant buffer on guest (0x300 to end) so replicated game and map variants from the host
            // are received and mounted cleanly without local conflict.
            // Note: do NOT memset +0x300..+0x2BF30; block is already zero-initialized and +0x1D308 holds c_map_variant.
            block[0x2BF14] = 0xF3;
            block[0x2BF15] = 0x01;
            NOBLE_INFO("BlamEngine: Configured Guest Launch Block targeting host {}:{} (Local: 0x{:X}, Host: 0x{:X}, PlayerId: 0x{:X})",
                targetHostIp, targetHostPort, localMachineId, hostMachineId, guestPlayerId);
        } else {
            // Host mode: 1 local active player, host machine at 0x60 and 0xF8
            block[0] |= 0x08; // bit 3 set for host
            *reinterpret_cast<uint64_t*>(&block[0x60]) = localMachineId;
            block[0xE8] = 0x01; // 1 active local player
            block[0xF0] = 0xBE;
            block[0xF1] = 0xF3;
            block[0xF2] = 'Y';
            block[0xF3] = '?';
            *reinterpret_cast<uint64_t*>(&block[0xF8]) = localMachineId;
            block[0x100] = 0x00;
            block[0x101] = 0x00;
            block[0x102] = 0x00;
            block[0x103] = 0x00;
            block[0x104] = 0x02;
            block[0x108] = 0x00;
            block[0x109] = 0x00;
            block[0x10A] = 0x00;
            block[0x10B] = 0x00;
            block[0x2F0] = 0x01;
            *reinterpret_cast<uint32_t*>(&block[0x2F4]) = 0;
            *reinterpret_cast<uint64_t*>(&block[0x2F8]) = 0;
        }

        // Default variant buffer init at offset 0x1D308 (RVA 0x6BFF8)
        if (m_hModule) {
            SafeCallVariantInit(m_hModule, &block[0x1D308]);
        }

        // Map ID & Mode tagging (from host-probe::set_map lines 78-92)
        *reinterpret_cast<uint32_t*>(&block[0x10]) = effectiveMapId;
        *reinterpret_cast<uint32_t*>(&block[0x14]) = effectiveMapId;
        *reinterpret_cast<uint32_t*>(&block[0x18]) = 0x88880000;
        *reinterpret_cast<uint64_t*>(&block[0x1C]) = 0;
        *reinterpret_cast<uint32_t*>(&block[0x1D5C0]) = effectiveMapId;
        *reinterpret_cast<uint32_t*>(&block[0x1D5C4]) = 0x88880000;
        *reinterpret_cast<uint64_t*>(&block[0x1D5C8]) = 0;
        // +0x1D334 / +0x1D5BC hold the MAP ID (e.g. Sword Base 1000), not a game-mode tag. With 0 here the
        // engine resolves an empty scenario name ("haloreach\maps\.map") and has nothing to render.
        // Menu and multiplayer ids are known; campaign/firefight keep the previous value until verified.
        uint32_t mapInfoId = MapIdForLaunchIndex(effectiveMapId);
        // Campaign blocks keep the menu/Boardwalk map-variant id (1035) here; the mission itself is
        // selected by the launch index at +0x10/+0x14/+0x1D5C0 (verified against RR's captured
        // campaign blocks, which differ from its MP block only in +0x0C, +0x10/+0x14, +0x28,
        // +0x34 and +0x1D5C0).
        if (isCampaign || isFirefight) mapInfoId = kMainMenuMapId;
        if (mapInfoId == 0) mapInfoId = gameModeTag;
        if (isCampaign) {
            block[0x28] = 0x01;
            block[0x29] = 0x00;
            block[0x2A] = 0x11;
            block[0x2B] = 0x11;
        }
        *reinterpret_cast<uint32_t*>(&block[0x1D334]) = mapInfoId;
        *reinterpret_cast<uint32_t*>(&block[0x1D5BC]) = mapInfoId;
        NOBLE_INFO("BlamEngine: launch block map index {} -> map id {}", static_cast<int32_t>(effectiveMapId), mapInfoId);
    }

    // Synchronized Network Session ID at +0x50 and local machine ID at +0x58
    if (!isMainMenu || !loadedPrebuilt) {
        const uint64_t localMachineId = NetworkTransport::Get().GetLocalMachineId();
        const uint64_t hostMachineId = NetworkTransport::Get().GetHostMachineId();
        *reinterpret_cast<uint64_t*>(&block[0x50]) = NetworkTransport::Get().GetSessionId();
        *reinterpret_cast<uint64_t*>(&block[0x58]) = localMachineId;
        if (!isMainMenu && loadedPrebuilt) {
            if (isGuest) {
                uint64_t guestPlayerId = localMachineId ^ 0x3F59F3BE00000000ULL;
                if (guestPlayerId == 0 || guestPlayerId == 0x000000003F59F3BEULL) {
                    guestPlayerId = localMachineId | 0x504C520000000001ULL;
                }
                block[0] &= ~0x08;
                *reinterpret_cast<uint64_t*>(&block[0x60]) = 0;
                block[0xE8] = 0x01;
                *reinterpret_cast<uint64_t*>(&block[0xF0]) = guestPlayerId;
                *reinterpret_cast<uint64_t*>(&block[0xF8]) = localMachineId;
                // Slot 0 is reserved for remote Host
                block[0x100] = 0xFF;
                block[0x101] = 0xFF;
                block[0x102] = 0xFF;
                block[0x103] = 0xFF;
                block[0x104] = 0x00;
                block[0x108] = 0xFF;
                block[0x109] = 0xFF;
                block[0x10A] = 0xFF;
                block[0x10B] = 0xFF;
                // Slot 1 is bound to local controller 0 for this Guest
                block[0x120] = 0x00;
                block[0x121] = 0x00;
                block[0x122] = 0x00;
                block[0x123] = 0x00;
                block[0x124] = 0x02;
                block[0x128] = 0x00;
                block[0x129] = 0x00;
                block[0x12A] = 0x00;
                block[0x12B] = 0x00;
                block[0x2F0] = 0x00;
                *reinterpret_cast<uint32_t*>(&block[0x2F4]) = 0xFFFFFFFF;
                *reinterpret_cast<uint64_t*>(&block[0x2F8]) = hostMachineId;
                block[0x2BF14] = 0xF3;
                block[0x2BF15] = 0x01;
            } else {
                block[0] |= 0x08;
                *reinterpret_cast<uint64_t*>(&block[0x60]) = localMachineId;
                block[0xE8] = 0x01;
                *reinterpret_cast<uint64_t*>(&block[0xF0]) = 0x000000003F59F3BEULL;
                *reinterpret_cast<uint64_t*>(&block[0xF8]) = localMachineId;
                block[0x100] = 0x00;
                block[0x101] = 0x00;
                block[0x102] = 0x00;
                block[0x103] = 0x00;
                block[0x104] = 0x02;
                block[0x108] = 0x00;
                block[0x109] = 0x00;
                block[0x10A] = 0x00;
                block[0x10B] = 0x00;
                block[0x2F0] = 0x01;
                *reinterpret_cast<uint32_t*>(&block[0x2F4]) = 0;
                *reinterpret_cast<uint64_t*>(&block[0x2F8]) = 0;
            }

            // In authentic prebuilt blocks (Campaign & Firefight), ensure host network listening socket is active
            if (!isGuest) {
                block[0x2BF14] = 0xF3;
                block[0x2BF15] = 0x01;
            }

            // Reset unassigned slots on host and guest so remote peers can be registered
            size_t startSlot = isGuest ? 2 : 1;
            for (size_t slot = startSlot; slot < 16; ++slot) {
                size_t offset = 0x100 + (slot * 0x20);
                block[offset + 0] = 0xFF;
                block[offset + 1] = 0xFF;
                block[offset + 2] = 0xFF;
                block[offset + 3] = 0xFF;
                block[offset + 4] = 0x00;
                block[offset + 5] = 0x00;
                block[offset + 6] = 0x00;
                block[offset + 7] = 0x00;
                for (size_t b = 8; b < 16; ++b) {
                    block[offset + b] = 0xFF;
                }
            }
        }
    }

    if (!isMainMenu) {
        uint32_t mapInfoId = MapIdForLaunchIndex(effectiveMapId);
        if (isCampaign || isFirefight) mapInfoId = kMainMenuMapId;
        if (mapInfoId == 0) mapInfoId = gameModeTag;
        if (isCampaign) {
            block[0x28] = 0x01;
            block[0x29] = 0x00;
            block[0x2A] = 0x11;
            block[0x2B] = 0x11;
        }
        *reinterpret_cast<uint32_t*>(&block[0x1D334]) = mapInfoId;
        *reinterpret_cast<uint32_t*>(&block[0x1D5BC]) = mapInfoId;
    }

    if (isCampaign || isFirefight) {
        // Campaign / Firefight: difficulty only at +0x34 (0 easy .. 3 legendary); +0x38 stays 0 as in RR's blocks.
        block[0x34] = static_cast<uint8_t>(clampedDiff);
    } else if (!isMainMenu) {
        // Apply difficulty override
        block[0x34] = static_cast<uint8_t>(clampedDiff);
        *reinterpret_cast<uint32_t*>(&block[0x38]) = static_cast<uint32_t>(clampedDiff);
    }
    // Skulls: u64 at +0x48 in MCC's skull numbering (bit = the skull's index in MCC's
    // alphabetical list: Iron 18, Black Eye 3, Tough Luck 36, Catch 7, Cloud 13, Famine 11,
    // Thunderstorm 34, Tilt 35, Mythic 23, Blind 4, Cowbell 8, Grunt Birthday Party 16, IWHBYD 19).
    // haloreach+0x3A22C converts it to the engine's own skull bits (game options +0x1D8).
    // (+0x3C, used before, is a different field the engine reads as u16.)
    if (!isMainMenu) {
        memcpy(&block[0x48], &skulls, sizeof(skulls));
    }

    // Dynamic Custom Map Variant (.mvar) Injection (RVA 0x17AF00 parse_map_variant)
    if (!customMvarPath.empty() && std::filesystem::exists(customMvarPath) && m_hModule) {
        NOBLE_INFO("BlamEngine: Parsing custom Forge map variant from '{}'...", customMvarPath);
        std::ifstream file(customMvarPath, std::ios::binary);
        if (file) {
            std::vector<uint8_t> mvarData((std::istreambuf_iterator<char>(file)),
                                           std::istreambuf_iterator<char>());
            if (mvarData.size() >= 0x7329) {
                std::vector<uint8_t> parsedVariant(0x20000, 0); // 128 KB buffer
                SafeCallVariantInit(m_hModule, parsedVariant.data());
                char status = 0;
                if (SafeCallMapVariantParse(m_hModule, mvarData.data(), 0x7329, parsedVariant.data(), &status) && status != 0) {
                    memcpy(&block[0x1D308], parsedVariant.data(), 0xE800);
                    uint32_t mvarMapId = *reinterpret_cast<uint32_t*>(&block[0x1D5C0]);
                    *reinterpret_cast<uint32_t*>(&block[0x10]) = mvarMapId;
                    *reinterpret_cast<uint32_t*>(&block[0x14]) = mvarMapId;
                    NOBLE_INFO("BlamEngine: Custom map variant parsed successfully. Target MapId: {}", mvarMapId);
                } else {
                    NOBLE_WARN("BlamEngine: Failed to parse map variant header (status=0x{:X})", status);
                }
            } else {
                NOBLE_WARN("BlamEngine: Map variant file size ({} bytes) is less than required 0x7329 bytes", mvarData.size());
            }
        }
    }

    // Game variant at +0x300 (0xFC04 bytes), decoded by the engine's own decoder (RVA 0x373A0),
    // as Reach Renaissance does. A raw .bin file is NOT the in-memory layout, so never memcpy it.
    // Without a variant the engine cannot resolve what to load (it requests "haloreach\maps\.map"
    // and stalls at 4%). Default: MCC's stock slayer_054.bin from the player's own install.
    // NOTE: Authentic pre-built Campaign launch blocks already contain authentic campaign state
    // and mission variants at +0x300, and must NOT be overwritten with multiplayer Slayer!
    // The main menu (mainmenu_all.map) never carries a game variant, so it skips this block entirely
    // (before 2026-10-29 it logged a bogus "No game variant found (looked for '')" on every menu boot).
    if (!isMainMenu && (!isCampaign || (!customVariantPath.empty() && customVariantPath != "Campaign" && customVariantPath != "campaign"))) {
        std::filesystem::path variantPath;
        if (!customVariantPath.empty()) {
            variantPath = ResolveVariantFile(customVariantPath);
            const std::filesystem::path requestedPath = variantPath;
            // mapId here is the launch index (190..209); the engine's compatibility field is
            // the map-info id (e.g. Boneyard 1080, Spire 1200, Breakpoint 2002).
            variantPath = SelectMapSpecificVariant(variantPath, MapIdForLaunchIndex(mapId));
            if (!requestedPath.empty() && variantPath != requestedPath) {
                NOBLE_INFO("BlamEngine: '{}' is not compatible with this map; using map-specific '{}' instead",
                           requestedPath.filename().string(), variantPath.filename().string());
            }
            if (variantPath.empty()) {
                NOBLE_WARN("BlamEngine: requested game variant '{}' was not found; refusing to substitute Slayer. "
                           "The match will start with the launch block's embedded mode instead.",
                           customVariantPath);
            }
        } else if (!isCampaign) {
            // Firefight needs a Firefight game variant (otherwise the engine reports "no game
            // selected"); RR likewise loads <MCC>\haloreach\game_variants\<mode>.bin for it.
            // Forge needs 00_basic_editing_054.bin for item spawning and editing.
            // Campaign blocks already contain their own mission variant and are left untouched.
            const char* defaultVariant = isFirefight ? "hr_ff_standard_1set.bin"
                                       : isForge     ? "00_basic_editing_054.bin"
                                       : "slayer_054.bin";
            variantPath = ReachHost::Get().GetReachDir() / "game_variants" / defaultVariant;
        }

        if (!variantPath.empty() && std::filesystem::exists(variantPath) && m_hModule) {
            std::ifstream varFile(variantPath, std::ios::binary);
            std::vector<uint8_t> varData((std::istreambuf_iterator<char>(varFile)),
                                          std::istreambuf_iterator<char>());
            if (varData.size() < 0x5329) {
                NOBLE_WARN("BlamEngine: Game variant '{}' is too small ({} bytes)", variantPath.string(), varData.size());
            } else {
                std::vector<uint8_t> decoded(0xFC04, 0);
                alignas(16) uint8_t status[256] = {};
                if (SafeCallGameVariantDecode(m_hModule, varData.data(), decoded.data(), status)) {
                    VariantFields fields;
                    if (fields.Read(decoded.data(), decoded.size())) {
                        if (!s_variantPatch.empty()) {
                            ApplyVariantPatch(s_variantPatch, fields);
                            fields.Write(decoded.data(), decoded.size());
                            NOBLE_INFO("BlamEngine: applied variant patch '{}' (time {} score {} rounds {})",
                                       s_variantPatch, fields.timeLimit, fields.scoreToWin, fields.roundsToWin);
                        }
                        if (s_variantOverrides) {
                            fields = *s_variantOverrides;
                            fields.Write(decoded.data(), decoded.size());
                            NOBLE_INFO("BlamEngine: applied host variant overrides (time {}, score {}, rounds {})",
                                       fields.timeLimit, fields.scoreToWin, fields.roundsToWin);
                        }
                        NOBLE_INFO("BlamEngine: variant time {} score {} rounds {} respawn {} traits {}",
                                   fields.timeLimit, fields.scoreToWin, fields.roundsToWin,
                                   fields.respawnTime, fields.traitsDuration);
                    }
                    memcpy(&block[0x300], decoded.data(), decoded.size());
                    NOBLE_INFO("BlamEngine: Decoded game variant '{}' into launch block (+0x300)", variantPath.string());
                } else {
                    NOBLE_ERROR("BlamEngine: haloreach.dll could not decode game variant '{}'", variantPath.string());
                }
            }
        } else if (!variantPath.empty()) {
            NOBLE_WARN("BlamEngine: No game variant found for launch block (looked for '{}')", variantPath.string());
        }
    }

    // Test switch "-rr-guest-block": lay out a guest's block the way Reach Renaissance does
    // (host-probe.exe host_probe::net::patch_launch_block, "--as-guest", read 2026-10-29): no local players
    // or player descriptors (+0xE8..+0x10F zero, +0x100 = 0xFFFFFFFF, +0x108 = -1), +0x2F0..+0x2FF zero with
    // +0x2F4 = 0xFFFFFFFF, everything from +0x300 on zeroed (no game variant: the host's comes over the
    // session), +0x2BF14 = F3 01, then +0x50 session, +0x58 self, +0x2F8 host. Noble's default guest block sets
    // one local player, a player id at +0xF0 and the decoded variant instead; guests have been stuck at
    // launch state 9 with traffic flowing both ways, so this lets the two layouts be compared.
    if (isGuest && !isMainMenu && std::wcsstr(GetCommandLineW(), L"-rr-guest-block") != nullptr) {
        const uint64_t self = NetworkTransport::Get().GetLocalMachineId();
        const uint64_t host = NetworkTransport::Get().GetHostMachineId();
        const uint64_t session = NetworkTransport::Get().GetSessionId();
        block[0] &= static_cast<uint8_t>(~0x08);
        *reinterpret_cast<uint64_t*>(&block[0x60]) = 0;
        std::memset(&block[0xE8], 0, 0x110 - 0xE8);
        *reinterpret_cast<uint32_t*>(&block[0x100]) = 0xFFFFFFFFu;
        *reinterpret_cast<uint64_t*>(&block[0x108]) = ~0ULL;
        std::memset(&block[0x2F0], 0, 0x10);
        *reinterpret_cast<uint32_t*>(&block[0x2F4]) = 0xFFFFFFFFu;
        std::memset(&block[0x300], 0, 0x1D308 - 0x300);
        block[0x2BF14] = 0xF3;
        block[0x2BF15] = 0x01;
        *reinterpret_cast<uint64_t*>(&block[0x50]) = session;
        *reinterpret_cast<uint64_t*>(&block[0x58]) = self;
        *reinterpret_cast<uint64_t*>(&block[0x2F8]) = host;
        NOBLE_INFO("BlamEngine: -rr-guest-block: guest block laid out like RR (self 0x{:X}, host 0x{:X}, session 0x{:X})",
                   self, host, session);
    }

    // Test-only: "-launch-poke 0x3C=0xFFFFFFFF,0x40=1" writes u32 values into the block (for finding
    // fields such as skulls by experiment). Not used by the menu.
    {
        int argc = 0;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        for (int i = 1; argv && i + 1 < argc; ++i) {
            if (std::wstring(argv[i]) != L"-launch-poke") continue;
            std::wstring list = argv[i + 1];
            size_t pos = 0;
            while (pos < list.size()) {
                size_t comma = list.find(L',', pos);
                if (comma == std::wstring::npos) comma = list.size();
                const std::wstring item = list.substr(pos, comma - pos);
                const size_t eq = item.find(L'=');
                if (eq != std::wstring::npos) {
                    const unsigned long off = wcstoul(item.substr(0, eq).c_str(), nullptr, 0);
                    const unsigned long val = wcstoul(item.substr(eq + 1).c_str(), nullptr, 0);
                    if (off + 4 <= block.size()) {
                        *reinterpret_cast<uint32_t*>(&block[off]) = static_cast<uint32_t>(val);
                        NOBLE_INFO("BlamEngine: -launch-poke +0x{:X} = 0x{:X}", off, val);
                    }
                }
                pos = comma + 1;
            }
        }
        if (argv) LocalFree(argv);
    }

    return block;
}

bool BlamEngine::ExecuteCommand(const std::string& command) {
    if (!m_engineInstance || !m_fnExecuteCommand) {
        NOBLE_WARN("BlamEngine: Cannot execute command '{}' - Engine not ready.", command);
        return false;
    }

    NOBLE_INFO("BlamEngine: Dispatching command -> '{}'", command);
    int32_t status = m_fnExecuteCommand(m_engineInstance, command.c_str());
    NOBLE_INFO("BlamEngine: Command status: 0x{:X}", status);
    return true;
}

bool BlamEngine::LaunchMap(const std::string& mapName, const std::string& gameMode, int difficulty, uint64_t skulls,
                           const std::string& customMvarPath, const std::string& customVariantPath,
                           const std::string& connectAddress, int expectedPeers) {
    // A fresh launch starts with no remote players in membership; the host's Command 11 barrier counts msg 27s.
    noble::LobbyPatches::ResetRemotePlayerJoined();
    if (!m_isInitialized || !m_engineInstance) return false;

    m_currentMap = mapName;
    m_currentMode = gameMode;
    m_isGameRunning = true;

    NOBLE_INFO("=================================================");
    NOBLE_INFO(" LAUNCHING HALO: REACH MISSION / MAP: {}", mapName);
    NOBLE_INFO(" Mode: {} | Difficulty: {} | Skulls: 0x{:X}", gameMode, difficulty, skulls);
    if (!customMvarPath.empty()) NOBLE_INFO(" Custom MVAR: {}", customMvarPath);
    if (!customVariantPath.empty()) NOBLE_INFO(" Custom Variant: {}", customVariantPath);
    NOBLE_INFO("=================================================");

    uint32_t mapId = ResolveMapId(mapName);

    // DIAGNOSTIC (temporary): NOBLE_MAP_NAME_OVERRIDE=<name> rewrites the engine's own map-name
    // string for the launch index, so the engine builds maps\<name>.map and maps\info\<name>.mapinfo
    // paths. Used to verify that a modded map file can stand in for a stock map slot.
    char* mapNameOverride = nullptr;
    size_t mapNameOverrideLen = 0;
    if (_dupenv_s(&mapNameOverride, &mapNameOverrideLen, "NOBLE_MAP_NAME_OVERRIDE") == 0 && mapNameOverride) {
        if (mapNameOverride[0] != '\0' && m_hModule && mapId >= 178 && mapId <= 219) {
            using MapNameFn = const char* (*)(int);
            auto engineMapName = reinterpret_cast<MapNameFn>(reinterpret_cast<uintptr_t>(m_hModule) + 0x38E98);
            const char* existing = engineMapName(static_cast<int>(mapId));
            if (existing) {
                const size_t len = std::strlen(mapNameOverride);
                DWORD oldProtect = 0;
                if (VirtualProtect(const_cast<char*>(existing), len + 1, PAGE_READWRITE, &oldProtect)) {
                    std::memcpy(const_cast<char*>(existing), mapNameOverride, len + 1);
                    VirtualProtect(const_cast<char*>(existing), len + 1, oldProtect, &oldProtect);
                    NOBLE_WARN("BlamEngine: DIAGNOSTIC map name override for index {} -> '{}'", mapId, mapNameOverride);
                }
            }
        }
        std::free(mapNameOverride);
    }

    uint32_t gameModeTag = 2; // Default multiplayer (Slayer, CTF, Invasion, all Megalo variants)
    const bool forgeMode = _stricmp(gameMode.c_str(), "forge") == 0 ||
                           _stricmp(gameMode.c_str(), "sandbox") == 0;
    if ((mapId >= 210 && mapId <= 219) || _stricmp(gameMode.c_str(), "firefight") == 0) {
        gameModeTag = 1035; // 0x40B Firefight
    } else if (forgeMode) {
        // Forge is a multiplayer Megalo mode, not the special map-editor engine (0x04).
        gameModeTag = 3;
    } else if ((mapId >= 178 && mapId <= 189) || _stricmp(gameMode.c_str(), "campaign") == 0) {
        gameModeTag = 1;    // Campaign
    } else if (mapId == static_cast<uint32_t>(-2) || mapId == 0 ||
               _stricmp(gameMode.c_str(), "mainmenu") == 0 || _stricmp(gameMode.c_str(), "menu") == 0) {
        gameModeTag = 0;    // Main Menu
    }

    // Career pays by base type, so modded maps/variants earn exactly what their stock type earns.
    if (gameModeTag != 0) Career::BeginMatch(Career::BaseTypeFromEngineMode(gameModeTag), mapName, difficulty, skulls);

    auto launchBlock = BuildLaunchBlock(mapId, gameModeTag, difficulty, skulls, customMvarPath, customVariantPath, connectAddress);
    // Campaign/Firefight co-op: the launch block declares how many players this session has (0xE8).
    // A host launched from a lobby with known guests must declare all of them, otherwise the engine only
    // creates the local player and the co-op state machine fails the mission after ~60 s.
    if (gameModeTag == 1 || gameModeTag == 5) {
        int declaredPlayers = 1;
        if (connectAddress.empty() && expectedPeers > 0) declaredPlayers = 1 + expectedPeers;
        launchBlock[0xE8] = static_cast<uint8_t>(declaredPlayers);
        NOBLE_INFO("BlamEngine: launch block declares {} player(s) (0xE8)", declaredPlayers);
    }

    // Request corresponding wireframe loading screen .dots
    LoadScreen::Get().RequestMapDots(mapName, ReachHost::Get().GetDataDir());

    if (m_fnStart) {
        void* hostPtr = ReachHost::Get().GetInterface();
        NOBLE_INFO("Invoking c_game_engine Start(engine, host:0x{:X}, launch_block) [Map: {}]...",
            reinterpret_cast<uintptr_t>(hostPtr), mapName);
        int32_t startStatus = m_fnStart(m_engineInstance, hostPtr, launchBlock.data());
        NOBLE_INFO("c_game_engine Start returned: 0x{:X}", startStatus);
    }

    const bool isGuest = !connectAddress.empty();
    if (!isGuest) {
        // PostCommand(10) and PostCommand(11) via vtbl+0x18 after ~300 ms for host / offline matches.
        // On a guest connecting to a remote host, posting command 10/11 forces local simulation before
        // session membership and world replication arrive from the host, causing an unrendered black screen.
        // If hosting an online match / lobby with network transport active, give remote
        // clients 4000 ms to establish membership sync before posting Command 10/11.
        // For solo / offline matches, 300 ms is sufficient.
        const bool isOnlineHost = noble::NetworkTransport::Get().IsActive() && noble::NetworkTransport::Get().IsHost();
        const bool settleLonger = (gameModeTag == 1 || gameModeTag == 3); // campaign co-op / Forge
        const int peersToWaitFor = (isOnlineHost && expectedPeers > 0) ? expectedPeers : 0;
        const bool holdMissionForPeers = (gameModeTag == 1);   // campaign only; Forge must open its session early
        // Invite / direct joins carry no lobby peer count. Campaign cinematics still cannot be joined, so without a
        // count the host holds the mission start briefly for any guest that is already connecting.
        // A host that has a lobby registered knows players are coming: give them the full cold-start window (lobby launch
        // event -> process start -> engine load ~7 s -> first engine packet). A bare online host gets a short wait.
        const bool lobbyRegistered = noble::MasterServerClient::Get().Enabled() &&
                                     !noble::MasterServerClient::Get().GetCurrentSessionId().empty();
        const int campaignGraceMs = lobbyRegistered ? 30000 : 9000;
        const bool campaignGraceWait = (gameModeTag == 1) && isOnlineHost && peersToWaitFor == 0;
        const bool isCampaignMission = (gameModeTag == 1);
        std::thread([this, isOnlineHost, peersToWaitFor, settleLonger, holdMissionForPeers, campaignGraceWait, campaignGraceMs, lobbyRegistered, isCampaignMission]() {
            const int startDelay = isOnlineHost ? 1000 : 300;
            std::this_thread::sleep_for(std::chrono::milliseconds(startDelay));

            // Campaign co-op and Forge: Command 10 (Event: Start) is what loads the mission and starts its opening
            // cinematic. The host used to post it 1 s after launch, before any guest could possibly have joined, so a
            // guest arriving a few seconds later entered a session that was already mid-cinematic: its camera had no
            // cinematic binding (the player saw the world from outside the map), and the engine then aborted the
            // co-op session (0x1B/0x1C/0x1D -> "An error occurred") and dropped everyone to the menu.
            // So the peer barrier has to gate the MISSION START, not just Command 11.
            size_t joinedBeforeStart = 0;
            if (campaignGraceWait) {
                // No lobby roster was available (guest invited directly or joined by address). Campaign's opening
                // cinematic cannot be joined mid-play - a guest that arrives late sees the world from outside the map
                // and the engine aborts the session - so hold the mission start for up to 12 s while a guest connects.
                const ULONGLONG graceStart = GetTickCount64();
                const ULONGLONG graceDeadline = graceStart + static_cast<ULONGLONG>(campaignGraceMs);
                NOBLE_INFO("BlamEngine: campaign host without a lobby peer count (lobby registered: {}) - holding the "
                           "mission start for up to {} ms so a joining guest can enter before the cinematic",
                           lobbyRegistered, campaignGraceMs);
                while (GetTickCount64() < graceDeadline) {
                    joinedBeforeStart = noble::NetworkTransport::Get().GetGamePeerCount();
                    if (joinedBeforeStart > 0) break;
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                if (joinedBeforeStart > 0) {
                    NOBLE_INFO("BlamEngine: guest engine traffic appeared after {} ms ({} peer(s)); settling 3 s before "
                               "the mission starts", GetTickCount64() - graceStart, joinedBeforeStart);
                    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
                } else {
                    NOBLE_INFO("BlamEngine: no guest appeared within {} ms; starting the mission", campaignGraceMs);
                }
            } else if (peersToWaitFor > 0 && holdMissionForPeers) {
                const ULONGLONG start = GetTickCount64();
                const ULONGLONG deadline = start + 25000;       // hard cap
                const ULONGLONG giveUpAt = start + 10000;       // no traffic at all -> the session is not joinable yet
                ULONGLONG lastLog = 0;
                NOBLE_INFO("BlamEngine: campaign co-op - holding the mission start until {}/{} engine peer(s) "
                           "have joined...", joinedBeforeStart, peersToWaitFor);
                while (GetTickCount64() < deadline) {
                    joinedBeforeStart = noble::NetworkTransport::Get().GetGamePeerCount();
                    if (joinedBeforeStart >= static_cast<size_t>(peersToWaitFor)) break;
                    if (joinedBeforeStart == 0 && GetTickCount64() >= giveUpAt) {
                        NOBLE_WARN("BlamEngine: no guest engine traffic after 10 s; the session is probably only "
                                   "joinable once the mission state advances - starting the mission now");
                        break;
                    }
                    const ULONGLONG now = GetTickCount64();
                    if (lastLog == 0 || now - lastLog >= 5000) {
                        lastLog = now;
                        NOBLE_INFO("BlamEngine: peer wait before start: {}/{} engine peer(s) connected",
                                   joinedBeforeStart, peersToWaitFor);
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                joinedBeforeStart = noble::NetworkTransport::Get().GetGamePeerCount();
                if (joinedBeforeStart >= static_cast<size_t>(peersToWaitFor)) {
                    NOBLE_INFO("BlamEngine: {} engine peer(s) joined before start; beginning the mission with them",
                               joinedBeforeStart);
                } else {
                    NOBLE_WARN("BlamEngine: only {}/{} engine peer(s) joined before start (gave up after {} ms); "
                               "beginning the mission anyway", joinedBeforeStart, peersToWaitFor,
                               GetTickCount64() - start);
                }
            }

            if (m_engineInstance && m_fnProcessEvent) {
                if (holdMissionForPeers && peersToWaitFor > 0 && isOnlineHost) {
                    // Campaign builds its player list when the mission loads (Command 10); the guest's first membership
                    // message arrives ~150 ms later, so hold the mission load until the lobby's guests are in membership.
                    const size_t c10Want = static_cast<size_t>(peersToWaitFor);
                    const ULONGLONG c10Deadline = GetTickCount64() + 15000;
                    while (GetTickCount64() < c10Deadline && noble::LobbyPatches::GetRemotePlayerAddCount() < c10Want) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                    NOBLE_INFO("BlamEngine: membership before mission load: {}/{} player(s) in session",
                               noble::LobbyPatches::GetRemotePlayerAddCount(), c10Want);
                }
                NOBLE_INFO("BlamEngine: Posting Command 10 (Event: Start) via vtbl+0x18 after {}ms delay...", startDelay);
                int32_t status10 = m_fnProcessEvent(m_engineInstance, 10, 0);
                NOBLE_INFO("BlamEngine: PostCommand(10) returned: 0x{:X}", status10);

                // Campaign co-op and Forge do not tolerate the host entering simulation before
                // the guest has completed the engine join handshake. Slayer/Firefight can recover
                // from a late join; these modes instead clean the peer out of the membership and
                // immediately end the mission. When the lobby told us how many guests it launched
                // with, hold Command 11 until their port-1000 engine traffic arrives.
                const size_t targetPeers = (peersToWaitFor > 0) ? static_cast<size_t>(peersToWaitFor)
                                         : (joinedBeforeStart > 0 ? 1 : 0);
                if (targetPeers > 0) {
                    const ULONGLONG deadline = GetTickCount64() + 60000;
                    ULONGLONG lastLog = 0;
                    NOBLE_INFO("BlamEngine: waiting for {} remote player(s) to finish loading and join membership before starting simulation...",
                               targetPeers);
                    while (GetTickCount64() < deadline) {
                        const size_t playersJoined = LobbyPatches::GetRemotePlayerAddCount();
                        if (playersJoined >= targetPeers) {
                            const int settleMs = settleLonger ? 4000 : 2000;
                            NOBLE_INFO("BlamEngine: {}/{} remote player(s) joined session membership (msg 27); settling {} ms for world replication",
                                       playersJoined, targetPeers, settleMs);
                            std::this_thread::sleep_for(std::chrono::milliseconds(settleMs));
                            break;
                        }
                        const ULONGLONG now = GetTickCount64();
                        if (lastLog == 0 || now - lastLog >= 5000) {
                            lastLog = now;
                            NOBLE_INFO("BlamEngine: membership wait: {}/{} player(s) in session (engine peers: {})",
                                       playersJoined, targetPeers, NetworkTransport::Get().GetGamePeerCount());
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                    if (LobbyPatches::GetRemotePlayerAddCount() < targetPeers) {
                        NOBLE_WARN("BlamEngine: membership wait timed out with {}/{} player(s); starting simulation anyway",
                                   LobbyPatches::GetRemotePlayerAddCount(), targetPeers);
                    }
                } else {
                    const int simDelay = isOnlineHost ? 5000 : 200;
                    std::this_thread::sleep_for(std::chrono::milliseconds(simDelay));
                }

                NOBLE_INFO("BlamEngine: Posting Command 11 via vtbl+0x18...");
                int32_t status11 = m_fnProcessEvent(m_engineInstance, 11, 0);
                NOBLE_INFO("BlamEngine: PostCommand(11) returned: 0x{:X}", status11);
            }
        }).detach();
    } else {
        if (BlamEngine::GuestPostStart() && gameModeTag == 1) {
            // Research: guest posts its own mission start after the session handshake, which is what creates a local
            // simulation player. Without it the guest sits in membership with no biped (Career reports +0x30 = 255).
            NOBLE_INFO("BlamEngine: [GUEST] -guest-post-start enabled - posting Command 10/11 after the session handshake");
            std::thread([this]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(7000));
                if (!m_engineInstance || !m_fnProcessEvent) return;
                NOBLE_INFO("BlamEngine: [GUEST] Posting Command 10 (guest mission start)");
                int32_t status10 = m_fnProcessEvent(m_engineInstance, 10, 0);
                NOBLE_INFO("BlamEngine: [GUEST] PostCommand(10) returned 0x{:X}", status10);
                std::this_thread::sleep_for(std::chrono::milliseconds(6000));
                NOBLE_INFO("BlamEngine: [GUEST] Posting Command 11 (guest simulation start)");
                int32_t status11 = m_fnProcessEvent(m_engineInstance, 11, 0);
                NOBLE_INFO("BlamEngine: [GUEST] PostCommand(11) returned 0x{:X}", status11);
            }).detach();
        } else {
            NOBLE_INFO("BlamEngine: [GUEST] Remote join active - skipping manual PostCommand(10/11); waiting for host session sync.");
        }
    }

    return true;
}

void BlamEngine::Update(float deltaTime) {
    (void)deltaTime;
}

void BlamEngine::Render() {
}


bool BlamEngine::DumpGameVariant(const std::string& path) {
    std::ifstream in(std::filesystem::path(path), std::ios::binary);
    if (!in || !m_hModule) { NOBLE_WARN("DumpGameVariant: cannot open {} or engine not loaded", path); return false; }
    std::vector<uint8_t> file((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (file.empty()) return false;
    std::vector<uint8_t> decoded(0xFC04);
    char status[64]{};
    const bool ok = SafeCallGameVariantDecode(m_hModule, file.data(), decoded.data(), status);
    const std::string outPath = (std::filesystem::current_path() / (std::filesystem::path(path).filename().string() + ".decoded.bin")).string();
    std::ofstream out(std::filesystem::path(outPath), std::ios::binary);
    if (out) out.write(reinterpret_cast<const char*>(decoded.data()), static_cast<std::streamsize>(decoded.size()));
    NOBLE_INFO("DumpGameVariant: {} -> {} (ok={}, status='{}', {} bytes in)", path, outPath, ok, status, file.size());
    if (ok) {
        uint32_t first = 0;
        std::memcpy(&first, decoded.data(), sizeof(first));
        NOBLE_INFO("DumpGameVariant: first u32 = 0x{:08X}", first);
        if (decoded.size() > 0x300) {
            const auto byte = [&](size_t offset) { return static_cast<unsigned>(decoded[offset]); };
            NOBLE_INFO("Variant fields: roundsToWin {} time {} roundLimit {} sudden {} grace {}",
                       byte(0x2BD), byte(0x2BE), byte(0x2BF), byte(0x2C0), byte(0x2C2));
            NOBLE_INFO("Variant respawn: flags {} lives {} teamLives {} time {} suicide {} betrayal {} growth {} cam {} traits {}",
                       byte(0x2C4), byte(0x2C5), byte(0x2C6), byte(0x2C7), byte(0x2C8), byte(0x2C9),
                       byte(0x2CA), byte(0x2CB), byte(0x2CC));
            NOBLE_INFO("Variant traits: health {} shield {} speed {} gravity {} radar {} weapon {} ability {} mapFlags {} social {}",
                       byte(0x2D0), byte(0x2D3), byte(0x2E8), byte(0x2E9), byte(0x2F8), byte(0x2E5),
                       byte(0x2E7), byte(0x300), byte(0x2FC));
            if (decoded.size() > 0xF842) {
                uint16_t score = 0;
                std::memcpy(&score, decoded.data() + 0xF840, sizeof(score));
                NOBLE_INFO("Variant score to win: {}", score);
            }
        }
    }
    return ok;
}

void BlamEngine::Shutdown() {
    if (s_active == this) s_active = nullptr;
    if (!m_isInitialized) return;

    NOBLE_INFO("Shutting down Blam! engine network transport...");
    NetworkTransport::Get().Shutdown();
    // Do not call m_fnShutdown(m_engineInstance) (vtable[0], c_game_engine's scalar deleting destructor)
    // while haloreach.dll's engine thread is still running (such as when the menu process exits to launch
    // a match): freeing c_game_engine's critical sections and heap while the engine thread is active
    // caused the ntdll.dll access violation on exit. Process termination reclaims all engine memory cleanly.
    m_engineInstance = nullptr;
    m_isInitialized = false;
}

} // namespace noble
