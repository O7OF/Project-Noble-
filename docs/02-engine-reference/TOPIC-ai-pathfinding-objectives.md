# AI, pathfinding, objectives and behaviour: where the engine does it (Xbox 360 retail names)

A map of Reach's AI by source file, for "better AI" work: smarter targeting, objectives, squads, pathfinding, and generating navigation for maps that have none (Forge).

**Reads-as architecture:**
- **Actors** (`actors`, `actor_combat`, `actor_perception`, `actor_stimulus`, `actor_looking`, `actor_moving*`, `actor_firing_position`, `actor_dynamic_firing_position`) with per-species types (`actor_type_elite`, `_grunt`, `_flying`, `_scarab`, `_bugger`, `_custom_behavior`, `_lod`).
- **Behaviour tree**: `behavior.obj` (401 functions, 251 behaviours: alert, ambush, avoid, cover, cower, engage, flee, formation, guard, huddle, investigate, kamikaze, leap_on_cover, melee_charge, patrol, perch, presearch, retreat, search, self_preservation, stalk, suppress, sync_attack, ...). Each behaviour has a `_get` (registers it) and decide/control functions; `behavior_root` is the top.
- **Orders / directives / objectives**: `ai_directive_*` (objective, hold_position, target, interact, enter_vehicle, regroup), `objectives.obj` (tasks: `objective_task_*`, `task_update_group_state`, `squad_set_objective`, leadership), `ai_orders`, `ai_scenario_definitions`, `ai_cues`, `ai_hint_definitions`.
- **Squads / fire teams / formations / swarms / flocks**: `ai_fire_teams`, `formations`, `joint_behavior`, `ai_flocks`, `ai_atoms`.
- **Pathfinding**: `path.obj` (A*-style: `path_state_new`, `path_state_build_path`, a heap with `path_heap_*`, `path_state_estimated_distance`, attractors, hint penalties), `path_influence_map` (a 2D grid with Bresenham line writes: danger/preference map), `path_flying`, `path_3d_obstacle_avoidance`, `ai_sector_structures` (the nav mesh is "sectors" + "hints"). Tunable: `g_path_state_traverse_max_iteration_count`.
- **Tuning is mostly DATA, not code**: `c_character_*_properties_internal` (engage, evasion, retreat, charge, movement, grenade, weapon, vehicle, flocking, perception) read the character tag; `c_ai_traits` (get/set/encode/decode) is the trait block also used by game-mode "AI traits". Squad/encounter layout lives in the scenario tag.

**Things you can do with it:**
1. **Better AI without new code**: re-tune character tags and traits (engage ranges, evasion, retreat, grenade use) in HREK; the property classes above tell you which tag fields are actually read at runtime.
2. **Objectives/"thought" layer**: `objectives.obj` + `ai_directive_objective` is the existing hook; a mod can drive it from scripts (`ai_scripting_set_task`, `squad_set_objective`) before any C++ work.
3. **Automatic pathfinding on Forge/custom maps**: the engine has no runtime nav-mesh generator (sectors come from the HREK compile). Options: (a) bake sectors in HREK/tool for each shipped map; (b) implement a runtime sector builder feeding `ai_sector_structures`; (c) for flying units reuse `path_flying`/`path_3d_obstacle_avoidance`, which already work without a mesh.
4. **Bots/AI players**: `ai_player.obj` and `g_ai_players_allocator` suggest AI-controlled players exist in the engine - investigate for bots.
5. **Debugging**: `ai_debug`, `ai_globals` and many `g_*` tunables below are the on-screen AI debug switches.

Source of everything below: names from the leaked Omaha linker map applied to the retail Xbox 360 build (halo3_cache_release, Jul 24 2010). "VA" = retail virtual address (image base 0x82000000). Names/addresses are facts from the map; "reads as" notes are interpretation until the function body is verified. MCC x64 `haloreach.dll` shares the engine design but not the addresses: use the matching method in `README.md` (shared strings, call shape).

## Source modules (81)

