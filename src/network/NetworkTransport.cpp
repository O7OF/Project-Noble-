#include "NetworkTransport.hpp"
#include "MasterServerClient.hpp"
#include "career/Career.hpp"
#include "common/BuildIdentity.hpp"
#include "common/Logger.hpp"
#include "network/HostModeration.hpp"
#include "network/HostVotes.hpp"
#include "common/Settings.hpp"
#include "engine/AppearanceSync.hpp"
#include "engine/DevCheats.hpp"
#include "engine/SharedTagCache.hpp"
#include "host/EngineLimits.hpp"
#include "ui/PauseMenu.hpp"
#include <nlohmann/json.hpp>

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <deque>
#include <vector>
#include <cctype>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <random>
#include <windows.h>
#include <winhttp.h>

namespace noble {

namespace {
std::string SanitizeVariantFilename(const std::string &name,
                                    const std::string &requiredExt) {
  std::string base = std::filesystem::path(name).filename().string();
  std::string clean;
  clean.reserve(base.size());
  for (unsigned char c : base) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-' ||
        c == ' ') {
      clean.push_back(static_cast<char>(c));
    } else {
      clean.push_back('_');
    }
  }
  while (!clean.empty() && (clean.front() == '.' || clean.front() == ' '))
    clean.erase(clean.begin());
  if (clean.empty())
    return {};
  if (!requiredExt.empty()) {
    std::string lowerClean = clean;
    for (char &ch : lowerClean)
      ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    if (lowerClean.size() < requiredExt.size() ||
        lowerClean.compare(lowerClean.size() - requiredExt.size(),
                           requiredExt.size(), requiredExt) != 0) {
      clean += requiredExt;
    }
  }
  return clean;
}
} // namespace

bool NetworkTransport::Base64Encode(const uint8_t *data, size_t size,
                                    std::string &out) {
  static constexpr char kTable[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  out.clear();
  if (!data && size > 0)
    return false;
  out.reserve(((size + 2) / 3) * 4);
  for (size_t i = 0; i < size; i += 3) {
    const uint32_t a = data[i];
    const uint32_t b = (i + 1 < size) ? data[i + 1] : 0;
    const uint32_t c = (i + 2 < size) ? data[i + 2] : 0;
    const uint32_t v = (a << 16) | (b << 8) | c;
    out.push_back(kTable[(v >> 18) & 0x3F]);
    out.push_back(kTable[(v >> 12) & 0x3F]);
    out.push_back((i + 1 < size) ? kTable[(v >> 6) & 0x3F] : '=');
    out.push_back((i + 2 < size) ? kTable[v & 0x3F] : '=');
  }
  return true;
}

bool NetworkTransport::Base64Decode(const std::string &b64,
                                    std::vector<uint8_t> &out) {
  out.clear();
  if (b64.empty())
    return false;
  static const int kLookup[256] = {
      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
      52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
      -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
      15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
      -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
      41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
  };
  uint32_t buf = 0;
  int bits = 0;
  for (unsigned char c : b64) {
    if (c == '=' || std::isspace(c))
      continue;
    int val = kLookup[c];
    if (val < 0)
      return false;
    buf = (buf << 6) | static_cast<uint32_t>(val);
    bits += 6;
    if (bits >= 8) {
      bits -= 8;
      out.push_back(static_cast<uint8_t>((buf >> bits) & 0xFF));
    }
  }
  return !out.empty();
}

std::string NetworkTransport::Sha256Hex(const uint8_t *data, size_t size) {
  if (!data && size > 0)
    return {};
  return noble::limits::DigestHex(noble::limits::Sha256(data, size));
}

bool NetworkTransport::VerifySha256Hex(const uint8_t *data, size_t size,
                                       const std::string &expectedHex) {
  if (expectedHex.empty())
    return true;
  if (!data && size > 0)
    return false;
  std::string actual = Sha256Hex(data, size);
  if (actual.size() != expectedHex.size())
    return false;
  for (size_t i = 0; i < actual.size(); ++i) {
    if (std::tolower(static_cast<unsigned char>(actual[i])) !=
        std::tolower(static_cast<unsigned char>(expectedHex[i]))) {
      return false;
    }
  }
  return true;
}

bool NetworkTransport::MaterializeHostVariant(const std::string &name,
                                              const std::string &body,
                                              std::string &outPath) {
  return MaterializeHostVariantFromPayload(name, body, false, outPath);
}

bool NetworkTransport::MaterializeHostVariantFromPayload(
    const std::string &name, const std::string &body, bool isBase64,
    std::string &outPath) {
  const std::string clean = SanitizeVariantFilename(name, "");
  if (clean.empty() || body.empty())
    return false;
  std::vector<uint8_t> raw;
  if (isBase64) {
    if (!Base64Decode(body, raw) || raw.size() > 2 * 1024 * 1024)
      return false;
  } else {
    if (body.size() > 2 * 1024 * 1024)
      return false;
    raw.assign(body.begin(), body.end());
  }
  std::error_code ec;
  const auto dir = std::filesystem::current_path(ec) / "data" / "host_variants";
  std::filesystem::create_directories(dir, ec);
  const auto path = dir / clean;
  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  if (!out)
    return false;
  out.write(reinterpret_cast<const char *>(raw.data()),
            static_cast<std::streamsize>(raw.size()));
  out.close();
  if (!out)
    return false;
  outPath = path.string();
  return true;
}

bool NetworkTransport::MaterializeHostMapVariant(const std::string &name,
                                                 const std::string &base64Body,
                                                 std::string &outPath) {
  const std::string clean = SanitizeVariantFilename(name, ".mvar");
  if (clean.empty() || base64Body.empty())
    return false;
  std::vector<uint8_t> raw;
  if (!Base64Decode(base64Body, raw) || raw.size() > 4 * 1024 * 1024)
    return false;
  std::error_code ec;
  const auto dir =
      std::filesystem::current_path(ec) / "data" / "host_map_variants";
  std::filesystem::create_directories(dir, ec);
  const auto path = dir / clean;
  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  if (!out)
    return false;
  out.write(reinterpret_cast<const char *>(raw.data()),
            static_cast<std::streamsize>(raw.size()));
  out.close();
  if (!out)
    return false;
  outPath = path.string();
  return true;
}

bool NetworkTransport::SelfTest() {
  const std::string body =
      "{\"name\":\"SELFTEST\",\"base\":\"slayer_054.bin\"}";
  std::string path;
  if (!MaterializeHostVariant("_selftest.json", body, path))
    return false;
  std::ifstream in(path, std::ios::binary);
  const std::string roundtrip((std::istreambuf_iterator<char>(in)),
                              std::istreambuf_iterator<char>());
  in.close();
  std::error_code ec;
  std::filesystem::remove(path, ec);
  if (roundtrip != body)
    return false;

  const std::vector<uint8_t> mvarSample = { '_', 'b', 'l', 'f', 1, 2, 3, 4 };
  std::string b64;
  if (!Base64Encode(mvarSample.data(), mvarSample.size(), b64))
    return false;
  const std::string expectedSha = Sha256Hex(mvarSample.data(), mvarSample.size());
  if (!VerifySha256Hex(mvarSample.data(), mvarSample.size(), expectedSha))
    return false;
  std::string mvarPath;
  if (!MaterializeHostMapVariant("_selftest.mvar", b64, mvarPath))
    return false;
  std::ifstream min(mvarPath, std::ios::binary);
  const std::vector<uint8_t> mvarRoundtrip(
      (std::istreambuf_iterator<char>(min)), std::istreambuf_iterator<char>());
  min.close();
  std::filesystem::remove(mvarPath, ec);
  if (mvarRoundtrip != mvarSample)
    return false;

  PeerRouteState unconfirmed{};
  unconfirmed.lastUdpRecvTick = 1000;
  if (IsRouteBidirectionalAlive(unconfirmed, 1100))
    return false;
  PeerRouteState confirmed{};
  confirmed.pongConfirmed = true;
  confirmed.lastPongRecvTick = 1000;
  if (!IsRouteBidirectionalAlive(confirmed, 4000))
    return false;
  if (IsRouteBidirectionalAlive(confirmed, 5000))
    return false;
  return true;
}

namespace {

// Diagnostic: hex-dump Blam! payloads so the join handshake (55-byte join
// request, 16-byte host reply, 26/10-byte channel traffic) can be decoded from
// the log.
constexpr uint64_t kPacketDumpLimit = 60;
// [uint64_t senderMachineId][uint16_t port] in front of every game packet (see
// OnNetSend).
constexpr size_t kWireHeaderBytes = sizeof(uint64_t) + sizeof(uint16_t);
std::string HexDump(const uint8_t *data, size_t size, size_t maxBytes = 96) {
  static const char *kHex = "0123456789ABCDEF";
  const size_t n = size < maxBytes ? size : maxBytes;
  std::string out;
  out.reserve(n * 3 + 8);
  for (size_t i = 0; i < n; ++i) {
    out.push_back(kHex[data[i] >> 4]);
    out.push_back(kHex[data[i] & 0xF]);
    out.push_back(' ');
  }
  if (n < size)
    out += "...";
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

std::wstring Widen(const std::string &narrow) {
  if (narrow.empty())
    return L"";
  int req = MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), -1, nullptr, 0);
  if (req <= 0)
    return L"";
  std::wstring out(static_cast<size_t>(req), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), -1, &out[0], req);
  if (!out.empty() && out.back() == L'\0')
    out.pop_back();
  return out;
}

} // namespace

NetworkTransport::NetworkTransport()
    : m_localMachineId(kDefaultHostMachineId),
      m_hostMachineId(kDefaultHostMachineId) {}

NetworkTransport::~NetworkTransport() { Shutdown(); }

bool NetworkTransport::Initialize(bool isHost, const std::string &remoteIp,
                                  uint16_t remotePort, uint16_t localPort) {
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
    // A guest must use the same machine id the lobby roster published for it, or the host's scoreboard /
    // peer_pings keys never match the id on the wire. The launcher passes it through NOBLE_MACHINE_ID.
    uint64_t override = 0;
    wchar_t value[32] = {};
    if (GetEnvironmentVariableW(L"NOBLE_MACHINE_ID", value, 32) > 0) {
      // The launcher writes "0x%016llX"; a leading '0' is part of a valid hex id, so parse first and
      // only treat the result 0 as "not set" (this check rejected every launcher-provided id).
      wchar_t* end = nullptr;
      override = std::wcstoull(value, &end, 0);
    }
    m_localMachineId = override != 0 ? override : GenerateRandomMachineId();
    m_hostMachineId = kDefaultHostMachineId;
    if (override != 0)
      NOBLE_INFO("NetworkTransport: guest machine id from lobby: 0x{:X}", override);
  }

  std::memset(&m_remoteHostAddr, 0, sizeof(m_remoteHostAddr));
  std::memset(&m_remoteHostLanAddr, 0, sizeof(m_remoteHostLanAddr));
  m_lastPunchTick = 0;
  if (!isHost && !remoteIp.empty()) {
    m_remoteHostAddr.sin_family = AF_INET;
    m_remoteHostAddr.sin_port = htons(remotePort);
    if (inet_pton(AF_INET, remoteIp.c_str(), &m_remoteHostAddr.sin_addr) <= 0) {
      NOBLE_ERROR("NetworkTransport: Invalid remote host IP address '{}'",
                  remoteIp);
      return false;
    }
    NOBLE_INFO("NetworkTransport: Configured guest target host {}:{}", remoteIp,
               remotePort);
  }

  // Create UDP socket
  m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (m_socket == INVALID_SOCKET) {
    NOBLE_ERROR("NetworkTransport: Failed to create UDP socket (WSAError: {})",
                WSAGetLastError());
    return false;
  }

  // Disable Windows ICMP Port Unreachable (WSAECONNRESET / 10054) on UDP
  // sockets so that probing 127.0.0.1 or LAN/WAN NAT hole-punch ports before
  // the remote peer binds never injects error frames into recvfrom().
