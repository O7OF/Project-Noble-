#include "ModDownloadPrompt.hpp"
#include "NobleStyle.hpp"
#include "NobleUi.hpp"
#include "UiArt.hpp"
#include "UiPad.hpp"
#include "common/Logger.hpp"
#include "vfs/VFS.hpp"

#include <imgui.h>
#include <nlohmann/json.hpp>
#include <winhttp.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

#pragma comment(lib, "winhttp.lib")

namespace noble {

// Filled on a detached worker thread; the render thread only reads a completed snapshot.
struct WorkshopDetails {
    std::mutex mutex;
    bool ready = false;
    bool ok = false;
    unsigned long long size = 0;
    std::string sizeText;   // Steam's own "20.644 GB" from the Workshop page when the API file_size is 0
    std::string title;
    std::string description;
    std::string creatorId;
    std::string authorName;
    std::string updatedText;
    std::string fileUrl;
    unsigned long long subscriptions = 0;
    unsigned long long favorited = 0;
    unsigned long long views = 0;
    int childCount = 0;
    std::vector<std::string> tags;
    std::string previewPath;
    std::string error;
};

namespace {

constexpr const char* kPopupId = "WORKSHOP MOD REQUIRED";
constexpr const char* kMissingMessage =
    "The lobby you are trying to join currently has a mod installed from the Workshop that you do not have.";

std::wstring Widen(const std::string& text) {
    if (text.empty()) return L"";
    const int n = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), nullptr, 0);
    std::wstring out(static_cast<size_t>(n), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), out.data(), n);
    return out;
}

struct HttpResult {
    bool ok = false;
    int status = 0;
    std::string body;
    std::string error;
};

// Blocking WinHTTP POST, only ever called on the details worker thread.
HttpResult HttpPostForm(const std::string& fullUrl, const std::string& body) {
    HttpResult result;
    const std::wstring wide = Widen(fullUrl);
    URL_COMPONENTS parts{};
    parts.dwStructSize = sizeof(parts);
    parts.dwSchemeLength = static_cast<DWORD>(-1);
    parts.dwHostNameLength = static_cast<DWORD>(-1);
    parts.dwUrlPathLength = static_cast<DWORD>(-1);
    parts.dwExtraInfoLength = static_cast<DWORD>(-1);
    if (!WinHttpCrackUrl(wide.c_str(), static_cast<DWORD>(wide.size()), 0, &parts)) {
        result.error = "invalid Steam Web API URL";
        return result;
    }
    const std::wstring host(parts.lpszHostName, parts.dwHostNameLength);
    std::wstring path = parts.dwUrlPathLength ? std::wstring(parts.lpszUrlPath, parts.dwUrlPathLength) : L"/";
    if (parts.dwExtraInfoLength) path += std::wstring(parts.lpszExtraInfo, parts.dwExtraInfoLength);
    const bool https = parts.nScheme == INTERNET_SCHEME_HTTPS;
    const INTERNET_PORT port = parts.nPort ? parts.nPort : (https ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT);

    HINTERNET session = WinHttpOpen(L"ProjectNoble/0.1", WINHTTP_ACCESS_TYPE_NO_PROXY, nullptr, nullptr, 0);
    HINTERNET connect = session ? WinHttpConnect(session, host.c_str(), port, 0) : nullptr;
    HINTERNET request = connect ? WinHttpOpenRequest(connect, L"POST", path.c_str(), nullptr, WINHTTP_NO_REFERER,
                                                     WINHTTP_DEFAULT_ACCEPT_TYPES, https ? WINHTTP_FLAG_SECURE : 0) : nullptr;
    if (!request) {
        result.error = "could not open the Steam Web API request";
    } else {
        WinHttpSetTimeouts(session, 5000, 5000, 5000, 5000);
        const std::wstring headers = L"Content-Type: application/x-www-form-urlencoded\r\nAccept: application/json\r\n";
        if (WinHttpSendRequest(request, headers.c_str(), static_cast<DWORD>(headers.size()),
                               const_cast<char*>(body.data()), static_cast<DWORD>(body.size()),
                               static_cast<DWORD>(body.size()), 0) &&
            WinHttpReceiveResponse(request, nullptr)) {
            DWORD status = 0, statusSize = sizeof(status);
            WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                                WINHTTP_HEADER_NAME_BY_INDEX, &status, &statusSize, WINHTTP_NO_HEADER_INDEX);
            result.status = static_cast<int>(status);
            char buffer[2048];
            DWORD read = 0;
            while (WinHttpReadData(request, buffer, sizeof(buffer), &read) && read > 0) {
                result.body.append(buffer, read);
                if (result.body.size() > 512 * 1024) break;
            }
            result.ok = status >= 200 && status < 300;
            if (!result.ok) result.error = "Steam Web API replied " + std::to_string(status);
        } else {
            result.error = "Steam Web API connection failed";
        }
    }
    if (request) WinHttpCloseHandle(request);
    if (connect) WinHttpCloseHandle(connect);
    if (session) WinHttpCloseHandle(session);
    return result;
}

