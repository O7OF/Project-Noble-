#include <functional>
#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <condition_variable>
#include <cstdint>
#include <string>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
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

    // Writes a host-supplied game-mode body under data/host_variants and returns its absolute path.
    // Used by the lobby_launch handler and by the headless -host-variant-test.
    static bool MaterializeHostVariant(const std::string& name, const std::string& body, std::string& outPath);
    // Writes/reads/removes a tiny host-variant body through the same helper.
    static bool SelfTest();

    // Fixed session identifier shared by all Project Noble matches
    // Closed-source session id ("PNCSLOB1"). Deliberately not the open-source "HALOREAC",
    // so an old client cannot join a closed-source host even over direct UDP.
    static constexpr uint64_t kNobleSessionId = 0x504E43534C4F4231ULL;
    // Well-known Machine ID for the lobby host
    static constexpr uint64_t kDefaultHostMachineId = 0x484F535400000001ULL; // "HOST...1"

    bool Initialize(bool isHost, const std::string& remoteIp = "", uint16_t remotePort = 11774, uint16_t localPort = 11774);
    void Shutdown();

    // Chat: the relay carries JSON chat frames; the UI owns rendering.
    struct ChatMessage { std::string channel, from, text, target; };
    using ChatCallback = std::function<void(const ChatMessage&)>;
    void SetChatCallback(ChatCallback cb);
    bool SendChat(const std::string& channel, const std::string& text, const std::string& from, const std::string& target = "");

    // Join handshake (host -> guests, repeated while the match runs): tells a joining client that the host's
    // simulation is actually running, so a guest - including one joining mid-match - can time its start to it
    // instead of guessing with a fixed delay.
    bool SendSessionSync(uint64_t frame, bool running);
    bool HostSimulationRunning() const { return m_hostSimRunning.load(); }
    uint64_t HostSimulationFrame() const { return m_hostSimFrame.load(); }

    bool IsActive() const { return m_active.load(); }
    bool IsHost() const { return m_isHost; }
    uint64_t GetLocalMachineId() const { return m_localMachineId; }
    uint64_t GetHostMachineId() const { return m_hostMachineId; }
    uint64_t GetSessionId() const { return m_sessionId; }
    uint16_t GetLocalPort() const { return m_localPort; }

    // Number of distinct remote peers whose Blam! channel traffic (engine port 1000) has
    // arrived in this process. Used as the join barrier for campaign co-op / Forge.
    size_t GetGamePeerCount() const;

    // Seconds since the last packet arrived from a remote peer (guest: the host). A guest whose host vanished keeps
    // getting engine frames but no packets; PauseMenu uses this to show a connection-lost panel instead of freezing.
    double SecondsSinceRemotePacket() const;

    void GetSessionGuid(uint8_t outGuid16[16]) const;

    // Direct peer registration & NAT hole punching
    void AddPeer(uint64_t machineId, const std::string& ip, uint16_t port);
    void SetRemoteHostLanAddress(const std::string& lanIp, uint16_t port = 0);

    // Real RTT ping measurement (ms) per remote peer machineId (0 = default/host peer, returns -1 if unknown)
    int GetPeerPingMs(uint64_t machineId = 0) const;
    std::unordered_map<uint64_t, int> GetAllPeerPingsMs() const;

    // Master Server WebSocket packet relay & lobby signaling
    using LobbyLaunchCallback = std::function<void(const std::string& map, const std::string& mode, const std::string& variant, int diff, uint64_t skulls, const std::string& hostIp, uint16_t hostPort, const std::string& sessionId, const std::string& mapOverride, const std::string& modFolder, const std::string& modId)>;
    using LobbyRosterCallback = std::function<void(const std::string& rosterJson)>;
    using LobbyClosedCallback = std::function<void(const std::string& reason)>;
    using LobbyReturnCallback = std::function<void()>;

    void SetLobbyLaunchCallback(LobbyLaunchCallback cb);
    void SetLobbyRosterCallback(LobbyRosterCallback cb);
    void SetLobbyClosedCallback(LobbyClosedCallback cb);
    void SetLobbyReturnCallback(LobbyReturnCallback cb);
    void SendLobbyControlJson(const std::string& jsonString);

    void StartRelay(const std::string& relayUrl, const std::string& sessionId, bool isHost, uint64_t machineId);
    void StopRelay();

    // Voice frames share the relay socket but not the game packets' queues: they're dropped rather than ever delaying a
    // game packet. The server recognises them by their "NVOX" start. See audio/VoiceChat.cpp for the layout.
    using VoiceFrameCallback = void(*)(const uint8_t* data, size_t size);
    void SetVoiceFrameCallback(VoiceFrameCallback cb);
    void SendVoiceFrame(const uint8_t* data, size_t size);
    bool IsRelayConnected() const { return m_relayConnected.load(); }
    uint64_t GetRelayMachineId() const { return m_relayMachineId.load(); }

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

    // Host Slot 41 callback: Engine sending packet to a remote peer. port = destination endpoint port
    // (haloreach.dll uses 1000 for channel traffic and 1001 for out-of-band messages).
    uint64_t OnNetSend(uint64_t targetPeerId, const uint8_t* data, size_t size, uint16_t port);

    // Host Slot 43 callback: Engine polling for received packets on the endpoint with this port.
    uint64_t OnNetRecv(uint8_t* outBuffer, size_t bufferCapacity, uint64_t* outSenderPeerId, uint16_t port);

