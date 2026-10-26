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

    // Pattern 1: Session Max Players Check (CMP reg, 16)
    // 83 F8 10 7E ?? (cmp eax, 0x10; jle ...)
    // Pattern 2: Lobby join validation (CMP reg, 16)
    // 83 FA 10 7C ?? (cmp edx, 0x10; jl ...)
    std::string_view sigLobbyCheck1 = "83 F8 10 7E";
    std::string_view sigLobbyCheck2 = "83 FA 10 7C";

    int patchCount = 0;
    uintptr_t currentAddr = base;
    uintptr_t endAddr = base + size;

    while (currentAddr < endAddr) {
        uintptr_t match = SigScanner::FindPattern(currentAddr, endAddr - currentAddr, sigLobbyCheck1);
        if (!match) break;

        uint8_t newLimit = static_cast<uint8_t>(s_maxMultiplayerPlayers);
        // Patch the immediate byte from 0x10 (16) to newLimit (e.g. 0x20 = 32)
        if (WriteProtectedMemory(reinterpret_cast<void*>(match + 2), &newLimit, 1)) {
            patchCount++;
            NOBLE_INFO("LobbyPatches: Patched session CMP EAX, 16 -> CMP EAX, {} at 0x{:X}", 
                s_maxMultiplayerPlayers, match);
        }
        currentAddr = match + 4;
    }

    currentAddr = base;
    while (currentAddr < endAddr) {
        uintptr_t match = SigScanner::FindPattern(currentAddr, endAddr - currentAddr, sigLobbyCheck2);
        if (!match) break;

        uint8_t newLimit = static_cast<uint8_t>(s_maxMultiplayerPlayers);
        if (WriteProtectedMemory(reinterpret_cast<void*>(match + 2), &newLimit, 1)) {
            patchCount++;
            NOBLE_INFO("LobbyPatches: Patched session CMP EDX, 16 -> CMP EDX, {} at 0x{:X}", 
                s_maxMultiplayerPlayers, match);
        }
        currentAddr = match + 4;
    }

    s_isApplied = true;
    NOBLE_INFO("LobbyPatches: Applied {} lobby expansion memory patches. Max MP players: {}, Max Co-op: {}",
        patchCount, s_maxMultiplayerPlayers, s_maxCoopPlayers);
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
