#pragma once

#include "common/Types.hpp"
#include "ReachHost.hpp"
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>

namespace noble {

// Host-to-Engine interface context passed during engine startup
struct EngineHostContext {
    HWND hWnd = nullptr;
    ID3D11Device* pDevice = nullptr;
    ID3D11DeviceContext* pContext = nullptr;
    IDXGISwapChain* pSwapChain = nullptr;
    uint32_t width = 1280;
    uint32_t height = 720;
};

class BlamEngine {
public:
    BlamEngine();
    ~BlamEngine();

    // Initializes the Blam! engine instance using resolved haloreach.dll exports
    bool Initialize(const ReachInstallInfo& installInfo, HMODULE hModule, const ReachExports& exports, const EngineHostContext& context);
    
    // Executes a single simulation and render frame
    void Update(float deltaTime);
    void Render();
    
    // Shuts down the engine cleanly
    void Shutdown();

    // Launch a retail map or mission from the Steam MCC installation
    bool LaunchMap(const std::string& mapName, const std::string& gameMode = "multiplayer", int difficulty = 1, uint32_t skulls = 0);

    // Dispatches a developer/console command to haloreach.dll
    bool ExecuteCommand(const std::string& command);

    const std::string& GetCurrentMap() const { return m_currentMap; }

    bool IsInitialized() const { return m_isInitialized; }
    bool IsGameRunning() const { return m_isGameRunning; }
    void SetGameRunning(bool running) { m_isGameRunning = running; }

    void* GetEngineInstance() const { return m_engineInstance; }

private:
    // Builds the 0x2BF30-byte match descriptor launch block reversed from host-probe
    std::vector<uint8_t> BuildLaunchBlock(uint32_t mapId, uint32_t gameModeTag, int difficulty, uint32_t skulls);
    uint32_t ResolveMapId(const std::string& mapName);
    void InitializePoolDescriptors();

    void* m_engineInstance = nullptr;
    HMODULE m_hModule = nullptr;
    ReachExports m_exports;
    EngineHostContext m_context;
    bool m_isInitialized = false;
    bool m_isGameRunning = false;
    std::string m_currentMap;

    // VTable function pointers for c_game_engine
    typedef void (*EngineShutdownFn)(void* thisPtr);
    typedef int32_t (*EngineInitFn)(void* thisPtr, void* device, void* reserved, void* swapChain, void* extra);
    typedef int32_t (*EngineStartFn)(void* thisPtr, void* hostInterface, void* launchBlock);
    typedef int32_t (*EngineProcessEventFn)(void* thisPtr, uint32_t eventId, uint32_t eventData);
    typedef int32_t (*EngineExecuteCommandFn)(void* thisPtr, const char* command);

    EngineShutdownFn       m_fnShutdown = nullptr;
    EngineInitFn           m_fnInitialize = nullptr;
    EngineStartFn          m_fnStart = nullptr;
    EngineProcessEventFn   m_fnProcessEvent = nullptr;
    EngineExecuteCommandFn m_fnExecuteCommand = nullptr;
};

} // namespace noble
