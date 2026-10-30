# Campaign co-op failure - both logs aligned (session b3e82185d33714a5)

Server timestamps are UTC (`Z`), client timestamps are ET = UTC-4. All times below are **ET**.

| ET time | side | event |
|---|---|---|
| 19:39:20.811 | server | host relay socket connected (menu process) |
| 19:39:46.132 | guest | lobby launch received (m10, campaign) |
| 19:39:46.562 | guest | guest relaunched with `-connect 147.124.149.202:11774` |
| 19:39:47.299 | server | host relay socket connected again (match process) |
| ~19:39:47-49 | server | second guest id connects (match process); room now has 2 guests, 1 host |
| 19:39:49.279 | guest | engine launched as guest, transport configured, relay thread started |
| 19:39:49.938 | guest | `SEND msg 8 (join-request)` -> host replies `msg 4` |
| 19:39:56.335 | server | host relay socket **disconnected** (the menu process exiting) |
| ~19:39:56 | server | first guest id disconnects (guest menu process exiting) |
| **19:39:57.497** | guest | **`game session started at frame 530, loading screen off`** - guest in game |
| 19:40:08.869 | guest | **`Slot 6 - GameResults`** - match ended 11 s after start |
| 19:40:10.215 | guest | channel closed, `reason 14` (state was 5) |
| 19:40:11.189 | guest | post-game panel -> quit (user action) |
| 19:40:20.995 | server | host relay socket disconnected (match process ending) |

## What this rules out

- **Relay/backbone is healthy.** Both sides had relay sockets, then the *menu* processes exited right after the hand-off
  (that is the 19:39:56 disconnect pair), while the match processes kept exchanging engine packets over direct UDP
  (`relayFallback: false` in the guest log) until the end. The server log shows no refusal and no missing room.
- **The guest joined the session.** `game session started` on the guest at 19:39:57.
- **It is not the old "guest joins mid-cinematic" bug.** The guest was in the session before the mission started.
- **No abort or boot on the guest.** `abort called`, `boot_peer`, join refusals and msg 10 (join-abort) are all absent from
  the guest log.

## What is still unknown

The match ended at 19:40:08 from the engine, with no client-side explanation logged on the guest. The deciding evidence is
in **O7's host client log** for 19:39:40-19:40:10:

```
abort called ... reason=...          (27/0x1B incompatible, 28/0x1C peer disconnect, 29/0x1D co-op state timeout)
SUPPRESSED session abort ...
boot_peer ... reason ...
evaluate_coop_insertion_point ...
campaign host without a lobby peer count ... / guest engine traffic appeared after ... ms
PauseMenu: game over ...   (and what came just before it)
Lobby start broadcast: ... roster=N
```

Extract it with:

```powershell
.\tools\extract-coop-log.ps1 -Log build\bin\Release\noble-client.log `
    -Start "2026-10-30 19:39:40" -End "2026-10-30 19:40:10" -Role host `
    -Out "$env:USERPROFILE\Downloads\campaign-host.log.txt"
```

## Two candidate explanations (both testable from that log)

1. **Host-side abort** - the engine's co-op validation fired on the host (0x1B/0x1C/0x1D). The existing LobbyPatches aborts
   for 0x1B are patched, but 0x1D (state sync timeout) is not; an 11 s lifetime looks like a timeout.
2. **Host ended the match normally** - e.g. the host's client went back to the menu (crash, alt-tab quit, or a mission end),
   which delivers GameResults to the guest. His log would end with `PauseMenu: game over` / engine shutdown and no abort.
