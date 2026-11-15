#include "SteamWorkshop.hpp"
#include "common/Logger.hpp"
#include "vfs/VFS.hpp"

#include <windows.h>
#include <shellapi.h>
#include <winhttp.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <atomic>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#pragma comment(lib, "winhttp.lib")

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

bool SpawnSubscribeChild(const std::string& publishedFileId, const fs::path& steamApiDll,
                         const fs::path& progressPath, DWORD parentPid, void*& process) {
    process = nullptr;
    wchar_t exe[MAX_PATH] = {};
    if (!GetModuleFileNameW(nullptr, exe, MAX_PATH)) return false;
    std::wstring command = L"\"" + std::wstring(exe) + L"\" -steam-subscribe " + ToWide(publishedFileId) +
                           L" -steam-dll \"" + steamApiDll.wstring() + L"\"";
    if (!progressPath.empty()) {
        command += L" -steam-progress \"" + progressPath.wstring() + L"\"";
    }
    if (parentPid != 0) {
        command += L" -parent-pid " + std::to_wstring(parentPid);
    }
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

fs::path StopFileForProgress(const fs::path& progressPath) {
    if (progressPath.empty()) return {};
    return fs::path(progressPath.wstring() + L".stop");
}

void WriteProgressAtomic(const fs::path& progressPath, const std::string& stage, const std::string& error,
                         unsigned int state, unsigned long long downloaded, unsigned long long total,
                         const std::string& folder) {
    if (progressPath.empty()) return;
    std::error_code ec;
    fs::create_directories(progressPath.parent_path(), ec);
    nlohmann::json j = {
        { "stage", stage },
        { "error", error },
        { "state", state },
        { "downloaded", downloaded },
        { "total", total },
        { "folder", folder },
        { "ts", static_cast<unsigned long long>(GetTickCount64()) }
    };
    const fs::path tmpPath(progressPath.wstring() + L".tmp");
    {
        std::ofstream out(tmpPath, std::ios::binary | std::ios::trunc);
        if (!out) return;
        out << j.dump();
        out.close();
        if (!out.good()) return;
    }
    MoveFileExW(tmpPath.c_str(), progressPath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
}

bool IsProcessAlive(unsigned long pid) {
    if (pid == 0) return true;
    HANDLE h = OpenProcess(SYNCHRONIZE, FALSE, pid);
    if (!h) {
        return GetLastError() == ERROR_ACCESS_DENIED;
    }
    const DWORD wait = WaitForSingleObject(h, 0);
    CloseHandle(h);
    return wait == WAIT_TIMEOUT;
}

bool NonEmptyDir(const fs::path& folder) {
    std::error_code ec;
    if (!fs::is_directory(folder, ec)) return false;
    auto it = fs::directory_iterator(folder, ec);
    return !ec && it != fs::directory_iterator();
}

std::string SafeFileName(const std::string& raw, const std::string& fallback) {
    std::string out;
    out.reserve(raw.size());
    for (unsigned char c : raw) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-') {
            out.push_back(static_cast<char>(c));
        }
    }
    while (!out.empty() && out.front() == '.') out.erase(out.begin());
    return out.empty() ? fallback : out;
}

std::string FileNameFromUrl(const std::string& url, const std::string& fallback) {
    size_t q = url.find('?');
    std::string base = (q == std::string::npos) ? url : url.substr(0, q);
    size_t slash = base.find_last_of('/');
    std::string name = (slash == std::string::npos) ? base : base.substr(slash + 1);
    return SafeFileName(name, fallback);
}

bool IsZipFile(const fs::path& filePath) {
    std::ifstream in(filePath, std::ios::binary);
    if (!in) return false;
    unsigned char sig[4] = {};
    in.read(reinterpret_cast<char*>(sig), 4);
    return in.gcount() >= 4 && sig[0] == 0x50 && sig[1] == 0x4B;
}

