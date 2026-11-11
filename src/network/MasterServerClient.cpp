#include "MasterServerClient.hpp"
#include "UPnP.hpp"
#include "NetworkTransport.hpp"
#include "common/DeviceIdentity.hpp"
#include "common/Logger.hpp"
#include "common/Version.hpp"
#include "common/BuildIdentity.hpp"
#include "career/Career.hpp"
#include "host/EngineLimits.hpp"

#include <windows.h>
#include <winhttp.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <filesystem>
#include <set>
#include <utility>

namespace noble {

namespace {

// The id a menu (not yet in a match) uses in a lobby. The menu's NetworkTransport isn't initialised, so
// GetLocalMachineId() was the fixed host id 0x484F535400000001 for every guest: the relay keys guests by that
// id, so a second guest replaced the first and only one of them got lobby events (2026-10-29 review).
uint64_t LobbyMachineId() {
    if (NetworkTransport::Get().IsActive()) return NetworkTransport::Get().GetLocalMachineId();
    static const uint64_t id = [] {
        LARGE_INTEGER t{};
        QueryPerformanceCounter(&t);
        uint64_t x = static_cast<uint64_t>(t.QuadPart) ^ (static_cast<uint64_t>(GetCurrentProcessId()) << 32) ^ 0x9E3779B97F4A7C15ULL;
        x ^= x >> 33; x *= 0xFF51AFD7ED558CCDULL; x ^= x >> 33;
        if (x == NetworkTransport::kDefaultHostMachineId) x ^= 0xFFFFFFFF00000000ULL;
        return x | 1ULL;
    }();
    return id;
}

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
        // Closed-source build identity. The cluster rejects requests that carry the
        // open-source build (or no build at all) with 403 client_outdated.
        headers += L"X-Noble-Build: " + Widen(std::string(kBuildTag)) + L"\r\n";
        headers += L"X-Noble-Protocol: " + Widen(std::to_string(kClientProtocol)) + L"\r\n";
        const DeviceIdentity& devId = DeviceIdentityStore::Get();
        if (!devId.deviceId.empty()) {
            headers += L"X-Device-Id: " + Widen(devId.deviceId) + L"\r\n";
        }
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

int CountLocalKnownPlayers() {
    std::set<std::string> knownNames;
    const DeviceIdentity& id = DeviceIdentityStore::Get();
    if (!id.playerName.empty()) {
        std::string lower = id.playerName;
        for (char& c : lower) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        knownNames.insert(lower);
    }
    auto scanDir = [&](const std::filesystem::path& dir) {
        std::error_code ec;
        if (!std::filesystem::exists(dir, ec)) return;
        for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
            if (!entry.is_regular_file(ec) || entry.path().extension() != ".json") continue;
            std::string stem = entry.path().stem().string();
            for (char& c : stem) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            if (!stem.empty()) knownNames.insert(stem);
        }
    };
    scanDir(DeviceIdentityStore::DataDir() / "players");
    scanDir(DeviceIdentityStore::UserDir() / "players");
    return std::max(1, static_cast<int>(knownNames.size()));
}

} // namespace

uint64_t MasterServerClient::GetLobbyMachineId() {
    return LobbyMachineId();
}

MasterServerClient& MasterServerClient::Get() {
    static MasterServerClient instance;
    return instance;
}

MasterServerClient::~MasterServerClient() {
    Shutdown();
}

