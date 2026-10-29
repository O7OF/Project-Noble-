#include "NetworkTransport.hpp"
#include "common/Logger.hpp"

#include <random>
#include <cstring>

namespace noble {

namespace {

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

} // namespace

NetworkTransport::NetworkTransport() = default;

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

    // Allow reuse if port was recently closed
    BOOL reuse = TRUE;
    setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&reuse), sizeof(reuse));

    // Bind local port
    sockaddr_in bindAddr = {};
    bindAddr.sin_family = AF_INET;
    bindAddr.sin_addr.s_addr = INADDR_ANY;
    bindAddr.sin_port = htons(isHost ? localPort : 0); // If host, bind 11774. If guest, ephemeral port (or 11774)

    if (bind(m_socket, reinterpret_cast<sockaddr*>(&bindAddr), sizeof(bindAddr)) != 0) {
        // If preferred port is busy (e.g. guest on same machine during local playtest), bind ephemeral
        bindAddr.sin_port = 0;
        if (bind(m_socket, reinterpret_cast<sockaddr*>(&bindAddr), sizeof(bindAddr)) != 0) {
            NOBLE_ERROR("NetworkTransport: Failed to bind UDP socket (WSAError: {})", WSAGetLastError());
            closesocket(m_socket);
            m_socket = INVALID_SOCKET;
            return false;
        }
    }

    // Retrieve actual bound port
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

    m_active.store(true);
    NOBLE_INFO("NetworkTransport: Initialized {} on UDP port {} (Local Machine ID: 0x{:X}, Host ID: 0x{:X}, Session ID: 0x{:X})",
        m_isHost ? "HOST" : "GUEST", m_localPort, m_localMachineId, m_hostMachineId, m_sessionId);

    return true;
}

void NetworkTransport::Shutdown() {
    if (!m_active.exchange(false)) return;

    // Grace period for any in-flight send/recv calls to complete before closing socket
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
    uint64_t secondary = m_sessionId ^ 0xA5A5A5A5A5A5A5A5ULL;
    std::memcpy(&outGuid16[8], &secondary, sizeof(secondary));
}

uint64_t NetworkTransport::OnNetSend(uint64_t targetPeerId, const uint8_t* data, size_t size) {
    if (!m_active.load() || m_socket == INVALID_SOCKET || !data || size == 0) {
        return 0;
    }

    // Reach network packet framing (reversed from host-probe::net::handle / send_route_kind):
    // Wire packet: [uint64_t senderMachineId] [raw Blam! engine payload]
    const size_t totalBytes = sizeof(uint64_t) + size;
    uint8_t stackBuf[2048];
    std::vector<uint8_t> heapBuf;
    uint8_t* wireData = stackBuf;

    if (totalBytes > sizeof(stackBuf)) {
        heapBuf.resize(totalBytes);
        wireData = heapBuf.data();
    }

    // Prepend sender's machine ID
    std::memcpy(wireData, &m_localMachineId, sizeof(uint64_t));
    std::memcpy(wireData + sizeof(uint64_t), data, size);

    uint64_t packetNum = ++m_packetsSent;

    if (!m_isHost) {
        // Guest sends all traffic to the remote host endpoint
        int sent = sendto(m_socket, reinterpret_cast<const char*>(wireData), static_cast<int>(totalBytes), 0,
                          reinterpret_cast<const sockaddr*>(&m_remoteHostAddr), sizeof(m_remoteHostAddr));
        if (packetNum <= 10 || (packetNum % 500) == 0) {
            NOBLE_INFO("NetworkTransport: [GUEST] Sent packet #{} to Host 0x{:X} ({} payload bytes, {} wire bytes, result: {})",
                       packetNum, targetPeerId, size, totalBytes, sent);
        }
        return (sent > 0) ? static_cast<uint64_t>(size) : 0;
    }

    // Host sends traffic to specific peer or active peers in peer table
    sockaddr_in destAddr = {};
    bool found = false;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_peerTable.find(targetPeerId);
        if (it != m_peerTable.end()) {
            destAddr = it->second;
            found = true;
        } else if (!m_peerTable.empty()) {
            // Initial join handshake reply: route to the most recent peer
            destAddr = m_peerTable.begin()->second;
            found = true;
        }
    }

    if (found) {
        int sent = sendto(m_socket, reinterpret_cast<const char*>(wireData), static_cast<int>(totalBytes), 0,
                          reinterpret_cast<const sockaddr*>(&destAddr), sizeof(destAddr));
        if (packetNum <= 10 || (packetNum % 500) == 0) {
            char ipStr[INET_ADDRSTRLEN] = { 0 };
            inet_ntop(AF_INET, &destAddr.sin_addr, ipStr, sizeof(ipStr));
            NOBLE_INFO("NetworkTransport: [HOST] Sent packet #{} to Peer 0x{:X} ({}:{}) ({} payload bytes, result: {})",
                       packetNum, targetPeerId, ipStr, ntohs(destAddr.sin_port), size, sent);
        }
        return (sent > 0) ? static_cast<uint64_t>(size) : 0;
    }

    // Packet accepted into pipeline even if peer endpoint is not registered yet
    return static_cast<uint64_t>(size);
}

uint64_t NetworkTransport::OnNetRecv(uint8_t* outBuffer, size_t bufferCapacity, uint64_t* outSenderPeerId) {
    if (!m_active.load() || m_socket == INVALID_SOCKET || !outBuffer || bufferCapacity == 0) {
        return 0;
    }

    sockaddr_in senderAddr = {};
    int senderLen = sizeof(senderAddr);
    uint8_t rawBuf[4096];

    int received = recvfrom(m_socket, reinterpret_cast<char*>(rawBuf), sizeof(rawBuf), 0,
                            reinterpret_cast<sockaddr*>(&senderAddr), &senderLen);

    if (received <= static_cast<int>(sizeof(uint64_t))) {
        return 0; // No packet available or below minimum wire framing size
    }

    // Extract sender machine ID from wire header
    uint64_t senderMachineId = *reinterpret_cast<const uint64_t*>(rawBuf);
    size_t payloadLen = static_cast<size_t>(received) - sizeof(uint64_t);

    if (payloadLen > bufferCapacity) {
        payloadLen = bufferCapacity;
    }

    std::memcpy(outBuffer, rawBuf + sizeof(uint64_t), payloadLen);

    if (outSenderPeerId) {
        *outSenderPeerId = senderMachineId;
    }

    uint64_t recvNum = ++m_packetsRecv;

    if (!m_isHost) {
        if (recvNum <= 10 || (recvNum % 500) == 0) {
            NOBLE_INFO("NetworkTransport: [GUEST] Received packet #{} from Host 0x{:X} ({} payload bytes)",
                       recvNum, senderMachineId, payloadLen);
        }
        return static_cast<uint64_t>(payloadLen);
    }

    // Host received packet from remote guest: map senderMachineId to senderAddr
    char ipStr[INET_ADDRSTRLEN] = { 0 };
    inet_ntop(AF_INET, &senderAddr.sin_addr, ipStr, sizeof(ipStr));
    std::string addrKey = std::string(ipStr) + ":" + std::to_string(ntohs(senderAddr.sin_port));

    {
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
        NOBLE_INFO("NetworkTransport: [HOST] Received packet #{} from Peer 0x{:X} ({}) ({} payload bytes)",
                   recvNum, senderMachineId, addrKey, payloadLen);
    }

    return static_cast<uint64_t>(payloadLen);
}

} // namespace noble

