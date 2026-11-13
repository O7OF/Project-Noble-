#include "AuthClient.hpp"
#include "common/DeviceIdentity.hpp"
#include "network/SteamAuth.hpp"
#include "network/MasterServerClient.hpp"
#include "network/SocialClient.hpp"
#include "common/Logger.hpp"
#include "common/Settings.hpp"

#include <windows.h>
#include <winhttp.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <functional>
#include <thread>
#include <vector>

#ifdef NOBLE_DEV_BUILD
constexpr bool kDevBuild = true;
#else
constexpr bool kDevBuild = false;
#endif

namespace noble {

namespace {

std::wstring Widen(const std::string& str) {
    if (str.empty()) return L"";
    const int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    if (len <= 0) return L"";
    std::wstring result(len - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result[0], len);
    return result;
}

std::string AuthPostJson(const std::string& fullUrl, const std::string& path, const std::string& jsonBody, int& outStatusCode) {
    outStatusCode = 0;
    if (fullUrl.empty()) return "";

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
    if (!connect) {
        WinHttpCloseHandle(session);
        return "";
    }

    DWORD flags = isHttps ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET request = WinHttpOpenRequest(connect, L"POST", requestPath.c_str(), nullptr,
                                           WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!request) {
        WinHttpCloseHandle(connect);
        WinHttpCloseHandle(session);
        return "";
    }

    if (isHttps) {
        DWORD secFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA |
                         SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
                         SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
                         SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;
        WinHttpSetOption(request, WINHTTP_OPTION_SECURITY_FLAGS, &secFlags, sizeof(secFlags));
    }

    std::wstring headers = L"Content-Type: application/json\r\nAccept: application/json\r\nConnection: close\r\n";
    BOOL sent = WinHttpSendRequest(request,
                                   headers.c_str(), static_cast<DWORD>(headers.size()),
                                   (void*)jsonBody.data(),
                                   static_cast<DWORD>(jsonBody.size()),
                                   static_cast<DWORD>(jsonBody.size()),
                                   0);

    std::string responseBody;
    if (sent && WinHttpReceiveResponse(request, nullptr)) {
        DWORD statusCode = 0;
        DWORD size = sizeof(statusCode);
        WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                            WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &size, WINHTTP_NO_HEADER_INDEX);
        outStatusCode = static_cast<int>(statusCode);

        DWORD bytesAvailable = 0;
        while (WinHttpQueryDataAvailable(request, &bytesAvailable) && bytesAvailable > 0) {
            std::vector<char> buf(bytesAvailable + 1, 0);
            DWORD bytesRead = 0;
            if (WinHttpReadData(request, buf.data(), bytesAvailable, &bytesRead) && bytesRead > 0) {
                responseBody.append(buf.data(), bytesRead);
            }
        }
    }

    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connect);
    WinHttpCloseHandle(session);

    return responseBody;
}

} // namespace

AuthClient& AuthClient::Get() {
    static AuthClient s_instance;
    return s_instance;
}

void AuthClient::Initialize() {
    DeviceIdentityStore::Reload();
    const SettingsData s = Settings::Get().Snapshot();
    if (s.rememberAccount && !s.accountToken.empty() && s.accountUserId > 0 && !s.accountUsername.empty()) {
        {
            std::lock_guard<std::mutex> lock(m_authMutex);
            m_isLoggedIn = true;
            m_username = s.accountUsername;
            m_sessionToken = s.accountToken;
            m_userId = s.accountUserId;
            m_isDeveloper = s.isDeveloper || kDevBuild;
        }
        NOBLE_INFO("AuthClient: restored remembered account '{}' (#{}, dev={})",
                   s.accountUsername, s.accountUserId, m_isDeveloper.load() ? "YES" : "NO");

        // Verify session validity with backend in the background
        std::thread([this, s]() {
            const std::string masterUrl = MasterServerClient::Get().Url();
            if (masterUrl.empty()) return;
            const auto& id = DeviceIdentityStore::Get();
            nlohmann::json body = {
                {"userId", s.accountUserId},
                {"session_token", s.accountToken},
                {"sessionToken", s.accountToken},
                {"username", s.accountUsername},
                {"deviceId", id.deviceId}
            };
            int status = 0;
            std::string res = AuthPostJson(masterUrl, "/api/presence", body.dump(), status);
            if (status == 401) {
                NOBLE_WARN("AuthClient: session token rejected by master server; invalidating local session");
                InvalidateSession("Session expired or logged in on another device");
            } else if (status >= 200 && status < 300 && !res.empty()) {
                try {
                    auto j = nlohmann::json::parse(res);
                    const bool isDev = j.value("is_developer", false);
                    const bool isTester = j.value("is_tester", false);
                    SetRoles(isDev, isTester);
                } catch (...) {}
            }
        }).detach();
    } else {
        std::lock_guard<std::mutex> lock(m_authMutex);
        m_isLoggedIn = false;
        m_username.clear();
        m_sessionToken.clear();
        m_userId = 0;
        m_isDeveloper = kDevBuild;
        m_isTester = false;
        NOBLE_INFO("AuthClient: initialized in logged-out state");
    }
}

