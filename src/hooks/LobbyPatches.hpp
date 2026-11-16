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

    // Seats a multiplayer session can really declare in this process: 16 on the stock engine, or 32 only when the
    // dev 32-player slot patch is installed AND the dev 32-slot declaration switch (NOBLE_MP_SLOTS32=1) is on. The
    // menus, the -max-players clamp, the session target and the adverts all read this one number, so nothing can
    // promise seats the engine cannot seat. Dev 32 mode is still behind the high-slot blocker.
    static int EffectiveMultiplayerSeats();
    static uint32_t GetMaxCoopPlayers() { return s_maxCoopPlayers; }
    static bool IsApplied() { return s_isApplied; }

    static bool HasRemotePlayerJoined();
    static void ResetRemotePlayerJoined();

    // Campaign co-op: a host boots its guest with reason 9 a few seconds after both spawn, which ends the mission with
    // "Error hosting co-op". While this is set, a reason-9 boot of a guest is logged and dropped instead.
    static void SetCampaignCoop(bool on);
    static bool IsCampaignCoop();
    static size_t GetPlayerCreateSuccessCount();
    static size_t GetRemotePlayerAddCount();

    // Live players from the engine's game-player table (authoritative "who is in this match" count). The transport
    // peer count and the msg-27 membership counter can over-count (retries) or lag (direct connects), so the
    // scoreboard/loading roster size themselves from this when it is available.
    static size_t GetGamePlayerCount();

private:
    static bool s_isApplied;
    static uint32_t s_maxMultiplayerPlayers;
    static uint32_t s_maxCoopPlayers;
};

} // namespace noble
