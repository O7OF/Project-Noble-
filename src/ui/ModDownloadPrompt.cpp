#include "ModDownloadPrompt.hpp"
#include "NobleStyle.hpp"
#include "UiArt.hpp"
#include "common/Logger.hpp"
#include "vfs/VFS.hpp"

#include <imgui.h>
#include <nlohmann/json.hpp>
#include <winhttp.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>

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
            unsigned long long size = 0;
            if (item.contains("file_size")) {
                const auto& value = item.at("file_size");
                if (value.is_string()) size = std::strtoull(value.get<std::string>().c_str(), nullptr, 10);
                else if (value.is_number_unsigned()) size = value.get<unsigned long long>();
                else if (value.is_number_integer() && value.get<long long>() > 0) {
                    size = static_cast<unsigned long long>(value.get<long long>());
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
            // Steam reports file_size 0 for items stored with hcontent_file (this mod: 20.6 GB), so read
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
            std::lock_guard<std::mutex> lock(details->mutex);
            details->size = size;
            details->sizeText = sizeText;
            details->title = item.value("title", std::string());
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
    std::string detailsTitle, detailsPreview;
    if (m_details) {
        std::lock_guard<std::mutex> lock(m_details->mutex);
        detailsReady = m_details->ready;
        detailsOk = m_details->ok;
        detailsSize = m_details->size;
        detailsSizeText = m_details->sizeText;
        detailsTitle = m_details->title;
        detailsPreview = m_details->previewPath;
    }

    if (!m_popupOpened) {
        ImGui::OpenPopup(kPopupId);
        m_popupOpened = true;
    }

    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(640.0f, 0.0f));
    bool joinAfterPopup = false;
    if (ImGui::BeginPopupModal(kPopupId, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Spacing();
        const bool canAct = m_state == State::Prompt || m_state == State::Failed;
        if (m_state == State::Prompt) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.92f, 0.95f, 0.98f, 1.0f));
            ImGui::TextWrapped("%s", kMissingMessage);
            ImGui::PopStyleColor();
            ImGui::Spacing();
        }

        if (!m_modTitle.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, NobleStyle::Accent());
            ImGui::TextWrapped("%s", m_modTitle.c_str());
            ImGui::PopStyleColor();
            ImGui::TextDisabled("Workshop item %s", m_modId.c_str());
        }

        // Steam's own Workshop picture and the item's download size, when the Web API answered.
        if (!m_demo) {
            UiTexture preview;
            if (detailsReady && detailsOk && !detailsPreview.empty()) {
                preview = UiArt::ImageFile(std::filesystem::path(detailsPreview));
            }
            if (preview) {
                const float imageW = 230.0f;
                const float imageH = preview.width > 0 ? imageW * preview.height / preview.width : 130.0f;
                ImGui::Spacing();
                ImGui::BeginGroup();
                ImGui::Image(preview.id, ImVec2(imageW, imageH));
                ImGui::EndGroup();
                ImGui::SameLine();
                ImGui::BeginGroup();
                if (!detailsTitle.empty()) ImGui::TextWrapped("%s", detailsTitle.c_str());
                if (detailsSize > 0) ImGui::TextDisabled("Download size: %s", FormatBytes(detailsSize).c_str());
                else if (!detailsSizeText.empty()) ImGui::TextDisabled("Download size: %s", detailsSizeText.c_str());
                ImGui::EndGroup();
            } else {
                if (detailsSize > 0) {
                    ImGui::Spacing();
                    ImGui::TextDisabled("Download size: %s", FormatBytes(detailsSize).c_str());
                } else if (!detailsSizeText.empty()) {
                    ImGui::Spacing();
                    ImGui::TextDisabled("Download size: %s", detailsSizeText.c_str());
                } else if (detailsReady && detailsOk) {
                    ImGui::Spacing();
                    ImGui::TextDisabled("Steam lists this item, but it has no preview picture.");
                } else if (detailsReady) {
                    ImGui::Spacing();
                    ImGui::TextDisabled("Workshop details and size are unavailable right now.");
                } else {
                    ImGui::Spacing();
                    ImGui::TextDisabled("Fetching the Workshop picture and download size...");
                }
            }
            ImGui::Spacing();
        }

        if (m_state == State::Prompt) {
            ImGui::TextDisabled("Noble can ask Steam to download it, then join the lobby once it is installed.");
            NobleStyle::SectionGap();
            const bool downloadPressed = NobleStyle::PrimaryButton("DOWNLOAD & JOIN", ImVec2(230.0f, 38.0f));
            if (!m_demo && (downloadPressed ||
                (canAct && ImGui::IsKeyPressed(ImGuiKey_Enter, false)) ||
                (canAct && ImGui::IsKeyPressed(ImGuiKey_GamepadFaceDown, false)))) {
                BeginDownload();
            }
            ImGui::SameLine();
            if (ImGui::Button("CANCEL", ImVec2(130.0f, 38.0f)) ||
                ImGui::IsKeyPressed(ImGuiKey_Escape, false) ||
                ImGui::IsKeyPressed(ImGuiKey_GamepadFaceRight, false)) {
                Cancel();
                ImGui::CloseCurrentPopup();
            }
        } else if (m_state == State::Downloading) {
            ImGui::TextWrapped("%s", m_download.Status().c_str());
            ImGui::Spacing();
            ImGui::ProgressBar(-1.0f * static_cast<float>(ImGui::GetTime()), ImVec2(-1.0f, 8.0f), "");
            ImGui::Spacing();
            if (m_download.NeedsManualSubscribe()) {
                ImGui::TextDisabled("The Steam window may be behind the game. Nothing else is needed here; Noble "
                                    "joins the lobby as soon as Steam finishes the download.");
            }
            NobleStyle::SectionGap();
            if (m_download.NeedsManualSubscribe() && ImGui::Button("OPEN WORKSHOP PAGE", ImVec2(200.0f, 34.0f))) {
                SteamWorkshop::OpenWorkshopPage(m_modId);
            }
            if (m_download.NeedsManualSubscribe()) ImGui::SameLine();
            if (ImGui::Button("CANCEL", ImVec2(130.0f, 34.0f)) || ImGui::IsKeyPressed(ImGuiKey_Escape, false) ||
                ImGui::IsKeyPressed(ImGuiKey_GamepadFaceRight, false)) {
                Cancel();
                ImGui::CloseCurrentPopup();
            }
        } else if (m_state == State::Finishing) {
            ImGui::TextWrapped("The mod is installed. Joining the lobby...");
            ImGui::Spacing();
            ImGui::ProgressBar(1.0f, ImVec2(-1.0f, 8.0f), "");
            ImGui::CloseCurrentPopup();
            joinAfterPopup = true;
        } else if (m_state == State::Failed) {
            ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.42f, 1.0f), "%s", m_message.c_str());
            ImGui::Spacing();
            ImGui::TextDisabled("You can also subscribe on the item's Workshop page, then press RETRY.");
            if (ImGui::Button("OPEN WORKSHOP PAGE", ImVec2(200.0f, 30.0f))) SteamWorkshop::OpenWorkshopPage(m_modId);
            NobleStyle::SectionGap();
            const bool retry = NobleStyle::PrimaryButton("RETRY", ImVec2(160.0f, 38.0f));
            if (!m_demo && (retry || ImGui::IsKeyPressed(ImGuiKey_Enter, false) ||
                            ImGui::IsKeyPressed(ImGuiKey_GamepadFaceDown, false))) {
                BeginDownload();
            }
            ImGui::SameLine();
            if (ImGui::Button("CANCEL", ImVec2(130.0f, 38.0f)) || ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
                Cancel();
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::Spacing();
        if (m_state == State::Prompt) ImGui::TextDisabled("ENTER  Download & Join        ESC  Cancel");
        else if (m_state == State::Failed) ImGui::TextDisabled("ENTER  Retry        ESC  Cancel");
        else ImGui::TextDisabled("ESC  Cancel");
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
