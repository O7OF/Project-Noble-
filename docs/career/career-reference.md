# Career reference: Halo: Reach credits, ranks, challenges, commendations, armory

This file holds the reference material for Noble's career system (`src/career/Career.cpp`). The tables are in the
JSON files next to it:

| File | Contents |
|---|---|
| `ranks_and_pay.json` | 50 ranks with total cR, per-minute pay per rank, game-type rates and minute caps, bonuses |
| `challenges.json` | every daily and weekly challenge from 14 Sep 2010 to 16 Feb 2012 (2,103 entries, 195 different names) |
| `commendations.json` | all 45 commendations with tier counts and cR per tier |
| `armor.json` | the Armory: every Spartan item per slot with rank and cR price, Elite sets, Firefight voices |
| `controls.json` | every input action (MCC config number, keyboard and gamepad defaults), 360 button and stick layouts, MCC setting ranges |

**Sources.** Halopedia (Rank (Halo: Reach), Challenges/Halo: Reach and the monthly pages, List of Halo: Reach
commendations, Armor customization (Halo: Reach), Loadout, Reach:Skulls, Reach:Control schemes, MCC:Control schemes,
Campaign scoring); Halo Fandom (Commendation (Halo: Reach)); Bungie.net forum post 2895104 "Complete Credit Breakdown
Per Rank & Game Type" and post 2534934. Halopedia text is CC BY-SA, so this repo keeps only names and numbers (facts),
in our own wording, with this credit. **No Bungie/343 artwork is committed**: rank, challenge and commendation icons
must be read from the player's MCC install at run time like the other menu art (`UiArt`).

---

## 1. How Reach paid cR after a game

Bungie never published the formula. Players measured it (forum post 2895104, with corrections from the same thread):

**cR = per-minute pay × game-type rate × whole minutes played (capped) + performance bonus + slot machine**,
plus commendation tiers and challenges as they complete. Daily cap 200,000 cR.

- **Per-minute pay (rate 1)** depends only on rank: 5 cR at Recruit to Sergeant Grade 2, 6 Warrant Officer, 7 Captain,
  8 Major, 9.2 Lt. Colonel (10 at Grade 3), 15 Commander … 60.9 Inheritor (`ranks_and_pay.json`). From Field Marshall
  up each rank adds 10 %.
- **Game-type rate** multiplies that: rate 1 = ×1, rate 2 = ×2.8, rate 3 = ×5.4, rate 4 = ×5.6, rate 5 = ×7.28.

| Rate | Minutes paid at most | Game types |
|---|---|---|
| 1 | 10 | solo and co-op campaign, Firefight (non-matchmaking), custom games, Forge |
| 2 | 75 | matchmaking co-op campaign |
| 2 | 15 | Score Attack |
| 3 | 15 | Invasion |
| 4 | 20 | Firefight Doubles, Firefight Limited, Firefight Arcade |
| 4 | 12 | Team Slayer, Squad Slayer, Arena, Rumble Pit, Living Dead, Team SWAT, Team Snipers, Double Team |
| 4 | 15 | BTB All Stars |
| 5 | 12 | Premium Slayer, Premium Battle (Anniversary BTB possibly rate 5, 15 min) |

- **Performance bonus**: matchmaking 1st place +20 % of the game pay (+10 % if tied), top half +10 %; Firefight +20 % if
  the score beats par (50,000 on every map).
- **Slot machine**: a random 100–3,000 cR for the whole lobby; a per-player jackpot up to 7,777; during promotions a
  "super jackpot" of 7,000–30,000 and double/triple slot-machine weeks.
- **Offline / custom**: about 50–60 cR per game once the account had been online (anti-idling).
- Worked examples from the thread: Recruit, Firefight Arcade, 12 min → 28 × 12 = 336 cR; Warrant Officer, Forge,
  17 min → 6 × 10 = 60 cR; General Grade 4, matchmaking co-op campaign, 82 min → 65.75 × 75 = 4,931 cR.