bool HttpDownload(const std::string& fullUrl, const std::filesystem::path& destination, std::string& error) {
    const std::wstring wide = Widen(fullUrl);
    URL_COMPONENTS parts{};
    parts.dwStructSize = sizeof(parts);
    parts.dwSchemeLength = static_cast<DWORD>(-1);
    parts.dwHostNameLength = static_cast<DWORD>(-1);
    parts.dwUrlPathLength = static_cast<DWORD>(-1);
    parts.dwExtraInfoLength = static_cast<DWORD>(-1);
    if (!WinHttpCrackUrl(wide.c_str(), static_cast<DWORD>(wide.size()), 0, &parts)) {
        error = "invalid Workshop preview URL";
        return false;
    }
    const std::wstring host(parts.lpszHostName, parts.dwHostNameLength);
    std::wstring path = parts.dwUrlPathLength ? std::wstring(parts.lpszUrlPath, parts.dwUrlPathLength) : L"/";
    if (parts.dwExtraInfoLength) path += std::wstring(parts.lpszExtraInfo, parts.dwExtraInfoLength);
    const bool https = parts.nScheme == INTERNET_SCHEME_HTTPS;
    const INTERNET_PORT port = parts.nPort ? parts.nPort : (https ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT);

    HINTERNET session = WinHttpOpen(L"ProjectNoble/0.1", WINHTTP_ACCESS_TYPE_NO_PROXY, nullptr, nullptr, 0);
    HINTERNET connect = session ? WinHttpConnect(session, host.c_str(), port, 0) : nullptr;
    HINTERNET request = connect ? WinHttpOpenRequest(connect, L"GET", path.c_str(), nullptr, WINHTTP_NO_REFERER,
                                                     WINHTTP_DEFAULT_ACCEPT_TYPES, https ? WINHTTP_FLAG_SECURE : 0) : nullptr;
    bool ok = false;
    if (!request) {
        error = "could not open the Workshop preview request";
    } else {
        WinHttpSetTimeouts(session, 5000, 5000, 5000, 5000);
        if (WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, nullptr, 0, 0, 0) &&
            WinHttpReceiveResponse(request, nullptr)) {
            DWORD status = 0, statusSize = sizeof(status);
            WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                                WINHTTP_HEADER_NAME_BY_INDEX, &status, &statusSize, WINHTTP_NO_HEADER_INDEX);
            if (status >= 200 && status < 300) {
                std::filesystem::path partial = destination;
                partial += L".part";
                std::ofstream out(partial, std::ios::binary | std::ios::trunc);
                char buffer[8192];
                DWORD read = 0;
                bool wrote = out.is_open();
                while (wrote && WinHttpReadData(request, buffer, sizeof(buffer), &read) && read > 0) {
                    out.write(buffer, static_cast<std::streamsize>(read));
                    if (!out) wrote = false;
                }
                out.close();
                std::error_code ec;
                if (wrote && out.good()) {
                    std::filesystem::rename(partial, destination, ec);
                    if (ec) {
                        std::filesystem::remove(destination, ec);
                        ec.clear();
                        std::filesystem::rename(partial, destination, ec);
                    }
                    ok = !ec;
                    if (!ok) error = "could not store the Workshop preview";
                } else {
                    error = "could not write the Workshop preview";
                }
                if (!ok) std::filesystem::remove(partial, ec);
            } else {
                error = "Workshop preview replied " + std::to_string(status);
            }
        } else {
            error = "Workshop preview download failed";
        }
    }
    if (request) WinHttpCloseHandle(request);
    if (connect) WinHttpCloseHandle(connect);
    if (session) WinHttpCloseHandle(session);
    return ok;
}

std::string FormatBytes(unsigned long long bytes) {
    char text[64];
    if (bytes >= 1024ull * 1024ull * 1024ull) std::snprintf(text, sizeof(text), "%.2f GB", bytes / (1024.0 * 1024.0 * 1024.0));
    else if (bytes >= 1024ull * 1024ull) std::snprintf(text, sizeof(text), "%.1f MB", bytes / (1024.0 * 1024.0));
    else if (bytes >= 1024ull) std::snprintf(text, sizeof(text), "%.0f KB", bytes / 1024.0);
    else std::snprintf(text, sizeof(text), "%llu bytes", bytes);
    return text;
}

