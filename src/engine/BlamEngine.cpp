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
            m_fnShutdown   = reinterpret_cast<EngineShutdownFn>(vtable[0]);
            m_fnInitialize = reinterpret_cast<EngineInitFn>(vtable[1]);
            m_fnUpdate     = reinterpret_cast<EngineUpdateFn>(vtable[2]);
            m_fnRender     = reinterpret_cast<EngineRenderFn>(vtable[3]);

            NOBLE_INFO("c_game_engine VTable bound successfully:");
            NOBLE_INFO(" - vfunc[0] (Shutdown):   0x{:X}", reinterpret_cast<uintptr_t>(m_fnShutdown));
            NOBLE_INFO(" - vfunc[1] (Initialize): 0x{:X}", reinterpret_cast<uintptr_t>(m_fnInitialize));
            NOBLE_INFO(" - vfunc[2] (Update):     0x{:X}", reinterpret_cast<uintptr_t>(m_fnUpdate));
            NOBLE_INFO(" - vfunc[3] (Render):     0x{:X}", reinterpret_cast<uintptr_t>(m_fnRender));
        }
    }

    m_isInitialized = true;
    NOBLE_INFO("BlamEngine interface emulation initialized and ready.");
    return true;
}

void BlamEngine::Update(float deltaTime) {
    if (m_isInitialized && m_isGameRunning && m_fnUpdate && m_engineInstance) {
        // Delta time tick call when game match is actively running
        m_fnUpdate(m_engineInstance, deltaTime);
    }
}

void BlamEngine::Render() {
    if (m_isInitialized && m_isGameRunning && m_fnRender && m_engineInstance) {
        m_fnRender(m_engineInstance);
    }
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
