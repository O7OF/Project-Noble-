import bpy, os, sys, traceback, importlib
LOG = r'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11\build_shaders_log.txt'
D = r'D:\Steam\steamapps\common\HREK\data\objects\characters\elite_noble\bitmaps'
# material: (diffuse, normal, emission) - texture names from PH's AMF shader records
SPEC = {
    'elite_council_helmet': ('ch_helmet_councilor_01_diff', 'ch_helmet_councilor_01_nm', None),
    'elite_ossoona_helmet': ('elite_ossoona_helmet_armor_diffuse_4k', 'elite_ossoona_helmet_armor_normal_2k', None),
    'elite_combat_armor': ('mp_newelite_diffuse', 'newelite_zbump', None),
    'default_ch_elite_armor_02_honor': ('ch_elite_armor_02', 'ch_elite_armor_02_nm', None),
    'default_sh_elite_honor_armor_01': ('ch_elite_honor_armor_01', 'ch_elite_honor_armor_01_nm', None),
    'default_ch_elite_honor_armor_01_illum': ('ch_elite_honor_armor_01_em', None, 'ch_elite_honor_armor_01_em'),
    'honor_lights_ch_elite_armor_emissive': ('ch_elite_armor_emissive', None, 'ch_elite_armor_emissive'),
}
log = []
def L(s):
    log.append(str(s)); open(LOG, 'w').write('\n'.join(log))

def img(name, noncolor):
    p = os.path.join(D, name + '.tif')
    im = bpy.data.images.load(p, check_existing=True)
    if noncolor:
        im.colorspace_settings.name = 'Non-Color'
    return im

try:
    sb = importlib.import_module('bl_ext.iloveagoodcrisp_github_io.io_scene_foundry.tools.shader_builder')
    for mname, (dif, nrm, emi) in SPEC.items():
        m = bpy.data.materials.get(mname)
        if m is None:
            L('NO MATERIAL ' + mname); continue
        m.use_nodes = True
        nt = m.node_tree; nt.nodes.clear()
        out = nt.nodes.new('ShaderNodeOutputMaterial')
        bsdf = nt.nodes.new('ShaderNodeBsdfPrincipled')
        nt.links.new(bsdf.outputs[0], out.inputs[0])
        if dif:
            t = nt.nodes.new('ShaderNodeTexImage'); t.image = img(dif, False)
            nt.links.new(t.outputs['Color'], bsdf.inputs['Base Color'])
        if nrm:
            t = nt.nodes.new('ShaderNodeTexImage'); t.image = img(nrm, True)
            nm = nt.nodes.new('ShaderNodeNormalMap')
            nt.links.new(t.outputs['Color'], nm.inputs['Color'])
            nt.links.new(nm.outputs['Normal'], bsdf.inputs['Normal'])
        if emi:
            t = nt.nodes.new('ShaderNodeTexImage'); t.image = img(emi, False)
            nt.links.new(t.outputs['Color'], bsdf.inputs['Emission Color'])
            bsdf.inputs['Emission Strength'].default_value = 1.0
        m.nwo.shader_path = ''
        m.nwo.uses_blender_nodes = True
        try:
            sb.build_shader(m, False)
            L('BUILT %s -> %s' % (mname, m.nwo.shader_path))
        except Exception:
            L('BUILD FAILED %s\n%s' % (mname, traceback.format_exc()))
    bpy.ops.wm.save_mainfile()
    L('SAVED')
    if '--export' in sys.argv:
        r = bpy.ops.nwo.export_scene()
        L('EXPORT ' + str(r))
except Exception:
    L('FATAL\n' + traceback.format_exc())
L('DONE')
