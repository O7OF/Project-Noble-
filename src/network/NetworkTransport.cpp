#include "NetworkTransport.hpp"
#include <nlohmann/json.hpp>
#include "common/Logger.hpp"
#include "engine/SharedTagCache.hpp"
#include "engine/DevCheats.hpp"
#include "career/Career.hpp"
#include "ui/PauseMenu.hpp"

#include <windows.h>
#include <winhttp.h>
#include <random>
#include <cstring>
#include <chrono>
#include <atomic>
#include <filesystem>
#include <fstream>

namespace noble {

bool NetworkTransport::MaterializeHostVariant(const std::string& name, const std::string& body, std::string& outPath) {
    if (name.empty() || body.empty()) return false;
    std::error_code ec;
    const auto dir = std::filesystem::current_path(ec) / "data" / "host_variants";
    std::filesystem::create_directories(dir, ec);
    const auto path = dir / std::filesystem::path(name).filename();
    std::ofstream out(path, std::ios::binary);
    if (!out) return false;
    out.write(body.data(), static_cast<std::streamsize>(body.size()));
    out.close();
    if (!out) return false;
    outPath = path.string();
    return true;
}

bool NetworkTransport::SelfTest() {
    const std::string body = "{\"name\":\"SELFTEST\",\"base\":\"slayer_054.bin\"}";
    std::string path;
    if (!MaterializeHostVariant("_selftest.json", body, path)) return false;
    std::ifstream in(path, std::ios::binary);
    const std::string roundtrip((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();
    std::error_code ec;
    std::filesystem::remove(path, ec);
    return roundtrip == body;
}

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
    std::memset(&m_remoteHostLanAddr, 0, sizeof(m_remoteHostLanAddr));
    m_lastPunchTick = 0;
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

    // Disable Windows ICMP Port Unreachable (WSAECONNRESET / 10054) on UDP sockets so that probing
    // 127.0.0.1 or LAN/WAN NAT hole-punch ports before the remote peer binds never injects error frames
    // into recvfrom().
#ifndef SIO_UDP_CONNRESET
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)
#endif
    BOOL newBehavior = FALSE;
    DWORD bytesReturned = 0;
    WSAIoctl(m_socket, SIO_UDP_CONNRESET, &newBehavior, sizeof(newBehavior), nullptr, 0, &bytesReturned, nullptr, nullptr);

    // Set non-blocking mode
    u_long nonBlocking = 1;
    if (ioctlsocket(m_socket, FIONBIO, &nonBlocking) != 0) {
        NOBLE_WARN("NetworkTransport: Failed to set non-blocking socket mode");
    }

    // Set socket buffer sizes to 1 MB for zero-drop burst absorption
    int bufSize = 1024 * 1024;
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
        m_gamePeers.clear();
    }

    m_packetsSent.store(0);
    m_packetsRecv.store(0);
    m_lastUdpRecvTick.store(0);

    m_active.store(true);
    NOBLE_INFO("NetworkTransport: Initialized {} on UDP port {} (Local Machine ID: 0x{:X}, Host ID: 0x{:X}, Session ID: 0x{:X})",
        m_isHost ? "HOST" : "GUEST", m_localPort, m_localMachineId, m_hostMachineId, m_sessionId);

    // If Guest, fire an immediate NAT hole-punch PING packet burst to the Host so the local NAT mapping opens
    // and the Host learns our external IP:port before the first Blam! handshake packet.
    if (!m_isHost && m_remoteHostAddr.sin_addr.s_addr != 0) {
        uint8_t punch[sizeof(uint64_t) + 4] = { 0 };
        std::memcpy(punch, &m_localMachineId, sizeof(uint64_t));
        punch[sizeof(uint64_t)]     = 'P';
        punch[sizeof(uint64_t) + 1] = 'I';
        punch[sizeof(uint64_t) + 2] = 'N';
        punch[sizeof(uint64_t) + 3] = 'G';
        for (int i = 0; i < 5; ++i) {
            sendto(m_socket, reinterpret_cast<const char*>(punch), sizeof(punch), 0,
                   reinterpret_cast<const sockaddr*>(&m_remoteHostAddr), sizeof(m_remoteHostAddr));
        }
        // Also send a localhost punch to 127.0.0.1:remotePort so same-PC Online P2P tests connect directly
        // without relying on router Hairpin NAT (harmless if the Host is on a remote PC since Guest is on an ephemeral port).
        if (m_remoteHostAddr.sin_addr.s_addr != htonl(INADDR_LOOPBACK)) {
            sockaddr_in localHostAddr = {};
            localHostAddr.sin_family = AF_INET;
            localHostAddr.sin_port = m_remoteHostAddr.sin_port;
            localHostAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            for (int i = 0; i < 3; ++i) {
                sendto(m_socket, reinterpret_cast<const char*>(punch), sizeof(punch), 0,
                       reinterpret_cast<const sockaddr*>(&localHostAddr), sizeof(localHostAddr));
            }
        }
        NOBLE_INFO("NetworkTransport: [GUEST] Sent initial NAT punch PING burst to {}:{}", remoteIp, remotePort);
    }

    return true;
}

double NetworkTransport::SecondsSinceRemotePacket() const {
    const uint64_t last = m_lastRemotePacketMs.load();
    if (last == 0) return 0.0;
    return static_cast<double>(GetTickCount64() - last) / 1000.0;
}