bool ExtractZipToDir(const fs::path& zipPath, const fs::path& destDir, std::string& error) {
    std::error_code ec;
    fs::create_directories(destDir, ec);
    std::wstring ps = L"powershell.exe";
    std::wstring cmd = L"powershell.exe -NoProfile -NonInteractive -ExecutionPolicy Bypass -Command "
                       L"\"Expand-Archive -LiteralPath '" + zipPath.wstring() + L"' -DestinationPath '" +
                       destDir.wstring() + L"' -Force\"";
    STARTUPINFOW si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};
    if (!CreateProcessW(nullptr, cmd.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        error = "Could not launch PowerShell archive extractor";
        return false;
    }
    WaitForSingleObject(pi.hProcess, 180000);
    DWORD exitCode = 1;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    if (exitCode != 0 || !NonEmptyDir(destDir)) {
        error = "Archive extraction failed (exit " + std::to_string(exitCode) + ")";
        return false;
    }
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
        if ((HasModInfo(direct) || NonEmptyDir(direct)) && WorkshopManifestLists(publishedFileId)) return direct;
        for (const auto& entry : fs::directory_iterator(workshop, ec)) {
            if (ec || !entry.is_directory(ec)) continue;
            if (HasModInfo(entry.path()) && FolderIsItem(entry.path(), publishedFileId)) return entry.path();
        }
    }

    // 2. Noble's own data\mods (a mod copied out of Steam, direct HTTP install, or local build).
    const fs::path localRoot = VFS::GetUserStoragePath() / "mods";
    if (!localRoot.empty() && fs::is_directory(localRoot, ec)) {
        const fs::path direct = localRoot / publishedFileId;
        if (HasModInfo(direct) || NonEmptyDir(direct)) return direct;
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

struct Download::DirectState {
    std::mutex mutex;
    std::atomic<bool> cancelRequested{ false };
    bool done = false;
    bool failed = false;
    unsigned long long downloaded = 0;
    unsigned long long total = 0;
    std::string stageLabel = "Downloading directly...";
    std::string message;
    std::string error;
    fs::path installedFolder;
};

Download::~Download() {
    CloseChild(true);
    if (m_direct) m_direct->cancelRequested.store(true);
}

void Download::CloseChild(bool signalStop) {
    if (signalStop && !m_stopPath.empty()) {
        std::error_code ec;
        fs::create_directories(m_stopPath.parent_path(), ec);
        std::ofstream stopOut(m_stopPath, std::ios::binary | std::ios::trunc);
        if (stopOut) stopOut << "stop\n";
    }
    if (m_child) {
        if (signalStop) {
            WaitForSingleObject(static_cast<HANDLE>(m_child), 250);
        }
        CloseHandle(static_cast<HANDLE>(m_child));
        m_child = nullptr;
    }
}

void Download::SetHints(const std::string& directFileUrl, unsigned long long knownSizeBytes) {
    m_directFileUrl = directFileUrl;
    if (knownSizeBytes > 0) m_knownSizeBytes = knownSizeBytes;
}

double Download::EtaSeconds() const {
    if (m_speedBps <= 1.0) return -1.0;
    const unsigned long long total = ExpectedBytes();
    if (total == 0 || m_bytesDownloaded >= total) return -1.0;
    return static_cast<double>(total - m_bytesDownloaded) / m_speedBps;
}

float Download::ProgressFraction() const {
    if (m_stage == Stage::Done) return 1.0f;
    if (m_bytesTotal > 0) {
        double f = static_cast<double>(m_bytesDownloaded) / static_cast<double>(m_bytesTotal);
        if (f < 0.0) f = 0.0;
        if (f > 1.0) f = 1.0;
        return static_cast<float>(f);
    }
    if (m_knownSizeBytes > 0 && m_bytesDownloaded > 0) {
        double f = static_cast<double>(m_bytesDownloaded) / static_cast<double>(m_knownSizeBytes);
        if (f < 0.0) f = 0.0;
        if (f > 0.99) f = 0.99;
        return static_cast<float>(f);
    }
    return 0.0f;
}

bool Download::TryStartDirectHttp(const std::string& reason) {
    if (m_directFileUrl.empty()) return false;
    auto state = std::make_shared<DirectState>();
    state->total = m_knownSizeBytes;
    state->stageLabel = "Connecting to Workshop CDN...";
    state->message = "Downloading directly in-app (" + reason + ")...";
    m_direct = state;
    m_stage = Stage::DirectHttp;
    m_startedAt = GetTickCount64();
    m_speedSampleAt = m_startedAt;
    m_speedSampleBytes = 0;
    m_speedBps = 0.0;
    m_stageLabel = state->stageLabel;
    m_message = state->message;

    const std::string url = m_directFileUrl;
    const std::string id = m_id;
    const fs::path modsRoot = VFS::GetUserStoragePath() / "mods";

    std::thread([state, url, id, modsRoot]() {
        const std::wstring wide = ToWide(url);
        URL_COMPONENTS parts{};
        parts.dwStructSize = sizeof(parts);
        parts.dwSchemeLength = static_cast<DWORD>(-1);
        parts.dwHostNameLength = static_cast<DWORD>(-1);
        parts.dwUrlPathLength = static_cast<DWORD>(-1);
        parts.dwExtraInfoLength = static_cast<DWORD>(-1);
        if (!WinHttpCrackUrl(wide.c_str(), static_cast<DWORD>(wide.size()), 0, &parts)) {
            std::lock_guard<std::mutex> lock(state->mutex);
            state->error = "Workshop file URL is invalid.";
            state->failed = true;
            return;
        }
        const std::wstring host(parts.lpszHostName, parts.dwHostNameLength);
        std::wstring path = parts.dwUrlPathLength ? std::wstring(parts.lpszUrlPath, parts.dwUrlPathLength) : L"/";
        if (parts.dwExtraInfoLength) path += std::wstring(parts.lpszExtraInfo, parts.dwExtraInfoLength);
        const bool https = parts.nScheme == INTERNET_SCHEME_HTTPS;
        const INTERNET_PORT port = parts.nPort ? parts.nPort : (https ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT);

        std::error_code ec;
        fs::create_directories(modsRoot, ec);
        const fs::path finalDir = modsRoot / id;
        const fs::path stageDir = modsRoot / (id + ".staging");
        fs::remove_all(stageDir, ec);
        fs::create_directories(stageDir, ec);
        const std::string fileName = FileNameFromUrl(url, id + ".bin");
        const fs::path dlPath = modsRoot / (id + ".download.part");
        fs::remove(dlPath, ec);

        HINTERNET session = WinHttpOpen(L"ProjectNoble/0.1", WINHTTP_ACCESS_TYPE_NO_PROXY, nullptr, nullptr, 0);
        if (session) {
            DWORD policy = WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS;
            WinHttpSetOption(session, WINHTTP_OPTION_REDIRECT_POLICY, &policy, sizeof(policy));
            WinHttpSetTimeouts(session, 10000, 10000, 15000, 30000);
        }
        HINTERNET connect = session ? WinHttpConnect(session, host.c_str(), port, 0) : nullptr;
        HINTERNET request = connect ? WinHttpOpenRequest(connect, L"GET", path.c_str(), nullptr, WINHTTP_NO_REFERER,
                                                         WINHTTP_DEFAULT_ACCEPT_TYPES, https ? WINHTTP_FLAG_SECURE : 0) : nullptr;
        bool ok = false;
        std::string err = "Could not connect to Workshop CDN.";
        if (request && WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, nullptr, 0, 0, 0) &&
            WinHttpReceiveResponse(request, nullptr)) {
            DWORD status = 0, sz = sizeof(status);
            WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                                WINHTTP_HEADER_NAME_BY_INDEX, &status, &sz, WINHTTP_NO_HEADER_INDEX);
            unsigned long long contentLength = 0;
            wchar_t lenBuf[64] = {};
            DWORD lenBufSize = sizeof(lenBuf);
            if (WinHttpQueryHeaders(request, WINHTTP_QUERY_CONTENT_LENGTH, WINHTTP_HEADER_NAME_BY_INDEX,
                                    lenBuf, &lenBufSize, WINHTTP_NO_HEADER_INDEX)) {
                contentLength = _wcstoui64(lenBuf, nullptr, 10);
            }
            if (status >= 200 && status < 300) {
                {
                    std::lock_guard<std::mutex> lock(state->mutex);
                    if (contentLength > 0) state->total = contentLength;
                    state->stageLabel = "Downloading mod files...";
                    state->message = "Downloading mod package directly...";
                }
                std::ofstream out(dlPath, std::ios::binary | std::ios::trunc);
                if (out) {
                    std::vector<char> buffer(65536);
                    DWORD read = 0;
                    unsigned long long writtenTotal = 0;
                    bool writeOk = true;
                    while (!state->cancelRequested.load() &&
                           WinHttpReadData(request, buffer.data(), static_cast<DWORD>(buffer.size()), &read) && read > 0) {
                        out.write(buffer.data(), static_cast<std::streamsize>(read));
                        if (!out) {
                            writeOk = false;
                            err = "Disk write failed while downloading mod.";
                            break;
                        }
                        writtenTotal += read;
                        std::lock_guard<std::mutex> lock(state->mutex);
                        state->downloaded = writtenTotal;
                        if (state->total < writtenTotal) state->total = writtenTotal;
                    }
                    out.close();
                    if (state->cancelRequested.load()) {
                        err = "Download cancelled.";
                    } else if (writeOk && out.good() && writtenTotal > 0) {
                        {
                            std::lock_guard<std::mutex> lock(state->mutex);
                            state->stageLabel = "Extracting and installing mod files...";
                            state->message = "Installing downloaded files...";
                        }
                        if (IsZipFile(dlPath)) {
                            std::string extractErr;
                            if (ExtractZipToDir(dlPath, stageDir, extractErr)) {
                                fs::remove_all(finalDir, ec);
                                fs::rename(stageDir, finalDir, ec);
                                ok = !ec;
                                if (!ok) err = "Could not finalize mod directory.";
                            } else {
                                err = extractErr;
                            }
                        } else {
                            fs::rename(dlPath, stageDir / fileName, ec);
                            if (!ec) {
                                fs::remove_all(finalDir, ec);
                                fs::rename(stageDir, finalDir, ec);
                                ok = !ec;
                            }
                            if (!ok) err = "Could not move downloaded file into mod directory.";
                        }
                    }
                } else {
                    err = "Could not create temporary download file.";
                }
            } else {
                err = "Workshop CDN returned HTTP " + std::to_string(status) + ".";
            }
        }
        fs::remove(dlPath, ec);
        fs::remove_all(stageDir, ec);
        if (request) WinHttpCloseHandle(request);
        if (connect) WinHttpCloseHandle(connect);
        if (session) WinHttpCloseHandle(session);

        std::lock_guard<std::mutex> lock(state->mutex);
        if (ok) {
            state->installedFolder = finalDir;
            state->downloaded = state->total;
            state->done = true;
            state->stageLabel = "Complete";
            state->message = "Mod downloaded and installed.";
        } else if (!state->cancelRequested.load()) {
            state->error = err;
            state->failed = true;
        }
    }).detach();
    return true;
}