#ifndef SIO_UDP_CONNRESET
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)
#endif
  BOOL newBehavior = FALSE;
  DWORD bytesReturned = 0;
  WSAIoctl(m_socket, SIO_UDP_CONNRESET, &newBehavior, sizeof(newBehavior),
           nullptr, 0, &bytesReturned, nullptr, nullptr);

  // Set non-blocking mode
  u_long nonBlocking = 1;
  if (ioctlsocket(m_socket, FIONBIO, &nonBlocking) != 0) {
    NOBLE_WARN("NetworkTransport: Failed to set non-blocking socket mode");
  }

  // Set socket buffer sizes to 1 MB for zero-drop burst absorption
  int bufSize = 1024 * 1024;
  setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF,
             reinterpret_cast<const char *>(&bufSize), sizeof(bufSize));
  setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF,
             reinterpret_cast<const char *>(&bufSize), sizeof(bufSize));

  // Exclusive use of the port. On Windows SO_REUSEADDR (used here before
  // 2026-10-29) lets a SECOND process bind the same UDP port and take datagrams
  // meant for the first: a stray second Noble on the guest's PC bound 11774
  // next to the joined match. With the port taken, the bind below falls back to
  // a free port.
  BOOL exclusive = TRUE;
  setsockopt(m_socket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE,
             reinterpret_cast<const char *>(&exclusive), sizeof(exclusive));

  // Bind local port:
  // - Host binds localPort (11774) so UPnP / port-forwarding and LAN/WAN guests
  // reach it. Retry briefly
  //   in case the parent menu process from Relaunch::Start is still releasing
  //   11774, then fall back to 0.
  // - Guest ALWAYS binds an ephemeral port (0) so it never steals UDP 11774
  // from a local Host or receives
  //   its own outbound packets addressed to 11774.
  sockaddr_in bindAddr = {};
  bindAddr.sin_family = AF_INET;
  bindAddr.sin_addr.s_addr = INADDR_ANY;
  bindAddr.sin_port = htons(isHost ? localPort : 0);

  bool bound = (bind(m_socket, reinterpret_cast<sockaddr *>(&bindAddr),
                     sizeof(bindAddr)) == 0);
  if (!bound && isHost && localPort != 0) {
    for (int attempt = 0; attempt < 10 && !bound; ++attempt) {
      Sleep(25);
      bound = (bind(m_socket, reinterpret_cast<sockaddr *>(&bindAddr),
                    sizeof(bindAddr)) == 0);
    }
  }
  if (!bound) {
    bindAddr.sin_port = 0;
    if (bind(m_socket, reinterpret_cast<sockaddr *>(&bindAddr),
             sizeof(bindAddr)) != 0) {
      NOBLE_ERROR("NetworkTransport: Failed to bind UDP socket (WSAError: {})",
                  WSAGetLastError());
      closesocket(m_socket);
      m_socket = INVALID_SOCKET;
      return false;
    }
  }

  int bindLen = sizeof(bindAddr);
  if (getsockname(m_socket, reinterpret_cast<sockaddr *>(&bindAddr),
                  &bindLen) == 0) {
    m_localPort = ntohs(bindAddr.sin_port);
  }

  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_peerTable.clear();
    m_addrToPeer.clear();
    m_gamePeers.clear();
    m_relayPeers.clear();
    m_peerRouteStates.clear();
    m_heldPackets.clear();
    std::memset(m_dedupeEntries, 0, sizeof(m_dedupeEntries));
    m_dedupeIndex = 0;
  }

  m_packetsSent.store(0);
  m_packetsRecv.store(0);
  m_lastUdpRecvTick.store(0);

  m_active.store(true);
  NOBLE_INFO("NetworkTransport: Initialized {} on UDP port {} (Local Machine "
             "ID: 0x{:X}, Host ID: 0x{:X}, Session ID: 0x{:X})",
             m_isHost ? "HOST" : "GUEST", m_localPort, m_localMachineId,
             m_hostMachineId, m_sessionId);

  // If Guest, fire an immediate NAT hole-punch PING packet burst to the Host so
  // the local NAT mapping opens and the Host learns our external IP:port before
  // the first Blam! handshake packet.
  if (!m_isHost && m_remoteHostAddr.sin_addr.s_addr != 0) {
    uint8_t punch[sizeof(uint64_t) + 4] = {0};
    std::memcpy(punch, &m_localMachineId, sizeof(uint64_t));
    punch[sizeof(uint64_t)] = 'P';
    punch[sizeof(uint64_t) + 1] = 'I';
    punch[sizeof(uint64_t) + 2] = 'N';
    punch[sizeof(uint64_t) + 3] = 'G';
    for (int i = 0; i < 5; ++i) {
      sendto(m_socket, reinterpret_cast<const char *>(punch), sizeof(punch), 0,
             reinterpret_cast<const sockaddr *>(&m_remoteHostAddr),
             sizeof(m_remoteHostAddr));
    }
    // Also send a localhost punch to 127.0.0.1:remotePort so same-PC Online P2P
    // tests connect directly without relying on router Hairpin NAT (harmless if
    // the Host is on a remote PC since Guest is on an ephemeral port).
    if (m_remoteHostAddr.sin_addr.s_addr != htonl(INADDR_LOOPBACK)) {
      sockaddr_in localHostAddr = {};
      localHostAddr.sin_family = AF_INET;
      localHostAddr.sin_port = m_remoteHostAddr.sin_port;
      localHostAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
      for (int i = 0; i < 3; ++i) {
        sendto(m_socket, reinterpret_cast<const char *>(punch), sizeof(punch),
               0, reinterpret_cast<const sockaddr *>(&localHostAddr),
               sizeof(localHostAddr));
      }
    }
    NOBLE_INFO(
        "NetworkTransport: [GUEST] Sent initial NAT punch PING burst to {}:{}",
        remoteIp, remotePort);
  }

  return true;
}

double NetworkTransport::SecondsSinceRemotePacket() const {
  const uint64_t last = m_lastRemotePacketMs.load();
  if (last == 0)
    return 0.0;
  return static_cast<double>(GetTickCount64() - last) / 1000.0;
}

size_t NetworkTransport::GetGamePeerCount() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_gamePeers.size();
}

void NetworkTransport::Shutdown() {
  StopRelay();

  if (!m_active.exchange(false))
    return;

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
  m_relayPeers.clear();
  m_peerRouteStates.clear();
  m_heldPackets.clear();

  NOBLE_INFO("NetworkTransport: Shutdown complete.");
}

std::string NetworkTransport::TakeMatchOverJson() {
  std::lock_guard<std::mutex> lock(m_relayMutex);
  std::string out = std::move(m_matchOverJson);
  m_matchOverJson.clear();
  return out;
}

void NetworkTransport::ShutdownForExit() {
  // haloreach.dll's network thread can still be calling OnNetSend/OnNetRecv on this object when a guest
  // leaves at match end. Closing m_socket under recvfrom was crashing non-host clients. Tell peers we
  // left and stop the relay, then let process exit reclaim the socket.
  SendDisconnectPacket();
  StopRelay();
  m_active.store(false);
  NOBLE_INFO("NetworkTransport: exit teardown (disconnect sent; UDP socket left to process exit).");
}

void NetworkTransport::GetSessionGuid(uint8_t outGuid16[16]) const {
  if (!outGuid16)
    return;
  std::memcpy(&outGuid16[0], &m_sessionId, sizeof(m_sessionId));
  std::memcpy(&outGuid16[8], &m_sessionId, sizeof(m_sessionId));
}

void NetworkTransport::AddPeer(uint64_t machineId, const std::string &ip,
                               uint16_t port) {
  if (ip.empty() || port == 0)
    return;
  sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0)
    return;

  std::string key = ip + ":" + std::to_string(port);
  if (machineId != 0) {
    std::lock_guard<std::mutex> lock(m_mutex);
    // Do not overwrite an already-learned direct UDP peer address with a
    // signaling hint
    if (m_peerTable.find(machineId) == m_peerTable.end()) {
      m_peerTable[machineId] = addr;
      m_addrToPeer[key] = machineId;
      auto &route = m_peerRouteStates[machineId];
      route.addr = addr;
      // Do NOT mark directConfirmed/lastUdpRecvTick from a signaling hint;
      // wait until OnNetRecv observes an actual UDP packet or PONG from this peer
      // so WS relay fallback stays active during NAT punching / CGNAT JIP.
      route.directConfirmed = false;
      route.pongConfirmed = false;
      route.lastPongRecvTick = 0;
      route.lastUdpRecvTick = 0;
    }
  }

  // Fire UDP punch packet burst immediately to punch open NAT router firewall
  if (m_socket != INVALID_SOCKET) {
    if (machineId != 0) {
      std::lock_guard<std::mutex> lock(m_mutex);
      NoteProbeSent(m_peerRouteStates[machineId], GetTickCount64());
    }
    uint8_t punch[sizeof(uint64_t) + 4] = {0};
    std::memcpy(punch, &m_localMachineId, sizeof(uint64_t));
    punch[sizeof(uint64_t)] = 'P';
    punch[sizeof(uint64_t) + 1] = 'I';
    punch[sizeof(uint64_t) + 2] = 'N';
    punch[sizeof(uint64_t) + 3] = 'G';
    for (int i = 0; i < 3; ++i) {
      sendto(m_socket, reinterpret_cast<const char *>(punch), sizeof(punch), 0,
             reinterpret_cast<const sockaddr *>(&addr), sizeof(addr));
    }
    NOBLE_INFO(
        "NetworkTransport: Dispatched NAT punch datagrams to {} (Peer 0x{:X})",
        key, machineId);
  }
}

void NetworkTransport::SetRemoteHostLanAddress(const std::string &lanIp,
                                               uint16_t port) {
  if (lanIp.empty())
    return;
  sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = port != 0 ? htons(port) : m_remoteHostAddr.sin_port;
  if (inet_pton(AF_INET, lanIp.c_str(), &addr.sin_addr) <= 0)
    return;
  if (addr.sin_addr.s_addr == m_remoteHostAddr.sin_addr.s_addr)
    return;

  m_remoteHostLanAddr = addr;
  if (!m_isHost && m_socket != INVALID_SOCKET &&
      m_remoteHostLanAddr.sin_addr.s_addr != 0) {
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      NoteProbeSent(m_peerRouteStates[m_hostMachineId], GetTickCount64());
    }
    uint8_t punch[sizeof(uint64_t) + 4] = {0};
    std::memcpy(punch, &m_localMachineId, sizeof(uint64_t));
    punch[sizeof(uint64_t)] = 'P';
    punch[sizeof(uint64_t) + 1] = 'I';
    punch[sizeof(uint64_t) + 2] = 'N';
    punch[sizeof(uint64_t) + 3] = 'G';
    for (int i = 0; i < 3; ++i) {
      sendto(m_socket, reinterpret_cast<const char *>(punch), sizeof(punch), 0,
             reinterpret_cast<const sockaddr *>(&m_remoteHostLanAddr),
             sizeof(m_remoteHostLanAddr));
    }
    NOBLE_INFO(
        "NetworkTransport: [GUEST] Configured & punched Host LAN address {}:{}",
        lanIp, ntohs(m_remoteHostLanAddr.sin_port));
  }
}

void NetworkTransport::SendDisconnectPacket() {
  if (m_socket == INVALID_SOCKET)
    return;

  uint8_t disc[sizeof(uint64_t) + 4] = {0};
  std::memcpy(disc, &m_localMachineId, sizeof(uint64_t));
  disc[sizeof(uint64_t)] = 'D';
  disc[sizeof(uint64_t) + 1] = 'I';
  disc[sizeof(uint64_t) + 2] = 'S';
  disc[sizeof(uint64_t) + 3] = 'C';

  if (!m_isHost) {
    if (m_remoteHostAddr.sin_addr.s_addr != 0) {
      for (int i = 0; i < 3; ++i) {
        sendto(m_socket, reinterpret_cast<const char *>(disc), sizeof(disc), 0,
               reinterpret_cast<const sockaddr *>(&m_remoteHostAddr),
               sizeof(m_remoteHostAddr));
      }
    }
    if (m_remoteHostLanAddr.sin_addr.s_addr != 0) {
      for (int i = 0; i < 3; ++i) {
        sendto(m_socket, reinterpret_cast<const char *>(disc), sizeof(disc), 0,
               reinterpret_cast<const sockaddr *>(&m_remoteHostLanAddr),
               sizeof(m_remoteHostLanAddr));
      }
    }
  } else {
    std::vector<sockaddr_in> targets;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      targets.reserve(m_peerTable.size());
      for (const auto &kv : m_peerTable) {
        targets.push_back(kv.second);
      }
    }
    for (const auto &dest : targets) {
      for (int i = 0; i < 3; ++i) {
        sendto(m_socket, reinterpret_cast<const char *>(disc), sizeof(disc), 0,
               reinterpret_cast<const sockaddr *>(&dest), sizeof(dest));
      }
    }
  }

  if (m_relayConnected.load()) {
    nlohmann::json j = {{"type", "lobby_leave"}};
    SendLobbyControlJson(
        j.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace));
  }
  NOBLE_INFO("NetworkTransport: Dispatched disconnect datagrams (DISC).");
}

int NetworkTransport::GetPeerPingMs(uint64_t machineId) const {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (machineId == 0) {
    // 0 means "the host": the host itself is the reference point (0 ms), a guest measures the host.
    // Never return an arbitrary peer's RTT here - that is what made every remote row share one number.
    if (m_isHost)
      return 0;
    machineId = m_hostMachineId;
  }
  auto it = m_peerRouteStates.find(machineId);
  if (it == m_peerRouteStates.end() || it->second.rttMs < 0)
    return -1;
  const uint64_t last = it->second.lastPongRecvTick;
  if (last == 0 || GetTickCount64() - last > 5000ULL)
    return -1; // stale: unknown, not a ghost value from a peer that stopped answering
  return it->second.rttMs;
}

std::unordered_map<uint64_t, int> NetworkTransport::GetAllPeerPingsMs() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  std::unordered_map<uint64_t, int> out;
  const uint64_t now = GetTickCount64();
  for (const auto &kv : m_peerRouteStates) {
    if (kv.second.rttMs >= 0 && kv.second.lastPongRecvTick != 0 &&
        now - kv.second.lastPongRecvTick <= 5000ULL) {
      out[kv.first] = kv.second.rttMs;
    }
  }
  return out;
}

