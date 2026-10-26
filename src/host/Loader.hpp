#pragma once

#include "common/Types.hpp"
#include <windows.h>

namespace noble {

class Loader {
public:
    Loader();
    ~Loader();

    // Loads haloreach.dll and resolves factory export functions
    bool LoadReachDll(const fs::path& dllPath);
    void Unload();

    HMODULE GetModuleHandle() const { return m_hModule; }
    uintptr_t GetBaseAddress() const { return m_baseAddress; }
    size_t GetModuleSize() const { return m_moduleSize; }
    const ReachExports& GetExports() const { return m_exports; }

private:
    HMODULE m_hModule = nullptr;
    uintptr_t m_baseAddress = 0;
    size_t m_moduleSize = 0;
    ReachExports m_exports;
};

} // namespace noble
