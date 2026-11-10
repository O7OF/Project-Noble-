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

    // True while an engine UI screen (e.g. the native Forge menu) is up.
    static bool IsUiScreenActive();
    // GetTickCount64 of the last engine frame (0 until the first), for the stall watchdog.
    static uint64_t GetLastFrameTickMs();

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
