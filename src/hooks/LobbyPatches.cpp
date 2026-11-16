#include "LobbyPatches.hpp"
#include "HookManager.hpp"
#include "SigScanner.hpp"
#include "common/Logger.hpp"
#include "host/SlotPatch.hpp"
#ifndef NOBLE_DEDICATED
#include "network/NetworkTransport.hpp"
#include "career/Career.hpp"
#include "ui/PauseMenu.hpp"
#include "EnginePatches.hpp"
#include "engine/ReachHost.hpp"
#include "engine/BlamEngine.hpp"
#endif
#include <psapi.h>
#include <vector>
#include <string>
#include <cstdio>
#include <atomic>
#include <cstring>
#include <cwchar>
#include <intrin.h>

namespace noble {

static bool IsNobleHost() {
#ifdef NOBLE_DEDICATED
    return true;
#else
    const bool isGuest = (wcsstr(GetCommandLineW(), L"-connect") != nullptr);
    return !isGuest || NetworkTransport::Get().IsHost();
#endif
}

static bool IsTransportActive() {
#ifdef NOBLE_DEDICATED
    return false;
#else
    return NetworkTransport::Get().IsActive();
#endif
}

static bool ShouldBlockPrematureEnd() {
#ifdef NOBLE_DEDICATED
    return false;
#else
    return noble::EnginePatches::ShouldBlockPrematureGameOver();
#endif
}

static bool IsSessionActive() {
#ifdef NOBLE_DEDICATED
    return true;
#else
    return noble::ReachHost::IsSessionStarted();
#endif
}

bool LobbyPatches::s_isApplied = false;
uint32_t LobbyPatches::s_maxMultiplayerPlayers = 128; // Expanded 128 players for Custom Games / Forge
uint32_t LobbyPatches::s_maxCoopPlayers = 16;        // Default to expanded 16 players for campaign/firefight

// Detour for c_network_session_membership::set_slot_counts (?set_slot_counts@c_network_session_membership@@QAAXJJ_N@Z)
// Discovered from Omaha symbol database: reach_omaha_release_symbols.csv
typedef void(__fastcall* SetSlotCountsFn)(void* thisPtr, int32_t privateSlots, int32_t publicSlots, bool isRanked);
static SetSlotCountsFn s_originalSetSlotCounts = nullptr;

static void __fastcall Hook_SetSlotCounts(void* thisPtr, int32_t privateSlots, int32_t publicSlots, bool isRanked) {
    NOBLE_INFO("LobbyPatches: set_slot_counts private={} public={} ranked={}", privateSlots, publicSlots, isRanked);
    int32_t targetSlots = publicSlots;
    const int32_t expandedMp = static_cast<int32_t>(LobbyPatches::GetMaxMultiplayerPlayers());
    const int32_t expandedCoop = static_cast<int32_t>(LobbyPatches::GetMaxCoopPlayers());
    if ((publicSlots == 16 || publicSlots == 8) && expandedMp > publicSlots) {
        targetSlots = std::min(expandedMp, 128);
        NOBLE_INFO("LobbyPatches: Expanding multiplayer/forge session slot count from {} to {} (private={}, isRanked={})",
            publicSlots, targetSlots, privateSlots, isRanked);
    } else if (publicSlots <= 16 && expandedCoop > 1) {
        targetSlots = std::max(publicSlots, std::min(expandedCoop, 16));
        NOBLE_INFO("LobbyPatches: Expanding campaign/firefight session slot count from {} to {} (private={}, isRanked={})",
            publicSlots, targetSlots, privateSlots, isRanked);
    }

    if (s_originalSetSlotCounts) {
        s_originalSetSlotCounts(thisPtr, privateSlots, targetSlots, isRanked);
    }
}


// Detours for c_network_session::can_accept_join_request and can_accept_player_join_request
// By default, haloreach.dll rejects remote join-in-progress requests during Firefight (refusal 0x20)
// and match simulation (refusal 6). These detours override non-zero refusals to 0 (ACCEPT) so
// all remote players can join the match and synchronize into the same game.
typedef int32_t(__fastcall* CanAcceptJoinRequestFn)(void* thisPtr, void* joinRequestData);
static CanAcceptJoinRequestFn s_originalCanAcceptJoinRequest = nullptr;

static int32_t __fastcall Hook_CanAcceptJoinRequest(void* thisPtr, void* joinRequestData) {
    int32_t result = 0;
    if (s_originalCanAcceptJoinRequest) {
        result = s_originalCanAcceptJoinRequest(thisPtr, joinRequestData);
    }
    if (result != 0) {
        NOBLE_WARN("LobbyPatches: can_accept_join_request rejected with refusal code 0x{:X}; overriding to ACCEPT (0)", result);
        result = 0;
    }
    return result;
}

// The function reads stack arguments 5, 6, 7 and 9 (hr+0x3973BC; [rsp+0x70/0x78/0x80/0x90] after its
// prologue). Pass all nine through, or the original reads garbage for the stack ones.
typedef int32_t(__fastcall* CanAcceptPlayerJoinRequestFn)(void* thisPtr, void* playerData, int32_t playerIndex, int32_t flags,
                                                         uintptr_t a5, uintptr_t a6, uintptr_t a7, uintptr_t a8, uintptr_t a9);
static CanAcceptPlayerJoinRequestFn s_originalCanAcceptPlayerJoinRequest = nullptr;

static int32_t __fastcall Hook_CanAcceptPlayerJoinRequest(void* thisPtr, void* playerData, int32_t playerIndex, int32_t flags,
                                                          uintptr_t a5, uintptr_t a6, uintptr_t a7, uintptr_t a8, uintptr_t a9) {
    int32_t result = 0;
    if (s_originalCanAcceptPlayerJoinRequest) {
        result = s_originalCanAcceptPlayerJoinRequest(thisPtr, playerData, playerIndex, flags, a5, a6, a7, a8, a9);
    }
    if (result != 0) {
        NOBLE_WARN("LobbyPatches: can_accept_player_join_request rejected with refusal code 0x{:X}; overriding to ACCEPT (0)", result);
        result = 0;
    }
    return result;
}

// Detour for c_network_session_membership::evaluate_coop_insertion_point (RVA 0x39ce10)
// When players connect in co-op campaign, haloreach.dll compares their saved campaign
// checkpoints/insertion points across the membership structure. If any player's campaign
// progress does not match, the function returns -1, which causes the host to abort with
// refusal code 0x1B (error_players_incompatible_for_network_coop: "GAME ERROR: An error
// occurred while playing co-op." / "Error hosting co-op").
// Overriding a negative return to 0 (mission start insertion point) allows all players
// to play co-op campaign together without desync or rejection.
typedef int32_t(__fastcall* EvaluateCoopInsertionPointFn)(void* thisPtr);
static EvaluateCoopInsertionPointFn s_originalEvaluateCoopInsertionPoint = nullptr;

static int32_t __fastcall Hook_EvaluateCoopInsertionPoint(void* thisPtr) {
    int32_t result = 0;
    if (s_originalEvaluateCoopInsertionPoint) {
        result = s_originalEvaluateCoopInsertionPoint(thisPtr);
    }
    static std::atomic<int> insertLog{0};
    if (insertLog.fetch_add(1) < 20) {
        NOBLE_WARN("LobbyPatches: evaluate_coop_insertion_point -> {}", result);
    }
    if (result < 0) {
        NOBLE_WARN("LobbyPatches: evaluate_coop_insertion_point returned {} (incompatible); overriding to 0 (mission start) for co-op network play", result);
        result = 0;
    }
    return result;
}

// Join / connect refusal tracing. the reference host-probe hooks these two haloreach.dll
// functions (RVA 0x425368 "join refused", RVA 0x42459C "connect refused") to log why a host turns a
// guest away. Log-only: the original always runs. Arg 3 points at {u64 session, u32 reason} for joins
// and {u32 id, u32 reason} for connects.
typedef void(__fastcall* RefuseFn)(void* a1, void* a2, void* a3);

static RefuseFn s_originalJoinRefuse = nullptr;
static RefuseFn s_originalConnectRefuse = nullptr;
static std::atomic<int> s_refuseLogCount{0};
static uintptr_t s_reachBase = 0;

// Which window logged it (both share one log file) and where in haloreach.dll the call came from.
static const char* RefuseRole() {
    return std::wcsstr(GetCommandLineW(), L"-connect") != nullptr ? "GUEST" : "HOST";
}
static uintptr_t CallerRva(void* ret) {
    uintptr_t r = reinterpret_cast<uintptr_t>(ret);
    return (s_reachBase && r > s_reachBase) ? r - s_reachBase : r;
}
static uintptr_t s_reachSize = 0;

// The haloreach.dll part of the call stack as "hr+0x... <- hr+0x... <- ...", so one test shows which
// code decided to refuse (the immediate caller is only the generic message encoder).
static std::string ReachStack() {
    void* frames[32] = {};
    const USHORT n = RtlCaptureStackBackTrace(1, 32, frames, nullptr);
    std::string out;
    int kept = 0;
    for (USHORT i = 0; i < n && kept < 12; ++i) {
        const uintptr_t a = reinterpret_cast<uintptr_t>(frames[i]);
        if (!s_reachBase || a < s_reachBase || a >= s_reachBase + s_reachSize) continue;
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%shr+0x%llX", kept ? " <- " : "", static_cast<unsigned long long>(a - s_reachBase));
        out += buf;
        ++kept;
    }
    return out.empty() ? std::string("(no haloreach frames)") : out;
}

// Detour for c_network_session::abort (RVA 0x44EBA0)
// Centralized engine function called whenever a network session encounters a fatal peer
// error or incompatibility. Sets [session + 0x90 + 0x30] = reason and invokes virtual teardown
// method which terminates the game session and throws "GAME ERROR: An error occurred while
// playing co-op." / "Error hosting co-op" dialog.
// Suppressing co-op incompatibility reason codes (0x10 <= reason <= 0x25, e.g. 0x1B, 0x1C, 0x1A, 0x19, 0x1E, 0x1F)
// keeps the simulation running and prevents the host from booting peers or aborting the game.
typedef void(__fastcall* SessionAbortFn)(void* session, uint32_t reason);
static SessionAbortFn s_originalSessionAbort = nullptr;
static std::atomic<bool> s_campaignCoop{false};
void LobbyPatches::SetCampaignCoop(bool on) { s_campaignCoop.store(on); }

static void __fastcall Hook_SessionAbort(void* session, uint32_t reason) {
    uintptr_t caller = CallerRva(_ReturnAddress());
    NOBLE_WARN("LobbyPatches: [{} pid {}] c_network_session::abort called: session=0x{:p}, reason={} (0x{:X}), caller hr+0x{:X}",
        RefuseRole(), GetCurrentProcessId(), session, reason, reason, caller);
    const bool isHost = IsNobleHost();
    if (isHost || s_campaignCoop.load() || IsTransportActive() || (reason >= 0x05 && reason <= 0x28)) {
        NOBLE_WARN("LobbyPatches: SUPPRESSED session abort reason 0x{:X} from hr+0x{:X} to keep host / match running!", reason, caller);
        return;
    }
    if (s_originalSessionAbort) {
        s_originalSessionAbort(session, reason);
    }
}


typedef void(__fastcall* AcknowledgeJoinRequestFn)(void* thisPtr, void* targetAddr, uint32_t reason);
static AcknowledgeJoinRequestFn s_originalAcknowledgeJoinRequest = nullptr;

static void __fastcall Hook_AcknowledgeJoinRequest(void* thisPtr, void* targetAddr, uint32_t reason) {
    NOBLE_WARN("LobbyPatches: [{} pid {}] c_network_session::acknowledge_join_request: session={}, target={}, reason={} (0x{:X}) caller hr+0x{:X}",
        RefuseRole(), GetCurrentProcessId(), thisPtr, targetAddr, reason, reason, CallerRva(_ReturnAddress()));
    if (reason != 0) {
        NOBLE_WARN("LobbyPatches: acknowledge_join_request refusal 0x{:X} overridden to ACCEPT (0) for join-in-progress", reason);
        reason = 0;
    }
    if (s_originalAcknowledgeJoinRequest) {
        s_originalAcknowledgeJoinRequest(thisPtr, targetAddr, reason);
    }
}

static void __fastcall Hook_JoinRefuse(void* a1, void* a2, void* a3) {
    if (a3 && s_refuseLogCount.fetch_add(1) < 50) {
        uint64_t session = 0; uint32_t reason = 0;
        std::memcpy(&session, a3, 8);
        std::memcpy(&reason, static_cast<uint8_t*>(a3) + 8, 4);
        NOBLE_WARN("LobbyPatches: [{} pid {}] JOIN REFUSED: session 0x{:X} reason {} (0x{:X}) caller hr+0x{:X} a1={} a2={}",
            RefuseRole(), GetCurrentProcessId(), session, reason, reason, CallerRva(_ReturnAddress()), a1, a2);
        NOBLE_WARN("LobbyPatches: JOIN REFUSED stack: {}", ReachStack());
    }
    if (s_originalJoinRefuse) s_originalJoinRefuse(a1, a2, a3);
}

static void __fastcall Hook_ConnectRefuse(void* a1, void* a2, void* a3) {
    if (a3 && s_refuseLogCount.fetch_add(1) < 50) {
        uint32_t id = 0, reason = 0;
        std::memcpy(&id, a3, 4);
        std::memcpy(&reason, static_cast<uint8_t*>(a3) + 4, 4);
        NOBLE_WARN("LobbyPatches: [{} pid {}] CONNECT REFUSED: id 0x{:X} reason {} (0x{:X}) caller hr+0x{:X}",
            RefuseRole(), GetCurrentProcessId(), id, reason, reason, CallerRva(_ReturnAddress()));
        NOBLE_WARN("LobbyPatches: CONNECT REFUSED stack: {}", ReachStack());
    }
    if (s_originalConnectRefuse) s_originalConnectRefuse(a1, a2, a3);
}

// Session message handler trace. hr+0x429AA4 handles every out-of-band session message by type
// (8 join-request, 9 peer-connect, 10 join-abort, ...); arg 6 is the decoded message. Reason 15 turned out to
// be the host's answer to a join-ABORT ("abort ignored"), so the guest's join-request (type 8) is dropped
// before any refusal: it is checked against a protocol word (0xAC here), the session id, versions and an
// executable type (must be 5). Log-only: dumps types 8-10 so the dropped field shows up.
typedef uint64_t(__fastcall* SessionMessageFn)(void* a1, void* address, uint64_t a3, int32_t type, uint64_t a5, const uint8_t* data);
static SessionMessageFn s_originalSessionMessage = nullptr;
static std::atomic<int> s_sessionMsgLogCount{0};

// Join-queue state read after a join-request is handled. Queued joins are only moved into the session by
// hr+0x39BDCC (from the session update) when the queue mode (hr+0x2A49308) is 1, 2 or 4, that mode's session
// pointer (hr+0x2A1FE08 / FE10 / FE18, used when the byte at hr+0x2A1FD40 is set) exists, and that session's
// state (+0x60EF8) is exactly 6. Raw reads under SEH so a bad pointer can't crash the host.
struct JoinQueueState {
    int32_t mode = -99, count = -99, flag = -99;
    uint64_t sessions[3] = {};
    int32_t state[3] = { -99, -99, -99 };
    int32_t kind[3] = { -99, -99, -99 };
};

static void ReadJoinQueueState(JoinQueueState& q) {
    if (!s_reachBase) return;
    __try {
        q.mode = *reinterpret_cast<const int32_t*>(s_reachBase + 0x2A49308);
        q.count = *reinterpret_cast<const int32_t*>(s_reachBase + 0x2A4930C);
        q.flag = *reinterpret_cast<const uint8_t*>(s_reachBase + 0x2A1FD40);
        for (int i = 0; i < 3; ++i) {
            const uint64_t p = *reinterpret_cast<const uint64_t*>(s_reachBase + 0x2A1FE08 + 8 * i);
            q.sessions[i] = p;
            if (p) {
                q.state[i] = *reinterpret_cast<const int32_t*>(p + 0x60EF8);
                q.kind[i] = *reinterpret_cast<const int32_t*>(p + 0x24);
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

static uint64_t __fastcall Hook_SessionMessage(void* a1, void* address, uint64_t a3, int32_t type, uint64_t a5, const uint8_t* data) {
    if (type >= 8 && type <= 10 && data && s_sessionMsgLogCount.fetch_add(1) < 40) {
        char hex[64 * 3 + 1] = {};
        for (int i = 0; i < 64; ++i) {
            std::snprintf(hex + i * 3, 4, "%02X ", static_cast<unsigned>(data[i]));
        }
        static const char* kNames[] = { "join-request", "peer-connect", "join-abort" };
        NOBLE_WARN("LobbyPatches: [{} pid {}] session message {} ({}) a3=0x{:X} a5=0x{:X} data: {}",
            RefuseRole(), GetCurrentProcessId(), type, kNames[type - 8], a3, a5, hex);
    }
    const uint64_t ret = s_originalSessionMessage ? s_originalSessionMessage(a1, address, a3, type, a5, data) : 0;
    if (type == 8 && s_sessionMsgLogCount.load() <= 40) {
        JoinQueueState q;
        ReadJoinQueueState(q);
        NOBLE_WARN("LobbyPatches: join queue after request: mode {} count {} flag {} | session1 0x{:X} state {} kind {} | "
                   "session2 0x{:X} state {} kind {} | session3 0x{:X} state {} kind {}",
            q.mode, q.count, q.flag, q.sessions[0], q.state[0], q.kind[0], q.sessions[1], q.state[1], q.kind[1],
            q.sessions[2], q.state[2], q.kind[2]);
    }
    return ret;
}

// Every session/channel message the game encodes (hr+0x423D88) or decodes (hr+0x423E40), both windows,
// first 15 of each type. 12:46 test: the host DOES accept the guest (it is in the host's membership and
// later join-requests are not queued again) but the guest never learns it, so this shows what each side
// sends and receives after the join-request. Names from the dll's table (hr+0x42609C).
static const char* SessionMessageName(int32_t t) {
    switch (t) {
    case 8: return "join-request"; case 9: return "peer-connect"; case 10: return "join-abort";
    case 11: return "join-refuse"; case 12: return "leave-session"; case 13: return "leave-acknowledge";
    case 14: return "session-disband"; case 15: return "session-boot"; case 16: return "host-handoff";
    case 17: return "peer-handoff"; case 18: return "host-transition"; case 19: return "host-reestablish";
    case 20: return "host-decline"; case 21: return "peer-reestablish"; case 22: return "peer-establish";
    case 23: return "election"; case 24: return "election-refuse"; case 25: return "time-synchronize";
    case 36: return "security"; case 46: return "end-game"; default: return "?";
    }
}
static std::atomic<int> s_msgOutCount[64];
static std::atomic<int> s_msgInCount[64];
static std::atomic<size_t> s_remotePlayersJoinedCount{0};

bool LobbyPatches::HasRemotePlayerJoined() {
    return s_remotePlayersJoinedCount.load() > 0;
}

void LobbyPatches::ResetRemotePlayerJoined() {
    s_remotePlayersJoinedCount.store(0);
}

size_t LobbyPatches::GetRemotePlayerAddCount() {
    return s_remotePlayersJoinedCount.load();
}

typedef uint64_t(__fastcall* EncodeMessageFn)(void* table, void* stream, int32_t type, int32_t size, void* data);
static EncodeMessageFn s_originalEncodeMessage = nullptr;
static uint64_t __fastcall Hook_EncodeMessage(void* table, void* stream, int32_t type, int32_t size, void* data) {
    if (type >= 0 && type < 64 && s_msgOutCount[type].fetch_add(1) < 15) {
        NOBLE_INFO("LobbyPatches: [{} pid {}] SEND msg {} ({}) size {}", RefuseRole(), GetCurrentProcessId(), type,
                   SessionMessageName(type), size);
    }
    // Session teardown messages: who asked for it. A guest that leaves on its own (msg 12) or disbands (14) used to be
    // logged only as an effect; the caller RVA says which engine path decided it.
    if (type == 7 || type == 39 || type == 43) {
        static std::atomic<int> launchMsgLog[64];
        if (launchMsgLog[type & 63].fetch_add(1) < 5) {
            NOBLE_WARN("LobbyPatches: [{} pid {}] SEND msg{} caller hr+0x{:X} stack {}", RefuseRole(), GetCurrentProcessId(),
                       type, CallerRva(_ReturnAddress()), ReachStack());
        }
    }
    if (type == 37) {
        static std::atomic<int> msg37OutLog{0};
        if (msg37OutLog.fetch_add(1) < 40) {
            char hex[48 * 3 + 1] = {};
            const auto* p = static_cast<const uint8_t*>(data);
            for (int i = 0; i < 48 && p; ++i) std::snprintf(hex + i * 3, 4, "%02X ", static_cast<unsigned>(p[i]));
            NOBLE_WARN("LobbyPatches: [{} pid {}] SEND msg37 size {} caller hr+0x{:X} stack {} bytes: {}", RefuseRole(),
                       GetCurrentProcessId(), size, CallerRva(_ReturnAddress()), ReachStack(), hex);
        }
    }
    if (type == 12 || type == 14 || type == 15) {
        NOBLE_WARN("LobbyPatches: [{} pid {}] SEND msg {} ({}) from hr+0x{:X} - session teardown requested here",
                   RefuseRole(), GetCurrentProcessId(), type, SessionMessageName(type), CallerRva(_ReturnAddress()));
    }
    return s_originalEncodeMessage ? s_originalEncodeMessage(table, stream, type, size, data) : 0;
}

typedef uint64_t(__fastcall* DecodeMessageFn)(void* table, void* stream, int32_t* type, int32_t* size, void* buffer);
static DecodeMessageFn s_originalDecodeMessage = nullptr;
static uint64_t __fastcall Hook_DecodeMessage(void* table, void* stream, int32_t* type, int32_t* size, void* buffer) {
    const uint64_t ok = s_originalDecodeMessage ? s_originalDecodeMessage(table, stream, type, size, buffer) : 0;
    const int32_t t = type ? *type : -1;
    if (t == 27 && (ok & 0xFF) != 0) {
        s_remotePlayersJoinedCount.fetch_add(1);
    }
    if (t == 37) {
        static std::atomic<int> msg37InLog{0};
        if (msg37InLog.fetch_add(1) < 40) {
            char hex[48 * 3 + 1] = {};
            const auto* p = static_cast<const uint8_t*>(buffer);
            for (int i = 0; i < 48 && p; ++i) std::snprintf(hex + i * 3, 4, "%02X ", static_cast<unsigned>(p[i]));
            NOBLE_WARN("LobbyPatches: [{} pid {}] RECV msg37 size {} bytes: {}", RefuseRole(), GetCurrentProcessId(),
                       size ? *size : -1, hex);
        }
    }
    if (t >= 0 && t < 64 && s_msgInCount[t].fetch_add(1) < 15) {
        NOBLE_INFO("LobbyPatches: [{} pid {}] RECV msg {} ({}) size {} decoded {}", RefuseRole(), GetCurrentProcessId(), t,
                   SessionMessageName(t), size ? *size : -1, (ok & 0xFF) != 0);
    }
    return ok;
}

// Channel lookup by remote address (hr+0x4234E4). The packet dispatcher (hr+0x4235E4) sends every
// non-out-of-band packet here and silently drops it when no channel matches (address bytes compared over the
// length at +0x12). 12:50 test: the guest's connect-establish reaches the host's slot 43 but is never
// decoded, so this logs the looked-up address and whether a channel was found (first 30 per window).
typedef void*(__fastcall* FindChannelFn)(const uint8_t* address);
static FindChannelFn s_originalFindChannel = nullptr;
static std::atomic<int> s_findChannelLogCount{0};
static void* __fastcall Hook_FindChannel(const uint8_t* address) {
    void* ch = s_originalFindChannel ? s_originalFindChannel(address) : nullptr;
    // Log the first 10 lookups, then only when the found channel's state changes (up to 60 lines).
    static std::atomic<int32_t> s_lastState{-1000};
    int32_t curState = -99;
    if (ch) std::memcpy(&curState, static_cast<const uint8_t*>(ch) + 0x110C, sizeof(curState));
    const bool changed = s_lastState.exchange(curState) != curState;
    if (address && (s_findChannelLogCount.load() < 10 || changed) && s_findChannelLogCount.fetch_add(1) < 60) {
        char hex[0x18 * 3 + 1] = {};
        for (int i = 0; i < 0x18; ++i) {
            std::snprintf(hex + i * 3, 4, "%02X ", static_cast<unsigned>(address[i]));
        }
        int16_t len = 0;
        std::memcpy(&len, address + 0x12, sizeof(len));
        // The dispatcher only processes the packet when the channel state (+0x110C) is above 2.
        int32_t state = -99;
        if (ch) std::memcpy(&state, static_cast<const uint8_t*>(ch) + 0x110C, sizeof(state));
        NOBLE_INFO("LobbyPatches: [{} pid {}] find channel caller hr+0x{:X} -> {} state {} (len {}) address: {}", RefuseRole(),
                   GetCurrentProcessId(), CallerRva(_ReturnAddress()), ch, state, len, hex);
    }
    return ch;
}

// Peer boot (hr+0x395DE8, boot_peer(session, peer index, reason) -> sends session-boot 15). 13:00 test: the
// host accepts the guest, the channel connects, then the host boots it ~8.5 s later. Logs who boots and why.
typedef uint64_t(__fastcall* BootPeerFn)(void* session, int32_t peerIndex, int32_t reason);
static BootPeerFn s_originalBootPeer = nullptr;

static uint64_t __fastcall Hook_BootPeer(void* session, int32_t peerIndex, int32_t reason) {
    int32_t sessionState = -99;
    if (session) std::memcpy(&sessionState, static_cast<const uint8_t*>(session) + 0x60EF8, sizeof(sessionState));
    NOBLE_WARN("LobbyPatches: [{} pid {}] BOOT peer {} reason {} (session state {}) stack: {}", RefuseRole(),
               GetCurrentProcessId(), peerIndex, reason, sessionState, ReachStack());
    // Never allow the engine to boot the local host (peer 0)!
    if (peerIndex == 0) {
        NOBLE_WARN("LobbyPatches: SUPPRESSED engine boot of local host peer 0 (reason {})", reason);
        return 0;
    }

    static const bool keepPeers = [] {
        wchar_t env[8] = {};
        return !(GetEnvironmentVariableW(L"NOBLE_COOP_NO_KEEP_PEER", env, 8) > 0 && env[0] != L'0');
    }();

    const bool isHost = IsNobleHost();
    // Suppress automatic engine peer boots during active sessions/matches across all modes so
    // connecting/JIP and in-match peers are never dropped (returns 0 = false / not booted).
    if (keepPeers && (isHost || s_campaignCoop.load() || IsTransportActive())) {
        static std::atomic<int> logged{0};
        if (logged.fetch_add(1) < 20) {
            NOBLE_WARN("LobbyPatches: keeping peer {} in the session (engine boot reason {}); match stays alive", peerIndex, reason);
        }
        return 0;
    }
    return s_originalBootPeer ? s_originalBootPeer(session, peerIndex, reason) : 0;
}

// The game's own network diagnostics (hr+0x3AFFB4, printf-style into a 10-entry ring nobody reads):
// "dscrd:%s" / "dscrd cnst:%s" when a channel packet is discarded (hr+0x43BAE8), "exp state: %s->%s",
// "thpt: ...", "cgst", "net blckd: %d ms". Copied into our log (first 100). Args are passed through as
// raw registers, which matches how x64 passes variadic integers, pointers and doubles.
typedef void(__fastcall* NetLogFn)(const char* fmt, uintptr_t a1, uintptr_t a2, uintptr_t a3);
static NetLogFn s_originalNetLog = nullptr;
static std::atomic<int> s_netLogCount{0};
static void __fastcall Hook_NetLog(const char* fmt, uintptr_t a1, uintptr_t a2, uintptr_t a3) {
    const uintptr_t f = reinterpret_cast<uintptr_t>(fmt);
    if (fmt && s_reachBase && f > s_reachBase && f < s_reachBase + s_reachSize && s_netLogCount.fetch_add(1) < 100) {
        char line[256] = {};
        std::snprintf(line, sizeof(line), fmt, a1, a2, a3);
        NOBLE_WARN("LobbyPatches: [{} pid {}] game net log: {} (from hr+0x{:X})", RefuseRole(), GetCurrentProcessId(), line,
                   CallerRva(_ReturnAddress()));
    }
    if (s_originalNetLog) s_originalNetLog(fmt, a1, a2, a3);
}

// Game finish (hr+0x59330, sets game globals +0x1EAB8 "game finished"; the main loop then writes the results and
// unloads the match). Callers: 0x355EC, 0x7BACB, 0x18D123, 0x1B084C, 0x3A4092 (network session command switch).
// 2026-11-09: with a joiner in peer 17 / player 16 (NOBLE_SLOT_HIGH) the host's running match ends ~10 ms after the
// join request. Logs who finishes the game (first 20).
typedef void(__fastcall* GameFinishFn)();
static GameFinishFn s_originalGameFinish = nullptr;
static std::atomic<int> s_gameFinishLogCount{0};

struct GameFinishStubBlock {
    uint8_t code[256];
    void* originalFn;
};
static GameFinishStubBlock* s_gameFinishStub = nullptr;

// Return 1 to continue into s_originalGameFinish, 0 to suppress.
static uint8_t __fastcall Hook_GameFinishImpl(uintptr_t callerAddr) {
    if (s_gameFinishLogCount.fetch_add(1) < 20) {
        NOBLE_WARN("LobbyPatches: [{} pid {}] GAME FINISH requested by hr+0x{:X} stack: {}", RefuseRole(), GetCurrentProcessId(),
                   CallerRva(reinterpret_cast<void*>(callerAddr)), ReachStack());
    }
#ifndef NOBLE_DEDICATED
    if (noble::BlamEngine::IsSavedFilmPlayback()) {
        NOBLE_WARN("LobbyPatches: SUPPRESSED GAME FINISH during Theater saved film playback");
        return 0;
    }
#endif
    if (ShouldBlockPrematureEnd()) {
        NOBLE_WARN("LobbyPatches: SUPPRESSED premature GAME FINISH during cutscene / match startup!");
        return 0;
    }
#ifndef NOBLE_DEDICATED   // client only: career save + game-over screen
    Career::EnsureMatchSaved("game_finish");
    PauseMenu::ShowGameOver();
#endif
    if (s_gameFinishStub && s_gameFinishStub->originalFn) {
        s_originalGameFinish = reinterpret_cast<GameFinishFn>(s_gameFinishStub->originalFn);
        return 1;
    }
    return 0;
}

// LTCG callers (e.g. hr+0x355EC -> 0x355F1, hr+0x1B084C) keep volatile registers like R10 live
// across calls to 0x59330. Preserve all volatile GP and XMM registers across our C++ guard and
// tail-jump directly to MinHook's trampoline with original registers/stack intact.
static void EnsureGameFinishStub() {
    if (s_gameFinishStub) return;
    auto* blk = static_cast<GameFinishStubBlock*>(
        VirtualAlloc(nullptr, sizeof(GameFinishStubBlock), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    if (!blk) return;
    std::memset(blk, 0, sizeof(GameFinishStubBlock));

    size_t p = 0;
    auto emit = [&](std::initializer_list<uint8_t> bytes) {
        for (uint8_t b : bytes) blk->code[p++] = b;
    };
    auto emitU32 = [&](uint32_t v) {
        std::memcpy(blk->code + p, &v, 4);
        p += 4;
    };
    auto emitU64 = [&](uint64_t v) {
        std::memcpy(blk->code + p, &v, 8);
        p += 8;
    };

    // Save rflags + volatile GP regs (8 qwords -> rsp % 16 == 0)
    emit({ 0x9C });                         // pushfq
    emit({ 0x50, 0x51, 0x52 });             // push rax, rcx, rdx
    emit({ 0x41, 0x50, 0x41, 0x51 });       // push r8, r9
    emit({ 0x41, 0x52, 0x41, 0x53 });       // push r10, r11
    // Reserve 0x88 bytes: 0x20 shadow + 0x60 xmm0..xmm5 + 0x08 align pad
    emit({ 0x48, 0x81, 0xEC, 0x88, 0x00, 0x00, 0x00 });
    emit({ 0xF3, 0x0F, 0x7F, 0x44, 0x24, 0x20 }); // movdqu [rsp+0x20], xmm0
    emit({ 0xF3, 0x0F, 0x7F, 0x4C, 0x24, 0x30 }); // movdqu [rsp+0x30], xmm1
    emit({ 0xF3, 0x0F, 0x7F, 0x54, 0x24, 0x40 }); // movdqu [rsp+0x40], xmm2
    emit({ 0xF3, 0x0F, 0x7F, 0x5C, 0x24, 0x50 }); // movdqu [rsp+0x50], xmm3
    emit({ 0xF3, 0x0F, 0x7F, 0x64, 0x24, 0x60 }); // movdqu [rsp+0x60], xmm4
    emit({ 0xF3, 0x0F, 0x7F, 0x6C, 0x24, 0x70 }); // movdqu [rsp+0x70], xmm5
    // Pass original return address ([rsp + 0x40 + 0x88] = [rsp + 0xC8]) in RCX
    emit({ 0x48, 0x8B, 0x8C, 0x24, 0xC8, 0x00, 0x00, 0x00 });
    emit({ 0x48, 0xB8 });
    emitU64(reinterpret_cast<uint64_t>(&Hook_GameFinishImpl));
    emit({ 0xFF, 0xD0 });                   // call rax
    // Restore xmm0..xmm5 (flags unchanged by movdqu)
    emit({ 0xF3, 0x0F, 0x6F, 0x44, 0x24, 0x20 }); // movdqu xmm0, [rsp+0x20]
    emit({ 0xF3, 0x0F, 0x6F, 0x4C, 0x24, 0x30 }); // movdqu xmm1, [rsp+0x30]
    emit({ 0xF3, 0x0F, 0x6F, 0x54, 0x24, 0x40 }); // movdqu xmm2, [rsp+0x40]
    emit({ 0xF3, 0x0F, 0x6F, 0x5C, 0x24, 0x50 }); // movdqu xmm3, [rsp+0x50]
    emit({ 0xF3, 0x0F, 0x6F, 0x64, 0x24, 0x60 }); // movdqu xmm4, [rsp+0x60]
    emit({ 0xF3, 0x0F, 0x6F, 0x6C, 0x24, 0x70 }); // movdqu xmm5, [rsp+0x70]
    emit({ 0x84, 0xC0 });                   // test al, al
    emit({ 0x75 });                         // jnz do_orig
    size_t jnzRelPos = p++;

    // Suppress: restore stack & volatile registers and return directly
    emit({ 0x48, 0x81, 0xC4, 0x88, 0x00, 0x00, 0x00 });
    emit({ 0x41, 0x5B, 0x41, 0x5A });       // pop r11, r10
    emit({ 0x41, 0x59, 0x41, 0x58 });       // pop r9, r8
    emit({ 0x5A, 0x59, 0x58, 0x9D, 0xC3 }); // pop rdx, rcx, rax, popfq, ret

    size_t doOrigPos = p;
    blk->code[jnzRelPos] = static_cast<uint8_t>(static_cast<int8_t>(doOrigPos - (jnzRelPos + 1)));

    // Proceed: restore exact entry state and tail-jump into s_originalGameFinish
    emit({ 0x48, 0x81, 0xC4, 0x88, 0x00, 0x00, 0x00 });
    emit({ 0x41, 0x5B, 0x41, 0x5A });       // pop r11, r10
    emit({ 0x41, 0x59, 0x41, 0x58 });       // pop r9, r8
    emit({ 0x5A, 0x59, 0x58, 0x9D });       // pop rdx, rcx, rax, popfq
    emit({ 0xFF, 0x25 });                   // jmp qword ptr [rip + disp32]
    size_t dispPos = p;
    emitU32(0);
    int32_t disp = static_cast<int32_t>(offsetof(GameFinishStubBlock, originalFn) - (dispPos + 4));
    std::memcpy(blk->code + dispPos, &disp, 4);

    FlushInstructionCache(GetCurrentProcess(), blk, sizeof(GameFinishStubBlock));
    s_gameFinishStub = blk;
}

// Game over with reason (hr+0x7BA58, (reason) -> 0x59330). Called from the game engine's end-of-game check
// (0x7B4xx..0x7B7B4: r15 = 7, 8 score limit, 9, 0xA, ...). Logs the reason code (first 20).
typedef void(__fastcall* GameOverFn)(int32_t reason);
static GameOverFn s_originalGameOver = nullptr;
static std::atomic<int> s_gameOverLogCount{0};
static void __fastcall Hook_GameOver(int32_t reason) {
    if (s_gameOverLogCount.fetch_add(1) < 20) {
        NOBLE_WARN("LobbyPatches: [{} pid {}] GAME OVER reason {} (0x{:X}) from hr+0x{:X}", RefuseRole(), GetCurrentProcessId(),
                   reason, reason, CallerRva(_ReturnAddress()));
    }
#ifndef NOBLE_DEDICATED
    if (noble::BlamEngine::IsSavedFilmPlayback()) {
        NOBLE_WARN("LobbyPatches: SUPPRESSED GAME OVER (reason {}) during Theater saved film playback", reason);
        return;
    }
#endif
    if (ShouldBlockPrematureEnd()) {
        NOBLE_WARN("LobbyPatches: SUPPRESSED premature GAME OVER (reason {}) during cutscene / match startup!", reason);
        return;
    }
#ifndef NOBLE_DEDICATED   // client only: career save + game-over screen
    Career::EnsureMatchSaved("game_over");
    PauseMenu::ShowGameOver();
#endif
    if (s_originalGameOver) s_originalGameOver(reason);
}

// Abort game detour (hr+0x3ABB70, (uint32_t reason) -> void). Called on error 9 ("An error occurred during film playback"),
// session monitor reason 7 (timeout / match ended), etc.
typedef void(__fastcall* AbortGameFn)(uint32_t reason);
static AbortGameFn s_originalAbortGame = nullptr;

static void __fastcall Hook_AbortGame(uint32_t reason) {
    NOBLE_INFO("LobbyPatches: AbortGame(reason={}) from hr+0x{:X}", reason, CallerRva(_ReturnAddress()));
#ifndef NOBLE_DEDICATED
    if (noble::BlamEngine::IsSavedFilmPlayback()) {
        NOBLE_WARN("LobbyPatches: SUPPRESSED AbortGame(reason={}) during Theater saved film playback", reason);
        if (s_reachBase && s_reachSize > 0x278EE90) {
            *reinterpret_cast<uint8_t*>(s_reachBase + 0x278EE88) = 0; // saved_film_has_error
            *reinterpret_cast<uint32_t*>(s_reachBase + 0x278EE8C) = 0; // saved_film_error_code
        }
        return;
    }
#endif
    if (s_originalAbortGame) s_originalAbortGame(reason);
}

// Saved film session monitor detour (hr+0x3ABE70, () -> void).
// Evaluates channel timeout and [c_game_engine+0x4c] (game ended). If either triggers, it shuts down the saved film
// director and aborts with reason 7. During local saved film playback, suppress this monitor and clear any error flags.
typedef void(__fastcall* SavedFilmSessionMonitorFn)();
static SavedFilmSessionMonitorFn s_originalSavedFilmSessionMonitor = nullptr;

static void __fastcall Hook_SavedFilmSessionMonitor() {
#ifndef NOBLE_DEDICATED
    if (noble::BlamEngine::IsSavedFilmPlayback()) {
        if (s_reachBase && s_reachSize > 0x278EE90) {
            *reinterpret_cast<uint8_t*>(s_reachBase + 0x278EE88) = 0; // saved_film_has_error
            *reinterpret_cast<uint32_t*>(s_reachBase + 0x278EE8C) = 0; // saved_film_error_code
        }
        return;
    }
#endif
    if (s_originalSavedFilmSessionMonitor) s_originalSavedFilmSessionMonitor();
}

// Set game ended detour (hr+0x3C08B4, (void* thisPtr, bool force) -> void).
// Sets [c_game_engine+0x4c] = 1 based on stock match limits. Suppressed during saved film playback so the
// recorded match plays continuously to the end of the film.
typedef void(__fastcall* SetGameEndedFn)(void* thisPtr, bool force);
static SetGameEndedFn s_originalSetGameEnded = nullptr;

static void __fastcall Hook_SetGameEnded(void* thisPtr, bool force) {
#ifndef NOBLE_DEDICATED
    if (noble::BlamEngine::IsSavedFilmPlayback()) {
        NOBLE_WARN("LobbyPatches: SUPPRESSED SetGameEnded(force={}) during Theater saved film playback", force);
        return;
    }
#endif
    if (s_originalSetGameEnded) s_originalSetGameEnded(thisPtr, force);
}

// External-launch readiness check (hr+0x41EB80(param) -> bool). hr+0x3AA268 calls it for a campaign guest; when it
// returns 0 the engine does AbortGame(reason=6). Probes for the campaign join-in-progress work (item #3): log the
// inputs so the failing condition (no launch source, wrong mode byte, session state, ...) is visible.
static LONG CALLBACK WatchHandler(EXCEPTION_POINTERS* info);   // defined below (player0/manager watches)
static void DumpGamePlayers(char* out, size_t cap);            // defined below (team-count diagnostics)
static uintptr_t s_player0Flags;                               // defined below (game-player flags watch target)
static void ArmPlayer0Watch();                                 // defined below

// Arms the existing Dr0 flags watch on the first game-player flags word (campaign co-op activation tracing).
static void ArmCampaignPlayerFlagsWatch() {
    __try {
        const uint32_t tlsIndex = *reinterpret_cast<const uint32_t*>(s_reachBase + 0xC17B18);
        const auto tlsArray = static_cast<uintptr_t>(__readgsqword(0x58));
        const uintptr_t block = *reinterpret_cast<const uintptr_t*>(tlsArray + tlsIndex * 8);
        const uintptr_t arr = block ? *reinterpret_cast<const uintptr_t*>(block + 0x18) : 0;
        const uint64_t size = arr ? *reinterpret_cast<const uint64_t*>(arr + 0x20) : 0;
        const uintptr_t data = arr ? *reinterpret_cast<const uintptr_t*>(arr + 0x50) : 0;
        if (data && size >= 0x80 && size <= 0x4000) {
            s_player0Flags = data + 4;
            ArmPlayer0Watch();
        }
    } __except (1) {
    }
}

// Campaign-JIP local-player-manager watch: the guest's launch-ready abort comes from localPlayerManager+0x20 staying 1
// (see launch_ready). The working pre-mission flow advances it 2 -> 3. Dr2 write watch on that field logs the writer
// (caller RVA + stack) so the missing launch-data apply step can be identified.
static uintptr_t s_localManagerStateAddr = 0;
static uintptr_t s_localManagerIdentityAddr = 0;
static std::atomic<int> s_managerWatchHits{0};
static std::atomic<int> s_managerIdentityWatchHits{0};
static DWORD WINAPI ArmManagerWatchThread(void* param) {
    const DWORD tid = static_cast<DWORD>(reinterpret_cast<uintptr_t>(param));
    HANDLE th = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT, FALSE, tid);
    if (!th) return 0;
    bool ok = false;
    if (SuspendThread(th) != static_cast<DWORD>(-1)) {
        CONTEXT c = {};
        c.ContextFlags = CONTEXT_DEBUG_REGISTERS;
        if (GetThreadContext(th, &c)) {
            c.Dr2 = s_localManagerStateAddr;
            c.Dr7 = (c.Dr7 & ~0x0F000010ull) | (1ull << 4) | (1ull << 24) | (3ull << 26);   // L2, write, 4 bytes
            if (s_localManagerIdentityAddr) {
                c.Dr3 = s_localManagerIdentityAddr;
                c.Dr7 = (c.Dr7 & ~0xF0000000ull) | (1ull << 6) | (1ull << 28) | (2ull << 30);   // L3, write, 8 bytes
            }
            ok = SetThreadContext(th, &c) != FALSE;
        }
        ResumeThread(th);
    }
    CloseHandle(th);
    // Logged after resume: the game thread may hold the logger lock while suspended.
    NOBLE_WARN("LobbyPatches: local player manager state write watch {} at {:#x} (thread {})",
               ok ? "armed" : "FAILED", s_localManagerStateAddr, tid);
    return 0;
}
static void ArmManagerWatch() {
    static bool armed = false;
    if (armed || !s_localManagerStateAddr) return;
    armed = true;
    if (HANDLE t = CreateThread(nullptr, 0, &ArmManagerWatchThread,
                                reinterpret_cast<void*>(static_cast<uintptr_t>(GetCurrentThreadId())), 0, nullptr))
        CloseHandle(t);
}

typedef uint8_t(__fastcall* LaunchReadyFn)(uint8_t* param);
static LaunchReadyFn s_originalLaunchReady = nullptr;
static std::atomic<int> s_launchReadyLogCount{0};
static uint8_t __fastcall Hook_LaunchReady(uint8_t* param) {
    if (param && !s_localManagerStateAddr) {
        const uintptr_t src = *reinterpret_cast<uintptr_t*>(param + 8);
        if (src) {
            s_localManagerStateAddr = src + 0x20;
            s_localManagerIdentityAddr = src + 0x14;
            AddVectoredExceptionHandler(1, &WatchHandler);
            ArmManagerWatch();
        }
    }
    // Campaign co-op: watch the first game-player flags word to catch the 0x8 -> 0x1 "player active" activation.
    if (s_campaignCoop.load() && !s_player0Flags) ArmCampaignPlayerFlagsWatch();
    // Campaign-JIP probe: dump the game-player table periodically (log only; enabled with NOBLE_JIP_DUMP=1).
    static const bool dumpPlayers = [] {
        wchar_t env[8] = {};
        return GetEnvironmentVariableW(L"NOBLE_JIP_DUMP", env, 8) > 0 && env[0] != L'0';
    }();
    if (dumpPlayers && s_campaignCoop.load()) {
        static ULONGLONG lastDumpTick = 0;
        const ULONGLONG tick = GetTickCount64();
        if (tick - lastDumpTick > 5000) {
            lastDumpTick = tick;
            char dump[1024];
            DumpGamePlayers(dump, sizeof(dump));
            NOBLE_WARN("LobbyPatches: [{} pid {}] player table dump: {}", RefuseRole(), GetCurrentProcessId(), dump);
        }
    }
    const uint8_t result = s_originalLaunchReady ? s_originalLaunchReady(param) : 0;
    const int call = s_launchReadyLogCount.fetch_add(1);
    if (result == 0 || call < 3) {
        uintptr_t src = 0, sess = 0;
        int srcState = -99, sessState = -99, srcExtra20 = -99, srcExtra3C = -99;
        if (param) {
            src = *reinterpret_cast<uintptr_t*>(param + 8);
            sess = *reinterpret_cast<uintptr_t*>(param + 0x18);
            if (src) {
                srcState = *reinterpret_cast<int*>(src + 0x10);
                srcExtra20 = *reinterpret_cast<int*>(src + 0x20);
                srcExtra3C = *reinterpret_cast<int*>(src + 0x3C);
            }
            if (sess) sessState = *reinterpret_cast<int*>(sess + 0x60EF8);
        }
        int modeSet = -1, modeVal = -1;
        if (s_reachBase) {
            modeSet = *reinterpret_cast<uint8_t*>(s_reachBase + 0x2A1FD40);
            modeVal = *reinterpret_cast<uint8_t*>(s_reachBase + 0x2A1FD48);
        }
        NOBLE_WARN("LobbyPatches: launch_ready -> {} (src 0x{:X} srcState {} src+0x20 {} src+0x3C {} sess 0x{:X} sessState {} modeSet {} modeVal {})",
                   result, src, srcState, srcExtra20, srcExtra3C, sess, sessState, modeSet, modeVal);
    }
    return result;
}

// Player identity mapping check (hr+0x41FBE8): finds the local player's identity hash among the session peer records
// and returns 0 when none matches; hr+0x3AA268 then AbortGame(6). Campaign-JIP probe: log the local hash vs peer hashes.
// The per-peer hash is a murmur-style fold of the peer's first 8 identity bytes (hr+0x3A922C does the same fold).
typedef uint8_t(__fastcall* IdentityMatchFn)(uint8_t* a1, uint64_t a2, int a3, uint32_t a4, uint64_t a5, uint32_t mask,
                                             uint64_t peerRecords);
static IdentityMatchFn s_originalIdentityMatch = nullptr;
static std::atomic<int> s_identityMatchLogCount{0};
static uint32_t HashPeerIdentity8(const uint8_t* p) {
    uint64_t v = 0;
    std::memcpy(&v, p, sizeof(v));
    const uint32_t a = static_cast<uint32_t>(v), b = static_cast<uint32_t>(v >> 32);
    uint32_t h = ((b * 0x5BD1E995u) >> 0x18 ^ (b * 0x5BD1E995u));
    h = h * 0x5BD1E995u;
    h ^= a * 0x5BD1E995u;
    h = h * 0x5BD1E995u;
    const uint32_t u = ((h >> 0xD) ^ h) * 0x5BD1E995u;
    return (u >> 0xF) ^ u;
}
static uint8_t __fastcall Hook_IdentityMatch(uint8_t* a1, uint64_t a2, int a3, uint32_t a4, uint64_t a5, uint32_t mask,
                                             uint64_t peerRecords) {
    const uint8_t result = s_originalIdentityMatch ? s_originalIdentityMatch(a1, a2, a3, a4, a5, mask, peerRecords) : 0;
    const int call = s_identityMatchLogCount.fetch_add(1);
    if (result == 0 || call < 3) {
        uintptr_t src = a1 ? *reinterpret_cast<uintptr_t*>(a1 + 8) : 0;
        int localInt = -1, localSlot = -1;
        int localShort = -1;
        if (src) {
            localInt = *reinterpret_cast<int*>(src + 0x15);
            localSlot = *reinterpret_cast<int*>(src + 0x1C);
            localShort = *reinterpret_cast<int*>(src + 0x19);
        }
        char peers[640] = {};
        size_t n = 0;
        for (int i = 0; i < 17 && n + 48 < sizeof(peers); ++i) {
            if (!(mask & (1u << i))) continue;
            const auto* rec = reinterpret_cast<const uint8_t*>(peerRecords + static_cast<uint64_t>(i) * 0x158);
            n += std::snprintf(peers + n, sizeof(peers) - n, " p%d=%08X", i,
                               static_cast<unsigned>(HashPeerIdentity8(rec)));
        }
        NOBLE_WARN("LobbyPatches: identity_match -> {} (mode {} a3 {} a4 0x{:X} mask 0x{:X} local {}/{} slot {} |{})",
                   result, a1 ? *reinterpret_cast<int*>(a1 + 0x20) : -1, a3, a4, mask, localInt, localShort, localSlot,
                   peers);
    }
    return result;
}

// Player-create gate (hr+0x420298(param_1, playerIndex a2, peerRecord a3, sessionPlayerId a4) -> bool). The launch-ready
// path uses it to seat the joining player; if it returns false the player stays unseated (spectator). Campaign-JIP probe:
// log the player-slot states it checks (DAT_182D1B960[slot*0x634] vs 7, DAT_182D1B7CC[slot*0x634] vs 5) and the result.
typedef uint8_t(__fastcall* PlayerCreateFn)(uint64_t a1, uint32_t a2, uint64_t a3, int32_t a4);
static PlayerCreateFn s_originalPlayerCreate = nullptr;
static std::atomic<int> s_playerCreateLogCount{0};
static uint8_t __fastcall Hook_PlayerCreate(uint64_t a1, uint32_t a2, uint64_t a3, int32_t a4) {
    const uint8_t result = s_originalPlayerCreate ? s_originalPlayerCreate(a1, a2, a3, a4) : 0;
    if (s_playerCreateLogCount.fetch_add(1) < 20) {
        int b960 = -1, b7cc = -1, engineState = -1, engine52 = -1, av = -1;
        if (s_reachBase && a4 >= 0 && a4 < 16) {
            b960 = *reinterpret_cast<const uint8_t*>(s_reachBase + 0x2D1B960 + static_cast<size_t>(a4) * 0x634);
            b7cc = *reinterpret_cast<const uint8_t*>(s_reachBase + 0x2D1B7CC + static_cast<size_t>(a4) * 0x634);
        }
        if (a1) {
            const uintptr_t lvar3 = *reinterpret_cast<uintptr_t*>(a1 + 8);
            if (lvar3) {
                engineState = *reinterpret_cast<const int*>(lvar3 + 0x10);
                engine52 = *reinterpret_cast<const uint8_t*>(lvar3 + 0x52);
            }
        }
        NOBLE_WARN("LobbyPatches: player_create -> {} (slot {} a3 0x{:X} a4 {} b960 {} b7cc {} engineState {} engine52 {})",
                   result, a2, a3, a4, b960, b7cc, engineState, engine52);
    }
    return result;
}

// Controller/player apply state machine (hr+0x3CDF38) called from the msg-37 dispatch. It either applies a player
// (FUN_1803CDDC0 -> player created) or sets a rejection state; the pre-mission guest reaches the manager state 4 through
// it, the JIP guest does not. Campaign-JIP probe: log the inputs and the object's mapping fields.
typedef int64_t(__fastcall* ControllerApplyFn)(uint64_t a1, int32_t a2, int64_t a3, int32_t a4, int64_t a5, char a6,
                                              int32_t a7);
static ControllerApplyFn s_originalControllerApply = nullptr;
static std::atomic<int> s_controllerApplyLogCount{0};
static int64_t __fastcall Hook_ControllerApply(uint64_t a1, int32_t a2, int64_t a3, int32_t a4, int64_t a5, char a6,
                                              int32_t a7) {
    int f40 = -1, f44 = -1, f3c = -1, f50 = -1, f54 = -1;
    if (a1) {
        f40 = *reinterpret_cast<const int*>(a1 + 0x40);
        f44 = *reinterpret_cast<const int*>(a1 + 0x44);
        f3c = *reinterpret_cast<const int*>(a1 + 0x3C);
        f50 = *reinterpret_cast<const int*>(a1 + 0x50);
        f54 = *reinterpret_cast<const int*>(a1 + 0x54);
    }
    const int64_t result = s_originalControllerApply ? s_originalControllerApply(a1, a2, a3, a4, a5, a6, a7) : 0;
    if (s_controllerApplyLogCount.fetch_add(1) < 24) {
        int f50After = -1, f54After = -1;
        if (a1) {
            f50After = *reinterpret_cast<const int*>(a1 + 0x50);
            f54After = *reinterpret_cast<const int*>(a1 + 0x54);
        }
        NOBLE_WARN("LobbyPatches: controller_apply -> {} (p2 {} p3 {} p4 {} p6 {} p7 {} | f40 {} f44 {} f3c {} f50 {}->{} f54 {}->{})",
                   result, a2, static_cast<int>(a3), a4, a6, a7, f40, f44, f3c, f50, f50After, f54, f54After);
    }
    return result;
}

// Per-player update/activation step (hr+0x47A3C(playerIndex)). The working flow flips game-player flags 0x8 -> 0x9 ->
// 0x1 through this path within a second of game start; on the JIP guest the entry stays 0x8. Log the per-index calls and
// the flags value each time it runs (first 60 calls).
typedef void(__fastcall* PlayerUpdateStepFn)(uint32_t playerIndex);
static PlayerUpdateStepFn s_originalPlayerUpdateStep = nullptr;
static std::atomic<int> s_playerUpdateStepLog{0};
static uint32_t ReadGamePlayerFlags(uint32_t index) {
    __try {
        const uint32_t tlsIndex = *reinterpret_cast<const uint32_t*>(s_reachBase + 0xC17B18);
        const auto tlsArray = static_cast<uintptr_t>(__readgsqword(0x58));
        const uintptr_t block = *reinterpret_cast<const uintptr_t*>(tlsArray + tlsIndex * 8);
        const uintptr_t arr = block ? *reinterpret_cast<const uintptr_t*>(block + 0x18) : 0;
        const uint64_t size = arr ? *reinterpret_cast<const uint64_t*>(arr + 0x20) : 0;
        const uintptr_t data = arr ? *reinterpret_cast<const uintptr_t*>(arr + 0x50) : 0;
        if (!data || size < 0x80 || size > 0x4000) return 0xFFFFFFFFu;
        return *reinterpret_cast<const uint32_t*>(data + static_cast<uintptr_t>(index) * size + 4);
    } __except (1) {
        return 0xFFFFFFFFu;
    }
}
static void __fastcall Hook_PlayerUpdateStep(uint32_t playerIndex) {
    const int n = s_playerUpdateStepLog.fetch_add(1);
    if (n < 60) {
        NOBLE_WARN("LobbyPatches: [{} pid {}] player_update_step index {} flags before 0x{:X}", RefuseRole(),
                   GetCurrentProcessId(), playerIndex & 0xFFFF, ReadGamePlayerFlags(playerIndex & 0xFFFF));
    }
    if (s_originalPlayerUpdateStep) s_originalPlayerUpdateStep(playerIndex);
}

// Per-player update loop entry (hr+0x57434(param_1, float* time)). It early-returns when param_1 < 1; the player
// activation chain (FUN_180047A3C) hangs off this call. Campaign-JIP probe: log param_1 on host vs guest.
typedef void(__fastcall* PlayerUpdateLoopFn)(int32_t, float*);
static PlayerUpdateLoopFn s_originalPlayerUpdateLoop = nullptr;
static std::atomic<int> s_playerUpdateLoopLog{0};
static void __fastcall Hook_PlayerUpdateLoop(int32_t a1, float* a2) {
    const int n = s_playerUpdateLoopLog.fetch_add(1);
    if (n < 40) {
        NOBLE_WARN("LobbyPatches: [{} pid {}] player_update_loop a1 {} t {:.3f}", RefuseRole(), GetCurrentProcessId(), a1,
                   a2 ? a2[0] : 0.0f);
    }
    if (s_originalPlayerUpdateLoop) s_originalPlayerUpdateLoop(a1, a2);
}

// Simulation update gate (hr+0x3AAB08 -> bool). The main loop only runs the per-player update (FUN_180057434, which
// drives the game-player activation chain) when this returns 1. Campaign-JIP probe: log the gate inputs on host/guest -
// the condition needs c_game_engine+0x20 == 4 and a thread-local game context field.
static int ReadTlsCtx4() {
    __try {
        const auto tls = static_cast<uintptr_t>(__readgsqword(0x58));
        const uintptr_t ctx = *reinterpret_cast<const uintptr_t*>(tls + 0x48);
        return ctx ? *reinterpret_cast<const int*>(ctx + 4) : 0;
    } __except (1) {
        return 0;
    }
}
typedef uint8_t(__fastcall* SimGateFn)();
static SimGateFn s_originalSimGate = nullptr;
static std::atomic<int> s_simGateLog{0};
static uint8_t __fastcall Hook_SimGate() {
    const uint8_t result = s_originalSimGate ? s_originalSimGate() : 0;
    const int n = s_simGateLog.fetch_add(1);
    static int lastTuple[5] = { -1, -1, -1, -1, -1 };
    bool changed = false;
    int modeSetL = -1, f2L = -1, f3L = -1, engine10L = -1, engine20L = -1, ctx4L = -1;
    if (s_reachBase) {
        modeSetL = *reinterpret_cast<const uint8_t*>(s_reachBase + 0x4E0E8F0);
        f2L = *reinterpret_cast<const uint8_t*>(s_reachBase + 0x4E0E8F2);
        f3L = *reinterpret_cast<const uint8_t*>(s_reachBase + 0x4E0E8F3);
        const auto engine = *reinterpret_cast<const uintptr_t*>(s_reachBase + 0x4E0E908);
        if (engine) {
            engine10L = *reinterpret_cast<const int*>(engine + 0x10);
            engine20L = *reinterpret_cast<const int*>(engine + 0x20);
        }
    }
    ctx4L = ReadTlsCtx4();
    const int tuple[5] = { result, engine10L, engine20L, ctx4L, f3L };
    changed = std::memcmp(tuple, lastTuple, sizeof(tuple)) != 0;
    std::memcpy(lastTuple, tuple, sizeof(tuple));
    if (n < 8 || (changed && n < 400)) {
        NOBLE_WARN("LobbyPatches: [{} pid {}] sim_gate -> {} (modeSet {} f2 {} f3 {} engine+10 {} engine+20 {} ctx+4 {})",
                   RefuseRole(), GetCurrentProcessId(), result, modeSetL, f2L, f3L, engine10L, engine20L, ctx4L);
    }
    return result;
}

// Launch reset (hr+0x3BF63C(manager, mode)): clears the local-player records/slots when the engine re-initialises for a
// new launch. Probe logging only (event ordering for campaign join diagnostics).
typedef uint64_t(__fastcall* LaunchResetFn)(uint64_t* manager, char mode);
static LaunchResetFn s_originalLaunchReset = nullptr;
static std::atomic<int> s_launchResetLog{0};
static uint64_t __fastcall Hook_LaunchReset(uint64_t* manager, char mode) {
    const int n = s_launchResetLog.fetch_add(1);
    if (n < 20) {
        int state = -1, f40 = -1;
        if (manager) {
            state = *reinterpret_cast<const int*>(reinterpret_cast<const uint8_t*>(manager) + 0x20);
            f40 = *reinterpret_cast<const int*>(reinterpret_cast<const uint8_t*>(manager) + 0x40);
        }
        NOBLE_WARN("LobbyPatches: [{} pid {}] launch_reset called mode {} managerState {} f40 {}", RefuseRole(),
                   GetCurrentProcessId(), static_cast<int>(mode), state, f40);
    }
    return s_originalLaunchReset ? s_originalLaunchReset(manager, mode) : 0;
}

// Team count for the game-over check (hr+0x133320(out last player?, out teams alive, out teams with players)). Called
// every tick by 0x7A61C; logs the outputs whenever they change (first 40 changes) to see why the check ends the match.
// Dumps the game's player datum array (TLS block [hr+0xC17B18] + 0x18; size at +0x20, data at +0x50, as the iterator
// 0x1381C reads it): header dwords +0x20..+0x5F, then per slot: salt (i16 +0), flags (+4), team (i8 +0xAD).
static void DumpGamePlayers(char* out, size_t cap) {
    out[0] = 0;
    __try {
        const uint32_t tlsIndex = *reinterpret_cast<const uint32_t*>(s_reachBase + 0xC17B18);
        const auto tlsArray = static_cast<uintptr_t>(__readgsqword(0x58));
        const uintptr_t block = *reinterpret_cast<const uintptr_t*>(tlsArray + tlsIndex * 8);
        const uintptr_t arr = *reinterpret_cast<const uintptr_t*>(block + 0x18);
        if (!arr) { std::snprintf(out, cap, "no array"); return; }
        size_t n = 0;
        n += std::snprintf(out + n, cap - n, "hdr");
        for (int o = 0x20; o < 0x60 && n < cap; o += 4)
            n += std::snprintf(out + n, cap - n, " %08X", *reinterpret_cast<const uint32_t*>(arr + o));
        const uint64_t size = *reinterpret_cast<const uint64_t*>(arr + 0x20);
        const uintptr_t data = *reinterpret_cast<const uintptr_t*>(arr + 0x50);
        if (!data || size < 0x80 || size > 0x4000) return;
        s_player0Flags = data + 4;
        for (int i = 0; i < 20 && n + 40 < cap; ++i) {
            const uintptr_t d = data + i * size;
            n += std::snprintf(out + n, cap - n, " | %d:%04X f%08X t%d", i, *reinterpret_cast<const uint16_t*>(d),
                               *reinterpret_cast<const uint32_t*>(d + 4), *reinterpret_cast<const int8_t*>(d + 0xAD));
        }
    } __except (1) {
        std::snprintf(out, cap, "read fault");
    }
}

// Campaign co-op join-in-progress: the game engine puts mid-mission joiners on their own team (t1, t2, ...), which
// the campaign team/game-over check reads as several teams and ends the match. Force every present game player onto
// team 0 while campaign co-op is active (the team is the int8 at game-player datum +0xAD; verified by DumpGamePlayers).
// Runs on host and guest so both simulations agree; a real team change could be done through the engine's own
// team-change function once its MCC address is proven, but the datum is what the game-over check reads this tick.
static std::atomic<int> s_campaignTeamFixLogCount{0};
void ForceCampaignPlayerTeams() {
    if (!s_campaignCoop.load()) return;
    __try {
        const uint32_t tlsIndex = *reinterpret_cast<const uint32_t*>(s_reachBase + 0xC17B18);
        const auto tlsArray = static_cast<uintptr_t>(__readgsqword(0x58));
        const uintptr_t block = *reinterpret_cast<const uintptr_t*>(tlsArray + tlsIndex * 8);
        const uintptr_t arr = block ? *reinterpret_cast<const uintptr_t*>(block + 0x18) : 0;
        if (!arr) return;
        const uint64_t size = *reinterpret_cast<const uint64_t*>(arr + 0x20);
        const uintptr_t data = *reinterpret_cast<const uintptr_t*>(arr + 0x50);
        if (!data || size < 0x80 || size > 0x4000) return;
        for (int i = 0; i < 16; ++i) {
            auto* d = reinterpret_cast<uint8_t*>(data + static_cast<uintptr_t>(i) * size);
            const uint32_t flags = *reinterpret_cast<const uint32_t*>(d + 4);
            if (!flags) continue;
            const int8_t team = *reinterpret_cast<const int8_t*>(d + 0xAD);
            if (team == 0) continue;
            if (s_campaignTeamFixLogCount.fetch_add(1) < 20) {
                NOBLE_WARN("LobbyPatches: campaign JIP: forcing game player {} from team {} to 0 (flags 0x{:X})",
                           i, team, flags);
            }
            *reinterpret_cast<int8_t*>(d + 0xAD) = 0;
        }
    } __except (1) {
    }
}

// Hardware write watch on game player 0's flags (+4): at the high-slot join the host's own player goes from flags 0x8
// to 0x2 ("left"), the team count drops to 0 and the match ends. Dr0 on the game thread, logged by a vectored handler.
static std::atomic<int> s_watchHits{0};
static uintptr_t s_sessionP0Id = 0;   // session player 0 identifier (+0x1744), Dr1
static std::atomic<int> s_sessionWatchHits{0};
static LONG CALLBACK WatchHandler(EXCEPTION_POINTERS* info) {
    if (info->ExceptionRecord->ExceptionCode != STATUS_SINGLE_STEP) return EXCEPTION_CONTINUE_SEARCH;
    CONTEXT* c = info->ContextRecord;
    if (c->Dr6 & 2) {
        c->Dr6 = 0;
        static uint64_t lastId = 0;
        const uint64_t id = s_sessionP0Id ? *reinterpret_cast<const uint64_t*>(s_sessionP0Id) : 0;
        if (id != lastId && s_sessionWatchHits.fetch_add(1) < 12) {
            NOBLE_WARN("LobbyPatches: [{} pid {}] SESSION PLAYER0 ID WRITE -> {:016X} at hr+0x{:X} rcx {:#x} rdx {:#x} r8 {:#x} stack: {}",
                       RefuseRole(), GetCurrentProcessId(), id, CallerRva(reinterpret_cast<void*>(c->Rip)), c->Rcx, c->Rdx, c->R8, ReachStack());
        }
        lastId = id;
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    if (c->Dr6 & 8) {
        c->Dr6 = 0;
        if (s_localManagerIdentityAddr && s_managerIdentityWatchHits.fetch_add(1) < 16) {
            const uint64_t value = *reinterpret_cast<const uint64_t*>(s_localManagerIdentityAddr);
            NOBLE_WARN("LobbyPatches: [{} pid {}] LOCAL PLAYER IDENTITY WRITE -> 0x{:016X} at hr+0x{:X} stack: {}", RefuseRole(),
                       GetCurrentProcessId(), value, CallerRva(reinterpret_cast<void*>(c->Rip)), ReachStack());
        }
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    if (c->Dr6 & 4) {
        c->Dr6 = 0;
        if (s_localManagerStateAddr && s_managerWatchHits.fetch_add(1) < 16) {
            const int32_t value = *reinterpret_cast<const int32_t*>(s_localManagerStateAddr);
            NOBLE_WARN("LobbyPatches: [{} pid {}] LOCAL PLAYER MANAGER STATE WRITE -> {} at hr+0x{:X} stack: {}", RefuseRole(),
                       GetCurrentProcessId(), value, CallerRva(reinterpret_cast<void*>(c->Rip)), ReachStack());
        }
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    if (!(c->Dr6 & 1)) return EXCEPTION_CONTINUE_SEARCH;
    c->Dr6 = 0;
    static uint32_t lastValue = 0x8;
    const uint32_t value = s_player0Flags ? *reinterpret_cast<const uint32_t*>(s_player0Flags) : 0;
    const bool changed = value != lastValue;
    lastValue = value;
    if (changed && s_watchHits.fetch_add(1) < 12) {
        NOBLE_WARN("LobbyPatches: [{} pid {}] PLAYER0 FLAGS WRITE -> 0x{:X} at hr+0x{:X} stack: {}", RefuseRole(), GetCurrentProcessId(),
                   value, CallerRva(reinterpret_cast<void*>(c->Rip)), ReachStack());
    }
    return EXCEPTION_CONTINUE_EXECUTION;
}
static DWORD WINAPI ArmWatchThread(void* param) {
    const DWORD tid = static_cast<DWORD>(reinterpret_cast<uintptr_t>(param));
    HANDLE th = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT, FALSE, tid);
    if (!th) return 0;
    bool ok = false;
    if (SuspendThread(th) != static_cast<DWORD>(-1)) {
        CONTEXT c = {};
        c.ContextFlags = CONTEXT_DEBUG_REGISTERS;
        if (GetThreadContext(th, &c)) {
            c.Dr0 = s_player0Flags;
            c.Dr7 = (c.Dr7 & ~0xF0003ull) | 1ull | (1ull << 16) | (3ull << 18);   // L0, write, 4 bytes
            if (s_sessionP0Id) {
                c.Dr1 = s_sessionP0Id;
                c.Dr7 = (c.Dr7 & ~0xF0000Cull) | 4ull | (1ull << 20) | (2ull << 22);   // L1, write, 8 bytes
            }
            ok = SetThreadContext(th, &c) != FALSE;
        }
        ResumeThread(th);
    }
    CloseHandle(th);
    // Logged after the resume: the game thread may hold the logger lock while it is suspended.
    NOBLE_WARN("LobbyPatches: player 0 flags watch {} at {:#x} (thread {})", ok ? "armed" : "FAILED", s_player0Flags, tid);
    return 0;
}
static void ArmPlayer0Watch() {
    static bool armed = false;
    if (armed || !s_player0Flags) return;
    armed = true;
    AddVectoredExceptionHandler(1, &WatchHandler);
    if (HANDLE t = CreateThread(nullptr, 0, &ArmWatchThread, reinterpret_cast<void*>(static_cast<uintptr_t>(GetCurrentThreadId())), 0, nullptr))
        CloseHandle(t);
}

typedef void(__fastcall* TeamCountFn)(int32_t* a1, int32_t* alive, int32_t* teams);
static TeamCountFn s_originalTeamCount = nullptr;
static std::atomic<int> s_teamCountLogCount{0};
static void __fastcall Hook_TeamCount(int32_t* a1, int32_t* alive, int32_t* teams) {
    ForceCampaignPlayerTeams();   // keep campaign JIP players on team 0 before the game-over check reads team counts
    if (s_originalTeamCount) s_originalTeamCount(a1, alive, teams);

    // Keep at least 1 alive player and 1 team while the match is running to prevent false GameOver (0x7A61C)
    if (alive && *alive <= 0 && IsSessionActive()) {
        *alive = 1;
    }
    if (teams && *teams <= 0 && IsSessionActive()) {
        *teams = 1;
    }

    static int32_t lastA = -99, lastAlive = -99, lastTeams = -99;
    const int32_t a = a1 ? *a1 : -98, al = alive ? *alive : -98, tm = teams ? *teams : -98;
    if ((a != lastA || al != lastAlive || tm != lastTeams) && s_teamCountLogCount.fetch_add(1) < 40) {
        NOBLE_WARN("LobbyPatches: [{} pid {}] team count: a1 {} alive {} teams {} (caller hr+0x{:X})", RefuseRole(),
                   GetCurrentProcessId(), a, al, tm, CallerRva(_ReturnAddress()));
        char dump[1024];
        DumpGamePlayers(dump, sizeof(dump));
        NOBLE_WARN("LobbyPatches: game players: {}", dump);
    }
    lastA = a; lastAlive = al; lastTeams = tm;
    static int highMode = -1;   // NOBLE_SLOT_HIGH, read once
    if (highMode < 0) {
        wchar_t high[4] = {};
        highMode = (GetEnvironmentVariableW(L"NOBLE_SLOT_HIGH", high, 4) > 0 && high[0] == L'1') ? 1 : 0;
    }
    if (highMode == 1 && s_player0Flags && s_sessionP0Id) ArmPlayer0Watch();   // once both addresses are known
}

// Session -> game player sync (hr+0x41F510(singleton, int* count, ?, records 0xB8 each)): compares the 16-entry
// game player table in the 0x4E0E910 singleton (+0xA0 mask, entries 0xA0 from +0xA8) with the session players
// (session = singleton+0x18; players 0xB0 from +0x1740, mask +0x173C) and appends events (+0 index, +4 id, +0xC type:
// 0 left, 1 moved, 2 ?, 3 joined, 4 changed). 2026-11-09: at a high-slot join it emits "left" for the host's player 0.
// Logs player 0's fields on both sides whenever they change, and every event it emits (first 60 lines).
typedef void(__fastcall* PlayerSyncFn)(uint8_t* singleton, int32_t* count, void* a3, uint8_t* records);
static PlayerSyncFn s_originalPlayerSync = nullptr;
static std::atomic<int> s_playerSyncLogCount{0};
static void __fastcall Hook_PlayerSync(uint8_t* singleton, int32_t* count, void* a3, uint8_t* records) {
    const int32_t before = count ? *count : 0;
    char state[400] = {};
    if (singleton) {
        const uint8_t* s = *reinterpret_cast<uint8_t* const*>(singleton + 0x18);
        const uint8_t* e = singleton + 0xA8;
        if (s && !s_sessionP0Id) s_sessionP0Id = reinterpret_cast<uintptr_t>(s) + 0x1744;
        if (s) {
            std::snprintf(state, sizeof(state),
                "session pmask %08X peermask %08X p0 id %016llX peer %d +10 %d +1C %d | table mask %08X e0 id %016llX flag %u "
                "addr %02X%02X%02X%02X%02X%02X +1C %d",
                *reinterpret_cast<const uint32_t*>(s + 0x173C), *reinterpret_cast<const uint32_t*>(s + 0x5C),
                static_cast<unsigned long long>(*reinterpret_cast<const uint64_t*>(s + 0x1744)), *reinterpret_cast<const int32_t*>(s + 0x174C),
                *reinterpret_cast<const int32_t*>(s + 0x1750), *reinterpret_cast<const int32_t*>(s + 0x175C),
                *reinterpret_cast<const uint32_t*>(singleton + 0xA0), static_cast<unsigned long long>(*reinterpret_cast<const uint64_t*>(e)),
                e[8], e[0x10], e[0x11], e[0x12], e[0x13], e[0x14], e[0x15], *reinterpret_cast<const int32_t*>(e + 0x1C));
        }
    }
    static std::string last;
    if (last != state && s_playerSyncLogCount.fetch_add(1) < 60) {
        NOBLE_WARN("LobbyPatches: [{} pid {}] player sync in: {}", RefuseRole(), GetCurrentProcessId(), state);
        last = state;
    }
    if (s_originalPlayerSync) s_originalPlayerSync(singleton, count, a3, records);
    ForceCampaignPlayerTeams();   // campaign JIP: joiners must stay on the host's team 0
    const int32_t after = count ? *count : 0;
    for (int32_t k = before; records && k < after && k < before + 8; ++k) {
        const uint8_t* r = records + static_cast<size_t>(k) * 0xB8;
        if (s_playerSyncLogCount.fetch_add(1) < 60)
            NOBLE_WARN("LobbyPatches: [{} pid {}] player sync event: index {} id {:016X} type {} +1C {}", RefuseRole(), GetCurrentProcessId(),
                       *reinterpret_cast<const int32_t*>(r), *reinterpret_cast<const uint64_t*>(r + 4),
                       *reinterpret_cast<const int32_t*>(r + 0xC), *reinterpret_cast<const int32_t*>(r + 0x1C));
    }
}

// Membership consumer (hr+0x39E47C(block, payload); block = session + 0x30): applies a decoded membership update
// (message 26). 2026-11-09 high-slot test: the guest receives 26 + 34 but never answers with 35. Logs the payload
// counts and the session state before / after (peer mask +0x5C, player mask +0x173C, local peer +0x4464, +0x4468),
// first 20 calls.
typedef uint64_t(__fastcall* MemberConsumeFn)(uint8_t* block, uint8_t* payload);
static MemberConsumeFn s_originalMemberConsume = nullptr;
static std::atomic<int> s_memberConsumeLogCount{0};
static uint64_t __fastcall Hook_MemberConsume(uint8_t* block, uint8_t* payload) {
    const bool log = s_memberConsumeLogCount.fetch_add(1) < 20;
    auto state = [](const uint8_t* b) {
        const uint8_t* s = b - 0x30;
        char buf[160];
        std::snprintf(buf, sizeof(buf), "peermask %08X pmask %08X local peer %d +4468 %d +4434 %d",
                      *reinterpret_cast<const uint32_t*>(s + 0x5C), *reinterpret_cast<const uint32_t*>(s + 0x173C),
                      *reinterpret_cast<const int32_t*>(s + 0x4464), *reinterpret_cast<const int32_t*>(s + 0x4468),
                      *reinterpret_cast<const int32_t*>(s + 0x4434));
        return std::string(buf);
    };
    std::string before = (log && block) ? state(block) : std::string();
    const uint64_t r = s_originalMemberConsume ? s_originalMemberConsume(block, payload) : 0;
    if (log && block) {
        NOBLE_WARN("LobbyPatches: [{} pid {}] membership consume (payload {} peers {} players) ret {} | before: {} | after: {}",
                   RefuseRole(), GetCurrentProcessId(), payload ? *reinterpret_cast<const int16_t*>(payload + 0x14) : -1,
                   payload ? *reinterpret_cast<const int16_t*>(payload + 0x16) : -1, r, before, state(block));
    }
    return r;
}

// Guest join gate (hr+0x411F38(join state) -> bool: the local peer's players are all known to the session; on true
// 0x4114E5 goes on with 0x412014). Logs the result and the state flags (+0x58) whenever they change (first 30).
typedef bool(__fastcall* LocalPlayersFn)(uint8_t* state);
static LocalPlayersFn s_originalLocalPlayers = nullptr;
static std::atomic<int> s_localPlayersLogCount{0};
static bool __fastcall Hook_LocalPlayers(uint8_t* state) {
    const bool r = s_originalLocalPlayers ? s_originalLocalPlayers(state) : false;
    static int last = -1;
    static uint32_t lastFlags = 0;
    const uint32_t flags = state ? *reinterpret_cast<const uint32_t*>(state + 0x58) : 0;
    if ((static_cast<int>(r) != last || flags != lastFlags) && s_localPlayersLogCount.fetch_add(1) < 30) {
        uint32_t pmask = 0; int32_t local = -9;
        if (state) {
            const uint8_t* session = *reinterpret_cast<uint8_t* const*>(*reinterpret_cast<uint8_t* const*>(state + 0x30) + 0xD0);
            if (session) { pmask = *reinterpret_cast<const uint32_t*>(session + 0x173C); local = *reinterpret_cast<const int32_t*>(session + 0x4464); }
        }
        NOBLE_WARN("LobbyPatches: [{} pid {}] local players check -> {} (flags {:08X}, session pmask {:08X}, local peer {})", RefuseRole(),
                   GetCurrentProcessId(), r, flags, pmask, local);
    }
    last = r; lastFlags = flags;
    return r;
}

// Guest "joined" step (hr+0x397048(session)): when the session state (+0x60EF8) is a joining/joined one, the local
// peer (+0x4464) record's state must be 8 before the guest goes on (time sync, peer-establish, ...). 2026-11-09:
// with the local peer in slot 17 the guest never goes on. Logs session state, local peer and its record state
// (shadow record for peers >= 17) whenever they change (first 30).
typedef void(__fastcall* GuestStepFn)(uint8_t* session);
static GuestStepFn s_originalGuestStep = nullptr;
static std::atomic<int> s_guestStepLogCount{0};
static void __fastcall Hook_GuestStep(uint8_t* session) {
    if (session) {
        const int32_t state = *reinterpret_cast<const int32_t*>(session + 0x60EF8);
        const int32_t local = *reinterpret_cast<const int32_t*>(session + 0x4464);
        const int32_t f38 = *reinterpret_cast<const int32_t*>(session + 0x38);
        const int32_t f58 = *reinterpret_cast<const int32_t*>(session + 0x58);
        const uint8_t* rec = noble::slots::PeerRecord(session, local);
        const int32_t peerState = rec ? *reinterpret_cast<const int32_t*>(rec + 0x8) : -99;
        static int32_t last[5] = { -100, -100, -100, -100, -100 };
        const int32_t now[5] = { state, local, f38, f58, peerState };
        if (std::memcmp(last, now, sizeof(now)) != 0 && s_guestStepLogCount.fetch_add(1) < 30) {
            NOBLE_WARN("LobbyPatches: [{} pid {}] guest step: session state {} local peer {} +38 {} +58 {} local peer record state {}",
                       RefuseRole(), GetCurrentProcessId(), state, local, f38, f58, peerState);
        }
        std::memcpy(last, now, sizeof(now));
    }
    if (s_originalGuestStep) s_originalGuestStep(session);
}

// Session update (hr+0x393830(session), every frame; dispatches on the session state +0x60EF8). Logs state,
// local peer, local peer record state (+8; shadow record for >= 17), host peer (+0x40) and the peer / player
// masks whenever they change (first 40), to see where a slot-17 guest stops.
typedef void(__fastcall* SessionUpdateFn)(uint8_t* session);
static SessionUpdateFn s_originalSessionUpdate = nullptr;
static std::atomic<int> s_sessionUpdateLogCount{0};
static void __fastcall Hook_SessionUpdate(uint8_t* session) {
    if (s_originalSessionUpdate) s_originalSessionUpdate(session);
    if (!session) return;
    const int32_t state = *reinterpret_cast<const int32_t*>(session + 0x60EF8);
    if (state == 0) return;
    const int32_t local = *reinterpret_cast<const int32_t*>(session + 0x4464);
    const uint8_t* rec = noble::slots::PeerRecord(session, local);
    // Record state (+8) of every peer in the mask, folded into one value for change detection.
    const uint32_t peerMask = *reinterpret_cast<const uint32_t*>(session + 0x5C);
    char peers[160] = {};
    size_t pn = 0;
    int32_t peerStates = 0;
    for (int32_t i = 0; i < 32 && pn + 12 < sizeof(peers); ++i) {
        if (!(peerMask & (1u << i))) continue;
        const uint8_t* pr = noble::slots::PeerRecord(session, i);
        const int32_t ps = pr ? *reinterpret_cast<const int32_t*>(pr + 0x8) : -99;
        peerStates = peerStates * 31 + ps + i;
        pn += static_cast<size_t>(std::snprintf(peers + pn, sizeof(peers) - pn, " p%d:%d", i, ps));
    }
    const int32_t now[8] = { state, local, rec ? *reinterpret_cast<const int32_t*>(rec + 0x8) : -99,
                             *reinterpret_cast<const int32_t*>(session + 0x40), *reinterpret_cast<const int32_t*>(session + 0x5C),
                             *reinterpret_cast<const int32_t*>(session + 0x173C), *reinterpret_cast<const int32_t*>(session + 0x614CC),
                             peerStates };
    static uint8_t* seen[4] = {};
    static int32_t lastBy[4][8] = {};
    int slot = 0;
    while (slot < 4 && seen[slot] && seen[slot] != session) ++slot;
    if (slot == 4) return;
    const bool fresh = seen[slot] == nullptr;
    seen[slot] = session;
    int32_t* last = lastBy[slot];
    if ((fresh || std::memcmp(last, now, sizeof(now)) != 0) && s_sessionUpdateLogCount.fetch_add(1) < 80) {
        NOBLE_WARN("LobbyPatches: [{} pid {}] session {} state {} local peer {} (record state {}) +40 {} peermask {:08X} pmask {:08X} +614CC {} | peer states:{}",
                   RefuseRole(), GetCurrentProcessId(), static_cast<void*>(session), now[0], now[1], now[2], now[3],
                   static_cast<uint32_t>(now[4]), static_cast<uint32_t>(now[5]), now[6], peers);
    }
    std::memcpy(last, now, sizeof(now));
}

// Channel close (hr+0x43B98C, close(channel, reason); the only writer of state 2). 13:26 test: the host's
// channel to the guest goes 3 (connecting) -> 2 (closed) every ~2 s and never reaches 4. Logs why.
typedef uint64_t(__fastcall* CloseChannelFn)(void* channel, int32_t reason);
static CloseChannelFn s_originalCloseChannel = nullptr;
static std::atomic<int> s_closeLogCount{0};
static uint64_t __fastcall Hook_CloseChannel(void* channel, int32_t reason) {
    if (s_closeLogCount.fetch_add(1) < 30) {
        int32_t state = -99;
        if (channel) std::memcpy(&state, static_cast<const uint8_t*>(channel) + 0x110C, sizeof(state));
        NOBLE_WARN("LobbyPatches: [{} pid {}] CLOSE channel {} reason {} (state was {}) stack: {}", RefuseRole(),
                   GetCurrentProcessId(), channel, reason, state, ReachStack());
    }
    return s_originalCloseChannel ? s_originalCloseChannel(channel, reason) : 0;
}

// Channel packet receive (hr+0x43BAE8, (channel, bitstream {+0 data, +0x10 size}, ...)). 13:29 test: the
// guest's connect-establish reaches the host's connecting channel (state 3) but is not taken in until one try
// ~27 s later. Logs packets of 14+ bytes with the channel state before and after, to compare an ignored
// establish with the accepted one (first 40).
typedef uint64_t(__fastcall* ChannelReceiveFn)(void* channel, void* bitstream, uint64_t a3, uint64_t a4);
static ChannelReceiveFn s_originalChannelReceive = nullptr;
static std::atomic<int> s_chanRecvLogCount{0};
static uint64_t __fastcall Hook_ChannelReceive(void* channel, void* bitstream, uint64_t a3, uint64_t a4) {
    int32_t before = -99, size = 0;
    const uint8_t* data = nullptr;
    if (channel) std::memcpy(&before, static_cast<const uint8_t*>(channel) + 0x110C, sizeof(before));
    if (bitstream) {
        std::memcpy(&data, bitstream, sizeof(data));
        std::memcpy(&size, static_cast<const uint8_t*>(bitstream) + 0x10, sizeof(size));
    }
    const uint64_t ret = s_originalChannelReceive ? s_originalChannelReceive(channel, bitstream, a3, a4) : 0;
    if (data && size >= 14 && size < 4096 && s_chanRecvLogCount.fetch_add(1) < 40) {
        int32_t after = -99;
        if (channel) std::memcpy(&after, static_cast<const uint8_t*>(channel) + 0x110C, sizeof(after));
        char hex[32 * 3 + 1] = {};
        const int n = size < 32 ? size : 32;
        for (int i = 0; i < n; ++i) std::snprintf(hex + i * 3, 4, "%02X ", static_cast<unsigned>(data[i]));
        NOBLE_INFO("LobbyPatches: [{} pid {}] channel recv size {} state {}->{} ret {} a3=0x{:X} a4=0x{:X} | {}", RefuseRole(),
                   GetCurrentProcessId(), size, before, after, ret & 0xFF, a3, a4, hex);
    }
    return ret;
}

// Host-handoff state evaluation (hr+0x4035DC, c_network_session::process_host_handoff).
// Reach's Xbox Live P2P session leader evaluates peer NAT/UPnP/bandwidth before starting a game and attempts
// to migrate host authority (msg 16 host-handoff) to a guest if it scores higher. Project Noble's relay/UDP
// transport pins the Host and Guest roles, so host-handoff causes the guest to leave (msg 12 leave-session)
// and the host to disband (msg 14 session-disband -> "An error occurred while loading co-op").
// Clearing the candidate peer mask (+0x60F04) and target handoff peer (+0x60F18 = -1) with +0x60F0D = 1
// makes process_host_handoff immediately take its native "current host stays host" completion branch
// (hr+0x4038B3 -> hr+0x403B79 -> hr+0x4029A0 / hr+0x3940E4) without ever sending msg 16 (host-handoff).
typedef void(__fastcall* ProcessHostHandoffFn)(void* session);
static ProcessHostHandoffFn s_originalProcessHostHandoff = nullptr;
static std::atomic<int> s_hostHandoffLogCount{0};
static void __fastcall Hook_ProcessHostHandoff(void* session) {
#ifdef NOBLE_DEDICATED
    // noble-dedicated has no Noble P2P transport (NetworkTransport.cpp is not part of that target; linking it failed
    // with unresolved NetworkTransport ctor/dtor, 2026-11-13): the transport is never active there.
    constexpr bool transportActive = false;
#else
    const bool transportActive = NetworkTransport::Get().IsActive();
#endif
    if (session && transportActive) {
        auto* s = static_cast<uint8_t*>(session);
        const uint32_t prevCandidates = *reinterpret_cast<const uint32_t*>(s + 0x60F04);
        const int32_t prevTarget = *reinterpret_cast<const int32_t*>(s + 0x60F18);
        *reinterpret_cast<uint8_t*>(s + 0x60F0D) = 1;
        *reinterpret_cast<uint32_t*>(s + 0x60F04) = 0;
        *reinterpret_cast<int32_t*>(s + 0x60F18) = -1;
        if ((prevCandidates != 0 || prevTarget != -1) && s_hostHandoffLogCount.fetch_add(1) < 10) {
            NOBLE_INFO("LobbyPatches: [{} pid {}] keeping session host on current host (suppressed P2P host-handoff, "
                       "candidates=0x{:X}, targetPeer={})",
                       RefuseRole(), GetCurrentProcessId(), prevCandidates, prevTarget);
        }
    }
    if (s_originalProcessHostHandoff) {
        s_originalProcessHostHandoff(session);
    }
}

static bool WriteProtectedMemory(void* destination, const void* source, size_t size) {
    DWORD oldProtect = 0;
    if (!VirtualProtect(destination, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        return false;
    }
    memcpy(destination, source, size);
    VirtualProtect(destination, size, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), destination, size);
    return true;
}

bool LobbyPatches::Apply(HMODULE hReachDll) {
    if (!hReachDll) {
        NOBLE_ERROR("LobbyPatches: Invalid haloreach.dll handle provided.");
        return false;
    }

    if (s_isApplied) {
        return true;
    }

    NOBLE_INFO("LobbyPatches: Applying c_network_session_membership lobby expansion hooks...");

    MODULEINFO modInfo{};
    if (!GetModuleInformation(GetCurrentProcess(), hReachDll, &modInfo, sizeof(modInfo))) {
        NOBLE_ERROR("LobbyPatches: Failed to query haloreach.dll module bounds.");
        return false;
    }

    uintptr_t base = reinterpret_cast<uintptr_t>(modInfo.lpBaseOfDll);
    size_t size = modInfo.SizeOfImage;

    int patchCount = 0;

    // 1. MinHook Detour on c_network_session_membership::set_slot_counts
    // Pattern from ReachPatterns in NobleCoreMath: "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 8B DA 48 8B F9 85 D2"
    std::string_view sigSetSlotCounts = "48 89 5C 24 ?? 48 89 74 24 ?? 57 48 83 EC 20 8B DA 48 8B F9 85 D2";
    uintptr_t matchSetSlots = SigScanner::FindPattern(base, size, sigSetSlotCounts);
    if (matchSetSlots) {
        if (HookManager::InstallHook(reinterpret_cast<void*>(matchSetSlots),
                                     reinterpret_cast<void*>(&Hook_SetSlotCounts),
                                     reinterpret_cast<void**>(&s_originalSetSlotCounts),
                                     "c_network_session_membership::set_slot_counts")) {
            patchCount++;
            NOBLE_INFO("LobbyPatches: Installed MinHook detour on set_slot_counts at 0x{:X}", matchSetSlots);
        }
    }

    // 2. Specific network session player bounds check signature (with surrounding context)
    std::string_view sigSessionPeerCap = "48 8B ?? ?? 83 F8 10 7E ?? 48 8D ?? ?? ?? ?? ?? E8";
    uintptr_t matchPeer = SigScanner::FindPattern(base, size, sigSessionPeerCap);
    if (matchPeer) {
        uint8_t newLimit = static_cast<uint8_t>(s_maxMultiplayerPlayers);
        // Find offset of 0x10 within the pattern (offset + 6)
        if (WriteProtectedMemory(reinterpret_cast<void*>(matchPeer + 6), &newLimit, 1)) {
            patchCount++;
            NOBLE_INFO("LobbyPatches: Patched validated network session peer capacity to {} at 0x{:X}",
                s_maxMultiplayerPlayers, matchPeer);
        }
    }

    
    // 3. MinHook Detour on c_network_session::can_accept_player_join_request (RVA 0x3973BC)
    std::string_view sigPlayerJoinReq = "48 8B C4 48 89 58 08 48 89 68 10 48 89 70 18 44 89 48 20 57 41 54 41 55 41 56 41 57 48 83 EC 20";
    uintptr_t matchPlayerJoin = SigScanner::FindPattern(base, size, sigPlayerJoinReq);
    if (matchPlayerJoin) {
        if (HookManager::InstallHook(reinterpret_cast<void*>(matchPlayerJoin),
                                     reinterpret_cast<void*>(&Hook_CanAcceptPlayerJoinRequest),
                                     reinterpret_cast<void**>(&s_originalCanAcceptPlayerJoinRequest),
                                     "c_network_session::can_accept_player_join_request")) {
            patchCount++;
            NOBLE_INFO("LobbyPatches: Installed MinHook detour on can_accept_player_join_request at 0x{:X}", matchPlayerJoin);
        }
    }

    // 3b. Fallback direct RVA for can_accept_player_join_request if pattern scan missed
    if (!s_originalCanAcceptPlayerJoinRequest) {
        uint8_t* fnPlayerJoin = reinterpret_cast<uint8_t*>(base + 0x3973BC);
        static const uint8_t kPlayerJoinPrologue[5] = { 0x48, 0x8B, 0xC4, 0x48, 0x89 };
        if (0x3973BC + 5 <= size && std::memcmp(fnPlayerJoin, kPlayerJoinPrologue, 5) == 0) {
            if (HookManager::InstallHook(fnPlayerJoin,
                                         reinterpret_cast<void*>(&Hook_CanAcceptPlayerJoinRequest),
                                         reinterpret_cast<void**>(&s_originalCanAcceptPlayerJoinRequest),
                                         "c_network_session::can_accept_player_join_request")) {
                patchCount++;
                NOBLE_INFO("LobbyPatches: Installed MinHook detour on can_accept_player_join_request at hr+0x3973BC");
            }
        }
    }

    // 4. MinHook Detour on c_network_session::can_accept_join_request (RVA 0x3975D8)
    std::string_view sigJoinReq = "8B 81 F8 0E 06 00 45 33 ED 83 C0 FA 48 8B EA";
    uintptr_t matchJoinPattern = SigScanner::FindPattern(base, size, sigJoinReq);
    uintptr_t fnJoinEntry = (matchJoinPattern && matchJoinPattern >= 0x29) ? (matchJoinPattern - 0x29) : (base + 0x3975D8);
    if (fnJoinEntry) {
        static const uint8_t kJoinPrologue[5] = { 0x48, 0x89, 0x5C, 0x24, 0x18 };
        if (std::memcmp(reinterpret_cast<void*>(fnJoinEntry), kJoinPrologue, 5) == 0) {
            if (HookManager::InstallHook(reinterpret_cast<void*>(fnJoinEntry),
                                         reinterpret_cast<void*>(&Hook_CanAcceptJoinRequest),
                                         reinterpret_cast<void**>(&s_originalCanAcceptJoinRequest),
                                         "c_network_session::can_accept_join_request")) {
                patchCount++;
                NOBLE_INFO("LobbyPatches: Installed MinHook detour on can_accept_join_request at 0x{:X}", fnJoinEntry);
            }
        }
    }

    // 4b. MinHook Detour on c_network_session::acknowledge_join_request (RVA 0x397914)
    {
        uint8_t* fnAck = reinterpret_cast<uint8_t*>(base + 0x397914);
        static const uint8_t kAckPrologue[5] = { 0x40, 0x53, 0x48, 0x83, 0xEC };
        if (0x397914 + 5 <= size && std::memcmp(fnAck, kAckPrologue, 5) == 0) {
            if (HookManager::InstallHook(fnAck,
                                         reinterpret_cast<void*>(&Hook_AcknowledgeJoinRequest),
                                         reinterpret_cast<void**>(&s_originalAcknowledgeJoinRequest),
                                         "c_network_session::acknowledge_join_request")) {
                patchCount++;
                NOBLE_INFO("LobbyPatches: Installed MinHook detour on acknowledge_join_request at hr+0x397914");
            }
        }
    }

    // 5. Refusal tracing (log-only, same RVAs the reference host-probe uses). Prologue bytes are
    //    checked first so a different haloreach.dll build is left alone.
    {
        s_reachBase = base;
        s_reachSize = size;
        static const uint8_t kPrologue[5] = { 0x48, 0x89, 0x5C, 0x24, 0x08 };
        struct { uintptr_t rva; void* detour; void** orig; const char* name; } traces[] = {
            { 0x425368, reinterpret_cast<void*>(&Hook_JoinRefuse),    reinterpret_cast<void**>(&s_originalJoinRefuse),    "join_refuse" },
            { 0x42459C, reinterpret_cast<void*>(&Hook_ConnectRefuse), reinterpret_cast<void**>(&s_originalConnectRefuse), "connect_refuse" },
        };
        // Message encode / decode traces (hr+0x423D88 starts 48 89 5C 24 08; hr+0x423E40 starts 48 8B C4 48 89).
        {
            struct { uintptr_t rva; uint8_t sig[5]; void* detour; void** orig; const char* name; } msgHooks[] = {
                { 0x423D88, { 0x48, 0x89, 0x5C, 0x24, 0x08 }, reinterpret_cast<void*>(&Hook_EncodeMessage),
                  reinterpret_cast<void**>(&s_originalEncodeMessage), "message_encode" },
                { 0x423E40, { 0x48, 0x8B, 0xC4, 0x48, 0x89 }, reinterpret_cast<void*>(&Hook_DecodeMessage),
                  reinterpret_cast<void**>(&s_originalDecodeMessage), "message_decode" },
                { 0x4234E4, { 0x48, 0x8B, 0xC4, 0x48, 0x89 }, reinterpret_cast<void*>(&Hook_FindChannel),
                  reinterpret_cast<void**>(&s_originalFindChannel), "find_channel" },
                { 0x395DE8, { 0x40, 0x53, 0x55, 0x56, 0x57 }, reinterpret_cast<void*>(&Hook_BootPeer),
                  reinterpret_cast<void**>(&s_originalBootPeer), "boot_peer" },
                { 0x3AFFB4, { 0x48, 0x8B, 0xC4, 0x48, 0x89 }, reinterpret_cast<void*>(&Hook_NetLog),
                  reinterpret_cast<void**>(&s_originalNetLog), "game_net_log" },
                { 0x393830, { 0x48, 0x8B, 0xC4, 0x48, 0x89 }, reinterpret_cast<void*>(&Hook_SessionUpdate),
                  reinterpret_cast<void**>(&s_originalSessionUpdate), "session_update" },
                { 0x397048, { 0x40, 0x53, 0x48, 0x83, 0xEC }, reinterpret_cast<void*>(&Hook_GuestStep),
                  reinterpret_cast<void**>(&s_originalGuestStep), "guest_step" },
                { 0x411F38, { 0x48, 0x8B, 0xC4, 0x48, 0x89 }, reinterpret_cast<void*>(&Hook_LocalPlayers),
                  reinterpret_cast<void**>(&s_originalLocalPlayers), "local_players" },
                { 0x39E47C, { 0x48, 0x89, 0x5C, 0x24, 0x18 }, reinterpret_cast<void*>(&Hook_MemberConsume),
                  reinterpret_cast<void**>(&s_originalMemberConsume), "member_consume" },
                { 0x41F510, { 0x48, 0x89, 0x5C, 0x24, 0x18 }, reinterpret_cast<void*>(&Hook_PlayerSync),
                  reinterpret_cast<void**>(&s_originalPlayerSync), "player_sync" },
                { 0x133320, { 0x4C, 0x8B, 0xDC, 0x49, 0x89 }, reinterpret_cast<void*>(&Hook_TeamCount),
                  reinterpret_cast<void**>(&s_originalTeamCount), "team_count" },
                { 0x7BA58, { 0x48, 0x89, 0x5C, 0x24, 0x08 }, reinterpret_cast<void*>(&Hook_GameOver),
                  reinterpret_cast<void**>(&s_originalGameOver), "game_over" },
                { 0x59330, { 0x48, 0x83, 0xEC, 0x28, 0x8B },
                  (EnsureGameFinishStub(), s_gameFinishStub ? reinterpret_cast<void*>(s_gameFinishStub->code) : nullptr),
                  (s_gameFinishStub ? &s_gameFinishStub->originalFn : reinterpret_cast<void**>(&s_originalGameFinish)),
                  "game_finish" },
                { 0x43B98C, { 0x48, 0x89, 0x5C, 0x24, 0x18 }, reinterpret_cast<void*>(&Hook_CloseChannel),
                  reinterpret_cast<void**>(&s_originalCloseChannel), "close_channel" },
                { 0x43BAE8, { 0x48, 0x89, 0x5C, 0x24, 0x18 }, reinterpret_cast<void*>(&Hook_ChannelReceive),
                  reinterpret_cast<void**>(&s_originalChannelReceive), "channel_receive" },
            };
            for (auto& h : msgHooks) {
                uint8_t* fn = reinterpret_cast<uint8_t*>(base + h.rva);
                if (h.rva + 5 <= size && std::memcmp(fn, h.sig, 5) == 0) {
                    if (HookManager::InstallHook(fn, h.detour, h.orig, h.name)) {
                        NOBLE_INFO("LobbyPatches: {} trace at hr+0x{:X}", h.name, h.rva);
                    }
                } else {
                    NOBLE_WARN("LobbyPatches: {} prologue mismatch at hr+0x{:X}; trace skipped", h.name, h.rva);
                }
            }
        }
        // hr+0x429AA4 starts 48 89 5C 24 18 (not ...08 like the two above), so it is checked on its own.
        {
            static const uint8_t kMsgPrologue[5] = { 0x48, 0x89, 0x5C, 0x24, 0x18 };
            uint8_t* fn = reinterpret_cast<uint8_t*>(base + 0x429AA4);
            if (0x429AA4 + 5 <= size && std::memcmp(fn, kMsgPrologue, 5) == 0) {
                if (HookManager::InstallHook(fn, reinterpret_cast<void*>(&Hook_SessionMessage),
                                             reinterpret_cast<void**>(&s_originalSessionMessage), "session_message")) {
                    NOBLE_INFO("LobbyPatches: session message trace at hr+0x429AA4");
                }
            } else {
                NOBLE_WARN("LobbyPatches: session_message prologue mismatch at hr+0x429AA4; trace skipped");
            }
        }
        for (auto& t : traces) {
            uint8_t* fn = reinterpret_cast<uint8_t*>(base + t.rva);
            if (t.rva + 5 <= size && std::memcmp(fn, kPrologue, 5) == 0) {
                if (HookManager::InstallHook(fn, t.detour, t.orig, t.name)) {
                    NOBLE_INFO("LobbyPatches: refusal trace on {} at hr+0x{:X}", t.name, t.rva);
                }
            } else {
                NOBLE_WARN("LobbyPatches: {} prologue mismatch at hr+0x{:X}; trace skipped", t.name, t.rva);
            }
        }
    }

    // 6. Join-Any patch (the reference host JOIN_ANY): Turns two join-gate branches into jumps
    // (hr+0x397635 74 5E -> EB 5E, hr+0x397363 74 4D -> EB 4D) so the host accepts any join
    // even while loading or in active match simulation. Enabled by default for all sessions.
    {
        struct { uintptr_t rva; uint8_t expect; uint8_t disp; } gates[] = {
            { 0x397635, 0x74, 0x5E },
            { 0x397363, 0x74, 0x4D },
        };
        for (auto& g : gates) {
            uint8_t* at = reinterpret_cast<uint8_t*>(base + g.rva);
            if (g.rva + 2 <= size && at[0] == g.expect && at[1] == g.disp) {
                const uint8_t jmp[2] = { 0xEB, g.disp };
                if (WriteProtectedMemory(at, jmp, 2)) {
                    patchCount++;
                    NOBLE_INFO("LobbyPatches: Join-any gate patched at hr+0x{:X} (74 {:02X} -> EB {:02X})", g.rva, static_cast<int>(g.disp), static_cast<int>(g.disp));
                }
            } else {
                NOBLE_WARN("LobbyPatches: Join-any gate bytes differ at hr+0x{:X} ({:02X} {:02X}); left alone", g.rva, static_cast<int>(at[0]), static_cast<int>(at[1]));
            }
        }
    }

    // 7. MinHook Detour on c_network_session_membership::evaluate_coop_insertion_point (RVA 0x39ce10)
    // Prevents host rejection with refusal 0x1B (error_players_incompatible_for_network_coop:
    // "GAME ERROR: An error occurred while playing co-op." / "Error hosting co-op").
    std::string_view sigCoopInsertion = "48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 8B 71 2C 48 83 C8 FF";
    uintptr_t matchCoopInsertion = SigScanner::FindPattern(base, size, sigCoopInsertion);
    uintptr_t fnCoopInsertion = matchCoopInsertion ? matchCoopInsertion : (base + 0x39CE10);
    if (fnCoopInsertion) {
        static const uint8_t kCoopInsertionPrologue[5] = { 0x48, 0x89, 0x5C, 0x24, 0x08 };
        if (std::memcmp(reinterpret_cast<void*>(fnCoopInsertion), kCoopInsertionPrologue, 5) == 0) {
            if (HookManager::InstallHook(reinterpret_cast<void*>(fnCoopInsertion),
                                         reinterpret_cast<void*>(&Hook_EvaluateCoopInsertionPoint),
                                         reinterpret_cast<void**>(&s_originalEvaluateCoopInsertionPoint),
                                         "c_network_session_membership::evaluate_coop_insertion_point")) {
                patchCount++;
                NOBLE_INFO("LobbyPatches: Installed MinHook detour on evaluate_coop_insertion_point at 0x{:X}", fnCoopInsertion);
            }
        }
    }

    // 7b. MinHook Detour on c_network_session::abort (RVA 0x44EBA0)
    // Centralized session abort sink for all co-op incompatibilities
    std::string_view sigSessionAbort = "48 89 5C 24 08 57 48 83 EC 20 8B FA 48 8B D9 48 83 C1 30";
    uintptr_t matchSessionAbort = SigScanner::FindPattern(base, size, sigSessionAbort);
    uintptr_t fnSessionAbort = matchSessionAbort ? matchSessionAbort : (base + 0x44EBA0);
    if (fnSessionAbort) {
        static const uint8_t kSessionAbortPrologue[5] = { 0x48, 0x89, 0x5C, 0x24, 0x08 };
        if (std::memcmp(reinterpret_cast<void*>(fnSessionAbort), kSessionAbortPrologue, 5) == 0) {
            if (HookManager::InstallHook(reinterpret_cast<void*>(fnSessionAbort),
                                         reinterpret_cast<void*>(&Hook_SessionAbort),
                                         reinterpret_cast<void**>(&s_originalSessionAbort),
                                         "c_network_session::abort")) {
                patchCount++;
                NOBLE_INFO("LobbyPatches: Installed MinHook detour on c_network_session::abort at 0x{:X}", fnSessionAbort);
            }
        }
    }

    // 7c. MinHook Detour on c_network_session::process_host_handoff (RVA 0x4035DC)
    // Keeps session leadership on the actual Noble Host and prevents msg 16 (host-handoff) -> msg 12 (leave-session)
    // -> msg 14 (session-disband) during Co-Op Campaign and Multiplayer launch.
    {
        uintptr_t fnHostHandoff = base + 0x4035DC;
        static const uint8_t kHostHandoffPrologue[15] = {
            0x48, 0x8B, 0xC4, 0x48, 0x89, 0x58, 0x10, 0x48, 0x89, 0x70, 0x18, 0x48, 0x89, 0x78, 0x20
        };
        if (0x4035DC + sizeof(kHostHandoffPrologue) <= size &&
            std::memcmp(reinterpret_cast<const void*>(fnHostHandoff), kHostHandoffPrologue, sizeof(kHostHandoffPrologue)) == 0) {
            if (HookManager::InstallHook(reinterpret_cast<void*>(fnHostHandoff),
                                         reinterpret_cast<void*>(&Hook_ProcessHostHandoff),
                                         reinterpret_cast<void**>(&s_originalProcessHostHandoff),
                                         "c_network_session::process_host_handoff")) {
                patchCount++;
                NOBLE_INFO("LobbyPatches: Installed MinHook detour on process_host_handoff at 0x{:X}", fnHostHandoff);
            }
        } else {
            NOBLE_WARN("LobbyPatches: process_host_handoff bytes differ at hr+0x4035DC; left alone");
        }
    }

    // 8. Co-op and Firefight player limit bypass gates
    // Patches the campaign co-op player cap gate at hr+0x40954A (7E 22 -> EB 22)
    // and Firefight player cap gate at hr+0x40955B (76 11 -> EB 11) to bypass "too many for net coop / firefight".
    {
        struct { uintptr_t rva; uint8_t expect; uint8_t disp; } coopGates[] = {
            { 0x40954A, 0x7E, 0x22 }, // Campaign co-op player cap check (7E 22 -> EB 22)
            { 0x40955B, 0x76, 0x11 }, // Firefight player cap check (76 11 -> EB 11)
            { 0x411B6E, 0x74, 0x0D }, // Co-op in-progress membership compatibility abort check (74 0D -> EB 0D)
            { 0x411D71, 0x74, 0x0D }, // Co-op peer disconnect/abort check (74 0D -> EB 0D, bypasses reason 1Ch)
            { 0x397689, 0x75, 0x0A }, // Campaign in-progress join-request gate (75 0A -> EB 0A, bypasses refusal 5)
        };
        for (auto& g : coopGates) {
            uint8_t* at = reinterpret_cast<uint8_t*>(base + g.rva);
            if (g.rva + 2 <= size && at[0] == g.expect && at[1] == g.disp) {
                const uint8_t jmp[2] = { 0xEB, g.disp };
                if (WriteProtectedMemory(at, jmp, 2)) {
                    patchCount++;
                    NOBLE_INFO("LobbyPatches: Co-op player cap gate patched at hr+0x{:X} ({:02X} {:02X} -> EB {:02X})",
                               g.rva, static_cast<int>(g.expect), static_cast<int>(g.disp), static_cast<int>(g.disp));
                }
            } else {
                NOBLE_WARN("LobbyPatches: Co-op player cap gate bytes differ at hr+0x{:X} ({:02X} {:02X}); left alone",
                           g.rva, static_cast<int>(at[0]), static_cast<int>(at[1]));
            }
        }
    }

    // 8b. NOP out cmovz ecx, edx at hr+0x40958B (0F 44 CA -> 90 90 90)
    // Prevents ecx from being loaded with reason 1Bh (error_players_incompatible_for_network_coop)
    // when campaign insertion evaluation returns -1.
    uint8_t* atCmov = reinterpret_cast<uint8_t*>(base + 0x40958B);
    static const uint8_t kExpectedCmov[3] = { 0x0F, 0x44, 0xCA };
    if (0x40958B + 3 <= size && std::memcmp(atCmov, kExpectedCmov, 3) == 0) {
        static const uint8_t kNops[3] = { 0x90, 0x90, 0x90 };
        if (WriteProtectedMemory(atCmov, kNops, 3)) {
            patchCount++;
            NOBLE_INFO("LobbyPatches: NOPed cmovz ecx, edx at hr+0x40958B (0F 44 CA -> 90 90 90)");
        }
    } else {
        NOBLE_WARN("LobbyPatches: cmovz bytes differ at hr+0x40958B; left alone");
    }

    // 9. Theater saved film playback hooks:
    // Prevents error dialog "An error occurred during film playback" and premature match aborts
    // (AbortGame reason 7/9, SavedFilmSessionMonitor timeout abort, and SetGameEnded).
    {
        struct { uintptr_t rva; uint8_t sig[5]; void* detour; void** orig; const char* name; } theaterHooks[] = {
            { 0x3ABB70, { 0x48, 0x89, 0x5C, 0x24, 0x08 }, reinterpret_cast<void*>(&Hook_AbortGame),
              reinterpret_cast<void**>(&s_originalAbortGame), "c_game_engine::abort_game" },
            { 0x3ABE70, { 0x48, 0x89, 0x5C, 0x24, 0x10 }, reinterpret_cast<void*>(&Hook_SavedFilmSessionMonitor),
              reinterpret_cast<void**>(&s_originalSavedFilmSessionMonitor), "c_saved_film_manager::session_monitor" },
            { 0x3C08B4, { 0x48, 0x8B, 0xC4, 0x48, 0x89 }, reinterpret_cast<void*>(&Hook_SetGameEnded),
              reinterpret_cast<void**>(&s_originalSetGameEnded), "c_game_engine::set_game_ended" },
            { 0x41EB80, { 0x48, 0x89, 0x5C, 0x24, 0x08 }, reinterpret_cast<void*>(&Hook_LaunchReady),
              reinterpret_cast<void**>(&s_originalLaunchReady), "external_launch_ready" },
            { 0x41FBE8, { 0x48, 0x8B, 0xC4, 0x48, 0x89 }, reinterpret_cast<void*>(&Hook_IdentityMatch),
              reinterpret_cast<void**>(&s_originalIdentityMatch), "player_identity_match" },
            { 0x420298, { 0x48, 0x89, 0x5C, 0x24, 0x10 }, reinterpret_cast<void*>(&Hook_PlayerCreate),
              reinterpret_cast<void**>(&s_originalPlayerCreate), "player_create" },
            { 0x3CDF38, { 0x48, 0x89, 0x5C, 0x24, 0x08 }, reinterpret_cast<void*>(&Hook_ControllerApply),
              reinterpret_cast<void**>(&s_originalControllerApply), "controller_apply" },
            { 0x47A3C, { 0x48, 0x89, 0x5C, 0x24, 0x08 }, reinterpret_cast<void*>(&Hook_PlayerUpdateStep),
              reinterpret_cast<void**>(&s_originalPlayerUpdateStep), "player_update_step" },
            { 0x57434, { 0x48, 0x8B, 0xC4, 0x48, 0x89 }, reinterpret_cast<void*>(&Hook_PlayerUpdateLoop),
              reinterpret_cast<void**>(&s_originalPlayerUpdateLoop), "player_update_loop" },
            { 0x3AAB08, { 0x48, 0x83, 0xEC, 0x28, 0x45 }, reinterpret_cast<void*>(&Hook_SimGate),
              reinterpret_cast<void**>(&s_originalSimGate), "sim_gate" },
            { 0x3BF63C, { 0x48, 0x8B, 0xC4, 0x48, 0x89 }, reinterpret_cast<void*>(&Hook_LaunchReset),
              reinterpret_cast<void**>(&s_originalLaunchReset), "launch_reset" },
        };
        // Campaign-JIP diagnostic hooks are probe-only: a normal (non-NOBLE_JIP_PROBE) session must not carry the
        // instrumentation detours. The fixes themselves are separate from these log hooks.
        static const bool jipProbe = [] {
            wchar_t env[8] = {};
            return GetEnvironmentVariableW(L"NOBLE_JIP_PROBE", env, 8) > 0 && env[0] != L'0';
        }();
        const auto isProbeHook = [](const char* name) {
            return std::strstr(name, "launch_ready") || std::strstr(name, "identity_match") ||
                   std::strstr(name, "controller_apply") || std::strstr(name, "player_create") ||
                   std::strstr(name, "player_update_step") || std::strstr(name, "player_update_loop") ||
                   std::strstr(name, "sim_gate") || std::strstr(name, "launch_reset");
        };
        for (auto& h : theaterHooks) {
            if (isProbeHook(h.name) && !jipProbe) continue;
            uint8_t* fn = reinterpret_cast<uint8_t*>(base + h.rva);
            if (h.rva + 5 <= size && std::memcmp(fn, h.sig, 5) == 0) {
                if (HookManager::InstallHook(fn, h.detour, h.orig, h.name)) {
                    patchCount++;
                    NOBLE_INFO("LobbyPatches: {} hook installed at hr+0x{:X}", h.name, h.rva);
                }
            } else {
                NOBLE_WARN("LobbyPatches: {} prologue mismatch at hr+0x{:X}; hook skipped", h.name, h.rva);
            }
        }
    }

    s_isApplied = true;
    NOBLE_INFO("LobbyPatches: Lobby expansion active (Target MP: {}, Co-op: {}, Active Detours/Patches: {}).",
        s_maxMultiplayerPlayers, s_maxCoopPlayers, patchCount);
    return true;
}

void LobbyPatches::SetMaxMultiplayerPlayers(uint32_t maxPlayers) {
    if (maxPlayers > 128) maxPlayers = 128;
    if (maxPlayers < 16) maxPlayers = 16;
    s_maxMultiplayerPlayers = maxPlayers;
}

void LobbyPatches::SetMaxCoopPlayers(uint32_t maxPlayers) {
    if (maxPlayers > 16) maxPlayers = 16;
    if (maxPlayers < 4) maxPlayers = 4;
    s_maxCoopPlayers = maxPlayers;
}

} // namespace noble