std::string PreviewExtension(const std::string& url) {
    const size_t dot = url.find_last_of('.');
    if (dot != std::string::npos && dot + 1 < url.size()) {
        std::string ext = url.substr(dot + 1);
        if (ext.size() <= 5) {
            for (char& c : ext) c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
            if (ext == "jpg" || ext == "jpeg" || ext == "png" || ext == "webp" || ext == "gif" || ext == "bmp") {
                return "." + ext;
            }
        }
    }
    return ".jpg";
}

std::string FormatCount(unsigned long long n) {
    std::string raw = std::to_string(n);
    std::string out;
    int group = 0;
    for (int i = static_cast<int>(raw.size()) - 1; i >= 0; --i) {
        if (group == 3) {
            out.push_back(',');
            group = 0;
        }
        out.push_back(raw[static_cast<size_t>(i)]);
        ++group;
    }
    std::reverse(out.begin(), out.end());
    return out;
}

std::string FormatUnixDate(long long ts) {
    if (ts <= 0) return {};
    std::time_t t = static_cast<std::time_t>(ts);
    std::tm tmVal{};
    if (localtime_s(&tmVal, &t) != 0) return {};
    char buf[48] = {};
    if (std::strftime(buf, sizeof(buf), "%b %d, %Y", &tmVal) == 0) return {};
    return buf;
}

std::string FormatSpeed(double bps) {
    char buf[64];
    if (bps >= 1024.0 * 1024.0 * 1024.0) std::snprintf(buf, sizeof(buf), "%.2f GB/s", bps / (1024.0 * 1024.0 * 1024.0));
    else if (bps >= 1024.0 * 1024.0) std::snprintf(buf, sizeof(buf), "%.1f MB/s", bps / (1024.0 * 1024.0));
    else if (bps >= 1024.0) std::snprintf(buf, sizeof(buf), "%.0f KB/s", bps / 1024.0);
    else if (bps > 0.0) std::snprintf(buf, sizeof(buf), "%.0f B/s", bps);
    else return "-- MB/s";
    return buf;
}

std::string FormatEta(double seconds) {
    if (seconds < 0.0 || seconds > 86400.0 * 7.0) return "";
    unsigned long long s = static_cast<unsigned long long>(seconds + 0.5);
    unsigned long long h = s / 3600;
    unsigned long long m = (s % 3600) / 60;
    unsigned long long rem = s % 60;
    char buf[64];
    if (h > 0) std::snprintf(buf, sizeof(buf), "ETA %lluh %llum", h, m);
    else if (m > 0) std::snprintf(buf, sizeof(buf), "ETA %llum %02llus", m, rem);
    else std::snprintf(buf, sizeof(buf), "ETA %llus", rem);
    return buf;
}

unsigned long long ParseSizeTextBytes(const std::string& text) {
    if (text.empty()) return 0;
    std::string cleaned;
    for (char c : text) {
        if (c != ',') cleaned.push_back(c);
    }
    char* endPtr = nullptr;
    const double val = std::strtod(cleaned.c_str(), &endPtr);
    if (val <= 0.0 || endPtr == cleaned.c_str()) return 0;
    std::string unit = endPtr;
    for (char& c : unit) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (unit.find("gb") != std::string::npos) return static_cast<unsigned long long>(val * 1024.0 * 1024.0 * 1024.0);
    if (unit.find("mb") != std::string::npos) return static_cast<unsigned long long>(val * 1024.0 * 1024.0);
    if (unit.find("kb") != std::string::npos) return static_cast<unsigned long long>(val * 1024.0);
    return static_cast<unsigned long long>(val);
}

std::string CleanWorkshopDescription(const std::string& raw) {
    std::string out;
    out.reserve(raw.size());
    size_t i = 0;
    while (i < raw.size()) {
        if (raw[i] == '\r') {
            ++i;
            continue;
        }
        if (raw[i] == '[') {
            const size_t close = raw.find(']', i + 1);
            if (close != std::string::npos && close - i < 128) {
                std::string tag = raw.substr(i + 1, close - i - 1);
                std::string lower = tag;
                for (char& c : lower) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                if (lower == "img") {
                    const size_t endImg = raw.find("[/img]", close + 1);
                    if (endImg != std::string::npos) {
                        i = endImg + 6;
                        continue;
                    }
                }
                if (lower == "*") {
                    if (!out.empty() && out.back() != '\n') out.push_back('\n');
                    out += "  * ";
                } else if (lower == "/h1" || lower == "/h2" || lower == "/h3" || lower == "hr" || lower == "/list") {
                    if (!out.empty() && out.back() != '\n') out.push_back('\n');
                }
                i = close + 1;
                continue;
            }
        }
        out.push_back(raw[i]);
        ++i;
    }
    // Collapse runs of >2 newlines cleanly
    std::string trimmed;
    trimmed.reserve(out.size());
    int nlRun = 0;
    for (char c : out) {
        if (c == '\n') {
            if (++nlRun <= 2) trimmed.push_back(c);
        } else {
            nlRun = 0;
            trimmed.push_back(c);
        }
    }
    while (!trimmed.empty() && (trimmed.front() == ' ' || trimmed.front() == '\n')) trimmed.erase(trimmed.begin());
    while (!trimmed.empty() && (trimmed.back() == ' ' || trimmed.back() == '\n')) trimmed.pop_back();
    return trimmed;
}