size_t NetworkTransport::GetGamePeerCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_gamePeers.size();
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
    m_gamePeers.clear();

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
            auto& route = m_peerRouteStates[machineId];
            route.addr = addr;
            route.directConfirmed = true;
            route.lastUdpRecvTick = GetTickCount64();
        }
    }

    // Fire UDP punch packet burst immediately to punch open NAT router firewall
    if (m_socket != INVALID_SOCKET) {
        if (machineId != 0) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_peerRouteStates[machineId].lastPingSentTick = GetTickCount64();
        }
        uint8_t punch[sizeof(uint64_t) + 4] = { 0 };
        std::memcpy(punch, &m_localMachineId, sizeof(uint64_t));
        punch[sizeof(uint64_t)] = 'P';
        punch[sizeof(uint64_t) + 1] = 'I';
        punch[sizeof(uint64_t) + 2] = 'N';
        punch[sizeof(uint64_t) + 3] = 'G';
        for (int i = 0; i < 3; ++i) {
            sendto(m_socket, reinterpret_cast<const char*>(punch), sizeof(punch), 0,
                   reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));
        }
        NOBLE_INFO("NetworkTransport: Dispatched NAT punch datagrams to {} (Peer 0x{:X})", key, machineId);
    }
}

void NetworkTransport::SetRemoteHostLanAddress(const std::string& lanIp, uint16_t port) {
    if (lanIp.empty()) return;
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = port != 0 ? htons(port) : m_remoteHostAddr.sin_port;
    if (inet_pton(AF_INET, lanIp.c_str(), &addr.sin_addr) <= 0) return;
    if (addr.sin_addr.s_addr == m_remoteHostAddr.sin_addr.s_addr) return;

    m_remoteHostLanAddr = addr;
    if (!m_isHost && m_socket != INVALID_SOCKET && m_remoteHostLanAddr.sin_addr.s_addr != 0) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_peerRouteStates[m_hostMachineId].lastPingSentTick = GetTickCount64();
        }
        uint8_t punch[sizeof(uint64_t) + 4] = { 0 };
        std::memcpy(punch, &m_localMachineId, sizeof(uint64_t));
        punch[sizeof(uint64_t)]     = 'P';
        punch[sizeof(uint64_t) + 1] = 'I';
        punch[sizeof(uint64_t) + 2] = 'N';
        punch[sizeof(uint64_t) + 3] = 'G';
        for (int i = 0; i < 3; ++i) {
            sendto(m_socket, reinterpret_cast<const char*>(punch), sizeof(punch), 0,
                   reinterpret_cast<const sockaddr*>(&m_remoteHostLanAddr), sizeof(m_remoteHostLanAddr));
        }
        NOBLE_INFO("NetworkTransport: [GUEST] Configured & punched Host LAN address {}:{}",
                   lanIp, ntohs(m_remoteHostLanAddr.sin_port));
    }
}

void NetworkTransport::SendDisconnectPacket() {
    if (m_socket == INVALID_SOCKET) return;

    uint8_t disc[sizeof(uint64_t) + 4] = { 0 };
    std::memcpy(disc, &m_localMachineId, sizeof(uint64_t));
    disc[sizeof(uint64_t)]     = 'D';
    disc[sizeof(uint64_t) + 1] = 'I';
    disc[sizeof(uint64_t) + 2] = 'S';
    disc[sizeof(uint64_t) + 3] = 'C';

    if (!m_isHost) {
        if (m_remoteHostAddr.sin_addr.s_addr != 0) {
            for (int i = 0; i < 3; ++i) {
                sendto(m_socket, reinterpret_cast<const char*>(disc), sizeof(disc), 0,
                       reinterpret_cast<const sockaddr*>(&m_remoteHostAddr), sizeof(m_remoteHostAddr));
            }
        }
        if (m_remoteHostLanAddr.sin_addr.s_addr != 0) {
            for (int i = 0; i < 3; ++i) {
                sendto(m_socket, reinterpret_cast<const char*>(disc), sizeof(disc), 0,
                       reinterpret_cast<const sockaddr*>(&m_remoteHostLanAddr), sizeof(m_remoteHostLanAddr));
            }
        }
    } else {
        std::vector<sockaddr_in> targets;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            targets.reserve(m_peerTable.size());
            for (const auto& kv : m_peerTable) {
                targets.push_back(kv.second);
            }
        }
        for (const auto& dest : targets) {
            for (int i = 0; i < 3; ++i) {
                sendto(m_socket, reinterpret_cast<const char*>(disc), sizeof(disc), 0,
                       reinterpret_cast<const sockaddr*>(&dest), sizeof(dest));
            }
        }
    }

    if (m_relayConnected.load()) {
        nlohmann::json j = {{"type", "lobby_leave"}};
        SendLobbyControlJson(j.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace));
    }
    NOBLE_INFO("NetworkTransport: Dispatched disconnect datagrams (DISC).");
}

int NetworkTransport::GetPeerPingMs(uint64_t machineId) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (machineId == 0) {
        if (!m_isHost) machineId = m_hostMachineId;
        else if (!m_peerRouteStates.empty()) return m_peerRouteStates.begin()->second.rttMs;
    }
    auto it = m_peerRouteStates.find(machineId);
    if (it != m_peerRouteStates.end() && it->second.rttMs >= 0) {
        return it->second.rttMs;
    }
    return -1;
}