**Noble today** (`Career.cpp`): 13 cR/min at Recruit (Tyler's call, 27 Oct) + 6 per rank, up to 90 minutes in every
mode, kills 10 / assists 5 / medals 25, a 1-in-8 random bonus of 100–3,000 and a 1-in-200 jackpot of 7,777, no daily
cap. 13/min is between Reach's rate 2 (14) and rate 1 (5) at Recruit; the +6 per rank grows much faster than Reach's
table (Inheritor would get 13 + 49 × 6 = 307/min, Reach about 61 × rate).

**Original post-game cR screen** (from Tyler's 360 screenshots, rank-up version "PROMOTED"):
- Top left: title (PROMOTED when the rank changed), then one line per source with its amount: GAME COMPLETE,
  PERFORMANCE BONUS, COMMENDATIONS, SLOT MACHINE (a line is left out when it is 0), then TOTAL: +n.
- Centre: the new rank's insignia large, rank name under it, the rank name and "PROMOTED" huge and faint behind it.
- Right: the player's Spartan in their armour and colours.
- Bottom bar: emblem, insignia, player name + service tag, rank name, cR balance, "Next: <rank>" or "MAX RANK".
- The cR shown in the bar is the spendable balance (after Armory purchases), not the career total that sets rank
  (an Inheritor showed 10,034,983 cR; Inheritor needs 20,000,000).
- In all four screenshots with a performance bonus it is exactly 1.2 × game complete (4636 → 5564, 3421 → 4106,
  5211 → 6253, 4777 → 5732), not the +20 % the forum thread describes. Either the bonus was 120 % in those playlists
  or these were promotion weeks; game-complete amounts there (3,400–5,200) are also above the forum's per-minute
  table, so double-cR weeks are likely.

Without a rank-up the same screen is titled **CREDITS EARNED** (faint "TOTAL CREDITS" behind) and the bottom bar
shows a progress bar towards the next rank. A non-promotion game confirms the normal bonus: Captain, game complete
274, performance bonus 55 (= 20 %), commendations 163, slot machine 30,540 (a super jackpot), total +31,032. So
+20 % is the normal performance bonus and the 1.2× cases above were boosted.

**Other original career screens** (Tyler's screenshots), useful as layout references:
- **The Armory**: list on the left (Helmet, Left Shoulder, Right Shoulder, Chest, Wrist, Utility, Visor Color, Knee
  Guards, Armor Effect, Firefight Voice, then Emblem, Colors), Spartan preview on the right, "Move Camera" to turn
  it. Picking a slot opens a vertical list of items with a picture each and a "NEW" tag for unseen items, a large
  preview and a one-line description of the selected item. Same bottom bar as the post-game screen, with
  "n%" progress to the next rank and "Next: <rank>" or "MAX RANK".
- **Player card** (selecting a player in MY PARTY): rank insignia + name + rank at the top, emblem large behind the
  Spartan, then four career stats: Daily Challenges Completed, Weekly Challenges Completed, Armory Completion (%),
  Overall Commendation Progress (%). The party list shows each player's rank insignia at the right of their row.

## 2. Campaign score (points, not cR)

Used by Score Attack and par scores; also useful for "score X points" challenges.

- Points = enemy's base points × medal multipliers × (difficulty + sum of skull multipliers) × time multiplier; the
  time multiplier is applied once at the end of the mission. Difficulty and skulls are **added**, not multiplied
  (Heroic + Iron = 1.25 + 1.2 = 2.45).
- Difficulty: Easy 0.75, Normal 1.00, Heroic 1.25, Legendary 1.50.
- Skulls: Iron 1.2, Black Eye 1.1, Tough Luck 1.1, Catch 1.1, Cloud 1.1, Famine 1.2, Thunderstorm 1.2, Tilt 1.2,
  Mythic 1.2; Blind, Cowbell, Grunt Birthday Party, IWHBYD 1.0 (MCC adds Acrophobia and Bandana).
- Time: under 10 min ×2.0, 10–15 ×1.5, 15–20 ×1.2, over 20 ×1.0 (same on every mission).
- Par scores: Winter Contingency 10,000; ONI: Sword Base 40,000; Nightfall 10,000; Tip of the Spear 15,000; Long Night
  of Solace 20,000; Exodus 10,000; New Alexandria 10,000; The Package 15,000; The Pillar of Autumn 20,000;
  Lone Wolf 50,000.
- Kill credit also goes to the player if an AI ally finishes an enemy the player damaged. Multikill medals stack
  (an Overkill also gives Double and Triple Kill).

## 3. Ranks

50 ranks, Recruit (0 cR) to Inheritor (20,000,000 cR); full table in `ranks_and_pay.json` and already in
`Career.cpp` (`kRanks`). At launch only the first 23 were open (to Lt. Colonel Grade 3); the rest opened on
10 Nov 2010. The jump Noble → Eclipse → Nova (1.5 M, 2.5 M, 2 M) is a known Bungie typo; keep it for accuracy.

**Use the original 2010 ranks, not MCC's.** MCC replaced Reach's cR ranks with its own season/tier progression;
Noble uses the original 50 Xbox 360 ranks and thresholds (checked against an original rank chart Tyler supplied, made
by H3FREAK09). That chart gives Major Grade 2 = 375,000 cR (Halopedia's table says 370,000); Noble uses 375,000.

**Rank icons.** The original insignia (by rank family, one icon per grade): Recruit and Private single slashes,
Corporal double/triple gold slashes, Sergeant gold chevrons, Warrant Officer silver chevron shields, Captain silver
shield with star, Major silver star and bars, Lt. Colonel and Commander gold "L" bars with star, Colonel gold
star/diamond, Brigadier winged star, General laurel wreaths (Grade 4 crowned), Field Marshall silver skull wreath,
Hero gold skull wreath, Legend flaming skull wreath, Mythic and Noble moons, Eclipse and Nova suns, Forerunner and
Reclaimer hexagons, Inheritor comet. They are Bungie art and stay out of the repo. MCC probably does not ship them
(it uses its own rank art), so the plan: `UiArt` loads `data\ui\custom\ranks\<rank index 0-49>.png` when the player
provides them (same folder rule as the other custom art), and otherwise Noble shows the rank name only. A quick check
of `<MCC>\Data\UI\texturepacks` for Reach insignia is still worth doing first.

## 4. Commendations

45 in total: 16 campaign, 15 Firefight, 14 multiplayer (`commendations.json`). Each has tiers Iron → Bronze → Silver →
Gold → Onyx → Max; reaching a tier pays cR (100–3,000), Max pays nothing. Not counted on Easy. Originally Firefight and
multiplayer commendations counted only in matchmaking; in Noble they would count in local/custom games. Unknown: the
cR per tier for the multiplayer "Heavy Weapon" commendation (use the same as "Grenadier" until found).

Needs from the engine: kills by weapon class (precision, automatic, pistol, heavy, grenade, vehicle, close quarters),
headshots, assists, sprees, multikills, assassinations, enemy class (infantry / specialist / leader), vehicle kills,
missions completed, deathless missions, rounds without dying. Most of these are in the slot-6 results record
(general stats, medal counts, weapon tables; HANDOFF §5 "Game results").

## 5. Challenges

All 18 months Halopedia has (Sep 2010 – Feb 2012) are in `challenges.json`; **no month is missing**. Each entry:
`kind` (daily / weekly), `mode` (any / campaign / firefight / multiplayer), `name`, `requirement`, `credits`, `start`,
`end`. Halopedia itself has no list for these days: 2011-11-26, 11-27, 12-16, 12-20; 2012-01-06, 01-07, 01-16, 01-19,
01-27, 01-31, 02-04, 02-09, 02-10, 02-14 (four days in Apr 2011, Oct–Nov 2011 and Feb 2012 list only 3 dailies).

Original rules: 4 dailies per day (mixed modes; multiplayer and Firefight most often, campaign about one day in
three) and 1 weekly per week, reset at 11:00 GMT, not on Easy. Rewards 100–30,343 cR. `mode` is taken from what the
requirement asks for, so a "kill 400 enemies in Firefight" daily counts as firefight.

Suggested Noble use: pick today's 4 dailies and this week's weekly from the pool by date (for example the same
calendar day/week 14–15 years earlier, or a seeded random pick per mode), track progress from the slot-6 results,
pay the cR when done. "Matchmaking" in a requirement means any Noble game of that mode. Skip challenges that need
Xbox Live or Bungie.net ("Link your gamertag", "Clip It. Share It.") or reword them.

## 6. Armory

`armor.json`, by slot: helmet (with attachments), shoulders (left and right share one entry; a few have separate L/R
prices), chest, wrist, utility, visor, knee guards, armour effects, Firefight voices, Elite sets (rank only, no cR).
Each item has the rank needed and the cR price. Dependencies: Haunted needs every other helmet and attachment;
EVA [C] needs EVA; [R] chests need the base chest; Chobham, NxRA and Trauma Kit need Hard Case. Items marked
`mcc_only` (AKIS, Mariner, MCC [R] chests, EXO/TSCS) were added by MCC seasons, not the 2010 Armory; MCC's Series 8
helmets are left out. Pre-order, Limited/Legendary edition and Waypoint items (Recon UA/HUL[3], Officer Elite,
Legendary and Eternal effects, Military Police, CQB, Multi-Threat [W], Base Security [W]) have no cR price: give them
another unlock (commendation or challenge milestones).

The armour itself is applied through the profile/engine (not found yet); colours: 30 (10 bases × 3 shades, see
HANDOFF §0), plus Black and Zombie only as forced colours in custom games.

## 7. Loadouts

Loadouts come from the game variant, so Noble only shows them. Reach's standard sets for reference:
Spartan: Airborne/Air Assault (AR, Magnum, 2 frags, Jetpack), Scout (AR, Magnum, Sprint), Marksman (DMR, 1 frag,
Sprint), Recon (DMR, AR, Sprint), Recon Marksman (DMR, Magnum, Sprint), Expert Marksman (DMR, Magnum, 3 frags, Sprint),
Guard (AR, Magnum, Armor Lock), Operator (Shotgun, 1 frag, Armor Lock), Guard Operator (Shotgun, Magnum, Armor Lock),
Expert Operator (Shotgun, AR, 3 frags, Armor Lock), Grenadier (Grenade Launcher, AR, 3 frags, Sprint), Stalker (AR,
Magnum, Active Camo), Medic (AR, DMR, Drop Shield), Specter (Grenade Launcher, Magnum, 1 frag, Active Camo).
Elite: Ranger (Plasma Repeater, Plasma Pistol, 2 plasma, Jetpack), Warrior (same, Evade), Gladiator (Energy Sword,
1 plasma, Evade), Honor/Champion Gladiator, Zealot / Royal / Champion Zealot (Needle Rifle), Assassin / Dark /
Champion Assassin (Needler, Active Camo), Sentry (Needle Rifle, Plasma Pistol, 3 plasma, Armor Lock), Champion
(Concussion Rifle, Plasma Pistol, Active Camo). Custom loadout names offered by the game include Noble 1–6, the
Noble Team and Spartan-II names, and joke names (FNG, NOOB, Redshirt, ...).

## 8. Controller layouts (for the CONTROLLER settings tab)

Xbox 360 Reach, button layouts (A / B / X / Y / LS / RS / RT / LT / RB / LB; Start = menu, Back = scores,
D-pad up team chat, down mic, left night vision):

| Layout | A | B | X | Y | LS | RS | RT | LT | RB | LB |
|---|---|---|---|---|---|---|---|---|---|---|
| Default | Jump | Switch grenades | Action/Reload | Swap weapons | Crouch | Zoom | Fire | Grenade | Melee | Armor ability |
| Southpaw | Jump | Switch grenades | Action/Reload | Swap weapons | Crouch | Zoom | Grenade | Fire | Armor ability | Melee |
| Boxer | Jump | Switch grenades | Action/Reload | Swap weapons | Crouch | Zoom | Fire | Melee | Grenade | Armor ability |
| Green Thumb | Jump | Switch grenades | Action/Reload | Swap weapons | Crouch | Melee | Fire | Grenade | Zoom | Armor ability |
| Bumper Jumper | Switch grenades | Action/Reload | Armor ability | Swap weapons | Crouch | Zoom | Fire | Grenade | Melee | Jump |
| Recon | Jump | Melee | Switch grenades | Swap weapons | Crouch | Zoom | Fire | Grenade | Action/Reload | Armor ability |

Thumbstick layouts (same on 360 and MCC; look inversion only flips look up/down):

| Layout | Left stick | Right stick |
|---|---|---|
| Default | move forward/back + strafe | look up/down + turn |
| Southpaw | look up/down + turn | move forward/back + strafe |
| Legacy | move forward/back + turn | look up/down + strafe |
| Legacy Southpaw | look up/down + strafe | move forward/back + turn |

Every action with its MCC config number, keyboard default and MCC gamepad default (including Forge and Theater), the
360 Forge/Theater buttons and all MCC gamepad/mouse setting ranges are in `controls.json`. Other 360 options: look inversion, look sensitivity 1–10
(default 3), auto-look centering, hold/toggle crouch, clench protection, flight inversion, Theater camera speeds.

MCC default ("Universal Default (Recon)"): A jump, B melee, X armor ability, Y change weapon, LS crouch, RS zoom,
RT fire, LT grenade, RB action + reload, LB switch grenades, D-pad up night vision, left/right previous/next grenade,
down push-to-talk; vehicle functions LT / A / X; View scoreboard. MCC presets: Universal Default (Recon), Universal
Reclaimer, Universal Zoom & Shoot, Universal Bump & Jump, Universal Green Thumbs, Universal Infinite, Default, Boxer,
Green Thumbs, Bumper Jumper, Recon, Custom; plus D-pad/buttons swap, bumpers/triggers swap, aim control
classic/modern, horizontal and vertical sensitivity 1–10 (3), look acceleration 1–10 (5), axial and radial dead zone
0–20 % (12 %), vibration, "Jumper Jumper" (jetpack on held jump).

MCC keyboard defaults match Noble's KEY BINDINGS tab (W/A/S/D, Space, Left Ctrl, Mouse 1/2, Z/X zoom in/out, Q melee,
R reload, F grenade, E action, 1 change weapon, 2 switch grenades, 3 previous grenades, Left Shift armor ability,
4 night vision, Tab scoreboard, L loadouts, J text chat, V push to talk). MCC's config-file action numbers
(Halopedia) are: jump 0, switch grenades 1, action 2, reload 3, change weapon 4, melee 5, night vision 6, grenade 7,
fire 8, crouch 9, zoom 10, zoom in 11, zoom out 12, forward 16, back 17, left 18, right 19, scoreboard 20,
vehicle 2/3/1 = 21/22/24, armor ability 23, Forge 25–35 and 51–54, Theater 36–50, previous grenades 56, loadouts 58,
switch grenades alt 65. These are MCC's `GameAbstractButton` numbers, the same numbering `KeyBindings.cpp` uses, so
they are a cross-check for its names.
