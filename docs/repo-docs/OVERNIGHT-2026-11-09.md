# Overnight 2026-11-09 (Claude, while Tyler slept): what happened

Tyler's order: 1. multiplayer toward 128 players, 2. multiplayer optimization + bugs, 3. swarm / Pelican / tools.
Rules kept: build-dev only (via `overnight_build.bat`), no commits/pushes, haloreach.dll untouched, CRLF, every write
re-read (the write glitch hit 3 times today; each time re-written and verified).

## Done
| Time (ET) | What | State |
|---|---|---|
| 10:12 | Pelican ramp/gear key X (`src/engine/PelicanControls.*`, dev only) + Forge World switched to the `_scripted_doors` Pelicans | built, not tested in game |
| 10:22 | Docs: `docs/pelican.md` (new), swarm notes status + perf goal, LIMITS-TODO priorities | done |
| 10:25 | First overnight build (Pelican + numpad keys) | OK |
| 10:30-10:55 | **#32 membership update message for 32 players** (`src/host/SlotMessage.*`, `SlotMessageTable.inc`, `tools/limits/gen_message.py`, `docs/limits-players-message.md`) | built, **LAN-tested OK with 3 players** |

### #32 test result
`overnight_lan3.bat` (host + 2 clients, `-slot-patch -slot-players 32`, 4 min): host sends message 26 at 0xA300 bytes, both
clients decode it (`RECV msg 26 size 41728 decoded true`), session runs, all exit 0. A first run of the same build at 10:39
did not join (host never sent 26) - not reproduced; `overnight_lan3_msgoff.bat` (group off) joined too.

| 10:58-11:01 | High-slot test mode (`NOBLE_SLOT_HIGH=1`, `overnight_lan3_high.bat`, `overnight_lan3_hightrace.bat`) + decoder fix 0x4273B0 | built; join **fails** (below) |
| evening | Root-cause analysis of the high-slot failure (disassembly of the logged stacks) | lead found, not patched yet |

### High-slot result (the real 32-player blocker right now)
With joining players forced into slots >= 16, the message goes through (client decodes every update), but the **host adds
the guest's player at slot 16 and removes it 16 ms later** without a boot. The guest then sees itself gone, leaves, and its
rejoin is refused. Cause (strong lead): the engine's player add / remove helpers (0x39D620, 0x39D7F8, 0x39D8A8, 0x39D950)
reach the peer's player-slot table through address math that has no shadow-slot cave yet, so for peer 17 they read the wrong
memory and remove a player. These sites are already in `docs/limits-players-sites.json` (`add@` / `not@` kinds); they just
were never turned into caves. Fix = cave them, rebuild, rerun `overnight_lan3_high.bat`.

## Evening session (Tyler back, 20:24-21:10 ET)
- Built + tested 3 times (Tyler ran the .bat files; screen control hides his other windows, so not used).
- Fix 1, player add/remove caves (`SlotMemberCaves.inc`): host now keeps the slot-16 player. Fix 2, 17-bit peer masks
  widened (`kPeerMasks32`): no visible change. Trace run: no unreviewed slot accesses on the guest.
- Real blocker found: with a joiner in peer 17 / player 16 the host's **running match ends ~10 ms after the join
  request** and the guest then waits for a match that is gone. Normal-slot join-in-progress still works (all earlier
  logs). Next: find why the engine unloads the game for a slot-16 joiner (in-game tables, see LIMITS-HANDOFF.md).

## Evening, part 2 (21:10-21:42 ET)
- Traced why the host's match ends when a slot-16 player joins: the engine's session -> game player sync (0x41F510,
  one of the known 16/17-entry "fixed tables") emits a "player left" event for the host's own player, so the match has
  no players and ends. Next job: give 0x41F510 / 0x40A760 32-player tables (details in LIMITS-HANDOFF.md).
- Two supporting fixes kept (team count to 31, game player event index to 5 bits). Dev traces added in LobbyPatches.cpp.

## Evening, part 3 (22:00-22:15 ET)
- **Host side fixed:** with a slot-16 joiner the host's match now keeps running (was ending instantly). Cause: the
  extra peer slots 17-31 started with "player 0" in their player list, so the join removed the host's player and wrote
  the guest over it. Fix in SlotPatch.cpp (`ResetShadowPeerPlayerTables`).
- **Still open:** the guest gets the membership update but never replies, then gives up after 16 s. Next: trace the
  guest side.

## Helpers added (repo root / tools)
- `overnight_build.bat`: build-dev without pause, log `build\overnight-build.log`.
- `overnight_lan3.bat` / `overnight_lan3_msgoff.bat` -> `tools\overnight-lan3.ps1` (`-MsgOff`, `-Tag`): 3-instance LAN test,
  logs `<tag>-host.log` (Release), `<tag>-client1.log` (Release2), `<tag>-client2.log` (Release3), summary
  `build\overnight-lan3-<tag>.txt`. A run joined only if the host log has `SEND msg 26` and `RECV msg 27/35`.

## Next
- Cave the uncaved player add/remove sites (LIMITS-HANDOFF.md, 2026-11-09 overnight section), rerun the high-slot test, then
  the hightrace run for anything else (MEMBER-NEW).
- The nine 17-bit `0x1FFFF` peer masks.
- Then the remaining C12-L blockers (LIMITS-HANDOFF.md), multiplayer optimization, bug pass.

## For Tyler to check
- `git status` will show everything above uncommitted. Suggested split: Pelican (PelicanControls, SpawnTest, DevMenu, docs/pelican.md,
  tools/hrek), #32 (SlotMessage*, SlotPatch.cpp, CMakeLists.txt, tools/limits/gen_message.py, docs/limits-players-message.md),
  overnight helpers + docs. The high-slot code (`ApplyHighSlotTest`, `NOBLE_SLOT_HIGH`) is test-only and off by default.
