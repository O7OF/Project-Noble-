#include "SteamWorkshop.hpp"
#include "common/Logger.hpp"
#include "vfs/VFS.hpp"

#include <windows.h>
#include <shellapi.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace noble {
namespace SteamWorkshop {

namespace fs = std::filesystem;

namespace {

// MCC's own manifest is the authority for "Steam finished downloading this item". The ACF lists every
// fully installed item under "WorkshopItemsInstalled"; while an item downloads it is absent from that
// block, so a folder that appears early in the cache cannot make us join a half-downloaded map pack.
bool WorkshopManifestLists(const std::string& publishedFileId) {
    const fs::path acf = VFS::GetWorkshopPath().parent_path().parent_path() / "appworkshop_976730.acf";
    std::ifstream in(acf, std::ios::binary);
    if (!in) return true;   // no manifest to consult: trust the folder check below
    const std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    const size_t begin = text.find("\"WorkshopItemsInstalled\"");
    if (begin == std::string::npos) return true;
    const size_t end = text.find("\"WorkshopItemDetails\"", begin);
    const std::string block = text.substr(begin, end == std::string::npos ? std::string::npos : end - begin);
    return block.find("\"" + publishedFileId + "\"") != std::string::npos;
}

bool ReadJson(const fs::path& path, nlohmann::json& out) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (text.size() >= 3 && static_cast<unsigned char>(text[0]) == 0xEF) text.erase(0, 3);
    out = nlohmann::json::parse(text, nullptr, false, true);
    return !out.is_discarded() && out.is_object();
}

std::string LocalisedText(const nlohmann::json& j, const char* key) {
    if (!j.contains(key)) return {};
    const nlohmann::json& v = j[key];
    if (v.is_string()) return v.get<std::string>();
    if (v.is_object()) {
        if (v.contains("Neutral") && v["Neutral"].is_string()) return v["Neutral"].get<std::string>();
        for (const auto& item : v.items()) if (item.value().is_string()) return item.value().get<std::string>();
    }
    return {};
}

std::string WorkshopIdFromModInfo(const fs::path& folder) {
    nlohmann::json info;
    if (!ReadJson(folder / "ModInfo.json", info)) return {};
    if (!info.contains("ModIdentifier") || !info["ModIdentifier"].is_object()) return {};
    const nlohmann::json& identifier = info["ModIdentifier"];
    if (!identifier.contains("HostedModIds") || !identifier["HostedModIds"].is_object()) return {};
    const nlohmann::json& hosted = identifier["HostedModIds"];
    if (!hosted.contains("SteamWorkshopId")) return {};
    const nlohmann::json& id = hosted["SteamWorkshopId"];
    if (id.is_string()) return id.get<std::string>();
    if (id.is_number_unsigned()) return std::to_string(id.get<uint64_t>());
    if (id.is_number_integer() && id.get<int64_t>() > 0) return std::to_string(id.get<int64_t>());
    return {};
}

std::string TitleFromModInfo(const fs::path& folder) {
    nlohmann::json info;
    if (!ReadJson(folder / "ModInfo.json", info)) return {};
    return LocalisedText(info, "Title");
}

bool HasModInfo(const fs::path& folder) {
    std::error_code ec;
    return fs::is_regular_file(folder / "ModInfo.json", ec);
}

// A Workshop-cache folder is this item when its manifest says so, or (older manifests without
// ModIdentifier) when the folder is named after the published-file id.
bool FolderIsItem(const fs::path& folder, const std::string& publishedFileId) {
    const std::string hosted = WorkshopIdFromModInfo(folder);
    if (!hosted.empty()) return hosted == publishedFileId;
    return folder.filename().string() == publishedFileId;
}

std::wstring ToWide(const std::string& text) {
    if (text.empty()) return {};
    const int len = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    if (len <= 0) return {};
    std::wstring out(static_cast<size_t>(len - 1), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, out.data(), len);
    return out;
}

std::vector<fs::path> ChildLogPaths() {
    std::vector<fs::path> paths;
    wchar_t temp[MAX_PATH] = {};
    const DWORD len = GetTempPathW(MAX_PATH, temp);
    if (len > 0 && len < MAX_PATH) paths.push_back(fs::path(temp) / "noble-steam-subscribe.log");
    wchar_t exe[MAX_PATH] = {};
    if (GetModuleFileNameW(nullptr, exe, MAX_PATH)) {
        paths.push_back(fs::path(exe).parent_path() / "noble-steam-subscribe.log");
    }
    return paths;
}

void ChildLog(const std::string& text) {
    SYSTEMTIME now{};
    GetLocalTime(&now);
    char stamp[64];
    std::snprintf(stamp, sizeof(stamp), "%04u-%02u-%02u %02u:%02u:%02u", now.wYear, now.wMonth, now.wDay,
                  now.wHour, now.wMinute, now.wSecond);
    const std::string line = "[" + std::string(stamp) + "] " + text + "\n";
    for (const fs::path& path : ChildLogPaths()) {
        std::ofstream out(path, std::ios::app);
        if (!out) continue;
        out << line;
        out.close();
        return;
    }
}

struct AppIdFile {
    fs::path path;
    bool created = false;
};

std::vector<AppIdFile> WriteAppIdFiles() {
    std::vector<AppIdFile> files;
    std::vector<fs::path> dirs;
    wchar_t exe[MAX_PATH] = {};
    if (GetModuleFileNameW(nullptr, exe, MAX_PATH)) dirs.push_back(fs::path(exe).parent_path());
    std::error_code ec;
    const fs::path cwd = fs::current_path(ec);
    if (!ec) dirs.push_back(cwd);

    for (const fs::path& dir : dirs) {
        const fs::path path = dir / "steam_appid.txt";
        if (std::find_if(files.begin(), files.end(), [&](const AppIdFile& f) { return f.path == path; }) != files.end()) continue;
        std::error_code existsEc;
        if (fs::exists(path, existsEc)) { files.push_back({ path, false }); continue; }
        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        if (!out) continue;
        out << "976730\n";
        out.close();
        files.push_back({ path, true });
    }
    return files;
}

void RemoveCreatedAppIdFiles(const std::vector<AppIdFile>& files) {
    for (const AppIdFile& file : files) {
        if (!file.created) continue;
        std::error_code ec;
        fs::remove(file.path, ec);
    }
}

bool SpawnSubscribeChild(const std::string& publishedFileId, const fs::path& steamApiDll, void*& process) {
    process = nullptr;
    wchar_t exe[MAX_PATH] = {};
    if (!GetModuleFileNameW(nullptr, exe, MAX_PATH)) return false;
    std::wstring command = L"\"" + std::wstring(exe) + L"\" -steam-subscribe " + ToWide(publishedFileId) +
                           L" -steam-dll \"" + steamApiDll.wstring() + L"\"";
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION info{};
    if (!CreateProcessW(exe, command.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr,
                        &startup, &info)) {
        return false;
    }
    CloseHandle(info.hThread);
    process = info.hProcess;
    return true;
}

} // namespace

