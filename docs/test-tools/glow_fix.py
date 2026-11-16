import importlib, traceback
LOG = r'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11\glow_log.txt'
S = 'objects\\characters\\elite_noble\\shaders\\'
WHITE = 'shaders\\default_bitmaps\\bitmaps\\color_white.bitmap'
# PH's AMF tints for these materials (tint 1): illum = white, honor lights = (0, 17, 110)
SPEC = {
    'default_ch_elite_honor_armor_01_illum': (1.0, 1.0, 1.0),
    'honor_lights_ch_elite_armor_emissive': (0.0, 17 / 255.0, 110 / 255.0),
}
log = []
def L(s):
    log.append(str(s)); open(LOG, 'w').write('\n'.join(log))
try:
    mb = importlib.import_module('bl_ext.iloveagoodcrisp_github_io.io_scene_foundry.managed_blam')
    for shader, rgb in SPEC.items():
        try:
            with mb.Tag(path=S + shader + '.shader') as t:
                block = t.tag.SelectField('Struct:render_method[0]/Block:parameters')
                el = t._Element_from_field_value(block, 'parameter name', 'self_illum_detail_map')
                if el is not None:
                    el.SelectField('Reference:bitmap').Path = t._TagPath_from_string(WHITE)
                    L('  %s detail -> %s' % (shader, t.get_path_str(el.SelectField('Reference:bitmap').Path)))
                el = t._Element_from_field_value(block, 'parameter name', 'self_illum_color')
                anim = el.SelectField('animated parameters')
                for sub in anim.Elements:
                    v = sub.SelectField('animation function').Value
                    c = t._GameColor_from_RGB(*rgb)
                    v.SetColor(0, c); v.SetColor(1, c)
                    L('  %s self_illum_color -> %s' % (shader, rgb))
                t.tag_has_changes = True
            L('OK ' + shader)
        except Exception:
            L('FAIL %s\n%s' % (shader, traceback.format_exc()))
except Exception:
    L('FATAL\n' + traceback.format_exc())
L('DONE')
