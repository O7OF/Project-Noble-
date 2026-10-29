#include "LobbyPatches.hpp"
#include "HookManager.hpp"
#include "SigScanner.hpp"
#include "common/Logger.hpp"
#include <psapi.h>
#include <vector>

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

    // 4. MinHook Detour on c_network_session::can_accept_join_request (RVA 0x3975D8)
    std::string_view sigJoinReq = "8B 81 F8 0E 06 00 45 33 ED 83 C0 FA 48 8B EA";
    uintptr_t matchJoinPattern = SigScanner::FindPattern(base, size, sigJoinReq);
    if (matchJoinPattern && matchJoinPattern >= 0x29) {
        uintptr_t fnJoinEntry = matchJoinPattern - 0x29;
        if (HookManager::InstallHook(reinterpret_cast<void*>(fnJoinEntry),
                                     reinterpret_cast<void*>(&Hook_CanAcceptJoinRequest),
                                     reinterpret_cast<void**>(&s_originalCanAcceptJoinRequest),
                                     "c_network_session::can_accept_join_request")) {
            patchCount++;
            NOBLE_INFO("LobbyPatches: Installed MinHook detour on can_accept_join_request at 0x{:X}", fnJoinEntry);
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
