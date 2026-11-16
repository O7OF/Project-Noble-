import importlib, traceback
LOG = r'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11\cc_shaders_log.txt'
B = 'objects\\characters\\elite_noble\\bitmaps\\'
S = 'objects\\characters\\elite_noble\\shaders\\'
SPEC = {
    'elite_council_helmet': {'change_color_map': 'ch_helmet_councilor_01_diff_cc'},
    'elite_ossoona_helmet': {'change_color_map': 'elite_ossoona_helmet_armor_diffuse_4k_cc'},
    'default_ch_elite_armor_02_honor': {'change_color_map': 'ch_elite_armor_02_cc'},
    'default_sh_elite_honor_armor_01': {'base_map': 'ch_elite_honor_armor_01_grey', 'change_color_map': 'ch_elite_honor_armor_01_cc'},
    'default_ch_elite_honor_armor_01_illum': {'change_color_map': 'cc_full_primary'},
    'honor_lights_ch_elite_armor_emissive': {'change_color_map': 'cc_full_primary'},
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
                    el.SelectField('Reference:bitmap').Path = t._TagPath_from_string(B + bmp + '.bitmap')
                    L('  %s %s -> %s' % (shader, pname, t.get_path_str(el.SelectField('Reference:bitmap').Path)))
                t.tag_has_changes = True
            L('OK ' + shader)
        except Exception:
            L('FAIL %s\n%s' % (shader, traceback.format_exc()))
except Exception:
    L('FATAL\n' + traceback.format_exc())
L('DONE')
