#include "DedicatedServer.hpp"
#include "common/Logger.hpp"
#include "network/UPnP.hpp"
#include "network/SessionManager.hpp"
#include "network/HostModeration.hpp"
#include "network/NetworkTransport.hpp"
#include "hooks/LobbyPatches.hpp"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <iostream>
#include <chrono>
#include <winsock2.h>
#include <ws2tcpip.h>

namespace noble {

DedicatedConfig DedicatedConfig::LoadFromFile(const std::string& filePath) {
    DedicatedConfig cfg;
    try {
        std::ifstream file(filePath);
        if (file.is_open()) {
            nlohmann::json j;
            file >> j;
            if (j.contains("serverName")) cfg.serverName = j["serverName"].get<std::string>();
            if (j.contains("port")) cfg.port = j["port"].get<uint16_t>();
            if (j.contains("rconPort")) cfg.rconPort = j["rconPort"].get<uint16_t>();
            if (j.contains("rconPassword")) cfg.rconPassword = j["rconPassword"].get<std::string>();
            if (j.contains("gameMode")) cfg.gameMode = j["gameMode"].get<std::string>();
            if (j.contains("initialMap")) cfg.initialMap = j["initialMap"].get<std::string>();
            if (j.contains("initialGametype")) cfg.initialGametype = j["initialGametype"].get<std::string>();
            if (j.contains("maxPlayers")) cfg.maxPlayers = j["maxPlayers"].get<uint32_t>();
            if (j.contains("tickRate")) cfg.tickRate = j["tickRate"].get<uint32_t>();
            if (j.contains("playlist") && j["playlist"].is_array()) {
                cfg.playlist = j["playlist"].get<std::vector<std::string>>();
            }
            if (j.contains("fastDlUrl")) cfg.fastDlUrl = j["fastDlUrl"].get<std::string>();
            NOBLE_INFO("Loaded dedicated server configuration from '{}'", filePath);
        } else {
            NOBLE_INFO("Configuration file '{}' not found. Creating default configuration.", filePath);
            cfg.SaveToFile(filePath);
        }
    } catch (const std::exception& e) {
        NOBLE_WARN("Failed to parse config '{}': {}. Using defaults.", filePath, e.what());
    }
    return cfg;
}

void DedicatedConfig::SaveToFile(const std::string& filePath) const {
    try {
        nlohmann::json j;
        j["serverName"] = serverName;
        j["port"] = port;
        j["rconPort"] = rconPort;
        j["rconPassword"] = rconPassword;
        j["gameMode"] = gameMode;
        j["initialMap"] = initialMap;
        j["initialGametype"] = initialGametype;
        j["maxPlayers"] = maxPlayers;
        j["tickRate"] = tickRate;
        j["playlist"] = playlist;
        j["fastDlUrl"] = fastDlUrl;

        std::ofstream file(filePath);
        if (file.is_open()) {
            file << j.dump(4);
            NOBLE_INFO("Saved dedicated server configuration to '{}'", filePath);
        }
    } catch (const std::exception& e) {
        NOBLE_ERROR("Failed to write config '{}': {}", filePath, e.what());
    }
}

DedicatedServer::DedicatedServer() = default;

DedicatedServer::~DedicatedServer() {
    Stop();
}

bool DedicatedServer::Start(const DedicatedConfig& config) {
    if (m_isRunning.load()) {
        return true;
    }

    m_config = config;
    m_isRunning.store(true);

    NOBLE_INFO("=============================================================");
    NOBLE_INFO(" Starting Noble Dedicated Server: '{}'", m_config.serverName);
    NOBLE_INFO(" Gameplay Port: {} | RCON Port: {} | Max Players: {}", 
        m_config.port, m_config.rconPort, m_config.maxPlayers);
    NOBLE_INFO(" Map: {} | Gametype: {}", m_config.initialMap, m_config.initialGametype);
    NOBLE_INFO("=============================================================");

    // 0. Ensure Winsock is initialized
    WSADATA wsaData{};
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // 1. Forward gameplay & RCON ports via UPnP
    std::thread([this]() {
        UPnP::Get().ForwardPort(m_config.port, "UDP", "Noble Dedicated Gameplay");
        UPnP::Get().ForwardPort(m_config.rconPort, "TCP", "Noble Dedicated RCON");
    }).detach();

    // 2. Start RCON listener thread
    m_rconThread = std::thread(&DedicatedServer::RunRconLoop, this);

    // 3. Start main server heartbeat & playlist manager thread
    m_serverThread = std::thread(&DedicatedServer::RunServerLoop, this);

    return true;
}

void DedicatedServer::Stop() {
    if (!m_isRunning.load()) return;

    NOBLE_INFO("Stopping Noble Dedicated Server...");
    m_isRunning.store(false);

    if (m_rconListenSocket != INVALID_SOCKET) {
        closesocket(m_rconListenSocket);
        m_rconListenSocket = INVALID_SOCKET;
    }

    if (m_serverThread.joinable()) {
        m_serverThread.join();
    }
    if (m_rconThread.joinable()) {
        m_rconThread.join();
    }

    WSACleanup();

    NOBLE_INFO("Dedicated server stopped cleanly.");
}

void DedicatedServer::ProcessConsoleCommand(const std::string& cmd) {
    if (cmd.empty()) return;

    if (cmd == "help") {
        std::cout << "Available Commands:\n"
                  << "  status            - Displays current server stats and player count\n"
                  << "  map <name>        - Changes active map (e.g. 'map forge_halo')\n"
                  << "  gametype <name>   - Sets gametype mode\n"
                  << "  say <message>     - Broadcasts admin message to players\n"
                  << "  players           - Lists connected players (machine ids)\n"
                  << "  kick <id|name>    - Removes a player (2-minute rejoin cooldown)\n"
                  << "  ban <id|name> [minutes] - Bans a player (no minutes = permanent)\n"
                  << "  bans              - Lists bans\n"
                  << "  unban <number>    - Lifts the ban with that number from 'bans'\n"
                  << "  quit / exit       - Gracefully shuts down dedicated server\n";
    } else if (cmd == "status") {
        std::cout << "[STATUS] Server: " << m_config.serverName << "\n"
                  << "         Map: " << m_config.initialMap << " (" << m_config.initialGametype << ")\n"
                  << "         Port: " << m_config.port << " | RCON: " << m_config.rconPort << "\n"
                  << "         Players: 0/" << m_config.maxPlayers << " (Lobby expanded: " 
                  << (LobbyPatches::IsApplied() ? "YES" : "NO") << ")\n";
    } else if (cmd.rfind("map ", 0) == 0) {
        std::string newMap = cmd.substr(4);
        m_config.initialMap = newMap;
        std::cout << "[SERVER] Active map changed to: " << newMap << "\n";
    } else if (cmd == "players") {
        const auto peers = NetworkTransport::Get().GetAllPeerPingsMs();
        std::cout << "[PLAYERS] " << peers.size() << " connected\n";
        for (const auto& [id, ping] : peers) {
            char buf[64];
            std::snprintf(buf, sizeof(buf), "  0x%016llX  %d ms\n", static_cast<unsigned long long>(id), ping);
            std::cout << buf;
        }
    } else if (cmd.rfind("kick ", 0) == 0 || cmd.rfind("ban ", 0) == 0) {
        // Same moderation as a player host (HostModeration): the host's tick sends the player away and keeps
        // sending banned / cooling-down players away when they come back.
        const bool ban = cmd.rfind("ban ", 0) == 0;
        std::string rest = cmd.substr(ban ? 4 : 5);
        int minutes = 0;
        if (ban) {
            const size_t sp = rest.find_last_of(' ');
            if (sp != std::string::npos) {
                const std::string tail = rest.substr(sp + 1);
                if (!tail.empty() && tail.find_first_not_of("0123456789") == std::string::npos) {
                    minutes = std::atoi(tail.c_str());
                    rest = rest.substr(0, sp);
                }
            }
        }
        HostModeration::PlayerRef ref;
        try {
            size_t used = 0;
            ref.machineId = std::stoull(rest, &used, 0);
            if (used != rest.size()) ref.machineId = 0;
        } catch (...) {
            ref.machineId = 0;
        }
        if (ref.machineId == 0) ref.name = rest;
        if (rest.empty()) {
            std::cout << "Usage: " << (ban ? "ban <id|name> [minutes]" : "kick <id|name>") << "\n";
        } else if (ban) {
            HostModeration::Ban(ref, minutes, "Banned by the server admin.");
            std::cout << "[SERVER] Banned '" << rest << "' "
                      << HostModeration::BanLengthText(minutes) << "\n";
        } else {
            HostModeration::Kick(ref, "Removed by the server admin.");
            std::cout << "[SERVER] Kicked '" << rest << "'\n";
        }
    } else if (cmd == "bans") {
        const auto bans = HostModeration::Bans();
        std::cout << "[BANS] " << bans.size() << "\n";
        for (size_t i = 0; i < bans.size(); ++i) {
            std::cout << "  " << (i + 1) << ". " << (bans[i].name.empty() ? "(no name)" : bans[i].name)
                      << (bans[i].until == 0 ? "  permanent" : "  timed") << "\n";
        }
    } else if (cmd.rfind("unban ", 0) == 0) {
        const int n = std::atoi(cmd.substr(6).c_str());
        const auto bans = HostModeration::Bans();
        if (n >= 1 && static_cast<size_t>(n) <= bans.size()) {
            HostModeration::Unban(static_cast<size_t>(n - 1));
            std::cout << "[SERVER] Lifted ban " << n << "\n";
        } else {
            std::cout << "No ban " << n << " (see 'bans').\n";
        }
    } else if (cmd.rfind("say ", 0) == 0) {
        std::cout << "[BROADCAST] ADMIN: " << cmd.substr(4) << "\n";
    } else if (cmd == "quit" || cmd == "exit") {
        std::cout << "[SERVER] Shutdown initiated by console admin.\n";
        m_isRunning.store(false);
    } else {
        std::cout << "Unknown command: '" << cmd << "'. Type 'help' for available commands.\n";
    }
}

void DedicatedServer::RunServerLoop() {
    uint32_t tick = 0;
    while (m_isRunning.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / m_config.tickRate));
        tick++;

