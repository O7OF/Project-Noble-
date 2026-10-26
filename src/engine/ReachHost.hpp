#pragma once

#include <windows.h>
#include <cstdint>
#include <filesystem>
#include <string>

namespace noble {

class ReachHost {
public:
    ReachHost();
    ~ReachHost() = default;

    static ReachHost& Get();

    void* GetInterface() { return &m_vptr; }

    void SetPaths(const std::filesystem::path& reachDir,
                  const std::filesystem::path& mapsDir,
                  const std::filesystem::path& dataDir);

    const std::filesystem::path& GetReachDir() const { return m_reachDir; }
    const std::filesystem::path& GetMapsDir() const { return m_mapsDir; }
    const std::filesystem::path& GetDataDir() const { return m_dataDir; }

private:
    void* m_vptr = nullptr;
    static void* s_vtable[160];
    static bool s_vtableInitialized;
    static void InitializeVTable();

    std::filesystem::path m_reachDir;
    std::filesystem::path m_mapsDir;
    std::filesystem::path m_dataDir;
};

} // namespace noble
