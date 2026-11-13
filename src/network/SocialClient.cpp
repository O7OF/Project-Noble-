#include "SocialClient.hpp"
#include "AuthClient.hpp"
#include "common/Logger.hpp"
#include "common/Settings.hpp"
#include "common/BuildIdentity.hpp"
#include "common/DeviceIdentity.hpp"

#include <windows.h>
#include <winhttp.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>

#pragma comment(lib, "winhttp.lib")

namespace noble {

namespace {

std::wstring Widen(const std::string& narrow) {
    if (narrow.empty()) return L"";
    const int req = MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), -1, nullptr, 0);
    if (req <= 0) return L"";
    std::wstring out(static_cast<size_t>(req), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), -1, &out[0], req);
    if (!out.empty() && out.back() == L'\0') out.pop_back();
    return out;
}

std::string ResolveMasterUrl() {
    std::string url = Settings::Get().Snapshot().masterServerUrl;
    if (url.empty()) {
        url = std::string(kMasterDefault);
    }
    return url;
}

std::string PostJson(const std::string& fullUrl, const std::string& path, const std::string& jsonBody, int& outStatusCode) {
    outStatusCode = 0;
    URL_COMPONENTS parts{};
    parts.dwStructSize = sizeof(parts);
    parts.dwSchemeLength = static_cast<DWORD>(-1);
    parts.dwHostNameLength = static_cast<DWORD>(-1);
    parts.dwUrlPathLength = static_cast<DWORD>(-1);
    parts.dwExtraInfoLength = static_cast<DWORD>(-1);

    std::wstring wideUrl = Widen(fullUrl);
    if (!WinHttpCrackUrl(wideUrl.c_str(), static_cast<DWORD>(wideUrl.size()), 0, &parts)) {
        return "";
    }

    std::wstring host(parts.lpszHostName, parts.dwHostNameLength);
    const bool isHttps = (parts.nScheme == INTERNET_SCHEME_HTTPS);
    const INTERNET_PORT port = parts.nPort ? parts.nPort : (isHttps ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT);

    std::wstring requestPath = parts.dwUrlPathLength > 0 ? std::wstring(parts.lpszUrlPath, parts.dwUrlPathLength) : L"";
    while (!requestPath.empty() && requestPath.back() == L'/') requestPath.pop_back();
    std::wstring subPath = Widen(path);
    if (!subPath.empty() && subPath.front() != L'/') requestPath += L'/';
    requestPath += subPath;

    HINTERNET session = WinHttpOpen(L"ProjectNoble/0.1", WINHTTP_ACCESS_TYPE_NO_PROXY, nullptr, nullptr, 0);
    if (!session) return "";
    WinHttpSetTimeouts(session, 5000, 5000, 5000, 5000);

    HINTERNET connect = WinHttpConnect(session, host.c_str(), port, 0);
    HINTERNET request = nullptr;
    if (connect) {
        DWORD flags = isHttps ? WINHTTP_FLAG_SECURE : 0;
        request = WinHttpOpenRequest(connect, L"POST", requestPath.c_str(), nullptr,
                                     WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    }

    std::string responseBody;
    if (request) {
        if (isHttps) {
            DWORD secFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA |
                             SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
                             SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
                             SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;
            WinHttpSetOption(request, WINHTTP_OPTION_SECURITY_FLAGS, &secFlags, sizeof(secFlags));
        }

        std::wstring headers = L"Content-Type: application/json\r\n";
        const std::string token = AuthClient::Get().GetSessionToken();
        if (!token.empty()) {
            headers += L"Authorization: Bearer " + Widen(token) + L"\r\n";
        }

        BOOL sent = WinHttpSendRequest(request, headers.c_str(), static_cast<DWORD>(headers.size()),
                                       (void*)jsonBody.data(), static_cast<DWORD>(jsonBody.size()),
                                       static_cast<DWORD>(jsonBody.size()), 0);
        if (sent && WinHttpReceiveResponse(request, nullptr)) {
            DWORD statusCode = 0;
            DWORD size = sizeof(statusCode);
            WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                                WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &size, WINHTTP_NO_HEADER_INDEX);
            outStatusCode = static_cast<int>(statusCode);

            DWORD bytesAvailable = 0;
            while (WinHttpQueryDataAvailable(request, &bytesAvailable) && bytesAvailable > 0) {
                std::vector<char> buffer(bytesAvailable + 1, 0);
                DWORD bytesRead = 0;
                if (WinHttpReadData(request, buffer.data(), bytesAvailable, &bytesRead) && bytesRead > 0) {
                    responseBody.append(buffer.data(), bytesRead);
                }
            }
        }
        WinHttpCloseHandle(request);
    }
    if (connect) WinHttpCloseHandle(connect);
    WinHttpCloseHandle(session);
    return responseBody;
}

} // namespace

