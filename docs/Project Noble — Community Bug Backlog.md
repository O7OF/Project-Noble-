# Project Noble — Community Bug Backlog

Oct 26, 2026 · @Tyler

## About this list

These are 38 problems players reported on Discord between Oct 24 and Oct 26, 2026, while playing **Reach Renaissance**, the existing community launcher (one report names its `host-probe.exe`). They aren't Project Noble bugs yet, since Noble doesn't boot. They show where a from-scratch host is likely to trip, so each row says what to check once Noble runs.

Reports are grouped by area and merged when several people hit the same thing. Off-topic chat and answered how-to questions are left out. Reporter names are left out too; the original thread has them if you need to follow up.

## Fix first once Noble boots

These five either block play or point at host code Noble is writing right now.

1. **Campaign crash on Winter Contingency (m10).** Reach Renaissance hit a "Game Error" there in a 2-player hosted game, and m10 is the mission Noble's current test crashes on. Test m10 solo and with 2 players.
2. **Finding MCC on any drive.** Players asked if Reach must be on C:. Noble's log only checks `c:/program files (x86)/steam`. Read Steam's `libraryfolders.vdf` for every library, fall back to a folder picker, and consider the Xbox app install too.
3. **Antivirus blocking the host.** One player's antivirus shut down `host-probe.exe`. Code that loads a game DLL and patches memory looks suspicious to scanners. Plan for code signing and a README note.
4. **Input settings that do nothing.** Controller sensitivity and scroll-wheel weapon switching both fail. Wire these through the host's input handling and test early.
5. **Challenges and stats only credit the host.** Decide whether Noble tracks progress per player at all before building on it.

## Install and launch

| Bug | Reports | What players saw | What to check in Noble |
| --- | --- | --- | --- |
| MCC not found unless on C: | 3 | One player asked if Reach must be on C:. Another runs it from D: fine. A third suggested looking for the game files directly so the Xbox app version works. | Search every Steam library, then the Xbox app location, then a folder picker. |
| Antivirus stops the host | 1 | Antivirus shut down `host-probe.exe`. | Sign the exe. Avoid packers. Document an exclusion. |
| Menus never appear | 1 | Menus stay missing with WebView2 installed. | Noble uses ImGui, not WebView2, but should still show a clear error if the UI fails to start. |
| Hosting fails with "couldn't join game" | 1 | Pressing host shows "couldn't join game" and nothing starts. | Log every step of starting a hosted game so failures say why. |

## Input and controls

Two of these were seen under Proton on Linux and may not happen on Windows.

| Bug | Reports | What players saw | What to check in Noble |
| --- | --- | --- | --- |
| Controller sensitivity does nothing | 2 | Moving the sliders changes nothing in game. | Confirm the setting reaches the engine's input settings. |
| Scroll wheel doesn't switch weapons | 2 | Rebinding works for other keys, not the wheel, even after a restart. | Make sure mouse wheel messages reach the engine's input. |
| Keyboard dead in main menu | 1 | Only the mouse works in the main menu (Proton). | Test menu navigation with keyboard only. |
| Pause menu won't close | 1 | Stays open until alt-tab (Proton). | Test pause open and close with mouse, keyboard and controller. |
| Can't tab back in | 1 | After alt-tab, clicking the game doesn't refocus it; only the taskbar does. | Handle window activation and focus messages in the host window. |

## Menus, lobby and party

| Bug | Reports | What players saw | What to check in Noble |
| --- | --- | --- | --- |
| No way to leave a party | 1 | No leave option in the menu. The workaround is Friends > Party > your name, and a party leader must kick everyone. | Add a visible Leave Party option, including for the leader. |
| No quit to desktop | 1 | Players use Alt+F4 or end the task in Steam. | Add Quit to the main menu. |
| Can't lock lobby size | 1 | Hosts can't cap the lobby at a chosen player count. | Add a max-players setting for hosts. |
| Invite-only doesn't work | 1 | Lobby privacy set to invite-only has no effect. | Enforce privacy when the server is listed and when joins arrive. |
| Only the host can change teams | 1 | Other players can't switch teams. | Decide who may change teams and expose it in the lobby. |
| Map images sometimes missing | 1 | Stock maps sometimes show no image. | Check the map image lookup against the `maps\images` files. |

## Gameplay and visuals

Several of these involve Elites, which Reach Renaissance changes with its own armor pack and patches. Noble should test Elites as a separate case.

| Bug | Reports | What players saw | What to check in Noble |
| --- | --- | --- | --- |
| Crash on Winter Contingency | 1 | "Game Error" crash in a 2-player hosted public game. | Same mission as Noble's current crash. Test solo and co-op. |
| Spartans can't assassinate Elites | 3 | Elites can assassinate Spartans, not the reverse. One player says it only happens on Assassination Tower. | Confirm whether it's map-specific before patching. |
| Elite shields recharge slowly | 1 | Much slower than in MCC. | Compare recharge time to MCC with no patches applied. |
| Armor effects don't show on Elites | 1 | Effects only appear on death (for example Grunt Birthday Party). Pestilence leaves a splatter without the visual effect. | Test each armor effect on Spartan and Elite. |
| Elite first-person model wrong | 1 | Players are waiting for a fix to the Elite first-person view. | Check the first-person model when playing as an Elite. |
| Parafoil chest knife misplaced | 1 | The knife on HP Parafoil sits top-left of the chest instead of horizontal in the middle. | Check armor attachment placement against MCC. |
| Half MCC's frame rate | 1 | About 30 fps on low settings where MCC runs 60. Others report heavy stutter. | Profile the host's present loop and frame limiter early. |

