#pragma once

#include "common/Types.hpp"
#include <windows.h>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <mutex>

namespace noble {

class VFS {
public:
    static bool Initialize(const ReachInstallInfo& installInfo, const fs::path& userStoragePath);
    static void Shutdown();

    // Hooks haloreach.dll's IAT for file I/O operations (CreateFileW, GetFileAttributesW, etc.)
    static bool HookTargetModule(HMODULE hTargetModule);
    static void UnhookTargetModule();

    // Resolves a requested file path to its redirected VFS target
    static std::wstring ResolvePath(const std::wstring& requestedPath, DWORD desiredAccess);

    static const fs::path& GetBaseMccPath() { return s_baseMccPath; }
    static const fs::path& GetUserStoragePath() { return s_userStoragePath; }
    static const fs::path& GetWorkshopPath() { return s_workshopPath; }

    // A mod folder (Workshop item or data\mods\<name>) whose files the engine reads before MCC's,
    // e.g. <mod>\maps\ff60_icecave.map for haloreach\maps\ff60_icecave.map. Empty = no mod.
    static void SetActiveMod(const fs::path& modFolder);

private:
    static bool RawPathExists(const std::wstring& path);

    // Detour function definitions
    static HANDLE WINAPI HookedCreateFileW(
        LPCWSTR lpFileName,
        DWORD dwDesiredAccess,
        DWORD dwShareMode,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        DWORD dwCreationDisposition,
        DWORD dwFlagsAndAttributes,
        HANDLE hTemplateFile
    );

    static DWORD WINAPI HookedGetFileAttributesW(LPCWSTR lpFileName);

    static BOOL WINAPI HookedGetFileAttributesExW(
        LPCWSTR lpFileName,
        GET_FILEEX_INFO_LEVELS fInfoLevelId,
        LPVOID lpFileInformation
    );

    // Original API pointers
    using CreateFileW_Fn = HANDLE(WINAPI*)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
    using GetFileAttributesW_Fn = DWORD(WINAPI*)(LPCWSTR);
    using GetFileAttributesExW_Fn = BOOL(WINAPI*)(LPCWSTR, GET_FILEEX_INFO_LEVELS, LPVOID);

    static CreateFileW_Fn s_origCreateFileW;
    static GetFileAttributesW_Fn s_origGetFileAttributesW;
    static GetFileAttributesExW_Fn s_origGetFileAttributesExW;

    static fs::path s_baseMccPath;
    static fs::path s_userStoragePath;
    static fs::path s_workshopPath;
    static fs::path s_activeMod;
    static bool s_isInitialized;
    static std::mutex s_mutex;
};

} // namespace noble
