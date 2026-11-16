# NOTE — Reach Renaissance v0.1.50 reference study (large-session existence proof)

Date: 2026-11-16 (ET). Author: rr-ref (subagent), shared task task-3. Noble baseline: branch wip-merge, HEAD 4bfe827, version 0.19.103 (no release).
**Status: static, read-only study only. RR was never launched. No RR code, data, assets or strings were copied into Noble; nothing here is a ship recommendation.**

Purpose: Reach Renaissance (RR) is a separate large-player Reach project whose install reportedly hosts up to 88 players. We use it **only** as an existence proof and as a UX/config reference for milestone 3 (32 then 64/128) and for lobby honesty (real player counts, MAX PLAYERS picker). Everything in section 7 is marked INFERENCE; every other claim cites the file/lines/strings it came from.

Path correction: the brief said C:\Users\tyler\Desktop\ReachRenaissance - v0.1.50. The real install is C:\Users\tyler\Desktop\ReachRenaissance (no version suffix in the folder name); version lives in VERSION.txt.

---

## 0. Scope and method

- **Never ran RR.** No host-probe.exe, rr-master.exe, rr-browser.exe, menu-overlay.exe, rr-voice.exe or Reach Renaissance.exe was started. No RR port was touched (RR uses 40011-40013 and 11700-11710 over UDP/TCP, the same range family as our rig's traffic).
- Read: VERSION.txt, README.txt, data\strings_en.json, the WebView2 menu sources in themes\classic\, the shipped .exe/.dll bytes (ASCII+UTF-16 strings extracted with a throwaway PowerShell script; no disassembler on RR), the two release zips in the forensic archive (read from zip central directory without extracting), and **Tyler's own pre-existing RR run artifacts** under %LOCALAPPDATA%\ReachRenaissance\runs\20261116-*\ (his runs, 09:29-09:38 ET today - read-only, not produced by me).
- The Ghidra MCP stays on haloreach-copy.dll; I used it only to check that engine addresses RR's logs mention exist in **our** copy (section 6). Nothing was switched.
- Budget spent: ~75 min static work.

---

## 1. Version / build identity and what the install layout tells us

C:\Users\tyler\Desktop\ReachRenaissance\VERSION.txt (68 bytes):

    Reach Renaissance 0.1.50, commit c9c8e3e7b, built 2026-11-16T00:01Z

Launcher (root) and the extracted forensic copy are **byte-identical**, which ties the archive to this install family:

| File | Size | SHA-256 |
|---|---|---|
| Reach Renaissance.exe (install root) | 2,773,272 | 2f665790127131b8df28f6b94d8f4f63fddfaeb5d2661ea49daa8a50976a16c7 |
| build\rr-forensics\Reach Renaissance.exe | 2,773,272 | 2f665790127131b8df28f6b94d8f4f63fddfaeb5d2661ea49daa8a50976a16c7 |

bin\ (85,980,485 B total):

| File | Size | SHA-256 |
|---|---|---|
| bin\host-probe.exe | 23,217,736 | 312afe895076b3f2c19be642e46a59732c22860126e91f01934eee2023d1a57c |
| bin\menu-overlay.exe | 15,837,763 | 979877603c84dec621aba6bb6d59729d8393820f58e982d27d97d4f56f2ecfc8 |
| bin\rr-browser.exe | 12,393,526 | 7c4b8b0f4e1e32588fbb945f655b3855c767f9875e4034dab8b8df1e57ef78e9 |
| bin\rr-master.exe | 12,508,964 | d1257509849d0e7f2011360ac42db710aed2c4b6ccc1f13c5b0fd8e03d2677c0 |
| bin\rr-voice.exe | 11,155,271 | 4da226cdb627fa1f63b573d1407b04c43e1e9a926ac8d73722ace3c743843cca |
| bin\WebView2Loader.dll | 194,912 | 3426dcc55fdfb8b5e7ac623cf33b4ccf283fbf68a0c5a65bddfdb4d749a8abee |
| VERSION.txt | 68 | de9a5776803f42529b291789bfdd9982ce0b3588eedfe10f29e99fa7fd87cfc0 |

Release zips in D:\Project-Noble-Closed-Source\build\rr-forensics\: ReachRenaissance-v0.1.23.zip (445,562,760 B, 2026-10-24) and ReachRenaissance-v0.1.45.zip (731,688,667 B, 2026-11-14). The v0.1.45 zip's own ReachRenaissance/VERSION.txt, read from inside the zip: "Reach Renaissance 0.1.45, commit 5a3dedbd4, built 2026-11-13T21:50Z".

Install tree (dir sizes): bin\ 86 MB, data\ 818 MB, themes\ 118 MB, licenses\ 4,747 B -> **~1.02 GB total**, almost all game data reassembly:

- data\menumaps\mainmenu_all.map 298,520,576 B + mainmenu_all/armory/armory_elite.string_ids.csv (~1.4 MB each) - a rebuilt **main menu map**. The Xbox-360 menus are still Reach's own scene, but RR ships its own menu map and string-id tables.
- data\blocks\campaign_178..189*.bin, firefight_210..219*.bin, launch_block_1.bin, lb_m_c_1.bin - 180,016 B launch blocks per mode/mission (matched by --launch-block in the args, section 2).
- data\armor\rr_elite_pack.map 103,829,504 B (+ .rremotes/.rrneeds/.rrpmcg/.rrreloc sidecars) and data\forge\rr_forge_pack.map 373,612,544 B - extra maps loaded via --extra-maps / --armor-pack.
- data\strings_en.json - 71 localized strings, the only UTF-8 JSON data file directly in data\.
- themes\classic\ - the actual UI: 219 KB index.html, ~70 .js files (start-menu.js 366 KB, gamesetup.js 172 KB, social.js 164 KB, changelog.js 57 KB, browser.js 54 KB, waiting-room.js 11 KB, ...), fonts/sounds/map art, and an 18 MB video\intro.mp4.
- licenses\ - vgmstream, rr-voice, WebView2 notices.

**Architecture read (from layout + embedded source names, not speculation):** RR is several Rust programs plus a WebView2 menu overlay. Source paths are embedded in panic/log strings: src\m128.rs, src\p64_session.rs, src\mvar.rs, src\main.rs, announce.rs, menu-overlay ... src\client\directory\src\server_admin\content.rs, plus "(client/menus-prototype app.js + live-directory.js)" in themes\classic\browser.js lines 5-6. Serde strings such as "struct Listing with 19 elements" confirm Rust structs for the listing wire schema. The menus are HTML drawn by menu-overlay.exe; it posts commands like max-players: / host-profile: over WebView2 postMessage (themes\classic\index.html lines 1945-1953, 2065-2068).

**Version history (from themes\classic\changelog.js, newest first):**

| Version | Date | Session-size fact (quoted) |
|---|---|---|
| 0.1.50 | 2026-11-15 | current: 64->81 lobby picker, AI Limit 128/192 rows (changelog.js lines 2-5, 31, 59) |
| 0.1.43 | 2026-11-13 | "Parties hold up to 81 players." (line 50) |
| 0.1.42 | 2026-11-11 | "Experimental for server hosts: big_lobby=81 in server.ini allows up to 80 players on a dedicated server. It hasn't been played with a full lobby yet..." (line 72) |
| 0.1.39 | 2026-11-08 | "Games over 12 players: the host sends far-away players' movement less often..."; "Games over 16 players: scoreboard updates go out only when they change..." (lines 103-104) |
| 0.1.34 | 2026-11-06 | "Custom Games: host up to 128 players in the experimental M128 profile. Max Players goes up to 128."; "128-player lobbies: ... two players able to share each of up to 64 machines."; "Hosting above 64 players uses the 128-player engine profile; the stock 64-player path is unchanged." (lines 156-163) |
| 0.1.27 | 2026-10-29 | "Big lobbies: players past 16 get in and stay in..." (line 220); "Max Players in each hosting lobby (up to 64 in Custom Games)..." (line 228) |
| 0.1.23 | 2026-10-24 | "A Reach-style scoreboard that fits all 64 players, and scrolls." (line 267) |
| 0.1.20 | 2026-10-22 | "Up to 64 players in Multiplayer, and 16 in Firefight (32 as a host option)." (line 285) |
| early | - | "Host, join and run dedicated servers with up to 32 players..." (line 451) |

Cross-checked against the shipped zips: v0.1.23's themes\classic\index.html has "const MAX_PLAYERS = 64;" (line 1532), while v0.1.45's has the playerCounts(...) pickers and "const MAX_PLAYERS = 81;" (line 2147). So the shipped UI ceiling went **16 stock -> 64 (0.1.20/0.1.23) -> 128 experiment (0.1.34) -> 81 practical cap (0.1.43+)**, with 128-profile engine code kept but not exposed above 81.

---

## 2. Host / join / reconnect flow

### 2.1 README (README.txt, read-only install copy)

- "Join: Matchmaking's server list shows public games from everywhere and games on your home network. Pick one and join; no router setup needed." (lines 23-24)
- "Host: Custom Games, Network: Host on Server Browser (anyone can find your game, the default), Host Online (your network only) or Offline (just you). Pick map and game type, Start Game." (lines 25-26)
- "First time hosting: SETTINGS > Network > Allow Reach Renaissance (Windows asks for administrator permission) so other players can reach your PC." (lines 27-28)
- Ports (README.txt lines 94-97): "allow TCP and UDP 40011-40013 (hosting) and 11700-11710 (games on your home network)"; "With a firewall on, also forward UDP and TCP 40011 on your router by hand: its automatic forwarding (UPnP) may not get through. Joining needs no setup."
- Logs live in %LOCALAPPDATA%\ReachRenaissance\runs - "the last 10 runs" (line 36).

### 2.2 Actual launch lines (Tyler's run %LOCALAPPDATA%\ReachRenaissance\runs\20261116-093404\)

host-args.txt (quote; first line is the working dir, then the flags):

    --armory-appearance ... --profile ... --strings ... --elite-looks --features-on 6e
    --seconds 86400 --user-id 0009541b9352df06 --user-name <name>
    --mcc D:\Steam\steamapps\common\Halo The Master Chief Collection
    --session-players --game-players --players-table 32
    --max-players 32 --max-machines 32
    --announce {master} --max-listed-players 32 --allow-insecure-http
    --net-lan --port 40011 --upnp --net-lan-addr --server-name {name}
    --map {map} --mode {mode} --time-limit {time}
    --extra-maps ...\data\armor --armor-pack rr_elite_pack --elite-armor-slots

- The {master}, {name}, {map}, {mode}, {time} in these files are **template placeholders the launcher fills at start** (the file is a template dump, not the final argv).
- join-args.txt: "--join {master} {id} --port 40012 --next-game ..." (guest's own port is 40012; --next-game carries rotation follow-through).
- menu-args.txt: "--menu-scene all --menu-scene-file ...\scene.txt --extra-maps ...\data\menumaps"; campaign-args.txt / firefight-args.txt / match-args.txt: "--launch-block ..." / "--exit-after-game" / "--mode {mode}".

### 2.3 Directory (master) protocol, from Tyler's host\host_probe_1794839564.log

- "directory: http://lan-010a:11700: listed as lan-010a:40011 (lease 120 s, heartbeat every 30 s)"
- "directory: https://2-29-30-1.sslip.io:11700: listed as ip-f96e:40011 (lease 120 s, heartbeat every 30 s)"
- "directory: server id 0deb8a36aa836adf on http://lan-010a:11700 (players join with --join http://lan-010a:11700 0deb8a36aa836adf)"
- "directory: registering this server's LAN address with the masters (RR_NET_LAN_ADDR); same-household players try it first"
- "upnp: asking the router to forward UDP and TCP 40011 (keeper process ...)"; upnp.log: "upnp: no Internet gateway answered (UPnP may be off on the router); forward UDP and TCP 40011 on the router by hand if players can't reach this game"
- On stop: "directory: the menus are stopping this game; taking it off the server lists" -> "listing withdrawn".
- **Two player-count sources, explicitly logged** (this is the honesty mechanism):
  - early: "directory: player count: the session's player list can't be read; counting this PC's player and the machines heard from in the last 15 s" (log line 23)
  - after the session exists: "directory: player count: from the session's player list" (log line 239)
- Network raised: "net: socket buffers receive 8192 KiB (was 64), send 8192 KiB (was 64), port-unreachable resets off"; "net: star on (host-only routing, the default); a guest that does not negotiate it reaches the host only".
- Listing schema (from rr-browser.exe strings): "protocol port name host_name kind map gametype players max_players max_ping password region client_version build dll_sha256 mock mods hide_address map_variant" - Rust "struct Listing with 19 elements". So **players and max_players are first-class fields the host announces**, not values the browser infers.
- Master limits (rr-master.exe strings): "Some lists are cut short: the server lists 512 of each, and 128 games of each playlist."

### 2.4 Reconnect / host migration / lobby persistence facts

- changelog.js line 168: "After Online Games: Stay Together now works: players land back in the host's party after a game." (0.1.33)
- line 212: "Players who joined a game follow the host into the next one again instead of leaving 30 seconds after the game ends, and a joining player waits 90 seconds for a slow host instead of 30."
- line 217: "Players who joined a game no longer get sent back to the main menu when it ends: their game crashed there in 0.1.27, and now follows the rotation into the next game again."
- line 407: "When the host leaves, players are back in their menus at once with 'The host left the game', instead of waiting on a black screen."
- host-probe.exe strings: "M81: ending the session because automatic host migration is unsupported"; "The host left the game, so you've gone your own way."
- rr-master.exe/menu-overlay.exe contain a Rotation struct field list that includes reconnect: "struct Rotation with 7 elements" / "Rotation current games more reconnect". RR has a per-rotation reconnect concept; exact semantics UNCERTAIN because we did not run it.

---

## 3. Server Browser: layout, columns, and how real player counts are shown

All from themes\classic\browser.js (830 lines; the live server browser) and themes\classic\index.html.

- Purpose/lineage comment (browser.js lines 1-8): "Laid out like Reach's 360 lobby screens: title and bumper tabs, a list with the lit selection bar, a details panel styled like the party panel, button hints." ... "Data and rules are the original server browser's (client/menus-prototype app.js + live-directory.js): rows from rr-browser serve GET /api/servers (every master, each server's own signed status, ping, compatibility, favorites), the same filter tabs, favorites-then-ping order, the same status labels, and joins refused for incompatible, unanswering or full servers."
- **Columns** (browser.js line 107): const COLUMNS = [["name", "Game"], ["map", "Map"], ["mode", "Game Type"], ["xp", "XP", true], ["players", "Players", true], ["ping", "Ping", true]]; - default sort (line 110) "let sortBy = { key: "players", dir: 1 };" (most players first; ties by ping, lines 331-332).
- **Filters** (lines 22-32): All, Multiplayer, Forge, Campaign, Compatible, Ping < 100, "Has Players" (s.players > 0), "Not Full" (s.players < s.max).
- **Real counts are shown as "players / max_players"** - never a fixed /16:
  - row cell (line 509): s.players + "/" + s.max
  - details panel fact (line 570): fact("Players", s.players + " / " + s.max);
  - mapping (line 254): "players: l.players ?? 0, max: l.max_players ?? 0, ping: r.ping_ms, version: l.client_version, region: l.region,"
  - older in-page path (index.html line 1680) falls back honestly: (l.players ?? "?") + "/" + (l.max_players ?? "?").
- **Status labels / verdicts** (verdict(), browser.js lines 303-320), quoted: Closed, Not answering, Update needed, Older version, Different MCC build, Version mismatch, Full ("if (s.players >= s.max)"), "Room for N of your M" (party does not fit: line 314), Ping too high, Not heard lately (over NOT_HEARD=75 s, line 45), Ready.
- **Freshness honesty** (lines 40-48): "STALE = 30, DROP = 120, NOT_HEARD = 75"; "WATCHDOG = 20000, BACKOFF = [3000, 6000, 12000, 15000], EVERY = 15000". Rows over 30 s are dimmed, over 120 s gone; the count line always carries the list's age (lines 533-541): "N of M games", "updated N s ago" / "list from N s ago".
- **Join refusals reach the browser as text** (index.html line 1698 "else if (text.startsWith("join-error:")) Browser.joinFailed(text.slice(11));", and browser.js joinFailed() lines 822-831 remove the row for 3 min when the text matches /no longer listed|isn't taking players|has closed|no answer/i).
- **Host option disclosure via the version string** (browser.js lines 571-572): if ((s.version || "").endsWith("+rr-m64-v1")) fact("Host Option", "64-player Multiplayer (test)", "", true); and +rr-ff32-v1 -> "32-player Firefight (test)". So the *browser learns the host's large-session profile from the advertised client version suffix* (section 5.4 for the full suffix table).
- **Server-side validation**: rr-master.exe strings contain "players is greater than max_players" and "#max_players must be from 1 through ?" (the bound is a runtime-formatted number we could not recover statically - UNCERTAIN; plausibly 81 or 128).

**Answer to "how do they present X/16 vs X/88?":** they do **not** hardcode either. The host announces max_players; the browser prints "players / max_players" and computes Full/Room-for-N from those two numbers. A 16-player stock host shows 7/16; an M64 host shows 7/64; an 81-cap host shows 31/81. **There is no 88 anywhere in the install** (section 5.6).

---

## 4. Lobby / session-setup UX for large counts

### 4.1 The MAX PLAYERS picker is a plain 1..N stepper

themes\classic\index.html lines 644-647 (quote):

    // A lobby's Max Players choices, 1 to what its game takes (owner, 2026-10-28: "lobby playercounts and game
    // playercounts should be adjustable to whatever number the host desires"). 81 is the most the M81 profile
    // seats, one player per copy of the game (host-probe m81.rs); other profiles cap their own rows.
    function playerCounts(most) { return Array.from({ length: most }, (_, i) => String(i + 1)); }

So the picker is **every integer 1..cap**, no presets, no 16/32/64 buckets. Defaults are the "pick" index: Campaign 4, Firefight 16, Custom Games 32, Forge 32.

| Lobby | Row (same index.html) | Range | Default | Quoted help text |
|---|---|---|---|---|
| Campaign | line 687 | playerCounts(4) | 4 | "The most players your hosted mission takes, you included (Campaign takes up to 4)." |
| Firefight | line 697 | playerCounts(32) | 16 | "Over 16 hosts the experimental big Firefight (expanded AI storage, normal wave counts)." |
| Custom Games | line 709 | playerCounts(81) | 32 | "Over 32 hosts the experimental 64-player lobby; over 64 hosts the experimental 81-player lobby (80 on a Dedicated Server; if the host leaves, the game ends). ..." |
| Forge | line 727 | playerCounts(32) | 32 | "The most players your hosted Forge session takes, you included (Forge takes up to 32)." |
| Party size (leader) | lines 2153-2159 | 2..81 | - | "const MAX_PLAYERS = 81;" "window.MAX_PARTY_PLAYERS = MAX_PLAYERS;"; comment: "Party size: what a party holds (directory social.rs MAX_PARTY) ... It was 32 ... 81 since 2026-11-13, the M81 lobby's machines (owner: 'we don't have 128 player capabilites ... the limit should be 81'); it was 128." |

### 4.2 The cap chooses a named engine profile

index.html lines 2065-2068 (Custom Games / Dedicated path):

    // (Max Players: over 32 is the 64-player profile and over 64 M81, not in Forge; the overlay holds the game to the count)
    const most = preparedScenario ? Math.min(16, +pickOf(lobby, "Max Players") || 16) : +pickOf(lobby, "Max Players") || 0;
    post("max-players:" + most);
    post("host-profile:" + (lobby.forge && (preparedScenario || window.FIREFIGHT_AUTHORING) ? "authoring" : !lobby.forge && most > 64 ? "m81" : !lobby.forge && most > 32 ? "m64" : "default"));

Firefight path (lines 1945-1948): "over 16 in Firefight is the big Firefight profile; the overlay holds the game to the count" -> kind === "firefight" && most > 16 ? "ff32" : "default"; Firefight authoring swaps to authoring128/authoring192 from the AI Limit row (96/128/192).

So **the host UI's MAX PLAYERS number is what decides which engine profile is installed**, and the overlay "holds the game to the count". The number is not cosmetic.

### 4.3 Teams and per-team counts

- Team list is Reach's own 8: data\strings_en.json has "Red Team", "Blue Team", "Green Team", "Orange Team", "Purple Team", "Gold Team", "Brown Team", "Pink Team"; live scoreboard.json carries "team_scores":[0,0,0,0,0,0,0,0] (8 entries).
- changelog.js line 157: "128-player lobbies: the party, scoreboard and team menus handle all 128 players, with two players able to share each of up to 64 machines."; line 158: "... team choices past slot 64." So they **scaled players per team and per machine, not the number of teams**. No per-team-size config row was found.
- elite-data.js line 1529 shows a value literally named "M64" in the Elite armor data - coincidental token, not a lobby constant (do not use).

### 4.4 The size profiles RR actually ships

| Profile | Where it comes from | Player capacity | Machine rule |
|---|---|---|---|
| default | stock Reach path | stock 16 (up to 32 for MP/Forge/FF host) | stock |
| m64 | --m64-profile (host-probe.exe); browser tag +rr-m64-v1 | up to 64 | experiments with P64 groups |
| m81 | --m81-capacity; browser tag +rr-m81-v1 | 81 seats, one player per copy of the game | 81 machines |
| m128 | --m128-capacity; browser tag +rr-m128-v1 | 128, two players per machine | 64 machines |
| ff32 | --ff32-profile; tag +rr-ff32-v1 | 32 Firefight | - |

Current UI exposes only up to **81** (0.1.43+); 128 stays as engine code/profile strings. index.html line 645 says why 81 is "the most the M81 profile seats, one player per copy of the game".

### 4.5 Lobby honesty mechanics found

- The listing/lobby always shows "players / max_players" (index.html line 1680, browser.js lines 509/570) with "?" when unknown - no invented numbers.
- waiting-room.js (lines 122-123): "The host's count also has players the masters don't name (no presence, or a second player on one PC): the rest, unnamed." - named players are listed, the remainder is shown as a count.
- changelog.js line 92 (0.1.41): "Lock Game in the host's game menu: while it's on, no new players can join. Players already in the game stay, and the host can unlock it at any time."
- Campaign lobby help (index.html line 686): "Players who join in the first 45 seconds play the mission with you (up to Max Players)."
- changelog.js line 407: host-leave messaging ("The host left the game") and line 220 big-lobby join fix ("players past 16 get in and stay in").

---

## 5. Binary strings that reveal their session-size model, refusals, and patch architecture

Method: ASCII + UTF-16 runs >=5-6 chars from the shipped binaries with a throwaway PS script, keyword-filtered. Quoted below are the exact strings/fragments (concatenated where the compiler merged literals).

### 5.1 Host flags (host-probe.exe, mirrored in menu-overlay.exe)

    --max-players --players-table --max-listed-players --max-machines
    --session-players --game-players --lazy-commit
    --m64-profile --ff32-profile --m81-capacity --m128-capacity --firefight-authoring

Literal block also found: "--max-machines81--players-table128RR_PBRR_PB64RR_P64_GROUPSABCDEFRR_M64_GROUPSJRPMUOVTRR_FF_PLAYERSRR_AI_GUARD" -> strongly suggests **m81 = --players-table 128 + --max-machines 81** (i.e. 81 seats inside the 128-player engine profile). UNCERTAIN on the exact mapping of "RR_M64_GROUPSJRPMUOVT" (group letters).

### 5.2 Hard capacity rules printed by host-probe.exe (quoted)

- "--max-machines must be 1..64, or81 with --m81-capacity"
- "33..64 machines require --players-table 64|128 --session-players --game-players, RR_PB=1 RR_PB64=1 and every P64/M64 group"
- "81 machines require --m8..." (truncated by the binary's literal layout)
- "p64: Stage 4 off (the membership has 32 slots without RR_PB=1 RR_PB64=1)"
- "--max-machines 64 needs Stage 4 (--player..." (truncated)
- "--m64-profile is for multiplayer; use --ff32-profile for Firefight" / "FF32 host requires a full Firefight launch block (game mode 5)"
- "the session machine table has no room for 64 entries after the moved sent copy"
- "the machine table has no room for ? machines in the session extension" / "machine table: no room in the extension area"
- "capacity overflow"
- "server config: big_lobby must be off, 64 or 81; firefight_32 must be off or on"
- "session local player count (slot 32+)" (source ref "src\p64_session.rs")
- "object profile: stock 2,048 slots (set RR_OBJECT_SLOTS=4096 for the local 4,640 diagnostic)" (Tyler's host_probe log line 96 - **object-slot limit is a separate large-lobby bottleneck**)
- "small player limits: bytes at +?"

### 5.3 Refusal / close reasons (quoted)

- "? reason=no-compatible-free-seat source=?" (join refusal reason token)
- "join refused: session ?"
- "Native seating refused ?"
- "mainjoin_refused" (state token; nearby: "artifacts/RUNTIME-003", "Player", "--armor-pack")
- "requested lobby profile differs from the host" / "unknown lobby profile; update to join"
- "players is greater than max_players" / "#max_players must be from 1 through ?" (master validation)
- "M81: ending the session because automatic host migration is unsupported"
- "The host left the game, so you've gone your own way."
- "this server takes players through the relay only"
- "The master refused this listing (?); trying again won't fix that, and the listing drops off that list when its lease expires"
- rr-voice: "struct variant MatchMsg::Refused with 2 elements"

### 5.4 The profile-suffix / capability table (rr-master.exe, rr-browser.exe, menu-overlay.exe)

Exactly one is appended to the advertised client version, per the error string "select exactly one lobby profile, once":

    +mf2-  +rr-dff-v1  +rr-m64-v1  +rr-ff32-v1  +rr-m128-v1  +rr-m81-v1
    +rr-fa-v2  +rr-fh-v2  +rr-f192-v1

Interpretation (fa = Firefight authoring, fh = Forge host, f192 = Firefight AI 192) is INFERENCE from context; mf2 UNCERTAIN. The browser only renders two of them as visible facts (+rr-m64-v1, +rr-ff32-v1, section 3) - the rest exist but are not surfaced in the v0.1.50 UI.

### 5.5 Architecture: self-checking "capacity" hooks over named engine RVAs

host-probe.exe strings:

- "capacity::engine_event_send", "capacity::megalo_message"
- "the call at +0x109c78 does not lead to capacity::engine_event_send"
- "FUN_1801f1c8c is not hooked by capacity::megalo_message"

So RR's host patch is a **Rust capacity module** that (a) hooks engine functions, (b) carries **self-checks that each expected call site is hooked**, naming engine RVAs as +0x... offsets and Ghidra-style FUN_180... names. This is architecturally close to Noble's src\host\Slot*.cpp/.inc numbered-slot approach: RR logs "host slot N first call", "iface2 slot N first call" for slots 0-118 (e.g. host slot 47 theater, slot 48 path requests, slot 23 load phase, slot 43 network poll, slot 52 armor pack, slots 57/61-67 text, slot 116 controller map). Those slot numbers are their internal hook IDs, not engine values.

Environment switches seen in strings (developer/experiment gating, same idea as our NOBLE_* overrides): RR_PB, RR_PB64, RR_P64_GROUPS, RR_M64_GROUPS, RR_FF_PLAYERS, RR_AI_GUARD, RR_OBJECTS_POOL, RR_OBJECT_SLOTS, RR_NET_LAN_ADDR, RR_TEST_SPAWN_SURVIVAL, RR_TEST_NAVIGATION_MARKERS, RR_NO_GEX, RR_NO_VIEW, RR_NO_MAP, RR_NO_OBS, RR_NO_REPL, RR_NO_GM, RR_NO_ACTION, RR_NO_HUDX.

Embedded source-module names (panic paths): src\m128.rs, src\p64_session.rs, src\mvar.rs, src\main.rs, src\server_admin\content.rs, src\social_client.rs, plus announce.rs, gametype.rs, forgeff.rs, modmaps.rs referenced in Tyler's logs. **One Rust module per engine subsystem**, with named "profiles" (m64/m81/m128/ff32) selected by flags - this is the part worth studying conceptually.

### 5.6 What is NOT in the install: there is no 88

Searched the full install text/JS/strings and the forensic zips' index.html values: the session-size constants anywhere in RR are **4 / 16 / 32 / 64 / 81 / 128**, plus 192 (Firefight AI limit), 255 (Forge item limits), 256 (AI props), 2,048/4,096/4,640 (object slots). **"88" appears only as unrelated byte/hex values.** The reported "88 players" is, with high confidence, a misremembering of **81** (the M81 cap) - or a blend of 81 and 128. UNCERTAIN which source said 88; nothing in the install backs 88.

---

## 6. Cross-check against our haloreach-copy.dll (convergence evidence only)

RR's logs/strings name engine addresses as RVAs (+0x..., FUN_180...) of the same Halo Reach build. I checked a few against our Ghidra program (read-only, no program switch):

| RR reference | Our Ghidra result | Note |
|---|---|---|
| "FUN_1801f1c8c is not hooked by capacity::megalo_message" | FUN_1801f1c8c exists, body 0x1801f1c8c..0x1801f1ecf (580 B) | RR's "megalo_message" hook target is a real function in our copy |
| "the call at +0x109c78 does not lead to capacity::engine_event_send" | RVA 0x109c78 (not checked in depth) | candidate engine_event_send call site |
| net poll stack "0x43aae4 <- 0x43a3ea <- 0x43a7a1 <- 0x33436" and "0x43aae4 <- 0x423d36 <- 0x423679 <- 0x3aea27 <- 0x34361 <- 0x34ba4 <- 0xdc11 <- 0xdcbe" | 0x43aae4 -> FUN_18043aa78 (271 B); 0x423d36 -> FUN_180423cb4 (210 B); 0x423679 -> FUN_1804235e4 (644 B); 0x3aea27 -> FUN_1803ae9fc (73 B) | RR's "slot 43 network poll" chain resolves cleanly in our binary |
| - | Noble's known CONNECT REFUSED site hr+0x423E2E -> FUN_180423d88 (182 B) | sits immediately **after** RR's 0x423d36 chain entry - same small-function cluster (engine connection handling). Useful corroboration for where 32->64 membership changes are refused. |

This is **corroboration that both projects are looking at the same engine functions**, not a claim that any RR approach is correct or that we should copy it. RR's profile sizes and hard-coded limits must not be imported as facts about our build.

---

## 7. What we can borrow conceptually / what we must not copy

**INFERENCE - not fact. Nothing below is a shipping recommendation; it is a study of RR's UX choices for our own design discussion.**

### 7.1 Conceptually useful for Milestone 3 (32 -> 64/128) and lobby honesty

1. **players + max_players as first-class announced fields** (their Listing schema, 19 elements). Our lobby/browser should display the *host's actual selected cap* rather than the stock 16, so a 32/64/128 game reads 31/64, and compute Full/Room-for-N from those two numbers. This is the single most transferable idea and needs no RR code.
2. **Profile = explicit capability, chosen by the cap** (default / m64 / m81 / m128, with the host UI's number selecting the profile and the overlay "holding the game to the count"). For Noble: make the 32-player install a *named profile* (our pinned/extended work already is), make 64/128 a distinct profile with a visible label, and refuse mismatched joins up front ("requested lobby profile differs from the host") instead of failing mid-join.
3. **A plain 1..N MAX PLAYERS stepper with safe defaults** (16/32) and experimental sizes opt-in, plus user-facing help text that says what the switch does ("Over 32 hosts the experimental 64-player lobby..."). This is much more honest than a fixed 16/16.5/32 preset list.
4. **Two-source count with a logged reason** ("the session's player list can't be read; counting this PC's player and the machines heard from in the last 15 s" vs "from the session's player list"). Directly relevant to our known gap: Noble's roster/scoreboard is fed only from the master lobby, so LAN/direct sessions show 1 row (documented in NOTE-PLAYER-COUNTS-2026-11-15.md). RR's pattern - session table when readable, fallback count otherwise, and *say which* - is the honest shape we want.
5. **Freshness + refusal UX**: list age on the count line, dim >30 s / drop >120 s, "Not heard lately", "Full", "Room for N of your M", and a join error that removes the dead row. Small, independent, high-polish, and it makes big lobbies legible.
6. **Machine table vs player table split** (--players-table 32/64/128 vs --max-machines): 128 was reached via 2 players per machine x 64 machines; 81 via 1 player per copy x 81 machines. For our 64/128 plan this is a concrete question we must answer explicitly (do we support >1 local player per machine?), and it maps onto our peer/machine mask widening work.
7. **"Lock Game" and clean host-leave messages** ("The host left the game...") as honesty features when a lobby is at/near a cap or its host disappears.

### 7.2 What we must not copy

- **No RR code, data or assets**: not themes\classic\*.js, not data\menumaps\mainmenu_all.map, not the armor/forge packs, not strings_en.json, not any binary strings/logs as shipped content. Read for evidence only.
- **Do not derive our implementation from RR's binary strings or decompiled structure.** We did not decompile RR; even if we had, our patch should be designed from our own Ghidra findings on haloreach-copy.dll.
- **Do not adopt RR's profile names/suffixes, wire schema, ports, or server.ini keys** as our public API. Same ideas, our own naming.
- **Do not claim or target 88**: RR's real numbers are 64 / 81 / 128, and RR itself says the 80-player dedicated path "hasn't been played with a full lobby yet" (changelog.js line 72). "88" is not an existence proof of anything.
- Do not present RR as endorsement, affiliation, or compatibility target; no public push or version bump follows from this note.

---

## 8. Uncertainties / open questions

- **What "88" referred to** - no 88 anywhere in the install. UNCERTAIN; likely a misreading of 81/128.
- **Whether an 81-player lobby has actually been played to a full count** - RR's own changelog admits the 80-player dedicated path "hasn't been played with a full lobby yet"; the install itself proves the *feature exists*, not that 81 is stable. Treat "RR runs 88 players" as unverified at any count above 64.
- **Exact bounds**: "#max_players must be from 1 through ?" - the numeric bound is runtime-formatted and not recoverable from strings (UNCERTAIN; plausibly 81 or 128). Truncated flags ("81 machines require --m8...", "--max-machines 64 needs Stage 4 (--player...") are literal-layout truncations.
- **RR_M64_GROUPSJRPMUOVT** letters - likely 8 group identifiers; not interpreted (UNCERTAIN).
- **RVA observations are from RR's logs of the same Reach build**, i.e. they may shift with an MCC update; only the Ghidra-confirmed FUN_1801f1c8c / FUN_180423* correspondence was checked against our copy today.
- **We did not run RR** (hard rule), so all runtime claims are from Tyler's existing logs; behavior under 64/81 players was not observed.

---

## Appendix - evidence index

| Claim area | Path / file |
|---|---|
| Version, build, hashes | C:\Users\tyler\Desktop\ReachRenaissance\VERSION.txt; bin\*.exe (SHA-256 table section 1); D:\Project-Noble-Closed-Source\build\rr-forensics\Reach Renaissance.exe |
| Host/join flow, ports | README.txt lines 22-30, 92-99 |
| Actual launch flags | %LOCALAPPDATA%\ReachRenaissance\runs\20261116-093404\{host,join,menu,match,campaign,firefight}-args.txt |
| Directory/masters, listing lifecycle, player-count source | ...\runs\20261116-093036\host\host_probe_1794839564.log (441 lines; lines 4-8, 16-18, 23, 239, 437-441); host\upnp.log; host\listed |
| Live session artifacts | ...\runs\20261116-093036\host\cwd\{scoreboard,match-chat,match-voice-session,match-chat-session}.json |
| Server browser rules | themes\classic\browser.js (lines 1-48, 107-110, 254, 303-320, 495-541, 570-572, 800-831) |
| Lobby pickers, profiles, party cap | themes\classic\index.html (lines 644-647, 680-730, 738-743, 1660-1690, 1944-1953, 2065-2068, 2145-2159); themes\classic\waiting-room.js lines 92-123 |
| Version history of size caps | themes\classic\changelog.js (lines 2, 31, 50, 70-72, 92, 103-104, 155-165, 220-228, 267, 284-285, 407, 451) |
| Old shipped configs | build\rr-forensics\ReachRenaissance-v0.1.23.zip -> MAX_PLAYERS=64; ...-v0.1.45.zip -> MAX_PLAYERS=81, playerCounts(4/32/81/32) |
| Host flags, capacity rules, refusal strings, patch module names, env switches | strings extracted from bin\host-probe.exe (307,326 unique runs >=6 chars), bin\rr-master.exe, bin\rr-browser.exe, bin\menu-overlay.exe, bin\rr-voice.exe |
| Ghidra cross-check | haloreach-copy.dll (image base 0x180000000): FUN_1801f1c8c, FUN_18043aa78, FUN_180423cb4, FUN_1804235e4, FUN_1803ae9fc, FUN_180423d88 |
| RR rules observed | RR never launched; no RR program loaded in Ghidra; install untouched (read-only); no files modified outside this note |