bool IsPublishedFileId(const std::string& id) {
    if (id.empty() || id.size() > 20) return false;
    if (!std::all_of(id.begin(), id.end(), [](unsigned char c) { return std::isdigit(c) != 0; })) return false;
    return id.find_first_not_of('0') != std::string::npos;   // zero is not an item
}

fs::path FindSteamApi64() {
    const fs::path reach = VFS::GetBaseMccPath();
    if (reach.empty()) return {};
    const fs::path candidates[] = {
        reach.parent_path() / "MCC" / "Binaries" / "Win64" / "steam_api64.dll",
        reach / "steam_api64.dll",
        reach.parent_path() / "steam_api64.dll",
    };
    std::error_code ec;
    for (const fs::path& candidate : candidates) {
        if (fs::is_regular_file(candidate, ec)) return candidate;
    }
    return {};
}

fs::path FindInstalledFolder(const std::string& publishedFileId) {
    if (!IsPublishedFileId(publishedFileId)) return {};
    std::error_code ec;

    // 1. The Steam Workshop cache. The manifest has to list the item, otherwise Steam is still
    //    downloading it (or queued it) and its folder is incomplete.
    const fs::path workshop = VFS::GetWorkshopPath();
    if (!workshop.empty() && fs::is_directory(workshop, ec) && WorkshopManifestLists(publishedFileId)) {
        // The cache folder is named after the published-file id; trust the manifest rather than the
        // item's own ModInfo (a collection republished under one id can carry another item's manifest).
        const fs::path direct = workshop / publishedFileId;
        if (HasModInfo(direct) && WorkshopManifestLists(publishedFileId)) return direct;
        for (const auto& entry : fs::directory_iterator(workshop, ec)) {
            if (ec || !entry.is_directory(ec)) continue;
            if (HasModInfo(entry.path()) && FolderIsItem(entry.path(), publishedFileId)) return entry.path();
        }
    }

    // 2. Noble's own data\mods (a mod copied out of Steam, or a local build of the same Workshop item).
    const fs::path localRoot = VFS::GetUserStoragePath() / "mods";
    if (!localRoot.empty() && fs::is_directory(localRoot, ec)) {
        const fs::path direct = localRoot / publishedFileId;
        if (HasModInfo(direct)) return direct;
        for (const auto& entry : fs::directory_iterator(localRoot, ec)) {
            if (ec || !entry.is_directory(ec)) continue;
            if (HasModInfo(entry.path()) && FolderIsItem(entry.path(), publishedFileId)) return entry.path();
        }
    }
    return {};
}

