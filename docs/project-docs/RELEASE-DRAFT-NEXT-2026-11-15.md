> STATUS: PUBLISHED as v0.19.103 on 2026-11-15 (commit eea9868). This draft is the pre-release working copy; the\n> shipped notes are release-notes/v0.19.103.md.\n\n# Draft release notes - next release after v0.19.102 (prepared 2026-11-15, NOT published)

Held until Tyler says go. When approved: bump version in CMakeLists/Version.hpp/BuildIdentity.hpp/noble.rc, copy this into
`release-notes/vX.md`, add a ChangelogData entry, build both gates, run `tools\publish-release.ps1`, verify assets.

## Map pickers
- Expanded base-map/variant drawer now has a variant **search** box, a **category cycler** (ALL MODES + forge/game-mode
  categories) and an `N OF M VARIANTS` count. Forge World AI Battlegrounds (353 variants) filters to 3 with "jump" and 11
  with BIG TEAM.
- Collapsed MODDED sections per mod, click/Accept to expand, FOLD on the heading to collapse again; banner polish.

## Campaign / lobbies
- Campaign **MAX PLAYERS** row (4/8/12/16) wired into HOST CO-OP.
- Campaign search box clamped; drawer header ASCII "-" instead of a missing glyph.

## Server browser
- Refresh now shows the "Refreshing server list..." status (it refreshed silently before).
- FAVOURITE / COPY INVITE pinned in a fixed footer under the details pane, always visible/clickable; details compacted
  so the primary JOIN / DOWNLOAD MOD & JOIN stays visible.
- The server-browser style-2 blue-glass layout is the default (style switch remains dev-only).

## Lobby share codes (new)
- Hosts of **public and private** lobbies get a shareable code: `NB1-XXXX-XXXX-XXXX-XXXX` (session id in Crockford
  Base32 with CRC). Copy from the lobby HOSTING panel; join from the **Main Menu > Join with Code**, the Server Browser
  **JOIN WITH CODE** button, or the Social screen. Private lobbies stay unlisted but the code works.

## Settings / input
- Game Options camera sliders share one start/length/value column; long labels ellipsised.
- Saving FOV in the front-end no longer zooms the Armory; match FOV applies at match start.
- Controller **toggle crouch** and M&K **toggle zoom** options (SETTINGS > CONTROLS > KEYBOARD & MOUSE).
- Game Options rows are clickable anywhere on the row; the main-menu "N What's New" hint is clickable.

## Visor colours
- Extended Armory visor finishes no longer render white in game: every finish maps to a valid stock engine slot
  (nearest colour) until the per-player exact-colour work lands. Exact custom RGB still shows in the Armory only.

## Known limitations / not in this release
- Exact **custom visor colour in matches** (per-player) - RE in progress (`RE-VISOR-COLOR-2026-11-15.md`).
- **Colour-variant armour effects** (blue flames etc.) - recolour proof done; selectable entries need engine effect-table
  work (`RE-ARMOR-FX-COLORS-2026-11-15.md`).
- Multiplayer >16 players - on hold per Tyler.

## QA status
See `QA-VERIFY-2026-11-15.md`; fresh captures for the server browser, variant drawer, game options, toggles and the
join-code UI are current at `64e6d22`. Still needs Tyler's hand check for the campaign skulls row and a real two-PC
lobby-code join test.
