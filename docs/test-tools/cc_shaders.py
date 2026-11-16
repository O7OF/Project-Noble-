import importlib, traceback
LOG = r'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11\cc_shaders_log.txt'
B = 'objects\\characters\\elite_noble\\bitmaps\\'
S = 'objects\\characters\\elite_noble\\shaders\\'
# shader: {parameter: bitmap}  (PH's AMF slots: 0 diffuse, 2 colour change, 3 bump, 5 self-illum)
SPEC = {
    'elite_council_helmet': {'base_map': 'ch_helmet_councilor_01_diff', 'bump_map': 'ch_helmet_councilor_01_nm', 'change_color_map': 'ch_helmet_councilor_01_diff'},
    'elite_ossoona_helmet': {'base_map': 'elite_ossoona_helmet_armor_diffuse_4k', 'bump_map': 'elite_ossoona_helmet_armor_normal_2k', 'change_color_map': 'elite_ossoona_helmet_armor_diffuse_4k'},
    'elite_combat_armor': {'base_map': 'mp_newelite_diffuse', 'bump_map': 'newelite_zbump', 'change_color_map': 'elite_cc'},
    'default_ch_elite_armor_02_honor': {'base_map': 'ch_elite_armor_02', 'bump_map': 'ch_elite_armor_02_nm', 'change_color_map': 'ch_elite_armor_02'},
    'default_sh_elite_honor_armor_01': {'base_map': 'ch_elite_honor_armor_01', 'bump_map': 'ch_elite_honor_armor_01_nm', 'change_color_map': 'ch_elite_honor_armor_01_classic'},
    'default_ch_elite_honor_armor_01_illum': {'base_map': 'ch_elite_honor_armor_01_em', 'bump_map': 'ch_elite_honor_armor_01_nm', 'change_color_map': 'ch_elite_honor_armor_01_em', 'self_illum_map': 'ch_elite_honor_armor_01_em'},
    'honor_lights_ch_elite_armor_emissive': {'base_map': 'ch_elite_armor_emissive', 'bump_map': 'ch_elite_armor_02_nm', 'change_color_map': 'ch_elite_armor_emissive', 'self_illum_map': 'ch_elite_armor_emissive'},
}
log = []
def L(s):
    log.append(str(s)); open(LOG, 'w').write('\n'.join(log))
try:
    mb = importlib.import_module('bl_ext.iloveagoodcrisp_github_io.io_scene_foundry.managed_blam')
    for shader, params in SPEC.items():
        try:
            with mb.Tag(path=S + shader + '.shader') as t:
                block = t.tag.SelectField('Struct:render_method[0]/Block:parameters')
                for pname, bmp in params.items():
                    el = t._Element_from_field_value(block, 'parameter name', pname)
                    if el is None:
                        L('  %s: no parameter %s' % (shader, pname)); continue
                    el.SelectField('Reference:bitmap').Path = t._TagPath_from_string(B + bmp + '.bitmap')
                    L('  %s %s -> %s' % (shader, pname, t.get_path_str(el.SelectField('Reference:bitmap').Path)))
                t.tag_has_changes = True
            L('OK ' + shader)
        except Exception:
            L('FAIL %s\n%s' % (shader, traceback.format_exc()))
except Exception:
    L('FATAL\n' + traceback.format_exc())
L('DONE')
