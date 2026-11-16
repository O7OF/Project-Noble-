# Draft — Bipeds, player representations, models, armour, animation

Input chapter for `PROJECT-NOBLE-COMPENDIUM.md`. Confidence tags as in the compendium: **proven** (one-variable
test + capture), **observed** (seen in a live run/log, not isolated), **community** (community guides),
**unverified** (code/disassembly/docs, not runtime-checked). Sources are named per claim.

---

## 1. Player representations (matg)

The globals tag (`matg`) holds one 0x6C-byte record per player character. Noble reads and rewrites this block
live to swap the player's biped (Trooper mode) and to find the Elite's first-person models (ElitePack).

| Field | Offset | Meaning | Confidence |
|---|---|---|---|
| representation count | `matg +0x474` (i32) | 3 in campaign, ≥2 in MP (0 = Spartan, 1 = Elite) | observed (TrooperMode logs "3 representations"; ElitePack checks `reps < 2`) |
| representation pointer | `matg +0x478` (u32 packed) | points at the array of 0x6C records | observed (both readers dereference it live) |
| record stride | `0x6C` | per character | observed |
| fp arms model name | `+0x14` | render model `objects\characters\elite\fp\fp` | unverified (ElitePack comment) |
| fp arms model datum | `+0x20` | the `mode` datum used to dress the arms | observed (ElitePack reads it) |
| fp body model name | `+0x24` | `objects\characters\elite\fp_body\fp_body` | unverified (comment) |
| fp body model datum | `+0x30` | | observed |
| third-person biped name | `+0x34` | | unverified (comment) |
| third-person biped datum | `+0x40` | the `bipd` datum the player spawns as | observed (TrooperMode/ElitePack rewrite it) |

Swapping: `TrooperMode.cpp` finds the `bipd` (`kBipd = 0x62697064`) whose datum equals the Spartan's, and writes
the mission's marine biped datum into `+0x40`. `ElitePack.cpp` does the same for the pack Elite, using
representation 1's `+0x40` as the stock Elite to match. Source: `src\engine\TrooperMode.cpp`, `src\engine\ElitePack.cpp`
(comments above `SetupFirstPerson` / `AfterPackStep`).

## 2. The model chain: biped → hlmt → mode → regions → permutations

Reach separates the *physical* model (`hlmt`, collision/damage) from the *render* model (`mode`, meshes/regions).
Offsets below come from Noble's own code (ElitePack/MarineArmor); several were read live so are more than guesses.

| Tag | Offset | Field | Confidence |
|---|---|---|---|
| `bipd` | `+0x70` | model (`hlmt`) datum | observed (MarineArmor/ElitePack read it, then call region lookup on it) |
| `hlmt` | `+0x0C` | render model (`mode`) datum | observed |
| `hlmt` | `+0x1C` | collision model datum | unverified (diagnostic dump) |
| `hlmt` | `+0xC0` | "new damage info" block `{i32 count, u32 packed ptr, u32}`; element 0x140 B: +0x00 flags, +0x04 material, +0x28 body vitality, +0x7C shield vitality | unverified (code); body/shield values observed in logs |
| `hlmt` | `+0xD0` | model shield strength (float) | observed (the 210 vs 79 Elite heuristic) |
| `hlmt` | `+0xE0` | damage sections block `{count, ptr, u32}`, element 0xC0 B; +0x00 section name (e.g. "body") | unverified (code); used to drop the death armour effect |
| `mode` | `+0x0C` | regions block `{i32 count, u32 packed ptr, u32}` | observed |
| region | (stride 0x10) `+0x00` name string-id, `+0x04` permutations block `{count, ptr, u32}` | | observed |
| permutation | (stride 0x18) `+0x00` name string-id, `+0x04..` mesh/flair data | | observed |

Sources: `src\engine\ElitePack.cpp` (`SetupFirstPerson`, `MatchStockDamage`, `DropDeathArmourEffect`, `AddNoblePieces`
comments), `src\engine\ReachArmory.cpp` (`MarineRegionIndexSEH` reads `bipd +0x70` then calls the region lookup).
The render-model region/permutation layout is also what the community port recipe depends on (`00_ULTIMATE_GUIDE.md`
§B7: "export the target game's own render_model, reuse its skeleton and markers").

