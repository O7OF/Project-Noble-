# Dump region/permutation -> meshes -> parts -> material shader path for elite_noble.render_model
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(r'objects\characters\elite_noble\elite_noble', 'render_model'))
    mats = tf.SelectField('Block:materials')
    mp = {}
    for i, e in enumerate(mats.Elements):
        p = ''
        for f in e.Fields:
            if str(f.FieldType) == 'Reference':
                try: p = str(f.Path.RelativePathWithExtension) if f.Path else ''
                except Exception: pass
                break
        mp[i] = p
    meshes = tf.SelectField('Struct:render geometry/Block:meshes')
    mparts = {}
    for mi, m in enumerate(meshes.Elements):
        lst = []
        for p in m.SelectField('Block:parts').Elements:
            idx = None
            for f in p.Fields:
                if str(f.FieldName) == 'render method index':
                    try: idx = int(f.Value)
                    except Exception:
                        try: idx = int(f.Data)
                        except Exception: pass
                    break
            lst.append(idx)
        mparts[mi] = lst
    regs = tf.SelectField('Block:regions')
    r0 = list(regs.Elements)[0]; L('REGFIELDS ' + ','.join(str(f.FieldName) for f in r0.Fields)); p0 = list(r0.SelectField('Block:permutations').Elements)[0]; L('PERMFIELDS ' + ','.join(str(f.FieldName)+':'+str(f.FieldType) for f in p0.Fields))
    for r in regs.Elements:
        rn = str(r.SelectField('name').Data)
        for pe in r.SelectField('Block:permutations').Elements:
            pn = str(pe.SelectField('name').Data)
            try: mi0 = int(pe.SelectField('mesh index').Data)
            except Exception: mi0 = None
            try: cnt = int(pe.SelectField('mesh count').Data)
            except Exception: cnt = 1
            if mi0 is None or mi0 < 0: continue
            for k in range(mi0, mi0 + max(cnt, 1)):
                L('%s/%s mesh %d: %s' % (rn, pn, k, ' | '.join('%s=%s' % (i, os.path.basename(mp.get(i, '?'))) for i in mparts.get(k, []))))
    mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc())
open(os.path.join(T, os.environ.get('OUT', 'rm_all.txt')), 'w').write('\n'.join(out))



