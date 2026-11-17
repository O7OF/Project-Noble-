# Armor customization, colours, palettes, emblems and the Armory (Xbox 360 retail names)

Where the engine handles what a player looks like and how the customization menus work: armor pieces, change-colour, emblems, loadouts and the "player model" UI used by the Armory. Existing Noble facts to keep in mind: only five real visor slots exist (extended finishes render white); Armory writes must run on the simulation thread; the Armory uses the menu map's GLOBALS table.

**Reads-as architecture:**
- **Appearance data**: `player_appearance.obj`, `player_model_customization.obj`, `profile.obj` / `c_player_profile*` (stored choices), `player_configuration_*_component` (UI readers/writers of the configuration), `c_player_trait_appearance` (appearance as a *player trait*, i.e. game modes can override look).
- **Loadouts**: `loadout_definition`, `c_loadout_traits`, `c_loadout_palette_traits`, `player_loadout_menu`, `player_loadout_utilities`, replicated by `c_simulation_player_loadout_request_event_definition` (so loadout choice is a network event).
- **Armory UI (the "player model" screens)**: `c_player_model_widget`, `c_player_model_*_list_data_component` (base / color / piece / section / gender / race), gender and race change handlers, purchase handler, `c_user_interface_player_model_camera` (the pedestal camera). These are CUI components bound to data (see TOPIC-ui-cui-main-menu-map).
- **Colour**: `color_math.obj`, `cui_color_preset.obj` (UI colour presets), `cui_argb_color_property` (a property type: ARGB colour), `g_recycling_object_colors`, `g_player_model_choice_enum_table`.
- **Emblems**: `emblems.obj`, `render_emblem`, `emblem_info_reader/writer_component`, `c_emblem_widget`.

**Things you can do with it:**
1. Read `c_player_trait_appearance` and `player_model_customization` to see exactly which fields the engine accepts (pieces, colours, visor, flags) - the visor-slot limit is a data-table limit worth confirming here.
2. Use the UI component list as a checklist when rebuilding Armory screens: each `*_list_data_component` is one data source the screen expects.
3. Palettes: colour presets (`cui_color_preset`) are UI-side; the in-world palette is object change-colour data - compare against Noble's `ReachArmory` changecolor writes.
4. Armour effects (flame / lightning variants): look at `render_object_attachments`, `effects`, `object_lights` (see TOPIC-lighting-rendering) for how effects attach to a model/region.

Source of everything below: names from the leaked Omaha linker map applied to the retail Xbox 360 build (halo3_cache_release, Jul 24 2010). "VA" = retail virtual address (image base 0x82000000). Names/addresses are facts from the map; "reads as" notes are interpretation until the function body is verified. MCC x64 `haloreach.dll` shares the engine design but not the addresses: use the matching method in `README.md` (shared strings, call shape).

## Source modules (51)

