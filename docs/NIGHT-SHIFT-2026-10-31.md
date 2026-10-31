# Night shift plan - 2026-10-31 03:30 to 11:30 ET

Tyler is asleep until ~11:30. Two agents, one tree. Loop per block: claim in TODO + AGENT-STATUS, build 0/0, evidence,
handoff entry, commit+push, review the other agent's last commit, next block. Tests muted, clients killed by path.

## Block 1 - 03:30-04:45
@deepseek: Elite pipeline. `tool.exe` over the Prophets' Hand `m10.map` -> Elite/rank material tags + shader colour
parameters; parse `rrpmcg` -> slot table in `docs/reference/elite-slots.md`; list HREK substitutes. Done when the reference
doc exists and a build still passes.
@claude: finish controller settings. Y=Apply / X=Discard icons, Button Layout row cycles, diagram follows preset, paddles
in the binding list (device path, not XInput), sensitivity 1-10. Done with captures of both prompts + preset change.

## Block 2 - 04:45-06:00
@deepseek: voice transport skeleton. WASAPI capture -> 20 ms Opus frames -> relay `voice` message -> jitter buffer ->
playback; loopback and two-client log tests. Done when two processes exchange voice frames (no UI yet).
@claude: armory layout. Centre Spartan/Elite under the rule, nameplate matches main menu, spotlight gradient; then draw
`emblemBackgroundId` in PlayerRow/lobby rows. Done with before/after captures.

## Block 3 - 06:00-07:15
@deepseek: post-game clean panel. Fix `-postgame-test` timing, find the multiplayer end path, raise ShowGameOver there,
capture evidence; then `exit_experience` sounds (counter_loop, totals, slot_machine at 70%) and `mission_complete` credit
split. Done with slayer+campaign end captures and a log line for the split.
@claude: scoreboard wiring. Real ping column once exposed, un-grey buttons when endpoints land, mute-icon placeholders;
results screen stubs for Return to lobby / Waiting for host. Done with captures.

## Block 4 - 07:15-08:30
@deepseek: backend endpoints - report, vote-to-kick, End Round/End Match host actions; pad Back suppression; real ping in
transport. Done with endpoint tests (`node --check`, curl) and a pad-Back capture.
@claude: variant picker (shipped variants) and Forge/custom/Firefight options page scaffolds against the planned schema.
Done with captures.

## Block 5 - 08:30-09:45
@deepseek: lobby profile summary fields + privacy flags on the server, then match-end session survival + rotation array.
Done with endpoint checks and a lobby-return test.
@claude: lobby profile card UI (rank/emblem/Spartan render/armour %/challenges/commendations) with ADD FRIEND disabled
until the friends API; friends-list stubs. Done with capture.

## Block 6 - 09:45-11:00
@deepseek: variant viewer RE of `c_game_variant` (field map doc) + credit validation design/implementation (caps, reject
impossible results) + voice transport polish/handshake for UI.
@claude: emblem colour picker + Nameplate(id) audit; readability re-captures of every changed screen.

## Block 7 - 11:00-11:30
Both: cross-review every open [x], rebuild both heads 0/0, refresh Downloads mirrors + Claude package, write the
"Night shift summary" handoff entry (landed / blocked / next), set AGENT-STATUS idle. Leave the tree clean.

## Guardrails
- Never edit the other agent's claimed files; pull before every commit; no force-push.
- Evidence: UI = committed capture opened and inspected; engine/netcode = log lines/two-process run; server = syntax +
  endpoint + head.
- Stop conditions: queue empty, two failed attempts on one item (record and move on), or a decision only Tyler can make
  -> BLOCKED (Tyler) + next item.
- Pending Tyler tests (do not mark done without him): controller presets, Firefight voice preview, reverse-preset naming,
  sensitivity baseline.
