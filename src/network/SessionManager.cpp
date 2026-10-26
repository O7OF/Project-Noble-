#include "SessionManager.hpp"
#include "UPnP.hpp"
#include "common/Logger.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <sstream>
#include <algorithm>

namespace noble {

SessionManager::SessionManager() = default;

SessionManager::~SessionManager() {
    Shutdown();
}

bool SessionManager::Initialize() {
    if (m_isInitialized) return true;

    UPnP::Get().Initialize();

    m_stopThreads = false;
    m_listenerThread = std::thread(&SessionManager::DiscoveryListenerLoop, this);

    m_isInitialized = true;
    NOBLE_INFO("SessionManager: P2P Network Subsystem initialized (Default Mode: Local LAN)");
    return true;
}

void SessionManager::Shutdown() {
    if (!m_isInitialized) return;

    StopHosting();

    m_stopThreads = true;
    if (m_listenerThread.joinable()) {
        m_listenerThread.join();
    }

    m_isInitialized = false;
    NOBLE_INFO("SessionManager: Network Subsystem shutdown complete.");
}

void SessionManager::SetNetworkMode(NetworkMode mode) {
    if (m_networkMode == mode) return;
    m_networkMode = mode;

    if (m_networkMode == NetworkMode::Online_P2P) {
        NOBLE_INFO("SessionManager: Swapping network mode to ONLINE (UPnP P2P)");
        UPnP::Get().ForwardPort(m_activeConfig.port, "UDP", "Halo Reach CE Lobby");
    } else {
        NOBLE_INFO("SessionManager: Swapping network mode to LOCAL (LAN / Offline)");
        UPnP::Get().ReleasePort(m_activeConfig.port, "UDP");
    }
}

bool SessionManager::StartHosting(const HostConfig& config) {
    m_activeConfig = config;
    SetNetworkMode(config.mode);

    m_isHosting = true;
    NOBLE_INFO("SessionManager: Self-hosting session '{}' on port {} (Mode: {}, Map: {})", 
        config.sessionName, config.port, config.gameMode, config.mapName);

    if (m_beaconThread.joinable()) {
        m_beaconThread.join();
    }
    m_beaconThread = std::thread(&SessionManager::BeaconLoop, this);

    return true;
}

void SessionManager::StopHosting() {
    if (!m_isHosting) return;

    m_isHosting = false;
    if (m_beaconThread.joinable()) {
        m_beaconThread.join();
    }

    if (m_networkMode == NetworkMode::Online_P2P) {
        UPnP::Get().ReleasePort(m_activeConfig.port, "UDP");
    }

    NOBLE_INFO("SessionManager: Stopped self-hosting session.");
}

void SessionManager::TriggerDiscoveryScan() {
    // Send a discovery probe request on subnet
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) return;

    BOOL broadcast = TRUE;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const char*>(&broadcast), sizeof(broadcast));

    sockaddr_in destAddr = { 0 };
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(11774);
    destAddr.sin_addr.s_addr = INADDR_BROADCAST;

    const char* probeMsg = "NOBL_PROBE";
    sendto(sock, probeMsg, static_cast<int>(strlen(probeMsg)), 0, 
        reinterpret_cast<sockaddr*>(&destAddr), sizeof(destAddr));

    closesocket(sock);
}

void SessionManager::BeaconLoop() {
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) return;

    BOOL broadcast = TRUE;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const char*>(&broadcast), sizeof(broadcast));

    sockaddr_in destAddr = { 0 };
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(11774);
    destAddr.sin_addr.s_addr = INADDR_BROADCAST;

    while (!m_stopThreads && m_isHosting) {
        // Beacon payload: NOBL_BEACON|Name|Mode|Map|Gametype|Players|MaxPlayers|Port|HasPass
        std::ostringstream ss;
        ss << "NOBL_BEACON|" 
           << m_activeConfig.sessionName << "|"
           << m_activeConfig.gameMode << "|"
           << m_activeConfig.mapName << "|"
           << m_activeConfig.gametypeName << "|1|"
           << m_activeConfig.maxPlayers << "|"
           << m_activeConfig.port << "|"
           << (!m_activeConfig.password.empty() ? "1" : "0");

        std::string payload = ss.str();
        sendto(sock, payload.c_str(), static_cast<int>(payload.length()), 0, 
            reinterpret_cast<sockaddr*>(&destAddr), sizeof(destAddr));

        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    }

    closesocket(sock);
}

void SessionManager::DiscoveryListenerLoop() {
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) return;

    BOOL reuse = TRUE;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&reuse), sizeof(reuse));

    DWORD timeoutMs = 500;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeoutMs), sizeof(timeoutMs));

    sockaddr_in bindAddr = { 0 };
    bindAddr.sin_family = AF_INET;
    bindAddr.sin_port = htons(11774);
    bindAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, reinterpret_cast<sockaddr*>(&bindAddr), sizeof(bindAddr)) != 0) {
        closesocket(sock);
        return;
    }

    char buffer[1024];

    while (!m_stopThreads) {
        sockaddr_in senderAddr = { 0 };
        int senderLen = sizeof(senderAddr);

        int received = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, 
            reinterpret_cast<sockaddr*>(&senderAddr), &senderLen);

        if (received > 0) {
            buffer[received] = '\0';
            std::string msg(buffer);

            if (msg.rfind("NOBL_BEACON|", 0) == 0) {
                // Parse beacon
                char ipStr[INET_ADDRSTRLEN] = { 0 };
                inet_ntop(AF_INET, &(senderAddr.sin_addr), ipStr, sizeof(ipStr));

                std::vector<std::string> tokens;
                std::stringstream ss(msg);
                std::string token;
                while (std::getline(ss, token, '|')) {
                    tokens.push_back(token);
                }

                if (tokens.size() >= 9) {
                    DiscoveredSession session;
                    session.hostAddress = ipStr;
                    session.sessionName = tokens[1];
                    session.gameMode = tokens[2];
                    session.mapName = tokens[3];
                    session.gametypeName = tokens[4];
                    session.playerCount = std::stoi(tokens[5]);
                    session.maxPlayers = std::stoi(tokens[6]);
                    session.port = static_cast<uint16_t>(std::stoi(tokens[7]));
                    session.isPasswordProtected = (tokens[8] == "1");
                    session.pingMs = 12; // Local LAN base latency
                    session.lastSeen = std::chrono::steady_clock::now();

                    std::lock_guard<std::mutex> lock(m_sessionsMutex);
                    auto it = std::find_if(m_discoveredSessions.begin(), m_discoveredSessions.end(),
                        [&](const DiscoveredSession& s) {
                            return s.hostAddress == session.hostAddress && s.port == session.port;
                        });

                    if (it != m_discoveredSessions.end()) {
                        *it = session;
                    } else {
                        m_discoveredSessions.push_back(session);
                    }
                }
            }
        }

        // Prune stale sessions (not seen in > 4.5 seconds)
        auto now = std::chrono::steady_clock::now();
        {
            std::lock_guard<std::mutex> lock(m_sessionsMutex);
            m_discoveredSessions.erase(
                std::remove_if(m_discoveredSessions.begin(), m_discoveredSessions.end(),
                    [&](const DiscoveredSession& s) {
                        return std::chrono::duration_cast<std::chrono::seconds>(now - s.lastSeen).count() > 4;
                    }),
                m_discoveredSessions.end());
        }
    }

    closesocket(sock);
}

std::vector<DiscoveredSession> SessionManager::GetDiscoveredSessions() {
    std::lock_guard<std::mutex> lock(m_sessionsMutex);
    return m_discoveredSessions;
}

} // namespace noble
