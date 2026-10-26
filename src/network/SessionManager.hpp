#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>
#include <cstdint>
#include <chrono>

namespace noble {

enum class NetworkMode {
    Local_LAN,
    Online_P2P
};

struct HostConfig {
    std::string sessionName = "Spartan's Reach Lobby";
    std::string gameMode = "Multiplayer";  // "Campaign", "Multiplayer", "Firefight", "Forge"
    std::string mapName = "forge_halo";
    std::string gametypeName = "Slayer";
    int maxPlayers = 16;
    uint16_t port = 11774;
    NetworkMode mode = NetworkMode::Local_LAN;
    std::string password;
};

struct DiscoveredSession {
    std::string sessionName;
    std::string hostAddress;
    uint16_t port = 11774;
    std::string gameMode;
    std::string mapName;
    std::string gametypeName;
    int playerCount = 1;
    int maxPlayers = 16;
    int pingMs = 0;
    bool isPasswordProtected = false;
    std::chrono::steady_clock::time_point lastSeen;
};

class SessionManager {
public:
    static SessionManager& Get() {
        static SessionManager instance;
        return instance;
    }

    bool Initialize();
    void Shutdown();

    // Self-Hosting API
    bool StartHosting(const HostConfig& config);
    void StopHosting();

    // Mode swapping
    void SetNetworkMode(NetworkMode mode);
    NetworkMode GetNetworkMode() const { return m_networkMode; }

    bool IsHosting() const { return m_isHosting; }
    const HostConfig& GetHostConfig() const { return m_activeConfig; }

    // Live discovered sessions for Server Browser
    std::vector<DiscoveredSession> GetDiscoveredSessions();

    // Manual refresh / broadcast ping
    void TriggerDiscoveryScan();

private:
    SessionManager();
    ~SessionManager();

    void BeaconLoop();
    void DiscoveryListenerLoop();

    std::atomic<bool> m_isInitialized{ false };
    std::atomic<bool> m_isHosting{ false };
    std::atomic<bool> m_stopThreads{ false };
    NetworkMode m_networkMode{ NetworkMode::Local_LAN };
    HostConfig m_activeConfig;

    std::vector<DiscoveredSession> m_discoveredSessions;
    std::mutex m_sessionsMutex;

    std::thread m_beaconThread;
    std::thread m_listenerThread;
};

} // namespace noble
