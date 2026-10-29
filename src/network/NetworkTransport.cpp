#include "NetworkTransport.hpp"
#include <nlohmann/json.hpp>
#include "common/Logger.hpp"

#include <windows.h>
#include <winhttp.h>
#include <random>
#include <cstring>
#include <chrono>

namespace noble {

namespace {

// Diagnostic: hex-dump Blam! payloads so the join handshake (55-byte join request,
// 16-byte host reply, 26/10-byte channel traffic) can be decoded from the log.
constexpr uint64_t kPacketDumpLimit = 60;
std::string HexDump(const uint8_t* data, size_t size, size_t maxBytes = 96) {
    static const char* kHex = "0123456789ABCDEF";
    const size_t n = size < maxBytes ? size : maxBytes;
    std::string out;
    out.reserve(n * 3 + 8);
    for (size_t i = 0; i < n; ++i) {
        out.push_back(kHex[data[i] >> 4]);
        out.push_back(kHex[data[i] & 0xF]);
        out.push_back(' ');
    }
    if (n < size) out += "...";
    return out;
}

uint64_t GenerateRandomMachineId() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dist;
    uint64_t id = dist(gen) | 1ULL; // Bit 0 always 1 per Reach convention
    if (id == NetworkTransport::kDefaultHostMachineId) {
        id ^= 0xFFFFFFFF00000000ULL;
    }
    return id;
}

std::wstring Widen(const std::string& narrow) {
    if (narrow.empty()) return L"";
    int req = MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), -1, nullptr, 0);
    if (req <= 0) return L"";
    std::wstring out(static_cast<size_t>(req), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), -1, &out[0], req);
    if (!out.empty() && out.back() == L'\0') out.pop_back();
    return out;
}

} // namespace

NetworkTransport::NetworkTransport()
    : m_localMachineId(kDefaultHostMachineId)
    , m_hostMachineId(kDefaultHostMachineId) {}

NetworkTransport::~NetworkTransport() {
    Shutdown();
}

