#include "MasterServerClient.hpp"
#include "UPnP.hpp"
#include "NetworkTransport.hpp"
#include "common/Logger.hpp"
#include "common/Version.hpp"

#include <windows.h>
#include <winhttp.h>
#include <nlohmann/json.hpp>

#include <chrono>

namespace noble {

namespace {

std::wstring Widen(const std::string& s) {
    if (s.empty()) return L"";
    const int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), nullptr, 0);
    std::wstring w(static_cast<size_t>(n), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), w.data(), n);
    return w;
}

struct HttpResult {
    bool ok = false;
    int status = 0;
    std::string body;
    std::string error;
};

// Blocking HTTP request (used only on the worker thread or during join setup). Short timeouts so a dead server
// does not freeze the worker loop.
HttpResult HttpRequest(const std::string& fullUrl,
                       const wchar_t* method,
                       const std::string& path,
                       const std::string& jsonBody,
                       const std::string& sessionToken = {}) {
    HttpResult result;
    if (fullUrl.empty()) {
        result.error = "no master server URL configured";
        return result;
    }

    std::wstring wideUrl = Widen(fullUrl);
    URL_COMPONENTS parts{};
    parts.dwStructSize = sizeof(parts);
    parts.dwSchemeLength = static_cast<DWORD>(-1);
    parts.dwHostNameLength = static_cast<DWORD>(-1);
    parts.dwUrlPathLength = static_cast<DWORD>(-1);
    if (!WinHttpCrackUrl(wideUrl.c_str(), static_cast<DWORD>(wideUrl.size()), 0, &parts)) {
        result.error = "invalid master server URL";
        return result;
    }

    const std::wstring host(parts.lpszHostName, parts.dwHostNameLength);
    const bool isHttps = (parts.nScheme == INTERNET_SCHEME_HTTPS);
    const INTERNET_PORT port = parts.nPort ? parts.nPort : (isHttps ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT);

    std::wstring requestPath = parts.dwUrlPathLength > 0 ? std::wstring(parts.lpszUrlPath, parts.dwUrlPathLength) : L"";
    while (!requestPath.empty() && requestPath.back() == L'/') requestPath.pop_back();
    std::wstring subPath = Widen(path);
    if (!subPath.empty() && subPath.front() != L'/') requestPath += L'/';
    requestPath += subPath;

    HINTERNET session = WinHttpOpen(L"ProjectNoble/0.1", WINHTTP_ACCESS_TYPE_NO_PROXY, nullptr, nullptr, 0);
    if (!session) {
        result.error = "WinHttpOpen failed";
        return result;
    }
    WinHttpSetTimeouts(session, 3000, 3000, 3000, 3000);

    HINTERNET connect = WinHttpConnect(session, host.c_str(), port, 0);
    HINTERNET request = nullptr;
    if (connect) {
        DWORD flags = isHttps ? WINHTTP_FLAG_SECURE : 0;
        request = WinHttpOpenRequest(connect, method, requestPath.c_str(), nullptr,
                                     WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    }
    if (request) {
        std::wstring headers = L"Content-Type: application/json\r\nAccept: application/json\r\n";
        if (!sessionToken.empty()) {
            headers += L"X-Session-Token: " + Widen(sessionToken) + L"\r\n";
        }

        const void* postData = jsonBody.empty() ? nullptr : jsonBody.data();
        const DWORD postSize = static_cast<DWORD>(jsonBody.size());
        if (WinHttpSendRequest(request, headers.c_str(), static_cast<DWORD>(headers.size()),
                               const_cast<void*>(postData), postSize, postSize, 0) &&
            WinHttpReceiveResponse(request, nullptr)) {
            DWORD statusCode = 0;
            DWORD statusSize = sizeof(statusCode);
            WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                                WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusSize, WINHTTP_NO_HEADER_INDEX);
            result.status = static_cast<int>(statusCode);

            std::string body;
            DWORD read = 0;
            char buffer[2048];
            while (WinHttpReadData(request, buffer, sizeof(buffer), &read) && read > 0) {
                body.append(buffer, read);
                if (body.size() > 256 * 1024) break;
            }
            result.body = std::move(body);
            result.ok = (statusCode >= 200 && statusCode < 300);
            if (!result.ok) {
                result.error = "master server replied " + std::to_string(statusCode);
                try {
                    auto ej = nlohmann::json::parse(result.body);
                    if (ej.contains("error") && ej["error"].is_string()) {
                        result.error += ": " + ej["error"].get<std::string>();
                    }
                } catch (...) {}
            }
        } else {
            result.error = "connection to master server failed";
        }
    } else {
        result.error = "could not open request";
    }

    if (request) WinHttpCloseHandle(request);
    if (connect) WinHttpCloseHandle(connect);
    WinHttpCloseHandle(session);
    return result;
}

