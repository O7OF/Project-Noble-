#include "BlamEngine.hpp"
#include "common/Logger.hpp"

namespace noble {

BlamEngine::BlamEngine() = default;

BlamEngine::~BlamEngine() {
    Shutdown();
}

bool BlamEngine::Initialize(const ReachExports& exports, const EngineHostContext& context) {
    if (m_isInitialized) return true;

    m_exports = exports;
    m_context = context;

    NOBLE_INFO("Instantiating Blam! Reach Game Engine & Data Access interfaces...");

    // 1. Create Data Access Interface
    if (m_exports.CreateDataAccess) {
        typedef int32_t (*CreateDataAccessCall)(void** outDataAccess);
        auto pCreateDataAccess = reinterpret_cast<CreateDataAccessCall>(m_exports.CreateDataAccess);
        
        int32_t status = pCreateDataAccess(&m_dataAccessInstance);
        NOBLE_INFO("CreateDataAccess returned status: 0x{:X}, Instance: 0x{:X}", 
            status, reinterpret_cast<uintptr_t>(m_dataAccessInstance));
    }

    // 2. Create Game Engine Interface
    if (m_exports.CreateGameEngine) {
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
            m_fnUpdate         = reinterpret_cast<EngineUpdateFn>(vtable[2]);
            m_fnRender         = reinterpret_cast<EngineRenderFn>(vtable[3]);
            m_fnExecuteCommand = reinterpret_cast<EngineExecuteCommandFn>(vtable[9]);

            NOBLE_INFO("c_game_engine VTable bound successfully:");
            NOBLE_INFO(" - vfunc[0] (Shutdown):       0x{:X}", reinterpret_cast<uintptr_t>(m_fnShutdown));
            NOBLE_INFO(" - vfunc[1] (Initialize):     0x{:X}", reinterpret_cast<uintptr_t>(m_fnInitialize));
            NOBLE_INFO(" - vfunc[2] (Update):         0x{:X}", reinterpret_cast<uintptr_t>(m_fnUpdate));
            NOBLE_INFO(" - vfunc[3] (Render):         0x{:X}", reinterpret_cast<uintptr_t>(m_fnRender));
            NOBLE_INFO(" - vfunc[9] (ExecuteCommand): 0x{:X}", reinterpret_cast<uintptr_t>(m_fnExecuteCommand));
        }
    }

    m_isInitialized = true;
    NOBLE_INFO("BlamEngine interface emulation initialized and ready.");
    return true;
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

bool BlamEngine::LaunchMap(const std::string& mapName, const std::string& gameMode, int difficulty, uint32_t skulls) {
    if (!m_isInitialized) return false;

    m_currentMap = mapName;
    m_isGameRunning = true;

    NOBLE_INFO("=================================================");
    NOBLE_INFO(" LAUNCHING HALO: REACH MISSION / MAP: {}", mapName);
    NOBLE_INFO(" Mode: {} | Difficulty: {} | Skulls: 0x{:X}", gameMode, difficulty, skulls);
    NOBLE_INFO("=================================================");

    // Format Blam! engine level path
    // e.g. "map_name levels\solo\m10\m10" or "map_name forge_halo"
    std::string mapCmd = "map_name " + mapName;
    ExecuteCommand(mapCmd);

    // Set game mode / difficulty if campaign
    if (gameMode == "Campaign") {
        ExecuteCommand("game_difficulty " + std::to_string(difficulty));
        ExecuteCommand("game_skulls " + std::to_string(skulls));
    }

    // Trigger map start countdown
    ExecuteCommand("game_start");

    return true;
}

void BlamEngine::Update(float deltaTime) {
    (void)deltaTime;
    // Internal simulation in haloreach.dll is driven asynchronously on Blam's internal engine threads.
    // Note: c_game_engine vfunc[2] at RVA 0xF118 is SetGameContext/SetupMatch and expects a 0x2BF30-byte
    // match context descriptor pointer in r8. Invoking it as a delta-time tick caused an invalid
    // memory read access violation in memcpy. Commands dispatched via ExecuteCommand govern gameplay lifecycle.
}

void BlamEngine::Render() {
    // Direct3D 11 presentation is handled via D3D11Renderer and Dear ImGui.
    // Note: c_game_engine vfunc[3] is an event dispatcher (ProcessEvent), not a DX11 render pass.
}

void BlamEngine::Shutdown() {
    if (!m_isInitialized) return;

    NOBLE_INFO("Shutting down Blam! engine...");
    if (m_fnShutdown && m_engineInstance) {
        m_fnShutdown(m_engineInstance);
    }

    m_engineInstance = nullptr;
    m_dataAccessInstance = nullptr;
    m_isInitialized = false;
}

} // namespace noble