SocialClient& SocialClient::Get() {
    static SocialClient s_instance;
    return s_instance;
}

static std::filesystem::path FriendsFilePath() {
    wchar_t exe[MAX_PATH];
    GetModuleFileNameW(nullptr, exe, MAX_PATH);
    return std::filesystem::path(exe).parent_path() / "data" / "userdata" / "friends.json";
}

void SocialClient::SaveFriendsLocked() {
    std::error_code ec;
    auto path = FriendsFilePath();
    std::filesystem::create_directories(path.parent_path(), ec);
    std::ofstream file(path, std::ios::trunc);
    if (file) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& e : m_friends) {
            nlohmann::json item;
            item["username"] = e.username;
            item["status"] = e.status;
            item["dir"] = e.direction;
            item["endpoint"] = e.endpoint;
            arr.push_back(item);
        }
        file << arr.dump(2);
    }
    DeviceIdentityStore::MirrorSaveFile("userdata/friends.json");
}

SocialClient::~SocialClient() {
    Shutdown();
}

void SocialClient::Initialize() {
    if (m_running) return;
    m_running = true;

    // Migrate friends.json if a newer file exists in AppData or userdata
    DeviceIdentityStore::MigrateNewestFile("userdata/friends.json");

    // Load locally persisted friends
    {
        std::error_code ec;
        auto path = FriendsFilePath();
        if (std::filesystem::exists(path, ec)) {
            std::ifstream file(path);
            if (file) {
                try {
                    auto j = nlohmann::json::parse(file);
                    if (j.is_array()) {
                        std::lock_guard<std::mutex> lock(m_mutex);
                        for (const auto& item : j) {
                            FriendEntry f;
                            f.username = item.value("username", "");
                            f.status = item.value("status", "accepted");
                            f.direction = item.value("dir", "local");
                            f.endpoint = item.value("endpoint", "");
                            if (!f.username.empty()) {
                                m_friends.push_back(f);
                            }
                        }
                    }
                } catch (...) {}
            }
        }
    }

    m_pollThread = std::thread([this]() {
        while (m_running) {
            if (AuthClient::Get().IsLoggedIn()) {
                RefreshFriendsAsync();
                PollInvitesAsync();
            }
            for (int i = 0; i < 50 && m_running; ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
        }
    });
}

void SocialClient::AddFriendDirect(const std::string& nameOrEndpoint) {
    if (nameOrEndpoint.empty()) return;
    std::string clean = nameOrEndpoint;
    while (!clean.empty() && (clean.front() == ' ' || clean.front() == '\t')) clean.erase(clean.begin());
    while (!clean.empty() && (clean.back() == ' ' || clean.back() == '\t')) clean.pop_back();
    if (clean.empty()) return;

    FriendEntry f;
    f.username = clean;
    f.status = "accepted";
    f.direction = "local";
    if (clean.find('.') != std::string::npos || clean.find(':') != std::string::npos) {
        f.endpoint = clean;
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const auto& existing : m_friends) {
            if (_stricmp(existing.username.c_str(), clean.c_str()) == 0) {
                SetStatusMessage("Friend '" + clean + "' is already on your list");
                return;
            }
        }
        m_friends.push_back(f);
        SaveFriendsLocked();
    }

    SetStatusMessage("Added friend '" + clean + "' to your friends list");
    if (AuthClient::Get().IsLoggedIn()) {
        SendFriendRequestAsync(clean);
    }
}

void SocialClient::Shutdown() {
    if (!m_running) return;
    m_running = false;
    if (m_pollThread.joinable()) {
        m_pollThread.join();
    }
}

std::vector<FriendEntry> SocialClient::GetFriends() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_friends;
}

std::vector<FriendEntry> SocialClient::GetIncomingRequests() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_incomingRequests;
}

std::vector<FriendEntry> SocialClient::GetOutgoingRequests() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_outgoingRequests;
}

std::vector<GameInvite> SocialClient::GetInvites() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_invites;
}

std::string SocialClient::GetLastStatusMessage() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_lastStatusMsg;
}

void SocialClient::SetStatusMessage(const std::string& msg) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_lastStatusMsg = msg;
}

