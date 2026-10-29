#include "NetworkTransport.hpp"
#include <nlohmann/json.hpp>
#include "common/Logger.hpp"

#include <windows.h>
#include <winhttp.h>
#include <random>
#include <cstring>
#include <chrono>
#include <atomic>

namespace noble {

namespace {

// Diagnostic: hex-dump Blam! payloads so the join handshake (55-byte join request,
// 16-byte host reply, 26/10-byte channel traffic) can be decoded from the log.
constexpr uint64_t kPacketDumpLimit = 60;
// [uint64_t senderMachineId][uint16_t port] in front of every game packet (see OnNetSend).
constexpr size_t kWireHeaderBytes = sizeof(uint64_t) + sizeof(uint16_t);
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

    // Bind local port:
    // - Host binds localPort (11774) so UPnP / port-forwarding and LAN/WAN guests reach it. Retry briefly
    //   in case the parent menu process from Relaunch::Start is still releasing 11774, then fall back to 0.
    // - Guest ALWAYS binds an ephemeral port (0) so it never steals UDP 11774 from a local Host or receives
    //   its own outbound packets addressed to 11774.
    sockaddr_in bindAddr = {};
    bindAddr.sin_family = AF_INET;
    bindAddr.sin_addr.s_addr = INADDR_ANY;
    bindAddr.sin_port = htons(isHost ? localPort : 0);