bool NetworkTransport::Initialize(bool isHost, const std::string& remoteIp, uint16_t remotePort, uint16_t localPort) {
    if (m_active.load()) {
        Shutdown();
    }

    m_isHost = isHost;
    m_localPort = localPort;
    m_sessionId = kNobleSessionId;

    if (m_isHost) {
        m_localMachineId = kDefaultHostMachineId;
        m_hostMachineId = kDefaultHostMachineId;
    } else {
        m_localMachineId = GenerateRandomMachineId();
        m_hostMachineId = kDefaultHostMachineId;
    }

    std::memset(&m_remoteHostAddr, 0, sizeof(m_remoteHostAddr));
    if (!isHost && !remoteIp.empty()) {
        m_remoteHostAddr.sin_family = AF_INET;
        m_remoteHostAddr.sin_port = htons(remotePort);
        if (inet_pton(AF_INET, remoteIp.c_str(), &m_remoteHostAddr.sin_addr) <= 0) {
            NOBLE_ERROR("NetworkTransport: Invalid remote host IP address '{}'", remoteIp);
            return false;
        }
        NOBLE_INFO("NetworkTransport: Configured guest target host {}:{}", remoteIp, remotePort);
    }

    // Create UDP socket
    m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_socket == INVALID_SOCKET) {
        NOBLE_ERROR("NetworkTransport: Failed to create UDP socket (WSAError: {})", WSAGetLastError());
        return false;
    }

    // Set non-blocking mode
    u_long nonBlocking = 1;
    if (ioctlsocket(m_socket, FIONBIO, &nonBlocking) != 0) {
        NOBLE_WARN("NetworkTransport: Failed to set non-blocking socket mode");
    }

    // Set socket buffer sizes to 512 KB
    int bufSize = 512 * 1024;
    setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char*>(&bufSize), sizeof(bufSize));
    setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<const char*>(&bufSize), sizeof(bufSize));

    // Exclusive use of the port. On Windows SO_REUSEADDR (used here before 2026-10-29) lets a SECOND process
    // bind the same UDP port and take datagrams meant for the first: a stray second Noble on the guest's PC
    // bound 11774 next to the joined match. With the port taken, the bind below falls back to a free port.
    BOOL exclusive = TRUE;
    setsockopt(m_socket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, reinterpret_cast<const char*>(&exclusive), sizeof(exclusive));

    // Bind local port (try localPort first so UPnP port mapping works, then fallback to ephemeral port 0)
    sockaddr_in bindAddr = {};
    bindAddr.sin_family = AF_INET;
    bindAddr.sin_addr.s_addr = INADDR_ANY;
    bindAddr.sin_port = htons(localPort);

    if (bind(m_socket, reinterpret_cast<sockaddr*>(&bindAddr), sizeof(bindAddr)) != 0) {
        bindAddr.sin_port = 0;
        if (bind(m_socket, reinterpret_cast<sockaddr*>(&bindAddr), sizeof(bindAddr)) != 0) {
            NOBLE_ERROR("NetworkTransport: Failed to bind UDP socket (WSAError: {})", WSAGetLastError());
            closesocket(m_socket);
            m_socket = INVALID_SOCKET;
            return false;
        }
    }

    int bindLen = sizeof(bindAddr);
    if (getsockname(m_socket, reinterpret_cast<sockaddr*>(&bindAddr), &bindLen) == 0) {
        m_localPort = ntohs(bindAddr.sin_port);
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_peerTable.clear();
        m_addrToPeer.clear();
    }

    m_packetsSent.store(0);
    m_packetsRecv.store(0);
    m_lastUdpRecvTick.store(0);

    m_active.store(true);
    NOBLE_INFO("NetworkTransport: Initialized {} on UDP port {} (Local Machine ID: 0x{:X}, Host ID: 0x{:X}, Session ID: 0x{:X})",
        m_isHost ? "HOST" : "GUEST", m_localPort, m_localMachineId, m_hostMachineId, m_sessionId);

    // If Guest, fire an immediate NAT hole-punch PING packet to the Host so the local NAT mapping opens
    // and the Host learns our external IP:port before the first Blam! handshake packet.
    if (!m_isHost && m_remoteHostAddr.sin_addr.s_addr != 0) {
        uint8_t punch[sizeof(uint64_t) + 4] = { 0 };
        std::memcpy(punch, &m_localMachineId, sizeof(uint64_t));
        punch[sizeof(uint64_t)]     = 'P';
        punch[sizeof(uint64_t) + 1] = 'I';
        punch[sizeof(uint64_t) + 2] = 'N';
        punch[sizeof(uint64_t) + 3] = 'G';
        sendto(m_socket, reinterpret_cast<const char*>(punch), sizeof(punch), 0,
               reinterpret_cast<const sockaddr*>(&m_remoteHostAddr), sizeof(m_remoteHostAddr));
        NOBLE_INFO("NetworkTransport: [GUEST] Sent initial NAT punch PING to {}:{}", remoteIp, remotePort);
    }

    return true;
}