void NetworkTransport::StartRelay(const std::string &relayUrl,
                                  const std::string &sessionId, bool isHost,
                                  uint64_t machineId) {
  if (relayUrl.empty())
    return;
  {
    std::lock_guard<std::mutex> lock(m_relayMutex);
    if (m_relayActive.load() && m_relayUrl == relayUrl) {
      return;
    }
  }
  StopRelay();

  m_relayMachineId.store(machineId != 0 ? machineId
                                        : (isHost ? kDefaultHostMachineId : 0));
  m_relayStopRequested.store(false);
  {
    std::lock_guard<std::mutex> lock(m_relayMutex);
    m_relayUrl = relayUrl;
    m_relaySessionId = sessionId;
  }
  m_relayActive.store(true);
  m_relayThread = std::thread(&NetworkTransport::RelayWorkerThread, this,
                              relayUrl, sessionId, isHost, machineId);
  NOBLE_INFO("NetworkTransport: Starting WebSocket packet relay thread ({})",
             relayUrl);
}

void NetworkTransport::SetVoiceFrameCallback(VoiceFrameCallback cb) {
  m_voiceFrameCb.store(cb);
}

void NetworkTransport::SendVoiceFrame(const uint8_t *data, size_t size) {
  if (!data || size == 0)
    return;
  // Drive voice directly P2P over Host <-> Guest UDP when the match socket is
  // up (zero Render traffic).
  if (m_active.load() && m_socket != INVALID_SOCKET) {
    SendControlUdp("NVOX", data, size);
    return;
  }
  // Only local headless voice self-tests use WebSocket relay when no UDP match
  // socket exists.
  std::string sid;
  {
    std::lock_guard<std::mutex> lock(m_relayMutex);
    sid = m_relaySessionId;
  }
  if (!m_relayConnected.load() || sid != "voice-test")
    return;
  {
    std::lock_guard<std::mutex> lock(m_relayMutex);
    while (m_relayVoiceSendQueue.size() >= 10)
      m_relayVoiceSendQueue.pop_front(); // old speech is worthless
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

void NetworkTransport::FlushHeldPackets(uint64_t peerId,
                                        const sockaddr_in &destAddr) {
  std::deque<HeldPacket> toFlush;
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_heldPackets.find(peerId);
    if (it != m_heldPackets.end()) {
      toFlush = std::move(it->second);
      m_heldPackets.erase(it);
    }
  }
  if (toFlush.empty())
    return;

  const uint64_t now = GetTickCount64();
  size_t count = 0;
  for (const auto &pkt : toFlush) {
    // Anything older than a few ticks is worse than dropping it: the engine re-sends, while a
    // multi-second replay lands as an input backlog.
    if (now - pkt.enqueueTick > 250)
      continue;
    sendto(m_socket, reinterpret_cast<const char *>(pkt.data.data()),
           static_cast<int>(pkt.data.size()), 0,
           reinterpret_cast<const sockaddr *>(&destAddr), sizeof(destAddr));
    count++;
  }
  if (count > 0) {
    NOBLE_INFO("NetworkTransport: Flushed {} held packet(s) to peer 0x{:X}",
               count, peerId);
  }
}

uint64_t NetworkTransport::ComputePacketHash(const uint8_t *data,
                                             size_t size) const {
  uint64_t hash = 14695981039346656037ULL;
  for (size_t i = 0; i < size; ++i) {
    hash ^= static_cast<uint64_t>(data[i]);
    hash *= 1099511628211ULL;
  }
  return hash;
}

bool NetworkTransport::IsDuplicatePacket(uint64_t hash) {
  if (hash == 0)
    return false;
  const uint64_t now = GetTickCount64();
  for (size_t i = 0; i < kDedupeBufferSize; ++i) {
    const auto &entry = m_dedupeEntries[i];
    if (entry.hash == hash && entry.tickMs != 0 && (now - entry.tickMs) < 350ULL) {
      m_duplicatesDropped++;
      return true;
    }
  }
  m_dedupeEntries[m_dedupeIndex] = {hash, now};
  m_dedupeIndex = (m_dedupeIndex + 1) % kDedupeBufferSize;
  return false;
}

float NetworkTransport::GetPeerLossPercent(uint64_t machineId) const {
  std::lock_guard<std::mutex> lock(const_cast<std::mutex &>(m_mutex));
  const uint64_t id = machineId == 0 ? m_hostMachineId : machineId;
  const auto it = m_peerRouteStates.find(id);
  if (it == m_peerRouteStates.end() || it->second.probes < 5) return -1.0f;
  return 100.0f * static_cast<float>(it->second.probesLost) / static_cast<float>(it->second.probes);
}

NetworkTransport::ConnectionQuality NetworkTransport::GetConnectionQuality() const {
  ConnectionQuality q;
  std::lock_guard<std::mutex> lock(const_cast<std::mutex &>(m_mutex));
  const uint64_t now = GetTickCount64();
  for (const auto &kv : m_peerRouteStates) {
    const PeerRouteState &r = kv.second;
    if (!m_isHost && kv.first != m_hostMachineId) continue;
    if (r.lastUdpRecvTick == 0 || now - r.lastUdpRecvTick > 10000) continue;   // gone
    ++q.peers;
    if (r.rttMs >= 0 && r.rttMs > q.pingMs) q.pingMs = r.rttMs;
    if (r.probes >= 5) {
      const float loss = 100.0f * static_cast<float>(r.probesLost) / static_cast<float>(r.probes);
      if (loss > q.lossPercent) q.lossPercent = loss;
    }
  }
  return q;
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
    m_sendRateBps =
        static_cast<uint32_t>((curSentBytes - m_lastSentBytesSnapshot) / secs);
    m_recvRateBps =
        static_cast<uint32_t>((curRecvBytes - m_lastRecvBytesSnapshot) / secs);
    m_packetsSentPerSec =
        static_cast<uint32_t>((curSentPkts - m_lastSentPacketsSnapshot) / secs);
    m_packetsRecvPerSec =
        static_cast<uint32_t>((curRecvPkts - m_lastRecvPacketsSnapshot) / secs);

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
    std::lock_guard<std::mutex> lk(const_cast<std::mutex &>(m_mutex));
    uint64_t heldTotal = 0;
    for (const auto &kv : m_heldPackets) {
      heldTotal += kv.second.size();
    }
    prof.heldPacketsCount = heldTotal;
  }

  const uint64_t lastUdpTick = m_lastUdpRecvTick.load();
  prof.directUdpActive = (lastUdpTick != 0 && (now - lastUdpTick) < 3000);
  prof.relayActive = m_relayActive.load();

  return prof;
}

bool NetworkTransport::IsDirectUdpActive() const {
  if (!m_active.load() || m_socket == INVALID_SOCKET)
    return false;
  const uint64_t nowTick = GetTickCount64();
  std::lock_guard<std::mutex> lock(m_mutex);
  if (!m_isHost) {
    const auto it = m_peerRouteStates.find(m_hostMachineId);
    if (it == m_peerRouteStates.end())
      return false;
    return IsRouteBidirectionalAlive(it->second, nowTick);
  }
  if (m_peerTable.empty())
    return false;
  auto isRouteAlive = [&](uint64_t peerId) -> bool {
    const auto it = m_peerRouteStates.find(peerId);
    if (it == m_peerRouteStates.end())
      return false;
    return IsRouteBidirectionalAlive(it->second, nowTick);
  };
  for (uint64_t rp : m_relayPeers) {
    if (m_peerTable.find(rp) == m_peerTable.end() || !isRouteAlive(rp))
      return false;
  }
  for (const auto &kv : m_peerTable) {
    if (!isRouteAlive(kv.first))
      return false;
  }
  return true;
}

bool NetworkTransport::SendControlUdp(const char magic[4],
                                      const uint8_t *payload, size_t len,
                                      uint64_t onlyPeerId,
                                      uint64_t excludePeerId) {
  if (!m_active.load() || m_socket == INVALID_SOCKET || !magic || len > 65000)
    return false;
  std::vector<uint8_t> pkt(sizeof(uint64_t) + 4 + len);
  std::memcpy(pkt.data(), &m_localMachineId, sizeof(uint64_t));
  std::memcpy(pkt.data() + sizeof(uint64_t), magic, 4);
  if (len > 0 && payload) {
    std::memcpy(pkt.data() + sizeof(uint64_t) + 4, payload, len);
  }

  if (!m_isHost) {
    if (m_remoteHostAddr.sin_family != AF_INET ||
        m_remoteHostAddr.sin_port == 0)
      return false;
    sendto(m_socket, reinterpret_cast<const char *>(pkt.data()),
           static_cast<int>(pkt.size()), 0,
           reinterpret_cast<const sockaddr *>(&m_remoteHostAddr),
           sizeof(m_remoteHostAddr));
    const uint64_t nowTick = GetTickCount64();
    const uint64_t lastUdp = m_lastUdpRecvTick.load();
    const bool directAlive = (lastUdp != 0 && (nowTick - lastUdp) < 5000);
    if (!directAlive && m_remoteHostLanAddr.sin_addr.s_addr != 0 &&
        m_remoteHostLanAddr.sin_addr.s_addr !=
            m_remoteHostAddr.sin_addr.s_addr) {
      sendto(m_socket, reinterpret_cast<const char *>(pkt.data()),
             static_cast<int>(pkt.size()), 0,
             reinterpret_cast<const sockaddr *>(&m_remoteHostLanAddr),
             sizeof(m_remoteHostLanAddr));
    }
    m_bytesSent += pkt.size();
    return true;
  }

  std::vector<sockaddr_in> targets;
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    targets.reserve(m_peerTable.size());
    for (const auto &kv : m_peerTable) {
      if (onlyPeerId != 0 && kv.first != onlyPeerId)
        continue;
      if (excludePeerId != 0 && kv.first == excludePeerId)
        continue;
      targets.push_back(kv.second);
    }
  }
  if (targets.empty())
    return false;
  for (const auto &destAddr : targets) {
    sendto(m_socket, reinterpret_cast<const char *>(pkt.data()),
           static_cast<int>(pkt.size()), 0,
           reinterpret_cast<const sockaddr *>(&destAddr), sizeof(destAddr));
    m_bytesSent += pkt.size();
  }
  return true;
}

bool NetworkTransport::SendSessionSync(uint64_t frame, bool running) {
  nlohmann::json j = {
      {"type", "session_sync"}, {"frame", frame}, {"running", running}};
  const std::string payload =
      j.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace);
  bool sent = false;
  if (m_active.load() && m_socket != INVALID_SOCKET) {
    sent = SendControlUdp("NBCJ",
                          reinterpret_cast<const uint8_t *>(payload.data()),
                          payload.size());
  }
  // Only fall back to WS relay while peers don't all have direct UDP confirmed
  // yet.
  if (m_relayConnected.load() && !IsDirectUdpActive()) {
    SendLobbyControlJson(payload);
    sent = true;
  }
  return sent;
}

void NetworkTransport::SendHostRemove(uint64_t machineId,
                                      const std::string &kind,
                                      const std::string &reason, int minutes) {
  if (machineId == 0 || !m_isHost)
    return;
  nlohmann::json j = {{"type", "host_remove"},
                      {"target", std::to_string(machineId)},
                      {"kind", kind},
                      {"reason", reason},
                      {"minutes", minutes}};
  SendLobbyControlJson(
      j.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace));
}

bool NetworkTransport::TakeHostRemoval(HostRemoval &out) {
  std::lock_guard<std::mutex> lock(m_removalMutex);
  if (!m_removalPending)
    return false;
  m_removalPending = false;
  if (GetTickCount64() - m_removalAtMs > 15000)
    return false; // old request: ignore
  out = m_removal;
  return true;
}

void NetworkTransport::SendPingLimitRefusal(uint64_t machineId, int limitMs,
                                            int pingMs) {
  if (machineId == 0)
    return;
  nlohmann::json j = {{"type", "host_ping_limit"},
                      {"target", std::to_string(machineId)},
                      {"limit", limitMs},
                      {"ping", pingMs}};
  SendLobbyControlJson(j.dump());
}

bool NetworkTransport::TakePingLimitRefusal(int &limitMs, int &pingMs) {
  if (!m_pingRefused.exchange(false))
    return false;
  if (GetTickCount64() - m_pingRefusedAtMs.load() > 10000)
    return false; // old request (arrived while not in a lobby): ignore
  limitMs = m_pingRefusedLimit.load();
  pingMs = m_pingRefusedPing.load();
  return true;
}

void NetworkTransport::SendHostPause(bool paused) {
  if (!m_isHost)
    return;
  SendLobbyControlJson(paused ? "{\"type\":\"host_pause\",\"paused\":true}"
                              : "{\"type\":\"host_pause\",\"paused\":false}");
}

uint64_t NetworkTransport::HostPauseAgeMs() const {
  const uint64_t at = m_hostPauseAtMs.load();
  return at == 0 ? UINT64_MAX : GetTickCount64() - at;
}