## Challenges, stats and service record

This is the most-reported area. It's likely one root cause: progress is recorded only on the hosting machine.

| Bug | Reports | What players saw | What to check in Noble |
| --- | --- | --- | --- |
| Challenges only credit the host | 3 | Campaign, Firefight and multiplayer challenges complete for the host only. | Decide where progress lives and send each player's results to them. |
| Only Firefight challenges track | 1 | Campaign and multiplayer challenges don't track at all. | Test one challenge per mode. |
| Service record misses kills, deaths, assists | 2 | Only games played is counted. | Check that end-of-game stats are read and saved per player. |
| Pink Mist won't complete | 1 | Supercombine kills in Firefight, online and offline, don't count. | Use as a test case once challenges work. |
| Roast 'em Toast 'em doesn't track | 1 | Challenge progress never moves. | Use as a test case once challenges work. |

## Multiplayer, Firefight and dedicated servers

The Invasion problems matter for Noble's 32-player goal. Reach Renaissance already struggles past 20 players.

| Bug | Reports | What players saw | What to check in Noble |
| --- | --- | --- | --- |
| Lag from about 20 players | 1 | Invasion lobbies start lagging around 20 players. | Load-test before raising the player cap. |
| Players stay behind after map vote | 1 | In Invasion, after the vote, only the host moves to the next map. | Test the map-to-map transition with several clients. |
| Text chat broken on dedicated servers | 1 | Chat doesn't work on dedicated servers. | Test chat on listen and dedicated servers. |
| Firefight Versus doesn't work | 1 | The mode exists but doesn't run. | Test Versus once Firefight loads. |
| Ending a game early kicks everyone | 2 | When the host ends a Firefight or custom game, everyone is kicked. Games that end on their own keep the lobby. | Return all players to the lobby whichever way the game ends. |
| New process for every game | 2 | Each move between menu and game starts a new process, which breaks Discord screen share. Project Reclaimer doesn't do this. | Keep one host process and reload the engine inside it. |

## Forge, game types and mods

Two reports come with test files (`ren_test.bin` and `ren_test_no_begin.bin`, two 8.6 KB game types), useful once Noble loads custom game types.

| Bug | Reports | What players saw | What to check in Noble |
| --- | --- | --- | --- |
| Can't pick a game mode in Forge | 1 | The game mode menu only offers Import, so objectives like CTF flag stands can't be placed. | Let Forge load any game type, including objective modes. |
| Saving creates junk entries | 1 | Each save adds many undated entries that can't be deleted. | Check save naming and timestamps, and allow deleting saves. |
| Megalo `begin` opcode missing | 1 | Scripts using `begin`, added to MCC in July 2023, seem to skip those sections. | Reach Renaissance also runs MCC's engine, so the import step is the likelier cause. Confirm with the test files. |
| MegaloEdit game types won't import | 1 | Custom game types fail to import, with or without `begin`. Stock ones import fine. | Test import with the two attached files. |
| Forge maps on modded maps can't be picked | 1 | A Forge map built on a mod map says the mod isn't loaded, and with the mod loaded it isn't listed. | Tag Forge saves with their mod and list them when that mod is active. |

## Feature requests

From the suggestions channel, Oct 25 to 26. Armor customization and better performance were asked for most. Joke requests, Discord server ideas and offensive suggestions are left out.

| Request | Area | Asked by | Note for Noble |
| --- | --- | --- | --- |
| Better performance, matching MCC | Performance | 4 | Players report lower fps and stutter on the same settings. Add exclusive fullscreen and a render-resolution option. |
| More armor slots: both wrists, thighs, waist, backpacks, up to 3 helmet attachments, any attachment on any helmet | Armor | 3 | Inspired by ElDewrito's Reach Anniversary pack. Large job: needs engine and tag work. |
| Higher Forge object limit | Forge | 3 | Reach Renaissance already raises the budget. Check what else caps object count. |
| Firefight Versus teams | Firefight | 3 | Also in the bug list as not working. |
| Faster credit progression: bigger payouts or cheaper items | Progression | 2 | A config value once progression exists. |
| Per-piece armor colors for Spartans, like Elites have | Armor | 2 | Example: Jorge's yellow chest with green and red elsewhere. |
| More armor from MCC and other games: Mariner, GRD, SPI | Armor | 2 | A mod has added SPI by replacing stock armor. |
| Quit button in the main menu | Menus | 2 | Also in the bug list. |
| Firefight map rotation | Firefight | 2 | Rotations exist for multiplayer only. |
| Joke and mythological items as an option, off by default | Armor | 2 | Keep fun items toggleable. |
| Keep flame effects available to everyone, with a choice of color | Armor | 2 | Players don't want them restricted, as H3 Reclaimer did. |
| Smoother menu-to-game transitions and the original menu scroll animations | Menus | 1 | Ties to the new-process bug. |
| Delete playlists | Menus | 1 |  |
| Axial and radial aim toggle, as in MCC | Controls | 1 |  |
| Weapon view offsets | Controls | 1 |  |
| More or customizable armor effects | Armor | 1 |  |
| Noble Team decals | Armor | 1 |  |
| Community-wide goals, such as 10,000 headshots across all players | Progression | 1 |  |
| Mac support | Platforms | 1 |  |

## Reports missing details

These four relied on screenshots that didn't come through in the copied text. Check the original Discord posts.

- An error asking whether Reach must be on the C: drive (Oct 24, 12:01 PM).
- A post with no text at all (Oct 24, 5:26 PM).
- A message shown while the player was already in their own dedicated server (Oct 25, 1:38 PM).
- A "How do I fix this?" post (Oct 25, 6:10 PM).
