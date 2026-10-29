#include "BlamEngine.hpp"
#include "ReachHost.hpp"
#include "network/NetworkTransport.hpp"
#include "ui/LoadScreen.hpp"
#include "career/Career.hpp"
#include "hooks/HookManager.hpp"
#include "common/Logger.hpp"
#include <algorithm>
#include <thread>
#include <chrono>
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
    if (isGuest) {
        loadedPrebuilt = false;
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
    const bool isCampaign = (mapId >= 178 && mapId <= 189);
    // Firefight maps: launch index 210 .. 219.
    const bool isFirefight = (mapId >= 210 && mapId <= 219);
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
        // +0x0C: 1 = menu / campaign, 3 = multiplayer, 5 = firefight (from the layout of RR's captured blocks).
        block[0x0C] = (isMainMenu || isCampaign) ? 0x01 : (isFirefight ? 0x05 : 0x03);
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
            // Guest mode: 1 local active player, target host machine at 0x2F8
            block[0] &= ~0x08; // bit 3 cleared for guest
            *reinterpret_cast<uint64_t*>(&block[0x60]) = 0; // Host machine id is 0 at 0x60 on guest
            block[0xE8] = 0x01; // 1 active local player
            block[0xF0] = 0xBE;
            block[0xF1] = 0xF3;
            block[0xF2] = 'Y';
            block[0xF3] = '?';
            *reinterpret_cast<uint64_t*>(&block[0xF8]) = localMachineId; // Local player 0 machine ID
            block[0x100] = 0x00;
            block[0x101] = 0x00;
            block[0x102] = 0x00;
            block[0x103] = 0x00;
            block[0x104] = 0x02;
            block[0x108] = 0x00;
            block[0x109] = 0x00;
            block[0x10A] = 0x00;
            block[0x10B] = 0x00;
            block[0x2F0] = 0x00;
            *reinterpret_cast<uint32_t*>(&block[0x2F4]) = 0xFFFFFFFF;
            *reinterpret_cast<uint64_t*>(&block[0x2F8]) = hostMachineId; // uint64 Host Machine ID
            block[0x2BF14] = 0xF3;
            block[0x2BF15] = 0x01;
            NOBLE_INFO("BlamEngine: Configured Guest Launch Block targeting host {}:{} (Local: 0x{:X}, Host: 0x{:X})",
                targetHostIp, targetHostPort, localMachineId, hostMachineId);
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
        *reinterpret_cast<uint64_t*>(&block[0x50]) = NetworkTransport::Get().GetSessionId();
        *reinterpret_cast<uint64_t*>(&block[0x58]) = NetworkTransport::Get().GetLocalMachineId();
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
    if (!isGuest && !customMvarPath.empty() && std::filesystem::exists(customMvarPath) && m_hModule) {
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
    if (!isCampaign || !customVariantPath.empty()) {
        std::filesystem::path variantPath;
        if (!customVariantPath.empty() && std::filesystem::exists(customVariantPath)) {
            variantPath = customVariantPath;
        } else if (!isCampaign && !isMainMenu) {
            // Firefight needs a Firefight game variant (otherwise the engine reports "no game
            // selected"); RR likewise loads <MCC>\haloreach\game_variants\<mode>.bin for it.
            variantPath = ReachHost::Get().GetReachDir() / "game_variants" /
                          (isFirefight ? "hr_ff_standard_1set.bin" : "slayer_054.bin");
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
                    memcpy(&block[0x300], decoded.data(), decoded.size());
                    NOBLE_INFO("BlamEngine: Decoded game variant '{}' into launch block (+0x300)", variantPath.string());
                } else {
                    NOBLE_ERROR("BlamEngine: haloreach.dll could not decode game variant '{}'", variantPath.string());
                }
            }
        } else {
            NOBLE_WARN("BlamEngine: No game variant found for launch block (looked for '{}')", variantPath.string());
        }
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
                           const std::string& connectAddress) {
    if (!m_isInitialized || !m_engineInstance) return false;

    m_currentMap = mapName;
    m_isGameRunning = true;

    NOBLE_INFO("=================================================");
    NOBLE_INFO(" LAUNCHING HALO: REACH MISSION / MAP: {}", mapName);
    NOBLE_INFO(" Mode: {} | Difficulty: {} | Skulls: 0x{:X}", gameMode, difficulty, skulls);
    if (!customMvarPath.empty()) NOBLE_INFO(" Custom MVAR: {}", customMvarPath);
    if (!customVariantPath.empty()) NOBLE_INFO(" Custom Variant: {}", customVariantPath);
    NOBLE_INFO("=================================================");

    uint32_t mapId = ResolveMapId(mapName);
    uint32_t gameModeTag = 2; // Default multiplayer
    if ((mapId >= 210 && mapId <= 219) || gameMode == "firefight" || gameMode == "Firefight") {
        gameModeTag = 1035; // 0x40B Firefight
    } else if ((mapId >= 178 && mapId <= 189) || gameMode == "Campaign" || gameMode == "campaign") {
        gameModeTag = 1;    // Campaign
    } else if (mapId == static_cast<uint32_t>(-2) || mapId == 0 || gameMode == "mainmenu" || gameMode == "menu") {
        gameModeTag = 0;    // Main Menu
    }

    // Career pays by base type, so modded maps/variants earn exactly what their stock type earns.
    if (gameModeTag != 0) Career::BeginMatch(Career::BaseTypeFromEngineMode(gameModeTag), mapName, difficulty, skulls);

    auto launchBlock = BuildLaunchBlock(mapId, gameModeTag, difficulty, skulls, customMvarPath, customVariantPath, connectAddress);

    // Request corresponding wireframe loading screen .dots
    LoadScreen::Get().RequestMapDots(mapName, ReachHost::Get().GetDataDir());

    if (m_fnStart) {
        void* hostPtr = ReachHost::Get().GetInterface();
        NOBLE_INFO("Invoking c_game_engine Start(engine, host:0x{:X}, launch_block) [Map: {}]...",
            reinterpret_cast<uintptr_t>(hostPtr), mapName);
        int32_t startStatus = m_fnStart(m_engineInstance, hostPtr, launchBlock.data());
        NOBLE_INFO("c_game_engine Start returned: 0x{:X}", startStatus);
    }

    // PostCommand(10) and PostCommand(11) via vtbl+0x18 after ~300 ms (verified from host-probe main.c lines 4306-4323)
    std::thread([this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        if (m_engineInstance && m_fnProcessEvent) {
            NOBLE_INFO("BlamEngine: Posting Command 10 (Event: Start) via vtbl+0x18 after 300ms delay...");
            int32_t status10 = m_fnProcessEvent(m_engineInstance, 10, 0);
            NOBLE_INFO("BlamEngine: PostCommand(10) returned: 0x{:X}", status10);

            NOBLE_INFO("BlamEngine: Posting Command 11 via vtbl+0x18...");
            int32_t status11 = m_fnProcessEvent(m_engineInstance, 11, 0);
            NOBLE_INFO("BlamEngine: PostCommand(11) returned: 0x{:X}", status11);
        }
    }).detach();

    return true;
}

void BlamEngine::Update(float deltaTime) {
    (void)deltaTime;
}

void BlamEngine::Render() {
}

void BlamEngine::Shutdown() {
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
