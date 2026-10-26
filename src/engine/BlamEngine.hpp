#pragma once

#include "common/Types.hpp"
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <memory>

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
    bool Initialize(const ReachExports& exports, const EngineHostContext& context);
    
    // Executes a single simulation and render frame
    void Update(float deltaTime);
    void Render();
    
    // Shuts down the engine cleanly
    void Shutdown();

    bool IsInitialized() const { return m_isInitialized; }
    void* GetEngineInstance() const { return m_engineInstance; }
    void* GetDataAccessInstance() const { return m_dataAccessInstance; }

private:
    void* m_engineInstance = nullptr;
    void* m_dataAccessInstance = nullptr;
    ReachExports m_exports;
    EngineHostContext m_context;
    bool m_isInitialized = false;

    // VTable function pointers for c_game_engine
    typedef void (*EngineShutdownFn)(void* thisPtr);
    typedef int32_t (*EngineInitFn)(void* thisPtr, void* arg1, void* arg2, void* arg3);
    typedef void (*EngineUpdateFn)(void* thisPtr, float deltaTime);
    typedef void (*EngineRenderFn)(void* thisPtr);

    EngineShutdownFn m_fnShutdown = nullptr;
    EngineInitFn     m_fnInitialize = nullptr;
    EngineUpdateFn   m_fnUpdate = nullptr;
    EngineRenderFn   m_fnRender = nullptr;
};

} // namespace noble