void MasterServerClient::SetUrl(const std::string& url) {
    std::lock_guard<std::mutex> lock(m_mutex);
    // Accept URLs copied from a browser (often with a trailing slash) and normalise before every
    // request; m_url + "/api/..." with a double slash 404s on Express.
    std::string normalized = url;
    while (!normalized.empty() && std::isspace(static_cast<unsigned char>(normalized.front()))) normalized.erase(normalized.begin());
    while (!normalized.empty() && std::isspace(static_cast<unsigned char>(normalized.back()))) normalized.pop_back();
    while (!normalized.empty() && normalized.back() == '/') normalized.pop_back();
    m_url = std::move(normalized);
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

void MasterServerClient::RequestOnlineCount() {
    if (m_totalPlayers.load() == 0) {
        m_totalPlayers.store(CountLocalKnownPlayers());
    }
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_url.empty()) return;
    m_onlineRequested = true;
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
    m_advertInProgress = advert.inProgress;
    if (!advert.id.empty()) {
        m_sessionId = advert.id;
        m_sessionToken = advert.token;
    } else {
        m_sessionId.clear();
        m_sessionToken.clear();
    }
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

void MasterServerClient::Shutdown(bool keepSession) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_advertising) {
            m_advertising = false;
            // Handing the session to the match process: deleting it here made the match's first heartbeat
            // get 404 and register a brand-new session, while the guests were sent to the old one (2026-10-29).
            m_unregisterRequested = !keepSession;
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
            return m_stop || m_listRequested || m_onlineRequested || m_unregisterRequested ||
                   (m_advertising && std::chrono::steady_clock::now() >= m_nextHeartbeat);
        });
        const bool doUnregister = m_unregisterRequested;
        const bool doList = m_listRequested;
        const bool doOnline = m_onlineRequested;
        const bool doHeartbeat = m_advertising && std::chrono::steady_clock::now() >= m_nextHeartbeat;
        const bool stop = m_stop;
        const bool registered = !m_sessionId.empty();
        m_unregisterRequested = false;
        m_listRequested = false;
        m_onlineRequested = false;
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
        if (doOnline) DoOnline();

        lock.lock();
        if (doHeartbeat) m_nextHeartbeat = std::chrono::steady_clock::now() + std::chrono::seconds(10);
        if (stop) break;
    }
}

void MasterServerClient::DoOnline() {
    const DeviceIdentity& id = DeviceIdentityStore::Get();
    nlohmann::json presenceBody = {
        {"deviceId", id.deviceId},
        {"username", id.playerName},
        {"serviceTag", id.serviceTag}
    };
    HttpResult r = HttpRequest(Url(), L"POST", "/api/presence", presenceBody.dump());
    if (!r.ok) {
        r = HttpRequest(Url(), L"GET", "/api/players/online", {});
    }
    if (!r.ok) {
        // Older deployments do not have the players endpoint; health carries the same counters.
        r = HttpRequest(Url(), L"GET", "/api/health", {});
    }
    if (!r.ok) return;
    try {
        const nlohmann::json j = nlohmann::json::parse(r.body);
        const int online = j.value("online", j.value("activeSessions", 0));
        int total = j.value("totalPlayers", j.value("registeredPlayers", 0));
        const int servers = j.value("servers", j.value("masterSessions", 0));

        // Master server cluster nodes (East, West, UK) register internal accounts for peer federation;
        // ensure master server infrastructure nodes are never counted as total players.
        int masterNodeCount = 0;
        if (j.contains("cluster") && j["cluster"].is_object()) {
            const auto& cluster = j["cluster"];
            masterNodeCount = 1 + cluster.value("peerCount", 0);
            if (cluster.contains("nodes") && cluster["nodes"].is_array()) {
                masterNodeCount = std::max(masterNodeCount, 1 + static_cast<int>(cluster["nodes"].size()));
            }
        } else if (j.contains("peerRegistries") && j["peerRegistries"].is_array()) {
            masterNodeCount = 1 + static_cast<int>(j["peerRegistries"].size());
        } else {
            masterNodeCount = 3;
        }
        total = std::max(0, total - masterNodeCount);

        m_onlinePlayers.store(std::max(0, online));
        if (total <= 1) {
            const HttpResult lb = HttpRequest(Url(), L"GET", "/api/leaderboard?limit=250", {});
            if (lb.ok) {
                try {
                    const auto lbJson = nlohmann::json::parse(lb.body);
                    if (lbJson.is_array() && static_cast<int>(lbJson.size()) > total) {
                        total = static_cast<int>(lbJson.size());
                    }
                } catch (...) {}
            }
        }
        const int localKnown = CountLocalKnownPlayers();
        if (total > 0 || localKnown > 0) {
            m_totalPlayers.store(std::max({ total, localKnown, m_totalPlayers.load() }));
        }
        m_onlineServers.store(std::max(0, servers));
    } catch (...) {
    }
}