void NetworkTransport::SendLobbyControlJson(const std::string &jsonString) {
  if (jsonString.empty())
    return;
  if (m_active.load() && m_socket != INVALID_SOCKET) {
    SendControlUdp("NBCJ", reinterpret_cast<const uint8_t *>(jsonString.data()),
                   jsonString.size());
  }
  const bool lifecycleEvent =
      jsonString.find("lobby_disband") != std::string::npos ||
      jsonString.find("lobby_launch") != std::string::npos ||
      jsonString.find("host_return_to_lobby") != std::string::npos ||
      jsonString.find("host_pause") != std::string::npos ||
      jsonString.find("lobby_auth") != std::string::npos ||
      jsonString.find("lobby_admit") != std::string::npos;
  if (!m_active.load() || !IsDirectUdpActive() || lifecycleEvent) {
    {
      std::lock_guard<std::mutex> lock(m_relayMutex);
      if (m_relayActive.load()) {
        m_relayTextSendQueue.push_back(jsonString);
      }
    }
    m_relaySendCv.notify_one();
  }
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
  // Join even when the worker already stopped on its own (handshake failed,
  // server closed the socket): a finished-but-unjoined std::thread that gets a
  // new thread assigned calls std::terminate, which would end Noble with no
  // crash log.
  if (m_relayThread.joinable() &&
      m_relayThread.get_id() != std::this_thread::get_id()) {
    m_relayThread.join();
  }
  if (!wasActive)
    return;
  {
    std::lock_guard<std::mutex> lock(m_relayMutex);
    m_relayUrl.clear();
    m_relaySessionId.clear();
    m_relaySendQueue.clear();
    m_relayRecvQueue.clear();
    m_relayTextSendQueue.clear();
    m_relayVoiceSendQueue.clear();
  }
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_relayPeers.clear();
  }
  NOBLE_INFO("NetworkTransport: Stopped WebSocket packet relay.");
}

void NetworkTransport::SetChatCallback(ChatCallback cb) {
  std::lock_guard<std::mutex> lock(m_chatCbMutex);
  m_chatCb = std::move(cb);
}

// Lobby-only chat: in-match chat flows directly Host <-> Guest over UDP
// ("NBCJ"); pre-game lobby chat before UDP starts uses the active lobby WS
// room. Global and Direct Messages are removed.
bool NetworkTransport::SendChat(const std::string &channel,
                                const std::string &text,
                                const std::string &from,
                                const std::string &target) {
  (void)target;
  if (text.empty())
    return false;
  if (channel == "global" || channel == "dm" || channel == "whisper")
    return false;
  const std::string ch =
      (channel == "team" || channel == "party") ? channel : "all";
  nlohmann::json j = {
      {"type", "chat"},
      {"channel", ch},
      {"text", m_isHost ? HostModeration::FilterText(text) : text},
      {"from", from},
      // Sender's machine id, so a player can text-mute someone (per PC).
      {"sid", std::to_string(m_localMachineId)}};
  const std::string payload =
      j.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace);

  if (m_active.load() && m_socket != INVALID_SOCKET) {
    if (m_isHost) {
      SendControlUdp("NBCJ", reinterpret_cast<const uint8_t *>(payload.data()),
                     payload.size());
      HandleControlJson(payload, m_localMachineId);
      if (!IsDirectUdpActive() && m_relayConnected.load()) {
        std::lock_guard<std::mutex> lock(m_relayMutex);
        m_relayTextSendQueue.push_back(payload);
        m_relaySendCv.notify_one();
      }
      return true;
    }
    if (IsDirectUdpActive()) {
      return SendControlUdp("NBCJ",
                            reinterpret_cast<const uint8_t *>(payload.data()),
                            payload.size());
    }
  }

  if (!m_relayConnected.load())
    return false;
  {
    std::lock_guard<std::mutex> lock(m_relayMutex);
    m_relayTextSendQueue.push_back(payload);
  }
  m_relaySendCv.notify_one();
  return true;
}

void NetworkTransport::HandleControlJson(const std::string &msgStr,
                                         uint64_t udpSenderPeerId) {
  std::string sessionId;
  {
    std::lock_guard<std::mutex> lock(m_relayMutex);
    sessionId = m_relaySessionId;
  }
  try {
    auto j = nlohmann::json::parse(msgStr);
    std::string type = j.value("type", "");
    if (type == "lobby_launch" && udpSenderPeerId == 0) {
      auto ld = j.value("launch", nlohmann::json::object());
      std::string lMap = ld.value("map", "20_sword_slayer");
      std::string lMode = ld.value("mode", "multiplayer");
      std::string lVar = ld.value("variant", "");
      std::string lVarBody = ld.value("variant_body", "");
      bool lVarB64 = ld.value("variant_is_base64", false);
      std::string lVarSha = ld.value("variant_sha256", "");
      std::string lVarPath = lVar;
      if (!lVarBody.empty() && !lVar.empty()) {
        std::vector<uint8_t> rawCheck;
        bool shaOk = true;
        if (!lVarSha.empty()) {
          if (lVarB64 && Base64Decode(lVarBody, rawCheck)) {
            shaOk = VerifySha256Hex(rawCheck.data(), rawCheck.size(), lVarSha);
          } else if (!lVarB64) {
            shaOk = VerifySha256Hex(
                reinterpret_cast<const uint8_t *>(lVarBody.data()),
                lVarBody.size(), lVarSha);
          }
        }
        if (shaOk && MaterializeHostVariantFromPayload(lVar, lVarBody, lVarB64,
                                                       lVarPath)) {
          NOBLE_INFO("NetworkTransport: host variant body '{}' saved to {}",
                     lVar, lVarPath);
        } else if (!shaOk) {
          NOBLE_WARN("NetworkTransport: rejected variant '{}' due to SHA-256 "
                     "mismatch",
                     lVar);
        }
      }
      std::string lMapVariant = ld.value("map_variant", "");
      std::string lMapVarBody = ld.value("map_variant_body", "");
      std::string lMapVarSha = ld.value("map_variant_sha256", "");
      std::string lMapVarPath = lMapVariant;
      if (!lMapVarBody.empty() && !lMapVariant.empty()) {
        std::vector<uint8_t> mvarBytes;
        bool shaOk = lMapVarSha.empty() ||
                     (Base64Decode(lMapVarBody, mvarBytes) &&
                      VerifySha256Hex(mvarBytes.data(), mvarBytes.size(),
                                      lMapVarSha));
        if (shaOk &&
            MaterializeHostMapVariant(lMapVariant, lMapVarBody, lMapVarPath)) {
          NOBLE_INFO("NetworkTransport: host map variant '{}' saved to {}",
                     lMapVariant, lMapVarPath);
        } else if (!shaOk) {
          NOBLE_WARN("NetworkTransport: rejected map variant '{}' due to "
                     "SHA-256 mismatch",
                     lMapVariant);
        }
      }
      int lDiff = ld.value("difficulty", 1);
      uint64_t lSkulls = 0;
      try {
        lSkulls = std::stoull(ld.value("skulls", "0"));
      } catch (...) {
      }
      std::string hIp = ld.value("host_ip", "");
      if (ld.value("is_lan", false) && ld.contains("host_lan_ip")) {
        std::string lanIp = ld.value("host_lan_ip", "");
        if (!lanIp.empty())
          hIp = lanIp;
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
        cb(lMap, lMode, lVarPath, lDiff, lSkulls, hIp, hPort, sId, lMapOverride,
           lModFolder, lModId, lMapVarPath);
      }
    } else if (type == "chat") {
      const std::string rawCh = j.value("channel", "all");
      if (rawCh == "global" || rawCh == "dm" || rawCh == "whisper")
        return;
      ChatMessage m;
      m.channel = rawCh;
      m.from = j.value("from", "player");
      m.text = j.value("text", "");
      uint64_t sid = 0;
      try {
        sid = std::stoull(j.value("sid", std::string("0")));
      } catch (...) {
      }
      // Text-muted on this PC (the player's own choice): drop the line.
      if (HostModeration::IsChatMuted(sid, m.from))
        return;
      // The host's mute list and word filter (also for relay chat that never
      // passed through the host).
      if (!HostModeration::ApplyChatRules(sid, m.text))
        return;
      ChatCallback cb;
      {
        std::lock_guard<std::mutex> lock(m_chatCbMutex);
        cb = m_chatCb;
      }
      if (cb && !m.text.empty())
        cb(m);
    } else if (type == "lobby_roster" && udpSenderPeerId == 0) {
      LobbyRosterCallback cb;
      {
        std::lock_guard<std::mutex> lock(m_relayMutex);
        cb = m_lobbyRosterCb;
      }
      if (cb) {
        cb(msgStr);
      }
    } else if (type == "lobby_closed" && udpSenderPeerId == 0) {
      std::string reason = j.value("reason", "Host closed the lobby");
      LobbyClosedCallback cb;
      {
        std::lock_guard<std::mutex> lock(m_relayMutex);
        cb = m_lobbyClosedCb;
      }
      if (cb) {
        cb(reason);
      }
    } else if (type == "host_ping_limit") {
      // Only the host (or the master server relay, sender 0) may ask a guest to leave.
      const bool fromHost = udpSenderPeerId == 0 || m_hostMachineId == 0 ||
                            udpSenderPeerId == m_hostMachineId;
      uint64_t target = 0;
      try {
        target = std::stoull(j.value("target", std::string("0")));
      } catch (...) {
      }
      if (fromHost && target != 0 &&
          (target == m_localMachineId || target == GetRelayMachineId())) {
        m_pingRefusedLimit.store(j.value("limit", 0));
        m_pingRefusedPing.store(j.value("ping", 0));
        m_pingRefusedAtMs.store(GetTickCount64());
        if (!m_pingRefused.exchange(true))
          NOBLE_INFO("NetworkTransport: the host's max ping setting asked this player to leave the lobby");
      }
    } else if (type == "host_remove") {
      // Host moderation: kick / ban. Only the host (or relay, sender 0).
      const bool fromHost = udpSenderPeerId == 0 || m_hostMachineId == 0 ||
                            udpSenderPeerId == m_hostMachineId;
      uint64_t target = 0;
      try {
        target = std::stoull(j.value("target", std::string("0")));
      } catch (...) {
      }
      if (fromHost && !m_isHost && target != 0 &&
          (target == m_localMachineId || target == GetRelayMachineId())) {
        std::lock_guard<std::mutex> lock(m_removalMutex);
        const bool first = !m_removalPending;
        m_removal.kind = j.value("kind", std::string("kick"));
        m_removal.reason = j.value("reason", std::string());
        m_removal.minutes = j.value("minutes", 0);
        m_removalAtMs = GetTickCount64();
        m_removalPending = true;
        if (first)
          NOBLE_INFO("NetworkTransport: the host removed this player ({})",
                     m_removal.kind);
      }
    } else if (type == "host_rules") {
      // Host moderation rules (anti-AFK limit, co-host list), every 5 s.
      const bool fromHost = udpSenderPeerId == 0 || m_hostMachineId == 0 ||
                            udpSenderPeerId == m_hostMachineId;
      if (fromHost && !m_isHost)
        HostModeration::OnHostRules(msgStr);
    } else if (type == "vote_request" || type == "vote_cast") {
      // In-match votes: players ask / vote, the host counts.
      if (m_isHost)
        HostVotes::OnMessage(type, msgStr, udpSenderPeerId, true);
    } else if (type == "vote_start" || type == "vote_state" ||
               type == "vote_end") {
      const bool fromHost = udpSenderPeerId == 0 || m_hostMachineId == 0 ||
                            udpSenderPeerId == m_hostMachineId;
      if (fromHost && !m_isHost)
        HostVotes::OnMessage(type, msgStr, udpSenderPeerId, false);
    } else if (type == "lobby_auth") {
      // A guest that just joined the lobby sends its join check (password
      // hash); the host decides whether it may stay.
      if (m_isHost)
        HostModeration::OnLobbyAuth(msgStr, udpSenderPeerId);
    } else if (type == "lobby_admit") {
      const bool fromHost = udpSenderPeerId == 0 || m_hostMachineId == 0 ||
                            udpSenderPeerId == m_hostMachineId;
      if (fromHost && !m_isHost)
        HostModeration::OnLobbyAdmit(msgStr);
    } else if (type == "cohost_action") {
      // A co-host asks the host to kick / mute; the host checks the sender.
      if (m_isHost)
        HostModeration::OnCoHostAction(msgStr, udpSenderPeerId);
    } else if (type == "lobby_disband") {
      // The host disbanded the lobby (post-game DISBAND LOBBY, RETURN TO LOBBY
      // ALONE). Sent straight to the guests, so LAN / direct games without a
      // master-server lobby hear it too, not only the relay's lobby_closed.
      const bool fromHost = udpSenderPeerId == 0 || m_hostMachineId == 0 ||
                            udpSenderPeerId == m_hostMachineId;
      if (fromHost && !m_isHost) {
        LobbyClosedCallback cb;
        {
          std::lock_guard<std::mutex> lock(m_relayMutex);
          cb = m_lobbyClosedCb;
        }
        NOBLE_INFO("NetworkTransport: the host disbanded the lobby");
        if (cb)
          cb("The host closed the lobby");
      }
    } else if (type == "host_pause") {
      // Only the host (or the master server relay, sender 0) may pause the match for everyone.
      const bool fromHost = udpSenderPeerId == 0 || m_hostMachineId == 0 ||
                            udpSenderPeerId == m_hostMachineId;
      if (fromHost && !m_isHost) {
        const int state = j.value("paused", false) ? 1 : 0;
        m_hostPauseAtMs.store(GetTickCount64());
        if (m_hostPauseState.exchange(state) != state)
          NOBLE_INFO("NetworkTransport: host {} the match for everyone",
                     state ? "paused" : "resumed");
      }
    } else if (type == "match_over") {
      // The relay thread must not touch UI/career state: the render thread is often already tearing the
      // match down when this arrives (the non-host match-end crash). Hand it to the UI frame instead.
      NOBLE_INFO("NetworkTransport: received match_over packet from host (deferred to the UI thread)");
      m_matchOver.store(true);
      {
        std::lock_guard<std::mutex> lock(m_relayMutex);
        m_matchOverJson = msgStr;
      }
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
        NOBLE_INFO(
            "NetworkTransport: host reports its simulation running at frame {}",
            frame);
      } else if (!running) {
        m_hostSimRunning.store(false);
      }
      m_hostSimFrame.store(frame);
    } else if (type == "appearance_extra") {
      // Noble-only appearance extras (flame colour) - see engine/AppearanceSync.hpp. Kept out of engine player data.
      const auto players = j.value("players", nlohmann::json::array());
      for (const auto &p : players) {
        if (!p.is_object())
          continue;
        uint64_t xuid = 0;
        try {
          xuid = std::stoull(p.value("xuid", std::string()), nullptr, 16);
        } catch (...) {
          continue;
        }
        noble::AppearanceSync::SetRemote(xuid, p.value("flame", -1));
      }
    } else if (type == "spawn_request") {
      if (m_isHost) {
        uint32_t datum = j.value("datum", 0xFFFFFFFF);
        std::string tagPath = j.value("tagPath", "");
        NOBLE_INFO("NetworkTransport: [HOST] Received guest spawn_request for "
                   "datum 0x{:X} '{}'",
                   datum, tagPath);
        SharedTagCache::QueueSpawn(datum, tagPath);
      }
    } else if (type == "dev_spawn") {
      if (m_isHost) {
        std::string name = j.value("name", "");
        std::string cat = j.value("category", "");
        std::string tagPath = j.value("tagPath", "");
        uint32_t tagIndex = j.value("tagIndex", 0xFFFFFFFF);
        bool give = j.value("give", false);
        NOBLE_INFO(
            "NetworkTransport: [HOST] Received guest dev_spawn for '{}' ({})",
            name, cat);
        DevCheats::QueueItemAction(name, cat, tagPath, tagIndex, give);
      }
    }
  } catch (...) {
  }
}