void AuthClient::RefreshDeveloperStatusAsync() {
    if (m_refreshing.exchange(true)) return;
    std::thread([this]() {
        struct Guard {
            std::atomic<bool>& f;
            ~Guard() { f.store(false); }
        } guard{ m_refreshing };

        const std::string masterUrl = MasterServerClient::Get().Url();
        if (masterUrl.empty()) return;

        const auto& id = DeviceIdentityStore::Get();
        std::string user;
        int uid = 0;
        {
            std::lock_guard<std::mutex> lock(m_authMutex);
            user = m_username;
            uid = m_userId;
        }

        nlohmann::json body;
        if (uid > 0) body["userId"] = uid;
        if (!user.empty()) body["username"] = user;
        if (!id.deviceId.empty()) body["deviceId"] = id.deviceId;

        int code = 0;
        std::string resp = AuthPostJson(masterUrl, "/api/dev/check", body.dump(), code);
        if (code >= 200 && code < 300 && !resp.empty()) {
            try {
                auto j = nlohmann::json::parse(resp);
                const bool isDev = j.value("is_developer", false);
                const bool isTester = j.value("is_tester", false);
                SetRoles(isDev, isTester);
            } catch (...) {}
        }
    }).detach();
}

AuthResult AuthClient::Login(const std::string& username, const std::string& password, bool rememberMe) {
    AuthResult res;
    if (username.empty() || password.empty()) {
        res.error = "Username and password required";
        return res;
    }

    const std::string masterUrl = MasterServerClient::Get().Url();
    if (masterUrl.empty()) {
        res.error = "Master server URL not configured";
        return res;
    }

    const auto& id = DeviceIdentityStore::Get();
    nlohmann::json body = {
        {"username", username},
        {"password", password},
        {"deviceId", id.deviceId}
    };

    int statusCode = 0;
    std::string response = AuthPostJson(masterUrl, "/api/login", body.dump(), statusCode);
    if (statusCode == 200 && !response.empty()) {
        try {
            auto j = nlohmann::json::parse(response);
            const int uid = j.value("userId", 0);
            const std::string uname = j.value("username", username);
            const std::string token = j.value("session_token", "");
            const bool isDev = j.value("is_developer", false) || kDevBuild;
            const bool isTester = j.value("is_tester", false);

            {
                std::lock_guard<std::mutex> lock(m_authMutex);
                m_isLoggedIn = true;
                m_username = uname;
                m_sessionToken = token;
                m_userId = uid;
                m_isDeveloper = isDev;
                m_isTester = isTester;
            }

            auto s = Settings::Get().Snapshot();
            s.accountUsername = uname;
            s.accountUserId = uid;
            s.isDeveloper = isDev;
            s.playerName = uname;
            s.rememberAccount = rememberMe;
            s.accountToken = rememberMe ? token : "";
            Settings::Get().Update(s);

            DeviceIdentity updatedId = id;
            updatedId.playerName = uname;
            DeviceIdentityStore::Save(updatedId);

            res.success = true;
            res.username = uname;
            res.token = token;
            res.userId = uid;
            res.isDeveloper = isDev;

            SocialClient::Get().RefreshFriendsAsync();
            return res;
        } catch (...) {
            res.error = "Invalid server response";
            return res;
        }
    }

    if (statusCode == 409) {
        res.error = "Error: Account is already logged in on another device. Please log out on that device to proceed.";
        return res;
    }
    if (statusCode == 401) {
        res.error = "Invalid username or password.";
        return res;
    }

    if (!response.empty()) {
        try {
            auto j = nlohmann::json::parse(response);
            if (j.contains("error")) {
                res.error = j["error"].get<std::string>();
                return res;
            }
        } catch (...) {}
    }

    res.error = (statusCode == 0) ? "Could not connect to master server" : ("Login failed (HTTP " + std::to_string(statusCode) + ")");
    return res;
}

