#include "SessionManager.hpp"
#include "UPnP.hpp"
#include "common/Logger.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <sstream>
#include <algorithm>
#include <charconv>

namespace noble {

namespace {

// Non-throwing integer parse; the whole string must be a number.
bool ParseInt(const std::string& s, int& out) {
    if (s.empty() || s.size() > 10) return false;
    const auto result = std::from_chars(s.data(), s.data() + s.size(), out);
    return result.ec == std::errc() && result.ptr == s.data() + s.size();
}

// Text from the network is shown in the menus: drop control characters and cap the length.
std::string SanitizeText(const std::string& s, size_t maxLen) {
    std::string out;
    out.reserve(std::min(s.size(), maxLen));
    for (char c : s) {
        if (static_cast<unsigned char>(c) < 0x20 || c == 0x7F) continue;
        out.push_back(c);
        if (out.size() >= maxLen) break;
    }
    return out;
}

} // namespace

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

    UPnP::Get().Shutdown();
    m_isInitialized = false;
    NOBLE_INFO("SessionManager: Network Subsystem shutdown complete.");
}

void SessionManager::SetNetworkMode(NetworkMode mode) {
    if (m_networkMode == mode) {
        if (mode == NetworkMode::Online_P2P && !UPnP::Get().IsActive()) {
            UPnP::Get().ForwardPort(m_activeConfig.port, "UDP", "Halo Reach CE Lobby");
        }
        return;
    }
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

                // Beacons come from anyone on the network: parse without exceptions (std::stoi throws
                // on bad input, and an exception escaping this thread ends the whole process).
                int players = 0, maxPlayers = 0, port = 0;
                if (tokens.size() >= 9 &&
                    ParseInt(tokens[5], players) && ParseInt(tokens[6], maxPlayers) && ParseInt(tokens[7], port) &&
                    players >= 0 && players <= 64 && maxPlayers >= 1 && maxPlayers <= 64 &&
                    port >= 1 && port <= 65535) {
                    DiscoveredSession session;
                    session.hostAddress = ipStr;
                    session.sessionName = SanitizeText(tokens[1], 64);
                    session.gameMode = SanitizeText(tokens[2], 32);
                    session.mapName = SanitizeText(tokens[3], 64);
                    session.gametypeName = SanitizeText(tokens[4], 64);
                    session.playerCount = players;
                    session.maxPlayers = maxPlayers;
                    session.port = static_cast<uint16_t>(port);
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
