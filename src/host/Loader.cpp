#include "Loader.hpp"
#include "common/Logger.hpp"

#include <psapi.h>

namespace noble {

Loader::Loader() = default;

Loader::~Loader() {
    Unload();
}

bool Loader::LoadReachDll(const fs::path& dllPath) {
    NOBLE_INFO("Bootstrapping Reach engine: Loading haloreach.dll into process memory...");
    NOBLE_INFO("Source DLL: {}", dllPath.string());

    if (!fs::exists(dllPath)) {
        NOBLE_ERROR("haloreach.dll does not exist at requested path: {}", dllPath.string());
        return false;
    }

    // Set DLL directory so haloreach.dll can find any sibling dependencies (e.g. fmod)
    SetDllDirectoryW(dllPath.parent_path().c_str());

    // Load library with altered search path so dependencies in its directory resolve
    m_hModule = LoadLibraryExW(dllPath.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);

    // Keep a DLL directory set for the rest of the run, pointing at MCC's own binaries folder
    // (<MCC>\MCC\Binaries\Win64), as the reference host does. haloreach.dll loads
    // xaudio2_9redist.dll by bare name during engine start-up (for sound); MCC ships it there,
    // and without this the load fails and the game is silent.
    const fs::path mccBinaries = dllPath.parent_path().parent_path() / L"MCC" / L"Binaries" / L"Win64";
    std::error_code binEc;
    if (fs::exists(mccBinaries / L"xaudio2_9redist.dll", binEc)) {
        SetDllDirectoryW(mccBinaries.c_str());
        NOBLE_INFO("DLL directory set to MCC binaries (xaudio2_9redist.dll found): {}", mccBinaries.string());
    } else {
        SetDllDirectoryW(nullptr);
        NOBLE_WARN("xaudio2_9redist.dll not found in {}; the game may have no sound", mccBinaries.string());
    }

    if (!m_hModule) {
        DWORD err = GetLastError();
        NOBLE_ERROR("Failed to load haloreach.dll! Win32 Error Code: 0x{:08X}", err);
        return false;
    }

    m_baseAddress = reinterpret_cast<uintptr_t>(m_hModule);

    // Query module size
    MODULEINFO modInfo = { 0 };
    if (GetModuleInformation(GetCurrentProcess(), m_hModule, &modInfo, sizeof(modInfo))) {
        m_moduleSize = modInfo.SizeOfImage;
    }

    NOBLE_INFO("haloreach.dll successfully loaded at Base Address: 0x{:X} (Size: 0x{:X} bytes)", 
        m_baseAddress, m_moduleSize);

    // Resolve exported engine interfaces
    m_exports.CreateGameEngine = reinterpret_cast<CreateGameEngineFn>(
        GetProcAddress(m_hModule, "CreateGameEngine"));
    m_exports.CreateDataAccess = reinterpret_cast<CreateDataAccessFn>(
        GetProcAddress(m_hModule, "CreateDataAccess"));
    m_exports.SetLibrarySettings = reinterpret_cast<SetLibrarySettingsFn>(
        GetProcAddress(m_hModule, "SetLibrarySettings"));

    NOBLE_INFO("Enumerating haloreach.dll exported factories:");
    NOBLE_INFO(" - CreateGameEngine:   0x{:X}", reinterpret_cast<uintptr_t>(m_exports.CreateGameEngine));
    NOBLE_INFO(" - CreateDataAccess:   0x{:X}", reinterpret_cast<uintptr_t>(m_exports.CreateDataAccess));
    NOBLE_INFO(" - SetLibrarySettings: 0x{:X}", reinterpret_cast<uintptr_t>(m_exports.SetLibrarySettings));

    if (!m_exports.IsValid()) {
        NOBLE_ERROR("One or more required exports from haloreach.dll could not be resolved!");
        return false;
    }

    NOBLE_INFO("Engine interfaces verified and ready for initialization.");
    return true;
}

void Loader::Unload() {
    if (m_hModule) {
        NOBLE_INFO("Unloading haloreach.dll from process memory...");
        FreeLibrary(m_hModule);
        m_hModule = nullptr;
        m_baseAddress = 0;
        m_moduleSize = 0;
        m_exports = ReachExports();
    }
}

} // namespace noble
