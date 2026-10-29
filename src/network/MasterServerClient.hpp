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
    std::string name;
    std::string address;
    uint16_t port = 11774;
    int playerCount = 1;
    int maxPlayers = 16;
    std::string map;
    std::string mode;
    bool hasPassword = false;
    bool isMod = false;
};

struct MasterJoinInfo {
    bool ok = false;
    std::string error;
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

    // Hosting: register, then send a heartbeat every 10 s until StopAdvertising (which removes the
    // listing). The server forgets a game 30 s after its last heartbeat.
    void StartAdvertising(const MasterAdvert& advert);
    void UpdateAdvert(int playerCount, bool inProgress);
    void StopAdvertising();
    bool IsAdvertising() const { return m_advertising.load(); }

    // Join signaling to trigger host NAT hole punching & WebSocket relay URL retrieval
    MasterJoinInfo JoinSessionSync(const std::string& sessionId, uint64_t guestMachineId, uint16_t guestPort);

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
