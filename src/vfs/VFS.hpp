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
    static bool HasActiveMod();

    // Serve <mapFile> whenever the engine asks for the stock map's haloreach\maps\<carrierScenario>.map.
    // The launch index and map id stay the stock map's (the engine selects the stock entry), while the
    // map data, header and scenario are the mod's. Pass an empty path to clear it.
    static void SetMapOverride(const std::string& carrierScenario, const fs::path& mapFile);
    static void ClearMapOverride();
    // The stem of the .map being substituted (e.g. "ff_unearthed" when a mod serves its map for a campaign slot), or
    // empty when no override is active. Mod maps are often installed against a stock carrier, so this is the only
    // place that knows which canvas the map was really built on.
    static std::string MapOverrideCanvas();
    static const std::filesystem::path& MapOverrideFile();

    // NOBLE_MAP_READ_TRACE=1: log every read from a .map handle (offset + size). The last line before an engine
    // launch routine unwinds says which part of the map it gave up on.
    static void InstallMapReadTrace();

    // Write a multiplayer-flavoured copy of <scenario>.mapinfo into user storage (VFS serves it first), so a mod
    // map built on a Firefight canvas can be launched through the multiplayer/Forge engine.
    static bool EnsureMultiplayerMapInfo(const std::string& scenario);

    // True when MCC ships <scenario> as a Firefight map (its map info says so).
    static bool IsFirefightCanvas(const std::string& scenario);
    static std::unordered_map<HANDLE, std::wstring> s_mapHandles;
    static std::mutex s_mapHandleMutex;

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
    static fs::path s_localDataDir;   // <exe>/data: loose game data a builder/user supplies
    static fs::path s_workshopPath;
    static fs::path s_activeMod;
    static fs::path s_mapOverrideFile;
    static std::string s_mapOverrideCarrier;   // lower case, no ".map"
    static bool s_isInitialized;
    static std::mutex s_mutex;
};

} // namespace noble