std::string Str(const nlohmann::json& j, const char* key, size_t maxLen) {
    auto it = j.find(key);
    if (it == j.end() || !it->is_string()) return {};
    std::string s = it->get<std::string>();
    std::string out;
    for (char c : s) {
        if (static_cast<unsigned char>(c) < 0x20 || c == 0x7F) continue;  // shown in the menu
        out.push_back(c);
        if (out.size() >= maxLen) break;
    }
    return out;
}

int Int(const nlohmann::json& j, const char* key, int fallback, int lo, int hi) {
    auto it = j.find(key);
    if (it == j.end() || !it->is_number_integer()) return fallback;
    const int v = it->get<int>();
    return v < lo ? lo : (v > hi ? hi : v);
}

} // namespace

MasterServerClient& MasterServerClient::Get() {
    static MasterServerClient instance;
    return instance;
}

MasterServerClient::~MasterServerClient() {
    Shutdown();
}

void MasterServerClient::SetUrl(const std::string& url) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_url = url;
}

std::string MasterServerClient::Url() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_url;
}

void MasterServerClient::SetStatus(const std::string& status) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_status = status;
}

std::string MasterServerClient::Status() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_status;
}

std::vector<MasterSession> MasterServerClient::Sessions() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_sessions;
}

void MasterServerClient::EnsureThread() {
    if (m_thread.joinable()) return;
    m_stop = false;
    m_thread = std::thread(&MasterServerClient::WorkerLoop, this);
}

void MasterServerClient::RequestList() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_url.empty()) return;
    m_listRequested = true;
    EnsureThread();
    m_wake.notify_all();
}

void MasterServerClient::StartAdvertising(const MasterAdvert& advert) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_url.empty()) return;
    m_advert = advert;
    std::string pub = UPnP::Get().GetExternalIP();
    if (UPnP::Get().IsPublicIP(pub)) {
        m_advert.address = pub;
    } else if (m_advert.address.empty() || m_advert.address == "Unknown" || m_advert.address == "127.0.0.1") {
        std::string ip = UPnP::Get().GetInternalIP();
        if (!ip.empty() && ip != "Unknown") {
            m_advert.address = ip;
        }
    }
    m_advertInProgress = false;
    m_sessionId.clear();
    m_sessionToken.clear();
    m_unregisterRequested = false;
    m_nextHeartbeat = std::chrono::steady_clock::now();
    m_advertising = true;
    EnsureThread();
    m_wake.notify_all();
}

void MasterServerClient::UpdateAdvert(int playerCount, bool inProgress) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_advert.playerCount = playerCount;
    m_advertInProgress = inProgress;
}

void MasterServerClient::UpdateLobbyAdvert(const std::string& map, const std::string& mode, const std::string& variant, int diff, uint64_t skulls) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_advert.map = map;
    m_advert.mode = mode;
    m_advert.variant = variant;
    m_advert.difficulty = diff;
    m_advert.skulls = skulls;
}

std::string MasterServerClient::GetCurrentSessionId() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_sessionId;
}

std::string MasterServerClient::GetCurrentSessionToken() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_sessionToken;
}

void MasterServerClient::StopAdvertising() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_advertising) return;
    m_advertising = false;
    m_unregisterRequested = true;
    m_wake.notify_all();
}

void MasterServerClient::Shutdown() {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_advertising) {
            m_advertising = false;
            m_unregisterRequested = true;
        }
        m_stop = true;
        m_wake.notify_all();
    }
    if (m_thread.joinable()) m_thread.join();
}

void MasterServerClient::WorkerLoop() {
    std::unique_lock<std::mutex> lock(m_mutex);
    while (true) {
        m_wake.wait_for(lock, std::chrono::seconds(1), [this] {
            return m_stop || m_listRequested || m_unregisterRequested ||
                   (m_advertising && std::chrono::steady_clock::now() >= m_nextHeartbeat);
        });
        const bool doUnregister = m_unregisterRequested;
        const bool doList = m_listRequested;
        const bool doHeartbeat = m_advertising && std::chrono::steady_clock::now() >= m_nextHeartbeat;
        const bool stop = m_stop;
        const bool registered = !m_sessionId.empty();
        m_unregisterRequested = false;
        m_listRequested = false;
        lock.unlock();

        if (doUnregister) DoUnregister();
        if (doHeartbeat) {
            if (!registered) {
                DoRegister();
                // Immediately heartbeat after registering so the host receives relay_url and connects its WebSocket relay right away
                DoHeartbeat();
            } else {
                DoHeartbeat();
            }
        }
        if (doList) DoList();

        lock.lock();
        if (doHeartbeat) m_nextHeartbeat = std::chrono::steady_clock::now() + std::chrono::seconds(3);
        if (stop) break;
    }
}