std::string FetchSteamPersonaName(const std::string& steamId64) {
    if (steamId64.empty()) return {};
    const std::wstring host = L"steamcommunity.com";
    const std::wstring path = Widen("/profiles/" + steamId64 + "/?xml=1");
    HINTERNET session = WinHttpOpen(L"ProjectNoble/0.1", WINHTTP_ACCESS_TYPE_NO_PROXY, nullptr, nullptr, 0);
    if (!session) return {};
    WinHttpSetTimeouts(session, 2500, 2500, 2500, 2500);
    HINTERNET connect = WinHttpConnect(session, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    HINTERNET request = connect ? WinHttpOpenRequest(connect, L"GET", path.c_str(), nullptr, WINHTTP_NO_REFERER,
                                                     WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE) : nullptr;
    std::string xml;
    if (request && WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, nullptr, 0, 0, 0) &&
        WinHttpReceiveResponse(request, nullptr)) {
        char buf[2048];
        DWORD read = 0;
        while (WinHttpReadData(request, buf, sizeof(buf), &read) && read > 0) {
            xml.append(buf, read);
            if (xml.size() > 65536 || xml.find("</steamID>") != std::string::npos) break;
        }
    }
    if (request) WinHttpCloseHandle(request);
    if (connect) WinHttpCloseHandle(connect);
    if (session) WinHttpCloseHandle(session);

    const size_t open = xml.find("<steamID>");
    const size_t close = (open != std::string::npos) ? xml.find("</steamID>", open) : std::string::npos;
    if (open == std::string::npos || close == std::string::npos) return {};
    std::string inner = xml.substr(open + 9, close - (open + 9));
    const std::string cdataOpen = "<![CDATA[";
    const std::string cdataClose = "]]>";
    if (inner.rfind(cdataOpen, 0) == 0 && inner.size() >= cdataOpen.size() + cdataClose.size()) {
        inner = inner.substr(cdataOpen.size(), inner.size() - cdataOpen.size() - cdataClose.size());
    }
    return inner;
}

unsigned long long JsonU64(const nlohmann::json& obj, const char* key) {
    if (!obj.contains(key)) return 0;
    const auto& value = obj.at(key);
    if (value.is_string()) return std::strtoull(value.get<std::string>().c_str(), nullptr, 10);
    if (value.is_number_unsigned()) return value.get<unsigned long long>();
    if (value.is_number_integer() && value.get<long long>() > 0) {
        return static_cast<unsigned long long>(value.get<long long>());
    }
    return 0;
}

} // namespace

ModDownloadPrompt& ModDownloadPrompt::Get() {
    static ModDownloadPrompt prompt;
    return prompt;
}

bool ModDownloadPrompt::IsInstalled(const std::string& publishedFileId, std::filesystem::path* folder) {
    const std::filesystem::path found = SteamWorkshop::FindInstalledFolder(publishedFileId);
    if (folder) *folder = found;
    return !found.empty();
}

void ModDownloadPrompt::Show(const std::string& publishedFileId, const std::string& title,
                             std::function<void(const std::filesystem::path&)> onReady, bool demo) {
    m_download.Cancel();
    m_demo = demo;
    m_modId = publishedFileId;
    m_modTitle = title;
    m_message.clear();
    m_installedFolder.clear();
    m_onReady = std::move(onReady);
    m_state = State::Prompt;
    m_popupOpened = false;
    StartDetailsFetch();
    NOBLE_INFO("ModDownloadPrompt: lobby requires Workshop item {} ('{}')", publishedFileId, title);
}

