#include "LobbyPatches.hpp"
#include "HookManager.hpp"
#include "SigScanner.hpp"
#include "common/Logger.hpp"
#include <psapi.h>
#include <vector>
#include <string>
#include <cstdio>
#include <atomic>
#include <cstring>
#include <cwchar>
#include <intrin.h>

namespace noble {

bool LobbyPatches::s_isApplied = false;
uint32_t LobbyPatches::s_maxMultiplayerPlayers = 32; // Default to expanded 32 players
uint32_t LobbyPatches::s_maxCoopPlayers = 8;         // Default to expanded 8 players for campaign/firefight

// Detour for c_network_session_membership::set_slot_counts (?set_slot_counts@c_network_session_membership@@QAAXJJ_N@Z)
// Discovered from Omaha symbol database: reach_omaha_release_symbols.csv
typedef void(__fastcall* SetSlotCountsFn)(void* thisPtr, int32_t privateSlots, int32_t publicSlots, bool isRanked);
static SetSlotCountsFn s_originalSetSlotCounts = nullptr;

static void __fastcall Hook_SetSlotCounts(void* thisPtr, int32_t privateSlots, int32_t publicSlots, bool isRanked) {
    int32_t targetSlots = publicSlots;
    if (publicSlots == 16 || publicSlots < static_cast<int32_t>(LobbyPatches::GetMaxMultiplayerPlayers())) {
        targetSlots = static_cast<int32_t>(LobbyPatches::GetMaxMultiplayerPlayers());
        NOBLE_INFO("LobbyPatches: Expanding session slot count from {} to {} (private={}, isRanked={})", 
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
    if (result < 0) {
        NOBLE_WARN("LobbyPatches: evaluate_coop_insertion_point returned {} (incompatible); overriding to 0 (mission start) for co-op network play", result);
        result = 0;
    }
    return result;
}

// Join / connect refusal tracing. Reach Renaissance's host-probe hooks these two haloreach.dll
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

typedef void(__fastcall* AcknowledgeJoinRequestFn)(void* thisPtr, void* targetAddr, uint32_t reason);
static AcknowledgeJoinRequestFn s_originalAcknowledgeJoinRequest = nullptr;

static void __fastcall Hook_AcknowledgeJoinRequest(void* thisPtr, void* targetAddr, uint32_t reason) {
    NOBLE_WARN("LobbyPatches: [{} pid {}] c_network_session::acknowledge_join_request: session={}, target={}, reason={} (0x{:X}) caller hr+0x{:X}",
        RefuseRole(), GetCurrentProcessId(), thisPtr, targetAddr, reason, reason, CallerRva(_ReturnAddress()));
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

typedef uint64_t(__fastcall* EncodeMessageFn)(void* table, void* stream, int32_t type, int32_t size, void* data);
static EncodeMessageFn s_originalEncodeMessage = nullptr;
static uint64_t __fastcall Hook_EncodeMessage(void* table, void* stream, int32_t type, int32_t size, void* data) {
    if (type >= 0 && type < 64 && s_msgOutCount[type].fetch_add(1) < 15) {
        NOBLE_INFO("LobbyPatches: [{} pid {}] SEND msg {} ({}) size {}", RefuseRole(), GetCurrentProcessId(), type,
                   SessionMessageName(type), size);
    }
    return s_originalEncodeMessage ? s_originalEncodeMessage(table, stream, type, size, data) : 0;
}

typedef uint64_t(__fastcall* DecodeMessageFn)(void* table, void* stream, int32_t* type, int32_t* size, void* buffer);
static DecodeMessageFn s_originalDecodeMessage = nullptr;
static uint64_t __fastcall Hook_DecodeMessage(void* table, void* stream, int32_t* type, int32_t* size, void* buffer) {
    const uint64_t ok = s_originalDecodeMessage ? s_originalDecodeMessage(table, stream, type, size, buffer) : 0;
    const int32_t t = type ? *type : -1;
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
        static const uint8_t kAckPrologue[5] = { 0x53, 0x48, 0x83, 0xEC, 0x50 };
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

    // 5. Refusal tracing (log-only, same RVAs Reach Renaissance's host-probe uses). Prologue bytes are
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

    // 6. Join-Any patch (Reach Renaissance RR_JOIN_ANY): Turns two join-gate branches into jumps
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

    // 8. Co-op and Firefight player limit bypass gates
    // Patches the campaign co-op player cap gate at hr+0x40954A (7E 22 -> EB 22)
    // and Firefight player cap gate at hr+0x40955B (76 11 -> EB 11) to bypass "too many for net coop / firefight".
    {
        struct { uintptr_t rva; uint8_t expect; uint8_t disp; } coopGates[] = {
            { 0x40954A, 0x7E, 0x22 }, // Campaign co-op player cap check (7E 22 -> EB 22)
            { 0x40955B, 0x76, 0x11 }, // Firefight player cap check (76 11 -> EB 11)
            { 0x411B6E, 0x74, 0x0D }, // Co-op in-progress membership compatibility abort check (74 0D -> EB 0D)
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

    s_isApplied = true;
    NOBLE_INFO("LobbyPatches: Lobby expansion active (Target MP: {}, Co-op: {}, Active Detours/Patches: {}).",
        s_maxMultiplayerPlayers, s_maxCoopPlayers, patchCount);
    return true;
}

void LobbyPatches::SetMaxMultiplayerPlayers(uint32_t maxPlayers) {
    if (maxPlayers > 64) maxPlayers = 64;
    if (maxPlayers < 16) maxPlayers = 16;
    s_maxMultiplayerPlayers = maxPlayers;
}

void LobbyPatches::SetMaxCoopPlayers(uint32_t maxPlayers) {
    if (maxPlayers > 16) maxPlayers = 16;
    if (maxPlayers < 4) maxPlayers = 4;
    s_maxCoopPlayers = maxPlayers;
}

} // namespace noble
