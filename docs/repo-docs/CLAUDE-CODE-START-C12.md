# Claude Code start: C12-L (32 players)

Paste into a local Claude Code session opened in `D:\Project-Noble-Closed-Source`:

> Continue Noble engine limits, task C12-L (32 players). Read `LIMITS-HANDOFF.md` (status section),
> `LIMITS-TODO.md` ("Next session" + C7e results), `docs/limits-players-audit.md`,
> `docs/limits-players-bounds-review.md`, `docs/limits-players-sites.json`, `docs/limits-players-rest.json`,
> `docs/limits-players-bounds.json`, and `src/host/SlotPatch.*` + `tools/limits/*.py`. Then, in order:
> 1. The 11 pointer-stepping loop functions: add step caves (stepping past the 16-slot array runs into scalar
>    fields, so each loop needs a cave, not just a bound change).
> 2. Bounds pass: 111 bit-scans, 31 C1 and 8 C4 widenings (C2 5 / C3 16 are covered by the review doc).
> 3. C3 tables: grow stack frames 5/7; shadow 8/10/31; heap objects 32/38/45/47/51-57.
> 4. Pools to 32 and the host player list.
> 5. Build with `build-dev.bat` (noble-dev.exe), then a three-window LAN test (host + 2 clients on one PC).
> All patches must be all-or-nothing byte-checked like SlotPatch, Extended/Full profile only, opt-in flag first.

## Rules (from Tyler)
- Never modify the players' installed haloreach.dll (Noble patches a copy at launch).
- Never run `build.bat` (it publishes a release). Use `build-dev.bat`.
- Never push without Tyler's OK. Never edit `D:\Project-Noble-ORIGINAL`.
- `recipes/` and `.nlp` files contain Reach code: never commit them.
- Commit messages end with:
  `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`
  `Claude-Session: https://claude.ai/code/session_01SzNr7z37j7jNFPDtqF65hh`

## Parallel work with the cloud Claude session
The cloud session can't run commands on this PC (it can only read and write files and click in apps), so the local
Claude Code session should own code edits, Python tools and builds for C12-L. The cloud session can review diffs,
read logs, and drive tests on monitor 1. Don't edit the same file from both at once; commit before handing over.

Swarm zombie test is parked; notes in `docs/swarm-test-notes.md`.