void ModDownloadPrompt::StartDetailsFetch() {
    auto details = std::make_shared<WorkshopDetails>();
    m_details = details;
    if (m_demo) {
        std::lock_guard<std::mutex> lock(details->mutex);
        details->ready = true;
        details->error = "demo";
        return;
    }
    const std::string id = m_modId;
    const std::filesystem::path cacheDir = VFS::GetUserStoragePath() / "ui" / "cache" / "workshop";
    std::thread([details, id, cacheDir] {
        std::error_code ec;
        std::filesystem::create_directories(cacheDir, ec);
        const std::string body = "itemcount=1&publishedfileids[0]=" + id;
        const HttpResult response = HttpPostForm(
            "https://api.steampowered.com/ISteamRemoteStorage/GetPublishedFileDetails/v1/", body);
        if (!response.ok) {
            std::lock_guard<std::mutex> lock(details->mutex);
            details->error = response.error;
            details->ready = true;
            return;
        }
        try {
            const nlohmann::json json = nlohmann::json::parse(response.body);
            const auto& list = json.at("response").at("publishedfiledetails");
            if (!list.is_array() || list.empty()) throw std::runtime_error("empty publishedfiledetails");
            const auto& item = list.at(0);
            const int result = item.value("result", 1);
            if (result != 1) throw std::runtime_error("Steam could not find this Workshop item");
            unsigned long long size = JsonU64(item, "file_size");
            const std::string title = item.value("title", std::string());
            const std::string description = CleanWorkshopDescription(item.value("file_description", std::string()));
            const std::string creatorId = item.value("creator", std::string());
            const std::string fileUrl = item.value("file_url", std::string());
            const long long timeUpdated = static_cast<long long>(JsonU64(item, "time_updated"));
            const long long timeCreated = static_cast<long long>(JsonU64(item, "time_created"));
            const std::string updatedText = FormatUnixDate(timeUpdated > 0 ? timeUpdated : timeCreated);
            const unsigned long long subscriptions = JsonU64(item, "subscriptions");
            const unsigned long long favorited = JsonU64(item, "favorited");
            const unsigned long long views = JsonU64(item, "views");
            int childCount = 0;
            if (item.contains("children") && item["children"].is_array()) {
                childCount = static_cast<int>(item["children"].size());
            }
            std::vector<std::string> tags;
            if (item.contains("tags") && item["tags"].is_array()) {
                for (const auto& t : item["tags"]) {
                    if (t.is_object() && t.contains("tag") && t["tag"].is_string()) {
                        std::string ts = t["tag"].get<std::string>();
                        if (!ts.empty()) tags.push_back(std::move(ts));
                    }
                    if (tags.size() >= 8) break;
                }
            }

            std::string preview;
            if (item.contains("preview_url") && item.at("preview_url").is_string()) {
                preview = item.at("preview_url").get<std::string>();
            }
            std::filesystem::path previewPath;
            if (!preview.empty()) {
                const std::filesystem::path candidate = cacheDir / (id + PreviewExtension(preview));
                std::string downloadError;
                if (HttpDownload(preview, candidate, downloadError)) previewPath = candidate;
                else NOBLE_WARN("ModDownloadPrompt: Workshop preview download failed: {}", downloadError);
            }
            // Steam reports file_size 0 for items stored with hcontent_file (large packs), so read
            // the size line off the Workshop page when the API has no usable number.
            std::string sizeText;
            if (size == 0) {
                const std::filesystem::path pagePath = cacheDir / (id + ".html");
                std::string pageError;
                if (HttpDownload("https://steamcommunity.com/sharedfiles/filedetails/?id=" + id, pagePath, pageError)) {
                    std::ifstream page(pagePath, std::ios::binary);
                    std::string html((std::istreambuf_iterator<char>(page)), std::istreambuf_iterator<char>());
                    page.close();
                    std::error_code removeEc;
                    std::filesystem::remove(pagePath, removeEc);
                    const size_t label = html.find("File Size");
                    if (label != std::string::npos) {
                        const size_t value = html.find("detailsStatRight\">", label);
                        if (value != std::string::npos) {
                            const size_t start = value + sizeof("detailsStatRight\">") - 1;
                            const size_t end = html.find('<', start);
                            if (end != std::string::npos && end > start && end - start < 32) {
                                sizeText = html.substr(start, end - start);
                                while (!sizeText.empty() && (sizeText.back() == ' ' || sizeText.back() == '\r' || sizeText.back() == '\n')) {
                                    sizeText.pop_back();
                                }
                            }
                        }
                    }
                } else {
                    NOBLE_WARN("ModDownloadPrompt: Workshop page size lookup failed: {}", pageError);
                }
            }
            if (size == 0 && !sizeText.empty()) {
                size = ParseSizeTextBytes(sizeText);
            }

            std::string authorName = FetchSteamPersonaName(creatorId);

            std::lock_guard<std::mutex> lock(details->mutex);
            details->size = size;
            details->sizeText = sizeText;
            details->title = title;
            details->description = description;
            details->creatorId = creatorId;
            details->authorName = authorName;
            details->updatedText = updatedText;
            details->fileUrl = fileUrl;
            details->subscriptions = subscriptions;
            details->favorited = favorited;
            details->views = views;
            details->childCount = childCount;
            details->tags = std::move(tags);
            details->previewPath = previewPath.string();
            details->ok = true;
            details->ready = true;
        } catch (const std::exception& e) {
            std::lock_guard<std::mutex> lock(details->mutex);
            details->error = e.what();
            details->ready = true;
        }
    }).detach();
}

