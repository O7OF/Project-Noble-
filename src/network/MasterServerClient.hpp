#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace noble {

// A participant in a pre-game lobby.
struct LobbyPlayer {
    uint64_t peerId = 0;
    std::string machineId;
    std::string username;
    std::string serviceTag = "117";
    bool isHost = false;
    bool isDev = false;
    int ping = 0;
};

// Snapshot of a pre-game lobby session.
struct LobbyState {
    bool ok = false;
    std::string sessionId;
    std::string name;
    std::string map;
    std::string mode;
    std::string variant;
    int difficulty = 1;
    uint64_t skulls = 0;
    bool inProgress = false;
    int playerCount = 1;
    int maxPlayers = 16;
    std::string hostIp;
    uint16_t hostPort = 11774;
    std::vector<LobbyPlayer> roster;
    bool launchTriggered = false;
    std::string launchMap;
    std::string launchMode;
    std::string launchVariant;
    int launchDifficulty = 1;
    uint64_t launchSkulls = 0;
};

// One game listed by the master server.
struct MasterSession {
    std::string id;
    std::string name;
    std::string address;   // public IP as seen by the master server
    uint16_t port = 11774;
    int playerCount = 0;
    int maxPlayers = 16;
    std::string map;
    std::string mode;
    bool inProgress = false;
    bool hasPassword = false;
    bool isMod = false;
    std::string build;
};

// Description of the game this player hosts, sent to the master server.
struct MasterAdvert {
    std::string id;
    std::string token;
    std::string name;
    std::string address;
    uint16_t port = 11774;
    int playerCount = 1;
    int maxPlayers = 16;
    std::string map;
    std::string mode;
    std::string variant;
    int difficulty = 1;
    uint64_t skulls = 0;
    bool inProgress = false;
    bool hasPassword = false;
    bool isMod = false;
    uint64_t hostPeerId = 1;
    std::string hostMachineId;
    std::string hostUsername;
    bool hostIsDev = false;
    std::string hostServiceTag = "117";
};

struct MasterJoinInfo {
    bool ok = false;
    std::string error;
    std::string sessionId;
    std::string hostIp;
    uint16_t hostPort = 11774;
    std::string relayUrl;
};

// Small HTTP client for tools/master-server (see its README). The address comes from the
// SETTINGS tab (settings.json "masterServerUrl") or "-master-server <url>"; when it is empty only
// LAN games are shown. All network calls run on one background thread, so the menus never block.
class MasterServerClient {
public:
    static MasterServerClient& Get();

    void SetUrl(const std::string& url);
    std::string Url() const;
    bool Enabled() const { return !Url().empty(); }

    // Hosting: register, then send a heartbeat until StopAdvertising (which removes the
    // listing). The server forgets a game 30 s after its last heartbeat.
    void StartAdvertising(const MasterAdvert& advert);
    void UpdateAdvert(int playerCount, bool inProgress);
    void StopAdvertising();
    bool IsAdvertising() const { return m_advertising.load(); }

    // Join signaling to trigger host NAT hole punching & WebSocket relay URL retrieval.
    // If sessionId is empty and fallbackHostIp is provided, resolves the session ID from /api/sessions.
    MasterJoinInfo JoinSessionSync(const std::string& sessionId,
                                   uint64_t guestMachineId,
                                   uint16_t guestPort,
                                   const std::string& fallbackHostIp = "",
                                   uint16_t fallbackHostPort = 11774);

    // Pre-game lobby API
    LobbyState FetchLobbyStateSync(const std::string& sessionId);
    LobbyState JoinLobbySync(const std::string& sessionId,
                            uint64_t peerId,
                            const std::string& username,
                            bool isDev,
                            const std::string& serviceTag,
                            uint16_t localPort);
    bool LeaveLobbySync(const std::string& sessionId, uint64_t peerId);
    bool LaunchLobbySync(const std::string& sessionId,
                         const std::string& map,
                         const std::string& mode,
                         const std::string& variant,
                         int difficulty,
                         uint64_t skulls);

    void UpdateLobbyAdvert(const std::string& map, const std::string& mode, const std::string& variant, int diff, uint64_t skulls);
    std::string GetCurrentSessionId() const;
    std::string GetCurrentSessionToken() const;

    // Browsing: ask for the list (asynchronously); the result is read with Sessions().
    void RequestList();
    std::vector<MasterSession> Sessions() const;
    std::string Status() const;   // last result / error, for the menu

    void Shutdown();

private:
    MasterServerClient() = default;
    ~MasterServerClient();
    void EnsureThread();
    void WorkerLoop();
    void DoList();
    void DoRegister();
    void DoHeartbeat();
    void DoUnregister();
    void SetStatus(const std::string& status);

    mutable std::mutex m_mutex;
    std::condition_variable m_wake;
    std::thread m_thread;
    bool m_stop = false;
    bool m_listRequested = false;
    bool m_unregisterRequested = false;

    std::string m_url;
    std::string m_status;
    std::vector<MasterSession> m_sessions;

    std::atomic<bool> m_advertising{false};
    MasterAdvert m_advert;
    bool m_advertInProgress = false;
    std::string m_sessionId;
    std::string m_sessionToken;
    std::chrono::steady_clock::time_point m_nextHeartbeat{};
};

} // namespace noble