## 3. How Noble applies armour

Three mechanisms, one engine primitive each. The engine primitive for a piece change is
`set_region_permutation(object, region string-id, permutation string-id, flags)` at RVA **0x4729F4**
(region lookup 0x1CBF0C, perm lookup 0x1CBF98, notify 0x4730CC). Colours use **0x473858**
(`set_change_color`); a variant reset uses **0x472B70** (`object_set_variant`). RVAs from
`02-LESSONS-AND-RULES.md` and `ReachArmory.cpp`.

**Armory (menu map).** An Armory item is *not* a permutation: the customization globals (`pmcg`) translate it into
one or more `(region, permutation)` changes (e.g. helmet `kurt_a` → `head:player_kurt` + `flair_head_a:kat` +
`flair_head_b:player_adrianna`). Noble loads that table from `data\armor\*.rrpmcg` (dump of the pack's pmcg; not in
git; embedded fallback `spartan.rrpmcg`). File format: `L <list-hex> <count>`, `S <item>`, `C <region> <permutation>
<flags>`. List offsets: `20` helmet, `2c`/`38` shoulders, `44` chest, `50` wrist, `5c` utility, `68` knees, `80`
Spartan effects (Elite sets `74`, Elite effects `8c`). Resolved regions for a Spartan:
`head, body, flair_head_a..c, flair_left/right_shoulder, flair_chest_a..b, arms, flair_forearm, flair_thigh,
flair_knees, armor_fx`. The menu script consumes 16 `(region, permutation)` pairs from its GLOBALS table and calls
`object_set_permutation` on the pedestal biped. Source: `ReachArmory.cpp` §"Armory item → model regions",
`ApplyGlobalsAndPermutations`. observed.

**MarineArmor (in-match, mods).** The Armory Marine tab saves `{region, permutation}` pairs (and 4 change colours);
`ReachArmory::SyncMarine` puts them on the local player's marine biped. Per-region apply: read the region name's
string-id from the scenario map file's own string tables (header +0x30 count / +0x34 data / +0x38 size / +0x3C
index / +0x40 namespace count / +0x44 namespace offset), confirm the unit's model has the region via 0x1CBF0C, then
call 0x4729F4 per pair and 0x473858 per colour. It runs for ~3 s after spawn/pick change, skips sessions with peers
(lockstep safety), and the pieces themselves are applied on the **simulation thread** (`NotifyMatchTick`): the render
thread's write is overwritten by the per-frame sim→render game-state copy at 0x1CD389 (inside 0x1CD2BC). Source:
`ReachArmory.cpp` §"Marine / ODST pieces". observed / proven (Trooper Firefight, odst_light/odst_jetpack).

**ElitePack (pack injection).** The pack's own pmcg has no Elite changes, and the engine keeps the map's pmcg, so
`AddNoblePieces` appends the pack Elite's per-piece permutations onto the map's six armour lists. The pack Elite's
render model (`elite_noble`) has 9 regions with permutation counts `{6,1,9,9,1,11,11,9,11}`; the six piece slots map
to region indices `{5,6,8,2,3,7}` (helmet, left/right shoulder, chest, forearms, legs). A pick becomes
`base[slot] + position-in-region`. First-person arms/body are dressed separately: both fp models have one
`armor_pieces` region, one permutation per Elite set, and `ApplyFirstPersonSet` copies the wanted set's mesh into
every permutation. Source: `ElitePack.cpp` (`AddNoblePieces`, `kNobleRegionOf`, `kNobleRegionPerms`,
`SetupFirstPerson`). observed.

## 4. Elite / Marine / Spartan specifics

**Elite.** Player representation 1. pmcg Elite set list at `+0x74`, set order: Minor, Spec Ops, Ranger, Ultra,
Zealot, General, Field Marshal, Officer (then Honor Guard/Council/Ossoona/Schism/Combat as extra pieces in Noble's
menu Elite). First-person arms follow the Armory Arms piece, body follows shoulders; pieces with no fp version fall
back to the Armor Set. Change-colour slots 2/3 are fixed by the engine (read from multiplayer globals), not player
picked. Source: `ElitePack.cpp`, `ReachArmory.cpp`. observed.

