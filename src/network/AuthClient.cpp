#include "AuthClient.hpp"
#include "SocialClient.hpp"
#include "common/Settings.hpp"
#include "common/Logger.hpp"

#include <windows.h>
#include <winhttp.h>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>

namespace noble {

namespace {

std::wstring Widen(const std::string& narrow) {
    if (narrow.empty()) return L"";
    int req = MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), -1, nullptr, 0);
    if (req <= 0) return L"";
    std::wstring out(static_cast<size_t>(req), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), -1, &out[0], req);
    if (!out.empty() && out.back() == L'\0') out.pop_back();
    return out;
}

std::string ResolveMasterUrl() {
    std::string url = Settings::Get().Snapshot().masterServerUrl;
    if (url.empty()) {
        url = "https://outbreak-z-test-server.onrender.com";
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
        std::wstring headers = L"Content-Type: application/json\r\nAccept: application/json\r\n";
        const void* postData = jsonBody.empty() ? nullptr : jsonBody.data();
        const DWORD postSize = static_cast<DWORD>(jsonBody.size());

        if (WinHttpSendRequest(request, headers.c_str(), static_cast<DWORD>(headers.size()),
                               const_cast<void*>(postData), postSize, postSize, 0) &&
            WinHttpReceiveResponse(request, nullptr)) {
            DWORD statusCode = 0;
            DWORD statusSize = sizeof(statusCode);
            WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                                WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusSize, WINHTTP_NO_HEADER_INDEX);
            outStatusCode = static_cast<int>(statusCode);

            DWORD read = 0;
            char buffer[2048];
            while (WinHttpReadData(request, buffer, sizeof(buffer), &read) && read > 0) {
                responseBody.append(buffer, read);
                if (responseBody.size() > 256 * 1024) break;
            }
        }
        WinHttpCloseHandle(request);
    }
    if (connect) WinHttpCloseHandle(connect);
    WinHttpCloseHandle(session);

    return responseBody;
}

} // namespace

AuthClient& AuthClient::Get() {
    static AuthClient s_instance;
    return s_instance;
}

void AuthClient::Initialize() {
    auto s = Settings::Get().Snapshot();
    if (!s.accountUsername.empty() && !s.accountToken.empty()) {
        {
            std::lock_guard<std::mutex> lock(m_authMutex);
            m_isLoggedIn = true;
            m_username = s.accountUsername;
            m_sessionToken = s.accountToken;
            m_isDeveloper = s.isDeveloper;
            m_userId = s.accountUserId;
        }
        NOBLE_INFO("AuthClient: Restored cached session for '{}' (Developer: {})", s.accountUsername, s.isDeveloper ? "YES" : "NO");
        RefreshDeveloperStatusAsync();
    }
}

void AuthClient::RefreshDeveloperStatusAsync() {
    int userId = 0;
    std::string username;
    {
        std::lock_guard<std::mutex> lock(m_authMutex);
        if (!m_isLoggedIn || m_refreshing.exchange(true)) return;
        userId = m_userId;
        username = m_username;
    }
    std::thread([this, userId, username]() {
        try {
            nlohmann::json body = { { "userId", userId }, { "username", username } };
            int statusCode = 0;
            const std::string response = PostJson(ResolveMasterUrl(), "/api/dev/check", body.dump(), statusCode);
            if (statusCode >= 200 && statusCode < 300) {
                auto j = nlohmann::json::parse(response);
                bool dev = false;
                if (j.contains("is_developer")) {
                    const auto& v = j["is_developer"];
                    dev = v.is_boolean() ? v.get<bool>() : (v.is_number() ? v.get<int>() == 1 : false);
                }
                bool updated = false;
                {
                    std::lock_guard<std::mutex> lock(m_authMutex);
                    if (m_isLoggedIn && m_username == username) {
                        const bool was = m_isDeveloper.exchange(dev);
                        if (was != dev) {
                            updated = true;
                        }
                    }
                }
                if (updated) {
                    auto s = Settings::Get().Snapshot();
                    s.isDeveloper = dev;
                    Settings::Get().Update(s);
                    NOBLE_INFO("AuthClient: Developer status for '{}' updated: {}", username, dev ? "DEVELOPER" : "STANDARD");
                }
            }
        } catch (...) {
        }
        m_refreshing = false;
    }).detach();
}

