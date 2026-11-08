#include "Loader.hpp"
#include "EngineLimits.hpp"
#include "PoolReport.hpp"
#include "SpawnTest.hpp"
#include "SchedulerGuard.hpp"
#include "PoolResize.hpp"
#include "MemberTrace.hpp"
#include "SlotPatch.hpp"
#include "common/Logger.hpp"

#include <psapi.h>

#include <algorithm>
#include <cstring>
#include <cwchar>
#include <string>

namespace {

// When Noble loads its patched copy of haloreach.dll (EngineLimits), the engine still asks Windows where
// its own DLL lives and loads files next to it (bin\waves\*.dll for the Waves audio effects, and so on).
// Those files only exist in the MCC folder, so for the copy GetModuleFileNameW is redirected to report the
// installed stock DLL's path. Calls for any other module go straight to Windows.
HMODULE s_redirectedModule = nullptr;
std::wstring s_reportedPath;

DWORD WINAPI Hook_GetModuleFileNameW(HMODULE module, LPWSTR buffer, DWORD size) {
    if (s_redirectedModule && module == s_redirectedModule && !s_reportedPath.empty()) {
        if (size == 0 || buffer == nullptr) {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return 0;
        }
        const DWORD length = static_cast<DWORD>(s_reportedPath.size());
        if (length >= size) {   // same truncation behaviour as Windows
            wmemcpy(buffer, s_reportedPath.c_str(), size - 1);
            buffer[size - 1] = L'\0';
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return size;
        }
        wmemcpy(buffer, s_reportedPath.c_str(), length + 1);
        SetLastError(ERROR_SUCCESS);
        return length;
    }
    return GetModuleFileNameW(module, buffer, size);
}

// Points the module's import of `functionName` at `replacement`. Returns false if it is not imported.
bool PatchImport(HMODULE module, const char* functionName, void* replacement) {
    auto* base = reinterpret_cast<uint8_t*>(module);
    auto* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return false;
    auto* nt = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
    const IMAGE_DATA_DIRECTORY& dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (dir.VirtualAddress == 0) return false;
    bool patched = false;
    for (auto* desc = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(base + dir.VirtualAddress); desc->Name; ++desc) {
        if (!desc->OriginalFirstThunk || !desc->FirstThunk) continue;
        auto* names = reinterpret_cast<IMAGE_THUNK_DATA*>(base + desc->OriginalFirstThunk);
        auto* slots = reinterpret_cast<IMAGE_THUNK_DATA*>(base + desc->FirstThunk);
        for (; names->u1.AddressOfData; ++names, ++slots) {
            if (IMAGE_SNAP_BY_ORDINAL(names->u1.Ordinal)) continue;
            auto* byName = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(base + names->u1.AddressOfData);
            if (strcmp(reinterpret_cast<const char*>(byName->Name), functionName) != 0) continue;
            DWORD oldProtect = 0;
            if (!VirtualProtect(&slots->u1.Function, sizeof(slots->u1.Function), PAGE_READWRITE, &oldProtect)) continue;
            slots->u1.Function = reinterpret_cast<ULONG_PTR>(replacement);
            VirtualProtect(&slots->u1.Function, sizeof(slots->u1.Function), oldProtect, &oldProtect);
            patched = true;
        }
    }
    return patched;
}

} // namespace

namespace noble {

Loader::Loader() = default;

Loader::~Loader() {
    Unload();
}

bool Loader::LoadReachDll(const fs::path& dllPath) {
    return LoadReachDll(dllPath, dllPath);
}

bool Loader::LoadReachDll(const fs::path& dllPath, const fs::path& dependencyDll) {
    NOBLE_INFO("Bootstrapping Reach engine: Loading haloreach.dll into process memory...");
    NOBLE_INFO("Source DLL: {}", dllPath.string());
    const fs::path dependencyDir = dependencyDll.parent_path();
    if (dependencyDll != dllPath) {
        NOBLE_INFO("Dependencies resolved from the installed DLL folder: {}", dependencyDir.string());
    }

    if (!fs::exists(dllPath)) {
        NOBLE_ERROR("haloreach.dll does not exist at requested path: {}", dllPath.string());
        return false;
    }

    // Set DLL directory so haloreach.dll can find any sibling dependencies (e.g. fmod)
    SetDllDirectoryW(dependencyDir.c_str());

    // Load library with altered search path so dependencies in its directory resolve
    m_hModule = LoadLibraryExW(dllPath.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);

    // Keep a DLL directory set for the rest of the run, pointing at MCC's own binaries folder
    // (<MCC>\MCC\Binaries\Win64), as the reference host does. haloreach.dll loads
    // xaudio2_9redist.dll by bare name during engine start-up (for sound); MCC ships it there,
    // and without this the load fails and the game is silent.
    const fs::path mccBinaries = dependencyDir.parent_path() / L"MCC" / L"Binaries" / L"Win64";
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

    // A patched copy (EngineLimits) must still find the files that sit next to the installed DLL.
    if (dependencyDll != dllPath) {
        std::error_code pathEc;
        s_reportedPath = fs::absolute(dependencyDll, pathEc).wstring();
        std::replace(s_reportedPath.begin(), s_reportedPath.end(), L'/', L'\\');
        s_redirectedModule = m_hModule;
        if (PatchImport(m_hModule, "GetModuleFileNameW", reinterpret_cast<void*>(&Hook_GetModuleFileNameW))) {
            NOBLE_INFO("Patched copy reports its location as the installed DLL: {}", fs::path(s_reportedPath).string());
        } else {
            NOBLE_WARN("Could not redirect GetModuleFileNameW for the patched copy; files next to the DLL may not load");
        }
    }

    // Query module size
    MODULEINFO modInfo = { 0 };
    if (GetModuleInformation(GetCurrentProcess(), m_hModule, &modInfo, sizeof(modInfo))) {
        m_moduleSize = modInfo.SizeOfImage;
    }

    NOBLE_INFO("haloreach.dll successfully loaded at Base Address: 0x{:X} (Size: 0x{:X} bytes)", 
        m_baseAddress, m_moduleSize);

    // Development builds: log the engine's live pool sizes (object, actor, ...) so a run proves which limits
    // are in effect. No-op in release builds.
    pools::StartReporter(m_baseAddress, m_moduleSize, limits::ProfileName(limits::ActiveProfile()));
    spawn::Install(m_hModule);   // development builds: F9 / -spawn-test object stress test
    scheduler::InstallGuard(m_hModule);   // V16 layout only: wait for free jobs instead of crashing past ~6,500 moving objects
    pools::InstallResize(m_hModule, limits::ActiveProfile() != limits::Profile::Stock);   // opt-in: -ai-actors N
    limits::RegisterUnwindTables(m_hModule);   // profile Full only: unwind info for the relocated light-volume code
    membertrace::Install(m_hModule);   // dev only, opt-in -trace-members (C12 slot access map)
    slots::Install(m_hModule, limits::ActiveProfile() != limits::Profile::Stock);   // opt-in -slot-patch (C12)

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
        if (s_redirectedModule == m_hModule) {
            s_redirectedModule = nullptr;
            s_reportedPath.clear();
        }
        m_hModule = nullptr;
        m_baseAddress = 0;
        m_moduleSize = 0;
        m_exports = ReachExports();
    }
}

} // namespace noble
