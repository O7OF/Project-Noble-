#include "LobbyPatches.hpp"
#include "HookManager.hpp"
#include "SigScanner.hpp"
#include "common/Logger.hpp"
#include <psapi.h>
#include <vector>
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

typedef int32_t(__fastcall* CanAcceptPlayerJoinRequestFn)(void* thisPtr, void* playerData, int32_t playerIndex, int32_t flags);
static CanAcceptPlayerJoinRequestFn s_originalCanAcceptPlayerJoinRequest = nullptr;

static int32_t __fastcall Hook_CanAcceptPlayerJoinRequest(void* thisPtr, void* playerData, int32_t playerIndex, int32_t flags) {
    int32_t result = 0;
    if (s_originalCanAcceptPlayerJoinRequest) {
        result = s_originalCanAcceptPlayerJoinRequest(thisPtr, playerData, playerIndex, flags);
    }
    if (result != 0) {
        NOBLE_WARN("LobbyPatches: can_accept_player_join_request rejected with refusal code 0x{:X}; overriding to ACCEPT (0)", result);
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
    }
    if (s_originalConnectRefuse) s_originalConnectRefuse(a1, a2, a3);
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
        static const uint8_t kPrologue[5] = { 0x48, 0x89, 0x5C, 0x24, 0x08 };
        struct { uintptr_t rva; void* detour; void** orig; const char* name; } traces[] = {
            { 0x425368, reinterpret_cast<void*>(&Hook_JoinRefuse),    reinterpret_cast<void**>(&s_originalJoinRefuse),    "join_refuse" },
            { 0x42459C, reinterpret_cast<void*>(&Hook_ConnectRefuse), reinterpret_cast<void**>(&s_originalConnectRefuse), "connect_refuse" },
        };
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

    // 6. -join-any: Reach Renaissance's RR_JOIN_ANY patch. Turns two join-gate branches into jumps
    //    (hr+0x397635 74 5E -> EB 5E, hr+0x397363 74 4D -> EB 4D) so the host accepts any join.
    //    Off by default; in-memory only.
    if (std::wcsstr(GetCommandLineW(), L"-join-any") != nullptr) {
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
                    NOBLE_INFO("LobbyPatches: -join-any patched hr+0x{:X} (74 {:02X} -> EB {:02X})", g.rva, static_cast<int>(g.disp), static_cast<int>(g.disp));
                }
            } else {
                NOBLE_WARN("LobbyPatches: -join-any bytes differ at hr+0x{:X} ({:02X} {:02X}); left alone", g.rva, static_cast<int>(at[0]), static_cast<int>(at[1]));
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
