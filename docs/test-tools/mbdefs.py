# Explore ManagedBlam tag definitions (no tag files): group -> element definition -> fields.
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []
try:
    from mbstart import B, mb_stop
    G = B.Tags.TagGroupType
    out.append("TagGroupType members: " + ", ".join(sorted(set(m.Name for m in clr_t.GetMembers()))) if False else "")
    groups = list(G.GetTagGroups()) if hasattr(G, 'GetTagGroups') else None
    out.append(f"groups: {len(groups) if groups else groups}")
    if groups: out.append(" ".join(str(g.GroupTag) for g in groups))
    g = G.GetDefinition('bipd') if hasattr(G, 'GetDefinition') else None
    out.append(f"bipd def: {g} parent {getattr(g,'ParentGroupTag',None)}")
    ed = g.ElementDefinition
    out.append(f"ElementSize {ed.ElementSize}")
    seen = set()
    def props(o):
        t = o.GetType()
        return t.FullName + " {" + ", ".join(f"{p.Name}={p.GetValue(o, None)}" for p in t.GetProperties() if p.GetIndexParameters().Length == 0) + "}"
    for i, f in enumerate(ed.Fields):
        tn = f.GetType().FullName
        line = f"{i:3} {str(f.FieldType):14} size {f.FieldSize:4} {f.RawFieldName[:50]}"
        out.append(line)
        if tn not in seen:
            seen.add(tn); out.append("     " + props(f)[:900])
        if i > 60: break
    mb_stop()
except Exception:
    out.append("ERROR\n" + traceback.format_exc())
open(os.path.join(T, os.environ.get('OUT', 'mbdefs.txt')), 'w').write("\n".join(out))
