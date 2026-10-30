#pragma once

#include <windows.h>
#include <cstdint>
#include <cstddef>

namespace noble {

class LobbyPatches {
public:
    // Applies in-memory patches to haloreach.dll to expand the 16-player lobby limit
    static bool Apply(HMODULE hReachDll);

    // Dynamic configuration
    static void SetMaxMultiplayerPlayers(uint32_t maxPlayers);
    static void SetMaxCoopPlayers(uint32_t maxPlayers);

    static uint32_t GetMaxMultiplayerPlayers() { return s_maxMultiplayerPlayers; }
    static uint32_t GetMaxCoopPlayers() { return s_maxCoopPlayers; }
    static bool IsApplied() { return s_isApplied; }

    static bool HasRemotePlayerJoined();
    static void ResetRemotePlayerJoined();
    static size_t GetRemotePlayerAddCount();

private:
    static bool s_isApplied;
    static uint32_t s_maxMultiplayerPlayers;
    static uint32_t s_maxCoopPlayers;
};

} // namespace noble