    bool bound = (bind(m_socket, reinterpret_cast<sockaddr*>(&bindAddr), sizeof(bindAddr)) == 0);
    if (!bound && isHost && localPort != 0) {
        for (int attempt = 0; attempt < 10 && !bound; ++attempt) {
            Sleep(25);
            bound = (bind(m_socket, reinterpret_cast<sockaddr*>(&bindAddr), sizeof(bindAddr)) == 0);
        }
    }
    if (!bound) {
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
        // Also send a localhost punch to 127.0.0.1:remotePort so same-PC Online P2P tests connect directly
        // without relying on router Hairpin NAT (harmless if the Host is on a remote PC since Guest is on an ephemeral port).
        if (m_remoteHostAddr.sin_addr.s_addr != htonl(INADDR_LOOPBACK)) {
            sockaddr_in localHostAddr = {};
            localHostAddr.sin_family = AF_INET;
            localHostAddr.sin_port = m_remoteHostAddr.sin_port;
            localHostAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            sendto(m_socket, reinterpret_cast<const char*>(punch), sizeof(punch), 0,
                   reinterpret_cast<const sockaddr*>(&localHostAddr), sizeof(localHostAddr));
        }
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

    m_relayStopRequested.store(false);
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

void NetworkTransport::SetLobbyClosedCallback(LobbyClosedCallback cb) {
    std::lock_guard<std::mutex> lock(m_relayMutex);
    m_lobbyClosedCb = std::move(cb);
}

void NetworkTransport::FlushHeldPackets(uint64_t peerId, const sockaddr_in& destAddr) {
    std::deque<HeldPacket> toFlush;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_heldPackets.find(peerId);
        if (it != m_heldPackets.end()) {
            toFlush = std::move(it->second);
            m_heldPackets.erase(it);
        }
    }
    if (toFlush.empty()) return;

    const uint64_t now = GetTickCount64();
    size_t count = 0;
    for (const auto& pkt : toFlush) {
        if (now - pkt.enqueueTick > 5000) continue;
        sendto(m_socket, reinterpret_cast<const char*>(pkt.data.data()),
               static_cast<int>(pkt.data.size()), 0,
               reinterpret_cast<const sockaddr*>(&destAddr), sizeof(destAddr));
        count++;
    }
    if (count > 0) {
        NOBLE_INFO("NetworkTransport: Flushed {} held packet(s) to peer 0x{:X}", count, peerId);
    }
}

uint64_t NetworkTransport::ComputePacketHash(const uint8_t* data, size_t size) const {
    uint64_t hash = 14695981039346656037ULL;
    for (size_t i = 0; i < size; ++i) {
        hash ^= static_cast<uint64_t>(data[i]);
        hash *= 1099511628211ULL;
    }
    return hash;
}

bool NetworkTransport::IsDuplicatePacket(uint64_t hash) {
    if (hash == 0) return false;
    for (size_t i = 0; i < kDedupeBufferSize; ++i) {
        if (m_dedupeHashes[i] == hash) {
            m_duplicatesDropped++;
            return true;
        }
    }
    m_dedupeHashes[m_dedupeIndex] = hash;
    m_dedupeIndex = (m_dedupeIndex + 1) % kDedupeBufferSize;
    return false;
}

NetworkTransport::NetworkProfile NetworkTransport::GetNetworkProfile() const {
    std::lock_guard<std::mutex> lock(m_profMutex);
    const uint64_t now = GetTickCount64();
    const uint64_t elapsed = (m_lastProfTick == 0) ? 0 : (now - m_lastProfTick);

    const uint64_t curSentBytes = m_bytesSent.load();
    const uint64_t curRecvBytes = m_bytesRecv.load();
    const uint64_t curSentPkts = m_packetsSent.load();
    const uint64_t curRecvPkts = m_packetsRecv.load();

    if (elapsed >= 1000) {
        const double secs = static_cast<double>(elapsed) / 1000.0;
        m_sendRateBps = static_cast<uint32_t>((curSentBytes - m_lastSentBytesSnapshot) / secs);
        m_recvRateBps = static_cast<uint32_t>((curRecvBytes - m_lastRecvBytesSnapshot) / secs);
        m_packetsSentPerSec = static_cast<uint32_t>((curSentPkts - m_lastSentPacketsSnapshot) / secs);
        m_packetsRecvPerSec = static_cast<uint32_t>((curRecvPkts - m_lastRecvPacketsSnapshot) / secs);

        m_lastProfTick = now;
        m_lastSentBytesSnapshot = curSentBytes;
        m_lastRecvBytesSnapshot = curRecvBytes;
        m_lastSentPacketsSnapshot = curSentPkts;
        m_lastRecvPacketsSnapshot = curRecvPkts;
    } else if (m_lastProfTick == 0) {
        m_lastProfTick = now;
        m_lastSentBytesSnapshot = curSentBytes;
        m_lastRecvBytesSnapshot = curRecvBytes;
        m_lastSentPacketsSnapshot = curSentPkts;
        m_lastRecvPacketsSnapshot = curRecvPkts;
    }

    NetworkProfile prof;
    prof.packetsSent = curSentPkts;
    prof.packetsRecv = curRecvPkts;
    prof.bytesSent = curSentBytes;
    prof.bytesRecv = curRecvBytes;
    prof.sendRateBps = m_sendRateBps;
    prof.recvRateBps = m_recvRateBps;
    prof.packetsSentPerSec = m_packetsSentPerSec;
    prof.packetsRecvPerSec = m_packetsRecvPerSec;
    prof.duplicatePacketsDropped = m_duplicatesDropped.load();

    {
        std::lock_guard<std::mutex> lk(const_cast<std::mutex&>(m_mutex));
        uint64_t heldTotal = 0;
        for (const auto& kv : m_heldPackets) {
            heldTotal += kv.second.size();
        }
        prof.heldPacketsCount = heldTotal;
    }

    const uint64_t lastUdpTick = m_lastUdpRecvTick.load();
    prof.directUdpActive = (lastUdpTick != 0 && (now - lastUdpTick) < 3000);
    prof.relayActive = m_relayActive.load();

    return prof;
}

void NetworkTransport::SendLobbyControlJson(const std::string& jsonString) {
    std::lock_guard<std::mutex> lock(m_relayMutex);
    if (m_relayActive.load()) {
        m_relayTextSendQueue.push_back(jsonString);
    }
}

void NetworkTransport::StopRelay() {
    m_relayStopRequested.store(true);
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

    while (!m_relayStopRequested.load()) {
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
            if (m_relayStopRequested.load()) break;
            for (int i = 0; i < 20 && !m_relayStopRequested.load(); ++i) Sleep(100);
            continue;
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
            if (hConnect) WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            if (m_relayStopRequested.load()) break;
            for (int i = 0; i < 20 && !m_relayStopRequested.load(); ++i) Sleep(100);
            continue;
        }

        WinHttpSetTimeouts(hWebSocket, 5000, 5000, 1000, 15);
        {
            std::lock_guard<std::mutex> lock(m_relayMutex);
            m_relaySessionHandle = hSession;
            m_relayWsHandle = hWebSocket;
        }
        m_relayActive.store(true);
        NOBLE_INFO("NetworkTransport: WebSocket relay connected successfully!");

        uint8_t recvBuffer[4096];

        while (!m_relayStopRequested.load() && m_relayActive.load()) {
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

            if (m_relayStopRequested.load() || !m_relayActive.load()) break;

            // 2. Poll for incoming packet (receive timeout ~15ms)
            DWORD bytesRead = 0;
            WINHTTP_WEB_SOCKET_BUFFER_TYPE bufferType;
            DWORD err = WinHttpWebSocketReceive(hWebSocket, recvBuffer, sizeof(recvBuffer), &bytesRead, &bufferType);

            if (err == NO_ERROR && bytesRead > 0) {
                if (bufferType == WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE ||
                    (bytesRead >= 2 && recvBuffer[0] == '{')) {
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
                        } else if (type == "lobby_closed") {
                            std::string reason = j.value("reason", "Host closed the lobby");
                            LobbyClosedCallback cb;
                            {
                                std::lock_guard<std::mutex> lock(m_relayMutex);
                                cb = m_lobbyClosedCb;
                            }
                            if (cb) {
                                cb(reason);
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

        if (m_relayStopRequested.load()) {
            m_relayActive.store(false);
            break;
        }

        NOBLE_WARN("NetworkTransport: WebSocket relay disconnected; reconnecting in 2s...");
        for (int i = 0; i < 20 && !m_relayStopRequested.load(); ++i) {
            Sleep(100);
        }
    }
}

// Wire packet: [uint64_t senderMachineId][uint16_t port][raw Blam! payload]
//
// haloreach.dll has two network endpoints, port 1000 (channel traffic) and port 1001 (out-of-band: join,
// connect-request/establish, boot...). Slot 41 is send(target, data, size, port) with the DESTINATION port and
// slot 43 is recv(buffer, capacity, &sender, port) with the endpoint's own port (hr+0x43AB88 / hr+0x43AA78).
// The packet dispatcher (hr+0x4235E4) treats whatever the port-1001 endpoint reads as out-of-band and whatever
// the port-1000 endpoint reads as channel data. Before 2026-10-29 the port was dropped and both endpoints
// pulled from one queue, so channel packets were randomly handled as out-of-band (and the reverse): the host
// ignored most connect-establish replies and joins timed out. The port now travels with each packet and every
// endpoint only gets packets for its port.
uint64_t NetworkTransport::OnNetSend(uint64_t targetPeerId, const uint8_t* data, size_t size, uint16_t port) {
    if (!m_active.load() || m_socket == INVALID_SOCKET || !data || size == 0) {
        return 0;
    }

    const size_t totalBytes = kWireHeaderBytes + size;
    if (size > 4088) {
        static std::atomic<int> s_bigSendLogs{0};
        if (s_bigSendLogs.fetch_add(1) < 20) {
            NOBLE_INFO("NetworkTransport: [{}] large packet {} bytes -> 0x{:X} port {}", m_isHost ? "HOST" : "GUEST", size,
                       targetPeerId, port);
        }
    }
    uint8_t stackBuf[2048];
    std::vector<uint8_t> heapBuf;
    uint8_t* wireData = stackBuf;

    if (totalBytes > sizeof(stackBuf)) {
        heapBuf.resize(totalBytes);
        wireData = heapBuf.data();
    }

    std::memcpy(wireData, &m_localMachineId, sizeof(uint64_t));
    std::memcpy(wireData + sizeof(uint64_t), &port, sizeof(uint16_t));
    std::memcpy(wireData + kWireHeaderBytes, data, size);

    m_bytesSent += totalBytes;
    uint64_t packetNum = ++m_packetsSent;
    if (packetNum <= kPacketDumpLimit) {
        NOBLE_INFO("NetworkTransport: [{}] TX #{} -> 0x{:X} port {} len={} | {}", m_isHost ? "HOST" : "GUEST",
                   packetNum, targetPeerId, port, size, HexDump(data, size));
    }

    const uint64_t nowTick = GetTickCount64();

    if (!m_isHost) {
        // Guest sends directly to host
        int sent = sendto(m_socket, reinterpret_cast<const char*>(wireData), static_cast<int>(totalBytes), 0,
                          reinterpret_cast<const sockaddr*>(&m_remoteHostAddr), sizeof(m_remoteHostAddr));

        const uint64_t lastUdpTick = m_lastUdpRecvTick.load();
        const bool directUdpAlive = (lastUdpTick != 0 && (nowTick - lastUdpTick) < 3000);

        // If direct UDP is not yet confirmed and target is a WAN IP, also probe 127.0.0.1:<hostPort>
        // in case Host and Guest are running on the same PC behind a router without Hairpin NAT.
        if (!directUdpAlive && m_remoteHostAddr.sin_addr.s_addr != 0 &&
            m_remoteHostAddr.sin_addr.s_addr != htonl(INADDR_LOOPBACK)) {
            sockaddr_in localHostAddr = {};
            localHostAddr.sin_family = AF_INET;
            localHostAddr.sin_port = m_remoteHostAddr.sin_port;
            localHostAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            sendto(m_socket, reinterpret_cast<const char*>(wireData), static_cast<int>(totalBytes), 0,
                   reinterpret_cast<const sockaddr*>(&localHostAddr), sizeof(localHostAddr));
        }

        // Fallback to relay if direct UDP not confirmed active
        if (m_relayActive.load() && !directUdpAlive) {
            std::lock_guard<std::mutex> lock(m_relayMutex);
            if (m_relaySendQueue.size() < 100) {
                m_relaySendQueue.emplace_back(wireData, wireData + totalBytes);
            }
        }

        if (packetNum <= 15 || (packetNum % 500) == 0) {
            NOBLE_INFO("NetworkTransport: [GUEST] Sent packet #{} to Host 0x{:X} ({} payload bytes, result: {}, relayFallback: {})",
                       packetNum, targetPeerId, size, sent, !directUdpAlive && m_relayActive.load());
        }
        return static_cast<uint64_t>(size);
    }

    // Host sends traffic:
    // If target is broadcast (0 or ~0ULL), broadcast to all known peers
    if (targetPeerId == 0 || targetPeerId == 0xFFFFFFFFFFFFFFFFULL) {
        std::vector<sockaddr_in> targets;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            targets.reserve(m_peerTable.size());
            for (const auto& kv : m_peerTable) {
                targets.push_back(kv.second);
            }
        }
        for (const auto& destAddr : targets) {
            sendto(m_socket, reinterpret_cast<const char*>(wireData), static_cast<int>(totalBytes), 0,
                   reinterpret_cast<const sockaddr*>(&destAddr), sizeof(destAddr));
        }
        return static_cast<uint64_t>(size);
    }

    // Target is a specific peer:
    bool foundInUdp = false;
    sockaddr_in destAddr{};
    bool directConfirmed = false;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_peerTable.find(targetPeerId);
        if (it != m_peerTable.end()) {
            destAddr = it->second;
            foundInUdp = true;
            auto rIt = m_peerRouteStates.find(targetPeerId);
            if (rIt != m_peerRouteStates.end()) {
                directConfirmed = rIt->second.directConfirmed && (nowTick - rIt->second.lastUdpRecvTick < 3000);
            }
        } else {
            // Target route unknown yet: hold packet (Star-routing) instead of broadcasting (Finding #9, #10)
            auto& q = m_heldPackets[targetPeerId];
            if (q.size() < 64) {
                q.push_back({ std::vector<uint8_t>(wireData, wireData + totalBytes), nowTick });
            }
        }
    }

    if (foundInUdp) {
        int sent = sendto(m_socket, reinterpret_cast<const char*>(wireData), static_cast<int>(totalBytes), 0,
                          reinterpret_cast<const sockaddr*>(&destAddr), sizeof(destAddr));
        if (packetNum <= 15 || (packetNum % 500) == 0) {
            char ipStr[INET_ADDRSTRLEN] = { 0 };
            inet_ntop(AF_INET, &destAddr.sin_addr, ipStr, sizeof(ipStr));
            NOBLE_INFO("NetworkTransport: [HOST] Sent packet #{} to Peer 0x{:X} ({}:{}) ({} payload bytes, result: {})",
                       packetNum, targetPeerId, ipStr, ntohs(destAddr.sin_port), size, sent);
        }
    }

    // If direct UDP is not confirmed bidirectional, or peer UDP route is unknown, fallback via relay
    if (m_relayActive.load() && (!foundInUdp || !directConfirmed)) {
        std::lock_guard<std::mutex> lock(m_relayMutex);
        if (m_relaySendQueue.size() < 100) {
            m_relaySendQueue.emplace_back(wireData, wireData + totalBytes);
        }
    }

    return static_cast<uint64_t>(size);
}

// Splits one wire packet into the per-port queues. Returns false for packets that are not game traffic.
// Duplicate filtering only runs while the relay is active: its job is to drop the second copy of a packet that
// arrived over both UDP and the relay. The game itself resends identical packets on purpose (keepalives,
// connect-request retries), so filtering on direct UDP alone would eat those. The hash covers sender id and
// port too, so identical packets from two different guests are never confused.
bool NetworkTransport::QueueWirePacket(const uint8_t* wire, size_t wireLen, bool fromRelay) {
    if (wireLen <= kWireHeaderBytes) {
        return false;
    }
    uint64_t sender = 0;
    std::memcpy(&sender, wire, sizeof(uint64_t));
    if (sender == 0 || sender == m_localMachineId) {
        return false; // Never queue our own packets
    }
    if (m_relayActive.load() && IsDuplicatePacket(ComputePacketHash(wire, wireLen))) {
        return false;
    }
    QueuedPacket pkt;
    pkt.sender = sender;
    uint16_t port = 0;
    std::memcpy(&port, wire + sizeof(uint64_t), sizeof(uint16_t));
    pkt.fromRelay = fromRelay;
    pkt.data.assign(wire + kWireHeaderBytes, wire + wireLen);
    auto& q = m_portQueues[port];
    if (q.size() >= 512) {
        q.pop_front();  // never let a port nobody reads grow without bound
    }
    q.push_back(std::move(pkt));
    return true;
}

uint64_t NetworkTransport::OnNetRecv(uint8_t* outBuffer, size_t bufferCapacity, uint64_t* outSenderPeerId, uint16_t port) {
    if (!m_active.load() || m_socket == INVALID_SOCKET || !outBuffer || bufferCapacity == 0) {
        return 0;
    }

    std::lock_guard<std::mutex> recvLock(m_recvMutex);

    // 1. Drain the UDP socket into the per-port queues (NAT hole-punch PINGs are handled here, not queued).
    // haloreach.dll accepts channel packets up to 0x3000 bytes (hr+0x43BC00); 64 KB = largest UDP datagram, so
    // nothing is truncated (a too-small buffer makes recvfrom fail with WSAEMSGSIZE and lose the datagram).
    static thread_local uint8_t rawBuf[65536];
    for (int drained = 0; drained < 256; ++drained) {
        sockaddr_in senderAddr = {};
        int senderLen = sizeof(senderAddr);
        const int received = recvfrom(m_socket, reinterpret_cast<char*>(rawBuf), sizeof(rawBuf), 0,
                                      reinterpret_cast<sockaddr*>(&senderAddr), &senderLen);
        if (received == SOCKET_ERROR) {
            if (WSAGetLastError() == WSAEMSGSIZE) {
                static std::atomic<int> s_oversizeLogs{0};
                if (s_oversizeLogs.fetch_add(1) < 20) {
                    NOBLE_WARN("NetworkTransport: dropped a datagram larger than the {}-byte receive buffer", sizeof(rawBuf));
                }
                continue;
            }
            break;  // WSAEWOULDBLOCK: nothing more waiting
        }
        if (received <= static_cast<int>(sizeof(uint64_t))) {
            continue;
        }

        uint64_t wireSender = 0;
        std::memcpy(&wireSender, rawBuf, sizeof(uint64_t));
        if (wireSender == 0 || wireSender == m_localMachineId) {
            continue;  // Ignore self-loopback packets before updating m_lastUdpRecvTick
        }

        const uint64_t now = GetTickCount64();
        m_lastUdpRecvTick.store(now);
        m_bytesRecv += received;
        char ipStr[INET_ADDRSTRLEN] = { 0 };
        inet_ntop(AF_INET, &senderAddr.sin_addr, ipStr, sizeof(ipStr));
        const std::string addrKey = std::string(ipStr) + ":" + std::to_string(ntohs(senderAddr.sin_port));

        // NAT hole-punch PING / PONG packet
        if (received == static_cast<int>(sizeof(uint64_t) + 4) &&
            rawBuf[8] == 'P' && rawBuf[9] == 'I' && rawBuf[10] == 'N' && rawBuf[11] == 'G') {
            if (wireSender != 0 && wireSender != m_localMachineId) {
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_peerTable[wireSender] = senderAddr;
                    m_addrToPeer[addrKey] = wireSender;
                    auto& route = m_peerRouteStates[wireSender];
                    route.addr = senderAddr;
                    route.lastUdpRecvTick = now;
                    route.directConfirmed = true;
                }

                // Flush any packets that were held waiting for this peer's address
                FlushHeldPackets(wireSender, senderAddr);

                if (m_isHost) {
                    // Echo PING back to Guest over direct UDP
                    uint8_t pong[sizeof(uint64_t) + 4] = { 0 };
                    std::memcpy(pong, &m_localMachineId, sizeof(uint64_t));
                    pong[sizeof(uint64_t)]     = 'P';
                    pong[sizeof(uint64_t) + 1] = 'I';
                    pong[sizeof(uint64_t) + 2] = 'N';
                    pong[sizeof(uint64_t) + 3] = 'G';
                    sendto(m_socket, reinterpret_cast<const char*>(pong), sizeof(pong), 0,
                           reinterpret_cast<const sockaddr*>(&senderAddr), sizeof(senderAddr));
                    NOBLE_INFO("NetworkTransport: [HOST] Registered peer 0x{:X} via NAT punch PING from {} (echoed PING)",
                               wireSender, addrKey);
                } else {
                    if (wireSender == m_hostMachineId) {
                        m_remoteHostAddr = senderAddr;
                    }
                    NOBLE_INFO("NetworkTransport: [GUEST] Received NAT punch PING echo from Host ({})", addrKey);
                }
            }
            continue;
        }

        if (!QueueWirePacket(rawBuf, static_cast<size_t>(received), false)) {
            continue;
        }

        if (!m_isHost && wireSender == m_hostMachineId) {
            m_remoteHostAddr = senderAddr;
        }

        // Host: remember where each guest's packets come from so replies go back to that address
        if (m_isHost) {
            bool needFlush = false;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                auto it = m_peerTable.find(wireSender);
                if (it == m_peerTable.end() ||
                    it->second.sin_addr.s_addr != senderAddr.sin_addr.s_addr ||
                    it->second.sin_port != senderAddr.sin_port) {
                    m_peerTable[wireSender] = senderAddr;
                    m_addrToPeer[addrKey] = wireSender;
                    needFlush = true;
                    NOBLE_INFO("NetworkTransport: Registered remote peer 0x{:X} at {}", wireSender, addrKey);
                }
                auto& route = m_peerRouteStates[wireSender];
                route.addr = senderAddr;
                route.lastUdpRecvTick = now;
                route.directConfirmed = true;
            }
            if (needFlush) {
                FlushHeldPackets(wireSender, senderAddr);
            }
        }
    }

