#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include "common/Types.hpp"
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <thread>
#include <nlohmann/json.hpp>

namespace noble {

struct DedicatedConfig {
    std::string serverName = "Project Noble Reach Dedicated Server";
    uint16_t port = 11774;
    uint16_t rconPort = 11775;
    std::string rconPassword = "noble_admin_password";
    std::string gameMode = "Multiplayer";
    std::string initialMap = "forge_halo";
    std::string initialGametype = "Slayer";
    uint32_t maxPlayers = 32;
    uint32_t tickRate = 60;
    std::vector<std::string> playlist = { "forge_halo", "20_sword_slayer", "50_panopticon", "45_aftship" };
    std::string fastDlUrl = "http://localhost:8080/maps/";

    static DedicatedConfig LoadFromFile(const std::string& filePath);
    void SaveToFile(const std::string& filePath) const;
};

class DedicatedServer {
public:
    DedicatedServer();
    ~DedicatedServer();

    bool Start(const DedicatedConfig& config);
    void Stop();
    bool IsRunning() const { return m_isRunning.load(); }

    void ProcessConsoleCommand(const std::string& cmd);

private:
    void RunServerLoop();
    void RunRconLoop();

    DedicatedConfig m_config;
    std::atomic<bool> m_isRunning{ false };
    std::thread m_serverThread;
    std::thread m_rconThread;
    SOCKET m_rconListenSocket = INVALID_SOCKET;
};

} // namespace noble