std::unordered_map<uint64_t, int> NetworkTransport::GetAllPeerPingsMs() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::unordered_map<uint64_t, int> out;
    for (const auto& kv : m_peerRouteStates) {
        if (kv.second.rttMs >= 0) {
            out[kv.first] = kv.second.rttMs;
        }
    }
    return out;
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

    m_relayMachineId.store(machineId != 0 ? machineId : (isHost ? kDefaultHostMachineId : 0));
    m_relayStopRequested.store(false);
    {
        std::lock_guard<std::mutex> lock(m_relayMutex);
        m_relayUrl = relayUrl;
    }
    m_relayActive.store(true);
    m_relayThread = std::thread(&NetworkTransport::RelayWorkerThread, this, relayUrl, sessionId, isHost, machineId);
    NOBLE_INFO("NetworkTransport: Starting WebSocket packet relay thread ({})", relayUrl);
}

void NetworkTransport::SetVoiceFrameCallback(VoiceFrameCallback cb) {
    m_voiceFrameCb.store(cb);
}

void NetworkTransport::SendVoiceFrame(const uint8_t* data, size_t size) {
    if (!m_relayConnected.load()) return;
    {
        std::lock_guard<std::mutex> lock(m_relayMutex);
        while (m_relayVoiceSendQueue.size() >= 10) m_relayVoiceSendQueue.pop_front();   // old speech is worthless
        m_relayVoiceSendQueue.emplace_back(data, data + size);
    }
    m_relaySendCv.notify_one();
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

void NetworkTransport::SetLobbyReturnCallback(LobbyReturnCallback cb) {
    std::lock_guard<std::mutex> lock(m_relayMutex);
    m_lobbyReturnCb = std::move(cb);
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

bool NetworkTransport::SendSessionSync(uint64_t frame, bool running) {
    if (!m_relayConnected.load()) return false;
    nlohmann::json j = {{"type", "session_sync"}, {"frame", frame}, {"running", running}};
    SendLobbyControlJson(j.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace));
    return true;
}

void NetworkTransport::SendLobbyControlJson(const std::string& jsonString) {
    {
        std::lock_guard<std::mutex> lock(m_relayMutex);
        if (m_relayActive.load()) {
            m_relayTextSendQueue.push_back(jsonString);
        }
    }
    m_relaySendCv.notify_one();
}

void NetworkTransport::StopRelay() {
    m_relayStopRequested.store(true);
    const bool wasActive = m_relayActive.exchange(false);
    m_relaySendCv.notify_all();
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
    m_relayVoiceSendQueue.clear();
    NOBLE_INFO("NetworkTransport: Stopped WebSocket packet relay.");
}

void NetworkTransport::SetChatCallback(ChatCallback cb) {
    std::lock_guard<std::mutex> lock(m_chatCbMutex);
    m_chatCb = std::move(cb);
}

// Through the sender thread's queue: WinHTTP allows one send at a time. No machine id here: the server knows this connection's
// id from the relay URL, and that's what its roster holds (GetLocalMachineId() is 0 in a menu lobby).
bool NetworkTransport::SendChat(const std::string& channel, const std::string& text, const std::string& from, const std::string& target) {
    if (!m_relayConnected.load() || text.empty()) return false;
    nlohmann::json j = {{"type", "chat"}, {"channel", channel}, {"text", text}, {"from", from}};
    if (!target.empty()) j["target"] = target;
    {
        std::lock_guard<std::mutex> lock(m_relayMutex);
        m_relayTextSendQueue.push_back(j.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace));
    }
    m_relaySendCv.notify_one();
    return true;
}