void MasterServerClient::DoList() {
    const HttpResult r = HttpRequest(Url(), L"GET", "/api/sessions", {});
    if (!r.ok) {
        SetStatus(r.error);
        return;
    }
    std::vector<MasterSession> list;
    try {
        const nlohmann::json j = nlohmann::json::parse(r.body);
        if (j.is_array()) {
            for (const auto& item : j) {
                if (!item.is_object() || list.size() >= 500) continue;
                MasterSession s;
                s.id = Str(item, "id", 64);
                s.name = Str(item, "name", 64);
                s.address = Str(item, "address", 64);
                s.port = static_cast<uint16_t>(Int(item, "port", 11774, 1, 65535));
                s.playerCount = Int(item, "player_count", 0, 0, 64);
                s.maxPlayers = Int(item, "max_players", 16, 1, 64);
                s.map = Str(item, "map", 64);
                s.mode = Str(item, "mode", 32);
                s.inProgress = item.value("in_progress", false);
                s.hasPassword = item.value("has_password", false);
                s.isMod = item.value("is_mod", false);
                s.build = Str(item, "build", 32);
                if (!s.address.empty()) list.push_back(std::move(s));
            }
        }
    } catch (...) {
        SetStatus("master server sent an unreadable list");
        return;
    }
    std::lock_guard<std::mutex> lock(m_mutex);
    m_status = std::to_string(list.size()) + " game(s) on the master server";
    m_sessions = std::move(list);
}

void MasterServerClient::DoRegister() {
    MasterAdvert advert;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        advert = m_advert;
    }
    std::string pub = UPnP::Get().GetExternalIP();
    if (UPnP::Get().IsPublicIP(pub)) {
        advert.address = pub;
    } else if (advert.address.empty() || advert.address == "Unknown" || advert.address == "127.0.0.1") {
        advert.address = UPnP::Get().GetInternalIP();
    }

    nlohmann::json body = {
        {"name", advert.name}, {"port", advert.port}, {"player_count", advert.playerCount},
        {"max_players", advert.maxPlayers}, {"map", advert.map}, {"mode", advert.mode},
        {"has_password", advert.hasPassword}, {"is_mod", advert.isMod}, {"address", advert.address}, {"build", std::string(VERSION_STRING.substr(0, VERSION_STRING.find(' ')))},
    };
    const HttpResult r = HttpRequest(Url(), L"POST", "/api/sessions", body.dump());
    if (!r.ok) {
        SetStatus("could not list your game: " + r.error);
        return;
    }
    try {
        const nlohmann::json j = nlohmann::json::parse(r.body);
        std::lock_guard<std::mutex> lock(m_mutex);
        m_sessionId = Str(j, "session_id", 64);
        m_sessionToken = Str(j, "token", 128);
        m_status = "your game is listed on the master server";
        NOBLE_INFO("MasterServer: registered session {}", m_sessionId);
    } catch (...) {
        SetStatus("master server sent an unreadable reply");
    }
}

void MasterServerClient::DoHeartbeat() {
    std::string id, token;
    nlohmann::json body;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        id = m_sessionId;
        token = m_sessionToken;
        body = {
            {"session_id", id},
            {"player_count", m_advert.playerCount},
            {"in_progress", m_advertInProgress},
            {"map", m_advert.map},
            {"mode", m_advert.mode},
            {"variant", m_advert.variant},
            {"difficulty", m_advert.difficulty},
            {"skulls", std::to_string(m_advert.skulls)}
        };
    }
    if (id.empty() || token.empty()) return;
    const HttpResult r = HttpRequest(Url(), L"PUT", "/api/sessions/heartbeat", body.dump(), token);
    if (r.ok) {
        try {
            auto j = nlohmann::json::parse(r.body);
            if (j.contains("pending_joins") && j["pending_joins"].is_array()) {
                for (const auto& pj : j["pending_joins"]) {
                    std::string ip = pj.value("ip", "");
                    uint16_t port = static_cast<uint16_t>(pj.value("port", 11774));
                    std::string midStr = pj.value("machine_id", "");
                    uint64_t mid = 0;
                    try { mid = std::stoull(midStr); } catch (...) {}
                    if (!ip.empty()) {
                        NetworkTransport::Get().AddPeer(mid, ip, port);
                    }
                }
            }
            if (j.contains("relay_url") && j["relay_url"].is_string()) {
                std::string relayUrl = j["relay_url"].get<std::string>();
                NetworkTransport::Get().StartRelay(relayUrl, id, true, NetworkTransport::Get().GetLocalMachineId());
            }
        } catch (...) {}
    } else if (r.status == 404) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_sessionId.clear();
        m_sessionToken.clear();
    } else {
        SetStatus("heartbeat failed: " + r.error);
    }
}