AuthResult AuthClient::Login(const std::string& username, const std::string& password) {
    AuthResult res;
    if (username.empty() || password.empty()) {
        res.error = "Username and password required";
        return res;
    }

    const std::string masterUrl = ResolveMasterUrl();
    nlohmann::json body = {
        { "username", username },
        { "password", password }
    };

    int statusCode = 0;
    std::string response = PostJson(masterUrl, "/api/login", body.dump(), statusCode);
    if (statusCode == 0) {
        res.error = "Could not connect to master authentication server";
        return res;
    }

    try {
        auto j = nlohmann::json::parse(response);
        if (statusCode >= 200 && statusCode < 300) {
            res.success = true;
            res.username = j.value("username", username);
            res.token = j.value("session_token", "");
            if (j.contains("is_developer")) {
                const auto& v = j["is_developer"];
                res.isDeveloper = v.is_boolean() ? v.get<bool>() : (v.is_number() ? v.get<int>() == 1 : false);
            } else {
                res.isDeveloper = false;
            }
            res.userId = j.value("userId", 0);

            {
                std::lock_guard<std::mutex> lock(m_authMutex);
                m_isLoggedIn = true;
                m_username = res.username;
                m_sessionToken = res.token;
                m_isDeveloper = res.isDeveloper;
                m_userId = res.userId;
            }

            // Persist to settings
            auto s = Settings::Get().Snapshot();
            s.accountUsername = res.username;
            s.accountToken = res.token;
            s.isDeveloper = res.isDeveloper;
            s.accountUserId = res.userId;
            s.playerName = res.username;
            Settings::Get().Update(s);

            SocialClient::Get().RefreshFriendsAsync();

            NOBLE_INFO("AuthClient: Login successful for '{}' (ID: {}, Developer: {})",
                       res.username, res.userId, res.isDeveloper ? "YES" : "NO");
            return res;
        } else {
            res.error = j.value("error", "Login failed (HTTP " + std::to_string(statusCode) + ")");
            return res;
        }
    } catch (...) {
        res.error = "Failed to parse login response from server";
        return res;
    }
}

AuthResult AuthClient::Register(const std::string& username, const std::string& password) {
    AuthResult res;
    if (username.empty() || password.empty()) {
        res.error = "Username and password required";
        return res;
    }
    if (username.length() > 14) {
        res.error = "Username must be 14 characters or fewer";
        return res;
    }

    const std::string masterUrl = ResolveMasterUrl();
    nlohmann::json body = {
        { "username", username },
        { "password", password }
    };

    int statusCode = 0;
    std::string response = PostJson(masterUrl, "/api/register", body.dump(), statusCode);
    if (statusCode == 0) {
        res.error = "Could not connect to master authentication server";
        return res;
    }

    try {
        auto j = nlohmann::json::parse(response);
        if (statusCode >= 200 && statusCode < 300) {
            res.success = true;
            res.username = username;
            res.userId = j.value("userId", 0);
            NOBLE_INFO("AuthClient: Registration successful for '{}' (ID: {})", username, res.userId);
            // Immediately login
            return Login(username, password);
        } else {
            res.error = j.value("error", "Registration failed (HTTP " + std::to_string(statusCode) + ")");
            return res;
        }
    } catch (...) {
        res.error = "Failed to parse registration response from server";
        return res;
    }
}

void AuthClient::Logout() {
    std::string user;
    int uid = 0;
    {
        std::lock_guard<std::mutex> lock(m_authMutex);
        user = m_username;
        uid = m_userId;
        m_isLoggedIn = false;
        m_isDeveloper = false;
        m_username.clear();
        m_sessionToken.clear();
        m_userId = 0;
    }

    if (uid > 0 || !user.empty()) {
        const std::string masterUrl = ResolveMasterUrl();
        nlohmann::json body = {
            { "userId", uid },
            { "username", user }
        };
        int status = 0;
        PostJson(masterUrl, "/api/logout", body.dump(), status);
    }

    auto s = Settings::Get().Snapshot();
    s.accountUsername.clear();
    s.accountToken.clear();
    s.isDeveloper = false;
    s.accountUserId = 0;
    Settings::Get().Update(s);

    NOBLE_INFO("AuthClient: Logged out cleanly.");
}

bool AuthClient::IsLoggedIn() const {
    std::lock_guard<std::mutex> lock(m_authMutex);
    return m_isLoggedIn;
}

bool AuthClient::IsDeveloper() const {
    return m_isDeveloper.load();
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