void SocialClient::RefreshFriendsAsync() {
    if (!AuthClient::Get().IsLoggedIn()) return;
    const int userId = AuthClient::Get().GetUserId();
    if (userId <= 0) return;

    std::thread([this, userId]() {
        const std::string url = ResolveMasterUrl();
        nlohmann::json req;
        req["userId"] = userId;
        int status = 0;
        std::string res = PostJson(url, "/api/friends/list", req.dump(), status);
        if (status == 200 && !res.empty()) {
            try {
                auto j = nlohmann::json::parse(res);
                if (j.is_array()) {
                    std::vector<FriendEntry> accepted;
                    std::vector<FriendEntry> incoming;
                    std::vector<FriendEntry> outgoing;
                    for (const auto& item : j) {
                        FriendEntry f;
                        f.username = item.value("username", "");
                        f.status = item.value("status", "");
                        f.direction = item.value("dir", "");
                        if (f.direction == "accepted") {
                            accepted.push_back(f);
                        } else if (f.direction == "incoming") {
                            incoming.push_back(f);
                        } else if (f.direction == "outgoing") {
                            outgoing.push_back(f);
                        }
                    }
                    std::lock_guard<std::mutex> lock(m_mutex);
                    // Preserve all locally added friends when syncing from backend
                    std::vector<FriendEntry> merged = std::move(accepted);
                    for (const auto& local : m_friends) {
                        if (local.direction == "local") {
                            bool alreadyInMerged = false;
                            for (const auto& m : merged) {
                                if (_stricmp(m.username.c_str(), local.username.c_str()) == 0) {
                                    alreadyInMerged = true;
                                    break;
                                }
                            }
                            if (!alreadyInMerged) {
                                merged.push_back(local);
                            }
                        }
                    }
                    m_friends = std::move(merged);
                    m_incomingRequests = std::move(incoming);
                    m_outgoingRequests = std::move(outgoing);
                    SaveFriendsLocked();
                }
            } catch (...) {}
        }
    }).detach();
}

void SocialClient::PollInvitesAsync() {
    if (!AuthClient::Get().IsLoggedIn()) return;
    const std::string username = AuthClient::Get().GetUsername();
    if (username.empty()) return;

    std::thread([this, username]() {
        const std::string url = ResolveMasterUrl();
        nlohmann::json req;
        req["username"] = username;
        int status = 0;
        std::string res = PostJson(url, "/api/invites/list", req.dump(), status);
        if (status == 200 && !res.empty()) {
            try {
                auto j = nlohmann::json::parse(res);
                if (j.is_array()) {
                    std::vector<GameInvite> invites;
                    for (const auto& item : j) {
                        GameInvite inv;
                        inv.senderUsername = item.value("senderUsername", "");
                        inv.hostIp = item.value("hostIp", "");
                        inv.hostPort = static_cast<uint16_t>(item.value("hostPort", 11774));
                        inv.mode = item.value("mode", "Custom Game");
                        inv.map = item.value("map", "");
                        inv.sessionId = item.value("sessionId", "");
                        inv.timestamp = item.value("timestamp", 0ULL);
                        if (!inv.senderUsername.empty() && !inv.hostIp.empty()) {
                            invites.push_back(inv);
                        }
                    }
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_invites = std::move(invites);
                }
            } catch (...) {}
        }
    }).detach();
}

void SocialClient::SearchUsersAsync(const std::string& prefix, std::function<void(bool ok, const std::vector<UserSearchResult>& results)> callback) {
    const int userId = AuthClient::Get().GetUserId();
    std::thread([prefix, userId, callback]() {
        const std::string url = ResolveMasterUrl();
        nlohmann::json req;
        req["prefix"] = prefix;
        if (userId > 0) req["userId"] = userId;
        int status = 0;
        std::string res = PostJson(url, "/api/friends/search", req.dump(), status);
        std::vector<UserSearchResult> out;
        if (status == 200 && !res.empty()) {
            try {
                auto j = nlohmann::json::parse(res);
                if (j.is_array()) {
                    for (const auto& u : j) {
                        UserSearchResult r;
                        r.id = u.value("id", 0);
                        r.username = u.value("username", "");
                        if (!r.username.empty()) out.push_back(r);
                    }
                }
            } catch (...) {}
        }
        if (callback) callback(status == 200, out);
    }).detach();
}

void SocialClient::SendFriendRequestAsync(const std::string& targetUsername, std::function<void(bool ok, const std::string& message)> callback) {
    const int userId = AuthClient::Get().GetUserId();
    if (userId <= 0) {
        if (callback) callback(false, "Must be logged in");
        return;
    }

    std::thread([this, userId, targetUsername, callback]() {
        const std::string url = ResolveMasterUrl();
        nlohmann::json req;
        req["userId"] = userId;
        req["targetUsername"] = targetUsername;
        int status = 0;
        std::string res = PostJson(url, "/api/friends/add", req.dump(), status);
        std::string msg = "Request sent";
        bool ok = (status == 200);
        if (!res.empty()) {
            try {
                auto j = nlohmann::json::parse(res);
                if (j.contains("error")) { msg = j["error"]; ok = false; }
                else if (j.contains("message")) { msg = j["message"]; }
            } catch (...) {}
        } else if (!ok) {
            msg = "Failed to send request";
        }
        SetStatusMessage(msg);
        RefreshFriendsAsync();
        if (callback) callback(ok, msg);
    }).detach();
}

