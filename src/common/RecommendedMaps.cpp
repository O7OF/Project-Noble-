#include "common/RecommendedMaps.hpp"

#include "common/Logger.hpp"
#include "common/ObfuscatedString.hpp"

#include <windows.h>
#include <winhttp.h>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <thread>

namespace noble {
namespace {

constexpr const wchar_t kUserAgent[] = L"ProjectNoble";
// /latest is asked first. The list is the fallback for a base-only latest release (v0.19.90 was one), so the pack
// shipped with an earlier release is still found. The list is ordered by the tag's commit date, not by publish date
// (on 2026-11-22 v0.20.28 came first and v0.20.35 third), so the list is searched by published_at.
const std::string kReleaseApi = NOBLE_OBF("https://api.github.com/repos/O7OF/Project-Noble-/releases?per_page=30");
const std::string kLatestReleaseApi = NOBLE_OBF("https://api.github.com/repos/O7OF/Project-Noble-/releases/latest");

std::wstring Widen(const std::string& s) {
    if (s.empty()) return L"";
    const int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    if (len <= 0) return L"";
    std::wstring w(static_cast<size_t>(len - 1), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, w.data(), len);
    return w;
}

void CloseHttp(HINTERNET& session, HINTERNET& connect, HINTERNET& request) {
    if (request) { WinHttpCloseHandle(request); request = nullptr; }
    if (connect) { WinHttpCloseHandle(connect); connect = nullptr; }
    if (session) { WinHttpCloseHandle(session); session = nullptr; }
}

// http(s)://host[:port]/path -> parts. The local test server uses plain http on 127.0.0.1.
bool SplitUrl(const std::string& url, std::wstring& host, std::wstring& path, INTERNET_PORT& port, bool& secure) {
    const bool https = url.rfind("https://", 0) == 0;
    const bool http = url.rfind("http://", 0) == 0;
    if (!https && !http) return false;
    secure = https;
    const size_t hostStart = https ? 8 : 7;
    const size_t slash = url.find('/', hostStart);
    if (slash == std::string::npos) return false;
    std::string hostPort = url.substr(hostStart, slash - hostStart);
    port = secure ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;
    const size_t colon = hostPort.rfind(':');
    if (colon != std::string::npos) {
        port = static_cast<INTERNET_PORT>(std::atoi(hostPort.c_str() + colon + 1));
        hostPort.resize(colon);
    }
    host = Widen(hostPort);
    path = Widen(url.substr(slash));
    return !host.empty() && !path.empty();
}

bool OpenHttp(const std::string& url, HINTERNET& session, HINTERNET& connect, HINTERNET& request,
              std::string& error) {
    std::wstring host, path;
    INTERNET_PORT port = 0;
    bool secure = true;
    if (!SplitUrl(url, host, path, port, secure)) {
        error = "unsupported URL";
        return false;
    }
    session = WinHttpOpen(kUserAgent, WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME,
                          WINHTTP_NO_PROXY_BYPASS, 0);
    if (!session) { error = "WinHttpOpen failed"; return false; }
    DWORD redirectPolicy = WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS;
    WinHttpSetOption(session, WINHTTP_OPTION_REDIRECT_POLICY, &redirectPolicy, sizeof(redirectPolicy));
    WinHttpSetTimeouts(session, 10000, 10000, 30000, 300000);
    connect = WinHttpConnect(session, host.c_str(), port, 0);
    if (!connect) { error = "WinHttpConnect failed"; CloseHttp(session, connect, request); return false; }
    request = WinHttpOpenRequest(connect, L"GET", path.c_str(), nullptr, WINHTTP_NO_REFERER,
                                 WINHTTP_DEFAULT_ACCEPT_TYPES, secure ? WINHTTP_FLAG_SECURE : 0);
    if (!request) { error = "WinHttpOpenRequest failed"; CloseHttp(session, connect, request); return false; }
    WinHttpSetOption(request, WINHTTP_OPTION_REDIRECT_POLICY, &redirectPolicy, sizeof(redirectPolicy));
    if (!WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) ||
        !WinHttpReceiveResponse(request, nullptr)) {
        error = "HTTP request failed (" + std::to_string(GetLastError()) + ")";
        CloseHttp(session, connect, request);
        return false;
    }
    return true;
}

bool HttpGetText(const std::string& url, std::string& body, std::string& error) {
    HINTERNET session = nullptr, connect = nullptr, request = nullptr;
    if (!OpenHttp(url, session, connect, request, error)) return false;
    DWORD available = 0;
    while (WinHttpQueryDataAvailable(request, &available) && available > 0) {
        std::string chunk(available, '\0');
        DWORD read = 0;
        if (!WinHttpReadData(request, chunk.data(), available, &read)) {
            error = "WinHttpReadData failed";
            CloseHttp(session, connect, request);
            return false;
        }
        if (read == 0) break;
        if (body.size() + read > 8u * 1024u * 1024u) {
            error = "response is too large";
            CloseHttp(session, connect, request);
            return false;
        }
        body.append(chunk.data(), read);
    }
    CloseHttp(session, connect, request);
    return true;
}

// Streams a URL into `file`. `onProgress` gets (received, total) after each chunk.
bool HttpGetToFile(const std::string& url, const std::filesystem::path& file,
                   const std::function<void(uint64_t, uint64_t)>& onProgress, std::string& error) {
    HINTERNET session = nullptr, connect = nullptr, request = nullptr;
    if (!OpenHttp(url, session, connect, request, error)) return false;
    uint64_t total = 0;
    {
        wchar_t lengthText[64] = {};
        DWORD size = sizeof(lengthText);
        if (WinHttpQueryHeaders(request, WINHTTP_QUERY_CONTENT_LENGTH, WINHTTP_HEADER_NAME_BY_INDEX, lengthText,
                                &size, WINHTTP_NO_HEADER_INDEX)) {
            total = _wcstoui64(lengthText, nullptr, 10);
        }
    }
    std::error_code ec;
    std::filesystem::create_directories(file.parent_path(), ec);
    std::ofstream out(file, std::ios::binary | std::ios::trunc);
    if (!out) {
        error = "cannot open " + file.string();
        CloseHttp(session, connect, request);
        return false;
    }
    uint64_t received = 0;
    DWORD available = 0;
    while (WinHttpQueryDataAvailable(request, &available) && available > 0) {
        std::string chunk(available, '\0');
        DWORD read = 0;
        if (!WinHttpReadData(request, chunk.data(), available, &read)) {
            error = "WinHttpReadData failed";
            CloseHttp(session, connect, request);
            return false;
        }
        if (read == 0) break;
        out.write(chunk.data(), static_cast<std::streamsize>(read));
        received += read;
        if (onProgress) onProgress(received, total);
    }
    out.close();
    CloseHttp(session, connect, request);
    if (!out) {
        error = "write failed";
        return false;
    }
    return true;
}

// Runs a command line, waits for it and returns the exit code (or 0xFFFFFFFF on failure to start).
DWORD RunAndWait(const std::wstring& commandLine, const std::filesystem::path* captureTo = nullptr) {
    SECURITY_ATTRIBUTES sa{ sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
    HANDLE outFile = INVALID_HANDLE_VALUE;
    STARTUPINFOW si{};
    si.cb = sizeof(si);
    if (captureTo) {
        outFile = CreateFileW(captureTo->wstring().c_str(), GENERIC_WRITE, FILE_SHARE_READ, &sa, CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
        if (outFile == INVALID_HANDLE_VALUE) return 0xFFFFFFFF;
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdOutput = outFile;
        si.hStdError = outFile;
        si.hStdInput = INVALID_HANDLE_VALUE;
    }
    std::wstring mutableCommand = commandLine;
    PROCESS_INFORMATION pi{};
    const BOOL ok = CreateProcessW(nullptr, mutableCommand.data(), nullptr, nullptr, captureTo != nullptr,
                                   CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
    if (!ok) {
        if (outFile != INVALID_HANDLE_VALUE) CloseHandle(outFile);
        return 0xFFFFFFFF;
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode = 0xFFFFFFFF;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    if (outFile != INVALID_HANDLE_VALUE) CloseHandle(outFile);
    return exitCode;
}

// SHA256 of a file via certutil (always present on Windows): its output holds the hex hash on its own line.
bool Sha256OfFile(const std::filesystem::path& file, std::string& hash, std::string& error) {
    const std::filesystem::path out = file.parent_path() / (file.filename().wstring() + L".sha256.txt");
    const std::wstring cmd = L"certutil.exe -hashfile \"" + file.wstring() + L"\" SHA256";
    const DWORD rc = RunAndWait(cmd, &out);
    std::string text;
    {
        // Close the file before deleting it: on Windows an open stream blocks the delete.
        std::ifstream in(out, std::ios::binary);
        if (in) text.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    }
    std::error_code ec;
    std::filesystem::remove(out, ec);
    if (rc != 0) {
        error = "certutil failed (" + std::to_string(rc) + ")";
        return false;
    }
    std::string cleaned;
    for (const char c : text) {
        if (c == '\n' || c == '\r') cleaned.push_back('\n');
        else if (!std::isspace(static_cast<unsigned char>(c))) cleaned.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    size_t lineStart = 0;
    while (lineStart <= cleaned.size()) {
        size_t lineEnd = cleaned.find('\n', lineStart);
        if (lineEnd == std::string::npos) lineEnd = cleaned.size();
        if (lineEnd - lineStart == 64) {
            bool hex = true;
            for (size_t i = lineStart; i < lineEnd; ++i) {
                if (!std::isxdigit(static_cast<unsigned char>(cleaned[i]))) { hex = false; break; }
            }
            if (hex) {
                hash = cleaned.substr(lineStart, 64);
                return true;
            }
        }
        lineStart = lineEnd + 1;
    }
    error = "could not parse certutil output";
    return false;
}

std::string Trim(std::string text) {
    while (!text.empty() && (text.back() == '\n' || text.back() == '\r' || text.back() == ' ')) text.pop_back();
    const size_t start = text.find_first_not_of(" \t\r\n");
    return start == std::string::npos ? std::string() : text.substr(start);
}

struct ReleaseAsset {
    std::string name;
    std::string url;
    std::string shaUrl;
    std::string tag;
    uint64_t size = 0;   // zip size on the release
};

// Picks the most recently published release that carries a `<assetPrefix>*.zip` asset (plus its optional `.sha256`
// sibling). Accepts a release list or a single release object (/latest).
bool FindReleaseAsset(const std::string& body, const std::string& assetPrefix, ReleaseAsset& found, std::string& error);

// What was installed last (written after a successful download): data\cache\recommended\<id>.installed, three lines:
// release tag, pack zip size, installed marker size. Missing for packs installed by hand or before 0.20.36.
struct InstalledRecord {
    std::string tag;
    uint64_t zipBytes = 0;
    uint64_t markerBytes = 0;
};

std::filesystem::path InstalledRecordPath(const std::filesystem::path& installDir, const std::string& id) {
    return installDir / "data" / "cache" / "recommended" / (id + ".installed");
}

bool ReadInstalledRecord(const std::filesystem::path& path, InstalledRecord& out) {
    std::ifstream in(path);
    if (!in) return false;
    std::string zip, marker;
    std::getline(in, out.tag);
    std::getline(in, zip);
    std::getline(in, marker);
    out.zipBytes = std::strtoull(zip.c_str(), nullptr, 10);
    out.markerBytes = std::strtoull(marker.c_str(), nullptr, 10);
    return !out.tag.empty() || out.zipBytes != 0;
}

void WriteInstalledRecord(const std::filesystem::path& path, const InstalledRecord& record) {
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    std::ofstream out(path, std::ios::trunc);
    if (out) out << record.tag << "\n" << record.zipBytes << "\n" << record.markerBytes << "\n";
}

bool FindReleaseAsset(const std::string& body, const std::string& assetPrefix, ReleaseAsset& found, std::string& error) {
    const nlohmann::json root = nlohmann::json::parse(body, nullptr, false);
    nlohmann::json releases = nlohmann::json::array();
    if (root.is_array()) {
        releases = root;
    } else if (root.is_object()) {
        releases.push_back(root);
    } else {
        error = "GitHub release JSON could not be parsed";
        return false;
    }
    // Several releases carry the pack; take the most recently published one (the list order is not publish order).
    bool any = false;
    std::string bestPublished;
    for (const auto& release : releases) {
        if (!release.is_object() || !release.contains("assets") || !release["assets"].is_array()) continue;
        if (release.value("draft", false)) continue;
        std::string name, url;
        uint64_t size = 0;
        for (const auto& asset : release["assets"]) {
            const std::string candidate = asset.value("name", "");
            if (candidate.rfind(assetPrefix, 0) == 0 && candidate.size() > 4 &&
                candidate.compare(candidate.size() - 4, 4, ".zip") == 0) {
                name = candidate;
                url = asset.value("browser_download_url", "");
                size = asset.value("size", static_cast<uint64_t>(0));
                break;
            }
        }
        if (name.empty() || url.empty()) continue;
        std::string sha;
        const std::string shaName = name + ".sha256";
        for (const auto& asset : release["assets"]) {
            if (asset.value("name", "") == shaName) {
                sha = asset.value("browser_download_url", "");
                break;
            }
        }
        const std::string published = release.value("published_at", "");   // ISO 8601: compares as text
        if (any && published <= bestPublished) continue;
        found = ReleaseAsset{name, url, sha, release.value("tag_name", ""), size};
        bestPublished = published;
        any = true;
    }
    if (any) return true;
    error = "no " + assetPrefix + "* asset in the latest releases";
    return false;
}

// The newest published pack: /releases/latest first, else the newest-published release in the list that has one.
// NOBLE_RECOMMENDED_API_URL (test hook) replaces both with one local JSON document.
bool ResolveNewestAsset(const std::string& assetPrefix, ReleaseAsset& found, std::string& error) {
    std::string body;
    if (const char* overrideApi = std::getenv("NOBLE_RECOMMENDED_API_URL"); overrideApi && *overrideApi) {
        if (!HttpGetText(overrideApi, body, error)) return false;
        return FindReleaseAsset(body, assetPrefix, found, error);
    }
    std::string latestError;
    if (HttpGetText(kLatestReleaseApi, body, latestError) && FindReleaseAsset(body, assetPrefix, found, latestError))
        return true;
    body.clear();
    if (!HttpGetText(kReleaseApi, body, error)) return false;
    return FindReleaseAsset(body, assetPrefix, found, error);
}

} // namespace

RecommendedMaps& RecommendedMaps::Get() {
    static RecommendedMaps instance;
    return instance;
}

void RecommendedMaps::Initialize(const std::filesystem::path& installDir) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::filesystem::path dir = installDir;
    // Test hook: NOBLE_RECOMMENDED_INSTALL_OVERRIDE redirects the install into a scratch folder, so a download test
    // never overwrites the real data\maps files.
    if (const char* overrideDir = std::getenv("NOBLE_RECOMMENDED_INSTALL_OVERRIDE")) {
        if (*overrideDir) dir = std::filesystem::u8path(overrideDir);
    }
    m_installDir = dir;
    if (!m_items.empty()) return;
    Item forge;
    forge.id = "forge_world";
    forge.title = "Noble Forge World";
    forge.description = "AI paths for MAP AI and PLACE AI, plus the Pelican, Spectre, Boathog and the targeting laser in "
                        "the Forge palette. Optional download.";
    forge.assetPrefix = "Noble-ForgeWorld-";   // deliberately not "Project-Noble-*": the updater must not see this
                                               // pack as a client update (see Updater.cpp asset selection).
    forge.installedMarker = std::filesystem::path("data") / "maps" / "forge_halo.map";
    forge.approxBytes = 682ull * 1024ull * 1024ull;   // shown until the release reports the real asset size
    // v0.20.35 pack (AI Forge World, SHA-256 EB9899D5...FDB9). The v0.20.34 map (796,790,784 bytes, no AI paths) now
    // shows UPDATE AVAILABLE instead of INSTALLED. Change this whenever a new pack map is published.
    forge.currentMarkerBytes = 838791168ull;
    m_items.push_back(forge);
    m_status.clear();
    m_updates.clear();
    for (const Item& item : m_items) {
        m_status.emplace_back(item.id, Status{});
        m_updates.emplace_back(item.id, UpdateInfo{});
    }
    NOBLE_INFO("RecommendedMaps: install folder {}", m_installDir.string());
}

bool RecommendedMaps::IsInstalled(const Item& item) const {
    std::error_code ec;
    if (m_installDir.empty()) return false;
    if (!std::filesystem::is_regular_file(m_installDir / item.installedMarker, ec)) return false;
    return !IsOutdated(item);
}

bool RecommendedMaps::IsOutdated(const Item& item) const {
    std::error_code ec;
    if (m_installDir.empty()) return false;
    const std::filesystem::path marker = m_installDir / item.installedMarker;
    if (!std::filesystem::is_regular_file(marker, ec)) return false;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& [key, info] : m_updates)
            if (key == item.id && info.checked && info.available) return true;   // CHECK FOR UPDATES found a newer pack
    }
    if (item.currentMarkerBytes == 0) return false;
    const uint64_t size = std::filesystem::file_size(marker, ec);
    return !ec && size != item.currentMarkerBytes;
}

RecommendedMaps::UpdateInfo RecommendedMaps::UpdateSnapshot(const std::string& id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& [key, info] : m_updates)
        if (key == id) return info;
    return {};
}

void RecommendedMaps::CheckForUpdates(const std::string& id) {
    Item item;
    std::filesystem::path installDir;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_installDir.empty()) return;
        installDir = m_installDir;
        for (const Item& candidate : m_items)
            if (candidate.id == id) item = candidate;
        if (item.id.empty()) return;
        for (auto& [key, info] : m_updates) {
            if (key != id) continue;
            if (info.checking) return;   // already running
            info.checking = true;
            info.error.clear();
        }
    }
    std::thread([this, item, installDir] {
        UpdateInfo result;
        result.checked = true;
        std::string error;
        ReleaseAsset asset;
        if (!ResolveNewestAsset(item.assetPrefix, asset, error)) {
            result.error = error.empty() ? "update check failed" : error;
            NOBLE_WARN("RecommendedMaps: update check for {} failed: {}", item.id, result.error);
        } else {
            result.latestTag = asset.tag;
            result.latestBytes = asset.size;
            std::error_code ec;
            const std::filesystem::path marker = installDir / item.installedMarker;
            const bool present = std::filesystem::is_regular_file(marker, ec);
            InstalledRecord record;
            if (!present) {
                result.available = false;   // nothing installed: the DOWNLOAD button covers it
            } else if (ReadInstalledRecord(InstalledRecordPath(installDir, item.id), record)) {
                result.installedTag = record.tag;
                // A different pack zip on the release than the one installed = an update. (Every release re-attaches
                // the pack; an unchanged map gives the same zip size, so an unchanged pack is not offered again.)
                result.available = asset.size != 0 && record.zipBytes != 0 && asset.size != record.zipBytes;
            } else {
                // Installed by hand or before install records: trust the marker size this client knows.
                const uint64_t size = std::filesystem::file_size(marker, ec);
                result.available = item.currentMarkerBytes != 0 && !ec && size != item.currentMarkerBytes;
                if (!result.available && !ec) {   // current: remember it so later checks compare zips
                    WriteInstalledRecord(InstalledRecordPath(installDir, item.id), InstalledRecord{asset.tag, asset.size, size});
                    result.installedTag = asset.tag;
                }
            }
            NOBLE_INFO("RecommendedMaps: update check {}: latest {} ({} bytes), installed '{}', update {}", item.id,
                       asset.tag, asset.size, result.installedTag, result.available ? "available" : "not needed");
        }
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& [key, info] : m_updates)
            if (key == item.id) info = result;
    }).detach();
}

