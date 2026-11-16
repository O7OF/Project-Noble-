# Noble Armory: status and to-do (2026-11-11, 06:25 ET)

(Copy of claude.ai project doc. Historical; superseded by HANDOFF-ARMORY-2026-11-11.md and later docs.)

## Done tonight (uncommitted at the time, in D:\Project-Noble-Closed-Source)
- Menu music: Reach tracks only (ReachAudio.cpp, kOtherGamesMusic = false).
- Per-screen layered backgrounds from Bungie's menu art, colour-matched (assets\ui\noble\screens\{firefight,custom_games,campaign}\layers.json + PNGs; UiArt ScreenLayers/DrawLayered). Main-menu slideshow every 20 s (UiArt MenuBackgroundNext).
- Armory: procedural "Auntie Dot" lattice backdrop (pixel shader, 36 s loop), LOADING... text, character fades in once presentable (ReachArmory::IsCharacterPresentable).
- Lattice hidden exactly behind the character using the engine depth buffer: ClearDepthStencilView hook (vtable 53), copy at UI time, CPU 32x18 grid read-back picks the near/far gap (reversed Z, cleared to 0). Fallback = brightness ring mask.
- Stale GLOBALS table rejected after 20 s (no more 45 s blind fallback that loaded the ship scene).
- Species switch: Armory picture held from the click until the switch + camera delay; placement (whole-body vs close-up) debounced 4 frames (the real cause of the Elite/Spartan one-frame "jump").
- Effect/armour rebuild hold: OFF (Tyler prefers the occasional one-frame white flash over stutter).
- Crash fix: no D3D context work in the main-thread fallback draw path (NVIDIA driver crash when clicking Nameplate).
- Old "LOADING ARMORY" text in ArmorCustomizer removed.

## Still to do (at that time)
1. Emblem shows wrong on the armour (check after this build: the live profile refresh is back on).
2. Nameplate click: engine stops drawing the menu -> fallback path; crash fixed, but find why the engine stalls.
3. Elite armour pieces don't change (stock Elite render model has no elite_* regions; needs RR-style regions).
4. Close-up cameras don't frame the piece (chest/wrist/utility/knees show the wrong side).
5. Knife stuck on the chest whatever the chest piece (flair_chest_b "default" is the chest knife).
6. Knee guards: male default looks small with a gap; default probably should show none; FJ/Para = default.
7. Grenadier helmet looks off on the female (possibly male/female swapped).
8. Re-check helmets/attachments that didn't show; confirm armour effects use the production script (effect enabled).
9. Clean-up: test hooks in ReachArmory.cpp (noble-test-skip/head/delay.txt), "script state" diagnostic log, unused HoldStageBriefly, zz_patchtest.scenario + HREK\reports\zz_patchtest, HREK\temp test files.
10. Commit only when Tyler asks (merge origin/main into wip-merge before pushing).
11. Later: Marine/ODST third character; more armour sets (Prophet's Hand H2A Elites, Halo 3/4 Spartans).