        // Moderation: send banned / just-kicked players away (rate-limited inside).
        {
            std::vector<HostModeration::PlayerRef> refs;
            for (const auto& [id, ping] : NetworkTransport::Get().GetAllPeerPingsMs()) {
                (void)ping;
                HostModeration::PlayerRef r;
                r.machineId = id;
                refs.push_back(r);
            }
            HostModeration::Tick(refs);
        }

        // Periodic heartbeat beacon every 5 seconds
        if (tick % (m_config.tickRate * 5) == 0) {
            // Heartbeat beacon broadcast
        }
    }
}

void DedicatedServer::RunRconLoop() {
    m_rconListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_rconListenSocket == INVALID_SOCKET) {
        NOBLE_ERROR("DedicatedServer: Failed to create RCON TCP socket.");
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(m_config.rconPort);

    if (bind(m_rconListenSocket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        NOBLE_ERROR("DedicatedServer: Failed to bind RCON socket on port {}", m_config.rconPort);
        closesocket(m_rconListenSocket);
        m_rconListenSocket = INVALID_SOCKET;
        return;
    }

    if (listen(m_rconListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        NOBLE_ERROR("DedicatedServer: RCON socket listen failed.");
        closesocket(m_rconListenSocket);
        m_rconListenSocket = INVALID_SOCKET;
        return;
    }

    NOBLE_INFO("RCON TCP Listener active on port {}", m_config.rconPort);

    // Set non-blocking with 500ms timeout
    timeval tv{};
    tv.tv_sec = 0;
    tv.tv_usec = 500000;

    while (m_isRunning.load()) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(m_rconListenSocket, &readSet);

        int selectRes = select(0, &readSet, nullptr, nullptr, &tv);
        if (selectRes > 0 && FD_ISSET(m_rconListenSocket, &readSet)) {
            sockaddr_in clientAddr{};
            int clientLen = sizeof(clientAddr);
            SOCKET clientSock = accept(m_rconListenSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);
            if (clientSock != INVALID_SOCKET) {
                // Read RCON auth password
                char authBuf[256]{};
                int bytes = recv(clientSock, authBuf, sizeof(authBuf) - 1, 0);
                if (bytes > 0) {
                    std::string authStr(authBuf);
                    while (!authStr.empty() && (authStr.back() == '\r' || authStr.back() == '\n')) {
                        authStr.pop_back();
                    }
                    if (authStr == m_config.rconPassword) {
                        const char* authOk = "RCON_AUTH_OK: Connected to Noble Dedicated RCON.\n";
                        send(clientSock, authOk, static_cast<int>(strlen(authOk)), 0);
                    } else {
                        const char* authFail = "RCON_AUTH_FAILED: Invalid password. Disconnecting.\n";
                        send(clientSock, authFail, static_cast<int>(strlen(authFail)), 0);
                        closesocket(clientSock);
                    }
                }
            }
        }
    }
}

} // namespace noble