AuthResult AuthClient::Register(const std::string& username, const std::string& password, bool rememberMe) {
    AuthResult res;
    if (username.empty() || password.empty()) {
        res.error = "Username and password required";
        return res;
    }
    if (username.length() > 14) {
        res.error = "Username must be 14 characters or fewer";
        return res;
    }

    const std::string masterUrl = MasterServerClient::Get().Url();
    if (masterUrl.empty()) {
        res.error = "Master server URL not configured";
        return res;
    }

    const auto& id = DeviceIdentityStore::Get();
    nlohmann::json body = {
        {"username", username},
        {"password", password},
        {"deviceId", id.deviceId}
    };

    int statusCode = 0;
    std::string response = AuthPostJson(masterUrl, "/api/register", body.dump(), statusCode);
    if (statusCode == 200 && !response.empty()) {
        try {
            auto j = nlohmann::json::parse(response);
            const int uid = j.value("userId", 0);
            const std::string uname = j.value("username", username);
            const std::string token = j.value("session_token", "");
            const bool isDev = j.value("is_developer", false) || kDevBuild;
            const bool isTester = j.value("is_tester", false);

            {
                std::lock_guard<std::mutex> lock(m_authMutex);
                m_isLoggedIn = true;
                m_username = uname;
                m_sessionToken = token;
                m_userId = uid;
                m_isDeveloper = isDev;
                m_isTester = isTester;
            }

            auto s = Settings::Get().Snapshot();
            s.accountUsername = uname;
            s.accountUserId = uid;
            s.isDeveloper = isDev;
            s.playerName = uname;
            s.rememberAccount = rememberMe;
            s.accountToken = rememberMe ? token : "";
            Settings::Get().Update(s);

            DeviceIdentity updatedId = id;
            updatedId.playerName = uname;
            DeviceIdentityStore::Save(updatedId);

            res.success = true;
            res.username = uname;
            res.token = token;
            res.userId = uid;
            res.isDeveloper = isDev;

            SocialClient::Get().RefreshFriendsAsync();
            return res;
        } catch (...) {
            res.error = "Invalid server response";
            return res;
        }
    }

    if (!response.empty()) {
        try {
            auto j = nlohmann::json::parse(response);
            if (j.contains("error")) {
                res.error = j["error"].get<std::string>();
                return res;
            }
        } catch (...) {}
    }

    res.error = (statusCode == 0) ? "Could not connect to master server" : ("Registration failed (HTTP " + std::to_string(statusCode) + ")");
    return res;
}

void AuthClient::Logout() {
    int uid = 0;
    std::string uname;
    {
        std::lock_guard<std::mutex> lock(m_authMutex);
        uid = m_userId;
        uname = m_username;
        m_isLoggedIn = false;
        m_username.clear();
        m_sessionToken.clear();
        m_userId = 0;
        m_isDeveloper = kDevBuild;
        m_isTester = false;
    }
    auto s = Settings::Get().Snapshot();
    s.accountToken.clear();
    s.accountUserId = 0;
    Settings::Get().Update(s);

    if (uid > 0 || !uname.empty()) {
        std::thread([uid, uname]() {
            const std::string masterUrl = MasterServerClient::Get().Url();
            if (masterUrl.empty()) return;
            nlohmann::json body = { {"userId", uid}, {"username", uname} };
            int status = 0;
            AuthPostJson(masterUrl, "/api/logout", body.dump(), status);
        }).detach();
    }
    NOBLE_INFO("AuthClient: user logged out");
}

