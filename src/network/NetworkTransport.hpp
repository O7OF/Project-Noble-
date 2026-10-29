#include <functional>
#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdint>
#include <string>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <deque>
#include <thread>
#include <atomic>

namespace noble {

class NetworkTransport {
public:
    static NetworkTransport& Get() {
        static NetworkTransport instance;
        return instance;
    }

    // Fixed session identifier shared by all Project Noble matches
    static constexpr uint64_t kNobleSessionId = 0x48414C4F52454143ULL;     // "HALOREAC"
    // Well-known Machine ID for the lobby host
    static constexpr uint64_t kDefaultHostMachineId = 0x484F535400000001ULL; // "HOST...1"

    bool Initialize(bool isHost, const std::string& remoteIp = "", uint16_t remotePort = 11774, uint16_t localPort = 11774);
    void Shutdown();

    bool IsActive() const { return m_active.load(); }
    bool IsHost() const { return m_isHost; }
    uint64_t GetLocalMachineId() const { return m_localMachineId; }
    uint64_t GetHostMachineId() const { return m_hostMachineId; }
    uint64_t GetSessionId() const { return m_sessionId; }
    uint16_t GetLocalPort() const { return m_localPort; }

    void GetSessionGuid(uint8_t outGuid16[16]) const;

    // Direct peer registration & NAT hole punching
    void AddPeer(uint64_t machineId, const std::string& ip, uint16_t port);

    // Master Server WebSocket packet relay & lobby signaling
    using LobbyLaunchCallback = std::function<void(const std::string& map, const std::string& mode, const std::string& variant, int diff, uint64_t skulls, const std::string& hostIp, uint16_t hostPort, const std::string& sessionId)>;
    using LobbyRosterCallback = std::function<void(const std::string& rosterJson)>;
    using LobbyClosedCallback = std::function<void(const std::string& reason)>;

    void SetLobbyLaunchCallback(LobbyLaunchCallback cb);
    void SetLobbyRosterCallback(LobbyRosterCallback cb);
    void SetLobbyClosedCallback(LobbyClosedCallback cb);
    void SendLobbyControlJson(const std::string& jsonString);

    void StartRelay(const std::string& relayUrl, const std::string& sessionId, bool isHost, uint64_t machineId);
    void StopRelay();

    // Simulation Profiler & Network Telemetry
    struct NetworkProfile {
        uint64_t packetsSent = 0;
        uint64_t packetsRecv = 0;
        uint64_t bytesSent = 0;
        uint64_t bytesRecv = 0;
        uint32_t sendRateBps = 0;
        uint32_t recvRateBps = 0;
        uint32_t packetsSentPerSec = 0;
        uint32_t packetsRecvPerSec = 0;
        uint64_t duplicatePacketsDropped = 0;
        uint64_t heldPacketsCount = 0;
        bool directUdpActive = false;
        bool relayActive = false;
    };
    NetworkProfile GetNetworkProfile() const;

    // Host Slot 41 callback: Engine sending packet to a remote peer
    uint64_t OnNetSend(uint64_t targetPeerId, const uint8_t* data, size_t size);

    // Host Slot 43 callback: Engine polling for received packets
    uint64_t OnNetRecv(uint8_t* outBuffer, size_t bufferCapacity, uint64_t* outSenderPeerId);

private:
    NetworkTransport();
    ~NetworkTransport();

    void RelayWorkerThread(std::string relayUrl, std::string sessionId, bool isHost, uint64_t machineId);

    std::atomic<bool> m_active{ false };
    bool m_isHost = true;
    SOCKET m_socket = INVALID_SOCKET;
    uint16_t m_localPort = 11774;

    sockaddr_in m_remoteHostAddr = {};
    uint64_t m_localMachineId = 0;
    uint64_t m_hostMachineId = 0;
    uint64_t m_sessionId = kNobleSessionId;

    std::mutex m_mutex;
    std::unordered_map<uint64_t, sockaddr_in> m_peerTable;
    std::unordered_map<std::string, uint64_t> m_addrToPeer;

    std::atomic<uint64_t> m_packetsSent{ 0 };
    std::atomic<uint64_t> m_packetsRecv{ 0 };
    std::atomic<uint64_t> m_lastUdpRecvTick{ 0 };

    // Relay members
    std::atomic<bool> m_relayActive{ false };
    std::string m_relayUrl;
    std::thread m_relayThread;
    std::mutex m_relayMutex;
    void* m_relaySessionHandle = nullptr;
    void* m_relayWsHandle = nullptr;
    std::deque<std::vector<uint8_t>> m_relaySendQueue;
    std::deque<std::vector<uint8_t>> m_relayRecvQueue;
    std::deque<std::string> m_relayTextSendQueue;
    LobbyLaunchCallback m_lobbyLaunchCb;
    LobbyRosterCallback m_lobbyRosterCb;
    LobbyClosedCallback m_lobbyClosedCb;

    struct HeldPacket {
        std::vector<uint8_t> data;
        uint64_t enqueueTick = 0;
    };
    struct PeerRouteState {
        sockaddr_in addr{};
        uint64_t lastUdpRecvTick = 0;
        uint64_t lastUdpSendTick = 0;
        bool directConfirmed = false;
    };

    void FlushHeldPackets(uint64_t peerId, const sockaddr_in& destAddr);
    uint64_t ComputePacketHash(const uint8_t* data, size_t size) const;
    bool IsDuplicatePacket(uint64_t hash);

    std::unordered_map<uint64_t, std::deque<HeldPacket>> m_heldPackets;
    std::unordered_map<uint64_t, PeerRouteState> m_peerRouteStates;

    static constexpr size_t kDedupeBufferSize = 128;
    uint64_t m_dedupeHashes[kDedupeBufferSize] = { 0 };
    size_t m_dedupeIndex = 0;
    std::atomic<uint64_t> m_duplicatesDropped{ 0 };

    std::atomic<uint64_t> m_bytesSent{ 0 };
    std::atomic<uint64_t> m_bytesRecv{ 0 };
    mutable std::mutex m_profMutex;
    mutable uint64_t m_lastProfTick{ 0 };
    mutable uint64_t m_lastSentBytesSnapshot{ 0 };
    mutable uint64_t m_lastRecvBytesSnapshot{ 0 };
    mutable uint64_t m_lastSentPacketsSnapshot{ 0 };
    mutable uint64_t m_lastRecvPacketsSnapshot{ 0 };
    mutable uint32_t m_sendRateBps{ 0 };
    mutable uint32_t m_recvRateBps{ 0 };
    mutable uint32_t m_packetsSentPerSec{ 0 };
    mutable uint32_t m_packetsRecvPerSec{ 0 };

    std::atomic<bool> m_relayStopRequested{ false };
};

} // namespace noble
