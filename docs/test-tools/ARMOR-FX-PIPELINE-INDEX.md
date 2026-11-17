# Armor FX pipeline - script index (2026-11-17)

Scripts used for the primary-colour flame variants and the engine findings. All HREK/ManagedBlam scripts run through
`runmb.ps1` from `build\bin\Release\data\userdata\claude-tests-2026-11-11\tools` (or copies here).
Full handoff: `../04-project-state/HANDOFF-ARMOR-FX-COLOURS-2026-11-17.md`.

## Asset generation (HREK tags)
| Script | Purpose |
|---|---|
| `pri_assets.py` | Creates 30 `fire_pri_<id>.bitmap` palettes + `head_fire_pri_<id>.particle` copies (palette refs redirected). |
| `pri_palettes_solid.py` | Recolours the 30 palettes with the final solid-tint formula: `target * ((0.10 + 0.90*m^0.95) * 1.45)`, no white-hot term. |
| `pri_effects.py` | Regenerates `armor_fx_ext0..4.effect` from stock `armor_fx.effect` (6 events/locations each) and points events at the variant particles. **Writes placeholder location names** - always follow with rename_ext_locations.py. |
| `rename_ext_locations.py` | Sets each ext effect tag's locations to the real `flame_pri_<id>` marker names (required or variants silently render nothing). |
| `pri_markers_export.py` / `foundry_import_spartans.py` / Foundry addon | Foundry markers-only export / Blender import for marker placement. |
| `pri_merge2.py` | Merges Foundry markers-only marker groups into stock `spartans.render_model` via SerializeElement/DeserializeElement. |
| `check_markers_centering.py` / `marker_field_compare.py` | Verify variant marker region/permutation/transform vs stock `armor_fx_00`. |
| `fix_default_perm.py` | Makes `armor_fx` perm 0 a markerless `armor_fx_default`, shifts stock perms/markers to 1..6 and variant perms after. |
| `build_9att_4ext.py` / `build_9att_5ext.py` / `build_9att_biped.py` / `build_10att_biped.py` | Biped attachment layouts used in bisection. **Final working layout = 9 attachments: 5 stock + ext0..3** (build_9att_4ext.py). |
| `remove_stock_effect.py`, `only_ext0.py`, `add_ext1.py`, `add_ext2.py`, `grow_effect_test.py` | Diagnostic scripts for the attachment-gating/location-cap experiments (results in handoff §3). |
| `contact_sheet.py`, `make_armor_fx_thumbs_all.py` | Contact sheet + Armory thumbnail generation from screenshots. |

## Maps / menu
| Script | Purpose |
|---|---|
| `build_map.ps1` | `tool.exe build-cache-file <scenario> pc english` wrapper. |
| `build_all_mod_maps.ps1` | Rebuilds every MCC Reach map into `data\mods\armor_fx\maps\` (VFS mod). Paused after Forge/Boneyard; rerun to resume. |
| `menu_sidcsv.ps1` | Regenerates `noble_menu.string_ids.csv` from the current mainmenu map. **Must run after every mainmenu build.** |
| `compile-mainmenu.ps1` | Mainmenu cache build helper. |

## Testing
- Armory: `-menu-screen armory -menu-ignore-input -armory-focus "Armor Effect" -armor-effect-test <idx> -capture-at 24 -quit-after 30 -NoFocus`
- Match: `-map forge_halo -mode multiplayer [-map-override HREK\maps\forge_halo.map] -third-person -capture-at 40 -quit-after 50 -NoFocus`
- VFS mod: `-mod <abs path>\data\mods\armor_fx` (no map override) for any map in the mod folder.
- Effect indices: 0..6 stock, 7 auto, 8+i = variant i (8 steel ... 30 brick ... 31 rose).
- Kill `noble-dev` and `MCC-Win64-Shipping` before every test; stale processes show old builds.

## Working order (for a rebuild from scratch)
1. `pri_assets.py` -> `pri_palettes_solid.py`
2. Foundry markers-only -> `pri_merge2.py` (render_model markers/perms)
3. `fix_default_perm.py` (perm reorder)
4. `pri_effects.py` -> `rename_ext_locations.py`
5. pmcg tag edits (script used tonight; see RE doc) + embedded `spartan_armor_parts.txt` + code header
6. `build_9att_4ext.py` (biped)
7. Rebuild forge/mainmenu; copy `noble_menu.map`; run `menu_sidcsv.ps1`
8. Build exe; test with clean processes.
