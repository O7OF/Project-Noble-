# Notes p1_08 (rigging channel, 2023-09 to 2026-09; thin chunk, mostly questions)

## Blender rig/pose basics
- To resize a posed Spartan, scale the ARMATURE (in object/pose mode), never the mesh; see Surasia's Infinite Spartan pose video https://www.youtube.com/watch?v=Jm39-vYl1zo (purplmunkii, 2025-02, rigging)
- Blender 4.x removed Bone Groups: hold Alt while picking a color to apply it to all selected bones (purplmunkii, 2024-04, rigging)
- To copy a bone color: shift-select target bones with the colored bone active, right-click bone > "Copy to Selected" (purplmunkii, 2024-04, rigging)
- Rest/pose shifts when toggling Pose/Rest position (elite bones jumping) usually mean bone placement/roll mismatches the mesh bind pose; the TauCeti_elites rig was cited as the base example (mr.chief4199, 2023-12, rigging)
- Bones "desynced" from mesh on a rigged elite: check armature vs mesh scale/transform (apply or match) before rebinding (thatmadmoose, 2025-12, rigging; fix not confirmed in chunk)
- Moving one object moves everything (e.g., elite follows shotgun): symptom of unintended parenting/constraints between objects (wastelander.ex, 2025-02, rigging; no fix confirmed)

## Weights / vertex groups
- Fix wrong weights: select all verts of the mesh and remove them from vertex groups they should not belong to (chiefster_4014, 2024-07, rigging)
- H2/H3 rigs are near-identical, so porting weights (e.g., H2A banshee to H3) works by renaming vertex groups to H3 bone names; no reweighting needed provided H3 rig lacks nothing the H2A has (chiefster_4014, 2024-07, rigging)
- Plate that bends wrongly (pelvis plate): flood-paint weight so the plate is 1.0 on the pelvis bone and 0.0 on thighs/spine (.username_ladi, 2026-01, rigging)
- Reclaimer-exported H2 models can have vertices missing from vertex groups (seen with the H2 grunt); community Google Drive copies were fixed by someone (exodst., 2025-12, rigging)
- Halo 4 multiplayer armor pack (swagster) ships unweighted; CATS did not help, expect manual weight paint or transfer from a rigged body (big_squish, 2024-03, rigging)
- Mirror modifier on a backpack blocks painting the mirrored side; apply the modifier or paint on the source side (failedmango, 2023-10, rigging)
- Rigging armored models: attach armor pieces to the rig, not weight-paint over body; helmets from ODST packs need parenting/vertex groups to the Spartan rig (it imports its own rig too, delete/merge it) (_spoopyuwu_, 2026-01, rigging)

## Rigs and rigging tools
- Halo 3 elite has no chest bone and no rigged public pack (as of 2025); H3 elite head is a separate overlapping mesh, which breaks auto-rig tools (Auto-Rig Pro / Rigify errors); fix overlaps or rig head separately (kozmetic, monokkii, 2025-01 to 2025-10, rigging)
- Auto-Rig Pro worked on H2/H3 first-person rigs but Reach FP rig is problematic (redlikerosess, 2025-02, rigging)
- Reach spartan Rigify conversion is a recurring pain; Rigify rigs were lost in later Spartan/Elite packs (its.basil, 2024-06; sneakyoutlaw, g1beongames, 2025-11, rigging)
- A well-regarded IK/FK rig with IK fingers exists; converting phenex's spartan pack to it was hard, try other base packs; root bone needed for the IK/FK switcher (zangh, 2024-08, rigging)
- Imported Reach models may use a different import scale than Infinite/H4 when testing animations on rigs (zangh, 2025-12, rigging)
- Porting Reach elites to Halo 3: the bone X axis must point along the bone for IK to work (winnower, 2026-07, rigging)
- Wrist twist driver flipping past a threshold: classic Euler/driver range flip, rework driver to use quaternion/twist bone (spartanatic, 2025-03, rigging; unresolved)
- Idea: hand "peg" socket with weapon peg to swap weapons easily on elite rigs (bionicemperor, 2024-08, rigging)
- Rigged an elite with Auto-Rig Pro with little effort; supports odd hand layouts (frankr2994, 2025-07, rigging)

## Export / engines
- FBX to UE from Blender can break per Spartan (Reach core fine, Chief garbled); check armature scale/axes and export settings (xen, 2024-02, rigging; unresolved)
- Unity shader for Halo materials is pinned in the Unity channel (r93_sniper, 2023-12, rigging)
- Color change on Spartan pack helmets requires the pack's color-mask/shader setup; many H3 helmet downloads lack it (jcrash426, 2026-09, rigging)
- Unreal H3 remaster contact: veil; use the Unreal channel (.username_ladi, 2026-01, rigging)

## Other
- Ekur tool and H3 Spartan Pack mentioned as sources for Spartan imports (xander_1993, 2026-07, rigging)
- Surasia Infinite Spartan pack shader by Average Goblin Enthusiast (purplmunkii, 2023-12, rigging)