void NetworkTransport::RelayWorkerThread(std::string relayUrl,
                                         std::string sessionId, bool isHost,
                                         uint64_t machineId) {
  (void)sessionId;
  (void)isHost;
  (void)machineId;

  // The relay stamps voice frames with this id, so the host (whose URL has
  // none) names itself too.
  if (relayUrl.find("peerId=") == std::string::npos &&
      m_relayMachineId.load() != 0) {
    char idText[32];
    std::snprintf(idText, sizeof(idText), "0x%llX",
                  static_cast<unsigned long long>(m_relayMachineId.load()));
    relayUrl +=
        (relayUrl.find('?') == std::string::npos ? "?peerId=" : "&peerId=") +
        std::string(idText);
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
    if (!WinHttpCrackUrl(wideUrl.c_str(), static_cast<DWORD>(wideUrl.size()), 0,
                         &parts)) {
      NOBLE_WARN("NetworkTransport: Relay URL parsing failed for '{}'",
                 relayUrl);
      m_relayActive.store(false);
      return;
    }

    std::wstring host(parts.lpszHostName, parts.dwHostNameLength);
    INTERNET_PORT port = parts.nPort ? parts.nPort
                                     : (isHttps ? INTERNET_DEFAULT_HTTPS_PORT
                                                : INTERNET_DEFAULT_HTTP_PORT);
    std::wstring path =
        parts.dwUrlPathLength > 0
            ? std::wstring(parts.lpszUrlPath, parts.dwUrlPathLength)
            : L"/ws/relay";
    if (parts.dwExtraInfoLength > 0) {
      path += std::wstring(parts.lpszExtraInfo, parts.dwExtraInfoLength);
    }

    HINTERNET hSession = WinHttpOpen(
        L"ProjectNoble/0.1", WINHTTP_ACCESS_TYPE_NO_PROXY, nullptr, nullptr, 0);
    if (!hSession) {
      if (m_relayStopRequested.load())
        break;
      for (int i = 0; i < 20 && !m_relayStopRequested.load(); ++i)
        Sleep(100);
      continue;
    }
    WinHttpSetTimeouts(hSession, 5000, 5000, 5000, 5000);

    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), port, 0);
    HINTERNET hRequest = nullptr;
    if (hConnect) {
      DWORD flags = isHttps ? WINHTTP_FLAG_SECURE : 0;
      hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(), nullptr,
                                    WINHTTP_NO_REFERER,
                                    WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    }

    HINTERNET hWebSocket = nullptr;
    if (hRequest) {
      if (WinHttpSetOption(hRequest, WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET,
                           nullptr, 0) &&
          WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                             nullptr, 0, 0, 0) &&
          WinHttpReceiveResponse(hRequest, nullptr)) {
        hWebSocket = WinHttpWebSocketCompleteUpgrade(hRequest, 0);
        m_relayWs.store(hWebSocket);
      }
      WinHttpCloseHandle(hRequest);
    }

    if (!hWebSocket) {
      if (hConnect)
        WinHttpCloseHandle(hConnect);
      WinHttpCloseHandle(hSession);
      if (m_relayStopRequested.load())
        break;
      for (int i = 0; i < 20 && !m_relayStopRequested.load(); ++i)
        Sleep(100);
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

    // WinHttpWebSocketReceive blocks until a frame arrives: the socket's
    // receive timeout does not apply to it. Sending has its own thread so that
    // nothing queued waits for the other side to speak first (found by voice,
    // 2026-10-31).
    std::atomic<bool> senderStop{false};
    std::thread sender([this, hWebSocket, &senderStop] {
      SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
      bool warned = false;
      const auto sendOne = [&](WINHTTP_WEB_SOCKET_BUFFER_TYPE type,
                               const void *data, size_t size) {
        const DWORD err =
            WinHttpWebSocketSend(hWebSocket, type, const_cast<void *>(data),
                                 static_cast<DWORD>(size));
        if (err != NO_ERROR && !warned && !senderStop.load() &&
            !m_relayStopRequested.load()) {
          warned = true;
          NOBLE_WARN("NetworkTransport: relay send failed ({})", err);
        }
      };
      while (!senderStop.load() && !m_relayStopRequested.load() &&
             m_relayActive.load()) {
        std::deque<std::string> text;
        std::deque<std::vector<uint8_t>> packets, voice;
        {
          std::unique_lock<std::mutex> lock(m_relayMutex);
          if (m_relayTextSendQueue.empty() && m_relaySendQueue.empty() &&
              m_relayVoiceSendQueue.empty()) {
            m_relaySendCv.wait_for(lock, std::chrono::milliseconds(50), [&] {
              return senderStop.load() || m_relayStopRequested.load() ||
                     !m_relayActive.load() || !m_relayTextSendQueue.empty() ||
                     !m_relaySendQueue.empty() ||
                     !m_relayVoiceSendQueue.empty();
            });
          }
          text.swap(m_relayTextSendQueue);
          packets.swap(m_relaySendQueue);
          voice.swap(m_relayVoiceSendQueue);
        }
        for (const auto &message : text) {
          if (!message.empty())
            sendOne(WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE, message.data(),
                    message.size());
        }
        for (const auto &packet : packets) {
          if (!packet.empty())
            sendOne(WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE,
                    packet.data(), packet.size());
        }
        for (const auto &frame : voice) {
          sendOne(WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE, frame.data(),
                  frame.size());
        }
      }
    });

    while (!m_relayStopRequested.load() && m_relayActive.load()) {
      // Drain incoming WebSocket frames with zero sleep delay
      // (WinHttpWebSocketReceive blocks in kernel until data arrives).
      for (int pollIter = 0; pollIter < 32 && !m_relayStopRequested.load() &&
                             m_relayActive.load();
           ++pollIter) {
        DWORD bytesRead = 0;
        WINHTTP_WEB_SOCKET_BUFFER_TYPE bufferType;
        DWORD err = WinHttpWebSocketReceive(
            hWebSocket, recvBuffer.data(),
            static_cast<DWORD>(recvBuffer.size()), &bytesRead, &bufferType);

        if (err == NO_ERROR && bytesRead > 0) {
          if (bufferType == WINHTTP_WEB_SOCKET_BINARY_FRAGMENT_BUFFER_TYPE ||
              bufferType == WINHTTP_WEB_SOCKET_UTF8_FRAGMENT_BUFFER_TYPE) {
            if (fragmentBuffer.size() + bytesRead <= 131072) {
              fragmentBuffer.insert(fragmentBuffer.end(), recvBuffer.data(),
                                    recvBuffer.data() + bytesRead);
            } else {
              fragmentBuffer.clear();
            }
            continue;
          }

          const uint8_t *msgData = recvBuffer.data();
          size_t msgLen = bytesRead;
          if (!fragmentBuffer.empty()) {
            if (fragmentBuffer.size() + bytesRead <= 131072) {
              fragmentBuffer.insert(fragmentBuffer.end(), recvBuffer.data(),
                                    recvBuffer.data() + bytesRead);
              msgData = fragmentBuffer.data();
              msgLen = fragmentBuffer.size();
            } else {
              fragmentBuffer.clear();
              continue;
            }
          }

          if (bufferType == WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE ||
              (msgLen >= 2 && msgData[0] == '{' &&
               msgData[msgLen - 1] == '}')) {
            std::string msgStr(reinterpret_cast<const char *>(msgData), msgLen);
            fragmentBuffer.clear();
            HandleControlJson(msgStr, 0);
          } else if (msgLen >= 4 && std::memcmp(msgData, "NVOX", 4) == 0) {
            if (const VoiceFrameCallback cb = m_voiceFrameCb.load())
              cb(msgData, msgLen);
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
          break; // normal idle; loop back into WinHttpWebSocketReceive without
                 // Sleep(1) stall
        } else if (err != NO_ERROR) {
          m_relayActive.store(false);
          break;
        }
      }

      if (!m_relayActive.load())
        break;
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
      WinHttpWebSocketClose(wsToClose, WINHTTP_WEB_SOCKET_SUCCESS_CLOSE_STATUS,
                            nullptr, 0);
      WinHttpCloseHandle(wsToClose);
    }
    if (hConnect)
      WinHttpCloseHandle(hConnect);
    if (sessionToClose)
      WinHttpCloseHandle(sessionToClose);

    if (m_relayStopRequested.load()) {
      m_relayActive.store(false);
      break;
    }

    NOBLE_WARN("NetworkTransport: WebSocket relay disconnected; reconnecting "
               "in 500ms...");
    for (int i = 0; i < 5 && !m_relayStopRequested.load(); ++i) {
      Sleep(100);
    }
  }
}

// Wire packet: [uint64_t senderMachineId][uint16_t port][raw Blam! payload]
//
// haloreach.dll has two network endpoints, port 1000 (channel traffic) and port
// 1001 (out-of-band: join, connect-request/establish, boot...). Slot 41 is
// send(target, data, size, port) with the DESTINATION port and slot 43 is
// recv(buffer, capacity, &sender, port) with the endpoint's own port
// (hr+0x43AB88 / hr+0x43AA78). The packet dispatcher (hr+0x4235E4) treats
// whatever the port-1001 endpoint reads as out-of-band and whatever the
// port-1000 endpoint reads as channel data. Before 2026-10-29 the port was
// dropped and both endpoints pulled from one queue, so channel packets were
// randomly handled as out-of-band (and the reverse): the host ignored most
// connect-establish replies and joins timed out. The port now travels with each
// packet and every endpoint only gets packets for its port.
// ---- Co-op packet redundancy (see NetworkTransport.hpp) ----
bool NetworkTransport::RedundancyWanted() {
  const uint64_t now = GetTickCount64();
  if (now - m_redundancyCheckTick.load(std::memory_order_relaxed) >= 1000ULL) {
    m_redundancyCheckTick.store(now, std::memory_order_relaxed);
    size_t peers = 0;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      peers = m_gamePeers.size();
    }
    if (m_isHost == false && peers == 0) peers = 1;   // a guest's game peer is the host
    const bool on = Settings::Get().Snapshot().packetRedundancy && peers >= 1 && peers <= 8;
    if (on != m_redundancyOn.exchange(on)) {
      NOBLE_INFO("NetworkTransport: packet redundancy {} ({} game peer(s))", on ? "on" : "off", peers);
    }
#ifdef NOBLE_DEV_BUILD
    static uint64_t s_lastReport = 0;
    if (now - s_lastReport >= 30000ULL) {
      s_lastReport = now;
      const uint64_t used = m_copiesUsed.exchange(0);
      if (used) NOBLE_INFO("NetworkTransport: last 30 s: {} lost game packets replaced by their redundant copy", used);
      const uint64_t pk = m_statGamePkts.exchange(0), by = m_statGameBytes.exchange(0);
      const uint64_t mx = m_statGameMax.exchange(0), big = m_statGameBig.exchange(0);
      if (pk)
        NOBLE_INFO("NetworkTransport: last 30 s: {} game packets sent, average {} bytes, largest {} bytes, {} over 1200 bytes",
                   pk, by / pk, mx, big);
    }
#endif
  }
  return m_redundancyOn.load(std::memory_order_relaxed);
}

