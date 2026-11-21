#pragma once

#include <windows.h>
#include <cstdint>
#include <filesystem>
#include <string>

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace noble {

class ReachHost {
public:
    ReachHost();
    ~ReachHost() = default;

    static ReachHost& Get();

    // True once the host simulation session has started.
    static bool IsSessionStarted();

    // Cinematic skip request
    static void RequestCinematicSkip();
    static bool IsCinematicSkipRequested();

    // True while an engine UI screen (e.g. the native Forge menu) is up.
    static bool IsUiScreenActive();
    // GetTickCount64 of the last engine frame (0 until the first), for the stall watchdog.
    static uint64_t GetLastFrameTickMs();
    // Ends the current round on the engine thread's next frame (same call as the script command
    // mp_round_end). Used by a passed END ROUND vote.
    static void RequestRoundEnd();
    // Places a scenario squad by name through the engine's ai_place worker. GAME thread only (the
    // per-player control tick); the slot-1 frame tick lacks the engine's thread-local AI state.
    static bool PlaceSquadByName(const char* squadName, int count);
    // GAME thread only. Fills a scenario squad's spawn points with a squad_template tag (by tag index), the
    // way ai_place_wave fills Firefight squads. count -1 = the template's default.
    static bool PlaceSquadTemplate(const char* squadName, uint16_t templateTagIndex, int count);
    // Thread-safe: queues a placement for the next game-thread tick. count -1 = the squad's default.
    static void RequestSquadPlacement(const char* squadName, int count);
    // Called by SpawnTest's per-player control tick hook (0x61A38, player 0) on the game thread.
    static void GameThreadTick();

    // Sends the settled award to every connected guest as a match_over packet (host only). Called
    // after any settlement, including the fallback used when the engine never produced results, so
    // every client can pay its own cR/challenges/commendations and show the after-action report.
    static void BroadcastMatchAward();

    // The engine's map state (haloreach+0xC1A118): 0 none, 1 loading, 2 running, 3 ending. -1 unknown.
    static int GetMapState();
    // The engine's launch state (haloreach+0xC1A114): 11 = a launched map is in progress. -1 unknown.
    static int GetLaunchState();


    void* GetInterface() { return m_hostObject; }
    void* GetSecondInterface() { return m_secondObject; }

    void SetPaths(const std::filesystem::path& reachDir,
                  const std::filesystem::path& mapsDir,
                  const std::filesystem::path& dataDir);

    void SetDeviceAndContext(struct ID3D11Device* device, struct ID3D11DeviceContext* context);
    struct ID3D11Device* GetDevice() const { return m_device; }
    struct ID3D11DeviceContext* GetContext() const { return m_context; }

    const std::filesystem::path& GetReachDir() const { return m_reachDir; }
    const std::filesystem::path& GetMapsDir() const { return m_mapsDir; }
    const std::filesystem::path& GetDataDir() const { return m_dataDir; }
    void SetDisplaySize(uint32_t width, uint32_t height) { m_displayWidth = width; m_displayHeight = height; }

    // True while the engine's own pad/keyboard state has this virtual key down (the same state the game reads, including
    // synthetic presses from test flags). Used by overlays that must appear exactly when the game shows a screen.
    static bool IsKeyHeld(int vk);
    // True while a controller is connected and this key is one of the paddle keys, so it isn't a keyboard key right now.
    static bool IsPaddleKey(int vk);
    uint32_t GetDisplayWidth() const { return m_displayWidth; }
    uint32_t GetDisplayHeight() const { return m_displayHeight; }

    static void SetRuntimeVolumes(float game, float music, float other);

private:
    struct ID3D11Device* m_device = nullptr;
    struct ID3D11DeviceContext* m_context = nullptr;
    void* m_hostObject[64] = { nullptr }; // 0x200 bytes host object
    void* m_secondObject[64] = { nullptr }; // 0x200 bytes second object
    static void* s_vtable[160];
    static void* s_vtable2[160]; // the engine: 160 host::VTABLE2 stubs
    static bool s_vtableInitialized;
    static void InitializeVTable();

    std::filesystem::path m_reachDir;
    std::filesystem::path m_mapsDir;
    std::filesystem::path m_dataDir;
    uint32_t m_displayWidth = 1280;
    uint32_t m_displayHeight = 720;
};

} // namespace noble