bool Download::Start(const std::string& publishedFileId, const std::string& title, std::string& message) {
    CloseChild(true);
    if (m_direct) {
        m_direct->cancelRequested.store(true);
        m_direct.reset();
    }
    m_id = publishedFileId;
    m_title = title;
    m_folder.clear();
    m_stage = Stage::Idle;
    m_message.clear();
    m_stageLabel.clear();
    m_bytesDownloaded = 0;
    m_bytesTotal = 0;
    m_steamState = 0;
    m_speedBps = 0.0;
    m_speedSampleBytes = 0;

    if (!IsPublishedFileId(publishedFileId)) {
        m_stage = Stage::Failed;
        m_message = "This lobby's mod id is not a valid Steam Workshop item.";
        message = m_message;
        return false;
    }

    m_folder = FindInstalledFolder(publishedFileId);
    if (!m_folder.empty()) {
        m_stage = Stage::Done;
        m_stageLabel = "Complete";
        m_message = "The mod is already installed.";
        message = m_message;
        return true;
    }

    const fs::path cacheDir = VFS::GetUserStoragePath() / "ui" / "cache" / "workshop";
    std::error_code ec;
    fs::create_directories(cacheDir, ec);
    m_progressPath = cacheDir / ("dl_" + publishedFileId + ".json");
    m_stopPath = StopFileForProgress(m_progressPath);
    fs::remove(m_progressPath, ec);
    fs::remove(m_stopPath, ec);

    const fs::path steamApi = FindSteamApi64();
    if (!steamApi.empty()) {
        void* process = nullptr;
        if (SpawnSubscribeChild(publishedFileId, steamApi, m_progressPath, GetCurrentProcessId(), process)) {
            m_child = process;
            m_stage = Stage::Starting;
            m_startedAt = GetTickCount64();
            m_lastCheck = 0;
            m_speedSampleAt = m_startedAt;
            m_stageLabel = "Initializing SteamUGC...";
            m_message = "Connecting to Steam for " + (title.empty() ? "the mod" : "\"" + title + "\"") + "...";
            message = m_message;
            return true;
        }
    }

    if (TryStartDirectHttp(steamApi.empty() ? "Steam API not present" : "Steam helper unavailable")) {
        message = m_message;
        return true;
    }

    m_stage = Stage::Failed;
    m_message = steamApi.empty()
        ? "MCC's steam_api64.dll was not found, so Steam Workshop downloads could not be started."
        : "Could not start the Steam download helper. Make sure Steam is running and try again.";
    message = m_message;
    return false;
}