RecommendedMaps::Status RecommendedMaps::Snapshot(const std::string& id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& [key, status] : m_status) {
        if (key == id) return status;
    }
    return {};
}

void RecommendedMaps::SetStatus(const std::string& id, const Status& status) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& [key, slot] : m_status) {
        if (key == id) { slot = status; return; }
    }
    m_status.emplace_back(id, status);
}

void RecommendedMaps::StartDownload(const std::string& id) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& [key, status] : m_status) {
            if (key != id) continue;
            if (status.state == State::Resolving || status.state == State::Downloading ||
                status.state == State::Verifying || status.state == State::Installing) {
                return;   // already running
            }
        }
        const bool known = std::any_of(m_items.begin(), m_items.end(), [&](const Item& item) { return item.id == id; });
        if (!known || m_installDir.empty()) return;
    }
    std::thread([this, id] { Worker(id); }).detach();
}

std::string RecommendedMaps::FormatBytes(uint64_t bytes) {
    char text[64];
    if (bytes >= 1024ull * 1024ull * 1024ull) std::snprintf(text, sizeof(text), "%.1f GB", bytes / (1024.0 * 1024 * 1024));
    else if (bytes >= 1024ull * 1024ull) std::snprintf(text, sizeof(text), "%.0f MB", bytes / (1024.0 * 1024));
    else if (bytes >= 1024ull) std::snprintf(text, sizeof(text), "%.0f KB", bytes / 1024.0);
    else std::snprintf(text, sizeof(text), "%llu B", static_cast<unsigned long long>(bytes));
    return text;
}

