#include "AuthClient.hpp"
#include "common/DeviceIdentity.hpp"
#include "network/SteamAuth.hpp"
#include "network/MasterServerClient.hpp"
#include "common/Logger.hpp"
#include "common/Settings.hpp"

#include <windows.h>
#include <winhttp.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <functional>
#include <thread>

#ifdef NOBLE_DEV_BUILD
constexpr bool kDevBuild = true;
#else
constexpr bool kDevBuild = false;
#endif

namespace noble {

AuthClient& AuthClient::Get() {
    static AuthClient s_instance;
    return s_instance;
}

void AuthClient::Initialize() {
    DeviceIdentityStore::Reload();
    const DeviceIdentity& id = DeviceIdentityStore::Get();
    {
        std::lock_guard<std::mutex> lock(m_authMutex);
        m_isLoggedIn = true;          // offline identity: always "logged in"
        m_username = id.playerName;
        m_sessionToken.clear();
        m_isDeveloper = kDevBuild;
        m_userId = static_cast<int>(std::hash<std::string>{}(id.deviceId) & 0x7FFFFFFF);
    }
    auto s = Settings::Get().Snapshot();
    s.accountUsername = id.playerName;
    s.accountToken.clear();
    s.accountUserId = GetUserId();
    s.isDeveloper = id.developer;
    if (s.playerName.empty()) s.playerName = id.playerName;
    if (!s.serviceTag.empty() && s.serviceTag != "NOBL" && id.serviceTag != s.serviceTag) {
        DeviceIdentity updatedId = id;
        updatedId.serviceTag = s.serviceTag;
        DeviceIdentityStore::Save(updatedId);
    }
    Settings::Get().Update(s);
    NOBLE_INFO("AuthClient: offline identity '{}' (device {}, developer {})",
               id.playerName, id.deviceId, id.developer ? "YES" : "NO");
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

        URL_COMPONENTS parts{};
        parts.dwStructSize = sizeof(parts);
        parts.dwSchemeLength = static_cast<DWORD>(-1);
        parts.dwHostNameLength = static_cast<DWORD>(-1);
        parts.dwUrlPathLength = static_cast<DWORD>(-1);
        std::wstring wUrl(masterUrl.begin(), masterUrl.end());
        if (!WinHttpCrackUrl(wUrl.c_str(), static_cast<DWORD>(wUrl.size()), 0, &parts)) return;
        std::wstring host(parts.lpszHostName, parts.dwHostNameLength);
        const bool https = parts.nScheme == INTERNET_SCHEME_HTTPS;
        const INTERNET_PORT port = parts.nPort ? parts.nPort : (https ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT);
        std::wstring reqPath = parts.dwUrlPathLength ? std::wstring(parts.lpszUrlPath, parts.dwUrlPathLength) : L"";
        while (!reqPath.empty() && reqPath.back() == L'/') reqPath.pop_back();
        reqPath += L"/api/dev/check";

        HINTERNET session = WinHttpOpen(L"ProjectNoble/0.1", WINHTTP_ACCESS_TYPE_NO_PROXY, nullptr, nullptr, 0);
        if (!session) return;
        WinHttpSetTimeouts(session, 4000, 4000, 4000, 4000);
        HINTERNET connect = WinHttpConnect(session, host.c_str(), port, 0);
        HINTERNET request = connect ? WinHttpOpenRequest(connect, L"POST", reqPath.c_str(), nullptr,
                                                         WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
                                                         https ? WINHTTP_FLAG_SECURE : 0) : nullptr;
        if (request) {
            std::wstring headers = L"Content-Type: application/json\r\nAccept: application/json\r\n";
            std::string payload = body.dump();
            if (WinHttpSendRequest(request, headers.c_str(), static_cast<DWORD>(headers.size()),
                                   payload.data(), static_cast<DWORD>(payload.size()),
                                   static_cast<DWORD>(payload.size()), 0) &&
                WinHttpReceiveResponse(request, nullptr)) {
                DWORD code = 0, size = sizeof(code);
                WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                                    WINHTTP_HEADER_NAME_BY_INDEX, &code, &size, WINHTTP_NO_HEADER_INDEX);
                if (code >= 200 && code < 300) {
                    std::string resp;
                    DWORD readBytes = 0;
                    char buf[2048];
                    while (WinHttpReadData(request, buf, sizeof(buf), &readBytes) && readBytes > 0) {
                        resp.append(buf, readBytes);
                    }
                    try {
                        auto j = nlohmann::json::parse(resp);
                        const bool isDev = j.value("is_developer", false);
                        const bool isTester = j.value("is_tester", false);
                        SetRoles(isDev, isTester);
                    } catch (...) {}
                }
            }
            WinHttpCloseHandle(request);
        }
        if (connect) WinHttpCloseHandle(connect);
        WinHttpCloseHandle(session);
    }).detach();
}

AuthResult AuthClient::Login(const std::string& username, const std::string& password) {
    (void)password;
    AuthResult res;
    DeviceIdentity id = DeviceIdentityStore::Get();
    if (!username.empty()) id.playerName = username;
    if (id.serviceTag.empty()) id.serviceTag = id.deviceId.substr(0, 4);
    DeviceIdentityStore::Save(id);
    {
        std::lock_guard<std::mutex> lock(m_authMutex);
        m_isLoggedIn = true;
        m_username = id.playerName;
        m_sessionToken.clear();
        m_isDeveloper = kDevBuild;
        m_userId = static_cast<int>(std::hash<std::string>{}(id.deviceId) & 0x7FFFFFFF);
    }
    auto s = Settings::Get().Snapshot();
    s.accountUsername = m_username;
    s.accountToken.clear();
    s.accountUserId = m_userId;
    s.isDeveloper = m_isDeveloper;
    s.playerName = m_username;
    Settings::Get().Update(s);
    res.success = true;
    res.username = m_username;
    res.isDeveloper = m_isDeveloper;
    res.userId = m_userId;
    return res;
}

AuthResult AuthClient::Register(const std::string& username, const std::string& password) {
    return Login(username, password);
}

void AuthClient::Logout() {
    // Offline identity cannot be logged out; keep the local profile active.
    Initialize();
}

bool AuthClient::SetPlayerName(const std::string& name) {
    const std::string clean = name.empty() ? "" : name.substr(0, 16);
    if (clean.empty()) return false;
    DeviceIdentity id = DeviceIdentityStore::Get();
    id.playerName = clean;
    DeviceIdentityStore::Save(id);          // deviceId and developer flag are preserved
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

bool AuthClient::IsLoggedIn() const { return true; }

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
