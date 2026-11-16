# Triangles (index count / 3) per material in render models: MODELS = "a.render_model|b.render_model".
import os, collections, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
def fld(el, *names):
    for f in el.Fields:
        n = str(f.FieldName).split('#')[0].split(':')[0].strip()
        if n in names: return f
    return None
try:
    from mbstart import B, mb_stop
    for p in os.environ['MODELS'].split('|'):
        base, ext = os.path.splitext(p)
        tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(base, ext[1:]))
        mats = fld(tf, 'materials')
        names = []
        for e in mats.Elements:
            r = fld(e, 'render method')
            names.append(os.path.basename(str(r.Path.RelativePathWithExtension)) if r and r.Path else '?')
        rg = fld(tf, 'render geometry')
        rge = rg.Elements[0] if hasattr(rg, 'Elements') else rg
        meshes = fld(rge, 'meshes')
        tri = collections.Counter()
        for m in meshes.Elements:
            parts = fld(m, 'parts')
            for pt in parts.Elements:
                mi = fld(pt, 'render method index'); ic = fld(pt, 'index count')
                k = int(getattr(mi, 'Value', None) if hasattr(mi, 'Value') else mi.Data) if mi is not None else -1
                tri[names[k] if 0 <= k < len(names) else f'#{k}'] += (int(ic.Data) if ic is not None else 0) // 3
        L(f"\n### {p}: {len(names)} materials, {meshes.Elements.Count} meshes")
        for n, c in sorted(tri.items()):
            L(f"{c:8} {n}")
        tf.Dispose()
    mb_stop()
except Exception:
    L("ERROR\n" + traceback.format_exc())
open(os.path.join(T, os.environ.get('OUT', 'matshare.txt')), 'w').write("\n".join(out))