void RecommendedMaps::Worker(const std::string& id) {
    Item item;
    std::filesystem::path installDir;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        installDir = m_installDir;
        for (const Item& candidate : m_items) {
            if (candidate.id == id) item = candidate;
        }
    }
    if (item.id.empty() || installDir.empty()) return;

    Status status;
    status.state = State::Resolving;
    SetStatus(id, status);

    std::string assetUrl, assetName, shaUrl, error, assetTag;
    uint64_t assetBytes = 0;
    // Test hook: a local HTTP server can stand in for the GitHub release.
    if (const char* overrideUrl = std::getenv("NOBLE_RECOMMENDED_ASSET_URL"); overrideUrl && *overrideUrl) {
        assetUrl = overrideUrl;
        const size_t slash = assetUrl.find_last_of('/');
        assetName = slash == std::string::npos ? "" : assetUrl.substr(slash + 1);
        if (const char* overrideSha = std::getenv("NOBLE_RECOMMENDED_SHA_URL"); overrideSha && *overrideSha) {
            shaUrl = overrideSha;
        } else {
            shaUrl = assetUrl + ".sha256";
        }
    } else {
        // Test hook: point the release lookup at a local JSON document so the fallback can be exercised without a real
        // GitHub release. NOBLE_RECOMMENDED_ASSET_URL (above) skips the lookup entirely.
        ReleaseAsset asset;
        if (!ResolveNewestAsset(item.assetPrefix, asset, error)) {
            status.state = State::Error;
            status.error = error;
            SetStatus(id, status);
            NOBLE_WARN("RecommendedMaps: {}", status.error);
            return;
        }
        assetName = asset.name;
        assetUrl = asset.url;
        shaUrl = asset.shaUrl;
        assetTag = asset.tag;
        assetBytes = asset.size;
        NOBLE_INFO("RecommendedMaps: {} from release {}", assetName, asset.tag);
    }

    status.assetName = assetName;
    status.state = State::Downloading;
    SetStatus(id, status);
    NOBLE_INFO("RecommendedMaps: downloading {}", assetUrl);

    const std::filesystem::path zip = installDir / "data" / "cache" / "recommended" / assetName;
    const bool downloaded = HttpGetToFile(assetUrl, zip,
        [&](uint64_t received, uint64_t total) {
            Status update = status;
            update.received = received;
            update.total = total;
            update.progress = total ? static_cast<float>(static_cast<double>(received) / static_cast<double>(total)) : 0.0f;
            SetStatus(id, update);
            status = update;
        }, error);
    if (!downloaded) {
        status.state = State::Error;
        status.error = error;
        SetStatus(id, status);
        NOBLE_WARN("RecommendedMaps: download failed: {}", error);
        return;
    }

    status.state = State::Verifying;
    SetStatus(id, status);
    if (!shaUrl.empty()) {
        std::string expected, shaBody;
        if (!HttpGetText(shaUrl, shaBody, error)) {
            status.state = State::Error;
            status.error = error;
            SetStatus(id, status);
            NOBLE_WARN("RecommendedMaps: checksum download failed: {}", error);
            return;
        }
        expected = Trim(shaBody).substr(0, 64);
        std::string actual, hashError;
        if (!Sha256OfFile(zip, actual, hashError)) {
            status.state = State::Error;
            status.error = hashError;
            SetStatus(id, status);
            NOBLE_WARN("RecommendedMaps: checksum failed: {}", hashError);
            return;
        }
        if (!expected.empty() && _stricmp(expected.c_str(), actual.c_str()) != 0) {
            status.state = State::Error;
            status.error = "checksum mismatch";
            SetStatus(id, status);
            NOBLE_WARN("RecommendedMaps: checksum mismatch (expected {}, got {})", expected, actual);
            return;
        }
        NOBLE_INFO("RecommendedMaps: checksum ok ({})", actual);
    }

    status.state = State::Installing;
    SetStatus(id, status);
    const std::wstring extract = L"tar.exe -xf \"" + zip.wstring() + L"\" -C \"" + installDir.wstring() + L"\"";
    const DWORD rc = RunAndWait(extract);
    std::error_code ec;
    std::filesystem::remove(zip, ec);
    if (rc != 0) {
        status.state = State::Error;
        status.error = "extract failed (" + std::to_string(rc) + ")";
        SetStatus(id, status);
        NOBLE_WARN("RecommendedMaps: {}", status.error);
        return;
    }

    {   // record what was installed, for CHECK FOR UPDATES
        std::error_code sizeEc;
        const uint64_t markerBytes = std::filesystem::file_size(installDir / item.installedMarker, sizeEc);
        WriteInstalledRecord(InstalledRecordPath(installDir, id), InstalledRecord{assetTag, assetBytes, sizeEc ? 0 : markerBytes});
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& [key, info] : m_updates) {
            if (key != id) continue;
            info.available = false;
            info.installedTag = assetTag;
        }
    }
    status.state = State::Installed;
    status.progress = 1.0f;
    SetStatus(id, status);
    NOBLE_INFO("RecommendedMaps: installed {} into {}", assetName, installDir.string());
}

} // namespace noble