void AuthClient::InvalidateSession(const std::string& reason) {
    {
        std::lock_guard<std::mutex> lock(m_authMutex);
        m_isLoggedIn = false;
        m_sessionToken.clear();
        m_userId = 0;
        m_isDeveloper = kDevBuild;
        m_isTester = false;
    }
    auto s = Settings::Get().Snapshot();
    s.accountToken.clear();
    s.accountUserId = 0;
    Settings::Get().Update(s);
    NOBLE_WARN("AuthClient: session invalidated: {}", reason.empty() ? "expired" : reason);
}

bool AuthClient::SetPlayerName(const std::string& name) {
    const std::string clean = name.empty() ? "" : name.substr(0, 16);
    if (clean.empty()) return false;
    DeviceIdentity id = DeviceIdentityStore::Get();
    id.playerName = clean;
    DeviceIdentityStore::Save(id);
    {
        std::lock_guard<std::mutex> lock(m_authMutex);
        m_username = clean;
    }
    auto s = Settings::Get().Snapshot();
    s.playerName = clean;
    s.accountUsername = clean;
    Settings::Get().Update(s);
    NOBLE_INFO("AuthClient: display name changed to '{}' (device {} unchanged)", clean, id.deviceId);
    return true;
}

AuthResult AuthClient::LoginWithSteam() {
    AuthResult res;
    const auto& id = DeviceIdentityStore::Get();
    const SteamAuthResult steam = SteamAuth::Authenticate(MasterServerClient::Get().Url(), id.deviceId);
    if (!steam.success) { res.error = steam.error; return res; }
    {
        std::lock_guard<std::mutex> lock(m_authMutex);
        m_isLoggedIn = true;
        m_steamLinked = true;
        m_username = steam.username;
        m_sessionToken = steam.token;
        m_isDeveloper = steam.isDeveloper || kDevBuild;
        m_isTester = steam.isTester;
        m_userId = static_cast<int>(std::hash<std::string>{}(steam.steamId) & 0x7FFFFFFF);
    }
    auto s = Settings::Get().Snapshot();
    s.accountUsername = steam.username;
    s.accountToken = steam.token;
    s.accountUserId = m_userId;
    s.isDeveloper = m_isDeveloper.load();
    s.playerName = steam.username;
    Settings::Get().Update(s);
    res.success = true; res.username = steam.username; res.token = steam.token;
    res.isDeveloper = m_isDeveloper.load(); res.userId = m_userId;
    NOBLE_INFO("AuthClient: Steam linked as '{}' ({}), isDev={}, isTester={}",
               steam.username, steam.steamId, res.isDeveloper ? "YES" : "NO", m_isTester.load() ? "YES" : "NO");
    return res;
}

bool AuthClient::IsSteamLinked() const {
    std::lock_guard<std::mutex> lock(m_authMutex);
    return m_steamLinked;
}

bool AuthClient::IsLoggedIn() const {
    std::lock_guard<std::mutex> lock(m_authMutex);
    return m_isLoggedIn;
}

bool AuthClient::IsDeveloper() const {
    return m_isDeveloper.load() || kDevBuild;
}

bool AuthClient::IsTester() const {
    return m_isTester.load();
}

void AuthClient::SetRoles(bool isDev, bool isTester) {
    m_isDeveloper.store(isDev || kDevBuild);
    m_isTester.store(isTester);
    auto s = Settings::Get().Snapshot();
    s.isDeveloper = m_isDeveloper.load();
    Settings::Get().Update(s);
}

std::string AuthClient::GetUsername() const {
    std::lock_guard<std::mutex> lock(m_authMutex);
    return m_username;
}

std::string AuthClient::GetSessionToken() const {
    std::lock_guard<std::mutex> lock(m_authMutex);
    return m_sessionToken;
}

int AuthClient::GetUserId() const {
    std::lock_guard<std::mutex> lock(m_authMutex);
    return m_userId;
}

} // namespace noble