| source file (.obj) | code symbols | first VA |
|---|---|---|
| emblem_info_reader_component | 19 | 0x820b07b0 |
| player_configuration_reader_component | 76 | 0x820b0810 |
| emblem_info_component_base | 3 | 0x820b08a8 |
| emblem_widget | 28 | 0x820b0bb8 |
| cui_color_preset | 2 | 0x820b1a78 |
| object_recycling | 11 | 0x820bbea0 |
| game_engine_player_traits | 7 | 0x820bc4e0 |
| game_engine_player_traits_structures | 57 | 0x820c3860 |
| player_loadout_utilities | 6 | 0x820c84b8 |
| models | 12 | 0x820c8978 |
| c_player_profile | 62 | 0x820d9268 |
| player_mapping | 19 | 0x820da070 |
| profiler_stopwatch | 6 | 0x820db000 |
| color_math | 6 | 0x82120300 |
| model_skinning | 4 | 0x82130438 |
| player_effects | 14 | 0x8213a878 |
| render_models | 3 | 0x82173958 |
| player_model_customization | 4 | 0x821eacc8 |
| player_loadout_menu | 3 | 0x821eba68 |
| object_activation_regions | 9 | 0x821f9500 |
| game_engine_loadout_traits_structures | 4 | 0x822140f0 |
| emblem_list_data_component | 38 | 0x822a8328 |
| c_player_profile_storage | 17 | 0x8231f400 |
| profile_data_reader_component | 148 | 0x82342128 |
| profile_data_writer_component | 56 | 0x8234bd38 |
| emblems | 1 | 0x8237e0a0 |
| player_appearance | 1 | 0x824bce08 |
| loadout_definition | 5 | 0x8253df50 |
| game_engine_loadout_traits | 6 | 0x825f77e0 |
| render_emblem | 2 | 0x826df540 |
| cui_player_select | 1 | 0x826e9328 |
| user_interface_player_model_camera | 2 | 0x82709b28 |
| emblem_update_new_item_record_component | 12 | 0x82718d00 |
| player_model_color_list_data_component | 26 | 0x8272a9f0 |
| player_model_set_camera_event_handler_component | 17 | 0x8272a9f0 |
| player_model_widget | 43 | 0x8272a9f0 |
| emblem_info_writer_component | 12 | 0x8272a9f0 |
| player_model_section_list_data_component | 16 | 0x8272a9f0 |
| player_model_race_change_event_handler_component | 13 | 0x8272a9f0 |
| loadout_list_data_component | 25 | 0x8272a9f0 |
| player_model_base_list_data_component | 22 | 0x8272a9f0 |
| loadout_select_component | 6 | 0x8272a9f0 |
| player_configuration_writer_component | 37 | 0x8272a9f0 |
| player_model_list_data_component | 13 | 0x8272a9f0 |
| player_model_gender_change_event_handler_component | 13 | 0x8272a9f0 |
| player_model_piece_list_data_component | 28 | 0x8272a9f0 |
| player_model_purchase_event_handler_component | 18 | 0x827452e8 |
| player_model_update_new_item_record_event_handler_component | 7 | 0x8278f430 |
| cui_argb_color_property | 1 | 0x827c4f70 |
| player_configuration_component_base | 2 | 0x827d1758 |
| profile | 2 | 0x829700b0 |

## Main classes (by number of functions)

`Vc_profile_data_component_base` (161), `Vc_player_configuration_component_base` (95), `c_player_profile_interface` (58), `c_profile_data_component_base` (35), `Vc_player_model_widget` (32), `Vc_emblem_list_data_component` (29), `Vc_emblem_info_component_base` (23), `Vc_player_model_piece_list_data_component` (23), `Vc_emblem_widget` (22), `Vc_player_model_color_list_data_component` (20), `c_player_trait_weapons` (19), `Vc_loadout_list_data_component` (19), `Vc_player_model_base_list_data_component` (17), `Vc_player_model_set_camera_event_handler_component` (14), `Vc_player_model_purchase_event_handler_component` (13), `Vc_player_model_section_list_data_component` (12), `c_player_trait_shield_vitality` (11), `c_player_trait_movement` (9), `Vc_player_model_race_change_event_handler_component` (9), `Vc_player_model_list_data_component` (9), `Vc_player_model_gender_change_event_handler_component` (9), `Vc_player_configuration_reader_component` (8), `c_player_trait_appearance` (8), `Vc_emblem_update_new_item_record_component` (8), `c_player_model_widget` (8), `c_player_traits` (6), `c_player_trait_sensors` (6), `c_loadout_traits` (6), `c_emblem_list_data_component` (6), `c_stop_watch` (5), `c_profile_read_overlapped_task` (5), `c_profile_write_overlapped_task` (5), `c_loadout_list_data_component` (5), `c_player_configuration_reader_component` (4), `Vc_loadout_select_component` (4), `c_player_model_purchase_event_handler_component` (4), `c_emblem_info_component_base` (3), `c_emblem_widget` (3), `c_player_loadout_menu` (3), `c_game_engine_loadout_traits` (3)

