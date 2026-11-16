# Where are pad/skip fields? Dump native tag field entries behind ManagedBlam TagFieldDefinition.Field.
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    from System.Runtime.InteropServices import Marshal
    from System import IntPtr
    from System.Reflection import BindingFlags as BF
    fl = BF.Public | BF.NonPublic | BF.Instance
    from System.Reflection import Pointer
    def ip(x):
        if type(x).__name__ == 'Pointer':
            from System.Runtime.Serialization import SerializationInfo, FormatterConverter, StreamingContext, StreamingContextStates
            import System
            si = SerializationInfo(System.Type.GetType('System.Reflection.Pointer'), FormatterConverter())
            from System.Runtime.Serialization import ISerializable
            ISerializable(x).GetObjectData(si, StreamingContext(StreamingContextStates.All))
            return si.GetValue('_ptr', System.Type.GetType('System.IntPtr')).ToInt64()
        try: return x.ToInt64()
        except Exception: return int(x)
    def priv(o, n):
        f = o.GetType().GetField(n, fl); return f.GetValue(o) if f else None
    G = B.Tags.TagGroupType
    def sub(ed, name):
        for f in ed.Fields:
            if str(f.RawFieldName).startswith(name): return f.Element
    hl = G.GetDefinition('hlmt').ElementDefinition
    cases = [('hlmt/new damage info/nodes', sub(sub(hl, 'new damage info'), 'nodes')),
             ('scnr/detail object collection palette', sub(G.GetDefinition('scnr').ElementDefinition, 'detail object collection palette')),
             ('bipd', G.GetDefinition('bipd').ElementDefinition)]
    for nm, ed in cases:
        L(f"=== {nm} size {ed.ElementSize}")
        L(" elem privs: " + ", ".join(f"{n}={priv(ed, n)}" for n in ('m_definition', 'm_element_definition_table', 'm_fields', 'm_parent')))
        fs = list(ed.Fields)
        for f in fs[:12]:
            fp = ip(priv(f, 'm_field'))
            L(f"  {str(f.FieldType):12} {f.FieldSize:4} {str(f.RawFieldName)[:30]:30} Field={fp:X} id={priv(f,'m_field_id')} tbl={priv(f,'m_field_definition_table')}")
        try:
            p0 = ip(priv(fs[0], 'm_field'))
            d0 = ip(priv(ed, 'm_definition')); L(' m_definition raw: ' + ' '.join(f'{Marshal.ReadInt64(IntPtr(d0 + 8*j)) & 0xFFFFFFFFFFFFFFFF:X}' for j in range(10)))
            L(" raw from first Field:")
            for k in range(14):
                q = [Marshal.ReadInt64(IntPtr(p0 + 8 * (4 * k + j))) for j in range(4)]
                L(f"  +{32*k:4X}: " + " ".join(f"{v & 0xFFFFFFFFFFFFFFFF:016X}" for v in q))
        except Exception as e:
            L(" raw failed: " + str(e))
    mb_stop()
except Exception:
    L("ERROR\n" + traceback.format_exc())
open(os.path.join(T, os.environ.get('OUT', 'mbpad.txt')), 'w').write("\n".join(out))