void MasterServerClient::DoList() {
    HttpResult r = HttpRequest(Url(), L"GET", "/api/browser", {});
    if (!r.ok) r = HttpRequest(Url(), L"GET", "/api/sessions", {});
    if (!r.ok) {
        SetStatus(r.error);
        return;
    }
    std::vector<MasterSession> list;
    int online = -1;
    int total = -1;
    try {
        const nlohmann::json j = nlohmann::json::parse(r.body);
        const nlohmann::json* items = &j;
        if (j.is_object()) {
            if (j.contains("online")) online = j.value("online", 0);
            if (j.contains("totalPlayers")) total = j.value("totalPlayers", 0);
            else if (j.contains("registeredPlayers")) total = j.value("registeredPlayers", 0);
            if (total >= 0) {
                int masterNodeCount = 0;
                if (j.contains("peerRegistries") && j["peerRegistries"].is_array()) {
                    masterNodeCount = 1 + static_cast<int>(j["peerRegistries"].size());
                } else if (j.contains("cluster") && j["cluster"].is_object()) {
                    masterNodeCount = 1 + j["cluster"].value("peerCount", 0);
                } else {
                    masterNodeCount = 3;
                }
                total = std::max(0, total - masterNodeCount);
            }
            if (j.contains("sessions") && j["sessions"].is_array()) items = &j["sessions"];
        }
        if (items->is_array()) {
            for (const auto& item : *items) {
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
                s.modId = Str(item, "mod_id", 32);
                s.modTitle = Str(item, "mod_title", 96);
                s.isPrivate = item.value("is_private", false);
                s.lanAddress = Str(item, "lan_address", 64);
                s.build = Str(item, "build", 32);
                if (!s.address.empty()) list.push_back(std::move(s));
            }
        }
    } catch (...) {
        SetStatus("master server sent an unreadable list");
        return;
    }
    if (online >= 0) m_onlinePlayers.store(std::max(0, online));
    const int localKnown = CountLocalKnownPlayers();
    if (total > 0 || localKnown > 0) {
        m_totalPlayers.store(std::max({ total, localKnown, m_totalPlayers.load() }));
    }
    std::lock_guard<std::mutex> lock(m_mutex);
    m_status = std::to_string(list.size()) + " game(s) on the master server";
    m_sessions = std::move(list);
}