| source file (.obj) | code symbols | first VA |
|---|---|---|
| objectives | 38 | 0x820b0248 |
| actor_firing_position | 27 | 0x820b12a8 |
| path_3d_obstacle_avoidance | 14 | 0x820b81b8 |
| actor_dynamic_firing_position | 14 | 0x820bc178 |
| swarms | 35 | 0x820c3c60 |
| actors | 111 | 0x820c6eb8 |
| actor_moving_objects | 4 | 0x820cdb08 |
| behavior | 401 | 0x820cf378 |
| actor_perception | 26 | 0x820db448 |
| squad_patrol | 8 | 0x820df600 |
| actor_moving | 55 | 0x820dfa10 |
| actor_moving_ground | 7 | 0x820e22b8 |
| path | 14 | 0x820e3a20 |
| ai_globals | 6 | 0x820e7b88 |
| actor_looking | 14 | 0x820e9e28 |
| ai_flocks | 24 | 0x820ea348 |
| props | 51 | 0x820ec008 |
| squads | 54 | 0x820ed0b0 |
| path_smoothing | 3 | 0x820ef2f0 |
| actor_stimulus | 43 | 0x820f0c88 |
| ai_script | 55 | 0x820f16c8 |
| combat_range | 1 | 0x820f1fc8 |
| actor_combat | 19 | 0x820f4f30 |
| behavior_misc | 4 | 0x820fdd40 |
| path_obstacles | 10 | 0x820ff7c8 |
| ai_traits | 7 | 0x82100f30 |
| character_definitions | 78 | 0x82101080 |
| path_obstacle_avoidance | 2 | 0x8210bd30 |
| ai_player | 14 | 0x82123d40 |
| ai_vehicles | 33 | 0x8212b708 |
| character_physics | 8 | 0x8212bfa0 |
| character_physics_mode_ground | 8 | 0x8212c518 |
| ai_dialogue | 26 | 0x8212d570 |
| ai_reference_frame | 16 | 0x82132758 |
| pathfinding_utilities | 9 | 0x8213a000 |
| ai_actions | 20 | 0x8213fa28 |
| actor_moving_vehicle_nondirectional_flying | 3 | 0x821406c8 |
| ai_sector_structures | 2 | 0x821462d8 |
| actor_type_lod | 8 | 0x8214ece8 |
| prop_dialogue | 4 | 0x8217b480 |
| ai_object_interaction | 2 | 0x821ad0a8 |
| joint_behavior | 19 | 0x821ad168 |
| actor_moving_vehicle_directional_flying | 1 | 0x821b12c8 |
| actor_moving_flying_vehicle_obstacle_avoidance | 3 | 0x821b21d0 |
| actor_type_flying | 23 | 0x821f0c58 |
| path_influence_map | 14 | 0x82206ea8 |
| ai_dialogue_queue | 6 | 0x8220c540 |
| character_physics_utilities | 2 | 0x82212a68 |
| character_physics_definitions | 11 | 0x82348158 |
| character_physics_mode_melee | 6 | 0x8253eaa8 |
| character_physics_mode_dead | 3 | 0x8253fa68 |
| character_physics_mode_sentinel | 3 | 0x82543898 |
| hs_unit_seats | 1 | 0x8254c650 |
| character_physics_mode_flying | 2 | 0x825514b0 |
| actor_type_scarab | 14 | 0x8257d880 |
| actor_moving_hints | 3 | 0x82636d50 |
| ai_hint_definitions | 30 | 0x82636f88 |
| ai_creatures | 1 | 0x82670678 |
| ai_sync_action_arranger | 14 | 0x82670778 |
| ai_fire_teams | 7 | 0x8267c910 |
| actor_types | 1 | 0x82683400 |
| ai_cues | 7 | 0x826913f0 |
| ai_atom_scripting | 23 | 0x82694290 |
| formations | 5 | 0x826a7a08 |
| ai_scenario_definitions | 1 | 0x826b0f98 |
| actor_moving_vehicle_directional_ground | 2 | 0x826b2288 |
| actor_moving_vehicle_nondirectional_ground | 1 | 0x826b3988 |
| actor_moving_giant_scarab | 3 | 0x826b4608 |
| path_flying | 3 | 0x826b8e70 |
| actor_type_grunt | 8 | 0x826bcb58 |
| ai_mission_dialogue | 2 | 0x826bd1d0 |
| ai_atoms | 1 | 0x826c0a08 |
| ai_directive_manager | 1 | 0x826c0f10 |
| ai_directive_regroup | 1 | 0x826c3778 |
| ai_directive_enter_vehicle | 7 | 0x826c3b48 |
| ai_directive_object_interact | 7 | 0x826c43a0 |
| ai_directive_target | 5 | 0x826c48e8 |
| ai_directive_hold | 2 | 0x826c4db0 |
| ai_directive_objective | 4 | 0x826c4e38 |
| pathmisc | 21 | 0x827decf8 |
| ai_debug | 2 | 0x828619f8 |

