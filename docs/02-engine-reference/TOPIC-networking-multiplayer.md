# Networking and multiplayer: where the engine does it (Xbox 360 retail names)

How Reach multiplayer is built, by module, so a networking change (more players, lobby codes, dedicated hosting, join-in-progress, bandwidth) can be traced to the right code. Read next to `repo-docs\limits-players*.md` and `knowledge\drafts\draft-session-networking.md` (Noble's own 32-slot work).

**Reads-as architecture (from the names; verify before relying):**
- **Session** = `c_network_session` (89 functions) + `c_network_session_membership` (35) + `c_network_session_parameters` (16 classes of replicated settings; `network_session_parameters.obj` alone has 628 functions: every replicated setting - game variant, map variant, max players, privacy, etc. - has its own get/set/encode/decode/validate). One peer is host; `c_host_selector`, `c_session_tracker`, `c_network_session_security`.
- **Transport** = `c_network_link` -> `c_network_channel` / `c_network_channel_manager` -> `c_network_connection`, with `c_network_message_queue` / `c_network_message_handler` and bandwidth modules (`network_bandwidth`, `network_bandwidth_estimation`). Secure addresses: `transport_secure_address_to_string`.
- **Simulation (gameplay replication)** = `c_simulation_world` (71) with `c_simulation_view`, `c_simulation_watcher`, `c_simulation_entity_database`, and one `*_entity_definition` / `*_event_definition` class per replicated thing (biped, vehicle, weapon, projectile, item, device, generic, player, megalo team, map variant, statborg, breakable surface, debug globals; events such as `biped_melee_damage`, `player_loadout_request`, `player_set_respawn_target_transform`, `vehicle_flip`). Scheduling: `c_replication_scheduler`, `c_replication_entity_manager`, control/event views.
- **Lifecycle / matchmaking** = `life_cycle_handler_*` state handlers (prepare_map, select_host, assemble_match/team, start, arbitration, recycle) and `logic_matchmaking_*` (seeker, desirability, session rating, query builder). Mostly Xbox Live logic - not needed for Noble's own master-server flow, but the state names document the host handshake.
- **Files / storage** = `network_blf*` (BLF file format), `network_storage_*`, `network_http_request_queue`.

**Things you can pull out of it:**
1. The exact list of what the host replicates and with which event types (the entity/event definition tables above) - needed for any 32 -> 128 player or spectator work.
2. Session-parameter encode/decode functions give the wire format of game variants and map variants (useful for external tools and lobby codes).
3. Named tunables below (bandwidth, quality, abort reasons) mark where the engine's own limits live.
4. The join/leave and host-migration state machine (life cycle handlers + `c_network_session` methods) is the reference for join-in-progress and rejoin behaviour.

Source of everything below: names from the leaked Omaha linker map applied to the retail Xbox 360 build (halo3_cache_release, Jul 24 2010). "VA" = retail virtual address (image base 0x82000000). Names/addresses are facts from the map; "reads as" notes are interpretation until the function body is verified. MCC x64 `haloreach.dll` shares the engine design but not the addresses: use the matching method in `README.md` (shared strings, call shape).

## Source modules (245)

| source file (.obj) | code symbols | first VA |
|---|---|---|
| network_session_membership | 46 | 0x820b13b0 |
| network_storage_files | 130 | 0x820b2f88 |
| simulation_watcher | 26 | 0x820b2f90 |
| network_life_cycle | 50 | 0x820b3660 |
| network_lsp_leaderboard | 11 | 0x820b3d68 |
| network_party | 4 | 0x820b3dc8 |
| online_guide_xenon | 25 | 0x820b3f70 |
| network_session_parameters_game | 46 | 0x820b4948 |
| network_memory | 18 | 0x820b4ab0 |
| network_link | 14 | 0x820b4b28 |
| simulation_world | 88 | 0x820b4c08 |
| network_bandwidth | 20 | 0x820b5768 |
| network_stream_manager | 36 | 0x820b6bb8 |
| transport_security | 17 | 0x820b6e50 |
| transport | 8 | 0x820b6f40 |
| online_file_transfer_tracker | 13 | 0x820b7218 |
| life_cycle_manager | 9 | 0x820b7788 |
| voice_xenon | 24 | 0x820b80a0 |
| network_channel_manager | 35 | 0x820b9288 |
| network_game_definitions | 35 | 0x820bced8 |
| voice_availability | 2 | 0x820bcf40 |
| network_managed_session | 33 | 0x820bd398 |
| network_session | 68 | 0x820bd468 |
| network_active_roster_join_manager | 11 | 0x820bebd8 |
| network_join | 11 | 0x820bec38 |
| online_xenon | 4 | 0x820bf118 |
| simulation_quality | 9 | 0x820c1f68 |
| network_session_parameters_quality | 24 | 0x820c2d68 |
| network_session_parameters | 628 | 0x820c2f28 |
| network_session_parameters_voice | 20 | 0x820c3110 |
| network_session_parameters_base | 16 | 0x820c3520 |
| network_http_buffer_downloader | 21 | 0x820c38a8 |
| simulation_game_action | 79 | 0x820c3ba0 |
| life_cycle_helpers | 45 | 0x820c5ae0 |
| network_friends_leaderboard | 5 | 0x820c89f8 |
| network_session_interface | 31 | 0x820c8a80 |
| network_leaderboard | 7 | 0x820c8c18 |
| network_session_parameter_interface | 26 | 0x820c8dc0 |
| online_url | 32 | 0x820c8ed0 |
| life_cycle_handler_in_game | 10 | 0x820ca798 |
| online_lsp | 14 | 0x820caf10 |
| voice_observer | 20 | 0x820cb2d8 |
| network_statistics | 10 | 0x820cf328 |
| simulation_game_objects | 50 | 0x820cfde0 |
| simulation | 38 | 0x820d4cb0 |
| network_lsp_presence | 4 | 0x820db128 |
| network_globals | 5 | 0x820ddb50 |
| online_user_xenon | 20 | 0x820dde88 |
| network_lsp_dlc | 3 | 0x820e2d40 |
| simulation_queue | 20 | 0x820e9ca8 |
| network_banhammer | 20 | 0x820eb158 |
| network_storage_manifest | 5 | 0x820f2280 |
| simulation_encoding | 31 | 0x82100840 |
| channel_base | 11 | 0x8210d438 |
| network_session_membership_interface | 4 | 0x82124828 |
| network_storage_constants | 1 | 0x82128720 |
| simulation_game_position_updates | 11 | 0x821318f0 |
| transport_endpoint_winsock | 12 | 0x82132a60 |
| network_session_state | 16 | 0x8215dda8 |
| network_storage_cache | 16 | 0x821940d0 |
| transport_endpoint_pollster | 2 | 0x821a4098 |
| transport_address | 3 | 0x821a8c10 |
| replication_encoding | 19 | 0x821cd7f8 |
| online_achievements | 20 | 0x821e2640 |
| logic_matchmaking_configuration | 30 | 0x822026b8 |
| partylib | 6 | 0x82202e30 |
| simulation_debug_globals | 6 | 0x82205a48 |
| online_lsp_xenon | 8 | 0x822107d8 |
| network_utilities | 6 | 0x82210d70 |
| simulation_game_weapons | 162 | 0x822136e0 |
| online_rich_presence | 2 | 0x82216348 |
| network_configuration | 3 | 0x82219058 |
| network_active_roster | 23 | 0x822190d8 |
| network_message_gateway | 3 | 0x822249e8 |
| online_party_xenon | 1 | 0x822258e8 |
| network_webstats | 5 | 0x82225ad0 |
| network_broadcast_search | 2 | 0x82225df8 |
| voice_routing | 2 | 0x82226d10 |
| simulation_game_engine_megalo_state_data | 34 | 0x8223ed80 |
| network_debug_dump | 7 | 0x82269668 |
| online_friend_xenon | 8 | 0x8226ab58 |
| online_marketplace | 15 | 0x8226ad40 |
| online_presence_xenon | 7 | 0x8226ad40 |
| online_session_search_xenon | 10 | 0x8226c940 |
| transport_qos | 7 | 0x8226d0d8 |
| life_cycle_component_search | 4 | 0x8227b790 |
| life_cycle_component_gather | 7 | 0x8227c040 |
| life_cycle_handler_none | 4 | 0x8227c9f8 |
| life_cycle_handler_pre_game | 10 | 0x8227cb70 |
| life_cycle_handler_start_game | 7 | 0x8227f3d8 |
| life_cycle_handler_end_game_write_stats | 6 | 0x82281108 |
| life_cycle_handler_leaving | 4 | 0x822816b0 |
| life_cycle_handler_joining | 4 | 0x82281b08 |
| life_cycle_handler_matchmaking_start | 11 | 0x82281d58 |
| life_cycle_handler_matchmaking_recycle | 12 | 0x82282d40 |
| life_cycle_handler_matchmaking_find_and_assemble_team | 10 | 0x82284500 |
| life_cycle_handler_matchmaking_assemble_team | 10 | 0x82284f18 |
| life_cycle_handler_matchmaking_assemble_match | 15 | 0x82284fc8 |
| life_cycle_handler_matchmaking_find_and_assemble_match | 9 | 0x82285af8 |
| life_cycle_handler_matchmaking_select_host | 5 | 0x82287860 |
| life_cycle_handler_matchmaking_arbitration | 16 | 0x82287de8 |
| life_cycle_handler_matchmaking_prepare_map | 26 | 0x82289628 |
| life_cycle_handler_in_match | 5 | 0x8228c200 |
| life_cycle_handler_end_match_write_stats | 7 | 0x8228c728 |
| life_cycle_handler_matchmaking_prepare_rematch | 8 | 0x8228d270 |
| life_cycle_handler_post_match | 10 | 0x8228dfa8 |
| life_cycle_state_handler | 3 | 0x82296ea0 |
| life_cycle_matchmaking_quality | 2 | 0x82297020 |
| logic_matchmaking_desirability | 10 | 0x8229d5a8 |
| network_session_parameters_matchmaking | 46 | 0x822a06b0 |
| network_session_parameters_session | 22 | 0x822a26b0 |
| network_session_parameters_map_variant | 8 | 0x822a3630 |
| network_session_parameters_game_variant | 8 | 0x822a3b40 |
| online_arbitration_xenon | 4 | 0x822a5be8 |
| online_statistics_xenon | 14 | 0x822a5d60 |
| online_avatar_xenon | 4 | 0x822a6850 |
| network_time | 1 | 0x822a8398 |
| network_session_manager | 1 | 0x822ab260 |
| network_session_security | 11 | 0x822b5dc0 |
| network_session_parameters_ui | 5 | 0x822bc8c8 |
| network_session_host | 14 | 0x822c7220 |
| logic_matchmaking_seeker | 13 | 0x822cc9d8 |
| logic_matchmaking_composition | 3 | 0x822cdb68 |
| logic_matchmaking_query_builder | 3 | 0x822d05b0 |
| network_storage_matchmaking_file_definitions | 8 | 0x822d8e38 |
| network_arbitration | 1 | 0x822dcd20 |
| online_file_megalo_categories | 3 | 0x822e13e8 |
| online_file_predefined_search_queries | 3 | 0x822e13e8 |
| network_server_signing | 5 | 0x822e13e8 |
| network_blf | 7 | 0x822e1890 |
| network_storage_machine_file_definitions | 1 | 0x822e9f38 |
| network_team_picker | 23 | 0x822ea068 |
| logic_matchmaking_session_rating | 1 | 0x822edcb8 |
| network_message_queue | 18 | 0x822f09b8 |
| network_channel | 24 | 0x822f2638 |
| network_connection | 19 | 0x822fa758 |
| network_session_message | 22 | 0x822fef68 |
| online_session_xenon | 4 | 0x82301430 |
| network_bandwidth_estimation | 2 | 0x82301b10 |
| simulation_object_glue | 8 | 0x82302a28 |
| simulation_game_interface | 17 | 0x82304470 |
| simulation_priority | 5 | 0x823063f8 |
| replication_entity_manager_view | 20 | 0x823070a8 |
| replication_event_manager_view | 12 | 0x82308c78 |
| replication_control_view | 10 | 0x823096b0 |
| simulation_definition_table | 2 | 0x82309a80 |
| simulation_queue_global_events | 18 | 0x8230cbc8 |
| simulation_view | 24 | 0x8230f840 |
| replication_entity_manager | 9 | 0x82311d40 |
| replication_event_manager | 4 | 0x82312508 |
| simulation_entity_database | 13 | 0x82312928 |
| simulation_event_handler | 4 | 0x82313db8 |
| simulation_players | 9 | 0x823143b0 |
| simulation_actors | 1 | 0x82315640 |
| simulation_queue_events | 4 | 0x82316078 |
| simulation_queue_entities | 14 | 0x82316638 |
| network_message_type_collection | 3 | 0x82317ac8 |
| network_message_handler | 9 | 0x82317d50 |
| logic_session_tracker | 15 | 0x8231c9c0 |
| online_file_metadata | 3 | 0x8231dec8 |
| online_file_transfer | 10 | 0x8231fb80 |
| network_web_events | 2 | 0x82320470 |
| network_friends | 9 | 0x823206c8 |
| network_presence | 7 | 0x82320a28 |
| network_http_request_queue | 13 | 0x823222c8 |
| network_user_messaging | 2 | 0x82323228 |
| voice_engine_xenon | 2 | 0x82323410 |
| voice_queue | 9 | 0x82323558 |
| network_messages_out_of_band | 9 | 0x82325cf0 |
| network_messages_connect | 9 | 0x823263e0 |
| network_messages_session_protocol | 41 | 0x82326ae8 |
| network_messages_session_membership | 19 | 0x82328b90 |
| network_messages_session_parameters | 8 | 0x8232ace0 |
| network_messages_simulation | 6 | 0x8232add8 |
| network_messages_simulation_synchronous | 17 | 0x8232b4f0 |
| network_messages_simulation_distributed | 6 | 0x8232c5d8 |
| network_messages_test | 4 | 0x8232c700 |
| online_files | 2 | 0x8232c988 |
| network_session_parameter_types | 1 | 0x8232cad8 |
| network_search | 1 | 0x8232ddb8 |
| network_whitelist | 1 | 0x8232f850 |
| replication_scheduler | 10 | 0x82337c20 |
| simulation_game_engine_sandbox | 11 | 0x82338c30 |
| simulation_game_engine_custom | 18 | 0x82339398 |
| simulation_game_engine_player | 180 | 0x8233a160 |
| simulation_game_device_machines | 43 | 0x8233a198 |
| simulation_game_projectiles | 113 | 0x8233a1c8 |
| simulation_game_engine_globals | 35 | 0x8233a210 |
| simulation_game_generics | 32 | 0x8233a260 |
| simulation_game_vehicles | 56 | 0x8233a370 |
| simulation_game_bipeds | 253 | 0x8233d018 |
| simulation_game_engine_megalo_team | 33 | 0x8233db80 |
| simulation_game_statborg | 29 | 0x8233eba8 |
| simulation_game_breakable_surfaces | 60 | 0x8233f368 |
| simulation_game_incidents | 15 | 0x8233f3a0 |
| simulation_map_variant | 31 | 0x8233fb70 |
| simulation_game_debug_globals | 28 | 0x82341130 |
| simulation_game_items | 29 | 0x823420e0 |
| simulation_game_damage | 34 | 0x82342170 |
| matchmaking_visualization_data_component | 18 | 0x82342200 |
| simulation_game_entities | 7 | 0x82361fa8 |
| simulation_game_internal | 4 | 0x82365618 |
| simulation_view_telemetry | 3 | 0x823661c0 |
| replication_entity | 4 | 0x823666f0 |
| simulation_game_events | 4 | 0x82366b88 |
| simulation_event_definition | 1 | 0x823670d8 |
| voice_shared_definitions | 2 | 0x82367340 |
| online_file_transfer_manager | 39 | 0x823674e8 |
| network_session_parameters_chunked | 4 | 0x8236a268 |
| network_session_parameters_film | 2 | 0x8236a628 |
| simulation_entity_encoding | 6 | 0x8236ac00 |
| simulation_game_object_index_state_data | 4 | 0x8236b230 |
| network_blf_compress | 21 | 0x8236b4e0 |
| online_file_transfer_tasks | 19 | 0x8236ce90 |
| online_file_request | 2 | 0x8236ec60 |
| game_engine_simulation | 8 | 0x824d8b90 |
| matchmaking_options_list_data_component | 27 | 0x8257d880 |
| voice_ring_buffer_effect | 12 | 0x825e1220 |
| online_file_screenshot_widget | 23 | 0x827001e0 |
| lobby_data_component | 28 | 0x82718db0 |
| online_file_details_pipe_component | 31 | 0x82718db0 |
| matchmaking_progress_data_component | 127 | 0x82725950 |
| network_status_component | 13 | 0x82729e50 |
| party_details_components | 46 | 0x8272a9f0 |
| matchmaking_voter_component | 6 | 0x8272a9f0 |
| networking_active_roster_writer_component | 8 | 0x8272a9f0 |
| network_url_component | 7 | 0x8272a9f0 |
| networking_squad_session_reader_component | 79 | 0x8272a9f0 |
| networking_squad_session_writer_component | 29 | 0x8272a9f0 |
| networking_active_roster_reader_component | 14 | 0x8272a9f0 |
| matchmaking_options_writer_component | 28 | 0x8272a9f0 |
| online_file_thumbnail_widget | 14 | 0x8273b540 |
| matchmaking_histogram_bitmap_widget | 7 | 0x82769130 |
| simulation_game_action_megalo | 6 | 0x827d7ac8 |
| online_file_deleter | 1 | 0x827d8470 |
| online_file_summary_listing | 2 | 0x827d85c0 |
| online_file_catalog_listing | 1 | 0x827d8740 |
| online_file_listing | 2 | 0x827d8870 |
| online_file_search_summary | 1 | 0x827d8c68 |
| online_file_predefined_search_summary | 1 | 0x827d8f80 |
| online_file_recommendations_fetch | 1 | 0x827d9088 |
| online_file_recommendation_submit | 1 | 0x827d9230 |
| online_file_details | 1 | 0x827d93b8 |
| online_file_tagger | 1 | 0x827d94f0 |
| online_file_identifier_list | 1 | 0x827d9600 |

## Main classes (by number of functions)

`c_network_session` (109), `Vc_matchmaking_progress_data_component` (98), `c_simulation_world` (86), `Vc_networking_squad_session_reader_component` (69), `c_network_session_membership` (45), `c_life_cycle_helpers` (44), `c_simulation_game_engine_player_entity_definition` (29), `c_simulation_device_entity_definition` (29), `c_simulation_generic_entity_definition` (29), `c_simulation_game_statborg_entity_definition` (29), `c_simulation_item_entity_definition` (29), `c_simulation_object_entity_definition` (28), `c_simulation_vehicle_entity_definition` (28), `c_simulation_biped_entity_definition` (28), `c_simulation_game_engine_megalo_team_definition` (28), `c_simulation_breakable_surface_group_entity_definition` (28), `c_simulation_map_variant_entity_definition` (28), `c_simulation_debug_globals_entity_definition` (28), `c_stream_manager` (27), `c_simulation_weapon_entity_definition` (27), `c_simulation_projectile_entity_definition` (27), `Vc_party_details_data_component` (27), `c_simulation_watcher` (25), `c_life_cycle_state_handler_matchmaking_prepare_map` (25), `Vc_matchmaking_options_list_data_component` (22), `c_network_connection` (21), `c_network_session_parameters` (20), `c_voice_observer` (20), `c_simulation_game_engine_globals_definition` (20), `Vc_networking_squad_session_writer_component` (20), `Vc_matchmaking_options_writer_component` (20), `Vc_lobby_data_component` (19), `c_network_message_queue` (18), `c_simulation_view` (18), `c_life_cycle_state_handler_matchmaking_arbitration` (17), `c_network_channel` (17), `c_replication_entity_manager_view` (17), `c_life_cycle_state_handler_matchmaking_assemble_match` (15), `c_network_session_parameter_base` (15), `c_simulation_action_weapon_fire_event_definition` (15)

## Named globals / tunables (145)

Many of these are developer tunables or debug switches (booleans/floats); changing them at runtime in MCC needs the matching MCC address.

| global | VA |
|---|---|
| g_managed_dll | 0x82027e5f |
| g_network_matchmaking_fake_progress_stage | 0x82a21ba4 |
| g_ghost_mode_names | 0x82a2d090 |
| g_network_debug_globals_basic_scoreboard_metrics_enabled | 0x82a43992 |
| g_simulation_biped_equipment_activation_event_definition | 0x82a43b60 |
| g_voice_bandwidth_use_subtype_strings | 0x82a43d70 |
| g_player_sources | 0x82a43e08 |
| g_simulation_view_reason_strings | 0x82a43ea0 |
| g_simulation_quality_machine_index | 0x82a43f10 |
| g_simulation_abort_reason_strings | 0x82a4430c |
| g_managed_channel_state_strings | 0x82a44374 |
| g_managed_channel_brief_state_strings | 0x82a4439c |
| g_simulation_object_glue_data_allocator | 0x82abd7e8 |
| g_simulation_debug_globals_allocator | 0x82abd840 |
| g_network_storage_cache | 0x82abe090 |
| g_simulation_custom_engine_globals_definition | 0x82ac9964 |
| g_simulation_game_engine_player_entity_definition | 0x82ac99b4 |
| g_simulation_player_editor_request_event_definition | 0x82ac99b8 |
| g_simulation_player_purchase_request_event_definition | 0x82ac99bc |
| g_simulation_player_loadout_request_event_definition | 0x82ac99c0 |
| g_simulation_player_money_grant_event_definition | 0x82ac99c4 |
| g_simulation_player_account_balance_notification_event_definition | 0x82ac99c8 |
| g_simulation_player_set_respawn_target_transform_event_definition | 0x82ac99cc |
| g_simulation_player_set_orbiting_camera_target_event_definition | 0x82ac99d0 |
| g_simulation_player_call_for_help_request_event_definition | 0x82ac99d4 |
| g_simulation_player_force_base_respawn_event_definition | 0x82ac99d8 |
| g_simulation_authority_ignored_predicted_position_event_definition | 0x82ac99dc |
| g_simulation_game_engine_megalo_team_entity_definition | 0x82ac9a20 |
| g_simulation_game_statborg_entity_definition | 0x82ac9a50 |
| g_simulation_breakable_surface_break_event_definition | 0x82ac9aa4 |
| g_simulation_breakable_surface_damage_aoe_event_definition | 0x82ac9aa8 |
| g_simulation_breakable_surface_group_entity_definition | 0x82ac9aac |
| g_simulation_map_variant_entity_definition | 0x82ac9afc |
| g_simulation_debug_globals_entity_definition | 0x82ac9b00 |
| g_simulation_biped_entity_definition | 0x82ac9b7c |
| g_simulation_biped_enter_vehicle_event_definition | 0x82ac9b80 |
| g_simulation_biped_exit_vehicle_event_definition | 0x82ac9b84 |
| g_simulation_biped_board_vehicle_event_definition | 0x82ac9b88 |
| g_simulation_biped_assassinate_event_definition | 0x82ac9b8c |
| g_simulation_biped_throw_initiate_event_definition | 0x82ac9b90 |
| g_simulation_biped_throw_release_event_definition | 0x82ac9b94 |
| g_simulation_biped_melee_initiate_event_definition | 0x82ac9b98 |
| g_simulation_biped_melee_damage_event_definition | 0x82ac9b9c |
| g_simulation_biped_melee_clang_event_definition | 0x82ac9ba0 |
| g_simulation_biped_pickup_event_definition | 0x82ac9ba4 |
| g_simulation_biped_pickup_item_request_event_definition | 0x82ac9ba8 |
| g_simulation_biped_laser_designation_event_definition | 0x82ac9bac |
| g_simulation_biped_debug_teleport_event_definition | 0x82ac9bb0 |
| g_simulation_biped_dodge_event_definition | 0x82ac9bb4 |
| g_simulation_unit_zoom_event_definition | 0x82ac9bb8 |
| g_simulation_item_entity_definition | 0x82ac9c14 |
| g_simulation_generic_entity_definition | 0x82ac9c6c |
| g_simulation_generic_garbage_entity_definition | 0x82ac9c70 |
| g_simulation_vehicle_entity_definition | 0x82ac9ccc |
| g_simulation_vehicle_flip_event_definition | 0x82ac9cd0 |
| g_simulation_vehicle_trick_event_definition | 0x82ac9cd4 |
| g_simulation_projectile_entity_definition | 0x82ac9d28 |
| g_simulation_projectile_attached_event_definition | 0x82ac9d2c |
| g_simulation_projectile_detonate_event_definition | 0x82ac9d30 |
| g_simulation_projectile_impact_effect_event_definition | 0x82ac9d34 |
| g_simulation_projectile_object_impact_effect_event_definition | 0x82ac9d38 |
| g_simulation_projectile_supercombine_request_event_definition | 0x82ac9d3c |
| g_simulation_request_projectile_attach_event_definition | 0x82ac9d40 |
| g_simulation_weapon_entity_definition | 0x82ac9da0 |
| g_simulation_request_weapon_fire_event_definition | 0x82ac9da4 |
| g_simulation_action_weapon_fire_event_definition | 0x82ac9da8 |
| g_simulation_weapon_reload_event_definition | 0x82ac9dac |
| g_simulation_weapon_pickup_event_definition | 0x82ac9db0 |
| g_simulation_weapon_put_away_event_definition | 0x82ac9db4 |
| g_simulation_weapon_drop_event_definition | 0x82ac9db8 |
| g_simulation_weapon_effect_event_definition | 0x82ac9dbc |
| g_simulation_weapon_empty_click_event_definition | 0x82ac9dc0 |
| g_simulation_weapon_tether_request_event_definition | 0x82ac9dc4 |
| g_simulation_device_entity_definition | 0x82ac9e18 |
| g_simulation_device_touch_event_definition | 0x82ac9e1c |
| g_simulation_damage_aftermath_event_definition | 0x82ac9e6c |
| g_simulation_damage_section_response_event_definition | 0x82ac9e70 |
| g_simulation_game_engine_request_boot_player_event_definition | 0x82ac9ec8 |
| g_megalo_chud_message_event_definition | 0x82ac9f10 |
| g_simulation_object_refresh_event_definition | 0x82ac9f70 |
| g_simulation_incident_event_definition | 0x82ac9fc4 |
| g_network_active_roster_mode_enum_table | 0x82b07cac |
| g_network_session_class_enum_table | 0x82b07cb8 |
| g_network_session_privacy_enum_table | 0x82b07cc4 |
| g_network_session_advertisement_mode_enum_table | 0x82b07cd0 |
| g_voice_mute_setting_enum_table | 0x82b07d84 |
| g_voice_output_setting_enum_table | 0x82b07d90 |
| g_voice_matchmaking_setting_enum_table | 0x82b07d9c |
| g_user_interface_voice_state_enum_table | 0x82b07dd8 |
| g_network_game_privacy_enum_table | 0x82b07dfc |
| g_session_game_mode_enum_table | 0x82b07e44 |
| g_network_game_type_enum_table | 0x82b07e68 |
| g_network_session_quality_enum_table | 0x82b07e74 |
| g_network_status_display_last_observer_timestamp | 0x82bd03a4 |
| g_network_configuration | 0x82bd28a8 |
| g_channel_manager_status_records | 0x82bd5078 |
| g_network_status_filter_name | 0x82bd6178 |
| g_priority_globals | 0x82bd6278 |
| g_simulation_quality_globals | 0x82bd6348 |
| g_definition_table | 0x82bd68d8 |
| g_network_user_messaging_globals | 0x82bd6968 |
| g_sound_channels | 0x82be2bf8 |
| g_network_configuration_initialized | 0x83150e2c |
| g_network_session_parameter_type_collection | 0x83150ea0 |
| g_network_storage_manifest_initialized | 0x83150ea4 |
| g_network_status_memory | 0x83150ea5 |
| g_network_status_link | 0x83150ea6 |
| g_network_status_channels | 0x83150ea7 |
| g_network_status_connections | 0x83151064 |
| g_network_status_message_queues | 0x83151065 |
| g_network_status_channel_manager | 0x83151066 |
| g_network_status_sessions | 0x83151067 |
| g_network_status_update_queues | 0x83151068 |
| g_network_status_leaderboard | 0x83151069 |
| g_network_status_idle_detection | 0x8315106a |
| g_network_interface_show_latency_and_framerate_metrics_on_chud | 0x8315106b |
| g_network_interface_fake_latency_and_framerate_metrics_on_chud | 0x8315106c |
| g_network_debug_globals_detailed_metrics_enabled | 0x8315106d |
| g_current_definition_table | 0x83151070 |
| g_network_join_data | 0x832facd8 |
| g_network_active_roster_join_manager_globals | 0x83309598 |
| g_network_banhammer_globals | 0x8330e588 |
| g_voice_routing | 0x833ab634 |
| g_voice_interface_globals | 0x833ab670 |
| g_voice_engine | 0x833acbc0 |
| g_voice_availability | 0x833acbc8 |
| g_voice_queue | 0x833acc10 |
| g_voice_observer | 0x833af858 |
| g_network_memory_status_line | 0x833afd00 |
| g_network_link_status_line | 0x833afe28 |
| g_network_channel_status_lines | 0x833aff50 |
| g_network_connection_status_lines | 0x833b23d0 |
| g_network_message_queue_status_lines | 0x833b6cd0 |
| g_network_channel_manager_status_lines | 0x833bb5d0 |
| g_network_session_status_lines | 0x833bfff8 |
| g_network_update_queue_status_lines | 0x833c3eb8 |
| g_network_leaderboard_query_status_lines | 0x833c4fd8 |
| g_network_leaderboard_write_status_lines | 0x833c5468 |
| g_network_idle_detection_status_lines | 0x833c66a8 |
| g_network_debug_globals_latency_and_framerate_metrics_enabled | 0x833c95e4 |

## Key functions (3057 matched; first 70 shown, rest in the CSV)

| function | VA |
|---|---|
| `c_network_session_membership::apply_peer_properties_update` | 0x820b13b0 |
| `c_network_session::is_host` | 0x820b3660 |
| `c_simulation_watcher::maintain_connection` | 0x820b3740 |
| `network_party_get_membership_non_local` | 0x820b3dc8 |
| `network_channel_iterate` | 0x820b4ab0 |
| `c_network_link::process_all_channels` | 0x820b4b28 |
| `c_simulation_world::process_playback_events` | 0x820b4c08 |
| `c_network_channel_manager_internal::check_stream_channel_connections` | 0x820b9288 |
| `s_network_session_privacy_mode::get_game_privacy` | 0x820bced8 |
| `c_network_session_membership::get_peer_from_secure_address` | 0x820bcfa0 |
| `managed_session_get_security_information` | 0x820bd398 |
| `c_network_session::get_secure_key` | 0x820bd468 |
| `network_active_roster_outgoing_join_get_squad_host_queued_join_lsp_request` | 0x820bebd8 |
| `network_join_flush_join_queue` | 0x820bec38 |
| `network_join_is_leaving_or_will_leave_posse` | 0x820bed40 |
| `peer_properties_update_encode` | 0x820bf3c8 |
| `player_configuration_host_encode` | 0x820c1b60 |
| `?request_change@?$c_generic_network_session_parameter@Us_network_quality_live_for_all_peers@@@@UAA_NPBUs_netwo` | 0x820c2d68 |
| `?set@?$c_generic_network_session_parameter@Us_network_quality_live_for_all_peers@@@@UAA_NPBUs_network_quality_` | 0x820c2e58 |
| `?request_change@?$c_generic_network_session_parameter@Us_network_session_privacy_mode@@@@UAA_NPBUs_network_ses` | 0x820c2f28 |
| `?set@?$c_generic_network_session_parameter@Us_network_session_privacy_mode@@@@UAA_NPBUs_network_session_privac` | 0x820c3030 |
| `?request_change@?$c_generic_network_session_parameter@Us_voice_repeater_configuration@@@@UAA_NPBUs_voice_repea` | 0x820c3110 |
| `?set@?$c_generic_network_session_parameter@Us_voice_repeater_configuration@@@@UAA_NPBUs_voice_repeater_configu` | 0x820c3200 |
| `?set@?$c_generic_network_session_parameter@Us_matchmaking_single_hopper_statistics@@@@UAA_NPBUs_matchmaking_si` | 0x820c3450 |
| `?set@?$c_generic_network_session_parameter@Us_network_session_parameter_request_campaign_quit@@@@UAA_NPBUs_net` | 0x820c3450 |
| `?set@?$c_generic_network_session_parameter@Us_network_session_parameter_speculative_migration@@@@UAA_NPBUs_net` | 0x820c3450 |
| `c_network_session_parameter_base::set_allowed` | 0x820c3520 |
| `network_session_qos_status_initialize` | 0x820c6d28 |
| `network_session_interface_get_local_user_properties` | 0x820c8a80 |
| `network_session_interface_set_local_user_properties` | 0x820c8b20 |
| `network_squad_session_get_hopper_statistics` | 0x820c8dc0 |
| `c_network_session_parameters::parameters_valid` | 0x820c9720 |
| `c_life_cycle_helpers::session_get_life_cycle_state` | 0x820ca848 |
| `c_network_time_statistics::average_events_per_second` | 0x820cf328 |
| `game_engine_player_entity_index_from_player_absolute_index` | 0x820d0f48 |
| `c_network_session::idle` | 0x820d1a08 |
| `network_active_roster_outgoing_join_data_encode` | 0x820d2b70 |
| `network_join_get_squad_join_result` | 0x820d2cb8 |
| `network_session_interface_get_social_preferences` | 0x820d2e60 |
| `stream_manager_get_status` | 0x820d5010 |
| `voice_device_is_connected` | 0x820d5ff8 |
| `c_network_session::leaving_session` | 0x820d6b98 |
| `network_join_is_join_to_squad_in_progress` | 0x820d6c00 |
| `network_life_cycle_get_target_session_for_join` | 0x820d6c50 |
| `network_join_is_any_join_in_progress` | 0x820d6c70 |
| `c_life_cycle_state_manager::verify_session_hierarchy` | 0x820d70e8 |
| `c_life_cycle_state_manager::verify_session_states` | 0x820d74a8 |
| `c_network_session_membership::does_player_exist` | 0x820d8fa0 |
| `network_bandwidth_update` | 0x820d9af0 |
| `c_network_session_membership::get_peer_from_managed_channel` | 0x820e1898 |
| `network_session_interface_update` | 0x820e2f80 |
| `c_life_cycle_state_handler_in_game::validate_host_session_parameters` | 0x820e5428 |
| `network_squad_session_get_map` | 0x820e58e0 |
| `network_squad_session_get_start_mode` | 0x820e5980 |
| `c_life_cycle_helpers::update_game_termination` | 0x820e5f78 |
| `network_session_interface_update_session` | 0x820e6478 |
| `network_squad_session_get_ui_game_mode` | 0x820e6b68 |
| `c_voice_observer::update_local_player_open_channel_masks` | 0x820e6cc8 |
| `c_voice_observer::update_local_player_open_channel_available_mask` | 0x820e7308 |
| `c_network_session_membership::all_peers_established` | 0x820e82a8 |
| `c_network_session::is_peer_joining_this_session` | 0x820eb070 |
| `c_simulation_world::update_queue_get_available_updates` | 0x820f3578 |
| `c_simulation_world::time_get_available` | 0x820f35c8 |
| `c_channel_base::animation_allows_interpolation_out` | 0x8210d438 |
| `network_session_interface_get_squad_session` | 0x821143c8 |
| `c_channel_base::setup_channel_base` | 0x821165e8 |
| `c_network_session_membership::get_first_peer` | 0x821246a0 |
| `network_session_get_membership` | 0x82124828 |
| `c_channel_base::get_animation_data` | 0x821377a8 |
| `c_channel_base::get_frame_count` | 0x821452d8 |