void NetworkTransport::Shutdown() {
    StopRelay();

    if (!m_active.exchange(false)) return;

    Sleep(50);

    SOCKET s = m_socket;
    m_socket = INVALID_SOCKET;
    if (s != INVALID_SOCKET) {
        closesocket(s);
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    m_peerTable.clear();
    m_addrToPeer.clear();

    NOBLE_INFO("NetworkTransport: Shutdown complete.");
}

void NetworkTransport::GetSessionGuid(uint8_t outGuid16[16]) const {
    if (!outGuid16) return;
    std::memcpy(&outGuid16[0], &m_sessionId, sizeof(m_sessionId));
    std::memcpy(&outGuid16[8], &m_sessionId, sizeof(m_sessionId));
}

void NetworkTransport::AddPeer(uint64_t machineId, const std::string& ip, uint16_t port) {
    if (ip.empty() || port == 0) return;
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) return;

    std::string key = ip + ":" + std::to_string(port);
    if (machineId != 0) {
        std::lock_guard<std::mutex> lock(m_mutex);
        // Do not overwrite an already-learned direct UDP peer address with a signaling hint
        if (m_peerTable.find(machineId) == m_peerTable.end()) {
            m_peerTable[machineId] = addr;
            m_addrToPeer[key] = machineId;
        }
    }

    // Fire UDP punch packet immediately to punch open NAT router firewall
    if (m_socket != INVALID_SOCKET) {
        uint8_t punch[sizeof(uint64_t) + 4] = { 0 };
        std::memcpy(punch, &m_localMachineId, sizeof(uint64_t));
        punch[sizeof(uint64_t)] = 'P';
        punch[sizeof(uint64_t) + 1] = 'I';
        punch[sizeof(uint64_t) + 2] = 'N';
        punch[sizeof(uint64_t) + 3] = 'G';
        sendto(m_socket, reinterpret_cast<const char*>(punch), sizeof(punch), 0,
               reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));
        NOBLE_INFO("NetworkTransport: Dispatched NAT punch datagram to {} (Peer 0x{:X})", key, machineId);
    }
}

void NetworkTransport::StartRelay(const std::string& relayUrl, const std::string& sessionId, bool isHost, uint64_t machineId) {
    if (relayUrl.empty()) return;
    {
        std::lock_guard<std::mutex> lock(m_relayMutex);
        if (m_relayActive.load() && m_relayUrl == relayUrl) {
            return;
        }
    }
    StopRelay();

    {
        std::lock_guard<std::mutex> lock(m_relayMutex);
        m_relayUrl = relayUrl;
    }
    m_relayActive.store(true);
    m_relayThread = std::thread(&NetworkTransport::RelayWorkerThread, this, relayUrl, sessionId, isHost, machineId);
    NOBLE_INFO("NetworkTransport: Starting WebSocket packet relay thread ({})", relayUrl);
}

void NetworkTransport::SetLobbyLaunchCallback(LobbyLaunchCallback cb) {
    std::lock_guard<std::mutex> lock(m_relayMutex);
    m_lobbyLaunchCb = std::move(cb);
}

void NetworkTransport::SetLobbyRosterCallback(LobbyRosterCallback cb) {
    std::lock_guard<std::mutex> lock(m_relayMutex);
    m_lobbyRosterCb = std::move(cb);
}

void NetworkTransport::SendLobbyControlJson(const std::string& jsonString) {
    std::lock_guard<std::mutex> lock(m_relayMutex);
    if (m_relayActive.load()) {
        m_relayTextSendQueue.push_back(jsonString);
    }
}

void NetworkTransport::StopRelay() {
    const bool wasActive = m_relayActive.exchange(false);
    if (wasActive) {
        std::lock_guard<std::mutex> lock(m_relayMutex);
        if (m_relayWsHandle) {
            WinHttpCloseHandle(static_cast<HINTERNET>(m_relayWsHandle));
            m_relayWsHandle = nullptr;
        }
        if (m_relaySessionHandle) {
            WinHttpCloseHandle(static_cast<HINTERNET>(m_relaySessionHandle));
            m_relaySessionHandle = nullptr;
        }
    }
    // Join even when the worker already stopped on its own (handshake failed, server closed the socket): a
    // finished-but-unjoined std::thread that gets a new thread assigned calls std::terminate, which would
    // end Noble with no crash log.
    if (m_relayThread.joinable() && m_relayThread.get_id() != std::this_thread::get_id()) {
        m_relayThread.join();
    }
    if (!wasActive) return;
    std::lock_guard<std::mutex> lock(m_relayMutex);
    m_relayUrl.clear();
    m_relaySendQueue.clear();
    m_relayRecvQueue.clear();
    m_relayTextSendQueue.clear();
    NOBLE_INFO("NetworkTransport: Stopped WebSocket packet relay.");
}

