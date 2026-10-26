#include "VFS.hpp"
#include "hooks/HookManager.hpp"
#include "common/Logger.hpp"

#include <algorithm>

namespace noble {

VFS::CreateFileW_Fn VFS::s_origCreateFileW = nullptr;
VFS::GetFileAttributesW_Fn VFS::s_origGetFileAttributesW = nullptr;
VFS::GetFileAttributesExW_Fn VFS::s_origGetFileAttributesExW = nullptr;

fs::path VFS::s_baseMccPath;
fs::path VFS::s_userStoragePath;
fs::path VFS::s_workshopPath;
bool VFS::s_isInitialized = false;
std::mutex VFS::s_mutex;

// Thread-local recursion guard to prevent re-entrancy within hook calls
static thread_local bool t_inVfsHook = false;

static std::wstring NormalizePathString(std::wstring pathStr) {
    std::replace(pathStr.begin(), pathStr.end(), L'/', L'\\');
    return pathStr;
}

// Low-level existence check that directly invokes the unhooked original Win32 API
bool VFS::RawPathExists(const std::wstring& path) {
    if (!s_origGetFileAttributesW) return false;
    DWORD attr = s_origGetFileAttributesW(path.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES);
}

bool VFS::Initialize(const ReachInstallInfo& installInfo, const fs::path& userStoragePath) {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (s_isInitialized) return true;

    s_baseMccPath = fs::absolute(installInfo.reachDirectory);
    s_userStoragePath = fs::absolute(userStoragePath);
    s_workshopPath = fs::absolute(installInfo.workshopDirectory);

    // Create user storage subfolders for mods, saves, and logs
    std::error_code ec;
    fs::create_directories(s_userStoragePath / "maps", ec);
    fs::create_directories(s_userStoragePath / "mods", ec);
    fs::create_directories(s_userStoragePath / "game_variants", ec);
    fs::create_directories(s_userStoragePath / "map_variants", ec);

    NOBLE_INFO("Initializing Virtual File System (VFS):");
    NOBLE_INFO(" - Base MCC Read-Only Path:  {}", s_baseMccPath.string());
    NOBLE_INFO(" - User Storage Output Path: {}", s_userStoragePath.string());
    NOBLE_INFO(" - Workshop Content Path:    {}", s_workshopPath.string());

    if (!InstallHooks()) {
        NOBLE_ERROR("Failed to install VFS Win32 API hooks.");
        return false;
    }

    s_isInitialized = true;
    NOBLE_INFO("VFS initialized and active. Retail game files are protected as read-only.");
    return true;
}

void VFS::Shutdown() {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!s_isInitialized) return;

    RemoveHooks();
    s_isInitialized = false;
    NOBLE_INFO("VFS shutdown complete.");
}

std::wstring VFS::ResolvePath(const std::wstring& requestedPath, DWORD desiredAccess) {
    if (requestedPath.empty()) return requestedPath;

    std::wstring normalized = NormalizePathString(requestedPath);
    fs::path p(normalized);

    // Check if it's a writing operation
    bool isWriting = (desiredAccess & (GENERIC_WRITE | FILE_WRITE_DATA | FILE_APPEND_DATA)) != 0;

    // Check if requested path starts with or is inside base MCC path
    std::wstring baseStr = s_baseMccPath.wstring();
    bool isTargetingMcc = false;

    if (p.is_absolute()) {
        if (normalized.rfind(baseStr, 0) == 0) {
            isTargetingMcc = true;
            // Extract the relative subpath within haloreach
            p = p.lexically_relative(s_baseMccPath);
        }
    } else {
        // Relative paths from the engine (e.g. "maps\forge_halo.map")
        isTargetingMcc = true;
    }

    if (isTargetingMcc) {
        // 1. If writing, REDIRECT to User Storage! Never write to base MCC.
        if (isWriting) {
            fs::path redirected = s_userStoragePath / p;
            std::error_code ec;
            fs::create_directories(redirected.parent_path(), ec);
            return redirected.wstring();
        }

        // 2. If reading:
        // Priority A: Check user storage (custom downloaded mods / custom variants)
        fs::path userOverride = s_userStoragePath / p;
        if (RawPathExists(userOverride.wstring())) {
            return userOverride.wstring();
        }

        // Priority B: Check user mods folder (e.g. data/mods/...)
        fs::path userMod = s_userStoragePath / "mods" / p;
        if (RawPathExists(userMod.wstring())) {
            return userMod.wstring();
        }

        // Priority C: Check base MCC retail directory (maps, audio, tags)
        fs::path baseAsset = s_baseMccPath / p;
        if (RawPathExists(baseAsset.wstring())) {
            return baseAsset.wstring();
        }
    }

    return requestedPath;
}

