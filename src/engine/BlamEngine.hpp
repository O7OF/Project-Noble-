#pragma once

#include <atomic>

#include "common/Types.hpp"
#include "ReachHost.hpp"
#include "VariantFields.hpp"
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <memory>
#include <optional>
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

    // Launch a retail map or mission from the Steam MCC installation with optional custom variants or connect to remote host
    bool LaunchMap(const std::string& mapName, const std::string& gameMode = "multiplayer", int difficulty = 1, uint64_t skulls = 0,
                   const std::string& customMvarPath = "", const std::string& customVariantPath = "",
                   const std::string& connectAddress = "", int expectedPeers = 0);

    // Dispatches a developer/console command to haloreach.dll
    bool ExecuteCommand(const std::string& command);

    // Runs the engine's own map-variant save (RVA 0x156974). It builds the whole BLF from the live session variant -
    // the same bytes the game's Forge save writes - and hands them to the engine's save system. Must be called on the
    // engine thread (PauseMenu::Render is). Returns false if the call faulted; the save itself may still be asynchronous.
    bool TriggerMapVariantSave();

    // Hooks the engine's map-variant serializer (RVA 0x17AC38) and keeps the 0x7028-byte chunk it produces. The
    // engine's save routine builds that chunk before handing it to the save device, so this is where the edited
    // variant exists even when the device write fails.
    bool InstallMapVariantCapture();

    // Serializes the live session map variant by calling the engine's serializer directly. The engine keeps it at
    // [TLS block + 0x48] + 0x110F0 (the pointer its own save routine copies from). Returns a 0x7028-byte mvar chunk;
    // this does not depend on the save UI or the save device, so it also works while the game is paused.
    bool TrySerializeLiveMapVariant(std::vector<uint8_t>& outChunk);
    bool TakeCapturedMapVariantChunk(std::vector<uint8_t>& out);

    // Dev: dump the engine state around the live map variant (raw state, the struct, and its serialized chunk) so
    // two snapshots can be diffed around a Forge placement.
    bool DumpLiveMapVariant(const std::filesystem::path& dir, int index);

    const std::string& GetCurrentMap() const { return m_currentMap; }
    const std::string& GetCurrentMode() const { return m_currentMode; }
    bool IsForgeMode() const { return _stricmp(m_currentMode.c_str(), "forge") == 0 || _stricmp(m_currentMode.c_str(), "sandbox") == 0; }

    bool IsInitialized() const { return m_isInitialized; }
    bool IsGameRunning() const { return m_isGameRunning; }
    void SetGameRunning(bool running) { m_isGameRunning = running; }

    void* GetEngineInstance() const { return m_engineInstance; }

    // Research hook: forces launch-block +0x0C (the engine class byte) instead of deriving it from the mode.
    // 1 campaign, 3 multiplayer/Megalo, 4 sandbox (Forge editor engine), 5 Firefight. "-engine-mode <n>" on the command
    // line sets it; 0 keeps the normal derivation. Used to test whether the sandbox engine can be launched as a mode.
    // The live client is created in main.cpp; that is the only place with the engine instance, so it publishes itself
    // here and any subsystem (UI, tests) can dispatch an engine command through ExecuteCommand().
    static BlamEngine* Active() { return s_active; }

    // True when the variant that was decoded for this match has the Teams option on. The pause menu
    // uses it to decide whether CHANGE TEAM applies. Set from BuildLaunchBlock before the engine starts.
    static bool GameTeamsEnabled() { return s_gameTeamsEnabled.load(std::memory_order_relaxed); }

    // Dev tool: decode a game-variant .bin with the engine's decoder and write the decoded struct to
    // <path>.decoded.bin for field mapping.
    bool DumpGameVariant(const std::string& path);

    // Host-side option overrides applied to the decoded c_game_variant before it is copied into the launch
    // block. Only the mapped General/Player Traits/score fields are changed; loadouts stay file-side.
    static void SetVariantOverrides(const VariantFields& fields) { s_variantOverrides = fields; }
    static void ClearVariantOverrides() { s_variantOverrides.reset(); }
    // Test/debug path: a comma-separated patch (time=15,score=50) applied after the selected variant is
    // decoded. Only names present in the patch change; other fields keep the variant's values.
    static void SetVariantPatch(std::string spec) { s_variantPatch = std::move(spec); }

    static void SetEngineModeOverride(int mode) { s_engineModeOverride = mode; }

    // Research switch: a guest normally skips PostCommand(10/11) and waits for the host's session sync. Campaign may
    // still need the guest to post them itself to create its own player; "-guest-post-start" enables that for a test.
    static void SetGuestPostStart(bool value) { s_guestPostStart = value; }
    static bool GuestPostStart() { return s_guestPostStart; }
    static int EngineModeOverride() { return s_engineModeOverride; }

private:
    static int s_engineModeOverride;   // research hook: forces launch-block +0x0C
    static bool s_guestPostStart;      // research hook: guest posts its own Command 10/11
    static BlamEngine* s_active;       // the running client's engine, for command dispatch
    static std::atomic<bool> s_gameTeamsEnabled;   // Teams option decoded for the running match
    static std::optional<VariantFields> s_variantOverrides;   // host options applied at launch
    static std::string s_variantPatch;                        // test path: time=15,score=50
    // Builds the 0x2BF30-byte match descriptor launch block reversed from host-probe
    std::vector<uint8_t> BuildLaunchBlock(uint32_t mapId, uint32_t gameModeTag, int difficulty, uint64_t skulls,
                                         const std::string& customMvarPath = "", const std::string& customVariantPath = "",
                                         const std::string& connectAddress = "");
    uint32_t ResolveMapId(const std::string& mapName);

    void* m_engineInstance = nullptr;
    HMODULE m_hModule = nullptr;
    ReachExports m_exports;
    EngineHostContext m_context;
    bool m_isInitialized = false;
    bool m_isGameRunning = false;
    std::string m_currentMap;
    std::string m_currentMode;

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