## Main classes (by number of functions)

`c_destination_orders` (11), `c_character_physics_mode_ground_datum` (8), `c_ai_traits` (7), `c_character_physics_mode_melee_datum` (6), `c_stimulus_distribution` (5), `$02` (5), `c_character_physics_component` (4), `c_stimulus_flags` (3), `c_squad_exhaustive_create_iterator` (3), `c_stimulus_origin` (3), `c_character_perception_properties_internal` (3), `c_ai_action` (3), `c_character_physics_mode_dead_datum` (3), `c_character_physics_mode_sentinel_datum` (3), `Vc_character_swarm_properties_internal` (2), `Vc_character_cover_properties_internal` (2), `Vc_character_charge_properties_internal` (2), `Vc_character_movement_properties_internal` (2), `Vc_character_voice_properties_internal` (2), `Vc_character_perception_properties_internal` (2), `Vc_character_evasion_properties_internal` (2), `Vc_character_engineer_properties_internal` (2), `Vc_character_kungfu_properties_internal` (2), `Vc_character_presearch_properties_internal` (2), `Vc_character_search_properties_internal` (2), `Vc_character_retreat_properties_internal` (2), `Vc_character_ready_properties_internal` (2), `Vc_character_interact_properties_internal` (2), `Vc_character_emotions_properties_internal` (2), `Vc_character_target_properties_internal` (2), `Vc_character_idle_properties_internal` (2), `Vc_character_flocking_properties_internal` (2), `Vc_character_look_properties_internal` (2), `c_stimulus_payload` (2), `Vc_character_engage_properties_internal` (2), `c_character_vehicle_properties_internal` (2), `c_character_weapon_properties_internal` (2), `Vc_character_vocalization_properties_internal` (2), `c_ai_direction` (2), `Vc_character_bunker_properties_internal` (2)

## Named globals / tunables (154)

Many of these are developer tunables or debug switches (booleans/floats); changing them at runtime in MCC needs the matching MCC address.

