# Project Noble: handoff (2026-10-28, 14:50 ET)

This picks up a reverse-engineering and C++ effort on **Tyler's Windows PC**. All paths below are real paths on that machine.
Tyler (GitHub `WeabooTyler117`) works with a partner (GitHub `O7OF`) who owns the repo and writes most of the code.
Tyler mostly runs builds, tests, and does the reverse engineering; he is not a C++ developer, so give him exact commands
(one command per code block, so each can be pasted on its own).

**Goal:** `noble-client.exe` loads MCC's own `haloreach.dll` (Halo: Reach) and drives it **without MCC**, like the existing
community project Reach Renaissance (RR, installed at `D:\Reach`) does. Ownership is verified through Steam, nothing in the
MCC install is modified, and no RR code or data is redistributed. RR is used only as a reference for behaviour.

---

## 0. Pick up here (2026-10-28, 14:50 ET) - post-game "exit experience" (spec step 7)

**Status:** on Tyler's PC, syntax-checked; `job.bat` builds, commits and pushes. Needs a real game to test (the screen
only appears after a match ends): Tyler plays a short game and sends a screenshot.

- `PauseMenu.cpp` `DrawGameOver` rewritten as a full-screen Reach-style results screen: header (GAME OVER, map, mode, minutes,
  difficulty icon), left = SCOREBOARD (local row highlighted with rank icon) + PROGRESS (challenge / commendation bars this
  game moved: old part blue-grey, this game's part amber, green COMPLETE / "<TIER> REACHED", commendation banner art),
  right = CREDITS (lines appear every 0.5 s and count up with `Totals`; "Random bonus"/"Jackpot!" roll like a slot machine with
  `SlotMachine`), CREDITS EARNED total, RANK (icon, bar fills from the old cR to the new; on promotion `RankUp`, amber glow,
  PROMOTED). `GameCompletion` plays when the screen opens. Enter / Space / a click away from the buttons skips to the end.
  Buttons bottom-right: PLAY AGAIN, RETURN TO MENU, QUIT (always usable). Sounds respect the Menu sounds setting.
  Keyboard is read with GetAsyncKeyState (the overlay only feeds the mouse into ImGui); no controller skip yet.
- `Career`: `Award` now has `progress` (`ProgressLine`: kind daily/weekly/commendation, mode, title, art, before, after,
  target, completed, tier) filled in `OnGameResults` for every challenge or commendation this match moved, and `difficulty`.
- Idea from Tyler for spec 4.14 (map voting), "not 1 for 1 but similar": a full-screen VOTE FOR THE NEXT GAME screen with
  the next game named at the top, a vote-closed ring with "N of M voted", 4 choice cards (map picture, map + game type, vote
  count badge, a bar for the share of votes, "PLAYLIST'S PICK" and "NEXT GAME" tags), a PLAYERS list on the right in
  team colours with each player's vote as a small map thumbnail, and "The next game starts once the results are over".
  Needs networking for the multiplayer side.

## 0. Pick up here (2026-10-28, 12:40 ET) - Reach art is now in git

- Tyler decided the Reach menu art goes into the repo so O7 sees exactly the same build (`3522203`, 106 PNGs, ~2 MB):
  `build/bin/Release/data/ui/reach/{ranks,difficulty,commendations}/` (force-added; `build\` is otherwise git-ignored).
  After a pull, O7's menus show the same rank icons, difficulty icons and commendation banners. New art goes in the same
  folders and is added with `git add -f build/bin/Release/data/ui/reach`.
- `8bad0ad`: Demon / Flawless Cowboy text says "(vanilla Noble Actual and Lone Wolf don't count)". Firefight "Coming later"
  label clipping dropped from the to-do (Firefight will be done before release).
- Claude raised that the repo is public and the art is Microsoft's (takedown risk); Tyler chose to push it anyway.

## 0. Pick up here (2026-10-28, 12:25 ET) - Reach's 45 commendations (spec step 5)

**Status:** built and pushed (`78a943e`, then `2241e59` with the Noble Actual / Lone Wolf block). `cm_tab2.bmp` checked:
mode tabs, banners, cR per tier and "Not tracked yet" rows look right.

- `Career.cpp`: `kCommendations` is now Reach's 45 (mode, art key, name, requirement, Reach's six tier counts from Halopedia).
  Tier names Steel, Bronze, Silver, Gold, Onyx, Max. Counts are kept per commendation in `record.commendationCounts`
  (`<mode>_<key>`), tiers paid in `record.commendations` (old Noble commendations are dropped). Tyler asked to drop
  Reach's rules (every difficulty counts).
- **Stock Noble Actual (`m05`) and Lone Wolf (`m70_bonus`) pay nothing** (`UnpaidMission` in OnGameResults): no cR lines at all,
  no challenge or commendation progress; the post-game shows one 0 cR line saying so. Tyler found Noble Actual paid 5,004 cR and
  completed a challenge after skipping the cutscene (0.3 min). Matched by scenario name, so modded maps with their own
  names still pay. Kills/deaths still go into lifetime stats.
- Each commendation pays Reach's own cR per tier (`CommendationDef::rewards`, from the Halopedia list Tyler sent; Max pays 0;
  Heavy Weapon was unknown, so it uses Halopedia's averages 200/700/1000/2400/2400). Rows show the next tier's cR in amber.
- Tyler's payout rule (`TierReward`): Steel..Gold pay Reach's cR x2.5 (rounded), Onyx pays 2x that Gold, Max pays 3x Onyx.
  Example Any Spree: 625, 1,063, 3,000, 6,000, Onyx 12,000, Max 36,000.
- Counted now: Demon (missions), Flawless Cowboy (mission with 0 deaths), Support Role / Backup / Assistant (assists),
  Perfectionist (Firefight game with 0 deaths, per game until rounds are decoded). Everything else shows "Not tracked yet".
- `CareerPanel.cpp` COMMENDATIONS tab: CAMPAIGN / FIREFIGHT / MULTIPLAYER sub-tabs (click, or LT/RT; `m_commendTab`), Reach-style rows with the banner art
  (`data\ui\reach\commendations\<mode>_<key>.png`) tinted by tier (Steel art only; Max gets an amber frame), name, tier,
  requirement and "x / next" bar.

### TO DO - commendation stats Noble can't read yet

These commendations stay at 0 until the stats are decoded from the end-of-game results record (or engine events):
- **Headshots:** Pinpoint, Dome Inspector, One Shot.
- **Weapon types** (precision, pistol, automatic, heavy, grenade, close quarters): Precisely, Longshot, Crack Shot,
  Small Arms, Trigger Happy, Sidearm, Standard Issue, Riflin' Through, Trigger Man, Splash Damage, Get Loud, Heavy Weapon,
  Nice Arm, Pull the Pin, Grenadier, Close Quarters.
- **Enemy classes** (infantry, leader, specialist): Cannon Fodder, Target Practice, Leadership Element, In Command, SpecOps, Specialized.
- **Multikills:** Walking Tank, Numbers Game, Multikill.
- **Sprees:** Super Soldier, Methodical, Any Spree.
- **Vehicles** (vehicle kills, vehicles destroyed, driving for a gunner): War Machine, Vehicular, Mobile Asset, Right of Way,
  Grounded, Downshift.
- **Medals / assassinations:** Jack of All Trades, Rear Admiral.
- **Firefight rounds:** Perfectionist should count rounds without dying, not games.
When a stat is decoded: add a `CommendationMetric`, set it on the rows above, and fill it in `CommendationAmount`.

## 0. Pick up here (2026-10-28, 12:10 ET) - commendation art in place (spec step 5)

- Tyler's art: `D:\Halo Commendations` (52 named webp/png). Copied as PNG to `build\bin\Release\data\ui\reach\commendations\`
  (now committed, see 12:40) as `<mode>_<name>.png`: 16 campaign (green), 15 firefight (red), 14 multiplayer (blue), all Steel tier,
  plus `example_*.png` (bronze sniper, silver technician, gold assassin, onyx stars, onyx max, steel assist, service record).
- Names/thresholds: halopedia "List of Halo: Reach commendations" (Steel/Iron, Bronze, Silver, Gold, Onyx, Max). Only Steel art
  exists, so other tiers need tinting in code (bronze/silver/gold/onyx tint; Max = onyx with flames per example_onyx_max).
- Full screen for Service Record/Games/Mods pushed as `bd002d9`; ranks capture confirmed the fixed order.
- Next: Commendations tab (Reach-style rows: banner, name, tier, progress x / next), tracking what Noble can read now
  (kills, assists, missions, no-death missions); the rest show 0 until headshots/multikills/weapons are decoded.

## 0. Pick up here (2026-10-28, 11:40 ET) - challenges pushed, panel screens full screen

- Challenges are committed and pushed as `c3b52f1`. `ch_tab1.bmp` checked: the cards, mode chips, countdowns and two columns look right.
- Tyler: version **stays 0.8.2** (patch bumps only, no change needed).
- Service Record, Games (server browser) and Mods now use the whole screen like Settings (`FrontEndMenu.cpp`: `full = true`).
  The job on the PC builds it, captures `fs_career1.bmp`, `fs_career0.bmp` and `fs_servers.bmp`, then commits and pushes.
  Waiting for Tyler to run `go.bat`.
- Rank art fixed per Tyler (files on his PC only, `data\ui\reach\ranks`): Major <- old Major G3, Major G1 <- old G2,
  Major G2 <- old G1 (by elimination), Major G3 <- old Major; Colonel G1 <- old G2, G2 <- old G3, G3 <- old G1;
  Mythic/Noble swapped; Forerunner/Reclaimer swapped. The job also captures `fs_career3.bmp` (RANKS tab).

## 0. Pick up here (2026-10-28, 11:35 ET) - Reach's real challenges (spec step 4)

**Status:** files are on Tyler's PC and verified; `build\agent\jobs\job.bat` builds, captures the Challenges tab
(`build\bin\Release\ch_tab1.bmp`), then commits and pushes. Waiting for Tyler to run `build\agent\go.bat`.
Results: `build\bin\Release\ch_done.txt` (DONE or BUILD FAILED), `build\nb_build_out.txt`, `build\git_out.txt`.

Tyler's choices: **original timing** (daily reset 11:00 GMT, weekly reset Monday 11:00 GMT, as in Reach) and a
**random pick from the whole pool** (7 daily, 5 weekly).

- `docs/career/challenges.json`: Reach's 2,103 daily/weekly challenges (2010-2012: kind, mode, name, requirement, cR).
- `tools/career/gen_reach_challenges.py` (run from the repo root) writes `src/career/ReachChallenges.inc`. It keeps only
  what Noble can track from a finished game: kills, assists, games, missions, best-game score, no-death games, mission
  time, difficulty, skulls, mission (m10..m70), "complete N daily challenges", "complete all dailies". 262 kept
  (228 daily, 34 weekly). Matchmaking wording is dropped. Left out: co-op, wins, waves, rounds, sets, Bungie.net/file share,
  and anything needing stats Noble can't read yet (headshots, weapons, medals, enemy types). Skull names map to MCC bits:
  Iron 18, Black Eye 3, Tough Luck 36, Catch 7, Cloud 13, Famine 11, Thunderstorm 34, Tilt 35, Mythic 23.
- `Career.cpp`: `ChallengeDay() = (unix time - 11 h) / 86400`, `ChallengeWeek() = (day - 4) / 7`; progress keys
  `D<day>:r<row>` / `W<week>:r<row>` (old date keys are pruned once). `PickChallenges` shuffles the pool with splitmix64
  seeded by the day/week (same set for everyone that day), taking 2 campaign, 2 Firefight, 2 multiplayer, 1 any for dailies
  and 2 campaign, 2 multiplayer, 1 any for weeklies, no repeated names, filling up from the rest. `ChallengeAmount` checks
  mode, mission (last part of the map path), difficulty (exact or "or harder"), skulls (all required), no deaths, time limit.
  Best-game challenges keep the max; others add up. Dailies are processed before weeklies so "complete N dailies" and
  "all dailies" count in the same game. New: `Challenge::mode`, `DailyResetText()`, `WeeklyResetText()`.
- `CareerPanel.cpp` CHALLENGES tab: DAILY / WEEKLY headers with done count and reset countdown, cards in two columns
  (one when narrow): mode chip, name, requirement, progress bar `x / y`, `+cR` in amber, green COMPLETE with a check.
- Next: check `ch_tab1.bmp`; then commendations (spec step 5, needs Tyler's commendation art), post-game, Armory v2.
  Still open: rank-order check (`rank_order_check.png`), version number (0.8.2, patch bumps only), Firefight
  "Coming later" label clipping.

## 0. Pick up here (2026-10-28, 11:30 ET) - Campaign / Firefight / Custom / Forge setup (spec step 3)

- Map and mission lists show a number and MCC's small map preview before each name (`ListItem::thumb/number`,
  rows 40-42 px, as many rows as fit above the hint line).
- Right side: `HeroBanner` (map picture, dark gradient, accent kicker "HALO: REACH Â· CAMPAIGN / FIREFIGHT / MULTIPLAYER /
  FORGE" or "MOD: <name>", big title, blurb under it), then the options, then `BigAction` buttons anchored above the
  hint row (fixes START MISSION covering the hints): Campaign/Firefight PLAY SOLO ("<mission> Â· <difficulty>",
  "Hold out alone Â· ...") + HOST CO-OP (greyed, "Needs online play"); Custom Game START GAME ("<map> Â· <game
  type>") + HOST OR JOIN (server browser); Forge START FORGE. The banner shrinks to fit the screen height.
- Campaign: the skulls that are on show as small skull icons next to the score multiplier.
- Firefight: "FIREFIGHT OPTIONS Â· DEFAULT" row shown greyed ("Coming later") until Noble can write Firefight variants.

## 0. Pick up here (2026-10-28, 11:15 ET) - Settings redesign (spec steps 1-2)

Tyler chose option A (build the spec in its order). Done:
- **UI kit** `src/ui/NobleUi.{hpp,cpp}` (spec §3, first pieces): colours, `Scale()`, `SectionHeader`, `RowBackground`
  (3 px accent edge + gradient focus, faint hover), `ChangedDot` (amber), `DrawSlider` (fill bar, default tick),
  `DrawCycle` (‹ value › with position dots, ends greyed when not wrapping), `DrawToggle` (ON/OFF pill), `DrawBox`,
  `Text`/`TextWidth`/`Paragraph`. They only draw; interaction lives in the screen so keys, pad and mouse share one path.
- **Settings in three columns** (`SettingsPanel` rewritten, same public API, same settings.json keys): left = search
  box, categories grouped DISPLAY & SOUND (Graphics, Audio) / GAME (Game options, Player) / CONTROLS (Keyboard & mouse,
  Controller) / ONLINE (Online services), each with a live summary and an amber dot when it has unapplied changes;
  bottom = "N UNSAVED CHANGES / In ..." with APPLY CHANGES / DISCARD, or a per-category RESTORE DEFAULTS (greyed when
  already default). Middle = rows (slider / cycle / toggle / key box / text / info), changed rows get a dot; search
  shows matching rows from every category. Right = details for the hovered (else focused) row: title, when it takes
  effect, description, DEFAULT, and previews (FOV cone vs Reach's 78, frame-time bar, volume meters, controller status
  + picture, graphics card from DXGI adapter 0). Keys/pad: Up/Down rows, Left/Right value, Enter activate, Q/E (LB/RB)
  category, Y apply, X discard. Rows that can't work yet are shown greyed with a reason (advanced graphics, mute in
  background, in-vehicle FOV, hide HUD, mouse acceleration, controller button layouts). `-menu-tab N` = category 0..6.
  Old menu-music values are normalised to `reach:menu` on load so they don't show as an unsaved change.
- **Full screen** (Tyler 11:09): the front-end SETTINGS screen uses the whole screen (dark backdrop, small title);
  the pause menu's SETTINGS window is the display size minus 16 px each side.
- Next per spec §6: campaign/Firefight setup polish (thumbnails, hero banner; difficulty tiles done), then challenges
  cards, commendations (Reach's 45), post-game exit experience, Armory v2.

## 0. Pick up here (2026-10-28, 11:00 ET) - menu redesign spec, rank + difficulty icons

**Menu redesign spec (Tyler 10:34):** `docs/menu-redesign-spec.md` ("Reclaimer layout, Reach soul"): the plan for
every front-end screen, the Armory, career screens and the features Reach never had. Read it before menu work; its
§6 gives the order (UI kit + three-column Settings first). Tyler has 43 Reclaimer reference screenshots: ask him when a
screen in the spec is unclear.

**Reach art (placeholders, never committed):** Tyler sent a sheet of Reach rank icons + the four difficulty icons.
Cut into one file each and copied to his PC only: `build\bin\Release\data\ui\reach\ranks\<NN>_<name>.png`
(NN = `Career::Ranks()` index 00 Recruit .. 49 Inheritor) and `...\reach\difficulty\{easy,normal,heroic,legendary}.png`
(git-ignored like all of `data\`). New `UiArt::ReachArt(category, stem)`, `RankIcon(index)`, `DifficultyIcon(d)`,
`DrawContain(...)` look in `data\ui\custom\reach\` then `data\ui\reach\`; everything falls back to text when a file
is missing. Rank order is partly a guess (Major = two stars, Lt. Colonel G3 = the slashed L, Warrant Officer grade
order, Mythic..Nova = moon, crescent, black hole, sun): Tyler checks `rank_order_check.png`; fixing one = renaming files.
- Campaign / Firefight: DIFFICULTY is four icon tiles (`DifficultyTiles` in FrontEndMenu.cpp; click or Left/Right).
- Party box: rank icon at the end of the player's row (career.json read at most every 2 s).
- Service Record: big rank icon in the header, progress "x / y cR", "Next: <icon> <rank> at N cR (M to go)";
  new **RANKS** tab (4th; Q/E cycle 4 tabs; `-menu-tab 3`): all 50 ranks, earned bright, current framed in blue, next
  outlined, cR under each.
- `-menu-screen` works (it only looked ignored before); the menu logs `FrontEndMenu: screen a -> b`.
- Note: the working copy of `src/ui/UiArt.cpp` on Tyler's PC had lost `NobleControllerImage` (an older copy had come
  back, the stale-file problem from §8): restored from `git show HEAD`. The test job now writes `build\git_status_r3.txt`
  (`git status` + `git diff --stat`) before building so stale files show up.

## 0. Pick up here (2026-10-28, 09:55 ET) - menu freeze fixed, clickable hints, quit confirmation

Also pushed since 09:15: `e950306` ODST-only Firefight voices removed (Armory lists Reach's 12; voice saved/loaded
by `voice_id`), `5808e5e` GRD helmet offers MCC's four variants (Base, Optrex, CBRN, Optrex/CBRN; helmets load by
`helmet_id`; tested: item 71 reaches the profile and a match runs). The menu changes below are committed with this
handoff (FrontEndMenu.cpp/.hpp, main.cpp); the quit box still needs a hand check (Esc on the main menu).

**Menu "freeze" (Tyler 09:26: Armory froze, Esc / Service Record tabs dead after Alt+Tab).** Not the Armory and not
Alt+Tab: in both of his runs the whole front-end menu stopped exactly ~60 s after start. The in-engine menu (O7OF's
mainmenu mode) asks the engine for a menu map MCC doesn't ship (RR's `mainmenu_all.map` is not used/shipped), so after
a minute the engine gives up (slot 3 with a1=0xA, then slot 4 `_map_state_none`) and stops calling slot 1, which is
where `FrontEndMenu::RenderOverlay` draws. Fix: `FrontEndMenu::HoldFrame()` (main thread, called from the message loop
in `main.cpp` next to `PauseMenu::HoldFrame`): once no slot-1 frame came for 1 s it clears the back buffer, draws the
menu itself and presents (mutex shared with RenderOverlay). Tested 09:48: log `engine stopped drawing the menu; Noble
draws it itself` at 09:48:06, menu kept running and `-menu-capture 5000` saved `m1_capture_5480.bmp` 30 s later.
After that minute the Armory has no 3D scene behind it. Note: `-menu-screen` is still ignored by the in-engine menu.

**Clickable button hints:** the ENTER Select / ESC Back hints at the bottom of every front-end screen are buttons now
(hover highlight, click = the key). Change / Tabs hints only describe keys and stay plain labels.

**Quit confirmation (Tyler 09:48):** Esc on the main menu, or choosing QUIT, opens "QUIT PROJECT NOBLE?" with YES / NO
(No preselected; arrows switch, Enter picks, Esc = No). The main-menu hint now reads "ESC Quit".

**Version number (Tyler 09:48):** "slow down with the version number, we'll hit 1.0 in no time." Currently 0.8.2
(`src/common/Version.hpp`). Waiting for Tyler's answer on the scheme before changing it; proposal: only bump the
patch number for normal changes, minor only for milestones Tyler/O7OF agree on, 1.0 reserved for a real release.

## 0. Pick up here (2026-10-28, 09:15 ET) - Armory appearance now reaches the game

**What was wrong:** Noble only wrote the Elite flag (+0x27/+0x38) into the profile; every armor field stayed 0,
and 0 means "item 0" of each slot (Mark V [B] helmet, default pieces, Steel, **Elite Minor**).

**How the profile carries appearance** (haloreach+0x2EDA08 copies it into the player's settings):
dword "global item ids" at +0x40 helmet, +0x44 left shoulder, +0x48 right shoulder, +0x4C chest, +0x50 wrist,
+0x54 utility, +0x64 body (192 male / 193 female), +0x68 Elite armor set, +0x6C knee guards, +0x70 visor colour,
+0x8C armor effect, +0x90 Elite armor effect, +0x94 Firefight voice, +0x98 primary colour, +0x9C secondary colour,
+0xA0 third colour (RR: Steel). (+0xA4/+0xA8 are the Armory poses, HR_Rifle_Salute / HR_Elite_Rifle_Salute in RR.)
A global item id is the item's position in **MCC's own Reach customization list**: the `HR_*` names stored in order
in `MCC-Win64-Shipping.exe` (HR_Helmet_MarkVB_Base = 0 ... Drengr_TYR = 89, LeftShoulder 90-107, RightShoulder
108-125, Chest 126-143, Wrist 144-149, Utility 150-156, Visor 157-161, Knees 162-165, ArmorEffect 166-172,
EliteArmorEffect 173-179, FirefightVoice 180-191, Spartan_Male/Female 192/193, Elite_Minor..Officer 194-201,
Color_Steel..Pale 202-231, later chests 232-239). `haloreach+0x3B2F4` turns it into the engine's slot index;
the group sizes match that function exactly.

**Code:** `ApplyArmoryAppearance` in `ReachHost.cpp` (replaces `ApplyArmorySpecies`) reads `data/appearance.json`
and writes all of the above; helmets and chests go by the Armory's `helmet_id` / `chest_id` string ids, the other
slots by index (the Armory lists are already in MCC order). `RefreshArmoryAppearance` re-applies it whenever the
file changes (checked twice a second), so an Armory save during a session reaches the profile.

**Tested on Tyler's PC (screenshots, he was away):**
- `-profile-set 98=E0` (Brick) turns the player's colour swatch on the scoreboard red; 27/38 = Elite gives the
  Elite HUD. So the colour fields reach the game.
- Built with the new code and two appearance.json files (Spartan EVA helmet / Brick, and Elite General / Blue):
  log `applied Armory appearance (Spartan, helmet 27, chest 133, Elite set 0, colours 22/0)` with a red swatch,
  and `(Elite, helmet 0, chest 126, Elite set 5, colours 15/2)` with the Elite HUD and a blue swatch
  (`build\bin\Release\t14_P/Q_capture_1450.bmp`). Tyler's own appearance.json was not touched (there was none in
  `build\bin\Release\data\`).
- **Not verifiable in first person:** helmet, shoulders, chest and the Elite armor set only show in third person
  (the first-person Elite hand looks the same for Minor and General). **Tyler: please check in game** (die, or look
  at yourself in the Armory/another player's view) that the helmet and Elite set match the Armory.
- GRD now offers MCC's four variants in MCC order (`5808e5e`): BASE `hilands`, OPTREX `hilands_b`, CBRN `hilands_a`,
  OPTREX/CBRN `hilands_c` (new; the Armory preview may have no model for it). Saved helmets load by `helmet_id`.
  Tested: GRD Optrex/CBRN reaches the profile as item 71 and a Sword Base match runs. Mariner variants map by name;
  "HIGHLANDS" chest has no MCC item (falls back to default). The ODST-only Firefight voices (Dare, Dutch,
  Mickey, Romeo) were removed from the Armory (`e950306`, Tyler's call); the voice now goes by `voice_id`, and old
  saves that picked one of them load as Noble Six. Whether the engine re-reads the profile at respawn (live change
  mid-match) is untested.

Head of main before this: `0f29293`.

## 0. Pick up here (2026-10-28, 08:30 ET) - Armory status from Tyler

**08:29 (Tyler, after O7OF's `f4f2d48`; Tyler is offline until later):**
- Preferred species now works (Spartan / Elite pick reaches the game).
- **Armor still doesn't swap in game**, and an **Elite always spawns as the Minor variant**.
- Appearance choices now save and survive a restart, but still don't change the character in game.
- From O7OF's side: FPS limit has several settings including uncapped, V-Sync fixed, FOV offset found,
  settings can be changed mid-match.
Next for the Armory: find the profile/player bytes the engine reads for armor pieces and the Elite variant
(Minor / Officer / Ultra / ...), e.g. with `-capture-profile` before/after captures and `-profile-set`
experiments like the controller look bytes (+0x1B5/+0x1B6) were found. Head of main: `6cfadc4`.

## 0. Pick up here (2026-10-28, 04:30 ET) - wheel, controller, music picker, service record, post-game

Tyler (03:47) asked for: career extras, post-game scoreboard, controller settings, music picker, FOV mid-match,
wheel down. All done and tested by test options + screenshots on Tyler's PC (he was away); details:

- **FOV mid-match: works.** New test option `-fov-test-at <s> <deg>` changes the saved FOV mid-match the way
  APPLY does; standing still, frame 900 (78) vs 1500 (110) show the wider view at once (`t1_A_capture_*.bmp`).
- **Mouse wheel down: solved.** The engine's wheel-down key code is **0xAE** (up is 0xAF); 0xB0 does nothing.
  Proven: 0xAE as change-weapon's second key + a -1 pulse switches weapons. Only the second key slot (+8) works
  (the "third slot" test that failed earlier was the wrong slot, not the wrong code). New settings
  `wheelUpAction` / `wheelDownAction` (default both 4 = change weapon; -1 = nothing), shown as KEY BINDINGS >
  MOUSE WHEEL. Same action both ways -> only 0xAF bound and down notches sent as +1 (old behaviour); different
  actions -> 0xAF / 0xAE on each action's second key and real -1 pulses (tested: up = melee, down = change weapon).
  Key bindings are now rewritten into the built-in profile every 500 ms, so changes may apply mid-game (not
  verified that the engine re-reads them live).
- **Controller: the engine never read the pad before.** With a host, haloreach.dll doesn't load XInput
  (`hr+0x23855`: only when the host global `hr+0xC1A100` is null); MCC passes the pad in the slot-36 buffer:
  `+0x000` u32 device (0 = controller, 1 = keyboard/mouse), `+0x120` u16 buttons, `+0x124/+0x125` u8 triggers,
  `+0x126/128/12A/12C` i16 LX/LY/RX/RY (decoded from `hr+0x23CF0..0x23D87`). So Tyler's "controller works" was
  probably Steam Input mapping it to keyboard/mouse - **ask him**. Noble now reads pad 0 with XInput
  (`FillLocalPad`) and switches device to whichever was used last; Start toggles Noble's pause menu.
  Tested with `-fake-pad <s>` (fake right stick for 1.5 s): view turns.
  Look speed: `-profile-set <off>=<byte>` test option. **+0x1B6 = horizontal stick speed** (0 -> ~19 deg instead
  of ~61 deg for the same push), **+0x1B5 = vertical** (`-fake-pad-y`: 0 -> barely looks up), +0x1B7 changed
  nothing measurable (range 0..9, stock 7; maybe acceleration). Engine range 0..18, stock 18.
  New CONTROLS > GAMEPAD rows: look sensitivity / vertical (1..19 -> +0x1B6/+0x1B5), invert (Noble flips RY),
  left/right stick dead zone (Noble-side, radial, 0..50%), vibration on/off + strength.
  Vibration: host slot 39 now calls XInputSetState (a1 = pad, a2 -> u16 {low, high, left trigger, right
  trigger}, from `hr+0x24194`). **Not verified** (no pad here; rumble may only fire for a pad user).
  The engine's dead-zone floats (+0x1B8 0.13, +0x1BC 0.12) were not tested, so Noble uses its own.
- **Menu music picker:** AUDIO tab now has a two-pane picker: Shuffle all / Off and the games (with track counts)
  on the left, the chosen game's tracks on the right, plus a search box over every game. Click = play + select.
- **Service Record** (new main-menu entry, `src/ui/CareerPanel.*`, `-menu-screen career -menu-tab 0..2`):
  rank + cR + progress, totals (games per mode, time, K/D/A, medals, best game, challenges), campaign best
  difficulty per mission, last 20 games. CHALLENGES (Tyler 08:00: 7 daily / 5 weekly, rotating): 7 daily from a pool
  of 18 and 5 weekly from 10; each day takes the next 7 in the list (wrapping), each week the next 5, so
  consecutive days never repeat one and all come back in turn. Paid when completed. COMMENDATIONS: 6 tracks (campaign missions, Firefight games, MP games, hours, kills,
  medals), tiers Iron..Max pay 500..25000 cR once. Campaign scoring: mission-complete bonus by difficulty
  (250/500/1000/2000) + 10% per skull. All in `career.json` under `"record"` (old files load fine).
- **Post-game scoreboard:** the post-game panel (560 px wide) now lists everyone in the results record (score,
  K/D/A, medals; only the local player is named - the record holds no names) above the cR lines, which now
  include challenge / commendation / mission payouts. Real 10-minute Sword Base match (04:24): paid 133 cR,
  history row + per-mode totals + "Time on Duty" challenge progress (10/45 min) saved; career.json was restored
  afterwards. Post-game panel checked (04:38, `build\bin\Release\t5_capture_900000.bmp`): GAME OVER, map, mode and
  time, scoreboard row "NobleSix [NOBL]" with score/K/D/A/medals, cR lines, rank bar (now blue), buttons.
- **Git (04:42):** pushed as `9e679d8`, rebased onto O7OF's `2b9e843` (ReachArmory + in-engine 3D menu: the front-end
  menu now runs inside the engine on `mainmenu`). FrontEndMenu.cpp conflicted; resolved by taking O7OF's file and
  re-adding the Service Record entry / screen / `-menu-screen career`. Builds clean; the in-engine main menu shows
  SERVICE RECORD (screenshot `capture_780.bmp`). Notes for O7OF: in his new menu `-menu-screen <name>` seemed to be
  ignored by the capture (it showed MAIN MENU), and the log warns `ReachArmory: Could not locate
  mainmenu_all.string_ids.csv`. Also check that nothing from RR's `mainmenu_all.map` is committed (repo rule).
- Small open bits: Start can't close the pause menu (the pad is only read while the game has input); vibration
  untested with a real pad; +0x1B7 meaning unknown; challenge kills/medals depend on the provisional stat slots.

### Earlier (03:40 ET) - FOV checked in game

**03:40 (Tyler's PC, O7OF's `c4ace4e` pulled and built clean):** FOV works when set before a match. Sword Base,
windowed 1600x900, `fieldOfView` 78 vs 110 in settings.json, frame 1500 of each run: the log shows "CameraFov: game
field of view set to 78/110 degrees" and the 110 shot has a visibly wider view (the rifle takes up much less of the
screen). Shots: `build\bin\Release\FOV78_capture_1500.bmp`, `FOV110_capture_1500.bmp` (different spawns, so
compare the gun, not the scenery). Not checked yet: changing FOV live from the pause menu mid-match. FOV range is
now 70-120 (Tyler's call, 03:45, commit `93681f4`): slider (always clamped, also for Ctrl+click typing), the
settings file clamp and the help text all say 70-120; CameraFov still guards the engine's own 1-150.
Tyler's settings were restored after the test (copy in `data\userdata\settings.backup.json`).

## 0a. Latest (2026-10-28, 02:00 ET) — FOV hooked up + settings reachable from the pause menu

**02:00:** Item 3 below is implemented (not yet run in game). Static REA/Ghidra analysis of `haloreach.dll`
(`docs/fov-findings.md`) shows the game's FOV is a float in degrees at **`haloreach+0xB42C10`** (78.0f in the
shipped DLL; the engine multiplies it by pi/180 for the camera and only accepts 1–150 degrees).
- New `src/engine/CameraFov.{hpp,cpp}`: `SetEngineModule()` from `main.cpp`, `Apply(degrees)` writes the global,
  `Read()` logs what the engine holds, and `Tick()` (called from ReachHost slot 1) applies `Settings::fieldOfView`
  when it changes and re-asserts it every 2 s because the engine refreshes its camera block from the profile.
- `SETTINGS > VIDEO > FIELD OF VIEW` now reaches the running game on APPLY; the default is 78 (Reach's own value),
  RESET goes back to 78, and the help text states the 1–150 limit.
- The pause menu has a **SETTINGS** entry: it renders the same `SettingsPanel` in a large window, so FOV, VSync,
  frame cap, mouse, audio and key bindings can be changed mid-match. Esc (or `< BACK`) returns to the pause menu;
  Esc during a key rebind still cancels the rebind only.
- Version is 0.8.2. Rebuilt `build\bin\Release\noble-client.exe`.
- Build note: `build\` had a broken Ninja `CMakeCache.txt` (left over from a failed configure; no build.ninja).
  It was reconfigured as `cmake -S . -B build -G "Visual Studio 18 2026" -A x64` (MSVC, not ClangCL) and built with
  `cmake --build build --config Release`.
- **Still to verify in game:** whether the camera picks the new value up live (it may cache FOV at spawn; a map load
  or a respawn may be needed) and whether 78 vs 90 looks right. Test: pause → SETTINGS → VIDEO, drag FOV, APPLY.

---


## 0. Pick up here (23:58 ET) — Display Settings & FPS Control + Controller Picture

**23:58 (O7OF):** Bumped to version 0.8.1 with FOV/VSync/FPS limit/FPS counter display fully implemented. The FPS counter displays in-game (upper-right, blue text on dark box, toggled via Settings > Display). VSync and frame-cap are applied in the swap-chain Present hook and take effect immediately without restarting. FOV slider stores selections (60–120°, default 90) but remains **UI-only** until the game camera mapping is verified via profile capture. The preview loads local MCC-derived Spartan/Elite OBJ groups, undersuits, and Elite harness layers; color masks and visor tint work in the preview. In-game armor and color IDs remain unmapped, so preview selections do not change the character yet.

**23:45 (upstream):** Controller picture replaced (item 5, built + screenshot-checked). O7OF made a 1600x1000 transparent PNG of an Xbox 360 pad; it is tracked at `assets/ui/noble/controller.png` (original work, OK for the public repo). CMake now also copies `assets\` into `<exe>\data\` after a build (data\ itself stays git-ignored). `UiArt::NobleControllerImage()` loads `controller.png` from `data\ui\custom` then `data\ui\noble`; SettingsPanel uses it with its own callout table (`kReach360`, straight leader lines, left margin sized from the widest label) and falls back to MCC's LARGECONTROLLERFRONT with the old table when the file is missing.

**Do not commit or distribute OBJ files from `data/armor` or profile captures.** Use `-capture-profile data/profile-captures` to record Slot 34 profile buffers for before/after comparisons; delete captures after analysis.

Open list, in priority order:
1. **Load armor geometry from MCC maps instead of local OBJ exports:**
   - Locate Spartan/Elite model files (`.model`) in `{MCC_INSTALL}\haloreach\models\` (e.g., `m_spartans.model`, `m_elites.model`)
   - Parse and extract mesh/skeleton at runtime; discard local OBJ files to avoid shipping game assets
   - Harness and armor should layer correctly on both species (tested in preview, not in-game)
2. **Determine armor equipment/color offsets in Slot 34 profile:**
   - Use profile captures (`-capture-profile data/profile-captures`) to identify which bytes control torso, helmet, shoulders, legs, visor color, and suit color
   - Map UI selections to equipment IDs and color indices
   - Test in-game: set armor in settings, launch a game, verify the character reflects the changes
3. ~~**Map FOV to the game camera and verify it works in-game**~~ (works at match start, 03:40) — wired in 0a below (FOV value at
   `haloreach+0xB42C10`, see `docs/fov-findings.md`); still needs an in-game check that the camera takes the value live
4. ~~**Controller settings**~~ done 04:30 (native XInput pad, look speed, invert, dead zones, vibration; see section 0). Left: verify with a real pad (vibration, +0x1B7)
5. ~~**Menu music picker**~~ done 04:30 (two-pane picker with search)
6. Check in game: Forge menus and loadout cards no longer show hex (expected fixed by Slot 55)
7. Noble exiting from inside a game: confirm `tasklist | findstr /i noble` is empty afterwards
8. Mod-only maps (not in Reach's tables) are listed but disabled: needs map-id work in BlamEngine
9. Halo 2 Anniversary music (`sounds_*.dat`) not readable yet

Build/run (Developer Command Prompt, not plain PowerShell - cmake is only on PATH there):
`cd /d D:\Halo-Reach-Community-Edition && cmake --build build --config Release && cd build\bin\Release && noble-client.exe`
If LNK1104 (exe locked) and `taskkill` fails, a stuck process needs a real reboot; rename the exe meanwhile.

## 0c2. Earlier (23:15 ET)
(23:15 ET)

**23:15: pushed + mouse settings + input prompts (built, Tyler OK'd "fine for now").** Stuck processes were from
quitting at the menu (10:12-10:17 runs, ExitProcess hang, likely XAudio2 DLL detach); real reboot cleared them; with
TerminateProcess the menu-quit check came back clean (in-game quit not re-checked yet). Commit `d31ccff` rebased onto
O7OF's `2248385` (adds ObjModel/ObjModelPreview armory preview, ArmorCustomizer::Render(device, ctx), default master
server URL) and pushed as `7500059`. Since then (not committed): `main.cpp` `return 0;` after TerminateProcess (C4715);
new `src/ui/InputPrompts.cpp/.hpp` (device tracking from ImGui input each frame in `UIManager::BeginFrame`; drawn
glyphs: A green / B red circles, LB/RB pills, D-pad, or keyboard key boxes); FrontEndMenu `DrawHints` takes
`Prompt` values, `AcceptPressed()` (Enter / keypad Enter / pad A) and D-pad for focus rows and skulls; SettingsPanel
MOUSE section (look sensitivity + RESET, new `mouseSensitivityY` vertical ratio 0.25-4, invert, help lines) and LB/RB
(Q/E) prompts around the tab row; ReachHost slot 36 reads look settings every 500 ms (`InputLookSettings`) so APPLY
works in a running game; `-mouse-scale` still overrides. Small ReachHost change - tell O7OF. Tyler (23:17): **a controller already works in game** (correct older notes saying gamepad isn't supported). Next:
CONTROLLER section (settings for it), Video options (FOV, V-Sync), cleaner music picker, pixelated controller picture.

## 0b2. Earlier (22:40 ET)
(22:40 ET)

**22:40: stuck Noble processes + exit change (written, build untested).** 22:28-22:34: Modded tab now collapsible
(mod heading rows with arrow + map count, click/Enter toggles, `st.openMods`), mouse-wheel scrolling + scroll bar in
`DrawList` (per-list offset), preview blurb height measured (fixed a const ImFont compile error). Tyler built it: works.
Three `noble-client.exe` (PIDs 41172/7516/8484, ~2 GB each) stayed listed after closing and could not be killed even
as admin ("no running instance") - stuck in the kernel (driver wait); only a reboot clears them, and they lock the exe
(LNK1104; workaround: rename the exe). main.cpp's three `ExitProcess(0)` are now `TerminateProcess(GetCurrentProcess(), 0)`
(ExitProcess runs DLL detach under the loader lock and can hang). If processes still get stuck: find when (after a
match? quit from menu?) and release D3D / stop the engine before exiting. Also sent Tyler `docs/career/armor-list.md`
(readable Armory list from armor.json). Not committed yet: everything since 355b31f (audio, menus, mods, VFS, main.cpp).

## 0a2. Earlier (22:30 ET)
(22:30 ET)

**22:30: VANILLA / MODDED tabs (written, NOT BUILT).** 22:20: Tyler tested the MOD row + click-to-pick: both work.
He wants MCC's way instead: map selection -> vanilla maps or modded maps. The MOD option row is gone; Campaign,
Firefight and Custom Game have `DrawSourceTabs` (VANILLA | MODDED (n); click, Q/E, LB/RB). The modded tab lists
every mod's maps under the mod's name (`ListItem.header` rows, not selectable; `ModdedMaps`, `FixSelection`); the
picked map carries its mod (`MapEntry.mod`) into `FrontEndChoice.modFolder`. "Mod: <title>" shows under the
preview. Campaign rows back to 1 difficulty, 2 skulls, 3 start. Forge has no tabs.

**22:25 (superseded in part by 22:30):**

**22:25: list picking + MOD option (written, NOT BUILT).** Tyler (22:04): in Campaign the mission under the mouse
became the pick, so moving to START changed it; and Campaign/Firefight need options such as Mods and game type.
- `DrawList(..., hoverSelects)`: mission/map lists now only change on click or keys (hover = light highlight);
  the main menu still follows hover.
- New `src/ui/ModCatalog.cpp/.hpp`: scans Workshop (976730) + `data\mods` for `Engine: HaloReach` mods; reads
  `campaign\*.json`, `firefight\*.json`, `multiplayer\*.json` (+ `GameModContents.*Maps`, `CampaignInfo.json`
  CampaignMaps): `ScenarioFile` last part = scenario, `Title/Description.Neutral`; and `maps\*.map` names.
- Campaign rows: MOD, DIFFICULTY, SKULLS, START (focus 1-4). Firefight/Custom: MOD, GAME TYPE, (DIFFICULTY), START.
  Choosing a mod swaps the list to the mod's maps; maps not in Reach's own tables are shown disabled (Noble
  launches by Reach map id, see BlamEngine::ResolveMapId). Maps the mod only replaces show as "<name> (modded)".
- `FrontEndChoice.modFolder/modTitle`; main.cpp calls new `VFS::SetActiveMod(folder)`; VFS::ResolvePath reads
  `<mod>\<relative path>` first (e.g. `maps\ff60_icecave.map`). Small VFS change - tell O7OF.
- Test: Firefight, MOD = "Icecave: The Last Stand" (Workshop 3724674100, replaces ff60_icecave = Glacier).

## 0c. Earlier (22:15 ET)

**22:15: Menu music catalogue from every readable MCC game (written, NOT BUILT).** 21:47 build tested: Reach UI
sounds + Reach menu music work (cache shows a_button, b_button, cursor_*, bumper7 L/R, 13 menu pieces; no warnings).
Tyler asked for the whole Reach OST plus Halo 1/2/3/ODST themes.
- ReachAudio now builds a track list per game. Reach/H3/ODST from `<game>\fmod\pc\sfx.fsb(.info)` (same format);
  Halo: CE from `halo1\sound\pc\sounds_music.fsb` + `lst\sounds_music.lst.bin` (u32 1, u32 count, then u32 len,
  name incl NUL, u32 first sub-sound, u32 permutations; `sound\music\` = Anniversary, `sound\old_music\music\` =
  classic, `sound\sinomatixx*` = cutscene music). H1 index->vgmstream sub-song mapping is ASSUMED (first+k+1);
  the stream names vgmstream prints are logged at debug level to check it.
- Halo 2 Anniversary: sound is in `halo2\h2_maps_win64_dx11\sounds_*.dat` (custom format) - not supported yet.
- A track = one music cue: intro, loop pieces (random), outro; `alt_*` layers, sustain/perc/stinger/ambient beds
  skipped. Counts from the .info files: Reach 62, H3 45, ODST 80, CE Anniversary 95, CE classic 85. Labels: curated
  for menu themes and Reach mission cues (m20 ONI Sword Base ... m70 Pillar of Autumn) and CE cutscenes (a30 Halo
  ... d40 The Maw); the rest are prettified cue names (the in-game score is stems, not the album tracks).
- Ids: `<game>:<cue path>` (`reach:menu`, `h3:menu`, `odst:menu`, `h1a:menu`, `h1:menu`), plus `all` (shuffle all
  games: each track ~2.5 min then its outro) and `off`. Old setting values ("shuffle", piece names) = `reach:menu`.
  Settings default is now `reach:menu`. MENU MUSIC row is a grouped drop-down (game headings), plays on pick.
- Music is decoded on demand (the Reach menu theme and UI sounds up front); cache trimmed to ~1.1-1.5 GB at start.

## 0b. Earlier (22:05 ET)

**22:05: Reach menu sounds + menu music written, NOT BUILT YET (task #17).** Tyler asked (21:31): Reach's menu
scroll sound instead of the beeps, Reach menu music with a track choice; then a Settings rework (MOUSE and CONTROLLER
sections, modern but Reach-feeling navigation and previews) and prompt icons that follow keyboard/mouse vs gamepad.
Hex fix is committed and pushed (`355b31f`).
- New `src/audio/ReachAudio.cpp/.hpp`: reads `haloreach\fmod\pc\sfx.fsb.info` (280-byte records, name at +24,
  record N = vgmstream sub-song N+1), decodes the sounds it needs once with `vgmstream-cli -s N+1 -o x.wav sfx.fsb`
  on a worker thread into `<exe>\data\userdata\cache\audio` (git-ignored via `data/`), plays them with XAudio2.
  UI sounds by bank path: omaha_ui cursor_vertical / cursor_horizontal / a_button / b_button, ui\error, first
  `bumpers\left_bumper\` / `right_bumper\` entry, exit_experience game_completion/totals/rank_up/slot_machine.
  Music: `data\sound\music\reach_menu\track1\in\*` = intro, `...\loop\*` = pieces (all pre-decoded in the
  background; only the playing piece is held in RAM). "shuffle" = intro then random pieces; a piece name = that
  piece looped; "off". vgmstream-cli is looked for in `<exe>\tools\vgmstream`, `<exe>\vgmstream`, `<exe>\bin\vgmstream`
  (NOT shipped; Tyler copies his own into `build\bin\Release\tools\vgmstream`). Without it the old beeps play and
  SETTINGS > AUDIO shows why.
- `MenuSounds` API now: Click (A), Back (B), Scroll (list up/down + hover), Change (value left/right), Tab(dir)
  (bumpers), Error. FrontEndMenu: list step -> Scroll, value cycle -> Change, goBack -> Back, the extra sound on
  screen change removed. SettingsPanel: tabs -> Tab(+-1), Cycle -> Change. Volume = `volumeOther`.
- Settings: new `menuMusicTrack` (default "shuffle"), SETTINGS > AUDIO > MENU MUSIC cycles the tracks and previews
  at once (UNDO restores the saved one). Music starts in `FrontEndMenu::Run`, stops (fade) before it returns.
- main.cpp calls `ReachAudio::Init` after `UiArt::Init`. CMake: ReachAudio files + `xaudio2`, `ole32`.
- All edited files are CRLF (repo uses CRLF; convert before copying back).
- Unverified: whether the `.info` order really matches vgmstream's sub-song order for every record (a warning is
  logged if vgmstream prints a different stream name), and the exact bumper file names.
- Next: Tyler builds + tests; then task #15 Settings rework (MOUSE: sensitivity X/Y, invert, raw input/accel?;
  CONTROLLER: look sensitivity, invert, dead zones, vibration, button layout picture - current one is pixelated;
  VIDEO: FOV, V-Sync, quality), task #16 prompt icons per input device, and ask Tyler to check Forge + loadouts.

## 0a. Earlier (21:40 ET)

**21:40: hex-number bug FIXED (tested).** With slot 55 returning 1 the engine registers its strings (log:
"StringTable: register ... found" for all 40 logged, lookups return real text such as "Tuesday", "October") and the
scoreboard shows place "1", score "0" (`capture_777777.bmp`, 21:27). Tyler confirmed it works. All diagnostics
(`crtfmt` IAT wrappers, `[tokendiag]`, `[hexdiag]`) are removed from ReachHost.cpp (112,875 bytes). Still to do:
build, commit ReachHost.cpp + this handoff, push; delete the untracked `src/engine/HudColor.*`. Then check Forge
menus and loadout cards in game (expected fixed by the same change).

**21:35: hex-number bug ROOT CAUSE FOUND: host slot 55 must return 1.**
- `hr+0x3140D` calls host slot 55 (vtable +0x1B8, "strings clear") and only if it returns true ("test al,al /
  je skip") registers the string tables through slot 56 (+0x1C0, ~8700 strings; RR's log: "strings registered by
  the engine: 8712; lookups answered from the language file: 11178"). Noble's slot 55 returned 0, so slot 56 was
  never called and every slot 57 lookup missed (Noble log: all "(none)"). Fields bound to string ids then show
  the raw id as 8 hex digits: score/place, "Respawn in", loadout grenades, Forge menu title "00000002", prices
  "$00000001", rotation snap "00000008". The `[tokendiag]` hook on `hr+0x2D26C0` never fired for them (they use
  the int -> string-id conversion of value tag 6, `hr+0x2F8FD4`, not `\x07` tokens).
- **Fix (uncommitted, ReachHost.cpp):** `HostSlot55_StringsClear` returns 1. Test with go.bat; check the log for
  "StringTable: register ... found" lines and the scoreboard / loadout / Forge numbers.
- Once confirmed: remove `[tokendiag]`, `[hexdiag]` and the crtfmt diagnostics; the fallback string table in
  ReachHost.cpp may then be mostly unneeded (lookups come from MCC's EN_HaloReach.bin via registered keys).

**21:25: hex bug, UI token pipeline mapped (static) + diagnostic running.**
- `hr+0x2D288C` = string id -> text: gets the text via `hr+0x2D25A4` -> `hr+0x9E0D0` (host slot 57 =
  vtable +0x1C8, then the map's own strings; no hex there), then expands `\x07` tokens in a loop. For each token it
  asks host slot 51 feature 5 (`[hr+0xC1A100]` vtable +0x198, Noble answers on, like RR). On: splits
  "datasource.property" at '.', turns both into string ids (`hr+0x9DB08`), finds the property (`hr+0x9E350`) and
  calls **`hr+0x2D26C0`** to write the value as text. Off: parses the token as 8 hex digits (`wcstoul` radix 16).
- `hr+0x2D26C0`: datasource object -> column (vtable +0x50), rows (+0x60), value (+0x68 -> {tag, value});
  int value whose column info (+0x58) says "string list" -> recurse into `hr+0x2D288C` with that string id;
  otherwise tag handler to_text (`hr+0x2D284E`).
- The slot 61 call stack only shows the draw path (`hr+0x2C2BF6` -> `0x2DA7FB` -> `0x2DA262` -> `0x2DF075` ->
  `0x2DF68F` -> `0x2DF533` -> `0x26921B` -> `0x268DD4` -> `0x1CF393` -> slot 61), not where the text is built.
- **Running (uncommitted, ReachHost.cpp):** `[tokendiag]` MinHook on `hr+0x2D26C0` logs column id, extra, output
  text (first 40 calls + any 4/8-digit hex-looking output with a call stack). `[hexdiag]` in slot 61 logs stacks for
  "Respawn"/"01c"/"02" texts. Both must be removed before committing (with the crtfmt code).
- **Warning:** the stale-copy revert also hit `ReachHost.cpp` twice (a 76 KB / older copy reappeared after a
  write). Always check sizes after writing and run `git diff --stat` before committing.

**21:30: hex-number bug, cause narrowed down (static analysis of haloreach.dll).**
- The loadout's "0000 / 0002" is one 8-digit value `00000002` wrapped onto two lines, so all cases are a 32-bit
  int shown as hex (score, place, respawn time, frag grenade count). All four come from strings with a
  `\x07datasource.prop_x\x07` token. Noble's fallback strings are identical to RR's (`D:\Reach\data\strings_en.json`
  is RR's capture of what it answered: 71 strings, all present in Noble's table; slot 56 never registers anything,
  so every lookup uses the fallback table). The strings are not the problem.
- UI value types: sorted handler array at `hr+0xB4C800` (14 entries, looked up by tag with binary search
  `hr+0x1802E9720`; built at init `hr+0x2AA09A`). Each handler = {u64 tag, can_convert_from(tag), convert(src,dst),
  to_text(value, buf, cap)}. Tag 1 = int32 (`hr+0xA36058`, to_text `hr+0x2F3AB8` = `_itow_s` radix 10). Tag 8 =
  int64 (`hr+0xA45B50`): convert `hr+0x381EF0` accepts tag 1 (sign-extends), **to_text `hr+0x381F10` =
  `_i64tow_s(v, buf, cap, 16)`**, i.e. hex. Other tags: 0 bool ("true"/"false"), 2 float, 3/12/13 strings.
- Value to text: `hr+0x2D284E` (handler->to_text for the value's tag; tag 6 handled separately). Data binding:
  `hr+0x2F4F29` converts the source value to the widget property's tag (handler->convert), then sets the property
  (vtable+0x70). So an int reaches the text as tag 8 and prints in hex. Why RR doesn't hit this is still unknown
  (maybe the source value's tag, or which widget property type is bound).
- Ruled out this round: the engine's own narrow printf at `hr+0x7C2EF0` is OpenSSL's BIO_printf (next to
  `crypto\bio\b_print.c`); narrow `%08x`/`%04x` strings are OpenSSL/network/marker messages; the only wide `%x`
  is the locale check; no hand-written hex-digit code in the UI.
- Padding to 8 digits isn't explained by `_i64tow_s` (it doesn't pad), so check where the zeros come from if the
  test below shows decimal but still padded.
- **Test (done, reverted):** `src/hooks/EnginePatches.cpp` patch 2 changes the radix byte at
  `hr+0x381F1A` (`41 B9 10 00 00 00` -> radix 10), only if the bytes match. Tyler runs
  `D:\Halo-Reach-Community-Edition\build\agent\go.bat` (job = build + Sword Base + scoreboard shot
  `capture_777777.bmp`, log `Y_noble-client.log`; look for "UI int64 values now convert to text in decimal").
  **Result (21:12): no change.** The patch was applied (log: "UI int64 values now convert to text in decimal"),
  but the scoreboard still shows place "0000" and score "0000001c" (`capture_777777.bmp`). So the tag-8 to_text
  is NOT the source; the patch was reverted (EnginePatches.cpp restored to the committed version).
  Remaining ideas: the token `\x07datasource.prop\x07` may not be evaluated at all (property lookup fails and the
  text widget dumps the raw value / handle; 0x1c and 1 look like small indices, place is 4 digits = 16-bit);
  find where the text widget expands `\x07` tokens (wide 0x0007 compare in the UI code around hr+0x2C0000-0x300000
  found nothing with `cmp ...,0x7`, so it may scan with a table or `wcschr`), or compare at run time which
  datasource the scoreboard binds (RR vs Noble). An interactive debugger (x64dbg on Tyler's PC, breakpoint on
  memory write of the text) would find it fastest.
- Still in `ReachHost.cpp` (uncommitted): the crtfmt diagnostics; remove before committing.

**20:55: hex-number bug (score "0000001c", "Respawn in 00000001", loadout grenades "0000 / 0002") still open.**
RR shows all of these correctly, so it is something Noble's host does differently.
- Ruled out: the engine's wide sprintf `hr+0x35D84` (only normal output: '%d'->'10', '%d:%02d'->'0:00', ...);
  `_itow_s` (only caller `hr+0x2F3AB8`, radix 10; a UI data-value vtable at `0x180A36070`); locale code
  `hr+0x2B6F12`; Noble's own source (no hex formatting on these paths).
- Found: `hr+0x381F10` calls `_i64tow_s` with **radix 16** (another data-value type: type 8 = int64, and
  `hr+0x381EF0` converts type 1 int32 -> type 8). It does not zero-pad, so it is a lead, not the answer.
- There are two more wide sprintf wrappers: `hr+0x35CC8` and `hr+0x35D24` (20 callers, incl. `hr+0x381ED4`).
- **Running now (uncommitted, on the PC):** the old 0x35D84 hook is replaced by IAT wrappers around all six CRT
  format imports (`__stdio_common_vsnwprintf_s/vswprintf/vswprintf_s/vsprintf/vsprintf_s/vsnprintf_s`, IAT RVAs
  0x91EA20/28/48/60/70/38). Log lines: `ReachHost [crtfmt ...]`, plus `zero-padded` for outputs starting "0000".
  job.bat = build + Sword Base with `-scoreboard-at 20 -quit-after 30`, log copied to
  `build\bin\Release\Y_noble-client.log`. **Result (20:56):** no CRT call produces any hex/zero-padded text
  (every format seen is normal: '%d'->'0', '%d:%02d'->'0:00', '%i'->'25', '%s%s%s'->'0', ...), so the hex text
  is made by engine code itself, not printf. If no CRT call makes the hex text, the text is
  made by engine code (look for code that writes hex digits by the value's size), or the value reaching the UI
  has the wrong type tag (compare with RR at the data-value vtables).
- Remove the diagnostic (`EnsureFormatDiagnostics` + `crtfmt` wrappers in ReachHost.cpp) before committing.

### Earlier (20:20 ET)

**20:15: hand checks done by computer use (all pass).** Windowed 1600x900 Sword Base:
- Post-game panel (held after the engine stops): PLAY AGAIN relaunches into a new match. A very short synthetic click
  was missed (the panel polled GetAsyncKeyState once per frame), so overlay menus now also count WM_LBUTTONDOWN
  (`Window::TakeLeftClicks`; cleared when the pause menu / post-game panel opens so in-game shots don't click).
- Pause menu QUIT TO MENU returns to the front-end menu; its QUIT closes Noble.
- Moving the window: with the mouse captured the title bar can't be reached (cursor clipped to the game), so the
  click lands in the game. Alt+Space > Move works, and with the pause menu open the title bar drags normally.
- Service tag: scoreboard shows "NobleSix  TEST" (`-scoreboard-at <s>` test option holds Tab 3 s and saves
  `capture_777777.bmp`).
- Not checked: kills/medals in the results (no enemies in an offline Slayer; needs Firefight or a second player).
- **New clue for the padded numbers:** the scoreboard shows score "0000001c" and place "0000": the numbers are printed
  as **hex at full field width** (8 digits for 32-bit, 4 for 16-bit), not just zero-padded. Looks like the UI's
  value-to-text falls back to a raw hex dump when it doesn't recognise the property's type. `%x` users in the DLL
  checked so far: `+0x2B6F12` is only locale detection (GetLocaleInfoEx + swscanf). Slot 61 only gets the set of
  unique characters, so it can't be used to catch the padded strings. Open question for Tyler: does RR show the
  same hex numbers?
- Tyler's settings were switched to windowed 1600x900 + tag TEST for the test and restored afterwards
  (`settings.backup.json` next to it is the copy).

**Pushed to GitHub (main):** `12a9971` HUD colours + post-game hold, `5e71e53` Mods screen (Tyler-tested),
`ab77f8b` service tag setting, plus the mouse-wheel / window-move commit below. Commits and pushes now run from
`build\agent\jobs\job.bat` through go.bat (output in `build\git_out.txt`); `src/engine/HudColor.*` stay untracked
(old stopgap, safe to delete).

**18:18: mouse wheel works (weapon switch, tested).** Input +0x114 is an **f32** wheel pulse (RR `host::WheelPulse`:
per notch +1/-1 for 40 ms, one pulse per 80 ms, up to 3 queued; `win::WHEEL` accumulates WM_MOUSEWHEEL / 120). The
engine only acts on it through the key bindings: RR's profile binds action 4 (change weapon) to '1' **and** second
key **0xAF** at entry +8 = wheel **up**. 0xAE in the third key slot did not give wheel down, so Noble sends every notch
as "up" (with two weapons it's the same). `-wheel-after <s>` test option injects one notch. Window.cpp now counts
WM_MOUSEWHEEL (`Window::TakeWheelNotches`) and, on WM_ENTERSIZEMOVE/KILLFOCUS, frees the cursor clip; ReachHost
releases the mouse while the window is being moved (`Window::InSizeMove`), so a windowed game can be dragged
(Alt+Space > Move, or title bar). The window-move part is untested by hand.

**17:35: post-game freeze FIXED (tested).** Full 10-minute Sword Base Slayer: slot 6 at 17:33:07 (cR paid, panel
shown), engine resolved `mainmenu.map` and stopped drawing; at 17:33:16 the log says "engine stopped drawing after
the match; Noble draws the post-game panel itself", and `capture_900000.bmp` (taken from the main-thread frames)
shows the GAME OVER panel with the cR lines, rank bar and the three buttons. Still to confirm by hand: clicking
PLAY AGAIN / RETURN TO MENU / QUIT on that panel (input is polled the same way as before, so it should work).
Note: the automated test runs pay cR into Tyler's `data\userdata\career.json` (now 2436 cR from test matches);
delete that file to start from zero. Not committed yet: `PauseMenu.*`, `main.cpp`.

**Number padding ("Respawn in 00000001", loadout grenades "0000 / 0002"), investigated 17:30, not solved.** The
engine builds those strings itself with the zeros already in them (slot 61 TextPrepare gets the characters
"Respawn i01"), so it is not a font/glyph problem. Widths are 8 and 4 digits, i.e. 32- and 16-bit fields, which
smells like a number-format setting the host should supply. Not the partner's IsWhitespace hook: the original
`haloreach+0x362E8` is a binary search over 12 Unicode whitespace ranges used for line breaking and really does
run away for characters below 0x09 (the `\x07` property markers MCC's own strings use), so the hook is correct.
Not slots 95/96 either (HUD scale / safe-area queries). Next idea: find where the UI formats an integer property
(look for the code that fills `camera_list.prop_respawn_time`) and see which host answer changes it.

**17:25: post-game freeze, fix (details).** After a match the engine goes for its main menu map (not in MCC) and stops
calling slot 1, so the post-game panel was never redrawn. RR handles this in its main loop: `host::loadscreen::
begin_hold()`, then `hold_frame()` + `swapchain->Present(1,0)` from the main thread for up to 60 s
(`host-probe.exe__main.c` ~4950). Noble now does the same: `PauseMenu::HoldFrame()` (main thread, called from the
message loop in `main.cpp`) draws the panel and presents once the game is over and no engine frame came for 1 s;
Render (engine thread) and HoldFrame share a mutex. Also a 5 s watchdog after the message loop, because the engine's
Shutdown may never return when its thread is stuck. Test running at 17:23 (full 10-minute Sword Base Slayer,
`U_*` files: look for "Noble draws the post-game panel itself" and `U_capture_900000.bmp`).

**17:20: dark HUD FIXED (host slot 97).** The engine sends every HUD colour through host slot 97 before it reaches
the HUD shader: `haloreach+0x2DDD8C` (sets constant id 0x60000 = chud_color_output_A..F, via `+0x271200`) calls
`+0x2DDF3C` for each colour, which packs it to ARGB and calls `[host]+0x308` (slot 97) with (a1 = filter/mode index
from `haloreach+0xD1F71C`, a2 = ARGB), then unpacks the returned ARGB. RR (`host::hit` case 0x61) returns the colour
unchanged; Noble returned 1, which made every HUD colour (0,0,1/255,0). `HostSlot97_ColorFilter` now returns a2. Test
(Sword Base, 17:19, `T_capture_2400.bmp`): shield bar, compass, tracker, reticle, ammo, grenades, game-type box all in
Reach's own colours. `src/engine/HudColor.*` (the Map/Unmap stopgap + diagnostics) is no longer built or called:
removed from `CMakeLists.txt` and `main.cpp`; delete the two files (`git rm src/engine/HudColor.cpp
src/engine/HudColor.hpp`). Not committed yet: `ReachHost.cpp`, `main.cpp`, `CMakeLists.txt`.

**Git state.** Everything up to the career work was committed on `main` as one commit ("Front-end menus, MCC art, skulls,
subtitles, settings, key binding names, career/cR, master server") and rebased onto the partner's `1d2a282` (their
`EnginePatches` whitespace-crash fix is kept). The `ReachHost.cpp` conflict was resolved by keeping Noble's version: all of
the partner's ReachHost edits were in the text code that Noble had already replaced or fixed the same way (glyph record,
digit/icon shortcuts, string fallback). The one idea not carried over: pre-rasterising text in slot 61 (TextPrepare).
If `git status` still says "rebase in progress", run `git add src/engine/ReachHost.cpp`, then
`git -c core.editor=true rebase --continue`, then `git push`.

**Not pushed yet:** a build fix made after that commit: `src/engine/KeyBindings.cpp` (the action table had landed outside
the anonymous namespace) and `src/engine/ReachHost.cpp` (unused-parameter warning). With them the build passes
(14:22). To push:
`git add src/engine/KeyBindings.cpp src/engine/ReachHost.cpp`, `git commit -m "Fix KeyBindings.cpp build"`, `git push`.

**Running the test job.** `D:\Halo-Reach-Community-Edition\build\agent\go.bat` (full path; from inside `build` the relative
path fails). It writes to files, so the console stays quiet; the line `EXITCODE` on screen is a quirk of `nb_build.bat`
(`echo EXITCODE 0>>` redirects) and means the build succeeded. Current job = `nb_run13.bat`: key-bindings screenshot
(`Q_keys.bmp`), then Sword Base with `-end-round-after 40 -quit-after 90` (`R_*` files in `build\bin\Release`).

**Result of the 14:22 run:** build OK, game ran 90 s without problems, but **`-end-round-after` did not end the game**:
the log shows "ending the round" at 40 s and then nothing (no slot 6, no `Career:` lines). Either `haloreach+0x7B3EC`
needs to run inside the simulation update (not from slot 1), or ending a round alone does not end a 1-round Slayer game
(try `game_won`'s evaluator `+0x1B0810`, or the engine's end-game function that `mp_round_end` leads to). Until then,
test career by **playing a short match to the end** (Slayer time limit) and reading the `Career:` log lines and
`data\userdata\results\last_game.bin`. `Q_keys.bmp` from this run has not been looked at yet.

**cR test passed (15:19).** Sword Base Slayer played to the time limit after the 13 cR/min rebuild. Log: `took 10.3 min;
local player 0 kills 0 assists 0 deaths 2 medals 0; paid 133 cR` = 10.3 x 13, matching Tyler's count (2 deaths, nothing
else); the post-game panel showed +133 cR and 133 / 7500 toward Private. No slot-machine bonus that game (1-in-8). Per-player
stats array: deaths appear at indices 11 and 13 (both 2).

**New: freeze after the post-game panel.** The panel opened (15:19:38) but its buttons could not be clicked. Right after
the results, the engine resolved `haloreach\maps\mainmenu.map` (slot 48), which MCC does not ship; about 7 s later it made
its first iface2 slot 140 call (a2=0x28, a3=0x40), a slot 4 EngineEvent with reason 0 and an empty message, and slot 1
frame ticks stopped (last frame 111000 at 15:19:45). With no frame ticks the panel is never redrawn or given input. Likely
cause: after game over the engine tries to go to the main menu map. Next: stop or answer that transition (slot 48 for
`mainmenu.map`, or slot 140 / slot 4), or move Noble to its own menu before the engine leaves the match.

**Tyler's reference list (from the afternoon chat).** He supplied 11 screenshots plus pasted notes (loadouts, control
schemes for MCC and Xbox 360, general Halopedia info on Reach) for: subtitles, main menus, skulls, controller layout
(360 and MCC), keyboard/mouse key bindings, loadout names. The files are not in the repo (Bungie/343 art and wiki text;
keep it that way). Status:
| Item | State |
|---|---|
| Subtitles | ✅ SUBTITLES setting + `0xD5244` hook (§5) |
| Main menus | ✅ Reach-style `FrontEndMenu` screens, art read from the MCC install (`UiArt`) |
| Skulls | ✅ Skulls screen + launch block +0x48 (§5) |
| Keyboard/mouse bindings | ✅ KEY BINDINGS tab, 66 slots named in MCC's groups; `Q_keys.bmp` (14:22 run) checked: renders correctly |
| Loadout names | ✅ come from the game variant and render correctly (§5) |
| Controller layout (360 and MCC) | ❌ **still to do**: a CONTROLLER tab in SETTINGS showing the MCC default and the original Xbox 360 layouts (controller picture already readable from `controllertexturepack`), and gamepad input itself (XInput into slot 36 +0x120.., §6 item 6) |
Checked against the Halopedia notes: the menu has all 24 MP maps (incl. Noble, Defiant and Anniversary packs), all 10
Firefight maps and all 11 campaign missions; mission descriptions are Noble's own wording (don't paste the official
taglines). Not used yet but useful later: the Armory's 30 armour colours (10 bases × 3 shades: Steel/Silver/White,
Red/Mauve/Salmon, Orange/Coral/Peach, Gold/Yellow/Pale, Sage/Green/Olive, Teal/Aqua/Cyan, Blue/Cobalt/Sapphire,
Violet/Orchid/Lavender, Crimson/Rubine/Pink, Brown/Tan/Khaki) for the Armor Locker, and the Arena divisions (Onyx, Gold,
Silver, Bronze, Iron) if a ranked mode is ever added.

**Career reference data (14:45).** Tyler collected the Reach career material from Halopedia and the Bungie.net
forums; it is now in `docs/career/`: `career-reference.md` (how Reach paid cR, campaign scoring, ranks, commendations,
challenges, Armory, loadouts, controller layouts, sources) and the data files `ranks_and_pay.json`, `challenges.json`
(all 18 months Sep 2010 – Feb 2012, 2,103 challenges; no month missing, only 14 days Halopedia itself lacks),
`commendations.json` (45) and `armor.json`. Names and numbers only, no Bungie art (rank/challenge icons must be read
from the MCC install; Tyler's rank-icon sheet stays local). Open question for Tyler: keep Noble's own cR numbers or
switch to Reach's measured table (`career-reference.md` §1).

**Armory appearance (current).**
- The Armory saves `data/appearance.json` under the resolved data directory. Slot 34 writes species, armor pieces,
  colours and the Elite armor set into the profile (see section 0, 09:15).
- The local OBJ preview resolves MCC MTL texture paths into the exported `bitmaps` tree, loads matching `_cc` masks,
  and applies the selected primary/secondary/detail colors and visor tint. It composes a Spartan base with selected
  MCC accessory groups, or an Elite body/undersuit layer followed by the selected rank harness group. The engine's
  own main-menu Spartan preview is unavailable because MCC has no Reach `mainmenu.map`; Armory uses Noble's DX11 OBJ
  renderer instead.
- The actual Spartan/Elite OBJ exports remain local-only under git-ignored `data\armor\`; never commit them. The OBJ
  model-group UI only offers geometry that exists in the local export. Reach's equipment IDs and live player color
  records are still unmapped, so piece/color selections affect the Armory preview, not yet the in-game character.
- A safe in-game equipment/color implementation needs a known-good profile capture with per-piece/color changes (or
  verified engine ID mappings). RR's profile contains candidate u32 lists at +0x40..+0x1AB, but their semantics are
  not confirmed. Ghidra 12.1 found no direct references to the `color_armor1` or Spartan helmet property strings, and the
  matching PDB was not present in the MCC install; static analysis has not established equipment or color IDs. Direct
  `.map` render-model decoding also remains future work.
- For an opt-in runtime capture, launch `noble-client.exe -capture-profile data/profile-captures`. Slot 34 writes the
  initial 64-KB profile buffer and any later changed buffer on subsequent Slot 34 calls as session-tagged, numbered
  `.bin` files; later runs do not overwrite earlier captures. This mode is disabled by default. Change one in-game
  setting at a time, retain the matching before/after files locally,
  and do not commit captures. If Slot 34 is not called again after a change, this mode cannot observe that change; Armory
  preview JSON selections are not included until they are mapped into the game profile. Compare matching captures by
  byte offset, substituting their actual session-tagged filenames:
  `$a=[IO.File]::ReadAllBytes('profile-<session>-0.bin'); $b=[IO.File]::ReadAllBytes('profile-<session>-1.bin'); 0..($a.Length-1) | ? { $a[$_] -ne $b[$_] } | % { '{0:X4}: {1:X2} -> {2:X2}' -f $_,$a[$_],$b[$_] }`.
- Forwarded community request (TRAXUSi, Workshop class mods): add extra player-host object types beyond Reach's
  three (0 spartan, 1 elite, 2 monitor), e.g. `elite2`, `spartan2`, `monitor2`, using the same host code paths, with
  matching Megalo names. Engine patch inside `haloreach.dll` at run time; not started, idea only.

**17:10 update: RR's real HUD colours (RenderDoc capture of RR, Spartan, Sword Base, frame 2662).**
- To play as a Spartan in RR, the profile matters: RR profile bytes **+0x27 and +0x38** are 1 for Elite, 0 for Spartan
  (only 5 bytes differ between the two profiles). Spartan copy: `D:\REACH REVERSE ENGINEER\rr_profile_spartan.bin`
  (RR data, never commit). RenderDoc: exe `D:\Reach\bin\host-probe.exe`, cwd `D:\Reach`, child capture off, args = RR's
  match line without `--show-behind/--exit-after-game/--hold-after-game` and without the three elite options.
- `CHUDWidgetPS` = Buffer 10679 (208 bytes = 13 float4, rewritten before every HUD draw). Rows 0-5 are
  chud_color_output_A..F; rows 6-12 are parameters (scale, timing, transforms). Values (0-255, R,G,B,A):

  | Row | Motion tracker (EID 20055, Draw 6) | Shield bar (EID 20084, Draw 18) |
  |---|---|---|
  | 0 A | 0,0,0,0 | 36,148,255,255 |
  | 1 B | 8,53,121,255 | 8,53,121,255 |
  | 2 C | 0,0,0,0 | 255,255,255,1 |
  | 3 D | 0,19,33,0 | 4,23,62,255 |
  | 4 E | 0,0,0,0 | 240,36,36,0 (red: low-shield colour) |
  | 5 F | 0,0,0,0 | 255,255,255,1 |

  Noble gets (0,0,1,0) in every row of every widget. **The colours differ per widget and per slot**, so a fixed
  replacement can't be right (that's why HudColor.cpp's sky blue gave the white block). The engine computes these on
  the CPU from the map's HUD (chud) data each frame; in Noble that computation returns 1 everywhere.
- Next (code work, no help needed from Tyler): in `haloreach.dll`, find the CPU code that fills this 208-byte buffer
  (follow the Map/Unmap of the CHUD widget constant buffer back to the colour evaluation), see what input makes it
  return 1 (likely a lookup that fails: a host answer, a profile field, or the player colour records RR writes,
  `looks::copy_colors`), and fix that instead of patching the buffer. Until then HudColor.cpp could use these values
  as a better stopgap, but that is a guess per widget.
- `armory-appearance.txt` (read 16:45): Spartan primary b3852a, secondary 000000, tertiary ff7a14, quaternary 6a2c00;
  Elite 3d6084; emblem colours 2fa59a/80ddda. None is the HUD blue, so the HUD isn't simply the armour colour.

**16:45 update: sky-blue HUD tested; next is an RR capture.**
- `HudColor.cpp` result (Sword Base, Forest): motion tracker, ammo, grenades, crosshair, ability icon now light blue
  like RR. Two regressions: the shield bar + compass background is a solid **white block** (several colour slots
  stack), and the scope mask outside the zoom view is a **light-blue haze** (it should stay dark). Every broken slot
  holds the same value (1), so one replacement colour can't be right. Not committed yet.
- Next: RenderDoc capture of RR for the real CHUDWidgetPS values (motion tracker, shield bar, scope mask). Launching
  `Reach.exe` with "Capture Child Processes" crashes RR's WebView2 menus, so launch the game process directly:
  - Exe `D:\Reach\bin\host-probe.exe`, working dir `D:\Reach`, child capture off.
  - RR's real launch line (16:41; `runs\<id>\match` is a per-run folder):
    `<run>\match --launch-block D:\Reach\data\launch_block_1.bin --armory-appearance
    %LOCALAPPDATA%\ReachRenaissance\armory-appearance.txt --profile <run>\profile.bin --strings
    D:\Reach\data\strings_en.json --elite-looks --features-on 6e --seconds 86400 --user-id <xuid> --user-name <name>
    --mcc "<MCC dir>" --map SwordBase --mode hr_4v4_team_slayer_dmr_ar_50points_tu --time-limit 12 --exit-after-game
    --extra-maps D:\Reach\data\armor --armor-pack rr_elite_pack --elite-armor-slots --service-tag ONYX
    --settings-file %LOCALAPPDATA%\ReachRenaissance\settings.txt --window-pos 100,100 --window 2560x1440
    --hold-after-game --show-behind <launcher hwnd> --map-variant "<MCC>\haloreach\hopper_map_variants\sword_base_default_031.mvar" --ingame-chat`
  - For RenderDoc: drop `--show-behind`, `--exit-after-game`, `--hold-after-game`; use a copied profile
    (`D:\REACH REVERSE ENGINEER\rr_profile.bin`, RR data: never commit).
- **New lead:** RR passes `--armory-appearance armory-appearance.txt` (player colours); HUD colours may come from
  the player's colours. Waiting for Tyler to paste that file.
- Loadout cards show grenades as "0000 / 0002": same zero-padding bug as the respawn timer (suspect the
  IsWhitespace hook in `EnginePatches.cpp`).

**16:30 update: dark HUD cause found (RenderDoc).**
- Capture of Noble on Sword Base (frame 1862, saved outside the repo; never commit `.rdc` files). The HUD is
  "Colour Pass #7" (EID 8935-9432, last pass before Present). Pixel history on the motion tracker: EID 8980 writes
  (0.04, 0.04, 0.08). Its pixel shader has two constant buffers: `ExposurePS` = g_exposure (0.5, 2.0, 1.0, 1.0),
  normal; `CHUDWidgetPS` (208 bytes) has **chud_color_output_A..F all (0, 0, 0.00392, 0)**, i.e. the integer 1
  read as ARGB. So the HUD colours the engine computes are an index/flag value, not a colour. Probably the same
  player/colour records RR rewrites in memory (`armory::write_colors`: four ARGB u32 >= 0xFF000000;
  `looks::copy_colors` hook), which would also explain why RR's profile file alone didn't help.
- **Stopgap (Tyler's call): `src/engine/HudColor.cpp`** hooks ID3D11DeviceContext::Map/Unmap (vtable 14/15) and,
  in 208-byte constant buffers, replaces colour slots holding exactly that broken pattern with sky blue #87CEEB
  (alpha kept). Installed in `main.cpp` after UIManager init; added to `CMakeLists.txt`. Real fix still to do: find
  what fills those colours (compare a RenderDoc capture of RR: same draw, CHUDWidgetPS values; then RR's
  `looks::copy_colors` / `armory::write_colors`).
- The 15:45 D3D11Renderer test (BGRA flag, 1 buffer) made the HUD black and was reverted (`git restore`).
- Mods screen rewrite (`WorkshopManager.*`) tested OK by Tyler.

**15:55 update.**
- Pushed `ebd9761` (on top of the partner's `ae7b71e`): career pay 13 cR/min, rank fix, `docs/career/`, fixed-size
  windowed mode (`Window.cpp`: no maximize/resize, since the engine can't change resolution after start; use
  BORDERLESS FULL SCREEN for full screen), Key Bindings no longer lists MCC's "other Halo games" actions.
- **Dark HUD:** not resolution (borderless 3840x2160 is still dark). Pixel samples: RR tracker edge (80,148,191) vs
  Noble (21,21,32), so the HUD colour is lost, not just dimmed. Same host slots as RR (RR's switch handles 1, 2, 4, 10,
  18, 32-34, 36, 39, 41, 43, 47, 48, 51, 52, 55-57, 88, 97, 116; slot 33 bytes identical; iface2 identical; RR gamma
  exponent default 1.0). Lobby patches are inactive (0 applied). **Test (reverted 16:05) in `src/host/D3D11Renderer.cpp`:**
  device flag `D3D11_CREATE_DEVICE_BGRA_SUPPORT` (RR passes 0x20) and RR's swap chain (1 buffer, flags 0). If the HUD
  stays dark, revert it; next suspects are RR's engine hooks (`resadapt` hooks 14 functions at haloreach+0x1BB6C0..
  +0x1BBD34 that read tag-block elements by u16 index; `hudx`, `widgetx`, `texcam`).
- **Respawn timer** "00000001": suspect Noble's `EnginePatches` IsWhitespace hook (returns 0 for chars < 0x09); the
  respawn string uses `\x07` property tokens. Test with the hook off.
- **UI to fix next (Tyler's order: HUD, then Mods screen and a clean Settings menu):** Mods: Halo 3 mods listed as
  Reach (read each mod's `ModInfo.json`: `"Engine"` and `"Title":{"Neutral":...}`), titles are the first map file
  name, ImGui ID clash (rows need `PushID(i)`), fixed pixel widths cut off at 4K. Controller picture is dotted/pixelated
  (scaling/alpha). Settings: lots of empty space, Video has only resolution and window mode (RR: FOV, V-Sync, quality,
  help text on the right). Party box says 1/16 (RR 1/64).
- Tyler added full Spartan and Elite rips (OBJ + bitmaps) at `D:\REACH REVERSE ENGINEER\spartans` and `...\elite`
  for the armory. Game assets: never commit them.

## 1. Where things are

| What | Path |
|---|---|
| **Repo (Noble)** | `D:\Halo-Reach-Community-Edition` (remote `https://github.com/O7OF/test-build-01`, branch `main`) |
| Built client | `D:\Halo-Reach-Community-Edition\build\bin\Release\noble-client.exe` |
| Run log / crash dump | `build\bin\Release\noble-client.log` (overwritten every launch), `noble-crash.dmp` |
| Noble data folder (git-ignored) | `build\bin\Release\data\` (`sandbox\`, `userdata\`, `mods\`, local test files) |
| Map-info sandbox (created at runtime) | `build\bin\Release\data\sandbox\haloreach\maps\info\` (the process `cwd` is switched to `data\sandbox`) |
| Repo docs | `docs\HANDOFF.md` (this file), `docs\hud-text-findings.md`, `docs\boot-investigation-2026-10-27.md`, `docs\architecture_plan.md` |
| **MCC (Steam)** | `D:\Steam\steamapps\common\Halo The Master Chief Collection\` |
| Engine DLL | `...\haloreach\haloreach.dll`, SHA-256 `738dd2d24ea3aea12e1ee9aa4a61094bf116027d42004c35a19e5048608b0894` (the only build RR supports; all RVAs below are for this build) |
| MCC game variants | `...\haloreach\game_variants\*.bin` (`slayer_054.bin`, `hr_ff_*.bin` for Firefight, ...) |
| MCC sound banks | `...\haloreach\fmod\pc\sfx.fsb`, `english.fsb` (found through host slot 48) |
| MCC binaries | `...\MCC\Binaries\Win64\` (ships `xaudio2_9redist.dll`, which `haloreach.dll` loads for sound) |
| Reach Renaissance install (reference only) | `D:\Reach` (`bin\host-probe.exe`, `bin\menu-overlay.exe`, `data\profile_1.bin`, `data\lb_m_c_1.bin`, `data\launch_block_1.bin`, `data\blocks\*.bin`, `data\menumaps\mainmenu_all.map`) |
| RR run logs | `C:\Users\tyler\AppData\Local\ReachRenaissance\runs\<timestamp>\` (`menu\`, `guest\`, `match\` host_probe logs, `*-args.txt`) |
| **RE workspace** | `D:\REACH REVERSE ENGINEER` (not a git repo) |
| RR analysis notes | `D:\REACH REVERSE ENGINEER\docs\rr-launch-flow.md`, `rr-tags.md` |
| RR pseudocode (Ghidra) | `D:\REACH REVERSE ENGINEER\docs\evidence\pseudocode\*.c` (`host-probe.exe__host__hit@0x14007c600.c`, `__main.c`, `__set_game_variant@0x1401c3450.c`, ...) |
| haloreach.dll index | `D:\REACH REVERSE ENGINEER\docs\evidence\haloreach\` (`haloreach_functions.csv`, `haloreach_strings.csv`, ...) |
| Omaha symbol tables | `D:\REACH REVERSE ENGINEER\docs\evidence\symbols\` (Xbox 360 addresses, names only) |
| Leaked 2010 build (source of Omaha symbols) | `D:\Halo Reach (Jul 24 2010) (Omaha) (Cache Build)`. Keep anything derived from it **out of the repo** |

**Tools installed:** Visual Studio 2022 Community + Desktop C++ (CMake bundled), Git, Node.js, **Ghidra 12.1.4** at
`C:\tools\ghidra_12.1.4_PUBLIC`, **JDK 21** at `C:\Program Files\Eclipse Adoptium\jdk-21.0.12.101-hotspot`
(`GHIDRA_INSTALL_DIR` and `JAVA_HOME` set). The partner's PC does **not** have Ghidra configured.

`host-probe.exe` (RR) still has Rust symbols: `llvm-nm -C` / `llvm-objdump -d -C` give real function names
(`host_probe::host::hit`, `host::hit2`, `host::load_profile`, `text::glyph`, `pad::fill`, `live::remap_keys`, `uicursor::*`, ...).
That is the fastest way to answer "what does RR do for slot N". `objdump` on `haloreach.dll` itself is also quick
(~2.5 M lines) for tracing engine code at an RVA.

## 2. Build and test (exact commands)

Use **Developer Command Prompt for VS 2022** (plain PowerShell has no `cmake`; in cmd use `cd /d` to change drive).

```
cd /d D:\Halo-Reach-Community-Edition
```
```
cmake --build build --config Release
```

* If `build\` is missing or CMakeLists changed a lot: `cmake -S . -B build -G "Visual Studio 17 2022" -A x64` first.
* The post-build step copies a top-level `data\` folder; if the repo has none the build "fails" after linking. Create it once:
  `if not exist D:\Halo-Reach-Community-Edition\data mkdir D:\Halo-Reach-Community-Edition\data` (the post-build copy should
  be wrapped in `if(EXISTS ...)` in CMakeLists).
* `LNK1104: cannot open file ...noble-client.exe` = the game is still running. Close it or `taskkill /IM noble-client.exe /F`.

Run (Steam must be running), from `build\bin\Release`:

| Command | What it does |
|---|---|
| `noble-client.exe` | Noble front-end menu (also for `-map mainmenu` / `-mode mainmenu`, which the C# launcher passes) |
| `noble-client.exe -map 20_sword_slayer -mode multiplayer` | Sword Base directly (most tested map) |
| `noble-client.exe -map m10 -mode campaign -difficulty 1` | Winter Contingency (0 easy .. 3 legendary) |
| `noble-client.exe -map ff50_park -mode firefight -difficulty 1` | Firefight on Beachhead (default game type Standard) |
| `noble-client.exe -map m10 -mode campaign -skulls 0x10` | campaign with skulls (MCC skull bits, here Blind; see §5 Skulls) |

Other args: `-variant <path to MCC game variant .bin>`, `-mvar <path>`, `-mouse-scale F` (mouse look per pixel, default 0.0008),
`-profile <file>` (load a captured profile instead of Noble's built-in one; relative paths are resolved against the exe folder).
Test-only args: `-menu-screen <main|campaign|skulls|firefight|custom|forge|servers|settings|armory|mods>` opens that
front-end screen, `-menu-tab <0-5>` picks the SETTINGS tab (1 = key bindings), `-menu-capture <frames>` saves `capture_<frames>.bmp` of the menu and exits; `-launch-poke 0x48=0x10,...`
writes u32 values into the launch block (for finding fields by experiment). `-capture-frames 300,3600` saves `capture_<frame>.bmp` next to the exe (and one shot ~120 frames after the
post-game panel appears); `-quit-after <seconds>` ends the run by itself; `-end-round-after <seconds>` ends the round (reaches the post-game results). Player settings (resolution, borderless, mouse,
invert, key overrides, volumes, menu sounds, player name, master-server URL, favourites) are in
`build\bin\Release\data\userdata\settings.json`, edited from the SETTINGS tab.

Unattended build without the developer prompt: `build\nb_build.bat` calls Visual Studio's bundled `cmake.exe` directly
and writes `build\nb_build_out.txt` (VsDevCmd hung when started from a script). `build\nb_run3.bat` = 12-minute Sword Base
test with captures. These helper scripts are under the git-ignored `build\`.

Read `build\bin\Release\noble-client.log` after a run. Useful lines: `launch state N`, `Slot 4 - EngineEvent`, `slot N first call`,
`FATAL EXCEPTION`, `FrontEndMenu: PLAY ...`, `Slot 48 ... .map`.

## 3. Current status

| Milestone | State |
|---|---|
| Find MCC, Steam ownership check, load `haloreach.dll`, init, start | ✅ |
| Sword Base and other MP maps render and play | ✅ |
| Keyboard/mouse input (move, look, jump, crouch, sprint/ability, fire, weapon/grenade swap, Tab) | ✅ |
| Scoped aiming | ✅ (look settings block in profile) |
| Sound (gunfire, footsteps, damage) | ✅ (profile volumes + XAudio2 redist directory) |
| Mouse cursor in Reach menus (class/loadout pick) | ✅ (Windows arrow; RR uses it too) |
| Pause menu (Esc: Resume / Quit to Menu / Quit) | ✅ (ImGui overlay, Noble style) |
| Loading screen while the map loads (was ~5 s black) | ✅ map name + moving bar (`PauseMenu::DrawLoading`), ends on the first slot-10 call (tested) |
| Settings tab (controls, key bindings, audio, display, player name) | ✅ `SettingsPanel`, saved to `settings.json`, applied at next launch |
| Menu look and sounds | ✅ `NobleStyle` (Reach-like palette, Bahnschrift), `MenuSounds` (synthesised clicks, 40 ms hover cooldown) |
| Front-end menu, Reach style (`FrontEndMenu.cpp`): MAIN MENU list (Campaign, Matchmaking*, Firefight, Custom Game, Forge, Theater*, Armory, Mods, Settings, Quit; *greyed), My Party box, button hints; Campaign (mission list + preview, difficulty, skulls, score multiplier), Skulls, Firefight / Custom Game (map + preview, game type, difficulty), Forge (MP launch with MCC's `00_basic_editing_054.bin`: Forge World loads with no score/timer HUD; switching to the editor (Monitor) is untested), Games (server browser), MCC-style Settings | ✅ screenshots of every screen checked |
| Menu / loading art (`UiArt`): player's `data\ui\custom\`, Noble's `data\ui\noble\`, else read from the MCC install at run time (nothing shipped) | ✅ campaign scenes as backgrounds, map previews, skull icons, controller picture |
| Skulls (launch block +0x48) | ✅ verified (Blind hides the HUD) |
| Cinematic / dialogue subtitles | ✅ SUBTITLES setting (on by default), hook on `haloreach+0xD5244`; tested on Winter Contingency's intro |
| Campaign missions (cutscene → gameplay) | ✅ tested Noble Actual (prologue, ends after cutscene) and Winter Contingency |
| Firefight | ✅ tested with Standard. Scenario names (from MCC's preview pictures): `ff50_park` = **Beachhead**, `ff10_prototype` = **Overlook** (the old front-end menu had them swapped; BlamEngine's name aliases "beachhead"/"overlook" are still swapped, the scenario names it maps are right) |
| Game end (engine reports results through host slot 6) | ⚠️ post-game panel shows the right cR (15:19 test), but its buttons can't be clicked: the engine stops sending frames about 7 s later (§0) |
| Memory | ✅ no leak during a match: 12-minute run, private bytes 3835 → 3852 MB, working set ~1.3 GB, flat |
| HUD / scoreboard text and icons | ✅ letters, numbers and weapon/grenade icons render (glyph layout and string slots fixed, see §5) |
| HUD colours (bars, compass, motion tracker) | ✅ fixed 17:20: host slot 97 must return the colour it is given (§0) |
| Key bindings: all 66 slots named | ✅ slot numbers = MCC's `GameAbstractButton` order (enum strings in `MCC-Win64-Shipping.exe`); `KeyBindings.cpp` groups them like MCC's CONFIGURE BINDINGS (movement, actions, vehicle, UI, Forge, Theater; 10 slots only other Halo games use). Crouch default is now Left Ctrl (MCC) |
| Career / cR (`src/career/Career.cpp`) | ⚠️ first version, needs a real match to confirm stat slots: reads MCC's game-results record at slot 6, pays cR, shows it on the post-game panel, saves `data\userdata\career.json`. See §5 "Game results" |
| Spire (`35_island`) crash ~15 s into a match | ✅ fixed by the text fixes: 3+ minutes of Slayer on Spire without a crash (13:30 run) |
| Servers tab | ⚠️ LAN list, own hosted game always listed, favourites, direct connect, optional master server (`tools\master-server`). **Joining** another player's game is not possible yet (engine net slots 41/43 not implemented); the UI says so |
| Armor Locker / Workshop Mods tabs | ⚠️ screens only; not wired to the engine |
| Engine main menu (`mainmenu.map`) | ❌ by design: MCC ships no `mainmenu.map` for Reach (RR ships its own modified 298 MB `mainmenu_all.map`, which Noble cannot) |

## 4. Repo state

Work from this session went up as separate branches/PRs, each building on the previous one:
`feat/input-slot36` → `feat/sound` → `feat/menu-cursor` → `feat/pause-menu` (merged with `main` on GitHub, includes the
partner's TextAtlas/GammaPass) → `fix/zoom-aim` → `feat/front-end-menu` (front-end menu, then campaign, Firefight and the
extra tabs). Check on GitHub which of these are merged into `main`. Uncommitted at the time of writing: `docs/hud-text-findings.md`
and this file, plus possibly the campaign/Firefight changes if they weren't committed (`git status` shows it).

**Uncommitted work from 2026-10-27 afternoon** (on top of the night session below): `src/ui/UiArt.*` (new),
`FrontEndMenu.*` (rewritten, Reach-style screens + skulls), `SettingsPanel.*` (MCC style), `NobleStyle.*` (hero/small
fonts), `PauseMenu.cpp` (loading art, Firefight names), `KeyBindings.*` (groups), `Settings.*` (subtitles, campaignSkulls,
menuBackground), `ReachHost.cpp` (subtitle hook), `BlamEngine.*` (skulls at +0x48 as u64, `-launch-poke`), `main.cpp`
(`-skulls`, UiArt), `FrameCapture.*` (`SaveNow`), `CMakeLists.txt` (UiArt, windowscodecs). Suggested extra PRs: (7) skulls +
subtitles (engine), (8) UiArt + Reach-style front end + MCC settings.

**Uncommitted work from the 2026-10-27 night session** (on disk in `D:\Halo-Reach-Community-Edition`, builds, nothing committed):
new `src/common/Settings.*`, `Relaunch.*`, `src/engine/KeyBindings.*`, `FrameCapture.*`, `src/network/MasterServerClient.*`,
`src/ui/NobleStyle.*`, `MenuSounds.*`, `SettingsPanel.*`, `tools/master-server/`; changes to `ReachHost.*` (text atlas, string
table, slots 6/32/33/51, settings, memory log), `main.cpp`, `Window.*`, `SessionManager.cpp` (LAN beacon crash), `ServerBrowser.*`,
`FrontEndMenu.cpp`, `PauseMenu.*`, `UIManager.cpp`, `MissionLauncher.cpp` (map names), `CMakeLists.txt` (new files, winhttp/winmm,
`if(EXISTS data)`). Suggested PR split: (1) LAN crash fix, (2) HUD text/icons + string table, (3) settings + key bindings + style
+ sounds, (4) server browser + master server, (5) loading/post-game overlay + Relaunch, (6) test tooling (FrameCapture).

Local test files that must **never** be committed (they're RR's data, all under the git-ignored `build\`):
`build\bin\Release\data\profile_1.bin` (copy of RR's captured profile) and `data\test_profile_*.bin` (bisection files). Delete
them when convenient; Noble no longer reads `profile_1.bin` automatically.

## 5. Engine facts (haloreach.dll build above)

**Exports / vtable**
| Item | Detail |
|---|---|
| `SetLibrarySettings(p)` | zeroed 0x1FE block, three UTF-16 locale strings |
| `CreateGameEngine(&engine)` | returns 0x46 |
| `Init` vtbl+0x08 | `(engine, ID3D11Device*, 0, IDXGISwapChain*, presenter=0)`; finds the HWND **of the calling thread** |
| `Start` vtbl+0x10 | `(engine, host, launch_block[0x2BF30])`; spawns the engine thread. The engine can't switch maps afterwards: RR and Noble restart the process (Noble: pause menu "Quit to Menu") |
| `PostCommand` vtbl+0x18 | `(engine, id, payload)`; 10 then 11 after ~300 ms |
| Present | done **by the engine thread**; host slot 1 runs first with `(swapchain, &flags)`. Host overlays (gamma pass, pause menu) draw there; never use the immediate context from another thread while the engine runs |

`haloreach.dll` loads `xaudio2_9redist.dll` by bare name during start-up: Noble keeps `SetDllDirectoryW(<MCC>\MCC\Binaries\Win64)`
set for the whole run (as RR does), otherwise there is no sound. Audio itself is FMOD inside the DLL.

**Launch state machine** `haloreach+0xC1A114`: 0…10 while launching, 11 = launched (reached ~5 s before the map is on
screen; the first slot-10 call marks the game session start). Failure strings table `0x1803A2DCC`
(17 `no map selected`, 18 `no game selected`, 19 `map load failure`).

**Paths:** map list from `FindFirstFileW` over `haloreach\maps\info\*.*` relative to cwd (hence the sandbox); files through slot 48.

**Launch block (0x2BF30 bytes).** Noble builds every block itself (`BlamEngine::BuildLaunchBlock`). RR's captured blocks for
menu, MP, campaign and Firefight are byte-identical apart from these fields (layout used as reference only):

| Offset | MP | Campaign | Firefight | Menu |
|---|---|---|---|---|
| +0x0C | 3 | 1 | 5 | 1 |
| +0x10, +0x14, +0x1D5C0 | launch index 190–209 | 178–189 | 210–219 | -2 |
| +0x28 | 0 | `01 00 11 11` | 0 | 0 |
| +0x34 | 1 | difficulty 0–3 | difficulty 0–3 | 1 |
| +0x1D334, +0x1D5BC (map id) | MP map id (Sword Base 1000, Boardwalk 1035, ...) | 1035 | 1035 | 1035 |
| +0x300 game variant (decoded with `haloreach+0x373A0`) | chosen variant (default `slayer_054`) | slayer | **a Firefight variant** (`hr_ff_*.bin`; RR loads `game_variants\<mode>.bin`) | slayer |

**Skulls: u64 at +0x48** in MCC's skull numbering, verified (Blind hides the HUD). `haloreach+0x3A22C` converts it to the
engine's own bits (game options +0x1D8, copied to game globals +0x1EAC8, which the `skull_enable` script command flips).
The MCC bit is the skull's index in MCC's alphabetical skull list: Iron 18, Black Eye 3, Tough Luck 36, Catch 7, Cloud 13,
Famine 11, Thunderstorm 34, Tilt 35, Mythic 23, Blind 4, Cowbell 8, Grunt Birthday Party 16, IWHBYD 19 (38 skulls in the
converter). Noble used to write skulls to +0x3C, which is a different
field the engine reads as u16; that was removed.

Also: +0x18 / +0x1D5C4 `0x88880000`, +0x50 random 16-byte launch id, +0x1D308 map variant area (init `haloreach+0x6BFF8`).
Launch indices: campaign 178 m05 Noble Actual, 179 m10, 180 m20, 181 m30, 182 m35, 183 m45, 184 m50, 185 m52, 186 m60, 187 m70,
188 m70_a, 189 m70_bonus; Firefight 210 ff10_prototype, 211 ff45_corvette, 212 ff20_courtyard, 213 ff60_icecave, 214 ff70_holdout,
215 ff60_airview (verified: loads Outpost), 216 ff50_park, 217 ff30_waterfront, 218 ff_unearthed, 219 cex_ff_halo.

**Host interface** (`host[0]` = 160-slot vtable, `host[1]` = secondary interface; args `(host, a1, a2, a3, a4)`).
| Slot | Meaning | Noble |
|---|---|---|
| 1 | per-frame, before Present: `(swapchain, &flags)` | gamma pass, then overlay (loading / pause / post-game), then test capture; memory log every 1800 frames |
| 4 | engine event `(reason, msg)` | logs |
| 6 | **game results** `(buffer, 0xD5A8, 0x14)`: called when a game ends (RR `results::on_game_results`) | shows the post-game panel (only once launch state is 11); results not decoded yet |
| 10 | return secondary interface | ✅ |
| 23 | load progress | ✅ |
| 32 / 33 | display settings / quality | 32: size from the window; 33: writes the 16 bytes `0x3F80000000000201, 0x2BF800000` **directly at a1** (as RR) |
| 34 | **profile**: pointer to a 64 KB block (see Profile below) | ✅ built-in defaults |
| 36 | **input poll** `(userIndex, u8 state[0x130])`, users 0–3 every tick (`haloreach+0x23AE7`) | ✅ |
| 47 / 48 | user-data folder / file path resolver | ✅ |
| 51 | feature query `(id, bool* out, u64* extra)`: RR answers from mask FEATURES_ON = 0x6E and zeroes `*extra` | same mask; the engine only asks 1, 2, 5, 6 (all on) |
| 55 / 56 / 57 | strings: clear / register `(table, id, const char* name, const char* suffix)` / lookup `(table, id, wchar_t* out, cap)` | key = CRC-32/MPEG-2 (poly 0x04C11DB7, init 0xFFFFFFFF, no xorout) of upper-case `name_suffix` (leading `$` skipped); text from `<MCC>\data\ui\Localization\EN_HaloReach.bin` (magic 0x90CE6B7A; u32@0x78 key bytes, keys from 0x80; u32@0x7C string block; NUL-terminated UTF-8 in key order). Lookup returns 0 when unknown so the engine uses the map's own strings |
| 61–67 | text: prepare, glyph, atlas, has_icon, kerning, register_icon, metrics | ✅ `TextAtlas`: 1024² pages (≤16), shelf packing, dirty-rect upload in slot 63 |
| 88 | identity `(u64* xuid, wchar_t* name, cap, userIndex)` → 1 | ✅ |
| 116 | controller map `(xuid)`; RR only logs | returns 0, writes nothing |
| iface2 147 | pointer to a 16-byte session GUID; every other iface2 slot returns 0 (RR does the same) | ✅ |

**Glyph record (slot 62 out, 26 bytes):** ch@0, zero@2..7, atlasX@8, atlasY@0xA, w@0xC, h@0xE, originY@0x10, originX@0x12,
advance@0x14, -ascent@0x16, page@0x18. Glyph alpha = min((gray·255+32)>>6, 255) from GDI GGO_GRAY8. Icons (0xE000–0xE2FF): the
engine registers one through slot 66 `(host, ch, size, f32 scale, face, w, h, top, ARGB4444-decoded pixels, pitch)` only when
slot 64 has_icon returns false (engine path `haloreach+0x1CF4D0` reads `font_package_icon.bin`); store the pixels unchanged,
originY = top, originX = 0, advance = w, ascent = 0.

**Input state (slot 36 buffer, 0x130 bytes, same as RR byte for byte).** +0x000 u32 1 (keyboard/mouse; 0 = gamepad);
+0x004+VK u8 per key (`0x80` pressed < 500 ms, then `0x81..0xFF` held); +0x104/+0x108 f32 mouse look; +0x114 wheel;
+0x118 f32 1.0; +0x11C mouse buttons; +0x120.. gamepad (RR `pad::fill`, not done). The engine reads keys through a 104-entry VK
table (`haloreach+0x98FDF0`) but maps them to actions **only through the profile's binding table**.

**Profile (slot 34).** RR returns a zeroed 64 KB block, optionally filled from a captured profile (`--profile`, first 0xC00 bytes),
with a 4-char UTF-16 service tag at +0x1AC. Everything the engine reads from a blank profile is 0, which broke several things.
Noble's built-in defaults (`ReachHost.cpp`):
| Range | Content | Effect when missing |
|---|---|---|
| +0x1B4..+0x1DF | look/controller settings (sensitivity bytes, dead zones 0.13/0.12, 1.0 multipliers, option bytes) | aim frozen while scoped |
| +0x42C..+0xA5B | key bindings: 66 × 0x18 `{u32 action, u32 VK, 16 zero bytes}`, MCC default PC layout | no key does anything |
| +0xA5C / +0xA60 / +0xA64 | f32 volumes (game / music / other) | no sound |
Still zero: rest of +0x000..+0x42B and +0xA68..+0xBFF (suspected to hold the HUD brightness/opacity setting).

**Subtitles.** Before drawing subtitles the engine calls `haloreach+0xD5244`, which walks the local players' table
(`haloreach+0x2A03C50`, stride 0xAB8, profile data at +8) and reads each player's subtitle setting at profile +0xA30
(0 automatic = only when voice and text language differ, 1 on, 2 off). That table is **not** a copy of the slot 34 buffer
(writing +0xA30 there does nothing; volumes etc. are read field by field). Noble hooks `0xD5244` (prologue `48 89 5C 24 08`)
and answers "on" when the SUBTITLES setting is on.

**MCC UI texture packs** (`<MCC>\Data\UI\texturepacks`, read by `UiArt`): `<name>.perm.bin` = records of 0x1E0 bytes, magic
0xCDBFA090, name at +0x44, u16 w/h at +0x74/+0x76, u64 size at +0x88, u64 offset into `<name>.temp.bin` at +0x90; data BC1 or
BC3 (decided by size per 4x4 block). Useful packs: `levelstexturepack` (every Reach map/mission preview, 316x160 and
668x292 `…LARGE`), `skullstexturepack` (128x128 skull icons), `controllertexturepack` (controller picture, button glyphs),
`mainmenutexturepack`, `globaluitexturepack`. `loadingtexturepack.perm.bin` (304 MB) has no names: entries
`{u32 0x5E73CDD7, u32 size, u32 size, u32 0, data}`, padded; 1920x1080 BC1. Reach's 12 campaign scenes are found by a CRC-32
of their first 4 KB (table in `UiArt.cpp`). `Data\UI\BackgroundVideos\Images\*.png` are only 392x220.

**Key-binding slots.** The 66 profile slots use the engine's own action numbering, which matches neither MCC list in
`Data\BuiltInData\HaloReachController.json`. Names in `KeyBindings.cpp` come from MCC's default keys (unique key or the
W/A/S/D block); the rest are shown as "OTHER (NOT IDENTIFIED YET)". Several defaults appear twice (two `R`, `F`, `LMB`, `Space`),
so e.g. Fire may be slot 8 or 27: test a rebind of each named action in game.

**Loadout names** come from the game variant (Slayer's default list is Sprint / Armor Lock / Jet Pack / Active Camo /
Hologram; Invasion on Spire shows its own tier list starting with Scout; Air Assault / Operator / Grenadier etc. belong to
such game types). They render correctly.

**UI cursor.** `haloreach+0x2D2018` runs only while a Reach screen that uses the pointer is up (RR `uicursor::install` hooks it,
prologue `48 89 5C 24 18`). Noble hooks it with MinHook; within 250 ms of a call the mouse is freed, the arrow shown and no
look delta sent. The engine reads the pointer itself (GetCursorPos/ScreenToClient).

### Game results (host slot 6)

`haloreach+0xF95CB` builds MCC's results record (**0x5D138 bytes**) with `haloreach+0x39128` from the engine's own
results at `haloreach+0x28B83B0`, then calls host slot 6 (`[host]+0x30`) with **a1 = pointer to the record**. a2 is not a
size: it is r8 left over from the conversion loop (always 0xD5A8). Layout read from `+0x39128`:

| Offset | Contents |
|---|---|
| +0x0000 u32 | game category (engine mode byte at results +5 mapped to MCC's list) |
| +0x0008, +0x0010 u64; +0x18, +0x19 u8 | copied from the engine header |
| +0x0020 | 16 player records × 0x1C8: +0x00 u8 present, +0x08 u64 player id, +0x18/+0x28/+0x2C i32, +0x30 u8, +0x34 i32 score (engine word or dword depending on mode), +0x38.., then appearance/name copy from +0x4C |
| +0x1CA0 | 16 × 4 bytes team data |
| +0x1CE0 | 16 per-player stat blocks × 0x5448: +0x000 23 u32 general stats, +0x05C 11 u32, +0x0AC 0x180 u32 medal counts, +0x6AC 0x32 u32, then weapon tables (0x54 weapons, 6 values each) |
| +0x56160 | per-player pairs; +0x5D120/+0x5D124/+0x5D128 u32 and +0x5D130 u64 match totals |

General-stat numbering: `haloreach+0x3B208` maps engine stat n to MCC slot (0→0, 1→1, n→n+1 for n ≥ 2). Assuming Reach
kept Halo 3's statistic order, slots 8/9/10/11 = score/kills/assists/deaths (**provisional**, `Career.cpp` kStat*).
Medal numbering: table built by `haloreach+0x39B00` (0x91 engine medals → MCC ids ≤ 0x17F).

Test option `-end-round-after <seconds>` calls the `mp_round_end` script command's work function
(`haloreach+0x7B3EC(-1, 0, 1)`) from slot 1, so automated runs reach slot 6. Noble writes the last record to
`data\userdata\results\last_game.bin` (local only: it holds player names and ids; never commit it).

## 6. Known problems and next steps (in order)

1. ~~**HUD bars, compass ticks and motion tracker are dark navy**~~ **Fixed 17:20 (slot 97, see §0).** Old notes: (letters/icons are fine). The game-type intro box shown
   during loadout selection, the "Visegrad Communications Outpost" waypoint box and the hint bar in Winter Contingency have
   the same dark tint (in MCC that box is bright cyan), while the loadout cards are correct. Ruled out: gamma, profile (RR's
   `profile_1.bin` gives the same picture), slot 51 mask, slot 33. **Confirmed 15:10:** RR's own HUD (Firefight, Beachhead)
   is bright light blue (shield bar, compass, tracker, timer box) while Noble's on Sword Base is dark navy; the world looks
   the same in both. The `GammaPass` in `ReachHost.cpp` is fixed at exponent 1.0 (a no-op; a whole-frame gamma washed the
   world out), so the fix must touch only the HUD. RR has a separate "UI Gamma" setting next to Gamma, which hints at a
   HUD-only value the host supplies. Next: look at how the HUD shader gets its colour/tint (chud globals, possibly a
   render-state or a constant the host sets in another slot, e.g. iface2 slots RR answers non-zero).
   Background (Bungie's Halo 3 HUD spec, cand.land/halohud): HUD element colours, flashes and scale come from the
   map's HUD tags ("HUD globals" / chud), the HUD is drawn mapped onto the inside of a sphere with a virtual camera
   and a per-resolution FOV from those globals, and layout uses anchors (4 corners + centre) with pixel offsets. So
   the colour is tag data the engine reads itself; if RR is bright, look for a host answer that changes which
   resolution/exception block or render target format the engine picks, rather than a colour value.
2. ~~Spire crash~~: no longer happens after the glyph/string fixes (3+ minutes on Spire, 2026-10-27 13:30).
3. **Memory**: Noble logs `Memory: frame N private X MB, working set Y MB` every 1800 frames. A full 10-minute match was
   flat (+17 MB). If a leak is still seen, it is probably across front-end menu ↔ match restarts or in the menu itself.
4. **Online joining**: engine networking slots 41/43 (and session setup) are not implemented, so the SERVERS tab can host
   (listed on LAN/master server) but not join. This is the big missing piece for multiplayer.
5. **Scoreboard/results**: the slot 6 record is decoded far enough for career pay (§5 "Game results"). Still to do: a
   post-game scoreboard from it, confirm the general-stat slots with a real match, campaign "next mission".
6. ~~Mouse wheel~~ done 18:18 (see §0). Gamepad (XInput into +0x120.., RR `pad::fill`) still to do.
7. Armor / Workshop tabs; remove debug-only bits when no longer useful.
   Menu ideas from Bungie's UI director (cand.land/new-page): menu backgrounds were layered concept art on cards with
   a slowly moving camera (parallax), and the main menu had a one-press "fast path" to the last mode played.
8. **Cursor lock** (fix in 18:18 commit, untested by hand): in windowed mode the game keeps the cursor clipped to its window, even while dragging the title bar,
   so the window can't be moved back (seen 15:14, the window ended up behind the taskbar). Only Alt+Tab frees it. Release
   the clip while the window is being moved or isn't focused (`WM_ENTERSIZEMOVE` / `WM_KILLFOCUS`), and re-clip on focus.
9. **Respawn timer** shows "Respawn in 00000001": the number filling `camera_list.prop_respawn_time` is zero-padded.
   Check the format used for that string (string table entry `0x84091d25` in `ReachHost.cpp`).
10. ~~Player name / service tag~~ done 17:58: the name comes from SETTINGS > GAMEPLAY > PLAYER NAME (slot 88) and the
   service tag from the new SERVICE TAG field (`Settings::serviceTag`, 1-4 letters/digits, default NOBL; written to
   profile +0x1AC).
11. **Forge**: the menu launches MP with `00_basic_editing_054.bin`; the map loads, check in game that the Monitor/editor toggle works.
9. **Career system** (planned with Tyler). Done: `Career` module, 50 ranks (Halopedia table), cR after each match
   (per-minute pay rising with rank, kills/assists/medals bonus, 1-in-8 random bonus 100–3000, 1-in-200 jackpot 7,777;
   **no daily cap** and up to **90 paid minutes** per match in every mode (Tyler's calls). Pay goes by **base type** =
   the engine game mode BlamEngine launches with (campaign / Firefight / multiplayer incl. Forge), so modded maps and game
   types earn exactly what their stock type earns (`Career::BeginMatch` is called from `BlamEngine::LaunchMap`); all constants at the top of `Career.cpp`, Noble's own numbers), post-game cR breakdown + rank bar.
   Next: confirm stat slots (play a short Slayer with a few kills; `last_game.bin` + the `Career:` log lines), campaign
   scoring (difficulty + skull multipliers, time bonus, par scores), 45 commendations, challenges (Sep 2010 – Feb 2012
   lists) paying cR, a career/service-record screen and rank icon in the menu, Armory with the armor list.
   All the data for this is in `docs/career/` (see §0).

## 7. Partner's Outbreak-Z code (reference for Noble's menus)

The partner (`O7OF`) wrote and owns a Godot game, Outbreak-Z, and shared its scripts as reference material. They are
GDScript for Godot, so none of them compile into Noble (C++ / D3D11 / ImGui). They can be used as designs to port; because
the partner owns them, porting or copying them into this repo is fine. Keep the copies of these files outside the repo
unless one is actually ported.

**Usable almost as-is**
- `master_server.js` + `package.json`: **ported** to `tools/master-server/` with `MasterServerClient` in Noble. The
  original already had a 1 MB body limit; the real gaps were that anyone could heartbeat/delete any session and that the
  listed address came from the request body. Now: secret token per session (`X-Session-Token`), address from the socket,
  4 KB body, 4 sessions per IP, 120 requests/min per IP, 2000 sessions, text sanitised. `node test_master_server.js` passes.

**Port as designs**
- `server_browser.gd`: **ported** (favourites, direct connect, filters, host dialog with password). Ping and join need
  networking.
- `settings_manager.gd`, `build_settings_menu.gd`: Gameplay / Controls / Keybinds / Controller tabs with sliders. In Noble
  the controls write the profile fields from section 5: look settings at +0x1B4, key bindings from +0x42C,
  volumes at +0xA5C/+0xA60/+0xA64. **Ported** as the SETTINGS tab (`SettingsPanel`), controller tab not yet.
- `glass_ui.gd`: **ported** into `NobleStyle`.
- `audio_manager.gd`: only the menu parts apply. Click / hover / tab sounds with a 40 ms hover cooldown, and a menu music
  playlist (in order or shuffled, loops, no repeat of the last track when a shuffle restarts). Play through XAudio2, which
  is already loaded. In-game sound is the engine's own FMOD and needs nothing from this file. Sound source: Reach's
  sounds can't be committed (public repo); either read them at runtime from the player's MCC install (FMOD sound banks)
  or use our own / royalty-free sounds plus a `data\menu_music\` folder for the player's own files. Click/hover/tab sounds
  are **ported** as synthesised blips (`MenuSounds`); music not done.
- `game_loading_screen.gd`: **ported** as the loading overlay. The engine's slot 23 progress stays at 5%, so the bar only
  shows activity, not real progress. (`ui/LoadScreen` is never drawn while the engine runs.)

**Not applicable to Reach**
- `barracks_menu.gd`, `save_stats_manager.gd`, `floating_damage_number.gd`: Outbreak-Z skins, capes, cards, prestige and
  zombie stats. Reach armour lives in the engine/profile.
- `dev_tools_menu.gd`: zombie-mode cheats (points, perks, power-ups).
- `dev_account_search_menu.gd`, `backend_client.gd`: need Outbreak-Z's account database; its login/register ignore the
  password, so don't carry that over.
- `hud.gd`: Reach draws its own HUD.
- `scoreboard.gd`: Reach's in-game scoreboard is the engine's; a post-game one would read the slot 6 results buffer.
- `chat_box.gd`: the engine does **not** draw chat outside MCC; a chat box would have to be Noble's own overlay, and needs
  networking first.
- `check_api.gd`: one-line test script.

**Status:** settings, style, menu sounds, server browser, master server and loading screen are ported (see §3). Left:
controller settings tab, menu music, chat (after networking).

## 7b. Partner's server (`server/`, commit `ae7b71e`) - review notes for O7OF

Accounts (scrypt + random tokens), SQLite tables (users, tokens, player_stats, matches, match_player_records),
session registry, matchmaker (8 playlists). DB goes to `server/data/` (git-ignored). Left for the partner to fix:
1. `server.js` is missing from the commit (`package.json` "main"/"start" and `node server.js` point to it).
2. `node:sqlite` needs Node 22.5+; `package.json` says `>=18`. The catch-block fallback requires `node:sqlite` again.
3. `npm test` points to `test/test_backend.js`, which isn't there.
4. `ranks.js` doesn't match `Career.cpp`: base pay 60 (game: 13 cR/min), Major Grade 2 370,000 (game: 375,000), random
   bonus 100-999 (game: 100-3000 in steps of 10), jackpot only on roll 77 of 200 (game: plain 1-in-200), +250 win bonus
   (not in the game). Both spell "Field Marshall"; Reach's rank is "Field Marshal".
5. For matchmaking stats the server should be the only place cR is calculated, from the host's match results;
   otherwise a client can submit any total.

## 8. Rules for this project

- Read-only toward MCC files. Writes go to `build\bin\Release\data\…`.
- Never commit or publish: RR binaries/data/decompiled code (including `profile_1.bin`, launch blocks, `mainmenu_all.map`),
  Omaha-derived files, `haloreach.dll` or dumps of it (e.g. dumpbin output), crash dumps, logs (they contain a player name/ID).
  The repo is **public**. GitHub rejects files over 100 MB.
- Exception (Tyler, 2026-10-28): the Reach menu art in `build/bin/Release/data/ui/reach/` is committed on purpose.
- Keep the Steam ownership check. No anti-cheat bypassing, no official MCC online services.
- Small, separate PRs. The partner edits `ReachHost.cpp`/`BlamEngine.cpp` too, so tell him before large edits; when both sides
  change `HostSlot1_FrameTick`, keep the gamma pass first and the pause menu after it.
- Before overwriting a file, re-read the current version first, and re-check it a minute later (file sync has put stale copies
  back before).
- Commits, PRs and docs carry only the authors' names (no "generated with" or co-author lines).
