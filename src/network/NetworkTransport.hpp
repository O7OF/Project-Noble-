#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdint>
#include <string>
#include <mutex>
#include <unordered_map>
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

    void GetSessionGuid(uint8_t outGuid16[16]) const;

    // Host Slot 41 callback: Engine sending packet to a remote peer
    uint64_t OnNetSend(uint64_t targetPeerId, const uint8_t* data, size_t size);

    // Host Slot 43 callback: Engine polling for received packets
    uint64_t OnNetRecv(uint8_t* outBuffer, size_t bufferCapacity, uint64_t* outSenderPeerId);

private:
    NetworkTransport();
    ~NetworkTransport();

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
};

} // namespace noble