**Marine.** The campaign/Firefight marine biped is `objects\characters\marine\marine` with model `marine.mode` and
animation graph `marine.jmad` (AGENT-COORDINATION 11-14 11:45; HREK tag dump). Render-model regions (HREK/ManagedBlam
dump, night handoff 02:00): `helmet` (10 perms), `head` (6), `shoulders L/R` (4), `chest_a` (4), `chest_b`,
`backpack` (11), `belt`, `neck`, `thighs`, `legs` (4); 4 change colours. Flair pieces are **instanced geometry** (35
instance placements, per-permutation masks), not merged meshes — in Reach's own model only `flair_backpack`
trooper_light / trooper_heavy / trooper_radio switch a placement on; every other backpack permutation has mask 0 (no
backpack). `marine_female` has fewer pieces and is the only one with a `flair_helmet_a` region. observed / proven
(night handoff 12:35, commit 8435473).

**Spartan.** Representation 0; default pieces through the pmcg lists; male/female body is a `body`/`arms` permutation
switch (`BipedSetGender` 0x49BB84 for the live biped); the prosthetic arm maps to `kat` (female) / `player_john117`
(male), never the raw `robot` string (crash 0xC0000005 at 0x6950A9). Source: `ReachArmory.cpp`. observed.

## 5. Animation graphs (jmad)

The animation tag is the **model animation graph** (`.model_animation_graph`, the `jmad` group); a biped references
its own. Noble touched these only when installing the Pelican (it replaces `marine`, `spartans`, `spartans_ai`
`.model_animation_graph` files in HREK). Extraction is a community pipeline (Bonobo/Baboon; `jmad` is the Reach
animation tag), and the corpus's one concrete "biped has no animation" fix is a *node-name* fix: bone/vertex-group
names must match the target skeleton exactly, else you get "matching node can't be found" / "no animation graph" on a
ported biped. No documented list of Reach region/permutation/marker names exists in the community corpus. Sources:
`repo-docs\pelican.md`, `discord-compendium\00_ULTIMATE_GUIDE.md` §B6/B7. community.

## 6. OPEN — campaign-Trooper Marine legs

**Symptom (observed).** With `-trooper` in campaign the player Marine renders folded at the hips, no legs below the
belt, while the unit's shadow is a normal standing figure. Reproduced on m35; not a merge regression
(AGENT-COORDINATION 11-14 11:24). The shadow standing while the render model folds means the fault is in the model
data applied to the swapped biped — regions / permutations / node mapping — not pose, armour choices or timing.

**Rejected with one-variable tests (each negative):**

| Hypothesis | Test | Result |
|---|---|---|
| Default profile appearance | removed `appearance.dat` | same legless/folded render |
| Cutscene pose | synthetic Space hold 30 s (`NOBLE_TEST_SKIP_CINEMATICS`) | no change at 90/120/150 s |
| Press-and-hold skip | pulsed Space hold (2 s down / 0.5 s up) | no change at 90/120/150 s |

**Not established.** Which region/permutation or node mapping folds the model is **unknown**; do not present it as
solved. Next step recorded: dump the marine model's regions/permutations/nodes from m35 (Reclaimer) and compare with
the live unit's permutation state. Sources: `00-START-HERE.md` "Current state", `AGENT-COORDINATION.md` recent
results (11-49/11-52/11-56), `02-LESSONS-AND-RULES.md`.

---

## Open questions

- The exact representation-record fields beyond `+0x40` (`+0x14/+0x20/+0x24/+0x30/+0x34`) are code-comment-derived
  and only the biped datum is proven live.
- The `hlmt` damage-section / `+0xA8`/`+0xB4` block layout is from a diagnostic dump, never independently verified.
- Whether the Trooper leg fold is a permutation the marine model lacks, a region left in its default/random state, or
  a skeleton/node mismatch (Spartan representation data on the marine model) — unresolved.
- No community documentation of Reach region/permutation/marker names exists to cross-check Noble's string-id lists.