void SocialClient::AcceptFriendRequestAsync(const std::string& senderUsername, std::function<void(bool ok, const std::string& message)> callback) {
    const int userId = AuthClient::Get().GetUserId();
    if (userId <= 0) {
        if (callback) callback(false, "Must be logged in");
        return;
    }

    std::thread([this, userId, senderUsername, callback]() {
        const std::string url = ResolveMasterUrl();
        nlohmann::json req;
        req["userId"] = userId;
        req["senderUsername"] = senderUsername;
        int status = 0;
        std::string res = PostJson(url, "/api/friends/accept", req.dump(), status);
        std::string msg = "Accepted";
        bool ok = (status == 200);
        if (!res.empty()) {
            try {
                auto j = nlohmann::json::parse(res);
                if (j.contains("error")) { msg = j["error"]; ok = false; }
                else if (j.contains("message")) { msg = j["message"]; }
            } catch (...) {}
        } else if (!ok) {
            msg = "Failed to accept";
        }
        SetStatusMessage(msg);
        RefreshFriendsAsync();
        if (callback) callback(ok, msg);
    }).detach();
}

void SocialClient::RemoveFriendAsync(const std::string& targetUsername, std::function<void(bool ok, const std::string& message)> callback) {
    // Remove locally first and persist immediately
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_friends.erase(std::remove_if(m_friends.begin(), m_friends.end(), [&](const FriendEntry& e) {
            return _stricmp(e.username.c_str(), targetUsername.c_str()) == 0;
        }), m_friends.end());
        SaveFriendsLocked();
    }

    const int userId = AuthClient::Get().GetUserId();
    if (userId <= 0) {
        SetStatusMessage("Removed '" + targetUsername + "' from friends list");
        if (callback) callback(true, "Removed");
        return;
    }

    std::thread([this, userId, targetUsername, callback]() {
        const std::string url = ResolveMasterUrl();
        nlohmann::json req;
        req["userId"] = userId;
        req["targetUsername"] = targetUsername;
        int status = 0;
        std::string res = PostJson(url, "/api/friends/remove", req.dump(), status);
        std::string msg = "Removed";
        bool ok = (status == 200);
        if (!res.empty()) {
            try {
                auto j = nlohmann::json::parse(res);
                if (j.contains("error")) { msg = j["error"]; ok = false; }
                else if (j.contains("message")) { msg = j["message"]; }
            } catch (...) {}
        } else if (!ok) {
            msg = "Failed to remove";
        }
        SetStatusMessage(msg);
        RefreshFriendsAsync();
        if (callback) callback(ok, msg);
    }).detach();
}

void SocialClient::SendInviteAsync(const std::string& targetUsername, const std::string& hostIp, uint16_t hostPort,
                                   std::function<void(bool ok, const std::string& message)> callback,
                                   const std::string& mode,
                                   const std::string& map,
                                   const std::string& sessionId) {
    const std::string senderUsername = AuthClient::Get().GetUsername();
    if (senderUsername.empty()) {
        if (callback) callback(false, "Must be logged in");
        return;
    }

    std::thread([this, targetUsername, senderUsername, hostIp, hostPort, mode, map, sessionId, callback]() {
        const std::string url = ResolveMasterUrl();
        nlohmann::json req;
        req["targetUsername"] = targetUsername;
        req["senderUsername"] = senderUsername;
        req["hostIp"] = hostIp;
        req["hostPort"] = hostPort;
        req["mode"] = mode.empty() ? "Custom Game" : mode;
        if (!map.empty()) req["map"] = map;
        if (!sessionId.empty()) req["sessionId"] = sessionId;
        int status = 0;
        std::string res = PostJson(url, "/api/invites/send", req.dump(), status);
        std::string msg = "Invite sent";
        bool ok = (status == 200);
        if (!res.empty()) {
            try {
                auto j = nlohmann::json::parse(res);
                if (j.contains("error")) { msg = j["error"]; ok = false; }
                else if (j.contains("message")) { msg = j["message"]; }
            } catch (...) {}
        } else if (!ok) {
            msg = "Failed to send invite";
        }
        SetStatusMessage(msg);
        if (callback) callback(ok, msg);
    }).detach();
}

void SocialClient::ClearInviteAsync(const std::string& senderUsername) {
    const std::string username = AuthClient::Get().GetUsername();
    if (username.empty()) return;

    std::thread([this, username, senderUsername]() {
        const std::string url = ResolveMasterUrl();
        nlohmann::json req;
        req["username"] = username;
        req["senderUsername"] = senderUsername;
        int status = 0;
        PostJson(url, "/api/invites/clear", req.dump(), status);
        PollInvitesAsync();
    }).detach();
}

} // namespace noble