    // 2. WebSocket relay packets
    if (m_relayActive.load()) {
        std::lock_guard<std::mutex> lock(m_relayMutex);
        while (!m_relayRecvQueue.empty()) {
            auto wire = std::move(m_relayRecvQueue.front());
            m_relayRecvQueue.pop_front();
            if (wire.size() == sizeof(uint64_t) + 4 &&
                wire[8] == 'P' && wire[9] == 'I' && wire[10] == 'N' && wire[11] == 'G') {
                continue;
            }
            if (QueueWirePacket(wire.data(), wire.size(), true)) {
                m_bytesRecv += wire.size();
            }
        }
    }

    // 3. Hand the game the oldest packet addressed to this endpoint's port
    auto qit = m_portQueues.find(port);
    if (qit == m_portQueues.end() || qit->second.empty()) {
        return 0;
    }
    QueuedPacket pkt = std::move(qit->second.front());
    qit->second.pop_front();

    size_t payloadLen = pkt.data.size();
    if (payloadLen > bufferCapacity) {
        static std::atomic<int> s_truncLogs{0};
        if (s_truncLogs.fetch_add(1) < 20) {
            NOBLE_WARN("NetworkTransport: packet of {} bytes truncated to the game's {}-byte buffer (port {})",
                       payloadLen, bufferCapacity, port);
        }
        payloadLen = bufferCapacity;
    }
    std::memcpy(outBuffer, pkt.data.data(), payloadLen);
    if (outSenderPeerId) {
        *outSenderPeerId = pkt.sender;
    }

    uint64_t recvNum = ++m_packetsRecv;
    if (recvNum <= kPacketDumpLimit) {
        NOBLE_INFO("NetworkTransport: [{}] RX #{} <- 0x{:X} via {} port {} len={} | {}", m_isHost ? "HOST" : "GUEST",
                   recvNum, pkt.sender, pkt.fromRelay ? "RELAY" : "UDP", port, payloadLen, HexDump(outBuffer, payloadLen));
    }
    if (recvNum <= 10 || (recvNum % 500) == 0) {
        NOBLE_INFO("NetworkTransport: [{}] Received packet #{} from 0x{:X} via {} port {} ({} payload bytes)",
                   m_isHost ? "HOST" : "GUEST", recvNum, pkt.sender, pkt.fromRelay ? "RELAY" : "UDP", port, payloadLen);
    }
    return static_cast<uint64_t>(payloadLen);
}

} // namespace noble