void ModDownloadPrompt::Cancel() {
    m_download.Cancel();
    m_state = State::Hidden;
    m_popupOpened = false;
    m_onReady = nullptr;
}

void ModDownloadPrompt::BeginDownload() {
    if (m_demo) return;
    m_message.clear();
    std::string directUrl;
    unsigned long long knownBytes = 0;
    if (m_details) {
        std::lock_guard<std::mutex> lock(m_details->mutex);
        directUrl = m_details->fileUrl;
        knownBytes = m_details->size;
    }
    m_download.SetHints(directUrl, knownBytes);
    if (m_download.Start(m_modId, m_modTitle, m_message)) {
        m_state = State::Downloading;
        NOBLE_INFO("ModDownloadPrompt: download of {} started: {}", m_modId, m_message);
    } else {
        m_state = State::Failed;
        NOBLE_WARN("ModDownloadPrompt: download of {} could not start: {}", m_modId, m_message);
    }
}

void ModDownloadPrompt::Render() {
    if (m_state == State::Hidden) return;

    if (m_state == State::Downloading) {
        m_download.Tick();
        if (m_download.IsDone()) {
            m_installedFolder = m_download.Folder();
            m_state = State::Finishing;
            NOBLE_INFO("ModDownloadPrompt: Workshop item {} installed at '{}'", m_modId, m_installedFolder.string());
        } else if (m_download.IsFailed()) {
            m_message = m_download.Status();
            m_state = State::Failed;
        }
    }

    bool detailsReady = false;
    bool detailsOk = false;
    unsigned long long detailsSize = 0;
    std::string detailsSizeText;
    std::string detailsTitle, detailsPreview, detailsDesc, detailsAuthor, detailsCreatorId, detailsUpdated;
    unsigned long long detailsSubs = 0, detailsFavs = 0;
    int detailsChildren = 0;
    std::vector<std::string> detailsTags;
    if (m_details) {
        std::lock_guard<std::mutex> lock(m_details->mutex);
        detailsReady = m_details->ready;
        detailsOk = m_details->ok;
        detailsSize = m_details->size;
        detailsSizeText = m_details->sizeText;
        detailsTitle = m_details->title;
        detailsPreview = m_details->previewPath;
        detailsDesc = m_details->description;
        detailsAuthor = m_details->authorName;
        detailsCreatorId = m_details->creatorId;
        detailsUpdated = m_details->updatedText;
        detailsSubs = m_details->subscriptions;
        detailsFavs = m_details->favorited;
        detailsChildren = m_details->childCount;
        detailsTags = m_details->tags;
    }

    if (!m_popupOpened) {
        ImGui::OpenPopup(kPopupId);
        m_popupOpened = true;
    }

    const float sScale = NobleUi::Scale();
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(740.0f * sScale, 0.0f));
    bool joinAfterPopup = false;
    if (ImGui::BeginPopupModal(kPopupId, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImGui::Spacing();
        const bool canAct = m_state == State::Prompt || m_state == State::Failed;
        static int s_promptBtn = 0;
        if (ImGui::IsWindowAppearing()) s_promptBtn = 0;
        if (canAct) {
            if (UiPad::Left() || UiPad::Right() ||
                ImGui::IsKeyPressed(ImGuiKey_LeftArrow, false) || ImGui::IsKeyPressed(ImGuiKey_RightArrow, false)) {
                s_promptBtn = 1 - s_promptBtn;
            }
        }
        const bool padAccept = UiPad::AcceptPressed() || ImGui::IsKeyPressed(ImGuiKey_Enter, false);
        const bool padBack = UiPad::BackPressed() || ImGui::IsKeyPressed(ImGuiKey_Escape, false) ||
                             ImGui::IsKeyPressed(ImGuiKey_GamepadFaceRight, false);

        if (m_state == State::Prompt) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.92f, 0.95f, 0.98f, 1.0f));
            ImGui::TextWrapped("%s", kMissingMessage);
            ImGui::PopStyleColor();
            ImGui::Spacing();
        }

        const std::string& displayTitle = !detailsTitle.empty() ? detailsTitle : m_modTitle;
        if (!displayTitle.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, NobleStyle::Accent());
            ImGui::TextWrapped("%s", displayTitle.c_str());
            ImGui::PopStyleColor();
        }

        // Steam Workshop details card: thumbnail, creator, size, updated date, subscribers, tags & description
        if (!m_demo) {
            UiTexture preview;
            if (detailsReady && detailsOk && !detailsPreview.empty()) {
                preview = UiArt::ImageFile(std::filesystem::path(detailsPreview));
            }
            const float metaStartX = ImGui::GetCursorPosX();
            if (preview) {
                const float maxImgW = preview.width > 0 ? std::min(220.0f * sScale, static_cast<float>(preview.width)) : (220.0f * sScale);
                const float imageH = preview.width > 0 ? maxImgW * preview.height / preview.width : (124.0f * sScale);
                ImGui::Spacing();
                ImGui::BeginGroup();
                const ImVec2 p0 = ImGui::GetCursorScreenPos();
                const ImVec2 p1(p0.x + maxImgW, p0.y + imageH);
                UiArt::DrawCoverCrisp(dl, preview, p0, p1, IM_COL32(255, 255, 255, 255), 1.0f);
                NobleUi::DrawTacticalFrame(dl, p0, p1, sScale, false, true, false, false);
                ImGui::Dummy(ImVec2(maxImgW, imageH));
                ImGui::EndGroup();
                ImGui::SameLine(0.0f, 14.0f * sScale);
            } else {
                ImGui::Spacing();
            }

            ImGui::BeginGroup();
            ImGui::TextDisabled("Workshop ID: %s", m_modId.c_str());
            if (!detailsAuthor.empty()) {
                ImGui::Text("Author: %s", detailsAuthor.c_str());
            } else if (!detailsCreatorId.empty()) {
                ImGui::TextDisabled("Creator ID: %s", detailsCreatorId.c_str());
            }
            if (!detailsSizeText.empty()) {
                ImGui::TextColored(ImVec4(0.75f, 0.90f, 1.0f, 1.0f), "Download Size: %s", detailsSizeText.c_str());
            } else if (detailsSize > 0) {
                ImGui::TextColored(ImVec4(0.75f, 0.90f, 1.0f, 1.0f), "Download Size: %s", FormatBytes(detailsSize).c_str());
            } else if (!detailsReady) {
                ImGui::TextDisabled("Fetching Workshop details...");
            }
            if (!detailsUpdated.empty()) {
                ImGui::TextDisabled("Updated: %s", detailsUpdated.c_str());
            }
            if (detailsSubs > 0 || detailsFavs > 0) {
                ImGui::TextDisabled("Subscribers: %s   |   Favorites: %s",
                                    FormatCount(detailsSubs).c_str(), FormatCount(detailsFavs).c_str());
            }
            if (detailsChildren > 0) {
                ImGui::TextDisabled("Includes %d Workshop dependency item(s)", detailsChildren);
            }
            if (!detailsTags.empty()) {
                std::string tagLine = "Tags: ";
                for (size_t i = 0; i < detailsTags.size(); ++i) {
                    if (i > 0) tagLine += " / ";
                    tagLine += detailsTags[i];
                }
                ImGui::PushTextWrapPos(metaStartX + 690.0f * sScale);
                ImGui::TextDisabled("%s", tagLine.c_str());
                ImGui::PopTextWrapPos();
            }
            if (detailsReady && !detailsOk) {
                ImGui::TextDisabled("Workshop details are unavailable right now.");
            }
            ImGui::EndGroup();

            if (!detailsDesc.empty() && m_state == State::Prompt) {
                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.08f, 0.12f, 0.72f));
                if (ImGui::BeginChild("ModDescriptionScroll", ImVec2(-1.0f, 125.0f * sScale), true)) {
                    if (UiPad::HasGamepad()) {
                        UiPad::ScrollWindow(false, true);
                    }
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.84f, 0.89f, 0.94f, 1.0f));
                    ImGui::TextWrapped("%s", detailsDesc.c_str());
                    ImGui::PopStyleColor();
                }
                ImGui::EndChild();
                ImGui::PopStyleColor();
            }
            ImGui::Spacing();
        }

        auto drawFocus = [&](int btnIdx) {
            if (UiPad::HasGamepad() && s_promptBtn == btnIdx) {
                NobleUi::DrawTacticalFrame(dl, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), sScale, true, true, false, true);
            }
        };

        if (m_state == State::Prompt) {
            ImGui::TextDisabled("Noble downloads this mod directly in the background and joins automatically once finished.");
            NobleStyle::SectionGap();
            const bool downloadPressed = NobleStyle::PrimaryButton("DOWNLOAD & JOIN", ImVec2(240.0f * sScale, 38.0f * sScale));
            drawFocus(0);
            if (!m_demo && (downloadPressed || (canAct && padAccept && s_promptBtn == 0))) {
                UiPad::ConsumeAccept();
                BeginDownload();
            }
            ImGui::SameLine();
            const bool cancelBtn = ImGui::Button("CANCEL", ImVec2(140.0f * sScale, 38.0f * sScale));
            drawFocus(1);
            if (cancelBtn || padBack || (canAct && padAccept && s_promptBtn == 1)) {
                UiPad::ConsumeAccept();
                UiPad::ConsumeBack();
                Cancel();
                ImGui::CloseCurrentPopup();
            }
        } else if (m_state == State::Downloading) {
            const unsigned long long dlBytes = m_download.BytesDownloaded();
            const unsigned long long totBytes = m_download.BytesTotal();
            const unsigned long long expBytes = m_download.ExpectedBytes();
            const double speed = m_download.SpeedBytesPerSec();
            const double eta = m_download.EtaSeconds();

            ImGui::TextWrapped("%s", m_download.Status().c_str());
            if (!m_download.StageLabel().empty()) {
                ImGui::TextDisabled("%s", m_download.StageLabel().c_str());
            }
            ImGui::Spacing();

            if (totBytes > 0) {
                const float frac = m_download.ProgressFraction();
                char overlay[128];
                std::snprintf(overlay, sizeof(overlay), "%.1f%%   (%s / %s)",
                              frac * 100.0f, FormatBytes(dlBytes).c_str(), FormatBytes(totBytes).c_str());
                ImGui::ProgressBar(frac, ImVec2(-1.0f, 24.0f * sScale), overlay);
            } else if (dlBytes > 0 && expBytes > 0) {
                const float frac = m_download.ProgressFraction();
                char overlay[128];
                std::snprintf(overlay, sizeof(overlay), "%.1f%%   (%s / ~%s)",
                              frac * 100.0f, FormatBytes(dlBytes).c_str(), FormatBytes(expBytes).c_str());
                ImGui::ProgressBar(frac, ImVec2(-1.0f, 24.0f * sScale), overlay);
            } else {
                ImGui::ProgressBar(-0.6f * static_cast<float>(ImGui::GetTime()), ImVec2(-1.0f, 22.0f * sScale),
                                   expBytes > 0 ? ("Preparing (~" + FormatBytes(expBytes) + ")...").c_str() : "Preparing download...");
            }

            ImGui::Spacing();
            std::string statsLine = "Speed: " + FormatSpeed(speed);
            const std::string etaText = FormatEta(eta);
            if (!etaText.empty()) statsLine += "   |   " + etaText;
            if (totBytes > 0 && dlBytes > 0) {
                statsLine += "   |   " + FormatBytes(totBytes - std::min(dlBytes, totBytes)) + " remaining";
            }
            ImGui::TextDisabled("%s", statsLine.c_str());

            NobleStyle::SectionGap();
            if (ImGui::Button("CANCEL DOWNLOAD", ImVec2(170.0f * sScale, 36.0f * sScale)) || padBack) {
                UiPad::ConsumeBack();
                Cancel();
                ImGui::CloseCurrentPopup();
            }
        } else if (m_state == State::Finishing) {
            ImGui::TextWrapped("The mod is installed and verified! Joining the lobby...");
            ImGui::Spacing();
            ImGui::ProgressBar(1.0f, ImVec2(-1.0f, 24.0f * sScale), "100%");
            ImGui::CloseCurrentPopup();
            joinAfterPopup = true;
        } else if (m_state == State::Failed) {
            ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.42f, 1.0f), "%s", m_message.c_str());
            ImGui::Spacing();
            NobleStyle::SectionGap();
            const bool retry = NobleStyle::PrimaryButton("RETRY DOWNLOAD", ImVec2(190.0f * sScale, 38.0f * sScale));
            drawFocus(0);
            if (!m_demo && (retry || (padAccept && s_promptBtn == 0))) {
                UiPad::ConsumeAccept();
                BeginDownload();
            }
            ImGui::SameLine();
            const bool cancelFail = ImGui::Button("CANCEL", ImVec2(130.0f * sScale, 38.0f * sScale));
            drawFocus(1);
            if (cancelFail || padBack || (padAccept && s_promptBtn == 1)) {
                UiPad::ConsumeAccept();
                UiPad::ConsumeBack();
                Cancel();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("OPEN WORKSHOP PAGE", ImVec2(190.0f * sScale, 38.0f * sScale))) {
                SteamWorkshop::OpenWorkshopPage(m_modId);
            }
        }

        ImGui::Spacing();
        if (m_state == State::Prompt) ImGui::TextDisabled("[A] / ENTER  Download & Join        [B] / ESC  Cancel        [Right Stick]  Scroll Description");
        else if (m_state == State::Failed) ImGui::TextDisabled("[A] / ENTER  Confirm        [B] / ESC  Cancel");
        else ImGui::TextDisabled("[B] / ESC  Cancel");
        ImGui::EndPopup();
    }

    if (joinAfterPopup) {
        m_state = State::Hidden;
        m_popupOpened = false;
        std::function<void(const std::filesystem::path&)> ready = std::move(m_onReady);
        m_onReady = nullptr;
        if (ready) ready(m_installedFolder);
    }
}

} // namespace noble
