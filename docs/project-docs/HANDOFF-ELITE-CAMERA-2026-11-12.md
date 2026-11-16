# Elite per-slot camera zoom - status 2026-11-12 ~03:25 ET (NOT WORKING; later fully REVERTED at Tyler's request)
(Copy of claude.ai project doc.)

Tyler asked for Spartan-style camera zoom on the Elite.

## What was tried
- ArmorCustomizer.cpp: Elite rows carry scenes; ReachArmory.cpp wrote a zoom code to GLOBALS[0x2C] (rr_spun: 0 whole body, 1 head, 2 chest, 3 arm, 4 leg, 5 knee, 6 shoulder), nudging rr_turn ~6 frames.
- tools\noble-menu-elite-cameras.ps1 (ManagedBlam): 48 cameras noble_e*_cam_00..07 in mainmenu.scenario_cinematics_resource (429 points).
- tools\noble-menu-elite-hsc.ps1: noble_view_e* scripts, noble_view_elite_zoom.

## Why it failed (findings)
- Scene numbers above 0x14 (21..26) are ignored by the engine: camera_set/texture camera do nothing. Do not use them.
- Even forcing camera points from scene 11 changed nothing: camera_set from scene 11 + new points not taking effect, or the Elite frame isn't driven by camera_set/noble_aim at all.
- Debug ideas if resumed: read back GLOBALS[0x2C]; visible side effect in the script to prove it runs; reuse a scene index inside 0x0A..0x14; Elite variant of FrontEndMenu close-up placement (kModelX 0.656).