void Download::Tick() {
    if (m_stage == Stage::Idle || m_stage == Stage::Done || m_stage == Stage::Failed) return;

    const unsigned long long now = GetTickCount64();

    if (m_stage == Stage::DirectHttp && m_direct) {
        std::lock_guard<std::mutex> lock(m_direct->mutex);
        m_bytesDownloaded = m_direct->downloaded;
        if (m_direct->total > 0) m_bytesTotal = m_direct->total;
        m_stageLabel = m_direct->stageLabel;
        m_message = m_direct->message;
        if (now - m_speedSampleAt >= 450) {
            const double dt = static_cast<double>(now - m_speedSampleAt) / 1000.0;
            const double inst = (m_bytesDownloaded >= m_speedSampleBytes && dt > 0.0)
                ? static_cast<double>(m_bytesDownloaded - m_speedSampleBytes) / dt
                : 0.0;
            m_speedBps = (m_speedBps <= 1.0) ? inst : (0.65 * m_speedBps + 0.35 * inst);
            m_speedSampleAt = now;
            m_speedSampleBytes = m_bytesDownloaded;
        }
        if (m_direct->done) {
            m_folder = m_direct->installedFolder;
            m_stage = Stage::Done;
        } else if (m_direct->failed) {
            m_stage = Stage::Failed;
            m_message = m_direct->error.empty() ? "Direct download failed." : m_direct->error;
        }
        return;
    }

    if (m_lastCheck == 0 || now - m_lastCheck >= 150) {
        m_lastCheck = now;

        nlohmann::json progress;
        if (!m_progressPath.empty() && ReadJson(m_progressPath, progress)) {
            const std::string pStage = progress.value("stage", std::string());
            const std::string pErr = progress.value("error", std::string());
            const std::string pFolder = progress.value("folder", std::string());
            const unsigned long long pDl = progress.value("downloaded", 0ULL);
            const unsigned long long pTot = progress.value("total", 0ULL);
            m_steamState = progress.value("state", 0u);

            if (pTot > m_bytesTotal) m_bytesTotal = pTot;
            if (pDl > m_bytesDownloaded) m_bytesDownloaded = pDl;
            if (m_bytesTotal > 0 && m_bytesDownloaded > m_bytesTotal) m_bytesDownloaded = m_bytesTotal;

            if (now - m_speedSampleAt >= 500) {
                const double dt = static_cast<double>(now - m_speedSampleAt) / 1000.0;
                const double inst = (m_bytesDownloaded >= m_speedSampleBytes && dt > 0.0)
                    ? static_cast<double>(m_bytesDownloaded - m_speedSampleBytes) / dt
                    : 0.0;
                m_speedBps = (m_speedBps <= 1.0) ? inst : (0.65 * m_speedBps + 0.35 * inst);
                m_speedSampleAt = now;
                m_speedSampleBytes = m_bytesDownloaded;
            }

            if (pStage == "pending") {
                m_stage = Stage::Downloading;
                m_stageLabel = "Allocating disk space in Steam...";
                m_message = "Steam is preparing to download " + (m_title.empty() ? "the mod" : "\"" + m_title + "\"") + "...";
            } else if (pStage == "downloading" || pStage == "updating") {
                m_stage = Stage::Downloading;
                m_stageLabel = (pStage == "updating") ? "Updating mod in Steam..." : "Downloading from Steam...";
                m_message = "Downloading " + (m_title.empty() ? "Workshop item" : "\"" + m_title + "\"") + " via Steam...";
            } else if (pStage == "verifying") {
                m_stage = Stage::Downloading;
                m_stageLabel = "Verifying installed files...";
                m_message = "Steam is verifying the installed mod files...";
            } else if (pStage == "done") {
                fs::path candidate = pFolder.empty() ? FindInstalledFolder(m_id) : fs::path(pFolder);
                if (!candidate.empty() && NonEmptyDir(candidate)) {
                    m_folder = std::move(candidate);
                    m_stage = Stage::Done;
                    m_stageLabel = "Complete";
                    if (m_bytesTotal > 0) m_bytesDownloaded = m_bytesTotal;
                    m_message = "The mod finished downloading.";
                    CloseChild(false);
                    return;
                }
            } else if (pStage == "failed" && !pErr.empty()) {
                m_message = pErr;
            }
        }

        fs::path installed = FindInstalledFolder(m_id);
        if (!installed.empty()) {
            m_folder = std::move(installed);
            m_stage = Stage::Done;
            m_stageLabel = "Complete";
            if (m_bytesTotal > 0) m_bytesDownloaded = m_bytesTotal;
            m_message = "The mod finished downloading.";
            CloseChild(true);
            return;
        }
    }

    if (m_child) {
        const DWORD wait = WaitForSingleObject(static_cast<HANDLE>(m_child), 0);
        if (wait == WAIT_OBJECT_0) {
            DWORD code = 1;
            GetExitCodeProcess(static_cast<HANDLE>(m_child), &code);
            CloseChild(false);
            if (code == 0) {
                fs::path installed = FindInstalledFolder(m_id);
                if (!installed.empty()) {
                    m_folder = std::move(installed);
                    m_stage = Stage::Done;
                    m_stageLabel = "Complete";
                    m_message = "The mod finished downloading.";
                    return;
                }
                // Check final folder written into progress JSON
                nlohmann::json progress;
                if (!m_progressPath.empty() && ReadJson(m_progressPath, progress)) {
                    const std::string pFolder = progress.value("folder", std::string());
                    if (!pFolder.empty() && NonEmptyDir(fs::path(pFolder))) {
                        m_folder = fs::path(pFolder);
                        m_stage = Stage::Done;
                        m_stageLabel = "Complete";
                        m_message = "The mod finished downloading.";
                        return;
                    }
                }
                m_stage = Stage::Downloading;
                m_stageLabel = "Downloading via Steam...";
                m_message = "Steam is finishing the download in the background...";
            } else {
                NOBLE_WARN("SteamWorkshop: subscribe helper exited with code {}", code);
                if (TryStartDirectHttp("Steam not active")) {
                    return;
                }
                m_stage = Stage::Failed;
                if (m_message.empty() || m_message.rfind("Connecting to Steam", 0) == 0 || m_message.rfind("Downloading", 0) == 0) {
                    m_message = (code == 3)
                        ? "Steam client is not running or signed in. Start Steam (you do not need to open MCC), then press RETRY."
                        : "Steam could not start this Workshop download. Make sure Steam is online and try again.";
                }
            }
        }
    }
}