void NetworkTransport::RelayWorkerThread(std::string relayUrl, std::string sessionId, bool isHost, uint64_t machineId) {
    (void)sessionId;
    (void)isHost;
    (void)machineId;

    // Convert ws:// or wss:// to http/https for WinHttpCrackUrl
    std::string httpUrl = relayUrl;
    bool isHttps = true;
    if (httpUrl.rfind("wss://", 0) == 0) {
        httpUrl = "https://" + httpUrl.substr(6);
        isHttps = true;
    } else if (httpUrl.rfind("ws://", 0) == 0) {
        httpUrl = "http://" + httpUrl.substr(5);
        isHttps = false;
    }

    URL_COMPONENTS parts{};
    parts.dwStructSize = sizeof(parts);
    parts.dwSchemeLength = static_cast<DWORD>(-1);
    parts.dwHostNameLength = static_cast<DWORD>(-1);
    parts.dwUrlPathLength = static_cast<DWORD>(-1);
    parts.dwExtraInfoLength = static_cast<DWORD>(-1);

    std::wstring wideUrl = Widen(httpUrl);
    if (!WinHttpCrackUrl(wideUrl.c_str(), static_cast<DWORD>(wideUrl.size()), 0, &parts)) {
        NOBLE_WARN("NetworkTransport: Relay URL parsing failed for '{}'", relayUrl);
        m_relayActive.store(false);
        return;
    }

    std::wstring host(parts.lpszHostName, parts.dwHostNameLength);
    INTERNET_PORT port = parts.nPort ? parts.nPort : (isHttps ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT);
    std::wstring path = parts.dwUrlPathLength > 0 ? std::wstring(parts.lpszUrlPath, parts.dwUrlPathLength) : L"/ws/relay";
    if (parts.dwExtraInfoLength > 0) {
        path += std::wstring(parts.lpszExtraInfo, parts.dwExtraInfoLength);
    }

    HINTERNET hSession = WinHttpOpen(L"ProjectNoble/0.1", WINHTTP_ACCESS_TYPE_NO_PROXY, nullptr, nullptr, 0);
    if (!hSession) {
        m_relayActive.store(false);
        return;
    }
    WinHttpSetTimeouts(hSession, 5000, 5000, 5000, 5000);

    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), port, 0);
    HINTERNET hRequest = nullptr;
    if (hConnect) {
        DWORD flags = isHttps ? WINHTTP_FLAG_SECURE : 0;
        hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(), nullptr,
                                      WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    }

    HINTERNET hWebSocket = nullptr;
    if (hRequest) {
        if (WinHttpSetOption(hRequest, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET, nullptr, 0) &&
            WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, nullptr, 0, 0, 0) &&
            WinHttpReceiveResponse(hRequest, nullptr)) {
            hWebSocket = WinHttpWebSocketCompleteUpgrade(hRequest, 0);
        }
        WinHttpCloseHandle(hRequest);
    }

    if (!hWebSocket) {
        NOBLE_WARN("NetworkTransport: WebSocket upgrade handshake failed on relay");
        if (hConnect) WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        m_relayActive.store(false);
        return;
    }

    // Set a short 15 ms receive timeout on the upgraded WebSocket handle so WinHttpWebSocketReceive
    // does not block outgoing packet flushing or shutdown for 5 seconds.
    WinHttpSetTimeouts(hWebSocket, 5000, 5000, 1000, 15);
    {
        std::lock_guard<std::mutex> lock(m_relayMutex);
        m_relaySessionHandle = hSession;
        m_relayWsHandle = hWebSocket;
    }
    NOBLE_INFO("NetworkTransport: WebSocket relay connected successfully!");

    uint8_t recvBuffer[4096];

    while (m_relayActive.load()) {
        // 1. Send queued outgoing control text frames and binary packets
        std::vector<std::string> toSendText;
        std::vector<std::vector<uint8_t>> toSend;
        {
            std::lock_guard<std::mutex> lock(m_relayMutex);
            while (!m_relayTextSendQueue.empty()) {
                toSendText.push_back(std::move(m_relayTextSendQueue.front()));
                m_relayTextSendQueue.pop_front();
            }
            while (!m_relaySendQueue.empty()) {
                toSend.push_back(std::move(m_relaySendQueue.front()));
                m_relaySendQueue.pop_front();
            }
        }

        for (const auto& text : toSendText) {
            if (!text.empty()) {
                WinHttpWebSocketSend(hWebSocket, WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE,
                                     const_cast<char*>(text.data()), static_cast<DWORD>(text.size()));
            }
        }

        for (const auto& pkt : toSend) {
            if (!pkt.empty() && m_relayActive.load()) {
                WinHttpWebSocketSend(hWebSocket, WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE,
                                     const_cast<uint8_t*>(pkt.data()), static_cast<DWORD>(pkt.size()));
            }
        }

        if (!m_relayActive.load()) break;

        // 2. Poll for incoming packet (receive timeout ~15ms)
        DWORD bytesRead = 0;
        WINHTTP_WEB_SOCKET_BUFFER_TYPE bufferType;
        DWORD err = WinHttpWebSocketReceive(hWebSocket, recvBuffer, sizeof(recvBuffer), &bytesRead, &bufferType);

        if (err == NO_ERROR && bytesRead > 0) {
            if (bufferType == WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE ||
                (bytesRead >= 2 && recvBuffer[0] == '{')) {
                // Parse JSON control message (lobby_launch, lobby_roster)
                std::string msgStr(reinterpret_cast<char*>(recvBuffer), bytesRead);
                try {
                    auto j = nlohmann::json::parse(msgStr);
                    std::string type = j.value("type", "");
                    if (type == "lobby_launch") {
                        auto ld = j.value("launch", nlohmann::json::object());
                        std::string lMap = ld.value("map", "20_sword_slayer");
                        std::string lMode = ld.value("mode", "multiplayer");
                        std::string lVar = ld.value("variant", "");
                        int lDiff = ld.value("difficulty", 1);
                        uint64_t lSkulls = 0;
                        try { lSkulls = std::stoull(ld.value("skulls", "0")); } catch (...) {}
                        std::string hIp = ld.value("host_ip", "");
                        uint16_t hPort = static_cast<uint16_t>(ld.value("host_port", 11774));
                        std::string sId = ld.value("session_id", sessionId);

                        LobbyLaunchCallback cb;
                        {
                            std::lock_guard<std::mutex> lock(m_relayMutex);
                            cb = m_lobbyLaunchCb;
                        }
                        if (cb) {
                            cb(lMap, lMode, lVar, lDiff, lSkulls, hIp, hPort, sId);
                        }
                    } else if (type == "lobby_roster") {
                        LobbyRosterCallback cb;
                        {
                            std::lock_guard<std::mutex> lock(m_relayMutex);
                            cb = m_lobbyRosterCb;
                        }
                        if (cb) {
                            cb(msgStr);
                        }
                    }
                } catch (...) {}
            } else if (bytesRead >= sizeof(uint64_t)) {
                std::vector<uint8_t> pkt(recvBuffer, recvBuffer + bytesRead);
                std::lock_guard<std::mutex> lock(m_relayMutex);
                if (m_relayRecvQueue.size() < 100) {
                    m_relayRecvQueue.push_back(std::move(pkt));
                }
            }
        } else if (err == ERROR_WINHTTP_TIMEOUT) {
            // normal idle
        } else if (err != NO_ERROR) {
            // disconnected or error
            break;
        }
    }

    HINTERNET wsToClose = nullptr;
    HINTERNET sessionToClose = nullptr;
    {
        std::lock_guard<std::mutex> lock(m_relayMutex);
        wsToClose = static_cast<HINTERNET>(m_relayWsHandle);
        sessionToClose = static_cast<HINTERNET>(m_relaySessionHandle);
        m_relayWsHandle = nullptr;
        m_relaySessionHandle = nullptr;
    }
    if (wsToClose) {
        WinHttpWebSocketClose(wsToClose, WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS, nullptr, 0);
        WinHttpCloseHandle(wsToClose);
    }
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (sessionToClose) WinHttpCloseHandle(sessionToClose);
    // Dropped by the server (redeploy, timeout) rather than stopped by us: mark the relay inactive so the next
    // StartRelay (the host's heartbeat every 3 s) connects again. Before, it stayed "active" and never came back.
    if (m_relayActive.exchange(false)) {
        NOBLE_WARN("NetworkTransport: WebSocket relay closed by the server; it reconnects on the next master-server heartbeat");
    }
}