void MasterServerClient::DoUnregister() {
    std::string id, token;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        id = m_sessionId;
        token = m_sessionToken;
        m_sessionId.clear();
        m_sessionToken.clear();
    }
    if (id.empty()) return;
    HttpRequest(Url(), L"DELETE", "/api/sessions/" + id, {}, token);
    SetStatus("your game was removed from the master server");
}

MasterJoinInfo MasterServerClient::JoinSessionSync(const std::string& sessionId,
                                                   uint64_t guestMachineId,
                                                   uint16_t guestPort,
                                                   const std::string& fallbackHostIp,
                                                   uint16_t fallbackHostPort) {
    MasterJoinInfo info;
    std::string resolvedId = sessionId;

    // If no sessionId was provided (e.g. Direct Connect via IP:port), attempt to resolve it from /api/sessions
    if (resolvedId.empty() && !fallbackHostIp.empty() && Enabled()) {
        const HttpResult listRes = HttpRequest(Url(), L"GET", "/api/sessions", {});
        if (listRes.ok) {
            try {
                const nlohmann::json arr = nlohmann::json::parse(listRes.body);
                if (arr.is_array()) {
                    for (const auto& item : arr) {
                        if (!item.is_object()) continue;
                        const std::string addr = Str(item, "address", 64);
                        const uint16_t port = static_cast<uint16_t>(Int(item, "port", 11774, 1, 65535));
                        if (addr == fallbackHostIp && port == fallbackHostPort) {
                            resolvedId = Str(item, "id", 64);
                            break;
                        }
                    }
                    // Fallback: match by IP if port was NAT-mapped
                    if (resolvedId.empty()) {
                        for (const auto& item : arr) {
                            if (!item.is_object()) continue;
                            if (Str(item, "address", 64) == fallbackHostIp) {
                                resolvedId = Str(item, "id", 64);
                                break;
                            }
                        }
                    }
                }
            } catch (...) {}
        }
    }

    if (resolvedId.empty()) {
        info.error = "Session ID is empty";
        return info;
    }

    nlohmann::json body = {
        { "guest_port", guestPort },
        { "guest_machine_id", std::to_string(guestMachineId) },
        { "guest_address", UPnP::Get().GetExternalIP() }
    };
    const HttpResult r = HttpRequest(Url(), L"POST", "/api/sessions/" + resolvedId + "/join", body.dump());
    if (!r.ok) {
        info.error = r.error;
        return info;
    }
    try {
        auto j = nlohmann::json::parse(r.body);
        info.ok = true;
        info.sessionId = resolvedId;
        info.hostIp = j.value("host_ip", "");
        info.hostPort = static_cast<uint16_t>(j.value("host_port", 11774));
        info.relayUrl = j.value("relay_url", "");
        NOBLE_INFO("MasterServer: joined session {} (relay: {})", resolvedId, info.relayUrl);
        return info;
    } catch (...) {
        info.error = "Unreadable join response";
        return info;
    }
}