## Named globals / tunables (10)

Many of these are developer tunables or debug switches (booleans/floats); changing them at runtime in MCC needs the matching MCC address.

| global | VA |
|---|---|
| g_emblem_info_property_editor | 0x82020028 |
| g_argb_color_property_editor | 0x82020048 |
| g_chud_debug_highlight_object_color | 0x82a2b224 |
| g_game_clear_color | 0x82a8f9ac |
| g_simulation_player_loadout_request_event_definition | 0x82ac99c0 |
| g_recycling_groups_allocator | 0x82ad0154 |
| g_recycling_volumes_allocator | 0x82ad0160 |
| g_contrail_profile_allocator | 0x82ad11a0 |
| g_player_model_choice_enum_table | 0x82b07de4 |
| g_recycling_object_colors | 0x82b0af44 |

## Key functions (328 matched; first 70 shown, rest in the CSV)

| function | VA |
|---|---|
| `Vc_emblem_info_component_base::cui_get_scalar_property_by_return_value` | 0x820b07b0 |
| `Vc_emblem_info_component_base::cui_get_scalar_property_by_out_pointer` | 0x820b0840 |
| `c_emblem_info_component_base::get_background_emblem_color` | 0x820b08a8 |
| `c_emblem_info_component_base::get_primary_emblem_color` | 0x820b0988 |
| `c_emblem_info_component_base::get_secondary_emblem_color` | 0x820b0a68 |
| `c_emblem_widget::set_emblem_info` | 0x820b0bb8 |
| `Vc_emblem_widget::cui_set_scalar_property_by_const_pointer` | 0x820b0d08 |
| `Vc_emblem_widget::cui_set_scalar_property_by_value` | 0x820b0d38 |
| `cui_color_preset_get_team_bitmap_tint` | 0x820b1a78 |
| `cui_color_preset_get_team_text_tint` | 0x820b1ba8 |
| `c_player_trait_weapons::equivalent` | 0x820bc4e0 |
| `c_player_trait_weapons::set_equipment_drop_setting` | 0x820c3860 |
| `c_player_trait_shield_vitality::set_assassination_immunity_setting` | 0x820c3970 |
| `c_player_trait_weapons::set_infinite_equipment_setting` | 0x820c3970 |
| `c_player_trait_weapons::set_melee_damage_modifier_percentage_setting` | 0x820c39b8 |
| `c_player_trait_weapons::set_weapon_pickup_allowed_setting` | 0x820c3a00 |
| `Vc_player_configuration_reader_component::cui_get_scalar_property_by_out_pointer` | 0x820c7b50 |
| `c_player_configuration_reader_component::get_player_bitmap_tint_color` | 0x820c7bb8 |
| `loadout_player_loadout_menu_is_available` | 0x820c84b8 |
| `model_find_variant_by_name` | 0x820c8978 |
| `c_player_trait_weapons::set` | 0x820ce4a8 |
| `c_player_trait_weapons::set_initial_equipment_absolute_index` | 0x820ce638 |
| `c_player_trait_weapons::set_initial_secondary_weapon_absolute_index` | 0x820ce6b8 |
| `c_player_trait_weapons::set_initial_primary_weapon_absolute_index` | 0x820ce9b8 |
| `c_player_traits::set` | 0x820cea38 |
| `c_player_trait_shield_vitality::set` | 0x820ceac0 |
| `c_player_trait_sensors::set` | 0x820ced50 |
| `c_player_trait_movement::set` | 0x820cee20 |
| `c_player_trait_appearance::set` | 0x820cf198 |
| `c_player_profile_interface::get_player_appearance` | 0x820d9268 |
| `c_player_profile_interface::get_player_model_customization` | 0x821101d8 |
| `real_argb_color_to_pixel32` | 0x82120300 |
| `rgb_colors_interpolate` | 0x821e1df8 |
| `player_model_customization_get_player_customization_string_id` | 0x821eacc8 |
| `customization_globals_try_and_get` | 0x821eadd0 |
| `c_player_loadout_menu::is_showing_loadout_menu` | 0x821eba68 |
| `game_engine_assemble_player_traits` | 0x821f0160 |
| `object_activation_regions_update` | 0x821f9500 |
| `c_player_trait_movement::equivalent` | 0x82209160 |
| `c_emblem_widget::do_render` | 0x82211ed8 |
| `c_game_engine_loadout_traits::set` | 0x822140f0 |
| `c_player_trait_weapons::set_initial_grenade_count_setting` | 0x822165b8 |
| `c_player_trait_weapons::set_infinite_ammo_setting` | 0x82216600 |
| `c_player_trait_sensors::set_directional_damage_setting` | 0x82216648 |
| `c_player_trait_weapons::set_recharging_grenades_setting` | 0x82216648 |
| `c_player_traits::set_defaults` | 0x82216690 |
| `model_select_initial_permutation` | 0x82219358 |
| `c_player_traits::clear` | 0x8221e860 |
| `c_game_engine_loadout_traits::initialize` | 0x82226710 |
| `emblem_set_render_constants_from_user_interface` | 0x8237e0a0 |
| `__tls_set_g_object_activation_region_data_allocator` | 0x823f1b00 |
| `object_activation_regions_initialize` | 0x823f1b18 |
| `object_activation_regions_initialize_for_new_map` | 0x823f1b60 |
| `object_activation_regions_dispose_from_old_map` | 0x823f1b78 |
| `object_activation_regions_dispose` | 0x823f1b90 |
| `object_activation_regions_add_region` | 0x823f1ba0 |
| `?allocate@?$t_restricted_allocation_manager@$00$0A@$0A@$1?__tls_set_g_object_activation_region_data_allocator@` | 0x823f1cc8 |
| `?deallocate@?$t_restricted_allocation_manager@$00$0A@$0A@$1?__tls_set_g_object_activation_region_data_allocato` | 0x823f1d80 |
| `real_rgb_color_to_real_hsv_color` | 0x8246b100 |
| `real_hsv_color_to_real_rgb_color` | 0x8246b248 |
| `pixel32_to_real_argb_color` | 0x8246b380 |
| `player_appearance_valid` | 0x824bce08 |
| `game_engine_get_loadouts_enabled_for_player` | 0x824fc060 |
| `player_sanitize_loadout_index` | 0x824fc1c8 |
| `loadout_reset_player_loadouts_for_new_round` | 0x824fc280 |
| `player_set_loadout_palette_type` | 0x824fc2f8 |
| `team_designator_set_loadout_palette_type` | 0x824fc3a8 |
| `c_player_trait_shield_vitality::set_damage_resistance_percentage_setting` | 0x82506300 |
| `c_player_trait_weapons::set_damage_modifier_percentage_setting` | 0x82506300 |
| `c_player_trait_shield_vitality::set_body_recharge_rate_percentage_setting` | 0x82506348 |