uint64_t NetworkTransport::OnNetSend(uint64_t targetPeerId, const uint8_t* data, size_t size) {
    if (!m_active.load() || m_socket == INVALID_SOCKET || !data || size == 0) {
        return 0;
    }

    // Wire packet: [uint64_t senderMachineId] [raw Blam! engine payload]
    const size_t totalBytes = sizeof(uint64_t) + size;
    uint8_t stackBuf[2048];
    std::vector<uint8_t> heapBuf;
    uint8_t* wireData = stackBuf;

    if (totalBytes > sizeof(stackBuf)) {
        heapBuf.resize(totalBytes);
        wireData = heapBuf.data();
    }

    std::memcpy(wireData, &m_localMachineId, sizeof(uint64_t));
    std::memcpy(wireData + sizeof(uint64_t), data, size);

    uint64_t packetNum = ++m_packetsSent;
    if (packetNum <= kPacketDumpLimit) {
        NOBLE_INFO("NetworkTransport: [{}] TX #{} -> 0x{:X} len={} | {}", m_isHost ? "HOST" : "GUEST",
                   packetNum, targetPeerId, size, HexDump(data, size));
    }

    // Only queue a copy to the WebSocket relay when direct UDP has not recently delivered packets.
    // Sending every packet over both UDP and WebSocket simultaneously causes every packet to arrive
    // twice (with the relay copy delayed ~50ms), corrupting Blam!'s sliding-window packet sequence.
    const uint64_t lastUdpTick = m_lastUdpRecvTick.load();
    const bool directUdpAlive = (lastUdpTick != 0 && (GetTickCount64() - lastUdpTick) < 3000);
    if (m_relayActive.load() && !directUdpAlive) {
        std::lock_guard<std::mutex> lock(m_relayMutex);
        if (m_relaySendQueue.size() < 100) {
            m_relaySendQueue.emplace_back(wireData, wireData + totalBytes);
        }
    }

    if (!m_isHost) {
        // Direct UDP send to host
        int sent = sendto(m_socket, reinterpret_cast<const char*>(wireData), static_cast<int>(totalBytes), 0,
                          reinterpret_cast<const sockaddr*>(&m_remoteHostAddr), sizeof(m_remoteHostAddr));
        if (packetNum <= 15 || (packetNum % 500) == 0) {
            NOBLE_INFO("NetworkTransport: [GUEST] Sent packet #{} to Host 0x{:X} ({} payload bytes, result: {}, relayFallback: {})",
                       packetNum, targetPeerId, size, sent, !directUdpAlive && m_relayActive.load());
        }
        return (sent > 0) ? static_cast<uint64_t>(size) : static_cast<uint64_t>(size);
    }

    // Host sends traffic to specific peer or broadcasts to all known peers if targetPeerId is unknown/broadcast
    std::vector<sockaddr_in> targets;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_peerTable.find(targetPeerId);
        if (it != m_peerTable.end()) {
            targets.push_back(it->second);
        } else {
            targets.reserve(m_peerTable.size());
            for (const auto& kv : m_peerTable) {
                targets.push_back(kv.second);
            }
        }
    }

    for (const auto& destAddr : targets) {
        int sent = sendto(m_socket, reinterpret_cast<const char*>(wireData), static_cast<int>(totalBytes), 0,
                          reinterpret_cast<const sockaddr*>(&destAddr), sizeof(destAddr));
        if (packetNum <= 15 || (packetNum % 500) == 0) {
            char ipStr[INET_ADDRSTRLEN] = { 0 };
            inet_ntop(AF_INET, &destAddr.sin_addr, ipStr, sizeof(ipStr));
            NOBLE_INFO("NetworkTransport: [HOST] Sent packet #{} to Peer 0x{:X} ({}:{}) ({} payload bytes, result: {})",
                       packetNum, targetPeerId, ipStr, ntohs(destAddr.sin_port), size, sent);
        }
    }

    return static_cast<uint64_t>(size);
}