private:
    NetworkTransport();
    ~NetworkTransport();

    void RelayWorkerThread(std::string relayUrl, std::string sessionId, bool isHost, uint64_t machineId);

    // Received game packets, one queue per destination port (see OnNetSend / OnNetRecv).
    struct QueuedPacket {
        uint64_t sender = 0;
        bool fromRelay = false;
        std::vector<uint8_t> data;
    };
    bool QueueWirePacket(const uint8_t* wire, size_t wireLen, bool fromRelay);
    std::mutex m_recvMutex;
    std::unordered_map<uint16_t, std::deque<QueuedPacket>> m_portQueues;

    std::atomic<bool> m_active{ false };
    std::atomic<void*> m_relayWs{ nullptr };
    ChatCallback m_chatCb;
    std::mutex m_chatCbMutex;
    std::atomic<uint64_t> m_lastRemotePacketMs{ 0 };
    bool m_isHost = true;
    SOCKET m_socket = INVALID_SOCKET;
    uint16_t m_localPort = 11774;

    sockaddr_in m_remoteHostAddr = {};
    sockaddr_in m_remoteHostLanAddr = {};
    uint64_t m_lastPunchTick = 0;
    uint64_t m_localMachineId = 0;
    uint64_t m_hostMachineId = 0;
    uint64_t m_sessionId = kNobleSessionId;

    mutable std::mutex m_mutex;
    std::unordered_map<uint64_t, sockaddr_in> m_peerTable;
    std::unordered_map<std::string, uint64_t> m_addrToPeer;
    // Distinct peers observed sending Blam! channel traffic (port 1000).
    std::unordered_set<uint64_t> m_gamePeers;

    std::atomic<uint64_t> m_packetsSent{ 0 };
    std::atomic<uint64_t> m_packetsRecv{ 0 };
    std::atomic<uint64_t> m_lastUdpRecvTick{ 0 };

    // Relay members
    std::atomic<bool> m_relayActive{ false };
    std::atomic<bool> m_relayConnected{ false };
    std::atomic<uint64_t> m_relayMachineId{ 0 };
    std::atomic<VoiceFrameCallback> m_voiceFrameCb{ nullptr };
    std::deque<std::vector<uint8_t>> m_relayVoiceSendQueue;
    std::string m_relayUrl;
    std::thread m_relayThread;
    std::mutex m_relayMutex;
    std::condition_variable m_relaySendCv;
    void* m_relaySessionHandle = nullptr;
    void* m_relayWsHandle = nullptr;
    std::deque<std::vector<uint8_t>> m_relaySendQueue;
    std::deque<std::vector<uint8_t>> m_relayRecvQueue;
    std::deque<std::string> m_relayTextSendQueue;
    LobbyLaunchCallback m_lobbyLaunchCb;
    LobbyRosterCallback m_lobbyRosterCb;
    LobbyClosedCallback m_lobbyClosedCb;
    LobbyReturnCallback m_lobbyReturnCb;
    std::atomic<bool> m_hostSimRunning{ false };    // last session_sync said the host's simulation is running
    std::atomic<uint64_t> m_hostSimFrame{ 0 };      // host frame number from that message

    struct HeldPacket {
        std::vector<uint8_t> data;
        uint64_t enqueueTick = 0;
    };
    struct PeerRouteState {
        sockaddr_in addr{};
        uint64_t lastUdpRecvTick = 0;
        uint64_t lastUdpSendTick = 0;
        uint64_t lastPingSentTick = 0;
        uint64_t lastPongRecvTick = 0;
        int rttMs = -1;
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