void NetworkTransport::RelayWorkerThread(std::string relayUrl, std::string sessionId, bool isHost, uint64_t machineId) {
    (void)sessionId;
    (void)isHost;
    (void)machineId;

    // The relay stamps voice frames with this id, so the host (whose URL has none) names itself too.
    if (relayUrl.find("peerId=") == std::string::npos && m_relayMachineId.load() != 0) {
        char idText[32];
        std::snprintf(idText, sizeof(idText), "0x%llX", static_cast<unsigned long long>(m_relayMachineId.load()));
        relayUrl += (relayUrl.find('?') == std::string::npos ? "?peerId=" : "&peerId=") + std::string(idText);
    }

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
        m_relayWs.store(hWebSocket);
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

        WinHttpSetTimeouts(hWebSocket, 5000, 5000, 1000, 50);
        {
            std::lock_guard<std::mutex> lock(m_relayMutex);
            m_relaySessionHandle = hSession;
            m_relayWsHandle = hWebSocket;
        }
        m_relayActive.store(true);
        m_relayConnected.store(true);
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
        NOBLE_INFO("NetworkTransport: WebSocket relay connected successfully!");

        std::vector<uint8_t> recvBuffer(65536);
        std::vector<uint8_t> fragmentBuffer;
        fragmentBuffer.reserve(16384);

        // WinHttpWebSocketReceive blocks until a frame arrives: the socket's receive timeout does not apply to it. Sending
        // has its own thread so that nothing queued waits for the other side to speak first (found by voice, 2026-10-31).
        std::atomic<bool> senderStop{ false };
        std::thread sender([this, hWebSocket, &senderStop] {
            SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
            bool warned = false;
            const auto sendOne = [&](WINHTTP_WEB_SOCKET_BUFFER_TYPE type, const void* data, size_t size) {
                const DWORD err = WinHttpWebSocketSend(hWebSocket, type, const_cast<void*>(data), static_cast<DWORD>(size));
                if (err != NO_ERROR && !warned && !senderStop.load() && !m_relayStopRequested.load()) {
                    warned = true;
                    NOBLE_WARN("NetworkTransport: relay send failed ({})", err);
                }
            };
            while (!senderStop.load() && !m_relayStopRequested.load() && m_relayActive.load()) {
                std::deque<std::string> text;
                std::deque<std::vector<uint8_t>> packets, voice;
                {
                    std::unique_lock<std::mutex> lock(m_relayMutex);
                    if (m_relayTextSendQueue.empty() && m_relaySendQueue.empty() && m_relayVoiceSendQueue.empty()) {
                        m_relaySendCv.wait_for(lock, std::chrono::milliseconds(50), [&] {
                            return senderStop.load() || m_relayStopRequested.load() || !m_relayActive.load() ||
                                   !m_relayTextSendQueue.empty() || !m_relaySendQueue.empty() || !m_relayVoiceSendQueue.empty();
                        });
                    }
                    text.swap(m_relayTextSendQueue);
                    packets.swap(m_relaySendQueue);
                    voice.swap(m_relayVoiceSendQueue);
                }
                for (const auto& message : text) {
                    if (!message.empty()) sendOne(WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE, message.data(), message.size());
                }
                for (const auto& packet : packets) {
                    if (!packet.empty()) sendOne(WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE, packet.data(), packet.size());
                }
                for (const auto& frame : voice) {
                    sendOne(WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE, frame.data(), frame.size());
                }
            }
        });

        while (!m_relayStopRequested.load() && m_relayActive.load()) {
            // Drain incoming WebSocket frames with zero sleep delay (WinHttpWebSocketReceive blocks in kernel until data arrives).
            for (int pollIter = 0; pollIter < 32 && !m_relayStopRequested.load() && m_relayActive.load(); ++pollIter) {
                DWORD bytesRead = 0;
                WINHTTP_WEB_SOCKET_BUFFER_TYPE bufferType;
                DWORD err = WinHttpWebSocketReceive(hWebSocket, recvBuffer.data(), static_cast<DWORD>(recvBuffer.size()), &bytesRead, &bufferType);

                if (err == NO_ERROR && bytesRead > 0) {
                    if (bufferType == WINHTTP_WEB_SOCKET_BINARY_FRAGMENT_BUFFER_TYPE ||
                        bufferType == WINHTTP_WEB_SOCKET_UTF8_FRAGMENT_BUFFER_TYPE) {
                        if (fragmentBuffer.size() + bytesRead <= 131072) {
                            fragmentBuffer.insert(fragmentBuffer.end(), recvBuffer.data(), recvBuffer.data() + bytesRead);
                        } else {
                            fragmentBuffer.clear();
                        }
                        continue;
                    }

                    const uint8_t* msgData = recvBuffer.data();
                    size_t msgLen = bytesRead;
                    if (!fragmentBuffer.empty()) {
                        if (fragmentBuffer.size() + bytesRead <= 131072) {
                            fragmentBuffer.insert(fragmentBuffer.end(), recvBuffer.data(), recvBuffer.data() + bytesRead);
                            msgData = fragmentBuffer.data();
                            msgLen = fragmentBuffer.size();
                        } else {
                            fragmentBuffer.clear();
                            continue;
                        }
                    }

                    if (bufferType == WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE ||
                        (msgLen >= 2 && msgData[0] == '{' && msgData[msgLen - 1] == '}')) {
                        std::string msgStr(reinterpret_cast<const char*>(msgData), msgLen);
                        fragmentBuffer.clear();
                        try {
                            auto j = nlohmann::json::parse(msgStr);
                            std::string type = j.value("type", "");
                            if (type == "lobby_launch") {
                                auto ld = j.value("launch", nlohmann::json::object());
                                std::string lMap = ld.value("map", "20_sword_slayer");
                                std::string lMode = ld.value("mode", "multiplayer");
                                std::string lVar = ld.value("variant", "");
                                std::string lVarBody = ld.value("variant_body", "");
                                std::string lVarPath = lVar;
                                if (!lVarBody.empty() && !lVar.empty() &&
                                    MaterializeHostVariant(lVar, lVarBody, lVarPath)) {
                                    NOBLE_INFO("NetworkTransport: host variant body '{}' saved to {}", lVar, lVarPath);
                                }
                                int lDiff = ld.value("difficulty", 1);
                                uint64_t lSkulls = 0;
                                try { lSkulls = std::stoull(ld.value("skulls", "0")); } catch (...) {}
                                std::string hIp = ld.value("host_ip", "");
                                if (ld.value("is_lan", false) && ld.contains("host_lan_ip")) {
                                    std::string lanIp = ld.value("host_lan_ip", "");
                                    if (!lanIp.empty()) hIp = lanIp;
                                }
                                uint16_t hPort = static_cast<uint16_t>(ld.value("host_port", 11774));
                                std::string sId = ld.value("session_id", sessionId);
                                std::string lMapOverride = ld.value("map_override", "");
                                std::string lModFolder = ld.value("mod_folder", "");
                                std::string lModId = ld.value("mod_id", "");

                                LobbyLaunchCallback cb;
                                {
                                    std::lock_guard<std::mutex> lock(m_relayMutex);
                                    cb = m_lobbyLaunchCb;
                                }
                                if (cb) {
                                    cb(lMap, lMode, lVarPath, lDiff, lSkulls, hIp, hPort, sId, lMapOverride, lModFolder, lModId);
                                }
                            } else if (type == "chat") {
                                ChatMessage m;
                                m.channel = j.value("channel", "all");
                                m.from = j.value("from", "player");
                                m.text = j.value("text", "");
                                m.target = j.value("target", "");
                                ChatCallback cb;
                                { std::lock_guard<std::mutex> lock(m_chatCbMutex); cb = m_chatCb; }
                                if (cb && !m.text.empty()) cb(m);
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
                            } else if (type == "match_over") {
                                NOBLE_INFO("NetworkTransport: received match_over packet from host");
                                Career::OnMatchOverJson(msgStr);
                                PauseMenu::ShowGameOver();
                            } else if (type == "host_return_to_lobby") {
                                LobbyReturnCallback cb;
                                {
                                    std::lock_guard<std::mutex> lock(m_relayMutex);
                                    cb = m_lobbyReturnCb;
                                }
                                if (cb) {
                                    cb();
                                }
                            } else if (type == "session_sync") {
                                const bool running = j.value("running", false);
                                const uint64_t frame = static_cast<uint64_t>(j.value("frame", 0ULL));
                                if (running && !m_hostSimRunning.exchange(true)) {
                                    NOBLE_INFO("NetworkTransport: host reports its simulation running at frame {}", frame);
                                } else if (!running) {
                                    m_hostSimRunning.store(false);
                                }
                                m_hostSimFrame.store(frame);
                            } else if (type == "spawn_request") {
                                if (m_isHost) {
                                    uint32_t datum = j.value("datum", 0xFFFFFFFF);
                                    std::string tagPath = j.value("tagPath", "");
                                    NOBLE_INFO("NetworkTransport: [HOST] Received guest spawn_request for datum 0x{:X} '{}'", datum, tagPath);
                                    SharedTagCache::QueueSpawn(datum, tagPath);
                                }
                            } else if (type == "dev_spawn") {
                                if (m_isHost) {
#if defined(NOBLE_DEV_BUILD)
                                    std::string name = j.value("name", "");
                                    std::string cat = j.value("category", "");
                                    std::string tagPath = j.value("tagPath", "");
                                    uint32_t tagIndex = j.value("tagIndex", 0xFFFFFFFF);
                                    bool give = j.value("give", false);
                                    NOBLE_INFO("NetworkTransport: [HOST] Received guest dev_spawn for '{}' ({})", name, cat);
                                    DevCheats::QueueItemAction(name, cat, tagPath, tagIndex, give);
#endif
                                }
                            }
                        } catch (...) {}
                    } else if (msgLen >= 4 && std::memcmp(msgData, "NVOX", 4) == 0) {
                        if (const VoiceFrameCallback cb = m_voiceFrameCb.load()) cb(msgData, msgLen);
                        fragmentBuffer.clear();
                    } else if (msgLen >= sizeof(uint64_t)) {
                        std::vector<uint8_t> pkt(msgData, msgData + msgLen);
                        fragmentBuffer.clear();
                        std::lock_guard<std::mutex> lock(m_relayMutex);
                        if (m_relayRecvQueue.size() < 2048) {
                            m_relayRecvQueue.push_back(std::move(pkt));
                        }
                    } else {
                        fragmentBuffer.clear();
                    }
                } else if (err == ERROR_WINHTTP_TIMEOUT) {
                    break; // normal idle; loop back into WinHttpWebSocketReceive without Sleep(1) stall
                } else if (err != NO_ERROR) {
                    m_relayActive.store(false);
                    break;
                }
            }

            if (!m_relayActive.load()) break;
        }

        m_relayConnected.store(false);
        senderStop.store(true);
        m_relaySendCv.notify_all();
        sender.join();
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
            m_relayWs.store(nullptr);
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
        bool directUdpAlive = false;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto& route = m_peerRouteStates[m_hostMachineId];
            route.lastUdpSendTick = nowTick;
            directUdpAlive = route.directConfirmed || (route.lastUdpRecvTick != 0 && (nowTick - route.lastUdpRecvTick) < 3000);
        }
        // Guest sends directly to host
        int sent = sendto(m_socket, reinterpret_cast<const char*>(wireData), static_cast<int>(totalBytes), 0,
                          reinterpret_cast<const sockaddr*>(&m_remoteHostAddr), sizeof(m_remoteHostAddr));

        // While direct UDP is not yet confirmed, also probe Host's LAN IP (if known) and 127.0.0.1:<hostPort>.
        if (!directUdpAlive && m_remoteHostAddr.sin_addr.s_addr != 0) {
            if (m_remoteHostLanAddr.sin_addr.s_addr != 0 &&
                m_remoteHostLanAddr.sin_addr.s_addr != m_remoteHostAddr.sin_addr.s_addr) {
                sendto(m_socket, reinterpret_cast<const char*>(wireData), static_cast<int>(totalBytes), 0,
                       reinterpret_cast<const sockaddr*>(&m_remoteHostLanAddr), sizeof(m_remoteHostLanAddr));
            }
            if (m_remoteHostAddr.sin_addr.s_addr != htonl(INADDR_LOOPBACK)) {
                sockaddr_in localHostAddr = {};
                localHostAddr.sin_family = AF_INET;
                localHostAddr.sin_port = m_remoteHostAddr.sin_port;
                localHostAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
                sendto(m_socket, reinterpret_cast<const char*>(wireData), static_cast<int>(totalBytes), 0,
                       reinterpret_cast<const sockaddr*>(&localHostAddr), sizeof(localHostAddr));
            }
        }

        // Periodically send a 12-byte PING every 500ms while !directUdpAlive (NAT hole punch) or every 1000ms
        // while active so the Host echoes PONG and both sides measure live RTT ping and confirm bidirectional UDP.
        const uint64_t pingInterval = directUdpAlive ? 1000ULL : 500ULL;
        if (m_remoteHostAddr.sin_addr.s_addr != 0 && (nowTick - m_lastPunchTick >= pingInterval)) {
            m_lastPunchTick = nowTick;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_peerRouteStates[m_hostMachineId].lastPingSentTick = nowTick;
            }
            uint8_t punch[sizeof(uint64_t) + 4] = { 0 };
            std::memcpy(punch, &m_localMachineId, sizeof(uint64_t));
            punch[sizeof(uint64_t)]     = 'P';
            punch[sizeof(uint64_t) + 1] = 'I';
            punch[sizeof(uint64_t) + 2] = 'N';
            punch[sizeof(uint64_t) + 3] = 'G';
            sendto(m_socket, reinterpret_cast<const char*>(punch), sizeof(punch), 0,
                   reinterpret_cast<const sockaddr*>(&m_remoteHostAddr), sizeof(m_remoteHostAddr));
            if (!directUdpAlive && m_remoteHostLanAddr.sin_addr.s_addr != 0 &&
                m_remoteHostLanAddr.sin_addr.s_addr != m_remoteHostAddr.sin_addr.s_addr) {
                sendto(m_socket, reinterpret_cast<const char*>(punch), sizeof(punch), 0,
                       reinterpret_cast<const sockaddr*>(&m_remoteHostLanAddr), sizeof(m_remoteHostLanAddr));
            }
        }

        // Fallback to relay ONLY if direct UDP is not confirmed active, or for out-of-band session control (port 1001).
        // Never send simulation channel traffic (port 1000) over relay when direct UDP is active!
        if (m_relayActive.load() && (!directUdpAlive || port == 1001)) {
            {
                std::lock_guard<std::mutex> lock(m_relayMutex);
                if (m_relaySendQueue.size() < 2048) {
                    m_relaySendQueue.emplace_back(wireData, wireData + totalBytes);
                }
            }
            m_relaySendCv.notify_one();
        }

        if (packetNum <= 15 || (packetNum % 500) == 0) {
            NOBLE_INFO("NetworkTransport: [GUEST] Sent packet #{} to Host 0x{:X} ({} payload bytes, result: {}, direct: {}, relayFallback: {})",
                       packetNum, targetPeerId, size, sent, directUdpAlive, (!directUdpAlive || port == 1001) && m_relayActive.load());
        }
        return static_cast<uint64_t>(size);
    }

    // Host sends traffic:
    // Periodically send a 12-byte PING every 1000ms to all known peers so Guests echo PONG
    // and the Host continuously measures accurate 1-RTT live ping and confirms bidirectional UDP for every joined client.
    if (nowTick - m_lastPunchTick >= 1000ULL) {
        m_lastPunchTick = nowTick;
        std::vector<sockaddr_in> pingTargets;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            pingTargets.reserve(m_peerTable.size());
            for (const auto& kv : m_peerTable) {
                pingTargets.push_back(kv.second);
                m_peerRouteStates[kv.first].lastPingSentTick = nowTick;
            }
        }
        if (!pingTargets.empty()) {
            uint8_t pingPkt[sizeof(uint64_t) + 4] = { 0 };
            std::memcpy(pingPkt, &m_localMachineId, sizeof(uint64_t));
            pingPkt[sizeof(uint64_t)]     = 'P';
            pingPkt[sizeof(uint64_t) + 1] = 'I';
            pingPkt[sizeof(uint64_t) + 2] = 'N';
            pingPkt[sizeof(uint64_t) + 3] = 'G';
            for (const auto& destAddr : pingTargets) {
                sendto(m_socket, reinterpret_cast<const char*>(pingPkt), sizeof(pingPkt), 0,
                       reinterpret_cast<const sockaddr*>(&destAddr), sizeof(destAddr));
            }
        }
    }

    // If target is broadcast (0 or ~0ULL), broadcast to all known peers
    if (targetPeerId == 0 || targetPeerId == 0xFFFFFFFFFFFFFFFFULL) {
        std::vector<sockaddr_in> targets;
        bool allDirectConfirmed = false;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            targets.reserve(m_peerTable.size());
            allDirectConfirmed = !m_peerTable.empty();
            for (const auto& kv : m_peerTable) {
                targets.push_back(kv.second);
                auto& route = m_peerRouteStates[kv.first];
                route.lastUdpSendTick = nowTick;
                const bool routeAlive = route.directConfirmed || (route.lastUdpRecvTick != 0 && (nowTick - route.lastUdpRecvTick) < 3000);
                if (!routeAlive) {
                    allDirectConfirmed = false;
                }
            }
        }
        for (const auto& destAddr : targets) {
            sendto(m_socket, reinterpret_cast<const char*>(wireData), static_cast<int>(totalBytes), 0,
                   reinterpret_cast<const sockaddr*>(&destAddr), sizeof(destAddr));
        }
        if (m_relayActive.load() && (!allDirectConfirmed || port == 1001)) {
            {
                std::lock_guard<std::mutex> lock(m_relayMutex);
                if (m_relaySendQueue.size() < 2048) {
                    m_relaySendQueue.emplace_back(wireData, wireData + totalBytes);
                }
            }
            m_relaySendCv.notify_one();
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
            auto& route = m_peerRouteStates[targetPeerId];
            route.lastUdpSendTick = nowTick;
            directConfirmed = route.directConfirmed || (route.lastUdpRecvTick != 0 && (nowTick - route.lastUdpRecvTick) < 3000);
        } else {
            // Target route unknown yet: hold packet (Star-routing) instead of broadcasting (Finding #9, #10)
            auto& q = m_heldPackets[targetPeerId];
            if (q.size() < 128) {
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
            NOBLE_INFO("NetworkTransport: [HOST] Sent packet #{} to Peer 0x{:X} ({}:{}) ({} payload bytes, result: {}, direct: {})",
                       packetNum, targetPeerId, ipStr, ntohs(destAddr.sin_port), size, sent, directConfirmed);
        }
    }

    // If direct UDP is not confirmed active, or peer UDP route is unknown, or out-of-band control (port 1001), fallback via relay
    if (m_relayActive.load() && (!foundInUdp || !directConfirmed || port == 1001)) {
        {
            std::lock_guard<std::mutex> lock(m_relayMutex);
            if (m_relaySendQueue.size() < 2048) {
                m_relaySendQueue.emplace_back(wireData, wireData + totalBytes);
            }
        }
        m_relaySendCv.notify_one();
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
    if (port == 1000) {
        // Port 1000 is the engine's channel endpoint. Seeing a peer here proves that its
        // haloreach.dll session has progressed past the out-of-band join handshake.
        std::lock_guard<std::mutex> lock(m_mutex);
        m_gamePeers.insert(sender);
    }
    pkt.fromRelay = fromRelay;
    pkt.data.assign(wire + kWireHeaderBytes, wire + wireLen);
    auto& q = m_portQueues[port];
    if (q.size() >= 2048) {
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
    for (int drained = 0; drained < 512; ++drained) {
        sockaddr_in senderAddr = {};
        int senderLen = sizeof(senderAddr);
        const int received = recvfrom(m_socket, reinterpret_cast<char*>(rawBuf), sizeof(rawBuf), 0,
                                      reinterpret_cast<sockaddr*>(&senderAddr), &senderLen);
        if (received == SOCKET_ERROR) {
            const int wsaErr = WSAGetLastError();
            if (wsaErr == WSAEMSGSIZE) {
                static std::atomic<int> s_oversizeLogs{0};
                if (s_oversizeLogs.fetch_add(1) < 20) {
                    NOBLE_WARN("NetworkTransport: dropped a datagram larger than the {}-byte receive buffer", sizeof(rawBuf));
                }
                continue;
            }
            if (wsaErr == WSAECONNRESET || wsaErr == WSAECONNREFUSED) {
                continue;  // Ignore ICMP Port Unreachable from NAT hole-punch / localhost probes
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

        // Disconnect datagram (DISC)
        if (received == static_cast<int>(sizeof(uint64_t) + 4) &&
            rawBuf[8] == 'D' && rawBuf[9] == 'I' && rawBuf[10] == 'S' && rawBuf[11] == 'C') {
            if (wireSender != 0 && wireSender != m_localMachineId) {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_peerTable.erase(wireSender);
                m_gamePeers.erase(wireSender);
                m_peerRouteStates.erase(wireSender);
                m_heldPackets.erase(wireSender);
                for (auto it = m_addrToPeer.begin(); it != m_addrToPeer.end();) {
                    if (it->second == wireSender) it = m_addrToPeer.erase(it);
                    else ++it;
                }
                NOBLE_INFO("NetworkTransport: [{}] Remote peer 0x{:X} sent DISC; removed from peer table immediately.",
                           m_isHost ? "HOST" : "GUEST", wireSender);
            }
            continue;
        }

        // NAT hole-punch PING / PONG packet
        if (received == static_cast<int>(sizeof(uint64_t) + 4) &&
            rawBuf[8] == 'P' && (rawBuf[9] == 'I' || rawBuf[9] == 'O') && rawBuf[10] == 'N' && rawBuf[11] == 'G') {
            const bool isPong = (rawBuf[9] == 'O');
            if (wireSender != 0 && wireSender != m_localMachineId) {
                bool sendPingProbe = false;
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_peerTable[wireSender] = senderAddr;
                    m_addrToPeer[addrKey] = wireSender;
                    auto& route = m_peerRouteStates[wireSender];
                    route.addr = senderAddr;
                    route.lastUdpRecvTick = now;
                    route.directConfirmed = true;
                    route.lastPongRecvTick = now;
                    if (isPong && route.lastPingSentTick != 0 && now >= route.lastPingSentTick) {
                        const int sample = static_cast<int>(std::min<uint64_t>(999, now - route.lastPingSentTick));
                        route.rttMs = (route.rttMs < 0) ? sample : ((route.rttMs * 3 + sample) / 4);
                    } else if (route.rttMs < 0 && route.lastUdpSendTick != 0 && now >= route.lastUdpSendTick && (now - route.lastUdpSendTick) < 1000) {
                        const int sample = static_cast<int>(now - route.lastUdpSendTick);
                        route.rttMs = sample;
                    }
                    if (!isPong && (route.lastPingSentTick == 0 || (now - route.lastPingSentTick) >= 1000)) {
                        route.lastPingSentTick = now;
                        sendPingProbe = true;
                    }
                }

                // Flush any packets that were held waiting for this peer's address
                FlushHeldPackets(wireSender, senderAddr);

                if (m_isHost && !isPong) {
                    // Echo PONG back to Guest over direct UDP (burst 3) so Guest measures its RTT and confirms bidirectional UDP
                    uint8_t pong[sizeof(uint64_t) + 4] = { 0 };
                    std::memcpy(pong, &m_localMachineId, sizeof(uint64_t));
                    pong[sizeof(uint64_t)]     = 'P';
                    pong[sizeof(uint64_t) + 1] = 'O';
                    pong[sizeof(uint64_t) + 2] = 'N';
                    pong[sizeof(uint64_t) + 3] = 'G';
                    for (int p = 0; p < 3; ++p) {
                        sendto(m_socket, reinterpret_cast<const char*>(pong), sizeof(pong), 0,
                               reinterpret_cast<const sockaddr*>(&senderAddr), sizeof(senderAddr));
                    }
                    if (sendPingProbe) {
                        // Also send a PING probe to Guest so Guest echoes PONG and Host gets an immediate 1-RTT sample
                        uint8_t pingPkt[sizeof(uint64_t) + 4] = { 0 };
                        std::memcpy(pingPkt, &m_localMachineId, sizeof(uint64_t));
                        pingPkt[sizeof(uint64_t)]     = 'P';
                        pingPkt[sizeof(uint64_t) + 1] = 'I';
                        pingPkt[sizeof(uint64_t) + 2] = 'N';
                        pingPkt[sizeof(uint64_t) + 3] = 'G';
                        sendto(m_socket, reinterpret_cast<const char*>(pingPkt), sizeof(pingPkt), 0,
                               reinterpret_cast<const sockaddr*>(&senderAddr), sizeof(senderAddr));
                    }
                    NOBLE_INFO("NetworkTransport: [HOST] Registered peer 0x{:X} via NAT punch PING from {} (echoed PONG)",
                               wireSender, addrKey);
                } else if (!m_isHost) {
                    if (wireSender == m_hostMachineId) {
                        m_remoteHostAddr = senderAddr;
                    }
                    // Echo PONG back so the Host also gets an exact 1-RTT sample and bidirectional confirmation for this Guest
                    if (!isPong) {
                        uint8_t pong[sizeof(uint64_t) + 4] = { 0 };
                        std::memcpy(pong, &m_localMachineId, sizeof(uint64_t));
                        pong[sizeof(uint64_t)]     = 'P';
                        pong[sizeof(uint64_t) + 1] = 'O';
                        pong[sizeof(uint64_t) + 2] = 'N';
                        pong[sizeof(uint64_t) + 3] = 'G';
                        for (int p = 0; p < 3; ++p) {
                            sendto(m_socket, reinterpret_cast<const char*>(pong), sizeof(pong), 0,
                                   reinterpret_cast<const sockaddr*>(&senderAddr), sizeof(senderAddr));
                        }
                        if (sendPingProbe) {
                            uint8_t pingPkt[sizeof(uint64_t) + 4] = { 0 };
                            std::memcpy(pingPkt, &m_localMachineId, sizeof(uint64_t));
                            pingPkt[sizeof(uint64_t)]     = 'P';
                            pingPkt[sizeof(uint64_t) + 1] = 'I';
                            pingPkt[sizeof(uint64_t) + 2] = 'N';
                            pingPkt[sizeof(uint64_t) + 3] = 'G';
                            sendto(m_socket, reinterpret_cast<const char*>(pingPkt), sizeof(pingPkt), 0,
                                   reinterpret_cast<const sockaddr*>(&senderAddr), sizeof(senderAddr));
                        }
                    }
                    NOBLE_INFO("NetworkTransport: [GUEST] Received NAT punch {} from Host ({})", isPong ? "PONG" : "PING", addrKey);
                }
            }
            continue;
        }

        if (!QueueWirePacket(rawBuf, static_cast<size_t>(received), false)) {
            continue;
        }

        if (!m_isHost && wireSender == m_hostMachineId) {
            m_remoteHostAddr = senderAddr;
            std::lock_guard<std::mutex> lock(m_mutex);
            auto& route = m_peerRouteStates[wireSender];
            route.addr = senderAddr;
            route.lastUdpRecvTick = now;
            route.directConfirmed = true;
            route.lastPongRecvTick = now;
            if (route.rttMs < 0 && route.lastUdpSendTick != 0 && now >= route.lastUdpSendTick && (now - route.lastUdpSendTick) < 500) {
                route.rttMs = static_cast<int>(now - route.lastUdpSendTick);
            }
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
                route.lastPongRecvTick = now;
                if (route.rttMs < 0 && route.lastUdpSendTick != 0 && now >= route.lastUdpSendTick && (now - route.lastUdpSendTick) < 500) {
                    route.rttMs = static_cast<int>(now - route.lastUdpSendTick);
                }
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
                wire[8] == 'P' && (wire[9] == 'I' || wire[9] == 'O') && wire[10] == 'N' && wire[11] == 'G') {
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

    m_lastRemotePacketMs.store(GetTickCount64());
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
