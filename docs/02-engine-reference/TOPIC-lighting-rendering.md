# Lighting, shadows, atmosphere and rendering (Xbox 360 retail names)

For changing how Elites/Spartans are lit (Armory preview, in-game), fog/sky/atmosphere, shadows and effects.

**Reads-as architecture:**
- **Per-object lighting**: `render_objects_static_lighting.obj` + `lightmap_sample.obj` (dynamic objects sample the lightmap / probes), `object_lights.obj` (lights attached to objects), `lights.obj`/`c_lights_view`, `lighting_interface` (12), `effect_lightprobe` (probes used by effects).
- **Cinematic lighting**: `cinematic_lighting.obj`, `new_cinematic_lighting_definitions`, `new_cinematic_sh_lighting` - per-shot lights and SH lighting overrides for characters in cinematics. This is the engine's built-in way to relight characters independently of the level (likely what menu/Armory scenes can use).
- **Shadows**: `render_lightmap_shadows` / `c_lightmap_shadows_view`; tunables `g_shadow_generate_depth_bias`, `..._slope_depth_bias`, `g_shadow_apply_*` (shadow acne/peter-panning controls).
- **Atmosphere**: `atmosphere*`, `planar_fog`, `render_patchy_fog`, `scenario_sky`, `c_atmosphere_fog_interface`, rain light volumes; **light volumes** (`light_volume*`, `c_light_volume_gpu`).
- **Draw path**: `c_object_renderer` (19), `c_rasterizer` (28), `c_structure_renderer`, `c_transparency_renderer`, `c_render_shield`, `c_water_renderer`, `c_wet_renderer`, `c_imposter_renderer`, screenshot engine (`c_screenshot_render_engine`).