int NetworkTransport::SendGamePacket(const sockaddr_in &dest, uint64_t destKey, const uint8_t *wire, size_t wireLen) {
  const int sent = sendto(m_socket, reinterpret_cast<const char *>(wire), static_cast<int>(wireLen), 0,
                          reinterpret_cast<const sockaddr *>(&dest), sizeof(dest));
#ifdef NOBLE_DEV_BUILD
  ++m_statGamePkts;
  m_statGameBytes += wireLen;
  if (wireLen > 1200) ++m_statGameBig;
  for (uint64_t prev = m_statGameMax.load(); wireLen > prev && !m_statGameMax.compare_exchange_weak(prev, wireLen);) {
  }
#endif
  if (wireLen <= kWireHeaderBytes || wireLen > 8192 || !RedundancyWanted()) return sent;
  const uint64_t now = GetTickCount64();
  std::vector<uint8_t> copy;
  {
    std::lock_guard<std::mutex> lock(m_redMutex);
    LastSent &last = m_lastSentTo[destKey];
    if (!last.wire.empty() && now - last.tick <= 100ULL) {
      // copy = [sender u64][kRedundantCopyPort u16][original port u16 + payload]
      copy.resize(last.wire.size() + sizeof(uint16_t));
      std::memcpy(copy.data(), last.wire.data(), sizeof(uint64_t));
      const uint16_t tag = kRedundantCopyPort;
      std::memcpy(copy.data() + sizeof(uint64_t), &tag, sizeof(uint16_t));
      std::memcpy(copy.data() + sizeof(uint64_t) + sizeof(uint16_t), last.wire.data() + sizeof(uint64_t),
                  last.wire.size() - sizeof(uint64_t));
    }
    last.wire.assign(wire, wire + wireLen);
    last.tick = now;
  }
  if (!copy.empty()) {
    sendto(m_socket, reinterpret_cast<const char *>(copy.data()), static_cast<int>(copy.size()), 0,
           reinterpret_cast<const sockaddr *>(&dest), sizeof(dest));
    m_bytesSent += copy.size();
  }
  return sent;
}

static uint64_t HashBytes(const uint8_t *data, size_t size) {
  uint64_t hash = 14695981039346656037ULL;
  for (size_t i = 0; i < size; ++i) {
    hash ^= static_cast<uint64_t>(data[i]);
    hash *= 1099511628211ULL;
  }
  return hash ? hash : 1;
}

void NetworkTransport::NotePrimaryReceived(const uint8_t *wire, size_t wireLen) {
  if (wireLen <= kWireHeaderBytes) return;
  uint64_t sender = 0;
  std::memcpy(&sender, wire, sizeof(uint64_t));
  const uint64_t h = HashBytes(wire + sizeof(uint64_t), wireLen - sizeof(uint64_t));
  std::lock_guard<std::mutex> lock(m_redMutex);
  auto &ring = m_recentPrimaries[sender];
  size_t &next = m_recentPrimaryNext[sender];
  ring[next] = {h, GetTickCount64()};
  next = (next + 1) % ring.size();
}

bool NetworkTransport::AcceptRedundantCopy(const uint8_t *wire, size_t wireLen, uint8_t *outWire, size_t &outLen) {
  if (wireLen <= kWireHeaderBytes + sizeof(uint16_t)) return false;
  uint64_t sender = 0;
  std::memcpy(&sender, wire, sizeof(uint64_t));
  const uint8_t *orig = wire + sizeof(uint64_t) + sizeof(uint16_t);   // original port + payload
  const size_t origLen = wireLen - sizeof(uint64_t) - sizeof(uint16_t);
  const uint64_t h = HashBytes(orig, origLen);
  const uint64_t now = GetTickCount64();
  {
    std::lock_guard<std::mutex> lock(m_redMutex);
    auto it = m_recentPrimaries.find(sender);
    if (it != m_recentPrimaries.end()) {
      for (const SeenHash &s : it->second) {
        if (s.hash == h && now - s.tick < 500ULL) return false;   // original arrived: drop the copy
      }
    }
  }
  std::memcpy(outWire, &sender, sizeof(uint64_t));
  std::memcpy(outWire + sizeof(uint64_t), orig, origLen);
  outLen = sizeof(uint64_t) + origLen;
  NotePrimaryReceived(outWire, outLen);   // a second copy of the same packet must not be delivered again
  ++m_copiesUsed;
  return true;
}
uint64_t NetworkTransport::OnNetSend(uint64_t targetPeerId, const uint8_t *data,
                                     size_t size, uint16_t port) {
  if (!m_active.load() || m_socket == INVALID_SOCKET || !data || size == 0) {
    return 0;
  }

  const size_t totalBytes = kWireHeaderBytes + size;
  if (size > 4088) {
    static std::atomic<int> s_bigSendLogs{0};
    if (s_bigSendLogs.fetch_add(1) < 20) {
      NOBLE_INFO(
          "NetworkTransport: [{}] large packet {} bytes -> 0x{:X} port {}",
          m_isHost ? "HOST" : "GUEST", size, targetPeerId, port);
    }
  }
  uint8_t stackBuf[2048];
  std::vector<uint8_t> heapBuf;
  uint8_t *wireData = stackBuf;

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
    NOBLE_INFO("NetworkTransport: [{}] TX #{} -> 0x{:X} port {} len={} | {}",
               m_isHost ? "HOST" : "GUEST", packetNum, targetPeerId, port, size,
               HexDump(data, size));
  }

  const uint64_t nowTick = GetTickCount64();

  if (!m_isHost) {
    bool directUdpAlive = false;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      auto &route = m_peerRouteStates[m_hostMachineId];
      route.lastUdpSendTick = nowTick;
      directUdpAlive = IsRouteBidirectionalAlive(route, nowTick);
    }
    // Guest sends directly to host
    int sent = SendGamePacket(m_remoteHostAddr, m_hostMachineId, wireData, totalBytes);


    // While direct UDP is not yet confirmed, also probe Host's LAN IP (if
    // known) and 127.0.0.1:<hostPort>.
    if (!directUdpAlive && m_remoteHostAddr.sin_addr.s_addr != 0) {
      if (m_remoteHostLanAddr.sin_addr.s_addr != 0 &&
          m_remoteHostLanAddr.sin_addr.s_addr !=
              m_remoteHostAddr.sin_addr.s_addr) {
        sendto(m_socket, reinterpret_cast<const char *>(wireData),
               static_cast<int>(totalBytes), 0,
               reinterpret_cast<const sockaddr *>(&m_remoteHostLanAddr),
               sizeof(m_remoteHostLanAddr));
      }
      if (m_remoteHostAddr.sin_addr.s_addr != htonl(INADDR_LOOPBACK)) {
        sockaddr_in localHostAddr = {};
        localHostAddr.sin_family = AF_INET;
        localHostAddr.sin_port = m_remoteHostAddr.sin_port;
        localHostAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        sendto(m_socket, reinterpret_cast<const char *>(wireData),
               static_cast<int>(totalBytes), 0,
               reinterpret_cast<const sockaddr *>(&localHostAddr),
               sizeof(localHostAddr));
      }
    }

    // Periodically send a 12-byte PING every 500ms while !directUdpAlive (NAT
    // hole punch) or every 1000ms while active so the Host echoes PONG and both
    // sides measure live RTT ping and confirm bidirectional UDP.
    const uint64_t pingInterval = directUdpAlive ? 1000ULL : 250ULL;
    if (m_remoteHostAddr.sin_addr.s_addr != 0 &&
        (nowTick - m_lastPunchTick >= pingInterval)) {
      m_lastPunchTick = nowTick;
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        NoteProbeSent(m_peerRouteStates[m_hostMachineId], nowTick);
      }
      uint8_t punch[sizeof(uint64_t) + 4] = {0};
      std::memcpy(punch, &m_localMachineId, sizeof(uint64_t));
      punch[sizeof(uint64_t)] = 'P';
      punch[sizeof(uint64_t) + 1] = 'I';
      punch[sizeof(uint64_t) + 2] = 'N';
      punch[sizeof(uint64_t) + 3] = 'G';
      sendto(m_socket, reinterpret_cast<const char *>(punch), sizeof(punch), 0,
             reinterpret_cast<const sockaddr *>(&m_remoteHostAddr),
             sizeof(m_remoteHostAddr));
      if (!directUdpAlive && m_remoteHostLanAddr.sin_addr.s_addr != 0 &&
          m_remoteHostLanAddr.sin_addr.s_addr !=
              m_remoteHostAddr.sin_addr.s_addr) {
        sendto(m_socket, reinterpret_cast<const char *>(punch), sizeof(punch),
               0, reinterpret_cast<const sockaddr *>(&m_remoteHostLanAddr),
               sizeof(m_remoteHostLanAddr));
      }
    }

    // Fallback to relay ONLY if direct UDP is not confirmed active (once UDP
    // PING/PONG completes, keep all ports 1000 & 1001 on direct UDP).
    if (m_relayActive.load() && !directUdpAlive) {
      {
        std::lock_guard<std::mutex> lock(m_relayMutex);
        if (m_relaySendQueue.size() < 2048) {
          m_relaySendQueue.emplace_back(wireData, wireData + totalBytes);
        }
      }
      m_relaySendCv.notify_one();
    }

    if (packetNum <= 15 || (packetNum % 500) == 0) {
      NOBLE_INFO("NetworkTransport: [GUEST] Sent packet #{} to Host 0x{:X} ({} "
                 "payload bytes, result: {}, direct: {}, relayFallback: {})",
                 packetNum, targetPeerId, size, sent, directUdpAlive,
                 !directUdpAlive && m_relayActive.load());
    }
    return static_cast<uint64_t>(size);
  }

  // Host sends traffic:
  // Periodically send a 12-byte PING every 1000ms to all known peers so Guests
  // echo PONG and the Host continuously measures accurate 1-RTT live ping and
  // confirms bidirectional UDP for every joined client.
  if (nowTick - m_lastPunchTick >= 1000ULL) {
    m_lastPunchTick = nowTick;
    std::vector<sockaddr_in> pingTargets;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      pingTargets.reserve(m_peerTable.size());
      for (const auto &kv : m_peerTable) {
        pingTargets.push_back(kv.second);
        NoteProbeSent(m_peerRouteStates[kv.first], nowTick);
      }
    }
    if (!pingTargets.empty()) {
      uint8_t pingPkt[sizeof(uint64_t) + 4] = {0};
      std::memcpy(pingPkt, &m_localMachineId, sizeof(uint64_t));
      pingPkt[sizeof(uint64_t)] = 'P';
      pingPkt[sizeof(uint64_t) + 1] = 'I';
      pingPkt[sizeof(uint64_t) + 2] = 'N';
      pingPkt[sizeof(uint64_t) + 3] = 'G';
      for (const auto &destAddr : pingTargets) {
        sendto(m_socket, reinterpret_cast<const char *>(pingPkt),
               sizeof(pingPkt), 0,
               reinterpret_cast<const sockaddr *>(&destAddr), sizeof(destAddr));
      }
    }
  }

  // If target is broadcast (0 or ~0ULL), broadcast to all known peers
  if (targetPeerId == 0 || targetPeerId == 0xFFFFFFFFFFFFFFFFULL) {
    std::vector<sockaddr_in> targets;
    std::vector<uint64_t> targetIds;
    std::vector<sockaddr_in> pingTargets;
    bool allDirectConfirmed = false;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      targets.reserve(m_peerTable.size());
      allDirectConfirmed = !m_peerTable.empty();
      for (const auto &kv : m_peerTable) {
        targets.push_back(kv.second);
        targetIds.push_back(kv.first);
        auto &route = m_peerRouteStates[kv.first];
        route.lastUdpSendTick = nowTick;
        const bool routeAlive = IsRouteBidirectionalAlive(route, nowTick);
        if (!routeAlive) {
          allDirectConfirmed = false;
        }
        const uint64_t pingGap = routeAlive ? 1000ULL : 400ULL;
        if (route.lastPingSentTick == 0 ||
            (nowTick - route.lastPingSentTick) >= pingGap) {
          NoteProbeSent(route, nowTick);
          pingTargets.push_back(kv.second);
        }
      }
      if (allDirectConfirmed) {
        for (uint64_t rp : m_relayPeers) {
          if (m_peerTable.find(rp) == m_peerTable.end()) {
            allDirectConfirmed = false;
            break;
          }
          const auto rit = m_peerRouteStates.find(rp);
          const bool routeAlive =
              rit != m_peerRouteStates.end() &&
              IsRouteBidirectionalAlive(rit->second, nowTick);
          if (!routeAlive) {
            allDirectConfirmed = false;
            break;
          }
        }
      }
    }
    for (size_t t = 0; t < targets.size(); ++t) {
      SendGamePacket(targets[t], targetIds[t], wireData, totalBytes);
    }
    if (!pingTargets.empty()) {
      uint8_t pingPkt[sizeof(uint64_t) + 4] = {0};
      std::memcpy(pingPkt, &m_localMachineId, sizeof(uint64_t));
      pingPkt[sizeof(uint64_t)] = 'P';
      pingPkt[sizeof(uint64_t) + 1] = 'I';
      pingPkt[sizeof(uint64_t) + 2] = 'N';
      pingPkt[sizeof(uint64_t) + 3] = 'G';
      for (const auto &pAddr : pingTargets) {
        sendto(m_socket, reinterpret_cast<const char *>(pingPkt),
               sizeof(pingPkt), 0, reinterpret_cast<const sockaddr *>(&pAddr),
               sizeof(pAddr));
      }
    }
    if (m_relayActive.load() && !allDirectConfirmed) {
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
  bool needHostPing = false;
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_peerTable.find(targetPeerId);
    if (it != m_peerTable.end()) {
      destAddr = it->second;
      foundInUdp = true;
      auto &route = m_peerRouteStates[targetPeerId];
      route.lastUdpSendTick = nowTick;
      directConfirmed = IsRouteBidirectionalAlive(route, nowTick);
      const uint64_t pingGap = directConfirmed ? 1000ULL : 400ULL;
      if (route.lastPingSentTick == 0 ||
          (nowTick - route.lastPingSentTick) >= pingGap) {
        NoteProbeSent(route, nowTick);
        needHostPing = true;
      }
    } else if (!m_relayActive.load()) {
      // Target route unknown yet and relay is inactive: hold packet (Star-routing)
      // instead of broadcasting (Finding #9, #10). When relay IS active, it
      // delivers the packet immediately below without stalling stale duplicates.
      auto &q = m_heldPackets[targetPeerId];
      if (q.size() < 32) {
        q.push_back(
            {std::vector<uint8_t>(wireData, wireData + totalBytes), nowTick});
      }
    }
  }

  if (foundInUdp) {
    int sent = SendGamePacket(destAddr, targetPeerId, wireData, totalBytes);

    if (needHostPing) {
      uint8_t pingPkt[sizeof(uint64_t) + 4] = {0};
      std::memcpy(pingPkt, &m_localMachineId, sizeof(uint64_t));
      pingPkt[sizeof(uint64_t)] = 'P';
      pingPkt[sizeof(uint64_t) + 1] = 'I';
      pingPkt[sizeof(uint64_t) + 2] = 'N';
      pingPkt[sizeof(uint64_t) + 3] = 'G';
      sendto(m_socket, reinterpret_cast<const char *>(pingPkt), sizeof(pingPkt),
             0, reinterpret_cast<const sockaddr *>(&destAddr), sizeof(destAddr));
    }
    if (packetNum <= 15 || (packetNum % 500) == 0) {
      char ipStr[INET_ADDRSTRLEN] = {0};
      inet_ntop(AF_INET, &destAddr.sin_addr, ipStr, sizeof(ipStr));
      NOBLE_INFO("NetworkTransport: [HOST] Sent packet #{} to Peer 0x{:X} "
                 "({}:{}) ({} payload bytes, result: {}, direct: {})",
                 packetNum, targetPeerId, ipStr, ntohs(destAddr.sin_port), size,
                 sent, directConfirmed);
    }
  }

  // If direct UDP is not confirmed active or peer UDP route is unknown,
  // fallback via relay
  if (m_relayActive.load() && (!foundInUdp || !directConfirmed)) {
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

// Splits one wire packet into the per-port queues. Returns false for packets
// that are not game traffic. Duplicate filtering only runs while the relay is
// active: its job is to drop the second copy of a packet that arrived over both
// UDP and the relay. The game itself resends identical packets on purpose
// (keepalives, connect-request retries), so filtering on direct UDP alone would
// eat those. The hash covers sender id and port too, so identical packets from
// two different guests are never confused.
bool NetworkTransport::QueueWirePacket(const uint8_t *wire, size_t wireLen,
                                       bool fromRelay) {
  if (wireLen <= kWireHeaderBytes) {
    return false;
  }
  uint64_t sender = 0;
  std::memcpy(&sender, wire, sizeof(uint64_t));
  if (sender == 0 || sender == m_localMachineId) {
    return false; // Never queue our own packets
  }
  // Only a relay copy can duplicate a packet already taken over UDP. haloreach.dll deliberately re-sends
  // byte-identical channel packets (keepalives, input/connect retries); dropping those on the direct path
  // costs a whole engine input tick.
  if (fromRelay && m_relayActive.load() &&
      IsDuplicatePacket(ComputePacketHash(wire, wireLen))) {
    return false;
  }
  QueuedPacket pkt;
  pkt.sender = sender;
  uint16_t port = 0;
  std::memcpy(&port, wire + sizeof(uint64_t), sizeof(uint16_t));
  if (port == 1000 || (fromRelay && m_isHost)) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (port == 1000) {
      // Port 1000 is the engine's channel endpoint. Seeing a peer here proves
      // that its haloreach.dll session has progressed past the out-of-band join
      // handshake.
      m_gamePeers.insert(sender);
    }
    if (fromRelay && m_isHost) {
      m_relayPeers.insert(sender);
    }
  }
  pkt.fromRelay = fromRelay;
  pkt.data.assign(wire + kWireHeaderBytes, wire + wireLen);
  auto &q = m_portQueues[port];
  if (q.size() >= 2048) {
    q.pop_front(); // never let a port nobody reads grow without bound
  }
  q.push_back(std::move(pkt));
  return true;
}

