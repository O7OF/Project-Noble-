#include "LobbyPatches.hpp"
#include "SigScanner.hpp"
#include "common/Logger.hpp"
#include <psapi.h>
#include <vector>

namespace noble {

bool LobbyPatches::s_isApplied = false;
uint32_t LobbyPatches::s_maxMultiplayerPlayers = 32; // Default to expanded 32 players
uint32_t LobbyPatches::s_maxCoopPlayers = 8;         // Default to expanded 8 players for campaign/firefight

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

    NOBLE_INFO("LobbyPatches: Scanning haloreach.dll for hardcoded 16-player lobby restrictions...");

    MODULEINFO modInfo{};
    if (!GetModuleInformation(GetCurrentProcess(), hReachDll, &modInfo, sizeof(modInfo))) {
        NOBLE_ERROR("LobbyPatches: Failed to query haloreach.dll module bounds.");
        return false;
    }

    uintptr_t base = reinterpret_cast<uintptr_t>(modInfo.lpBaseOfDll);
    size_t size = modInfo.SizeOfImage;

    // IMPORTANT: A 4-byte pattern such as "83 FA 10 7C" (CMP EDX, 16; JL) or "83 F8 10 7E" (CMP EAX, 16; JLE)
    // is generic compiler output matching 60+ unrelated loops (e.g. bitfield packers, audio channels,
    // math vectors, prediction atoms). Blindly replacing 16 with 32 across the entire .text section
    // corrupts internal engine data structures (specifically atom prediction at RVA 0x274E5E, causing
    // a fatal 0xC0000005 NULL-pointer dereference at RVA 0x2744F2 during map loading).
    //
    // Qualified session signatures must match complete session manager call sites.
    int patchCount = 0;

    // Specific network session player bounds check signature (with surrounding context)
    // e.g., session manager peer capacity validation
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

    s_isApplied = true;
    NOBLE_INFO("LobbyPatches: Lobby expansion initialized safely (Target MP: {}, Co-op: {}, Verified Patches: {}).",
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
