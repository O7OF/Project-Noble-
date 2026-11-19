#include "AppearanceSync.hpp"

#include "ElitePack.hpp"
#include "common/EngineModule.hpp"
#include "common/Logger.hpp"
#include "network/NetworkTransport.hpp"

#include <nlohmann/json.hpp>

#include <windows.h>

#include <atomic>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <climits>

namespace noble {
namespace AppearanceSync {
namespace {

// Sent on change, not on a timer: a player's own colour when it changes, the host's relay of a changed entry, and a
// full snapshot when someone new joins. A slow refresh covers a lost packet. This keeps traffic flat as lobbies grow.
constexpr uint64_t kRefreshIntervalMs = 15000;
constexpr uint64_t kMinGapMs = 250;   // bursts of changes are merged into one message
constexpr size_t kPlayerRecordSize = 0x490;   // engine players data array entry
constexpr int kMaxPlayers = 16;

std::mutex s_mutex;
std::unordered_map<uint64_t, int> s_remoteFlame;   // xuid -> flame variant (-1 = none)
std::unordered_set<uint64_t> s_pendingRelay;       // host only: entries changed since the last send
std::atomic<ULONGLONG> s_lastSendMs{0};
std::atomic<ULONGLONG> s_lastFullMs{0};
std::atomic<int> s_lastSentLocalFlame{INT_MIN};
std::atomic<size_t> s_lastPeerCount{0};
std::atomic<bool> s_wasActive{false};

// Where the engine keeps the player id inside a player record. Found once at runtime from the local player's own
// record (its id is known locally), so no fixed layout assumption is needed. -1 = not found yet.
std::atomic<int> s_xuidOffset{-1};
std::atomic<int> s_locateAttempts{0};

std::string ToHex(uint64_t v) {
    char buf[20] = {};
    std::snprintf(buf, sizeof(buf), "%016llX", static_cast<unsigned long long>(v));
    return buf;
}

// SEH-isolated: reads the engine players table from the engine thread's TLS block.
const uint8_t* PlayerEntries(int* outMax) {
    *outMax = 0;
    __try {
        HMODULE reach = ReachModule();
        if (!reach) return nullptr;
        const auto base = reinterpret_cast<uintptr_t>(reach);
        const uint32_t tlsIndex = *reinterpret_cast<const uint32_t*>(base + 0xC17B18);
        const auto tlsVec = static_cast<uintptr_t>(__readgsqword(0x58));
        const uintptr_t tls = *reinterpret_cast<const uintptr_t*>(tlsVec + static_cast<uintptr_t>(tlsIndex) * 8);
        if (!tls) return nullptr;
        const uint8_t* players = *reinterpret_cast<const uint8_t* const*>(tls + 0x18);
        if (!players) return nullptr;
        int maxP = *reinterpret_cast<const int32_t*>(players + 0x44);
        if (maxP <= 0) return nullptr;
        if (maxP > kMaxPlayers) maxP = kMaxPlayers;
        *outMax = maxP;
        return *reinterpret_cast<const uint8_t* const*>(players + 0x50);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return nullptr;
    }
}

// Scan the local machine's player record for the local id. Called from the engine thread (frame tick).
int LocateXuidOffset(uint64_t localXuid) {
    __try {
        int maxP = 0;
        const uint8_t* entries = PlayerEntries(&maxP);
        if (!entries) return -1;
        // Every record is searched for this machine's id (the "local machine" flag isn't reliable on every build of
        // the session); only this machine's own record holds its id.
        for (int i = 0; i < maxP; ++i) {
            const uint8_t* entry = entries + static_cast<size_t>(i) * kPlayerRecordSize;
            if (*reinterpret_cast<const uint16_t*>(entry) == 0) continue;
            for (size_t off = 0x08; off + 8 <= kPlayerRecordSize; off += 4) {
                uint64_t v = 0;
                std::memcpy(&v, entry + off, sizeof(v));
                if (v == localXuid) return static_cast<int>(off);
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
    return -1;
}

std::atomic<ULONGLONG> s_lastLocateMs{0};

void TryLocateXuidOffset() {
    if (s_xuidOffset.load() >= 0) return;
    // The local player record only exists once a game is running (lobby/loading can take minutes), so keep trying -
    // cheaply, twice a second - for the whole session instead of giving up.
    const ULONGLONG now = GetTickCount64();
    if (now - s_lastLocateMs.load() < 500) return;
    s_lastLocateMs.store(now);
    const int found = LocateXuidOffset(LocalXuid());
    if (found >= 0) {
        s_xuidOffset.store(found);
        NOBLE_INFO("AppearanceSync: located the player id in the engine player record; remote flame colours enabled");
        return;
    }
    if (s_locateAttempts.fetch_add(1) == 240) {   // ~2 minutes: say so once, keep trying
        NOBLE_WARN("AppearanceSync: player id not yet found in the engine player record; still looking");
    }
}

// full = local entry plus (host) every known entry; otherwise local entry plus (host) only the changed ones.
void SendEntries(bool full, int localFlame) {
    NetworkTransport& net = NetworkTransport::Get();
    nlohmann::json players = nlohmann::json::array();
    players.push_back({{"xuid", ToHex(LocalXuid())}, {"flame", localFlame}});
    if (net.IsHost()) {
        // The host relays guests' entries so guests also see each other.
        std::lock_guard<std::mutex> lock(s_mutex);
        if (full) {
            for (const auto& [xuid, flame] : s_remoteFlame) {
                players.push_back({{"xuid", ToHex(xuid)}, {"flame", flame}});
            }
        } else {
            for (const uint64_t xuid : s_pendingRelay) {
                const auto it = s_remoteFlame.find(xuid);
                if (it != s_remoteFlame.end()) players.push_back({{"xuid", ToHex(xuid)}, {"flame", it->second}});
            }
        }
        s_pendingRelay.clear();
    }
    const nlohmann::json j = {{"type", "appearance_extra"}, {"players", players}};
    net.SendLobbyControlJson(j.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace));
}
} // namespace

uint64_t LocalXuid() {
    uint64_t xuid = 0x0009000000000001ULL;
    NetworkTransport& net = NetworkTransport::Get();
    if (net.IsActive() && !net.IsHost()) {
        uint64_t guestLow = net.GetLocalMachineId() & 0x0000FFFFFFFFFFFFULL;
        if (guestLow <= 1ULL) guestLow = 2ULL;
        xuid = 0x0009000000000000ULL | guestLow;
    }
    return xuid;
}

void Tick() {
    NetworkTransport& net = NetworkTransport::Get();
    const bool active = net.IsActive();
    if (!active) {
        if (s_wasActive.exchange(false)) {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_remoteFlame.clear();
            s_pendingRelay.clear();
            s_lastSendMs.store(0);
            s_lastFullMs.store(0);
            s_lastSentLocalFlame.store(INT_MIN);
            s_lastPeerCount.store(0);
            s_xuidOffset.store(-1);
            s_locateAttempts.store(0);
        }
        return;
    }
    s_wasActive.store(true);
    TryLocateXuidOffset();
    const ULONGLONG now = GetTickCount64();
    const ULONGLONG lastSend = s_lastSendMs.load();
    if (lastSend != 0 && now - lastSend < kMinGapMs) return;

    const int localFlame = ElitePack::GetLocalFlameVariant();
    const size_t peers = net.GetGamePeerCount();
    const bool someoneJoined = peers > s_lastPeerCount.exchange(peers);
    const ULONGLONG lastFull = s_lastFullMs.load();
    const bool full = lastFull == 0 || someoneJoined || now - lastFull >= kRefreshIntervalMs;
    bool relayPending = false;
    if (net.IsHost()) {
        std::lock_guard<std::mutex> lock(s_mutex);
        relayPending = !s_pendingRelay.empty();
    }
    if (!full && localFlame == s_lastSentLocalFlame.load() && !relayPending) return;

    SendEntries(full, localFlame);
    s_lastSendMs.store(now);
    if (full) s_lastFullMs.store(now);
    s_lastSentLocalFlame.store(localFlame);
}

void SetRemote(uint64_t xuid, int flameVariant) {
    if (xuid == 0 || xuid == LocalXuid()) return;
    if (flameVariant < -1 || flameVariant >= 30) flameVariant = -1;
    bool changed = false;
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        auto it = s_remoteFlame.find(xuid);
        if (it == s_remoteFlame.end() || it->second != flameVariant) {
            s_remoteFlame[xuid] = flameVariant;
            if (NetworkTransport::Get().IsHost()) s_pendingRelay.insert(xuid);
            changed = true;
        }
    }
    if (changed) {
        NOBLE_INFO("AppearanceSync: a remote player's flame colour is now {}", flameVariant);
    }
}

int FlameForPlayerRecord(const uint8_t* playerRecord) {
    const int off = s_xuidOffset.load();
    if (!playerRecord || off < 0) return -1;
    uint64_t xuid = 0;
    std::memcpy(&xuid, playerRecord + off, sizeof(xuid));
    std::lock_guard<std::mutex> lock(s_mutex);
    const auto it = s_remoteFlame.find(xuid);
    return it == s_remoteFlame.end() ? -1 : it->second;
}

} // namespace AppearanceSync
} // namespace noble