| global | VA |
|---|---|
| g_player_identifier_property_editor | 0x82020018 |
| g_emblem_info_property_editor | 0x82020028 |
| g_player_configuration_ptr_property_editor | 0x82020038 |
| g_argb_color_property_editor | 0x82020048 |
| g_component_ptr_property_editor | 0x82020058 |
| g_const_utf8_ptr_property_editor | 0x82020068 |
| g_const_char_ptr_property_editor | 0x82020078 |
| g_const_wchar_ptr_property_editor | 0x82020088 |
| g_qword_property_editor | 0x82020098 |
| g_real_property_editor | 0x820200a8 |
| g_bool_property_editor | 0x820200b8 |
| g_string_id_property_editor | 0x820202e8 |
| g_tag_reference_property_editor | 0x82020400 |
| g_long_property_editor | 0x82020410 |
| g_actor_type_default_team | 0x82027890 |
| g_actor_type_leader_type | 0x820278d4 |
| g_activity_posture_variants | 0x820287b0 |
| g_atom_behavior | 0x82a27bac |
| g_atom_formation | 0x82a27f8c |
| g_atom_swarm_to | 0x82a293bc |
| g_atom_swarm_from | 0x82a2a364 |
| g_ai_directive_objective | 0x82a2cba8 |
| g_ai_directive_hold_position | 0x82a2cbc8 |
| g_ai_directive_target | 0x82a2cbe8 |
| g_ai_directive_interact | 0x82a2cc08 |
| g_ai_directive_enter_vehicle | 0x82a2cc28 |
| g_ai_directive_none | 0x82a2cc48 |
| g_ai_directive_program_defitinions | 0x82a2cc68 |
| g_force_combat_status | 0x82a2cc80 |
| g_objective_allow_laser_designation_selection_of_objectives | 0x82a2d05a |
| g_path_state_traverse_max_iteration_count | 0x82a2d3fc |
| g_movement_force_combat_status_mapping | 0x82a2d594 |
| g_activity_exit_combat_status | 0x82a300f0 |
| g_ai_number_values | 0x82a30208 |
| g_game_results_player_vs_ai_statistic_definitions | 0x82a39f58 |
| g_objective_dead_cam_respawning_delay_before_voluntary_control_of_spawn | 0x82a3a6c0 |
| g_objective_dead_cam_respawning_respawn_time_on_player_seconds | 0x82a3a6c4 |
| g_leap_flight_path_scale | 0x82a40dfc |
| g_equipment_type_ai_trait_field | 0x82a41448 |
| g_new_unit_action_ai_impulse | 0x82a4305c |
| g_bbsquad_allocator | 0x82acfd48 |
| g_bbsquad_unit_allocator | 0x82acfd54 |
| g_actor_data_allocator | 0x82ae018c |
| g_ai_reference_frame_data_allocator | 0x82ae0198 |
| g_actor_firing_position_hash_allocator | 0x82ae01a4 |
| g_ai_globals_allocator | 0x82ae0208 |
| g_character_properties_cache_allocator | 0x82ae0328 |
| g_squad_data_allocator | 0x82ae1f6c |
| g_squad_group_data_allocator | 0x82ae1f78 |
| g_swarm_data_allocator | 0x82ae1fd8 |
| g_swarm_spawners_allocator | 0x82ae1fe4 |
| g_ai_sync_action_arranger_data_allocator | 0x82ae2000 |
| g_ai_players_allocator | 0x82ae2058 |
| g_ai_directive_data_allocator | 0x82ae20ac |
| g_objectives_data_allocator | 0x82ae20d4 |
| g_ai_cue_data_allocator | 0x82ae2238 |
| g_squad_patrol_data_allocator | 0x82ae2244 |
| g_dynamic_firing_set_allocator | 0x82ae22d8 |
| g_prop_data_allocator | 0x82ae22e8 |
| g_prop_ref_data_allocator | 0x82ae22f4 |
| g_prop_search_data_allocator | 0x82ae230c |
| g_path_influence_data_allocator | 0x82ae2664 |
| g_formation_data_allocator | 0x82ae2670 |
| g_flocks_data_allocator | 0x82ae26b4 |
| g_forge_menu_value_type_enum_table | 0x82b008e0 |
| g_forge_menu_item_type_enum_table | 0x82b008ec |
| g_objectives_allocator | 0x82b0096c |
| g_component_type_name_enum_table | 0x82b00a08 |
| g_forge_menu_type_enum_table | 0x82b025f0 |
| g_page_1 | 0x82b045e8 |
| g_page_2 | 0x82b04e18 |
| g_page_3 | 0x82b05648 |
| g_page_4 | 0x82b05e78 |
| g_start_menu_campaign_objectives_allocator | 0x82b076e0 |
| g_active_roster_player_detail_level_enum_table | 0x82b07c4c |
| g_campaign_difficulty_level_enum_table | 0x82b07c58 |
| g_campaign_metagame_scoring_enum_table | 0x82b07c64 |
| g_campaign_progress_enum_table | 0x82b07c70 |
| g_controller_component_enum_table | 0x82b07c7c |
| g_controller_index_enum_table | 0x82b07c88 |
| g_event_type_enum_table | 0x82b07c94 |
| g_font_id_enum_table | 0x82b07ca0 |
| g_network_active_roster_mode_enum_table | 0x82b07cac |
| g_network_session_class_enum_table | 0x82b07cb8 |
| g_network_session_privacy_enum_table | 0x82b07cc4 |
| g_network_session_advertisement_mode_enum_table | 0x82b07cd0 |
| g_property_type_enum_table | 0x82b07cdc |
| g_text_drop_shadow_style_enum_table | 0x82b07ce8 |
| g_text_justification_enum_table | 0x82b07cf4 |
| g_look_sensitivity_enum_table | 0x82b07d00 |
| g_button_presets_enum_table | 0x82b07d0c |
| g_remappable_buttons_enum_table | 0x82b07d18 |
| g_gamepad_buttons_enum_table | 0x82b07d24 |
| g_button_actions_enum_table | 0x82b07d30 |
| g_button_remapping_failure_reason_enum_table | 0x82b07d3c |
| g_joystick_presets_enum_table | 0x82b07d48 |
| g_camera_panning_enum_table | 0x82b07d54 |
| g_camera_flying_movement_enum_table | 0x82b07d60 |
| g_camera_look_sensitivity_enum_table | 0x82b07d6c |
| g_camera_flying_thrust_enum_table | 0x82b07d78 |
| g_voice_mute_setting_enum_table | 0x82b07d84 |
| g_voice_output_setting_enum_table | 0x82b07d90 |
| g_voice_matchmaking_setting_enum_table | 0x82b07d9c |
| g_subtitle_setting_enum_table | 0x82b07da8 |
| g_display_brightness_enum_table | 0x82b07db4 |
| g_team_index_campaign_enum_table | 0x82b07dc0 |
| g_team_index_multiplayer_enum_table | 0x82b07dcc |
| g_user_interface_voice_state_enum_table | 0x82b07dd8 |
| g_player_model_choice_enum_table | 0x82b07de4 |
| g_purchase_category_enum_table | 0x82b07df0 |
| g_network_game_privacy_enum_table | 0x82b07dfc |
| g_game_mode_enum_table | 0x82b07e08 |
| g_game_playback_type_enum_table | 0x82b07e14 |
| g_gui_error_category_enum_table | 0x82b07e20 |
| g_gui_error_icon_enum_table | 0x82b07e2c |
| g_gui_game_mode_enum_table | 0x82b07e38 |
| g_session_game_mode_enum_table | 0x82b07e44 |
| g_game_type_enum_table | 0x82b07e50 |
| g_game_activity_type_enum_table | 0x82b07e5c |
| g_network_game_type_enum_table | 0x82b07e68 |
| g_network_session_quality_enum_table | 0x82b07e74 |
| g_online_file_transfer_status_enum_table | 0x82b07e80 |
| g_online_file_transfer_type_enum_table | 0x82b07e8c |
| g_game_engine_team_options_model_override_type_enum_table | 0x82b07e98 |
| g_game_engine_team_options_designator_switch_type_enum_table | 0x82b07ea4 |
| g_game_engine_timer_type_enum_table | 0x82b07eb0 |
| g_game_engine_type_enum_table | 0x82b07ebc |
| g_content_item_type_enum_table | 0x82b07ec8 |
| g_content_controller_state_enum_table | 0x82b07ed4 |
| g_content_item_category_enum_table | 0x82b07ee0 |
| g_commendation_progression_level_enum_table | 0x82b07eec |
| g_campaign_difficulty_level_string_enum_table | 0x82b07ef8 |
| g_campaign_metagame_scoring_string_enum_table | 0x82b07f04 |
| g_game_skull_enum_table | 0x82b07f10 |
| g_challenge_category_enum_table | 0x82b07f1c |
| g_input_event_name_enum_table | 0x82b07f28 |
| g_saved_film_category_enum_table | 0x82b07f34 |
| g_player_grade_enum_table | 0x82b07f40 |
| g_player_sub_grade_enum_table | 0x82b07f4c |
| g_hopper_player_investment_category_enum_table | 0x82b07f58 |