#ifdef NOBLE_DEV_BUILD
// ---- Dev-only network simulator (2026-11-19) ----
// NOBLE_NETSIM="delay=75,jitter=15,loss=2" adds that one-way delay (ms), +/- jitter (ms) and % packet loss to every
// direct UDP datagram this instance RECEIVES, so high-ping play can be tested on one PC. Set it on every instance
// (round trip = sum of both sides' delays). Off unless the variable is set; not compiled into release builds.
// Delivery is checked whenever the engine polls for packets, so timing is accurate to about one frame.
namespace netsim {
struct Held {
  ULONGLONG releaseTick;
  sockaddr_in from;
  std::vector<uint8_t> data;
};
struct Config {
  bool on = false;
  int delayMs = 0, jitterMs = 0;
  double loss = 0.0;
};
const Config &Get() {
  static const Config cfg = [] {
    Config c;
    char buf[128] = {};
    if (GetEnvironmentVariableA("NOBLE_NETSIM", buf, sizeof(buf)) == 0) return c;
    std::string s(buf);
    auto num = [&](const char *key) -> double {
      const auto p = s.find(key);
      return p == std::string::npos ? 0.0 : std::atof(s.c_str() + p + std::strlen(key));
    };
    c.delayMs = std::clamp(static_cast<int>(num("delay=")), 0, 2000);
    c.jitterMs = std::clamp(static_cast<int>(num("jitter=")), 0, 1000);
    c.loss = std::clamp(num("loss="), 0.0, 90.0) / 100.0;
    c.on = c.delayMs > 0 || c.jitterMs > 0 || c.loss > 0.0;
    if (c.on)
      NOBLE_WARN("NetworkTransport: network simulator ON: {} ms delay, +/-{} ms jitter, {:.1f}% loss on received packets",
                 c.delayMs, c.jitterMs, c.loss * 100.0);
    return c;
  }();
  return cfg;
}
std::deque<Held> &Queue() {
  static std::deque<Held> q;
  return q;
}
std::mt19937 &Rng() {
  static std::mt19937 rng(12345u);
  return rng;
}
} // namespace netsim
#endif

// recvfrom, through the dev network simulator when it is on (caller holds m_recvMutex).
static int TransportRecvFrom(SOCKET sock, uint8_t *buf, int len, sockaddr_in *from, int *fromLen) {
#ifdef NOBLE_DEV_BUILD
  const netsim::Config &cfg = netsim::Get();
  if (cfg.on) {
    auto &q = netsim::Queue();
    auto &rng = netsim::Rng();
    std::uniform_real_distribution<double> unit(0.0, 1.0);
    const ULONGLONG now = GetTickCount64();
    for (int i = 0; i < 512; ++i) {   // move everything waiting on the socket into the delay queue
      sockaddr_in a = {};
      int al = sizeof(a);
      const int n = recvfrom(sock, reinterpret_cast<char *>(buf), len, 0, reinterpret_cast<sockaddr *>(&a), &al);
      if (n == SOCKET_ERROR) {
        const int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK || q.size() > 20000) break;
        if (err == WSAECONNRESET || err == WSAECONNREFUSED) continue;
        return SOCKET_ERROR;   // real error: let the caller see it
      }
      if (unit(rng) < cfg.loss) continue;
      int j = 0;
      if (cfg.jitterMs > 0) j = std::uniform_int_distribution<int>(-cfg.jitterMs, cfg.jitterMs)(rng);
      const long long release = static_cast<long long>(now) + std::max(0, cfg.delayMs + j);
      q.push_back({static_cast<ULONGLONG>(release), a, std::vector<uint8_t>(buf, buf + n)});
    }
    for (auto it = q.begin(); it != q.end(); ++it) {
      if (it->releaseTick > now) continue;
      const int n = static_cast<int>(std::min<size_t>(it->data.size(), static_cast<size_t>(len)));
      std::memcpy(buf, it->data.data(), static_cast<size_t>(n));
      if (from) *from = it->from;
      if (fromLen) *fromLen = sizeof(sockaddr_in);
      q.erase(it);
      return n;
    }
    WSASetLastError(WSAEWOULDBLOCK);
    return SOCKET_ERROR;
  }