LobbyState MasterServerClient::FetchLobbyStateSync(const std::string& sessionId) {
    LobbyState state;
    if (sessionId.empty() || !Enabled()) return state;

    const HttpResult r = HttpRequest(Url(), L"GET", "/api/sessions/" + sessionId + "/lobby", {});
    if (!r.ok) return state;

    try {
        const auto j = nlohmann::json::parse(r.body);
        state.ok = true;
        state.sessionId = j.value("session_id", sessionId);
        state.name = j.value("name", "Reach Lobby");
        state.map = j.value("map", "20_sword_slayer");
        state.mode = j.value("mode", "multiplayer");
        state.variant = j.value("variant", "");
        state.difficulty = j.value("difficulty", 1);
        try { state.skulls = std::stoull(j.value("skulls", "0")); } catch (...) {}
        state.inProgress = j.value("in_progress", false);
        state.playerCount = j.value("player_count", 1);
        state.maxPlayers = j.value("max_players", 16);
        state.hostIp = j.value("host_ip", "");
        state.hostPort = static_cast<uint16_t>(j.value("host_port", 11774));

        if (j.contains("roster") && j["roster"].is_array()) {
            for (const auto& item : j["roster"]) {
                LobbyPlayer p;
                p.peerId = item.value("peerId", 1ULL);
                p.machineId = item.value("machineId", "");
                p.username = item.value("username", "Spartan");
                p.serviceTag = item.value("serviceTag", "117");
                p.isHost = item.value("isHost", false);
                p.isDev = item.value("isDev", false);
                p.ping = item.value("ping", 0);
                state.roster.push_back(p);
            }
        }

        if (j.contains("launch_data") && j["launch_data"].is_object()) {
            const auto& ld = j["launch_data"];
            state.launchTriggered = true;
            state.launchMap = ld.value("map", state.map);
            state.launchMode = ld.value("mode", state.mode);
            state.launchVariant = ld.value("variant", state.variant);
            state.launchDifficulty = ld.value("difficulty", state.difficulty);
            try { state.launchSkulls = std::stoull(ld.value("skulls", "0")); } catch (...) {}
        }
    } catch (...) {}

    return state;
}

LobbyState MasterServerClient::JoinLobbySync(const std::string& sessionId,
                                             uint64_t peerId,
                                             const std::string& username,
                                             bool isDev,
                                             const std::string& serviceTag,
                                             uint16_t localPort) {
    LobbyState state;
    if (sessionId.empty() || !Enabled()) return state;

    nlohmann::json body = {
        { "peer_id", peerId },
        { "player_id", peerId },
        { "username", username },
        { "is_dev", isDev },
        { "service_tag", serviceTag },
        { "machine_id", std::to_string(NetworkTransport::Get().GetLocalMachineId()) },
        { "guest_port", localPort },
        { "ping", 25 }
    };

    const HttpResult r = HttpRequest(Url(), L"POST", "/api/sessions/" + sessionId + "/lobby/join", body.dump());
    if (!r.ok) return state;

    try {
        const auto j = nlohmann::json::parse(r.body);
        state.ok = true;
        state.sessionId = j.value("session_id", sessionId);
        state.hostIp = j.value("host_ip", "");
        state.hostPort = static_cast<uint16_t>(j.value("host_port", 11774));
        state.map = j.value("map", "20_sword_slayer");
        state.mode = j.value("mode", "multiplayer");
        state.variant = j.value("variant", "");
        state.difficulty = j.value("difficulty", 1);
        state.inProgress = j.value("in_progress", false);

        if (j.contains("roster") && j["roster"].is_array()) {
            for (const auto& item : j["roster"]) {
                LobbyPlayer p;
                p.peerId = item.value("peerId", 1ULL);
                p.machineId = item.value("machineId", "");
                p.username = item.value("username", "Spartan");
                p.serviceTag = item.value("serviceTag", "117");
                p.isHost = item.value("isHost", false);
                p.isDev = item.value("isDev", false);
                p.ping = item.value("ping", 0);
                state.roster.push_back(p);
            }
        }

        std::string relayUrl = j.value("relay_url", "");
        if (!relayUrl.empty()) {
            NetworkTransport::Get().StartRelay(relayUrl, sessionId, false, NetworkTransport::Get().GetLocalMachineId());
        }
    } catch (...) {}

    return state;
}

bool MasterServerClient::LeaveLobbySync(const std::string& sessionId, uint64_t peerId) {
    if (sessionId.empty() || !Enabled()) return false;
    nlohmann::json body = {
        { "peer_id", peerId }
    };
    const HttpResult r = HttpRequest(Url(), L"POST", "/api/sessions/" + sessionId + "/lobby/leave", body.dump());
    return r.ok;
}

bool MasterServerClient::LaunchLobbySync(const std::string& sessionId,
                                         const std::string& map,
                                         const std::string& mode,
                                         const std::string& variant,
                                         int difficulty,
                                         uint64_t skulls) {
    std::string token;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        token = m_sessionToken;
        m_advertInProgress = true;
    }
    if (sessionId.empty() || token.empty() || !Enabled()) return false;

    nlohmann::json body = {
        { "map", map },
        { "mode", mode },
        { "variant", variant },
        { "difficulty", difficulty },
        { "skulls", std::to_string(skulls) }
    };

    const HttpResult r = HttpRequest(Url(), L"POST", "/api/sessions/" + sessionId + "/lobby/launch", body.dump(), token);
    return r.ok;
}

} // namespace noble