## Key functions (355 matched; first 70 shown, rest in the CSV)

| function | VA |
|---|---|
| `actor_evaluation_context_add_influence_point` | 0x820b12a8 |
| `dynamic_fireteam_firing_positions_find` | 0x820bc178 |
| `swarm_spawners_update` | 0x820c3c60 |
| `target_get_prop_class` | 0x820db448 |
| `squad_patrol_update` | 0x820df600 |
| `actor_path_refresh` | 0x820e3100 |
| `path_state_find` | 0x820e3a20 |
| `flock_place` | 0x820ea348 |
| `squad_get_leader` | 0x820ed0b0 |
| `behavior_squad_patrol_next` | 0x820eda38 |
| `path_smooth` | 0x820ef2f0 |
| `flock_boid_state_iterator_new` | 0x820f1ed0 |
| `prop_get_ineffability` | 0x820f3b88 |
| `squad_ancestor_of_squad_group` | 0x820f3e38 |
| `actor_combat_check_collateral_damage` | 0x820f4f30 |
| `boundary_force_bounding_volume` | 0x820f54c0 |
| `prop_ref_get_awareness` | 0x820ff940 |
| `squad_fireteam_is_following` | 0x821020b0 |
| `stimulus_information_build_actor_information` | 0x82102108 |
| `prop_unacknowledge` | 0x82104488 |
| `actor_perception_unassailable` | 0x8210bc10 |
| `path_avoid_obstacles` | 0x8210bd30 |
| `prop_get_audibility` | 0x8210fa10 |
| `Vc_character_flocking_properties_internal::actor_get_character_definition_properties` | 0x8211e090 |
| `actor_perception_find_sense_position` | 0x82125df8 |
| `actor_player_prop_state_refresh` | 0x8212a4e0 |
| `path_node_from_hash_table` | 0x8212d3b8 |
| `actor_dynamic_firing_positions_update` | 0x8212d6d0 |
| `actor_update_squad_status` | 0x8212d918 |
| `squad_patrol_get_squad_patrol_index` | 0x8212e548 |
| `actor_prop_facing_category` | 0x8212fec0 |
| `firing_position_ref_hash_compare` | 0x82132bd0 |
| `firing_position_ref_hash_function` | 0x82132be0 |
| `actor_prop_update_new_position` | 0x82134798 |
| `prop_view_get` | 0x82134978 |
| `firing_position_forced_evaluation` | 0x82134b18 |
| `firing_position_setup_current_definition` | 0x82134c30 |
| `objective_task_test_activation` | 0x82134d40 |
| `task_squad_iterator_next` | 0x821351e8 |
| `ai_scripting_set_task` | 0x821352b8 |
| `task_squad_iterator_new` | 0x82135410 |
| `task_iterator_next` | 0x82135548 |
| `ai_scripting_reset_objective` | 0x82135650 |
| `path_heap_bubble_up` | 0x82136208 |
| `prop_status_refresh` | 0x8213d008 |
| `prop_get_battle_vector` | 0x8213e0a8 |
| `flock_destination_get_by_name` | 0x8213e308 |
| `c_character_perception_properties_internal::checksum` | 0x8213e508 |
| `flocks_dispose_from_old_map` | 0x8213eb38 |
| `path_attractor_weight` | 0x8213fe18 |
| `flock_delete` | 0x821404e8 |
| `actor_select_firing_position_internal` | 0x8214cb78 |
| `path_state_estimated_distance` | 0x8214d878 |
| `actor_get_perception_mode` | 0x8214f420 |
| `actor_perception_properties_get` | 0x8214f558 |
| `flocks_dispose_from_old_structure_bsp` | 0x8214f9e0 |
| `actor_combat_update` | 0x821506e8 |
| `c_firing_point_array::iterate_valid` | 0x821537d0 |
| `actor_perception_aiming_vector_test_blockage` | 0x82153878 |
| `actor_setup_evaluation_context` | 0x82153b20 |
| `actor_prop_state_refresh` | 0x82179730 |
| `actor_compute_prop_salience` | 0x82179c50 |
| `prop_state_get` | 0x8217abb8 |
| `actor_active_select_firing_position` | 0x8217b008 |
| `actor_perception_update` | 0x8217bbd0 |
| `actor_perception_unreachable` | 0x8217bec8 |
| `prop_should_refresh_status` | 0x8217bf98 |
| `prop_should_refresh_state` | 0x8217c020 |
| `squad_iterator_next` | 0x8217c310 |
| `prop_state_refresh` | 0x8217df58 |
