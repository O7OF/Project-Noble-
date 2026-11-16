# Counts mesh parts per material index in elite_noble.render_model, and lists which regions/permutations point at meshes using given material indices (WANT env, default 80,100,101,79).
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(r'objects\characters\elite_noble\elite_noble', 'render_model'))
    want = set(int(x) for x in os.environ.get('WANT', '79,80,100,101').split(','))
    meshes = None
    for path in ('Block:meshes', 'Struct:render geometry/Block:meshes', 'Struct:render_geometry/Block:meshes'):
        try: meshes = tf.SelectField(path)
        except Exception: meshes = None
        if meshes is not None: L('meshes path ' + path); break
    if meshes is None:
        L('top fields: ' + ', '.join(str(f.FieldName) + ':' + str(f.FieldType) for f in tf.Root.Fields))
        raise SystemExit
    L('meshes: %d' % meshes.Elements.Count)
    mesh_use = {}
    first = True
    for mi, m in enumerate(meshes.Elements):
        parts = m.SelectField('Block:parts')
        for pi, p in enumerate(parts.Elements):
            names = [str(f.FieldName) + ':' + str(f.FieldType) for f in p.Fields]
            if first:
                L('part fields: ' + ', '.join(names)); first = False
            idx = None
            for f in p.Fields:
                if str(f.FieldName).lower() in ('render method index', 'material index', 'material'):
                    try: idx = int(f.Value)
                    except Exception:
                        try: idx = int(f.Data)
                        except Exception: pass
                    break
            if idx in want:
                mesh_use.setdefault(mi, []).append((pi, idx))
    L('meshes using wanted materials: ' + str({k: v for k, v in mesh_use.items()}))
    regs = tf.SelectField('Block:regions')
    for r in regs.Elements:
        rn = str(r.SelectField('name').Data)
        for pe in r.SelectField('Block:permutations').Elements:
            pn = str(pe.SelectField('name').Data)
            hits = []
            for f in pe.Fields:
                nm = str(f.FieldName)
                if 'mesh index' in nm:
                    try: hits.append((nm, int(f.Value)))
                    except Exception: pass
                if nm in ('mesh index', 'mesh count'):
                    pass
            mi0 = None
            try: mi0 = int(pe.SelectField('mesh index').Value)
            except Exception: pass
            try: cnt = int(pe.SelectField('mesh count').Value)
            except Exception: cnt = 1
            if mi0 is not None:
                for k in range(mi0, mi0 + max(cnt, 1)):
                    if k in mesh_use:
                        L('region %s perm %s mesh %d parts %s' % (rn, pn, k, mesh_use[k]))
    mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc())
open(os.path.join(T, os.environ.get('OUT', 'rm_parts_use.txt')), 'w').write('\n'.join(out))
