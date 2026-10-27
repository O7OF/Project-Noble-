#include "BlamEngine.hpp"
#include "ReachHost.hpp"
#include "hooks/HookManager.hpp"
#include "common/Logger.hpp"
#include <algorithm>
#include <thread>
#include <chrono>
#include <fstream>
#include <filesystem>

namespace noble {

typedef void(__fastcall* UpdateResourcePoolFn)(uint32_t category, uint32_t poolIndex);
static UpdateResourcePoolFn s_originalUpdateResourcePool = nullptr;
static uintptr_t s_haloReachBase = 0;

static void __fastcall Hook_UpdateResourcePool(uint32_t category, uint32_t poolIndex) {
    if (poolIndex >= 16) {
        return; // Out-of-bounds pool index (e.g. 0x1000)
    }
    if (s_haloReachBase) {
        uintptr_t poolDesc = s_haloReachBase + 0xC8E4C0 + (static_cast<uintptr_t>(poolIndex) * 0x70);
        if (!IsBadReadPtr(reinterpret_cast<void*>(poolDesc + 0x50), sizeof(void*))) {
            void* atomArray = *reinterpret_cast<void**>(poolDesc + 0x50);
            if (!atomArray) {
                return;
            }
        }
    }
    if (s_originalUpdateResourcePool) {
        s_originalUpdateResourcePool(category, poolIndex);
    }
}

typedef void(__fastcall* UpdatePoolFn)(uint32_t poolIndex);
static UpdatePoolFn s_originalUpdatePool = nullptr;

static void __fastcall Hook_UpdatePool(uint32_t poolIndex) {
    if (poolIndex >= 16) {
        return; // Out-of-bounds pool index (e.g. 0x1000)
    }
    if (s_haloReachBase) {
        uintptr_t poolDesc = s_haloReachBase + 0xC8E4C0 + (static_cast<uintptr_t>(poolIndex) * 0x70);
        if (!IsBadReadPtr(reinterpret_cast<void*>(poolDesc + 0x50), sizeof(void*))) {
            void* atomArray = *reinterpret_cast<void**>(poolDesc + 0x50);
            if (!atomArray) {
                return;
            }
        }
    }
    if (s_originalUpdatePool) {
        s_originalUpdatePool(poolIndex);
    }
}

typedef void(__fastcall* UpdatePredictionCacheFn)(uint32_t category, uint32_t poolIndex);
static UpdatePredictionCacheFn s_originalUpdatePredictionCache = nullptr;

static void __fastcall Hook_UpdatePredictionCache(uint32_t category, uint32_t poolIndex) {
    if (poolIndex >= 16) {
        return; // Out-of-bounds pool index (e.g. 0x1000)
    }
    if (s_haloReachBase) {
        uintptr_t poolDesc = s_haloReachBase + 0xC8E4C0 + (static_cast<uintptr_t>(poolIndex) * 0x70);
        if (!IsBadReadPtr(reinterpret_cast<void*>(poolDesc + 0x50), sizeof(void*))) {
            void* atomArray = *reinterpret_cast<void**>(poolDesc + 0x50);
            if (!atomArray) {
                return;
            }
        }
    }
    if (s_originalUpdatePredictionCache) {
        s_originalUpdatePredictionCache(category, poolIndex);
    }
}

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

// Fallback prediction atom buffer (512 atoms * 0x88 bytes = 69,632 bytes)
static uint8_t s_fallbackAtomBuffer[512 * 0x88] = { 0 };

void BlamEngine::InitializePoolDescriptors() {
    if (!m_hModule) return;
    uintptr_t base = reinterpret_cast<uintptr_t>(m_hModule);
    uintptr_t poolTable = base + 0xC8E4C0;

    DWORD oldProtect = 0;
    VirtualProtect(reinterpret_cast<void*>(poolTable), 32 * 0x70, PAGE_READWRITE, &oldProtect);

    for (size_t i = 0; i < 32; ++i) {
        uintptr_t poolDesc = poolTable + (i * 0x70);
        void** pAtomArray = reinterpret_cast<void**>(poolDesc + 0x50);
        if (*pAtomArray == nullptr) {
            *pAtomArray = s_fallbackAtomBuffer;
        }
    }

    if (oldProtect) {
        VirtualProtect(reinterpret_cast<void*>(poolTable), 32 * 0x70, oldProtect, &oldProtect);
    }
    NOBLE_INFO("BlamEngine: Initialized fallback atom arrays for all 32 resource pools.");
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
        NOBLE_INFO("Configuring haloreach.dll with SetLibrarySettings (510-byte layout)...");
        ReachLibrarySettings settings{};
        wcsncpy_s(settings.gameRoot, installInfo.reachDirectory.wstring().c_str(), _TRUNCATE);
        wcsncpy_s(settings.mapsDirectory, installInfo.mapsDirectory.wstring().c_str(), _TRUNCATE);
        wcsncpy_s(settings.userDirectory, L"data", _TRUNCATE);

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

    // Configure Host object paths
    ReachHost::Get().SetPaths(installInfo.reachDirectory, installInfo.mapsDirectory, "data");

    // Install safeguard detours for prediction atom cache (RVA 0x274488 and 0x274524)
    s_haloReachBase = reinterpret_cast<uintptr_t>(m_hModule);
    void* updateResourcePoolTarget = reinterpret_cast<void*>(s_haloReachBase + 0x274488);
    HookManager::InstallHook(updateResourcePoolTarget,
        reinterpret_cast<void*>(&Hook_UpdateResourcePool),
        reinterpret_cast<void**>(&s_originalUpdateResourcePool),
        "UpdateResourcePool_SafeGuard");

    void* updatePredictionCacheTarget = reinterpret_cast<void*>(s_haloReachBase + 0x274524);
    HookManager::InstallHook(updatePredictionCacheTarget,
        reinterpret_cast<void*>(&Hook_UpdatePredictionCache),
        reinterpret_cast<void**>(&s_originalUpdatePredictionCache),
        "UpdatePredictionCache_SafeGuard");

    void* updatePoolTarget = reinterpret_cast<void*>(s_haloReachBase + 0x2743A4);
    HookManager::InstallHook(updatePoolTarget,
        reinterpret_cast<void*>(&Hook_UpdatePool),
        reinterpret_cast<void**>(&s_originalUpdatePool),
        "UpdatePool_SafeGuard");

    // Initialize pool descriptors with fallback atom arrays to prevent NULL dereferences
    InitializePoolDescriptors();

    // 4. Start(engine, host, launch_block) targeting mainmenu.map
    m_currentMap = "mainmenu";
    m_isGameRunning = true;
    auto launchBlock = BuildLaunchBlock(0, 0, 0, 0); // mapId 0 = mainmenu

    if (m_fnStart) {
        void* hostPtr = ReachHost::Get().GetInterface();
        NOBLE_INFO("Calling c_game_engine Start(engine, host:0x{:X}, launch_block) [Target: mainmenu.map]...",
            reinterpret_cast<uintptr_t>(hostPtr));
        int32_t startStatus = m_fnStart(m_engineInstance, hostPtr, launchBlock.data());
        NOBLE_INFO("c_game_engine Start returned: 0x{:X}", startStatus);
    }

    // 5. PostCommand(10) via vtbl+0x18 after ~300 ms
    std::thread([this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        if (m_engineInstance && m_fnProcessEvent) {
            NOBLE_INFO("BlamEngine: Posting Command 10 (Event: Start) via vtbl+0x18 after 300ms delay...");
            int32_t postStatus = m_fnProcessEvent(m_engineInstance, 10, 0);
            NOBLE_INFO("BlamEngine: PostCommand(10) returned: 0x{:X}", postStatus);
        }
    }).detach();

    m_isInitialized = true;
    NOBLE_INFO("BlamEngine host interface emulation initialized and mainmenu launch sequence dispatched.");
    return true;
}

uint32_t BlamEngine::ResolveMapId(const std::string& mapName) {
    std::string lower = mapName;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    if (lower.find("mainmenu") != std::string::npos) return 0;
    if (lower.find("m10") != std::string::npos) return 1000; // Noble Actual
    if (lower.find("m20") != std::string::npos) return 20;   // Winter Contingency
    if (lower.find("m30") != std::string::npos) return 30;   // ONI: Sword Base
    if (lower.find("m35") != std::string::npos) return 35;   // Nightfall
    if (lower.find("m45") != std::string::npos) return 45;   // Tip of the Spear
    if (lower.find("m50") != std::string::npos) return 50;   // Long Night of Solace
    if (lower.find("m52") != std::string::npos) return 52;   // Exodus
    if (lower.find("m60") != std::string::npos) return 60;   // New Alexandria
    if (lower.find("m70") != std::string::npos) return 70;   // The Package
    if (lower.find("forge_halo") != std::string::npos) return 7000;  // Forge World
    if (lower.find("50_panopticon") != std::string::npos) return 5000; // Boardwalk
    if (lower.find("ff10_prototype") != std::string::npos) return 1010; // Corvette Firefight

    return 0;
}

std::vector<uint8_t> BlamEngine::BuildLaunchBlock(uint32_t mapId, uint32_t gameModeTag, int difficulty, uint32_t skulls,
                                                   const std::string& customMvarPath, const std::string& customVariantPath) {
    constexpr size_t LAUNCH_BLOCK_SIZE = 0x2BF30; // 179,984 bytes
    std::vector<uint8_t> block(LAUNCH_BLOCK_SIZE, 0);

    // Initial header markers (reversed from host-probe::new_launch_block)
    block[0] = 0x08;
    block[1] = 0x03;
    block[2] = 0x00;
    block[3] = 0x10;
    block[4] = 0x03;
    block[0x0C] = 0x03;
    block[0x10] = 0xBE;
    block[0x14] = 0xBE;
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

    // Set local player 0 active (from host-probe::new_launch_block lines 251-274)
    block[0xE8] = 0x01; // 1 active player
    block[0xF0] = 0xBE;
    block[0xF1] = 0xF3;
    block[0xF2] = 'Y';
    block[0xF3] = '?';
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

    // Map ID & Mode tagging (from host-probe::set_map lines 78-92)
    *reinterpret_cast<uint32_t*>(&block[0x10]) = mapId;
    *reinterpret_cast<uint32_t*>(&block[0x14]) = mapId;
    *reinterpret_cast<uint32_t*>(&block[0x18]) = 0x88880000;
    *reinterpret_cast<uint64_t*>(&block[0x1C]) = 0;
    *reinterpret_cast<uint32_t*>(&block[0x1D5C0]) = mapId;
    *reinterpret_cast<uint32_t*>(&block[0x1D5C4]) = 0x88880000;
    *reinterpret_cast<uint64_t*>(&block[0x1D5C8]) = 0;
    *reinterpret_cast<uint32_t*>(&block[0x1D334]) = gameModeTag;
    *reinterpret_cast<uint32_t*>(&block[0x1D5BC]) = gameModeTag;

    // Store difficulty & skulls
    *reinterpret_cast<uint32_t*>(&block[0x38]) = static_cast<uint32_t>(difficulty);
    *reinterpret_cast<uint32_t*>(&block[0x3C]) = skulls;

    // Default variant buffer init at offset 0x1D308 (RVA 0x6BFF8)
    if (m_hModule) {
        SafeCallVariantInit(m_hModule, &block[0x1D308]);
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

    // Dynamic Custom Game Variant (.bin) Injection at offset 0x300 (size 0xFC04)
    if (!customVariantPath.empty() && std::filesystem::exists(customVariantPath)) {
        NOBLE_INFO("BlamEngine: Injecting custom game variant from '{}'...", customVariantPath);
        std::ifstream varFile(customVariantPath, std::ios::binary);
        if (varFile) {
            std::vector<uint8_t> varData((std::istreambuf_iterator<char>(varFile)),
                                          std::istreambuf_iterator<char>());
            size_t bytesToCopy = (std::min)(varData.size(), static_cast<size_t>(0xFC04));
            memcpy(&block[0x300], varData.data(), bytesToCopy);
            NOBLE_INFO("BlamEngine: Injected {} bytes of game variant into launch block (+0x300)", bytesToCopy);
        }
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

bool BlamEngine::LaunchMap(const std::string& mapName, const std::string& gameMode, int difficulty, uint32_t skulls,
                           const std::string& customMvarPath, const std::string& customVariantPath) {
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
    uint32_t gameModeTag = (gameMode == "Campaign") ? 1 : (gameMode == "mainmenu" ? 0 : 2);

    auto launchBlock = BuildLaunchBlock(mapId, gameModeTag, difficulty, skulls, customMvarPath, customVariantPath);

    if (m_fnStart) {
        void* hostPtr = ReachHost::Get().GetInterface();
        NOBLE_INFO("Invoking c_game_engine Start(engine, host:0x{:X}, launch_block) [Map: {}]...",
            reinterpret_cast<uintptr_t>(hostPtr), mapName);
        int32_t startStatus = m_fnStart(m_engineInstance, hostPtr, launchBlock.data());
        NOBLE_INFO("c_game_engine Start returned: 0x{:X}", startStatus);
    }

    // PostCommand(10) via vtbl+0x18 after ~300 ms
    std::thread([this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        if (m_engineInstance && m_fnProcessEvent) {
            NOBLE_INFO("BlamEngine: Posting Command 10 via vtbl+0x18 after 300ms delay...");
            m_fnProcessEvent(m_engineInstance, 10, 0);
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

    NOBLE_INFO("Shutting down Blam! engine...");
    if (m_fnShutdown && m_engineInstance) {
        m_fnShutdown(m_engineInstance);
    }

    m_engineInstance = nullptr;
    m_isInitialized = false;
}

} // namespace noble