#endif
  return recvfrom(sock, reinterpret_cast<char *>(buf), len, 0, reinterpret_cast<sockaddr *>(from), fromLen);
}
uint64_t NetworkTransport::OnNetRecv(uint8_t *outBuffer, size_t bufferCapacity,
                                     uint64_t *outSenderPeerId, uint16_t port) {
  if (!m_active.load() || m_socket == INVALID_SOCKET || !outBuffer ||
      bufferCapacity == 0) {
    return 0;
  }

  std::lock_guard<std::mutex> recvLock(m_recvMutex);

  // 1. Drain the UDP socket into the per-port queues (NAT hole-punch PINGs are
  // handled here, not queued). haloreach.dll accepts channel packets up to
  // 0x3000 bytes (hr+0x43BC00); 64 KB = largest UDP datagram, so nothing is
  // truncated (a too-small buffer makes recvfrom fail with WSAEMSGSIZE and lose
  // the datagram).
  static thread_local uint8_t rawBuf[65536];
  for (int drained = 0; drained < 512; ++drained) {
    sockaddr_in senderAddr = {};
    int senderLen = sizeof(senderAddr);
    const int received = TransportRecvFrom(m_socket, rawBuf, static_cast<int>(sizeof(rawBuf)),
                                           &senderAddr, &senderLen);
    if (received == SOCKET_ERROR) {
      const int wsaErr = WSAGetLastError();
      if (wsaErr == WSAEMSGSIZE) {
        static std::atomic<int> s_oversizeLogs{0};
        if (s_oversizeLogs.fetch_add(1) < 20) {
          NOBLE_WARN("NetworkTransport: dropped a datagram larger than the "
                     "{}-byte receive buffer",
                     sizeof(rawBuf));
        }
        continue;
      }
      if (wsaErr == WSAECONNRESET || wsaErr == WSAECONNREFUSED) {
        continue; // Ignore ICMP Port Unreachable from NAT hole-punch /
                  // localhost probes
      }
      break; // WSAEWOULDBLOCK: nothing more waiting
    }
    if (received <= static_cast<int>(sizeof(uint64_t))) {
      continue;
    }

    uint64_t wireSender = 0;
    std::memcpy(&wireSender, rawBuf, sizeof(uint64_t));
    if (wireSender == 0 || wireSender == m_localMachineId) {
      continue; // Ignore self-loopback packets before updating
                // m_lastUdpRecvTick
    }

    const uint64_t now = GetTickCount64();
    m_lastUdpRecvTick.store(now);
    m_bytesRecv += received;
    char ipStr[INET_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, &senderAddr.sin_addr, ipStr, sizeof(ipStr));
    const std::string addrKey =
        std::string(ipStr) + ":" + std::to_string(ntohs(senderAddr.sin_port));

    // Disconnect datagram (DISC)
    if (received == static_cast<int>(sizeof(uint64_t) + 4) &&
        rawBuf[8] == 'D' && rawBuf[9] == 'I' && rawBuf[10] == 'S' &&
        rawBuf[11] == 'C') {
      if (wireSender != 0 && wireSender != m_localMachineId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_peerTable.erase(wireSender);
        m_gamePeers.erase(wireSender);
        m_relayPeers.erase(wireSender);
        m_peerRouteStates.erase(wireSender);
        m_heldPackets.erase(wireSender);
        for (auto it = m_addrToPeer.begin(); it != m_addrToPeer.end();) {
          if (it->second == wireSender)
            it = m_addrToPeer.erase(it);
          else
            ++it;
        }
        NOBLE_INFO("NetworkTransport: [{}] Remote peer 0x{:X} sent DISC; "
                   "removed from peer table immediately.",
                   m_isHost ? "HOST" : "GUEST", wireSender);
      }
      continue;
    }

    // NAT hole-punch PING / PONG packet
    if (received == static_cast<int>(sizeof(uint64_t) + 4) &&
        rawBuf[8] == 'P' && (rawBuf[9] == 'I' || rawBuf[9] == 'O') &&
        rawBuf[10] == 'N' && rawBuf[11] == 'G') {
      const bool isPong = (rawBuf[9] == 'O');
      if (wireSender != 0 && wireSender != m_localMachineId) {
        bool sendPingProbe = false;
        bool newPeerAddr = false;
        {
          std::lock_guard<std::mutex> lock(m_mutex);
          {
            const auto known = m_peerTable.find(wireSender);
            newPeerAddr = known == m_peerTable.end() ||
                          known->second.sin_addr.s_addr != senderAddr.sin_addr.s_addr ||
                          known->second.sin_port != senderAddr.sin_port;
          }
          m_peerTable[wireSender] = senderAddr;
          m_addrToPeer[addrKey] = wireSender;
          auto &route = m_peerRouteStates[wireSender];
          route.addr = senderAddr;
          route.lastUdpRecvTick = now;
          if (isPong) {
            // Only true PONG proves bidirectional UDP (inbound + outbound both open).
            route.directConfirmed = true;
            route.pongConfirmed = true;
            NoteProbeAnswered(route);
            route.lastPongRecvTick = now;
            if (route.lastPingSentTick != 0 && now >= route.lastPingSentTick) {
              const int sample = static_cast<int>(
                  std::min<uint64_t>(999, now - route.lastPingSentTick));
              route.rttMs =
                  (route.rttMs < 0) ? sample : ((route.rttMs * 3 + sample) / 4);
            }
          } else if (route.rttMs < 0 && route.lastUdpSendTick != 0 &&
                     now >= route.lastUdpSendTick &&
                     (now - route.lastUdpSendTick) < 1000) {
            const int sample = static_cast<int>(now - route.lastUdpSendTick);
            route.rttMs = sample;
          }
          if (!isPong && (!route.pongConfirmed || route.lastPingSentTick == 0 ||
                          (now - route.lastPingSentTick) >= 1000)) {
            NoteProbeSent(route, now);
            sendPingProbe = true;
          }
        }

        // Flush any packets that were held waiting for this peer's address
        FlushHeldPackets(wireSender, senderAddr);

        if (m_isHost && !isPong) {
          // Echo PONG back to Guest over direct UDP (burst 3) so Guest measures
          // its RTT and confirms bidirectional UDP
          uint8_t pong[sizeof(uint64_t) + 4] = {0};
          std::memcpy(pong, &m_localMachineId, sizeof(uint64_t));
          pong[sizeof(uint64_t)] = 'P';
          pong[sizeof(uint64_t) + 1] = 'O';
          pong[sizeof(uint64_t) + 2] = 'N';
          pong[sizeof(uint64_t) + 3] = 'G';
          for (int p = 0; p < 3; ++p) {
            sendto(m_socket, reinterpret_cast<const char *>(pong), sizeof(pong),
                   0, reinterpret_cast<const sockaddr *>(&senderAddr),
                   sizeof(senderAddr));
          }
          if (sendPingProbe) {
            // Also send a PING probe to Guest so Guest echoes PONG and Host
            // gets an immediate 1-RTT sample
            uint8_t pingPkt[sizeof(uint64_t) + 4] = {0};
            std::memcpy(pingPkt, &m_localMachineId, sizeof(uint64_t));
            pingPkt[sizeof(uint64_t)] = 'P';
            pingPkt[sizeof(uint64_t) + 1] = 'I';
            pingPkt[sizeof(uint64_t) + 2] = 'N';
            pingPkt[sizeof(uint64_t) + 3] = 'G';
            sendto(m_socket, reinterpret_cast<const char *>(pingPkt),
                   sizeof(pingPkt), 0,
                   reinterpret_cast<const sockaddr *>(&senderAddr),
                   sizeof(senderAddr));
          }
          if (newPeerAddr) {   // once per peer/address, not on every ping
            NOBLE_INFO("NetworkTransport: [HOST] Registered peer 0x{:X} via NAT "
                       "punch PING from {} (echoed PONG)",
                       wireSender, addrKey);
          }
        } else if (!m_isHost) {
          if (wireSender == m_hostMachineId) {
            m_remoteHostAddr = senderAddr;
          }
          // Echo PONG back so the Host also gets an exact 1-RTT sample and
          // bidirectional confirmation for this Guest
          if (!isPong) {
            uint8_t pong[sizeof(uint64_t) + 4] = {0};
            std::memcpy(pong, &m_localMachineId, sizeof(uint64_t));
            pong[sizeof(uint64_t)] = 'P';
            pong[sizeof(uint64_t) + 1] = 'O';
            pong[sizeof(uint64_t) + 2] = 'N';
            pong[sizeof(uint64_t) + 3] = 'G';
            for (int p = 0; p < 3; ++p) {
              sendto(m_socket, reinterpret_cast<const char *>(pong),
                     sizeof(pong), 0,
                     reinterpret_cast<const sockaddr *>(&senderAddr),
                     sizeof(senderAddr));
            }
            if (sendPingProbe) {
              uint8_t pingPkt[sizeof(uint64_t) + 4] = {0};
              std::memcpy(pingPkt, &m_localMachineId, sizeof(uint64_t));
              pingPkt[sizeof(uint64_t)] = 'P';
              pingPkt[sizeof(uint64_t) + 1] = 'I';
              pingPkt[sizeof(uint64_t) + 2] = 'N';
              pingPkt[sizeof(uint64_t) + 3] = 'G';
              sendto(m_socket, reinterpret_cast<const char *>(pingPkt),
                     sizeof(pingPkt), 0,
                     reinterpret_cast<const sockaddr *>(&senderAddr),
                     sizeof(senderAddr));
            }
          }
          if (newPeerAddr) {
            NOBLE_INFO(
                "NetworkTransport: [GUEST] Received NAT punch {} from Host ({})",
                isPong ? "PONG" : "PING", addrKey);
          }
        }
      }
      continue;
    }

    // Direct UDP voice frame ("NVOX")
    if (received >= static_cast<int>(sizeof(uint64_t) + 4 + 18) &&
        rawBuf[8] == 'N' && rawBuf[9] == 'V' && rawBuf[10] == 'O' &&
        rawBuf[11] == 'X') {
      if (m_isHost) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_peerTable[wireSender] = senderAddr;
        m_addrToPeer[addrKey] = wireSender;
        auto &route = m_peerRouteStates[wireSender];
        route.addr = senderAddr;
        route.lastUdpRecvTick = now;
      }
      // Host moderation: a player the host muted is not heard by anyone.
      if (m_isHost && HostModeration::IsHostMuted(wireSender))
        continue;
      if (std::memcmp(rawBuf + 12, "NVOX", 4) == 0) {
        if (const VoiceFrameCallback cb = m_voiceFrameCb.load()) {
          cb(rawBuf + 12, static_cast<size_t>(received - 12));
        }
        if (m_isHost) {
          SendControlUdp("NVOX", rawBuf + 12,
                         static_cast<size_t>(received - 12), 0, wireSender);
        }
      }
      continue;
    }

    // Direct UDP control/chat frame ("NBCJ")
    if (received > static_cast<int>(sizeof(uint64_t) + 4) && rawBuf[8] == 'N' &&
        rawBuf[9] == 'B' && rawBuf[10] == 'C' && rawBuf[11] == 'J') {
      if (m_isHost) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_peerTable[wireSender] = senderAddr;
        m_addrToPeer[addrKey] = wireSender;
        auto &route = m_peerRouteStates[wireSender];
        route.addr = senderAddr;
        route.lastUdpRecvTick = now;
      }
      std::string msgStr(reinterpret_cast<const char *>(rawBuf + 12),
                         static_cast<size_t>(received - 12));
      if (m_isHost && msgStr.find("\"chat\"") != std::string::npos) {
        bool dropChat = false;
        try {
          auto jc = nlohmann::json::parse(msgStr);
          if (jc.value("type", "") == "chat") {
            // Host moderation: a player the host muted is not heard by anyone,
            // and every relayed line goes through the host's word filter.
            if (HostModeration::IsHostMuted(wireSender)) {
              dropChat = true;
            } else {
              const std::string text = jc.value("text", std::string());
              const std::string filtered = HostModeration::FilterText(text);
              if (filtered != text) {
                jc["text"] = filtered;
                msgStr = jc.dump(-1, ' ', false,
                                 nlohmann::json::error_handler_t::replace);
              }
              const std::string rch = jc.value("channel", "all");
              if (rch != "global" && rch != "dm" && rch != "whisper") {
                SendControlUdp("NBCJ",
                               reinterpret_cast<const uint8_t *>(msgStr.data()),
                               msgStr.size(), 0, 0);
              }
            }
          }
        } catch (...) {
        }
        if (dropChat)
          continue;
      }
      HandleControlJson(msgStr, wireSender);
      continue;
    }

    {
      uint16_t wirePort = 0;
      std::memcpy(&wirePort, rawBuf + sizeof(uint64_t), sizeof(uint16_t));
      if (wirePort == kRedundantCopyPort) {
        static thread_local uint8_t copyBuf[65536];
        size_t copyLen = 0;
        if (!AcceptRedundantCopy(rawBuf, static_cast<size_t>(received), copyBuf, copyLen) ||
            !QueueWirePacket(copyBuf, copyLen, false)) {
          continue;
        }
      } else {
        NotePrimaryReceived(rawBuf, static_cast<size_t>(received));
        if (!QueueWirePacket(rawBuf, static_cast<size_t>(received), false)) {
          continue;
        }
      }
    }

    if (!m_isHost && wireSender == m_hostMachineId) {
      m_remoteHostAddr = senderAddr;
      std::lock_guard<std::mutex> lock(m_mutex);
      auto &route = m_peerRouteStates[wireSender];
      route.addr = senderAddr;
      route.lastUdpRecvTick = now;
      if (route.rttMs < 0 && route.lastUdpSendTick != 0 &&
          now >= route.lastUdpSendTick && (now - route.lastUdpSendTick) < 500) {
        route.rttMs = static_cast<int>(now - route.lastUdpSendTick);
      }
    }

    // Host: remember where each guest's packets come from so replies go back to
    // that address
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
          NOBLE_INFO("NetworkTransport: Registered remote peer 0x{:X} at {}",
                     wireSender, addrKey);
        }
        auto &route = m_peerRouteStates[wireSender];
        route.addr = senderAddr;
        route.lastUdpRecvTick = now;
        if (route.rttMs < 0 && route.lastUdpSendTick != 0 &&
            now >= route.lastUdpSendTick &&
            (now - route.lastUdpSendTick) < 500) {
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
    std::deque<std::vector<uint8_t>> relayBatch;
    {
      std::lock_guard<std::mutex> lock(m_relayMutex);
      if (!m_relayRecvQueue.empty()) {
        relayBatch.swap(m_relayRecvQueue);
      }
    }
    while (!relayBatch.empty()) {
      auto wire = std::move(relayBatch.front());
      relayBatch.pop_front();
      if (wire.size() == sizeof(uint64_t) + 4 && wire[8] == 'P' &&
          (wire[9] == 'I' || wire[9] == 'O') && wire[10] == 'N' &&
          wire[11] == 'G') {
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
      NOBLE_WARN("NetworkTransport: packet of {} bytes truncated to the game's "
                 "{}-byte buffer (port {})",
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
    NOBLE_INFO(
        "NetworkTransport: [{}] RX #{} <- 0x{:X} via {} port {} len={} | {}",
        m_isHost ? "HOST" : "GUEST", recvNum, pkt.sender,
        pkt.fromRelay ? "RELAY" : "UDP", port, payloadLen,
        HexDump(outBuffer, payloadLen));
  }
  if (recvNum <= 10 || (recvNum % 500) == 0) {
    NOBLE_INFO("NetworkTransport: [{}] Received packet #{} from 0x{:X} via {} "
               "port {} ({} payload bytes)",
               m_isHost ? "HOST" : "GUEST", recvNum, pkt.sender,
               pkt.fromRelay ? "RELAY" : "UDP", port, payloadLen);
  }
  return static_cast<uint64_t>(payloadLen);
}

} // namespace noble
