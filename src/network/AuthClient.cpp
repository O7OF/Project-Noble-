#include "AuthClient.hpp"
#include "common/DeviceIdentity.hpp"
#include "network/SteamAuth.hpp"
#include "network/MasterServerClient.hpp"
#include "common/Logger.hpp"
#include "common/Settings.hpp"

#include <algorithm>
#include <functional>

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
        m_isDeveloper = id.developer;
        m_userId = static_cast<int>(std::hash<std::string>{}(id.deviceId) & 0x7FFFFFFF);
    }
    auto s = Settings::Get().Snapshot();
    s.accountUsername = id.playerName;
    s.accountToken.clear();
    s.accountUserId = GetUserId();
    s.isDeveloper = id.developer;
    if (s.playerName.empty()) s.playerName = id.playerName;
    Settings::Get().Update(s);
    NOBLE_INFO("AuthClient: offline identity '{}' (device {}, developer {})",
               id.playerName, id.deviceId, id.developer ? "YES" : "NO");
}

void AuthClient::RefreshDeveloperStatusAsync() {
    // No server to ask. The local identity file is authoritative.
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
        m_isDeveloper = id.developer;
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
        m_isDeveloper = steam.isDeveloper;
        m_userId = static_cast<int>(std::hash<std::string>{}(steam.steamId) & 0x7FFFFFFF);
    }
    auto s = Settings::Get().Snapshot();
    s.accountUsername = steam.username;
    s.accountToken = steam.token;
    s.accountUserId = m_userId;
    s.isDeveloper = steam.isDeveloper;
    s.playerName = steam.username;
    Settings::Get().Update(s);
    res.success = true; res.username = steam.username; res.token = steam.token;
    res.isDeveloper = steam.isDeveloper; res.userId = m_userId;
    NOBLE_INFO("AuthClient: Steam linked as '{}' ({})", steam.username, steam.steamId);
    return res;
}

bool AuthClient::IsSteamLinked() const {
    std::lock_guard<std::mutex> lock(m_authMutex);
    return m_steamLinked;
}

bool AuthClient::IsLoggedIn() const { return true; }

bool AuthClient::IsDeveloper() const { return m_isDeveloper.load(); }

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