---

# Noble armor-effect colour/variant findings (2026-11-17)

This section is derived from the primary-colour flame variant work; it is MCC x64 (haloreach.dll 1.3528.0.0) engine
behaviour, not Omaha names. Full handoff: `../../04-project-state/HANDOFF-ARMOR-FX-COLOURS-2026-11-17.md`.

## Profile / item mapping
- Spartan effect field +0x68, Elite +0x90; MCC translator RVA `0x3B2F4`, profile application `0x2EDA08`,
  appearance builder `0x1055EC` (image base 0x180000000).
- Stock items `kEffect + 0..6` = default, birthday, heart, pestilence, inclement, legendary, eternal. Noble custom effect
  ids `0xF0..0x10D` are detoured through `0x3B2F4` to pmcg list indices 7..36 (`src/common/ArmorEffects.hpp`).
- pmcg lists `80` (Spartan) and `8C` (Elite) hold item name + "customised bits" changes {region, permutation, flags}.
  Armory also uses the embedded copy `src/res/spartan_armor_parts.txt` - keep both in sync.

## Effect attachment / marker gating limits
- Correct permutation gating empirically stops after **5 armor-effect attachments total** (stock `armor_fx.effect` + 4
  extra `armor_fx_ext*.effect`). A 5th extra tag makes all extra effects spawn regardless of selection ("two effects at
  once"/regular+gold mixing), even at equal total attachment counts. 4 extras = 24 variant slots is the safe maximum.
- An effect tag has a hard **6-location** cap; ManagedBlam `TagFile.Save()` silently drops locations added past 6 (events
  can still grow). Use more effect tags, not more locations.
- Marker groups are filtered by marker `region index` + `permutation index`. Marker transform fields can be copied from
  the stock `armor_fx_00` marker (node 42, stock translation/rotation/scale) to keep flames centred.
- Region permutation 0 is active by default. Stock `armor_fx_00` at perm 0 made the legendary flame stack under every
  selection; fixed by a markerless `armor_fx_default` at index 0, stock `armor_fx_00..05` at perms 1..6 and variant
  perms after that (markers shifted to match).
- Render-model region cap: 16 regions (a 17th crashes ManagedBlam). `armor_fx` has 32 permutation slots; a 16th region
  `armor_fx_pri` holds variants that do not fit.

## Menu / Armory
- Armory reads the rebuilt mainmenu map (`data\menumaps\noble_menu.map`) and its matching `noble_menu.string_ids.csv`.
  Rerun `tools/menu_sidcsv.ps1` after **every** mainmenu cache build; stale ids make effects vanish or resolve to the
  wrong permutation.
- Armory effect list is `kArmorEffects` in `src/ui/ArmorCustomizer.cpp`; the menu map's render model must contain the
  variant markers/permutations and pmcg must contain the items (engine tag + embedded data).

## VFS mod maps
- Noble VFS serves an active mod's `<mod>\maps\<name>.map` ahead of MCC stock maps; activate with `-mod <folder>`.
  One mod folder can carry many rebuilt maps, giving variants on every included map without per-map overrides.
- Matchmaking does not use map mods; custom games only, and all peers need the same mod.
- Measured rebuild size: 70_boneyard stock 199 MB -> rebuilt 566 MB (~2.8x). A full 44-map pack is ~25-30 GB built.


## Runtime particle-colour facts (2026-11-17)

- The engine renders with its **own D3D11 device/context**, not the launcher's. Reliable capture: hook `IDXGISwapChain::Present`, then `swapChain->GetDevice(ID3D11Device)` + `device->GetImmediateContext()`; hook that context (and higher `ID3D11DeviceContext1..4` QI entries).
- `ID3D11DeviceContext::PSSetShaderResources` is vtable index **8** (IUnknown 0-2, DeviceChild GetDevice/GetPrivateData/SetPrivateData/SetPrivateDataInterface 3-6, VSSetConstantBuffers 7, PSSetShaderResources 8). Index 4 is GetPrivateData and fails silently.
- Flame palette: **256x16, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB (91), bound at PS slot 2**. The PS samples it at `(base_map.r, v3.z)`; final colour is `o0.xyz = palette * v1.xyz + v2.xyz`. PS carries only `ExposurePS`; there is no per-draw colour constant.
- Flame draw: PS ResourceId 8701, VS 8693; VS hash `f54376d4-5c51d3b9-77511f98-85bba44f`; five 4-index billboard draws per system. VS constant blocks: ParametersVS, ViewVS, ExposureVS, LightingVS, FogVS, MeshVS, ParticleIndex (16 B), ParticleRenderVS (592 B), AtmosphereFogVS. `ParticleRenderVS` has no owner/team/colour field; VS reads `vs_particle_state_buffer` (structured, stride 80).
- Runtime override: replace the palette SRV per device with a generated 256x16 SRV. Per-player selection still needs the particle state buffer/shader palette index or per-draw owner (M2).