uint64_t NetworkTransport::OnNetRecv(uint8_t* outBuffer, size_t bufferCapacity, uint64_t* outSenderPeerId) {
    if (!m_active.load() || m_socket == INVALID_SOCKET || !outBuffer || bufferCapacity == 0) {
        return 0;
    }

    sockaddr_in senderAddr = {};
    int senderLen = sizeof(senderAddr);
    uint8_t rawBuf[4096];

    // 1. Check direct UDP socket (loop past any NAT hole-punch PING datagrams after registering peer address)
    int received = -1;
    while (true) {
        senderLen = sizeof(senderAddr);
        received = recvfrom(m_socket, reinterpret_cast<char*>(rawBuf), sizeof(rawBuf), 0,
                            reinterpret_cast<sockaddr*>(&senderAddr), &senderLen);
        if (received <= static_cast<int>(sizeof(uint64_t))) {
            break;
        }
        m_lastUdpRecvTick.store(GetTickCount64());
        if (received == static_cast<int>(sizeof(uint64_t) + 4) &&
            rawBuf[8] == 'P' && rawBuf[9] == 'I' && rawBuf[10] == 'N' && rawBuf[11] == 'G') {
            uint64_t pingSenderId = 0;
            std::memcpy(&pingSenderId, rawBuf, sizeof(uint64_t));
            if (m_isHost && pingSenderId != 0 && pingSenderId != m_localMachineId) {
                char ipStr[INET_ADDRSTRLEN] = { 0 };
                inet_ntop(AF_INET, &senderAddr.sin_addr, ipStr, sizeof(ipStr));
                std::string addrKey = std::string(ipStr) + ":" + std::to_string(ntohs(senderAddr.sin_port));

                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_peerTable[pingSenderId] = senderAddr;
                    m_addrToPeer[addrKey] = pingSenderId;
                }
                // Echo PING back to Guest over direct UDP so Guest immediately confirms direct UDP is open
                uint8_t pong[sizeof(uint64_t) + 4] = { 0 };
                std::memcpy(pong, &m_localMachineId, sizeof(uint64_t));
                pong[sizeof(uint64_t)]     = 'P';
                pong[sizeof(uint64_t) + 1] = 'I';
                pong[sizeof(uint64_t) + 2] = 'N';
                pong[sizeof(uint64_t) + 3] = 'G';
                sendto(m_socket, reinterpret_cast<const char*>(pong), sizeof(pong), 0,
                       reinterpret_cast<const sockaddr*>(&senderAddr), sizeof(senderAddr));
                NOBLE_INFO("NetworkTransport: [HOST] Registered peer 0x{:X} via NAT punch PING from {} (echoed PING)",
                           pingSenderId, addrKey);
            }
            received = -1;
            continue;
        }
        break;
    }

    // 2. If direct UDP had no game packet, check WebSocket relay queue (only when direct UDP is not active)
    bool fromRelay = false;
    if (received <= static_cast<int>(sizeof(uint64_t))) {
        if (m_relayActive.load()) {
            const uint64_t lastUdpTick = m_lastUdpRecvTick.load();
            const bool directUdpAlive = (lastUdpTick != 0 && (GetTickCount64() - lastUdpTick) < 3000);
            std::lock_guard<std::mutex> lock(m_relayMutex);
            if (directUdpAlive) {
                m_relayRecvQueue.clear();
            } else {
                while (!m_relayRecvQueue.empty()) {
                    auto pkt = std::move(m_relayRecvQueue.front());
                    m_relayRecvQueue.pop_front();
                    if (pkt.size() == sizeof(uint64_t) + 4 &&
                        pkt[8] == 'P' && pkt[9] == 'I' && pkt[10] == 'N' && pkt[11] == 'G') {
                        continue;
                    }
                    if (pkt.size() > sizeof(uint64_t)) {
                        received = static_cast<int>(std::min(pkt.size(), sizeof(rawBuf)));
                        std::memcpy(rawBuf, pkt.data(), received);
                        fromRelay = true;
                        break;
                    }
                }
            }
        }
    }

    if (received <= static_cast<int>(sizeof(uint64_t))) {
        return 0;
    }

    // Extract sender machine ID from wire header
    uint64_t senderMachineId = 0;
    std::memcpy(&senderMachineId, rawBuf, sizeof(uint64_t));
    size_t payloadLen = static_cast<size_t>(received) - sizeof(uint64_t);

    if (payloadLen > bufferCapacity) {
        payloadLen = bufferCapacity;
    }

    std::memcpy(outBuffer, rawBuf + sizeof(uint64_t), payloadLen);

    if (outSenderPeerId) {
        *outSenderPeerId = senderMachineId;
    }

    uint64_t recvNum = ++m_packetsRecv;
    if (recvNum <= kPacketDumpLimit) {
        NOBLE_INFO("NetworkTransport: [{}] RX #{} <- 0x{:X} via {} len={} | {}", m_isHost ? "HOST" : "GUEST",
                   recvNum, senderMachineId, fromRelay ? "RELAY" : "UDP", payloadLen, HexDump(outBuffer, payloadLen));
    }

    if (!m_isHost) {
        if (recvNum <= 10 || (recvNum % 500) == 0) {
            NOBLE_INFO("NetworkTransport: [GUEST] Received packet #{} from Host 0x{:X} via {} ({} payload bytes)",
                       recvNum, senderMachineId, fromRelay ? "RELAY" : "UDP", payloadLen);
        }
        return static_cast<uint64_t>(payloadLen);
    }

    // Host received packet from remote guest
    if (!fromRelay) {
        char ipStr[INET_ADDRSTRLEN] = { 0 };
        inet_ntop(AF_INET, &senderAddr.sin_addr, ipStr, sizeof(ipStr));
        std::string addrKey = std::string(ipStr) + ":" + std::to_string(ntohs(senderAddr.sin_port));

        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_peerTable.find(senderMachineId);
        if (it == m_peerTable.end() ||
            it->second.sin_addr.s_addr != senderAddr.sin_addr.s_addr ||
            it->second.sin_port != senderAddr.sin_port) {
            m_peerTable[senderMachineId] = senderAddr;
            m_addrToPeer[addrKey] = senderMachineId;
            NOBLE_INFO("NetworkTransport: Registered remote peer 0x{:X} at {}",
                       senderMachineId, addrKey);
        }
    }

    if (recvNum <= 10 || (recvNum % 500) == 0) {
        NOBLE_INFO("NetworkTransport: [HOST] Received packet #{} from Peer 0x{:X} via {} ({} payload bytes)",
                   recvNum, senderMachineId, fromRelay ? "RELAY" : "UDP", payloadLen);
    }

    return static_cast<uint64_t>(payloadLen);
}

} // namespace noble
