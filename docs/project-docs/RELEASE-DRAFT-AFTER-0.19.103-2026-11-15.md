> STATUS: **DRAFT - NOT PUBLISHED.** Version not bumped (tree is still 0.19.103). Nothing here ships
> until Tyler says "release it". Written 2026-11-15 ~12:05 ET after the stabilization pass;
> companion: `STABILIZATION-REPORT-2026-11-15.md` (changes + QA matrix) and `QA-VERIFY-2026-11-15.md`.

# Release draft + known issues - next release after v0.19.103

## What would actually be in a release built from the current tree

The tree differs from published v0.19.103 by exactly one commit plus this pass's clamps:

- `d636b31` - test harness: per-player tick gate fixed (dev/test only; `-horde` / `-spawn-test` / dev cheats work
  again). No user-facing change.
- Stabilization clamps (`b2065bd`, pushed to private main+wip-merge; version unchanged):
  campaign co-op MAX PLAYERS clamped to 4; multiplayer MAX PLAYERS picker capped at 32; ENGINE LIMITS shows
  STOCK only (extended profiles stay disabled).

There are **no new user-facing features, content or gameplay changes** since v0.19.103.

## Draft release notes (only if Tyler chooses a small stability release, e.g. v0.19.104)

### Stability and safety

- **Campaign co-op MAX PLAYERS is limited to 4.** Higher values (8/12/16) were offered after v0.19.102 but 5-player
  campaign co-op was never proven and could crash; the only proven count is 4.
- **Multiplayer MAX PLAYERS picker is capped at 32** (stock 16 remains the default). The 48/64/96/128 entries were
  never supported by a tested session; 128-player work is on hold. Matchmaking/server-browser limits are unchanged.
- **ENGINE LIMITS now shows STOCK only.** The extended profiles (V16 / particles / light volumes) were already disabled
  in v0.19.103 (the build force-resets them for stability); the Settings row now reflects that instead of offering a
  selection that could not take effect and had been reported to cause black-screen loads / network kicks.

### Under the hood

- Fixed the dev/test per-player tick hook so the heavy-load test harness (`-horde`, `-spawn-test`, dev cheats) runs
  again. No effect on normal play.

## Known issues (current tree)

**Blocked / not implemented (do not advertise):**
1. Exact **per-player visor colour** in matches: stock-slot fallback only (v0.19.102+ behaviour). Extended finishes are
   exact in the Armory preview but use the nearest stock colour in a match.
2. **Selectable colour-variant armour effects** (blue flames etc.): recolour proof exists; no Armory entries (needs
   engine effect-table work).
3. **Multiplayer above 32 players**: 128-player work is ON HOLD; the UI is clamped, session/replication work belongs to
   Claude (see `NOTE-NETWORK-HANDOFF-TO-CLAUDE.md`).
4. **Campaign co-op above 4 players**: broken/unproven (5-player attempt crashed or black-screened); UI clamped.
5. **Extended engine limits** (V16 / Extended+Particles / Full): disabled for stability; the setting is Stock-only.
   Root cause work is Stage 3 of the Phase-2 prompt; networking profile matching is Claude's.
6. **Real two-PC lobby-code join** (private and public): codec + UI proven (self-test 0 failures) but the end-to-end
   join has never been tested - needs Tyler/a second PC.
7. **Forge editor (monitor) toggle** inside Forge World: Forge mode loads and plays, but switching to the editor monitor
   is untested.
8. **Full save LOAD/resume** flow from the menu: checkpoint saving is proven; loading an old checkpoint was not
   exercised automatically.
9. **Hex-number UI bug** ("0000001c" / "Respawn in 00000001"): UNPROVEN root cause; runtime check did not reproduce.
10. Tiny cosmetic: the server-browser footer hint text ("Select a server from the list to favourite it or copy…") can
    clip at the right edge when no server is selected.

**Waiting on Tyler's hand checks:** campaign skulls row retest; toggle crouch / toggle zoom feel; two-PC lobby code;
campaign skulls + late UI items from v0.19.103.

## Recommendation: SHIP vs HOLD

**Recommendation: HOLD - do not publish a release from this tree right now** (and no version bump has been made).

Why: since v0.19.103 there is no new content or feature work; the only user-visible delta is three safety clamps. A
release would mostly re-ship the same build and restart the public download/tag/notes cycle for a small risk reduction,
and the biggest shipped risk (extended limits causing black screens/network kicks) has not been fixed, only hidden.

**Ship a small stability release only if** Tyler wants the crash-capable player-count options removed from users'
builds before the next feature batch; in that case v0.19.104 would contain exactly the "Stability and safety" notes
above (version bump + rebuild + release notes + ChangelogData entry + publish, all only after "release it").

**Hold back from any release until proven:** the extended engine limits (V16/V16P/Full), multiplayer >32, campaign >4,
exact per-player visor colour, selectable armour-effect variants, two-PC lobby-code join, Forge editor monitor, and the
128-player work (Tyler's hold). When features resume, the next real feature release should be built on top of this
stabilized baseline.

## If Tyler says "release it" (checklist, not done)

1. Bump CMakeLists/Version.hpp/BuildIdentity.hpp/noble.rc to the new version; add `release-notes/vX.md` + ChangelogData.
2. Rebuild both gates; run the QA matrix in `STABILIZATION-REPORT` again on the final commit; regenerate the four
   captures if the version string changed on screen.
3. Commit + push `main` and `wip-merge`; run `tools\publish-release.ps1`; verify asset hashes/digests, draft=false,
   latest; report both commit hashes + release URL.