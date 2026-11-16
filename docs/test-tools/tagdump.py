# Generic ManagedBlam tag dumper: TAGS = "path.ext|path.ext" (relative to HREK tags), MAXEL per block, DEPTH.
import os, glob, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
HT = r"D:\Steam\steamapps\common\HREK\tags"
out = []; L = out.append
MAXEL = int(os.environ.get('MAXEL', '40')); DEPTH = int(os.environ.get('DEPTH', '6'))
SKIPT = {'Explanation', 'Custom', 'Pad', 'Skip', 'UselessPad', 'Terminator'}
def val(f):
    ft = str(f.FieldType)
    try:
        if ft == 'Reference': return str(f.Path.RelativePathWithExtension) if f.Path else ''
        if ft in ('StringId', 'OldStringId', 'String', 'LongString'): return str(f.Data)
        if ft.endswith('Enum'): return str(f.Value) + ':' + str(f.Items[f.Value].EnumName if f.Value < len(list(f.Items)) else '?')
        if hasattr(f, 'Data'):
            d = f.Data
            try: return ",".join(f"{x:g}" if isinstance(x, float) else str(x) for x in d)
            except TypeError: return str(d)
        if hasattr(f, 'RawValue'): return hex(f.RawValue)
        if hasattr(f, 'Value'): return str(f.Value)
    except Exception as e: return '?' + str(e)[:30]
    return ''
def dump(fields, depth, pre):
    for f in fields:
        ft = str(f.FieldType)
        if ft in SKIPT: continue
        nm = str(f.FieldName)
        if ft == 'Block':
            n = f.Elements.Count
            L(f"{pre}{nm} [{n}]")
            if depth < DEPTH:
                for i in range(min(n, MAXEL)):
                    L(f"{pre}  #{i}"); dump(f.Elements[i].Fields, depth + 1, pre + '    ')
        elif ft in ('Struct', 'Array'):
            L(f"{pre}{nm} {{")
            els = f.Elements if ft == 'Array' else [f.Elements[0]] if hasattr(f, 'Elements') else []
            try:
                for e in els: dump(e.Fields, depth + 1, pre + '  ')
            except Exception as e: L(pre + '  ?' + str(e)[:60])
        elif ft == 'Data':
            L(f"{pre}{nm} = <data>")
        else:
            v = val(f)
            if v not in ('', '0', '0,0', '0,0,0', '0x0') or os.environ.get('ZEROS'): L(f"{pre}{nm} = {v}")
try:
    from mbstart import B, mb_stop
    paths = []
    for spec in os.environ['TAGS'].split('|'):
        if '*' in spec: paths += [os.path.relpath(p, HT) for p in sorted(glob.glob(os.path.join(HT, spec)))]
        else: paths.append(spec)
    for p in paths:
        base, ext = os.path.splitext(p)
        L(f"\n######## {p}")
        try:
            tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(base, ext[1:]))
            dump(tf.Fields, 0, '')
            tf.Dispose()
        except Exception as e: L("load failed: " + str(e)[:200])
    mb_stop()
except Exception:
    L("ERROR\n" + traceback.format_exc())
open(os.path.join(T, os.environ.get('OUT', 'tagdump.txt')), 'w', encoding='utf-8').write("\n".join(out))