**Things you can do with it:**
1. Elite lighting: first try data - add or edit lights/probe data on the menu/Armory scene (object lights, cinematic light definitions) before touching code; use these modules to find which fields the engine reads.
2. Shadow quality fixes in MCC map to the `g_shadow_*` tunables.
3. Exposure: `g_texture_cam_exposure` and `scripted_exposure` globals control texture-camera exposure (Armory/portrait renders).
4. The draw-path class list is the starting point if you need to hook rendering from Noble (MCC's x64 build uses a different D3D layer, so match by structure, not address).

Source of everything below: names from the leaked Omaha linker map applied to the retail Xbox 360 build (halo3_cache_release, Jul 24 2010). "VA" = retail virtual address (image base 0x82000000). Names/addresses are facts from the map; "reads as" notes are interpretation until the function body is verified. MCC x64 `haloreach.dll` shares the engine design but not the addresses: use the matching method in `README.md` (shared strings, call shape).

## Source modules (98)

| source file (.obj) | code symbols | first VA |
|---|---|---|
| atmosphere_control | 4 | 0x820b2d20 |
| render_imposter | 13 | 0x820b3510 |
| rasterizer_globals | 2 | 0x820b3598 |
| render_water | 28 | 0x820b5070 |
| rasterizer_surface_tiling | 1 | 0x820b7be8 |
| rasterizer_xenon_main | 16 | 0x820baf78 |
| atmosphere | 9 | 0x820bca40 |
| atmosphere_definition | 1 | 0x820bcdc8 |
| rasterizer_projection | 5 | 0x820bd090 |
| lights | 31 | 0x820be180 |
| render_player_view | 43 | 0x820c2200 |
| rasterizer_states | 29 | 0x820c35e8 |
| rasterizer_surfaces | 13 | 0x820c3800 |
| light_volumes | 26 | 0x820c7548 |
| render_rain_sheets | 6 | 0x820c7d00 |
| render_structure | 42 | 0x820c98b8 |
| render_cameras | 10 | 0x820c9ec0 |
| rasterizer_text | 20 | 0x820d03a0 |
| render_tessellation | 1 | 0x820d3ab8 |
| render_bbsquad | 8 | 0x820d3df0 |
| render_lightmap_shadows | 19 | 0x820d3f10 |
| render_first_person_view | 5 | 0x820d40f0 |
| lightmap_sample | 6 | 0x820d8bb0 |
| planar_fog | 11 | 0x820dba50 |
| render_objects | 33 | 0x820de658 |
| light_volume_definitions | 1 | 0x820e2660 |
| rasterizer_instance_occlusion | 10 | 0x820eafc0 |
| render_object_attachments | 6 | 0x820f1ce0 |
| render_lights | 26 | 0x820f2340 |
| render_objects_static_lighting | 14 | 0x820f2d88 |
| render_wet | 9 | 0x820f5ff8 |
| light_volume_gpu | 34 | 0x820fc010 |
| render_visibility_collection | 4 | 0x82104d48 |
| render_mesh | 20 | 0x8210d4d0 |
| render_lens_flares | 9 | 0x8210db38 |
| scenario_lightmap_definitions | 4 | 0x821138f8 |
| rasterizer_samplers | 9 | 0x82116fa8 |
| render_rasterizer_utilities | 4 | 0x82119490 |
| render_method_submit | 29 | 0x8211f118 |
| render_decals | 1 | 0x82122f90 |
| render_method_definitions | 2 | 0x8212e850 |
| effect_lightprobe | 12 | 0x82130fd0 |
| rasterizer_primitives | 5 | 0x82135f60 |
| rasterizer_occlusion_queries | 10 | 0x82136360 |
| lighting_interface | 15 | 0x82137028 |
| rasterizer_dx9_dynamic_geometry | 17 | 0x82139f60 |
| rasterizer_geometry | 12 | 0x8213c868 |
| render_sprite | 1 | 0x8213f6b8 |
| render_rain_light_volume | 13 | 0x8214a3d8 |
| rasterizer_textures_cache | 4 | 0x82156618 |
| render_blurs | 9 | 0x8215bfd0 |
| rasterizer_shaders | 8 | 0x82161428 |
| render_decorators | 4 | 0x82164aa0 |
| rasterizer_stipple | 2 | 0x82165018 |
| render_visibility | 4 | 0x82165cd0 |
| render_models | 3 | 0x82173958 |
| render_view_stack | 3 | 0x82179050 |
| camera_fx_settings | 19 | 0x82179080 |
| rasterizer_resource_definitions | 6 | 0x82180830 |
| render_transparents | 11 | 0x82183b28 |
| rasterizer_render_targets | 8 | 0x8218c9b8 |
| render_rain_occlusion | 4 | 0x82192228 |
| render_view | 3 | 0x821a5d68 |
| render_time | 1 | 0x821a95c8 |
| render_patchy_fog | 4 | 0x821b7280 |
| render_ssao | 2 | 0x821b8200 |
| render_cloud | 2 | 0x821bc620 |
| rasterizer_hue_saturation | 4 | 0x821cdb38 |
| render_texture_camera_view | 18 | 0x821ce158 |
| rasterizer_main | 23 | 0x821d2708 |
| render_2d_screen_view | 7 | 0x821f2be8 |
| rasterizer_vertex_definitions | 3 | 0x82203a68 |
| render_shield_cache | 8 | 0x82204d28 |
| rasterizer_performance_throttles | 3 | 0x82222840 |
| rasterizer_loading_screen | 4 | 0x82239348 |
| rasterizer_dynamic_render_targets | 3 | 0x8223daa8 |
| rasterizer_global_shaders | 1 | 0x82246b90 |
| rasterizer_implicit_geometry | 5 | 0x82246ce0 |
| rasterizer_loading_screen_auntie_dot | 25 | 0x822478f0 |
| rasterizer_d3d_allocations | 7 | 0x8224c9a0 |
| rasterizer_LUT_control | 2 | 0x8224daa0 |
| rasterizer_textures | 2 | 0x82250a18 |
| render_low_res_transparency | 5 | 0x82251d68 |
| rasterizer_loading_screen_resource_loader | 2 | 0x82256598 |
| rasterizer_shader_definitions | 4 | 0x82258868 |
| render_game_state | 8 | 0x82260580 |
| render_tron_effect | 1 | 0x82262e38 |
| render_shield | 11 | 0x82265bb8 |
| render_active_camo | 4 | 0x82266b10 |
| render_fur | 2 | 0x82267538 |
| object_lights | 3 | 0x8237e258 |
| light_definitions | 1 | 0x82395788 |
| light_volume_states | 2 | 0x82468ad8 |
| new_cinematic_sh_lighting | 1 | 0x824a04f0 |
| cinematic_lighting | 10 | 0x824a4778 |
| scenario_sky | 6 | 0x82531a60 |
| new_cinematic_lighting_definitions | 2 | 0x8258d9d8 |
| render_emblem | 2 | 0x826df540 |

## Main classes (by number of functions)

`c_player_view` (34), `c_rasterizer` (32), `c_rasterizer_states` (29), `c_object_renderer` (25), `c_structure_renderer` (23), `c_water_renderer` (21), `c_lights_view` (21), `c_rasterizer_shaders` (14), `c_surface` (12), `c_lightmap_shadows_view` (12), `c_lighting_interface` (12), `c_rasterizer_dynamic_geometry` (12), `c_light_volume_gpu` (11), `c_dot_load_screen` (11), `c_rasterizer_samplers` (10), `c_transparency_renderer` (10), `c_imposter_renderer` (9), `c_light_volume_system` (9), `c_wet_renderer` (9), `c_rasterizer_draw_string` (8), `c_bbsquad_renderer` (8), `c_atmosphere_fog_interface` (7), `c_light_volume` (7), `c_rain_light_volume` (7), `c_render_shield` (7), `c_d3d_resource_allocator` (7), `c_texture_camera_view` (6), `s_rasterizer_projection` (5), `c_first_person_view` (5), `s_lightmap_sample` (5), `c_blur` (5), `s_scripted_exposure` (5), `c_vertex_declaration_table` (5), `c_fullscreen_view` (5), `PostProcess` (5), `s_cinematic_light_globals` (5), `s_atmosphere_parameters` (4), `c_view_stack` (4), `c_render_surface` (4), `c_rain_sheets` (4)

## Named globals / tunables (26)

Many of these are developer tunables or debug switches (booleans/floats); changing them at runtime in MCC needs the matching MCC address.

| global | VA |
|---|---|
| g_chud_debug_highlight_object_index | 0x82a221cc |
| g_chud_debug_highlight_object_color | 0x82a2b224 |
| g_leap_flight_path_scale | 0x82a40dfc |
| g_stream_manager_probe_state_strings | 0x82a43fa0 |
| g_texture_cam_exposure | 0x82a45638 |
| g_shadow_generate_slope_depth_bias | 0x82a8f9b0 |
| g_shadow_generate_depth_bias | 0x82a8f9b4 |
| g_shadow_generate_dynamic_lights_slope_depth_bias | 0x82a8f9b8 |
| g_shadow_generate_dynamic_lights_depth_bias | 0x82a8f9bc |
| g_shadow_apply_depth_bias | 0x82a900b8 |
| g_shadow_apply_slope_depth_bias | 0x82a900bc |
| g_scripted_exposure_game_globals_allocator | 0x82a94154 |
| g_light_data_allocator | 0x82acfcc8 |
| g_lights_game_globals_allocator | 0x82acfcd4 |
| g_effect_lightprobe_slot_data_allocator | 0x82ad1080 |
| g_effect_lightprobe_data_allocator | 0x82ad108c |
| g_light_volume_system_allocator | 0x82ad11b0 |
| g_light_volume_location_allocator | 0x82ad11bc |
| g_light_volume_allocator | 0x82ad11c8 |
| g_cinematic_light_globals_allocator | 0x82ad1288 |
| g_text_drop_shadow_style_enum_table | 0x82b07ce8 |
| g_lightmap_shadows_view_stack_element | 0x82bb28a0 |
| g_light_visibility_input | 0x82bb3230 |
| g_lights_view_stack_element | 0x82bcd380 |
| g_visible_fog_bit_vector | 0x82bd09e8 |
| g_use_command_buffer_shadow_generate | 0x83150d50 |

## Key functions (303 matched; first 70 shown, rest in the CSV)

| function | VA |
|---|---|
| `s_atmosphere_parameters::fog_active` | 0x820b2d20 |
| `s_atmosphere_parameters::normalize` | 0x820b9e18 |
| `c_atmosphere_fog_interface::sample_atmosphere` | 0x820bca40 |
| `s_atmosphere_parameters::precalculate_runtime_constants` | 0x820bcbe0 |
| `s_solo_fog_parameters::get_actual_fog_thickness` | 0x820bcdc8 |
| `light_disconnect_from_map` | 0x820be180 |
| `c_light_volume_system::frame_advance_all` | 0x820c7548 |
| `light_reconnect_to_map_force_inside_world_expensive` | 0x820c8378 |
| `render_patchy_fog_callback` | 0x820ca040 |
| `light_new_attached` | 0x820d2a20 |
| `c_imposter_renderer::render_object_imposters_end` | 0x820d3da0 |
| `c_lightmap_shadows_view::compute_object_shadow_resolution` | 0x820d3f10 |
| `c_structure_renderer::render_static_lighting` | 0x820da298 |
| `c_player_view::static_lighting_setup_internals` | 0x820da768 |
| `c_planar_fog_render::build_visible_fog_list` | 0x820dba50 |
| `c_lightmap_shadows_view::setup_camera` | 0x820decb0 |
| `c_structure_renderer::render_single_pass_lighting` | 0x820df128 |
| `c_rasterizer_states::setup_targets_single_pass_lighting` | 0x820df550 |
| `c_light_volume_definition::calc_lod_amount` | 0x820e2660 |
| `c_lightmap_shadows_view::render` | 0x820e3c18 |
| `c_lightmap_shadows_view::render_first_person_shadow` | 0x820e3da0 |
| `c_light_volume_system::submit_all` | 0x820f0da8 |
| `c_lights_view::initialize_simple_light` | 0x820f2340 |
| `render_objects_prepare_for_window` | 0x820f2d88 |
| `light_adjust_position_from_attachment_last_minute` | 0x820f8ac8 |
| `??$set_shader_state@Us_render_vertex_shader_constant_list@c_light_volume_gpu@@@c_light_volume_gpu@@ABAXPBU?$s_` | 0x820fc010 |
| `c_lightmap_shadows_view::compute_visibility` | 0x82104df8 |
| `add_simple_light_callback` | 0x8210a0b8 |
| `c_lights_view::push_simple_lights_for_transparent_pass` | 0x8210a340 |
| `c_lightmap_shadows_view::object_shadow_visible` | 0x8210df58 |
| `scenario_lightmap_find_bsp` | 0x821138f8 |
| `c_lightmap_shadows_view::submit_visibility_and_render` | 0x82116970 |
| `c_lightmap_shadows_view::render_shadow_generate` | 0x82116a18 |
| `c_rasterizer_states::setup_targets_shadow_generate` | 0x82116c88 |
| `c_rasterizer_states::setup_targets_static_lighting` | 0x82116d78 |
| `c_atmosphere_fog_interface::render_screen_atm_fog` | 0x82117140 |
| `c_object_renderer::render_shadows_generate` | 0x821185a8 |
| `c_lights_view::render_cheap_screenspace_lights_from_draw_list` | 0x82118a38 |
| `?list_iterator_new@?$c_simple_list@Us_visible_light@@$0BJA@@@QAAPAUs_visible_light@@PAJ@Z` | 0x82119430 |
| `c_light_volume_gpu::render_transparent_callback` | 0x821231c8 |
| `c_light_volume_gpu::render` | 0x82123718 |
| `render_screenspace_light_callback` | 0x8212dfb8 |
| `light_build_render_geometry` | 0x8212e2d0 |
| `effect_lighting_update` | 0x82130fd0 |
| `get_max_light_definition_and_instance` | 0x82137028 |
| `c_lights_view::render_screenspace_light` | 0x821382c0 |
| `light_get_geometry` | 0x8213ba90 |
| `c_lights_view::screenspace_light_setup_static_textures` | 0x8213c2f0 |
| `c_lightmap_shadows_view::render_third_person_shadows` | 0x8213d7c0 |
| `c_rain_light_volume::push_light_volume` | 0x8214a3d8 |
| `c_lights_view::push_rain_light_volumes` | 0x8214a520 |
| `light_move_map_attachment` | 0x8214bb28 |
| `c_lighting_interface::set_lightmap_bounce_light_constants` | 0x8214bfb0 |
| `light_calculate_priority_and_fade` | 0x8214eb10 |
| `c_object_renderer::render_object_occlusion_boxes` | 0x82159590 |
| `c_lights_view::render_screenspace_light_internal` | 0x8215ab80 |
| `light_is_screen_space` | 0x82162528 |
| `light_theoretically_produces_light` | 0x82162f08 |
| `s_render_type_default_opaque::render_object_instances` | 0x821632b0 |
| `c_lights_view::fade_and_process_highest_priority_visible_lights` | 0x821639f0 |
| `c_lighting_interface::setup_object_lighting_for_pass` | 0x82164bd0 |
| `c_object_renderer::setup_per_object_render_state` | 0x821650c0 |
| `s_render_type_default_opaque::render_object_mesh_parts` | 0x82165260 |
| `c_lighting_interface::set_lightmap_analytical_light_constants` | 0x82166c58 |
| `c_structure_renderer::determine_cluster_lighting_attributes` | 0x82167860 |
| `s_setup_lighting::setup_structure_instance_part` | 0x82168110 |
| `?handle_structure_instance_part@?$s_render_structure_cached@Us_setup_lighting@@@@QAAXXZ` | 0x821681a8 |
| `?iterate_all_parts@?$s_visible_structure_part_iterator@U?$s_render_structure_two_pass@Us_setup_lighting@@@@@@Q` | 0x821686c0 |
| `c_object_renderer::render_object_mesh_part` | 0x8216a180 |
| `render_mesh_part` | 0x8216b988 |