bool OpenWorkshopPage(const std::string& publishedFileId) {
    if (!IsPublishedFileId(publishedFileId)) return false;
    const std::string steamUrl = "steam://url/CommunityFilePage/" + publishedFileId;
    const HINSTANCE launched = ShellExecuteA(nullptr, "open", steamUrl.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    if (reinterpret_cast<INT_PTR>(launched) > 32) return true;
    const std::string webUrl = "https://steamcommunity.com/sharedfiles/filedetails/?id=" + publishedFileId;
    const HINSTANCE web = ShellExecuteA(nullptr, "open", webUrl.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    return reinterpret_cast<INT_PTR>(web) > 32;
}

Download::~Download() {
    CloseChild();
}

void Download::CloseChild() {
    if (m_child) {
        CloseHandle(static_cast<HANDLE>(m_child));
        m_child = nullptr;
    }
}

void Download::StartManualFallback(const std::string& reason) {
    const bool opened = OpenWorkshopPage(m_id);
    if (opened) {
        m_stage = Stage::Manual;
        m_message = reason + " Subscribe to " + (m_title.empty() ? "the mod" : "\"" + m_title + "\"") +
                    " in Steam; Noble joins automatically when it has finished downloading.";
    } else {
        m_stage = Stage::Failed;
        m_message = reason + " Steam is not running and the Workshop page could not be opened.";
    }
}

bool Download::Start(const std::string& publishedFileId, const std::string& title, std::string& message) {
    CloseChild();
    m_id = publishedFileId;
    m_title = title;
    m_folder.clear();
    m_stage = Stage::Idle;
    m_message.clear();

    if (!IsPublishedFileId(publishedFileId)) {
        m_stage = Stage::Failed;
        m_message = "This lobby's mod id is not a Steam Workshop item.";
        message = m_message;
        return false;
    }

    m_folder = FindInstalledFolder(publishedFileId);
    if (!m_folder.empty()) {
        m_stage = Stage::Done;
        m_message = "The mod is already installed.";
        message = m_message;
        return true;
    }

    const fs::path steamApi = FindSteamApi64();
    if (!steamApi.empty()) {
        void* process = nullptr;
        if (SpawnSubscribeChild(publishedFileId, steamApi, process)) {
            m_child = process;
            m_stage = Stage::Starting;
            m_startedAt = GetTickCount64();
            m_lastCheck = 0;
            m_message = "Asking Steam to download " + (title.empty() ? "the mod" : "\"" + title + "\"") + "...";
            message = m_message;
            return true;
        }
    }

    StartManualFallback(steamApi.empty() ? "Steam's API was not found." : "Steam could not be started automatically.");
    message = m_message;
    return m_stage != Stage::Failed;
}

void Download::Tick() {
    if (m_stage == Stage::Idle || m_stage == Stage::Done || m_stage == Stage::Failed) return;

    const unsigned long long now = GetTickCount64();
    if (m_lastCheck == 0 || now - m_lastCheck >= 500) {
        m_lastCheck = now;
        fs::path installed = FindInstalledFolder(m_id);
        if (!installed.empty()) {
            m_folder = std::move(installed);
            m_stage = Stage::Done;
            m_message = "The mod finished downloading.";
            CloseChild();
            return;
        }
    }

    if (m_stage == Stage::Starting && m_child) {
        const DWORD wait = WaitForSingleObject(static_cast<HANDLE>(m_child), 0);
        if (wait == WAIT_OBJECT_0) {
            DWORD code = 1;
            GetExitCodeProcess(static_cast<HANDLE>(m_child), &code);
            CloseChild();
            if (code == 0) {
                m_stage = Stage::Downloading;
                m_message = "Steam is downloading " + (m_title.empty() ? "the mod" : "\"" + m_title + "\"") +
                            "... Noble joins automatically when it finishes.";
            } else {
                NOBLE_WARN("SteamWorkshop: subscribe helper exited with code {}", code);
                StartManualFallback("Steam could not start the download automatically.");
            }
        } else if (now - m_startedAt > 30000) {
            TerminateProcess(static_cast<HANDLE>(m_child), 1);
            CloseChild();
            NOBLE_WARN("SteamWorkshop: subscribe helper timed out");
            StartManualFallback("Steam did not answer.");
        }
    }
}

void Download::Cancel() {
    m_stage = Stage::Idle;
    CloseChild();   // a subscription already requested keeps downloading in Steam, which is what the player asked for
}

int SubscribeChildMain(const std::string& publishedFileId, const fs::path& steamApiDll) {
    ChildLog("helper start id=" + publishedFileId + " dll=" + steamApiDll.string());
    if (!IsPublishedFileId(publishedFileId)) {
        ChildLog("invalid published-file id");
        return 5;
    }
    if (!fs::exists(steamApiDll)) {
        ChildLog("steam_api64.dll not found: " + steamApiDll.string());
        return 2;
    }

    HMODULE module = LoadLibraryW(steamApiDll.c_str());
    if (!module) {
        ChildLog("LoadLibrary failed for " + steamApiDll.string());
        return 2;
    }

    using InitFn = bool (*)();
    using ShutdownFn = void (*)();
    using RunCallbacksFn = void (*)();
    using UgcFn = void* (*)();
    using SubscribeFn = unsigned long long (*)(void*, unsigned long long);
    using ItemStateFn = unsigned int (*)(void*, unsigned long long, unsigned int);

    InitFn init = reinterpret_cast<InitFn>(GetProcAddress(module, "SteamAPI_Init"));
    ShutdownFn shutdown = reinterpret_cast<ShutdownFn>(GetProcAddress(module, "SteamAPI_Shutdown"));
    RunCallbacksFn runCallbacks = reinterpret_cast<RunCallbacksFn>(GetProcAddress(module, "SteamAPI_RunCallbacks"));
    SubscribeFn subscribe = reinterpret_cast<SubscribeFn>(GetProcAddress(module, "SteamAPI_ISteamUGC_SubscribeItem"));
    ItemStateFn itemState = reinterpret_cast<ItemStateFn>(GetProcAddress(module, "SteamAPI_ISteamUGC_GetItemState"));

    UgcFn ugcAccessor = nullptr;
    for (const char* name : { "SteamAPI_SteamUGC_v016", "SteamAPI_SteamUGC_v017", "SteamAPI_SteamUGC_v018",
                              "SteamAPI_SteamUGC_v019", "SteamAPI_SteamUGC_v020", "SteamAPI_SteamUGC_v021" }) {
        ugcAccessor = reinterpret_cast<UgcFn>(GetProcAddress(module, name));
        if (ugcAccessor && subscribe && itemState) break;
    }
    if (!init || !ugcAccessor || !subscribe || !itemState) {
        ChildLog("steam_api64.dll is missing the UGC exports");
        FreeLibrary(module);
        return 2;
    }

    const std::vector<AppIdFile> appIdFiles = WriteAppIdFiles();
    ChildLog("calling SteamAPI_Init");
    const bool started = init();
    RemoveCreatedAppIdFiles(appIdFiles);
    if (!started) {
        ChildLog("SteamAPI_Init failed (is Steam running and logged in?)");
        FreeLibrary(module);
        return 3;
    }

    void* ugc = ugcAccessor();
    if (!ugc) {
        ChildLog("SteamUGC interface unavailable");
        if (shutdown) shutdown();
        FreeLibrary(module);
        return 4;
    }

    const unsigned long long fileId = std::strtoull(publishedFileId.c_str(), nullptr, 10);
    subscribe(ugc, fileId);
    unsigned int state = 0;
    for (int attempt = 0; attempt < 100; ++attempt) {
        if (runCallbacks) runCallbacks();
        state = itemState(ugc, fileId, 0xFFFFFFFFu);
        const unsigned int wanted = 0x1u /*subscribed*/ | 0x4u /*installed*/ | 0x10u /*downloading*/ | 0x20u /*pending*/;
        if ((state & wanted) != 0) break;
        Sleep(100);
    }
    ChildLog("subscribe finished, item state 0x" + std::to_string(state));
    if (shutdown) shutdown();
    FreeLibrary(module);
    return (state & 0x35u) != 0 ? 0 : 4;
}

} // namespace SteamWorkshop
} // namespace noble