void Download::Cancel() {
    m_stage = Stage::Idle;
    if (m_direct) {
        m_direct->cancelRequested.store(true);
        m_direct.reset();
    }
    CloseChild(true);
}

int SubscribeChildMain(const std::string& publishedFileId, const fs::path& steamApiDll,
                       const fs::path& progressPath, unsigned long parentPid) {
    ChildLog("helper start id=" + publishedFileId + " dll=" + steamApiDll.string() +
             " progress=" + progressPath.string());
    if (!IsPublishedFileId(publishedFileId)) {
        ChildLog("invalid published-file id");
        WriteProgressAtomic(progressPath, "failed", "Invalid Steam Workshop item ID.", 0, 0, 0, "");
        return 5;
    }
    if (!fs::exists(steamApiDll)) {
        ChildLog("steam_api64.dll not found: " + steamApiDll.string());
        WriteProgressAtomic(progressPath, "failed", "MCC steam_api64.dll was not found.", 0, 0, 0, "");
        return 2;
    }

    HMODULE module = LoadLibraryW(steamApiDll.c_str());
    if (!module) {
        ChildLog("LoadLibrary failed for " + steamApiDll.string());
        WriteProgressAtomic(progressPath, "failed", "Failed to load steam_api64.dll.", 0, 0, 0, "");
        return 2;
    }

    using InitFn = bool (*)();
    using ShutdownFn = void (*)();
    using RunCallbacksFn = void (*)();
    using UgcFn = void* (*)();
    using SubscribeFn = unsigned long long (*)(void*, unsigned long long);
    using DownloadItemFn = bool (*)(void*, unsigned long long, bool);
    using ItemStateFn = unsigned int (*)(void*, unsigned long long);
    using DownloadInfoFn = bool (*)(void*, unsigned long long, unsigned long long*, unsigned long long*);
    using InstallInfoFn = bool (*)(void*, unsigned long long, unsigned long long*, char*, unsigned int, unsigned int*);

    InitFn init = reinterpret_cast<InitFn>(GetProcAddress(module, "SteamAPI_Init"));
    ShutdownFn shutdown = reinterpret_cast<ShutdownFn>(GetProcAddress(module, "SteamAPI_Shutdown"));
    RunCallbacksFn runCallbacks = reinterpret_cast<RunCallbacksFn>(GetProcAddress(module, "SteamAPI_RunCallbacks"));
    SubscribeFn subscribe = reinterpret_cast<SubscribeFn>(GetProcAddress(module, "SteamAPI_ISteamUGC_SubscribeItem"));
    DownloadItemFn downloadItem = reinterpret_cast<DownloadItemFn>(GetProcAddress(module, "SteamAPI_ISteamUGC_DownloadItem"));
    ItemStateFn itemState = reinterpret_cast<ItemStateFn>(GetProcAddress(module, "SteamAPI_ISteamUGC_GetItemState"));
    DownloadInfoFn getDownloadInfo = reinterpret_cast<DownloadInfoFn>(GetProcAddress(module, "SteamAPI_ISteamUGC_GetItemDownloadInfo"));
    InstallInfoFn getInstallInfo = reinterpret_cast<InstallInfoFn>(GetProcAddress(module, "SteamAPI_ISteamUGC_GetItemInstallInfo"));

    UgcFn ugcAccessor = nullptr;
    for (const char* name : { "SteamAPI_SteamUGC_v016", "SteamAPI_SteamUGC_v017", "SteamAPI_SteamUGC_v018",
                              "SteamAPI_SteamUGC_v019", "SteamAPI_SteamUGC_v020", "SteamAPI_SteamUGC_v021" }) {
        ugcAccessor = reinterpret_cast<UgcFn>(GetProcAddress(module, name));
        if (ugcAccessor && subscribe && itemState) break;
    }
    if (!init || !ugcAccessor || !subscribe || !itemState) {
        ChildLog("steam_api64.dll is missing the UGC exports");
        WriteProgressAtomic(progressPath, "failed", "Steam UGC exports missing from steam_api64.dll.", 0, 0, 0, "");
        FreeLibrary(module);
        return 2;
    }

    const fs::path stopPath = StopFileForProgress(progressPath);
    WriteProgressAtomic(progressPath, "subscribing", "", 0, 0, 0, "");

    const std::vector<AppIdFile> appIdFiles = WriteAppIdFiles();
    ChildLog("calling SteamAPI_Init");
    const bool started = init();
    RemoveCreatedAppIdFiles(appIdFiles);
    if (!started) {
        ChildLog("SteamAPI_Init failed (is Steam running and logged in?)");
        WriteProgressAtomic(progressPath, "failed",
                            "Steam client is not running or signed in. Open Steam and press RETRY.",
                            0, 0, 0, "");
        FreeLibrary(module);
        return 3;
    }

    void* ugc = ugcAccessor();
    if (!ugc) {
        ChildLog("SteamUGC interface unavailable");
        WriteProgressAtomic(progressPath, "failed", "SteamUGC interface unavailable.", 0, 0, 0, "");
        if (shutdown) shutdown();
        FreeLibrary(module);
        return 4;
    }

    const unsigned long long fileId = std::strtoull(publishedFileId.c_str(), nullptr, 10);
    subscribe(ugc, fileId);
    if (downloadItem) downloadItem(ugc, fileId, true);

    const unsigned long long startMs = GetTickCount64();
    unsigned long long lastWriteMs = 0;
    unsigned long long lastPokeMs = startMs;
    unsigned long long maxDownloaded = 0;
    unsigned long long maxTotal = 0;
    unsigned int state = 0;
    std::string lastStage;

    const unsigned long long kMaxWaitMs = 6ULL * 60ULL * 60ULL * 1000ULL;
    while (GetTickCount64() - startMs < kMaxWaitMs) {
        std::error_code ec;
        if ((!stopPath.empty() && fs::exists(stopPath, ec)) || !IsProcessAlive(parentPid)) {
            ChildLog("stop requested or parent exited; stopping helper cleanly");
            if (shutdown) shutdown();
            FreeLibrary(module);
            return 0;
        }

        if (runCallbacks) runCallbacks();
        state = itemState(ugc, fileId);

        unsigned long long dl = 0, tot = 0;
        if (getDownloadInfo && getDownloadInfo(ugc, fileId, &dl, &tot)) {
            if (tot > maxTotal) maxTotal = tot;
            if (dl > maxDownloaded) maxDownloaded = dl;
            if (maxTotal > 0 && maxDownloaded > maxTotal) maxDownloaded = maxTotal;
        }

        const bool isInstalled = (state & 0x04u) != 0;
        const bool needsUpdate = (state & 0x08u) != 0;
        const bool isDownloading = (state & 0x10u) != 0;
        const bool isPending = (state & 0x20u) != 0;

        char installFolderBuf[1024] = {};
        unsigned long long sizeOnDisk = 0;
        unsigned int timeStamp = 0;
        const bool haveInstallInfo = getInstallInfo &&
            getInstallInfo(ugc, fileId, &sizeOnDisk, installFolderBuf, sizeof(installFolderBuf), &timeStamp);
        fs::path installedPath = FindInstalledFolder(publishedFileId);
        if (installedPath.empty() && haveInstallInfo && installFolderBuf[0] != '\0') {
            fs::path candidate(installFolderBuf);
            if (NonEmptyDir(candidate) && isInstalled && !needsUpdate && !isDownloading && !isPending) {
                installedPath = std::move(candidate);
            }
        }

        if (isInstalled && !needsUpdate && !isDownloading && !isPending && !installedPath.empty()) {
            const unsigned long long doneTotal = maxTotal > 0 ? maxTotal : (sizeOnDisk > 0 ? sizeOnDisk : maxDownloaded);
            WriteProgressAtomic(progressPath, "done", "", state, doneTotal, doneTotal, installedPath.string());
            ChildLog("install complete id=" + publishedFileId + " folder=" + installedPath.string());
            if (shutdown) shutdown();
            FreeLibrary(module);
            return 0;
        }

        const unsigned long long nowMs = GetTickCount64();
        if ((!isInstalled || needsUpdate) && !isDownloading && !isPending && (nowMs - lastPokeMs >= 4000)) {
            lastPokeMs = nowMs;
            subscribe(ugc, fileId);
            if (downloadItem) downloadItem(ugc, fileId, true);
        }

        if (nowMs - startMs > 25000 && (state & 0x35u) == 0 && maxTotal == 0) {
            ChildLog("subscribe/download timed out with state 0x" + std::to_string(state));
            WriteProgressAtomic(progressPath, "failed",
                                "Steam did not start downloading this item (it may be private, removed, or require an eligible account).",
                                state, maxDownloaded, maxTotal, "");
            if (shutdown) shutdown();
            FreeLibrary(module);
            return 4;
        }

        std::string stage = "subscribing";
        if (isDownloading) stage = "downloading";
        else if (isPending) stage = "pending";
        else if (needsUpdate) stage = "updating";
        else if (isInstalled) stage = "verifying";

        if (stage != lastStage || (nowMs - lastWriteMs >= 200)) {
            lastStage = stage;
            lastWriteMs = nowMs;
            WriteProgressAtomic(progressPath, stage, "", state, maxDownloaded, maxTotal, "");
        }

        // Fallback when no progress file was requested: keep legacy quick exit behavior.
        if (progressPath.empty() && (state & 0x35u) != 0 && (nowMs - startMs >= 1000)) {
            break;
        }

        Sleep(150);
    }

    ChildLog("subscribe monitor ended, state 0x" + std::to_string(state));
    if (shutdown) shutdown();
    FreeLibrary(module);
    return (state & 0x35u) != 0 ? 0 : 4;
}

} // namespace SteamWorkshop
} // namespace noble
