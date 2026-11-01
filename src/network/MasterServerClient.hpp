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
    // Identity for the party/lobby row, so other players' rows show their own rank, emblem, nameplate
    // and team instead of placeholders (2026-10-30). 0 / -1 = not known.
    int rankIndex = -1;
    int emblemId = 0;
    int emblemBackgroundId = 0;   // 0 = none, so other players see it
    int nameplateId = 0;
    int team = 0;            // 0 = no team (Free for All / no team mode), 1 = red, 2 = blue
};

// Snapshot of a pre-game lobby session.
struct LobbyState {
    bool ok = false;
    std::string sessionId;
    std::string name;
    std::string map;
    std::string mode;
    std::string variant;
    // Steam Workshop mod the host is using (published-file id). Empty = vanilla / local-only mod.
    std::string modId;
    std::string modTitle;
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
    std::string hostLanIp;
    bool isLan = false;
    bool isClosed = false;
    std::string closeReason;
    // A running vote on what to play next: up to 3 options, then "none of these" as the last count.
    std::string voteId;
    std::vector<std::string> voteOptions;
    std::vector<int> voteCounts;
    int voteMsLeft = 0;
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
    std::string modId;
    std::string modTitle;
    bool isPrivate = false;
    std::string lanAddress;
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
    // Set when the host's chosen map comes from a Steam Workshop mod; the joiner checks it before
    // entering the lobby and can download it through Steam.
    std::string modId;
    std::string modTitle;
    bool isPrivate = false;
    std::string lanAddress;
    uint64_t hostPeerId = 1;
    std::string hostMachineId;
    std::string hostUsername;
    bool hostIsDev = false;
    std::string hostServiceTag = "117";
    // The host's row identity (same meaning as LobbyPlayer's fields).
    int hostRankIndex = -1;
    int hostEmblemId = 0;
    int hostEmblemBackgroundId = 0;
    int hostNameplateId = 0;
    int hostTeam = 0;
};

struct MasterJoinInfo {
    bool ok = false;
    std::string error;
    std::string sessionId;
    std::string hostIp;
    uint16_t hostPort = 11774;
    std::string relayUrl;
    std::string hostLanIp;
    bool isLan = false;
};

// Small HTTP client for tools/master-server (see its README). The address comes from the
// SETTINGS tab (settings.json "masterServerUrl") or "-master-server <url>"; when it is empty only
// LAN games are shown. All network calls run on one background thread, so the menus never block.
class MasterServerClient {
public:
    static MasterServerClient& Get();
    static uint64_t GetLobbyMachineId();

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
                            int rankIndex,
                            int emblemId,
                            int emblemBackgroundId, 
                            int nameplateId,
                            int team,
                            uint16_t localPort,
                            const std::string& fallbackHostIp = "",
                            uint16_t fallbackHostPort = 11774);
    bool LeaveLobbySync(const std::string& sessionId, uint64_t peerId);
    // Voting: the host opens and closes it with the session token; a lobby member's choice is 0..options-1, or options for none.
    bool StartVoteSync(const std::string& sessionId, const std::vector<std::string>& options, int seconds);
    bool CastVoteSync(const std::string& sessionId, uint64_t peerId, int choice);
    bool EndVoteSync(const std::string& sessionId);
    // The host sets each lobby member's team: 0 none, 1 red, 2 blue. Pairs of peer id and team.
    bool SetTeamsSync(const std::string& sessionId, const std::vector<std::pair<uint64_t, int>>& teams);
    void SetAdvertHostTeam(int team);   // the heartbeat carries the host's team, so it has to stay current
    // A player report from the scoreboard. True when the server answered; it stores the report for the team to read.
    bool SendReportSync(const std::string& sessionId, const std::string& reporter, const std::string& target,
                        const std::string& reason, const std::string& map, const std::string& mode);
    bool LaunchLobbySync(const std::string& sessionId,
                         const std::string& map,
                         const std::string& mode,
                         const std::string& variant,
                         int difficulty,
                         uint64_t skulls,
                         const std::string& variantBody = "");

    void UpdateLobbyAdvert(const std::string& map, const std::string& mode, const std::string& variant, int diff, uint64_t skulls);
    std::string GetCurrentSessionId() const;
    std::string GetCurrentSessionToken() const;

    // Browsing: ask for the list (asynchronously); the result is read with Sessions().
    void RequestList();
    // Live count of logged-in clients from /api/players/online (health endpoint as fallback).
    void RequestOnlineCount();
    int OnlinePlayers() const { return m_onlinePlayers.load(); }
    int TotalPlayers() const { return m_totalPlayers.load(); }
    int OnlineServers() const { return m_onlineServers.load(); }
    std::vector<MasterSession> Sessions() const;
    std::string Status() const;   // last result / error, for the menu

    // keepSession: stop this process's master-server thread but leave the session listed, because the
    // relaunched match process (given -session-id/-session-token) carries it on.
    void Shutdown(bool keepSession = false);

private:
    MasterServerClient() = default;
    ~MasterServerClient();
    void EnsureThread();
    void WorkerLoop();
    void DoList();
    void DoOnline();
    void DoRegister();
    void DoHeartbeat();
    void DoUnregister();
    void SetStatus(const std::string& status);

    mutable std::mutex m_mutex;
    std::condition_variable m_wake;
    std::thread m_thread;
    bool m_stop = false;
    bool m_listRequested = false;
    bool m_onlineRequested = false;
    bool m_unregisterRequested = false;

    std::string m_url;
    std::string m_status;
    std::vector<MasterSession> m_sessions;
    std::atomic<int> m_onlinePlayers{0};
    std::atomic<int> m_totalPlayers{0};
    std::atomic<int> m_onlineServers{0};

    std::atomic<bool> m_advertising{false};
    MasterAdvert m_advert;
    bool m_advertInProgress = false;
    std::string m_sessionId;
    std::string m_sessionToken;
    std::chrono::steady_clock::time_point m_nextHeartbeat{};
};

} // namespace noble
