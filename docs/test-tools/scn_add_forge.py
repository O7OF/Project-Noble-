# 2026-11-13: Forge World palette (scenario "map variant palettes"): add
#   ff_vehicles      entry "boathog"      (Spartan Tanner): variants default / gauss / rocket / troop
#   ff_weapons_human entry "target_laser" (Drksteel): the airstrike designator + its Longsword / MAC / Pelican variants
# Entries are copies of existing ones (warthogs / assault_rifle) so every other field stays valid. Idempotent.
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
BOAT = r'stannerreach\objects\vehicles\human\boathog\boathog'
LASER = r'custom\weapons\target_laser'
NEW = {
    'ff_vehicles': ('warthogs', 'boathog', 'vehicle', [
        ('boathog', BOAT, 'default'), ('boathog_gauss', BOAT, 'gauss'), ('boathog_rocket', BOAT, 'rocket'),
        ('boathog_troop', BOAT, 'troop')]),
    'ff_weapons_human': ('assault_rifle', 'target_laser', 'weapon', [
        ('target_laser', LASER + r'\target_laser', ''),
        ('target_laser_longsword', LASER + r'\variants\Longsword\regular', ''),
        ('target_laser_carpet_bomb', LASER + r'\variants\Longsword\carpet_bomb', ''),
        ('target_laser_mac', LASER + r'\variants\Mac_round\mac_round', ''),
        ('target_laser_pelican_warthog', LASER + r'\variants\Pelican\Warthog', ''),
        ('target_laser_pelican_scorpion', LASER + r'\variants\Pelican\Scorpion', ''),
        ('target_laser_pelican_weapons', LASER + r'\variants\Pelican\Weapons', '')]),
}
try:
    from mbstart import B, mb_stop
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(r'levels\multi\forge_halo\forge_halo', 'scenario'))
    pals = tf.SelectField('Block:map variant palettes')
    changed = False
    for p in pals.Elements:
        pname = str(p.SelectField('name').Data)
        if pname not in NEW: continue
        src_name, new_name, ext, variants = NEW[pname]
        entries = p.SelectField('Block:entries')
        names = [str(e.SelectField('name').Data) for e in entries.Elements]
        if new_name in names: L('%s: %s already there' % (pname, new_name)); continue
        entries.DuplicateElement(names.index(src_name)); ne = list(entries.Elements)[-1]
        ne.SelectField('name').Data = new_name
        vb = ne.SelectField('Block:variants')
        while vb.Elements.Count > 1: vb.RemoveElement(vb.Elements.Count - 1)
        for i, (disp, obj, var) in enumerate(variants):
            if i > 0: vb.DuplicateElement(0)
            ve = list(vb.Elements)[i]
            ve.SelectField('display name').Data = disp
            ve.SelectField('object').Path = B.Tags.TagPath.FromPathAndExtension(obj, ext)
            ve.SelectField('variant name').Data = var
        L('%s: added %s with %d variant(s): %s' % (pname, new_name, vb.Elements.Count,
          ', '.join('%s=%s/%s' % (str(v.SelectField('display name').Data), str(v.SelectField('object').Path.RelativePathWithExtension), str(v.SelectField('variant name').Data)) for v in vb.Elements)))
        changed = True
    if changed: tf.Save(); L('SAVED')
    tf.Dispose(); mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc()[:1500])
open(os.path.join(T, os.environ.get('OUT', 'scn_add_forge.txt')), 'w').write('\n'.join(out))
