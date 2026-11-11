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

    // Campaign co-op: a host boots its guest with reason 9 a few seconds after both spawn, which ends the mission with
    // "Error hosting co-op". While this is set, a reason-9 boot of a guest is logged and dropped instead.
    static void SetCampaignCoop(bool on);
    static size_t GetRemotePlayerAddCount();

private:
    static bool s_isApplied;
    static uint32_t s_maxMultiplayerPlayers;
    static uint32_t s_maxCoopPlayers;
};

} // namespace noble