void MasterServerClient::DoRegister() {
    std::string existingId;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        existingId = m_sessionId;
    }
    if (!existingId.empty()) {
        NOBLE_INFO("MasterServer: Continuing existing session {}", existingId);
        DoHeartbeat();
        return;
    }
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
        {"has_password", advert.hasPassword}, {"is_mod", advert.isMod}, {"mod_id", advert.modId}, {"mod_title", advert.modTitle},
        {"is_private", advert.isPrivate}, {"address", advert.address}, {"lan_address", UPnP::Get().GetInternalIP()}, {"build", std::string(kBuildTag)}, {"protocol", kClientProtocol},
        {"variant", advert.variant}, {"difficulty", advert.difficulty}, {"skulls", std::to_string(advert.skulls)}, {"in_progress", advert.inProgress},
        {"host_peer_id", advert.hostPeerId == 0 ? 1ULL : advert.hostPeerId}, {"player_id", advert.hostPeerId == 0 ? 1ULL : advert.hostPeerId},
        {"host_machine_id", advert.hostMachineId.empty() ? "0x484F535400000001" : advert.hostMachineId},
        {"host_username", advert.hostUsername.empty() ? "Host" : advert.hostUsername}, {"player_name", advert.hostUsername.empty() ? "Host" : advert.hostUsername},
        {"host_is_dev", advert.hostIsDev}, {"host_service_tag", advert.hostServiceTag.empty() ? "117" : advert.hostServiceTag},
        {"host_rank_index", advert.hostRankIndex}, {"host_emblem_id", advert.hostEmblemId},
      {"host_emblem_background_id", advert.hostEmblemBackgroundId},
        {"host_nameplate_id", advert.hostNameplateId}, {"host_team", advert.hostTeam},
        {"host_games_played", advert.hostGamesPlayed}, {"host_kills", advert.hostKills},
        {"host_deaths", advert.hostDeaths}, {"host_challenges_done", advert.hostChallengesDone},
        {"host_commendations", advert.hostCommendations}, {"host_armory_owned", advert.hostArmoryOwned},
        {"limits_id", noble::limits::LocalNetworkId()},
    };
    if (!advert.inProgress) {
        body["roster"] = nlohmann::json::array({
            {
                {"peerId", advert.hostPeerId == 0 ? 1ULL : advert.hostPeerId},
                {"machineId", advert.hostMachineId.empty() ? "0x484F535400000001" : advert.hostMachineId},
                {"username", advert.hostUsername.empty() ? "Host" : advert.hostUsername},
                {"isHost", true},
                {"isDev", advert.hostIsDev},
                {"serviceTag", advert.hostServiceTag.empty() ? "117" : advert.hostServiceTag},
                {"rankIndex", advert.hostRankIndex}, {"emblemId", advert.hostEmblemId},
        {"emblemBackgroundId", advert.hostEmblemBackgroundId},
                {"nameplateId", advert.hostNameplateId}, {"team", advert.hostTeam},
                {"gamesPlayed", advert.hostGamesPlayed}, {"kills", advert.hostKills},
                {"deaths", advert.hostDeaths}, {"challengesDone", advert.hostChallengesDone},
                {"commendations", advert.hostCommendations}, {"armoryOwned", advert.hostArmoryOwned},
                {"ping", 0}
            }
        });
    }
    const HttpResult r = HttpRequest(Url(), L"POST", "/api/sessions", body.dump());
    if (!r.ok) {
        SetStatus("could not list your game: " + r.error);
        return;
    }
    try {
        const nlohmann::json j = nlohmann::json::parse(r.body);
        std::string sid;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_sessionId = Str(j, "session_id", 64);
            m_sessionToken = Str(j, "token", 128);
            m_status = "your game is listed on the master server";
            sid = m_sessionId;
        }
        NOBLE_INFO("MasterServer: registered session {}", sid);
        if (j.contains("relay_url") && j["relay_url"].is_string()) {
            std::string relayUrl = j["relay_url"].get<std::string>();
            NetworkTransport::Get().StartRelay(relayUrl, sid, true, NetworkTransport::Get().GetLocalMachineId());
        }
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
            {"build", std::string(kBuildTag)},
            {"protocol", kClientProtocol},
            {"player_count", m_advert.playerCount},
            {"in_progress", m_advertInProgress},
            {"is_private", m_advert.isPrivate},
            {"lan_address", UPnP::Get().GetInternalIP()},
            {"map", m_advert.map},
            {"mode", m_advert.mode},
            {"variant", m_advert.variant},
            {"difficulty", m_advert.difficulty},
            {"skulls", std::to_string(m_advert.skulls)},
            {"host_peer_id", m_advert.hostPeerId},
            {"host_machine_id", m_advert.hostMachineId},
            {"host_username", m_advert.hostUsername},
            {"host_is_dev", m_advert.hostIsDev},
            {"host_service_tag", m_advert.hostServiceTag},
            {"host_rank_index", m_advert.hostRankIndex},
            {"host_emblem_id", m_advert.hostEmblemId},
            {"host_emblem_background_id", m_advert.hostEmblemBackgroundId},
            {"host_nameplate_id", m_advert.hostNameplateId},
            {"host_team", m_advert.hostTeam},
            {"limits_id", noble::limits::LocalNetworkId()}
        };
        // Always send the mod fields, empty when this host has no mod. Omitting them made the master server keep
        // the previous session's mod id (it defaults to the existing value), so a host that switched from a modded
        // map back to a stock one kept advertising the old Workshop item - guests then looked for that mod's maps
        // and joined with the wrong map file.
        body["mod_id"] = m_advert.modId;
        body["mod_title"] = m_advert.modTitle;
    }
    const auto peerPings = NetworkTransport::Get().GetAllPeerPingsMs();
    if (!peerPings.empty()) {
        nlohmann::json pingsObj = nlohmann::json::object();
        for (const auto& kv : peerPings) {
            pingsObj[std::to_string(kv.first)] = kv.second;
        }
        body["peer_pings"] = std::move(pingsObj);
    }
    if (id.empty() || token.empty()) return;
    const HttpResult r = HttpRequest(Url(), L"PUT", "/api/sessions/heartbeat", body.dump(), token);
    if (r.ok) {
        try {
            auto j = nlohmann::json::parse(r.body);
            if (j.contains("pending_joins") && j["pending_joins"].is_array()) {
                for (const auto& pj : j["pending_joins"]) {
                    std::string ip = pj.value("ip", "");
                    std::string lanIp = pj.value("lan_ip", "");
                    uint16_t port = static_cast<uint16_t>(pj.value("port", 11774));
                    std::string midStr = pj.value("machine_id", "");
                    uint64_t mid = 0;
                    try { mid = std::stoull(midStr); } catch (...) {}
                    if (!ip.empty()) {
                        NetworkTransport::Get().AddPeer(mid, ip, port);
                    }
                    if (!lanIp.empty() && lanIp != ip) {
                        NetworkTransport::Get().AddPeer(0, lanIp, port);
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
        { "build", std::string(kBuildTag) },
        { "protocol", kClientProtocol },
        { "guest_port", guestPort },
        { "guest_machine_id", std::to_string(guestMachineId) },
        { "guest_address", UPnP::Get().GetExternalIP() },
        { "guest_lan_address", UPnP::Get().GetInternalIP() }
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
        info.hostLanIp = j.value("host_lan_ip", "");
        info.isLan = j.value("is_lan", false);
        if (!info.hostLanIp.empty()) {
            NetworkTransport::Get().SetRemoteHostLanAddress(info.hostLanIp, info.hostPort);
        }
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
    const std::string& resolvedId = sessionId;

    const HttpResult r = HttpRequest(Url(), L"GET", "/api/sessions/" + sessionId + "/lobby", {});
    state.httpStatus = r.status;
    if (!r.ok) return state;

    try {
        const auto j = nlohmann::json::parse(r.body);
        state.ok = true;
        state.sessionId = j.value("session_id", resolvedId);
        state.name = j.value("name", "Reach Lobby");
        state.map = j.value("map", "20_sword_slayer");
        state.mode = j.value("mode", "multiplayer");
        state.variant = j.value("variant", "");
        state.modId = Str(j, "mod_id", 32);
        state.modTitle = Str(j, "mod_title", 96);
        state.difficulty = j.value("difficulty", 1);
        try { state.skulls = std::stoull(j.value("skulls", "0")); } catch (...) {}
        state.inProgress = j.value("in_progress", false);
        state.playerCount = j.value("player_count", 1);
        state.maxPlayers = j.value("max_players", 16);
        state.hostIp = j.value("host_ip", "");
        state.hostPort = static_cast<uint16_t>(j.value("host_port", 11774));
        state.hostLanIp = j.value("host_lan_ip", "");
        state.isLan = j.value("is_lan", false);
        state.isClosed = j.value("is_closed", false);
        state.closeReason = j.value("close_reason", "");
        if (j.contains("limits_id") && j["limits_id"].is_number_integer()) state.limitsId = j["limits_id"].get<int64_t>();
        if (j.contains("vote") && j["vote"].is_object()) {
            const auto& vote = j["vote"];
            state.voteId = vote.value("id", std::string());
            state.voteMsLeft = vote.value("ms_left", 0);
            if (vote.contains("options") && vote["options"].is_array()) {
                for (const auto& option : vote["options"]) if (option.is_string()) state.voteOptions.push_back(option.get<std::string>());
            }
            if (vote.contains("counts") && vote["counts"].is_array()) {
                for (const auto& count : vote["counts"]) if (count.is_number_integer()) state.voteCounts.push_back(count.get<int>());
            }
        }

        if (j.contains("roster") && j["roster"].is_array()) {
            for (const auto& item : j["roster"]) {
                LobbyPlayer p;
                p.peerId = item.value("peerId", 1ULL);
                p.machineId = item.value("machineId", "");
                if (item.contains("username") && item["username"].is_string() && !item["username"].get<std::string>().empty())
                    p.username = item["username"].get<std::string>();
                else if (item.contains("player_name") && item["player_name"].is_string() && !item["player_name"].get<std::string>().empty())
                    p.username = item["player_name"].get<std::string>();
                else if (item.contains("name") && item["name"].is_string() && !item["name"].get<std::string>().empty())
                    p.username = item["name"].get<std::string>();
                else
                    p.username = item.value("isHost", false) ? "Host" : "Spartan";
                p.serviceTag = item.value("serviceTag", "117");
            p.rankIndex = item.value("rankIndex", -1);
            p.emblemId = item.value("emblemId", 0);
  p.emblemBackgroundId = item.value("emblemBackgroundId", 0);
            p.nameplateId = item.value("nameplateId", 0);
            p.team = item.value("team", 0);
            p.gamesPlayed = item.value("gamesPlayed", item.value("games_played", 0));
            p.kills = item.value("kills", 0);
            p.deaths = item.value("deaths", 0);
            p.challengesDone = item.value("challengesDone", item.value("challenges_done", 0));
            p.commendations = item.value("commendations", 0);
            p.armoryOwned = item.value("armoryOwned", item.value("armory_owned", 0));
                p.isHost = item.value("isHost", false);
                p.isDev = item.value("isDev", false);
                p.ping = item.value("ping", 0);
                if (!p.isHost && !p.machineId.empty()) {
                    try {
                        uint64_t mid = std::stoull(p.machineId, nullptr, 0);
                        int livePing = NetworkTransport::Get().GetPeerPingMs(mid);
                        if (livePing > 0) p.ping = livePing;
                    } catch (...) {}
                } else if (p.isHost) {
                    int liveHostPing = NetworkTransport::Get().GetPeerPingMs(0);
                    if (liveHostPing > 0 && (p.ping <= 0 || p.ping == 25)) p.ping = liveHostPing;
                }
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
            state.launchMapOverride = ld.value("map_override", "");
            state.launchMapVariant = ld.value("map_variant", "");
            state.launchModFolder = ld.value("mod_folder", "");
            state.launchModId = ld.value("mod_id", state.modId);
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
                                             int rankIndex,
                                             int emblemId,
                                             int emblemBackgroundId,
                                             int nameplateId,
                                             int team,
                                             uint16_t localPort,
                                             const std::string& fallbackHostIp,
                                             uint16_t fallbackHostPort) {
    LobbyState state;

    if (!Enabled()) return state;
    std::string resolvedId = sessionId;
    if (resolvedId.empty() && !fallbackHostIp.empty()) {
        const HttpResult listRes = HttpRequest(Url(), L"GET", "/api/sessions", {});
        if (listRes.ok) {
            try {
                const nlohmann::json arr = nlohmann::json::parse(listRes.body);
                if (arr.is_array()) {
                    for (const auto& item : arr) {
                        if (!item.is_object()) continue;
                        const std::string addr = Str(item, "address", 64);
                        const std::string lanAddr = Str(item, "lan_address", 64);
                        const uint16_t port = static_cast<uint16_t>(Int(item, "port", 11774, 1, 65535));
                        if ((addr == fallbackHostIp || (!lanAddr.empty() && lanAddr == fallbackHostIp)) && port == fallbackHostPort) {
                            resolvedId = Str(item, "id", 64);
                            break;
                        }
                    }
                }
            } catch (...) {}
        }
    }
    if (resolvedId.empty()) return state;

    const int measuredPing = NetworkTransport::Get().GetPeerPingMs(0);
    nlohmann::json body = {
        { "build", std::string(kBuildTag) },
        { "protocol", kClientProtocol },
        { "peer_id", peerId },
        { "player_id", peerId },
        { "username", username },
        { "is_dev", isDev },
        { "service_tag", serviceTag },
        { "rank_index", rankIndex },
        { "emblem_id", emblemId },
        { "emblem_background_id", emblemBackgroundId },
        { "nameplate_id", nameplateId },
        { "team", team },
        { "games_played", Career::GetRecord().totals.gamesPlayed },
        { "kills", Career::GetRecord().totals.kills },
        { "deaths", Career::GetRecord().totals.deaths },
        { "challenges_done", [&]() { int d = 0; for (const auto& c : Career::Challenges()) if (c.done) ++d; return d; }() },
        { "commendations", [&]() {
            int t = 0, tm = 0;
            for (const auto& c : Career::Commendations()) { if (c.tracked) { t += c.tier; tm += 6; } }
            return tm > 0 ? (t * 100 / tm) : 0;
        }() },
        { "armory_owned", static_cast<int>(Career::GetRecord().totals.armoryOwned.size()) },
        { "machine_id", std::to_string(LobbyMachineId()) },
        { "guest_port", localPort },
        { "guest_lan_address", UPnP::Get().GetInternalIP() },
        { "ping", measuredPing > 0 ? measuredPing : 25 }
    };

    const HttpResult r = HttpRequest(Url(), L"POST", "/api/sessions/" + resolvedId + "/lobby/join", body.dump());
    if (!r.ok) return state;

    try {
        const auto j = nlohmann::json::parse(r.body);
        state.ok = true;
        state.sessionId = j.value("session_id", resolvedId);
        state.hostIp = j.value("host_ip", "");
        state.hostPort = static_cast<uint16_t>(j.value("host_port", 11774));
        state.hostLanIp = j.value("host_lan_ip", "");
        state.isLan = j.value("is_lan", false);
        if (!state.hostLanIp.empty()) {
            NetworkTransport::Get().SetRemoteHostLanAddress(state.hostLanIp, state.hostPort);
        }
        state.isClosed = j.value("is_closed", false);
        state.closeReason = j.value("close_reason", "");
        if (j.contains("limits_id") && j["limits_id"].is_number_integer()) state.limitsId = j["limits_id"].get<int64_t>();
        state.map = j.value("map", "20_sword_slayer");
        state.mode = j.value("mode", "multiplayer");
        state.variant = j.value("variant", "");
        state.modId = Str(j, "mod_id", 32);
        state.modTitle = Str(j, "mod_title", 96);
        state.difficulty = j.value("difficulty", 1);
        state.inProgress = j.value("in_progress", false);

        if (j.contains("roster") && j["roster"].is_array()) {
            for (const auto& item : j["roster"]) {
                LobbyPlayer p;
                p.peerId = item.value("peerId", 1ULL);
                p.machineId = item.value("machineId", "");
                if (item.contains("username") && item["username"].is_string() && !item["username"].get<std::string>().empty())
                    p.username = item["username"].get<std::string>();
                else if (item.contains("player_name") && item["player_name"].is_string() && !item["player_name"].get<std::string>().empty())
                    p.username = item["player_name"].get<std::string>();
                else if (item.contains("name") && item["name"].is_string() && !item["name"].get<std::string>().empty())
                    p.username = item["name"].get<std::string>();
                else
                    p.username = item.value("isHost", false) ? "Host" : "Spartan";
                p.serviceTag = item.value("serviceTag", "117");
            p.rankIndex = item.value("rankIndex", -1);
            p.emblemId = item.value("emblemId", 0);
  p.emblemBackgroundId = item.value("emblemBackgroundId", 0);
            p.nameplateId = item.value("nameplateId", 0);
            p.team = item.value("team", 0);
            p.gamesPlayed = item.value("gamesPlayed", item.value("games_played", 0));
            p.kills = item.value("kills", 0);
            p.deaths = item.value("deaths", 0);
            p.challengesDone = item.value("challengesDone", item.value("challenges_done", 0));
            p.commendations = item.value("commendations", 0);
            p.armoryOwned = item.value("armoryOwned", item.value("armory_owned", 0));
                p.isHost = item.value("isHost", false);
                p.isDev = item.value("isDev", false);
                p.ping = item.value("ping", 0);
                if (!p.isHost && !p.machineId.empty()) {
                    try {
                        uint64_t mid = std::stoull(p.machineId, nullptr, 0);
                        int livePing = NetworkTransport::Get().GetPeerPingMs(mid);
                        if (livePing > 0) p.ping = livePing;
                    } catch (...) {}
                } else if (p.isHost) {
                    int liveHostPing = NetworkTransport::Get().GetPeerPingMs(0);
                    if (liveHostPing > 0 && (p.ping <= 0 || p.ping == 25)) p.ping = liveHostPing;
                }
                state.roster.push_back(p);
            }
        }

        std::string relayUrl = j.value("relay_url", "");
        if (!relayUrl.empty()) {
            NetworkTransport::Get().StartRelay(relayUrl, resolvedId, false, LobbyMachineId());
        }

        if (j.contains("launch_data") && j["launch_data"].is_object()) {
            const auto& ld = j["launch_data"];
            state.launchTriggered = true;
            state.launchMap = ld.value("map", state.map);
            state.launchMode = ld.value("mode", state.mode);
            state.launchVariant = ld.value("variant", state.variant);
            state.launchDifficulty = ld.value("difficulty", state.difficulty);
            state.launchMapOverride = ld.value("map_override", "");
            state.launchModFolder = ld.value("mod_folder", "");
            state.launchModId = ld.value("mod_id", state.modId);
            try { state.launchSkulls = std::stoull(ld.value("skulls", "0")); } catch (...) {}
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

bool MasterServerClient::SendReportSync(const std::string& sessionId, const std::string& reporter, const std::string& target,
                                        const std::string& reason, const std::string& map, const std::string& mode) {
    if (!Enabled()) return false;
    const nlohmann::json body = {
        { "session_id", sessionId }, { "reporter", reporter }, { "target", target },
        { "reason", reason }, { "map", map }, { "mode", mode }
    };
    return HttpRequest(Url(), L"POST", "/api/reports", body.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace)).ok;
}

bool MasterServerClient::StartVoteSync(const std::string& sessionId, const std::vector<std::string>& options, int seconds) {
    std::string token;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        token = m_sessionToken;
    }
    if (sessionId.empty() || token.empty() || !Enabled()) return false;
    const nlohmann::json body = { { "options", options }, { "seconds", seconds } };
    return HttpRequest(Url(), L"POST", "/api/sessions/" + sessionId + "/vote/start", body.dump(), token).ok;
}

bool MasterServerClient::CastVoteSync(const std::string& sessionId, uint64_t peerId, int choice) {
    if (sessionId.empty() || !Enabled()) return false;
    const nlohmann::json body = { { "peer_id", peerId }, { "choice", choice } };
    return HttpRequest(Url(), L"POST", "/api/sessions/" + sessionId + "/vote", body.dump()).ok;
}

void MasterServerClient::SetAdvertHostTeam(int team) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_advert.hostTeam = team;
}

bool MasterServerClient::SetTeamsSync(const std::string& sessionId, const std::vector<std::pair<uint64_t, int>>& teams) {
    std::string token;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        token = m_sessionToken;
    }
    if (sessionId.empty() || token.empty() || !Enabled()) return false;
    nlohmann::json list = nlohmann::json::array();
    for (const auto& [peer, team] : teams) list.push_back({ { "peer_id", peer }, { "team", team } });
    const nlohmann::json body = { { "teams", list } };
    return HttpRequest(Url(), L"POST", "/api/sessions/" + sessionId + "/teams", body.dump(), token).ok;
}

bool MasterServerClient::EndVoteSync(const std::string& sessionId) {
    std::string token;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        token = m_sessionToken;
    }
    if (sessionId.empty() || token.empty() || !Enabled()) return false;
    return HttpRequest(Url(), L"POST", "/api/sessions/" + sessionId + "/vote/end", "{}", token).ok;
}

bool MasterServerClient::LaunchLobbySync(const std::string& sessionId,
                                         const std::string& map,
                                         const std::string& mode,
                                         const std::string& variant,
                                         int difficulty,
                                         uint64_t skulls,
                                         const std::string& variantBody,
                                         const std::string& mapOverride,
                                         const std::string& modFolder,
                                         const std::string& modId,
                                         const std::string& mapVariant) {
    std::string token;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        token = m_sessionToken;
        m_advertInProgress = true;
    }
    if (sessionId.empty() || token.empty() || !Enabled()) return false;

    try {
        nlohmann::json body = {
            { "map", map },
            { "mode", mode },
            { "variant", variant },
            { "variant_body", variantBody },
            { "difficulty", difficulty },
            { "skulls", std::to_string(skulls) },
            { "map_override", mapOverride },
            { "mod_folder", modFolder },
            { "mod_id", modId },
            // File name only: guests look it up in their own map_variants folder.
            { "map_variant", mapVariant }
        };

        const HttpResult r = HttpRequest(Url(), L"POST", "/api/sessions/" + sessionId + "/lobby/launch",
                                         body.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace), token);
        return r.ok;
    } catch (const std::exception& e) {
        NOBLE_ERROR("MasterServerClient: LaunchLobbySync exception: {}", e.what());
        return false;
    } catch (...) {
        NOBLE_ERROR("MasterServerClient: LaunchLobbySync unknown exception");
        return false;
    }
}

} // namespace noble
