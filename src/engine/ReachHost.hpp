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

private:
    struct ID3D11Device* m_device = nullptr;
    struct ID3D11DeviceContext* m_context = nullptr;
    void* m_hostObject[64] = { nullptr }; // 0x200 bytes host object
    void* m_secondObject[64] = { nullptr }; // 0x200 bytes second object
    static void* s_vtable[160];
    static void* s_vtable2[160]; // RR: 160 host::VTABLE2 stubs
    static bool s_vtableInitialized;
    static void InitializeVTable();

    std::filesystem::path m_reachDir;
    std::filesystem::path m_mapsDir;
    std::filesystem::path m_dataDir;
};

} // namespace noble
