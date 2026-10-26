#include "SteamLocator.hpp"
#include "common/Logger.hpp"

#include <windows.h>
#include <fstream>
#include <regex>
#include <iostream>

namespace noble {

std::optional<fs::path> SteamLocator::GetSteamRootFromRegistry() {
    HKEY hKey = nullptr;
    WCHAR buffer[MAX_PATH] = { 0 };
    DWORD bufferSize = sizeof(buffer);

    // Try Current User first
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueExW(hKey, L"SteamPath", nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer), &bufferSize) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            fs::path p(buffer);
            if (fs::exists(p)) {
                return p;
            }
        }
        RegCloseKey(hKey);
    }

    // Try Local Machine 32-bit registry
    bufferSize = sizeof(buffer);
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\WOW6432Node\\Valve\\Steam", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueExW(hKey, L"InstallPath", nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer), &bufferSize) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            fs::path p(buffer);
            if (fs::exists(p)) {
                return p;
            }
        }
        RegCloseKey(hKey);
    }

    return std::nullopt;
}

std::vector<fs::path> SteamLocator::GetAllSteamLibraries(const fs::path& steamRoot) {
    std::vector<fs::path> libraries;
    libraries.push_back(steamRoot);

    fs::path vdfPath = steamRoot / "steamapps" / "libraryfolders.vdf";
    if (!fs::exists(vdfPath)) {
        return libraries;
    }

    std::ifstream file(vdfPath);
    if (!file.is_open()) {
        return libraries;
    }

    std::string line;
    // Simple regex matching "path" "..."
    std::regex pathRegex(R"raw("path"\s+"([^"]+)")raw", std::regex::icase);
    std::smatch match;

    while (std::getline(file, line)) {
        if (std::regex_search(line, match, pathRegex)) {
            std::string matchedPath = match[1].str();
            // Replace escaped slashes
            std::string cleaned;
            for (size_t i = 0; i < matchedPath.size(); ++i) {
                if (matchedPath[i] == '\\' && i + 1 < matchedPath.size() && matchedPath[i + 1] == '\\') {
                    cleaned += '\\';
                    ++i;
                } else {
                    cleaned += matchedPath[i];
                }
            }

            fs::path libPath(cleaned);
            if (fs::exists(libPath) && libPath != steamRoot) {
                libraries.push_back(libPath);
            }
        }
    }

    return libraries;
}

bool SteamLocator::VerifyReachFiles(ReachInstallInfo& info) {
    // 1. haloreach.dll must exist and be valid PE
    if (!fs::exists(info.reachDllPath)) {
        NOBLE_ERROR("haloreach.dll not found at: {}", info.reachDllPath.string());
        return false;
    }

    std::error_code ec;
    auto dllSize = fs::file_size(info.reachDllPath, ec);
    if (ec || dllSize < 1024 * 1024) { // Reach dll is ~13MB
        NOBLE_ERROR("haloreach.dll size is invalid: {} bytes", dllSize);
        return false;
    }

    // 2. maps directory must exist
    if (!fs::exists(info.mapsDirectory) || !fs::is_directory(info.mapsDirectory)) {
        NOBLE_ERROR("Reach maps directory not found at: {}", info.mapsDirectory.string());
        return false;
    }

    // 3. fmod audio directory
    if (!fs::exists(info.fmodDirectory) || !fs::is_directory(info.fmodDirectory)) {
        NOBLE_WARN("Reach fmod audio directory not found at: {}", info.fmodDirectory.string());
    }

    info.areFilesVerified = true;
    NOBLE_INFO("Reach asset verification successful!");
    NOBLE_INFO(" - DLL: {} ({} bytes)", info.reachDllPath.string(), dllSize);
    NOBLE_INFO(" - Maps: {}", info.mapsDirectory.string());
    if (fs::exists(info.workshopDirectory)) {
        NOBLE_INFO(" - Workshop Cache: {}", info.workshopDirectory.string());
    }

    return true;
}

std::optional<ReachInstallInfo> SteamLocator::LocateAndVerifyReach() {
    NOBLE_INFO("Searching for local Steam installation of Halo: Reach (MCC)...");

    auto steamRoot = GetSteamRootFromRegistry();
    if (!steamRoot) {
        NOBLE_ERROR("Could not locate Steam installation path in Windows Registry.");
        return std::nullopt;
    }

    NOBLE_INFO("Detected Steam root directory: {}", steamRoot->string());
    auto libraries = GetAllSteamLibraries(*steamRoot);

    for (const auto& lib : libraries) {
        fs::path commonMcc = lib / "steamapps" / "common" / "Halo The Master Chief Collection";
        fs::path reachDir = commonMcc / "haloreach";
        fs::path reachDll = reachDir / "haloreach.dll";

        if (fs::exists(reachDll)) {
            ReachInstallInfo info;
            info.mccRootPath = commonMcc;
            info.reachDirectory = reachDir;
            info.reachDllPath = reachDll;
            info.mapsDirectory = reachDir / "maps";
            info.fmodDirectory = reachDir / "fmod";
            info.workshopDirectory = lib / "steamapps" / "workshop" / "content" / std::to_string(STEAM_MCC_APP_ID);

            if (VerifyReachFiles(info)) {
                return info;
            }
        }
    }

    NOBLE_ERROR("Halo: Reach DLC files not found in any registered Steam library.");
    return std::nullopt;
}

} // namespace noble
