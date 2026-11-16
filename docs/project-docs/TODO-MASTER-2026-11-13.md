# Project Noble - master to-do (updated 2026-11-14 06:45 ET)
(Copy of claude.ai project doc. Newer: C:\Users\tyler\Desktop\TODO-MASTER-2026-11-14.md, copied to ..\desktop-guides\, which adds the 10:45 merge state.)

Branch wip-merge (pushed). Latest: c9f2630 Trooper mode prototype, c92b451 Marine pieces proven, 7f71ec4 release zip ships Noble's Forge World. Details: handoff on Tyler's PC (claude-tests-2026-11-11\HANDOFF-ELITE-2026-11-12-NIGHT.md, sections from "2026-11-14").

## Tyler's decisions (2026-11-14 01:32)
- Option 1 (ship the rebuilt Forge World) - DONE and proven. Option 2 next (see notice).
- Swing / 501st: dropped. Parked: multiplayer two-PC test, flashlight, other Armory items. Hand tests: Tyler later.

## Done 2026-11-14
- Forge World with Pelican, Spectre, Boathog, targeting laser on every normal Forge World launch; release zip ships it.
- Dev switches removed (only NOBLE_MARINE_TEST + -armory-* capture flags remain, for test scripts).
- Elite pack 486 MB -> 286 MB, live, A/B identical.
- Trooper Firefight Marine pieces WORK (proven with ODST pieces).
- Trooper mode prototype: -trooper -third-person makes the campaign player a UNSC Marine in third person (proven on Tip of the Spear: marine biped, plays and fights; tr35h captures).
- Test windows always open on the AOC (second monitor).

## Needs Tyler's decision
- Releases ship data\armor + data\menumaps from the REPO data folder: the old RR pack (99 MB) and mainmenu_all.map. Players don't get noble_elite_pack.map (now 286 MB) or noble_menu.map. Copy them into the repo data folder (git-ignored) before publishing if they should ship.

## NOTICE - Option 2 to do after Option 1
Make the Pelican, Spectre, Boathog and targeting laser an add-on pack injected into ANY map's Forge palette (like ElitePack for Elites: SharedTagCache + InjectScenarioPalette / data\forge_palette.json), instead of shipping a whole 750 MB rebuilt Forge World. The noble_pack approach (stripped scenario) could keep it small.

## Trooper mode - next steps
Target: Campaign -> Vanilla / Trooper / Modded; Firefight -> Spartan / Trooper / Modded; MP unchanged. Third person, cutscenes untouched. Civility (Siegebreaker) preferred as the animation base.
1. Menu option (Campaign: Vanilla / Trooper / Modded) instead of the -trooper launch option; Trooper turns third person on automatically.
2. Shields / health (HUD shows the shield WARNING bar).
3. Marine pieces on stock maps (Marine Armory picks): stock maps are compressed, so the string-id lookup finds nothing; needs another string-id source.
4. Firefight: stock Firefight maps have no marine biped -> needs a pack (like ElitePack) or the Trooper Firefight mod.
5. Missions other than m35 and co-op: check.

## Old bugs to double-check (hand checks)
- Forge on a Firefight canvas: fixes landed upstream (e119116). Custom-map missions: work landed (6afd742).
- "Respawn in 00000001" number padding and pause not freezing: no fix in the code.

## Parked
- Flashlight (merged into HREK spartans.biped, L key; open value reset to 0 every update in multiplayer).
- Multiplayer two-PC test. Armory: knife chests, emblem, undersuit colour, Elite shine/lights.