HANDLE WINAPI VFS::HookedCreateFileW(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
) {
    if (t_inVfsHook || !s_isInitialized || !lpFileName) {
        return s_origCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    }

    t_inVfsHook = true;
    std::wstring resolved = ResolvePath(lpFileName, dwDesiredAccess);
    HANDLE hFile = s_origCreateFileW(
        resolved.c_str(),
        dwDesiredAccess,
        dwShareMode,
        lpSecurityAttributes,
        dwCreationDisposition,
        dwFlagsAndAttributes,
        hTemplateFile
    );
    t_inVfsHook = false;

    return hFile;
}

DWORD WINAPI VFS::HookedGetFileAttributesW(LPCWSTR lpFileName) {
    if (t_inVfsHook || !s_isInitialized || !lpFileName) {
        return s_origGetFileAttributesW(lpFileName);
    }

    t_inVfsHook = true;
    std::wstring resolved = ResolvePath(lpFileName, GENERIC_READ);
    DWORD result = s_origGetFileAttributesW(resolved.c_str());
    t_inVfsHook = false;

    return result;
}

BOOL WINAPI VFS::HookedGetFileAttributesExW(
    LPCWSTR lpFileName,
    GET_FILEEX_INFO_LEVELS fInfoLevelId,
    LPVOID lpFileInformation
) {
    if (t_inVfsHook || !s_isInitialized || !lpFileName) {
        return s_origGetFileAttributesExW(lpFileName, fInfoLevelId, lpFileInformation);
    }

    t_inVfsHook = true;
    std::wstring resolved = ResolvePath(lpFileName, GENERIC_READ);
    BOOL result = s_origGetFileAttributesExW(resolved.c_str(), fInfoLevelId, lpFileInformation);
    t_inVfsHook = false;

    return result;
}

bool VFS::InstallHooks() {
    HMODULE hKernel32 = GetModuleHandleW(L"kernel32.dll");
    if (!hKernel32) return false;

    void* pCreateFileW = reinterpret_cast<void*>(GetProcAddress(hKernel32, "CreateFileW"));
    void* pGetFileAttributesW = reinterpret_cast<void*>(GetProcAddress(hKernel32, "GetFileAttributesW"));
    void* pGetFileAttributesExW = reinterpret_cast<void*>(GetProcAddress(hKernel32, "GetFileAttributesExW"));

    bool ok = true;
    ok &= HookManager::InstallHook(pCreateFileW, reinterpret_cast<void*>(&HookedCreateFileW), reinterpret_cast<void**>(&s_origCreateFileW), "CreateFileW");
    ok &= HookManager::InstallHook(pGetFileAttributesW, reinterpret_cast<void*>(&HookedGetFileAttributesW), reinterpret_cast<void**>(&s_origGetFileAttributesW), "GetFileAttributesW");
    ok &= HookManager::InstallHook(pGetFileAttributesExW, reinterpret_cast<void*>(&HookedGetFileAttributesExW), reinterpret_cast<void**>(&s_origGetFileAttributesExW), "GetFileAttributesExW");

    return ok;
}

void VFS::RemoveHooks() {
    HMODULE hKernel32 = GetModuleHandleW(L"kernel32.dll");
    if (hKernel32) {
        void* pCreateFileW = reinterpret_cast<void*>(GetProcAddress(hKernel32, "CreateFileW"));
        void* pGetFileAttributesW = reinterpret_cast<void*>(GetProcAddress(hKernel32, "GetFileAttributesW"));
        void* pGetFileAttributesExW = reinterpret_cast<void*>(GetProcAddress(hKernel32, "GetFileAttributesExW"));

        HookManager::RemoveHook(pCreateFileW);
        HookManager::RemoveHook(pGetFileAttributesW);
        HookManager::RemoveHook(pGetFileAttributesExW);
    }
}

} // namespace noble
