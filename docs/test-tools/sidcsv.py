"""Write Noble's string_ids CSV (Index,Total,ID,String) from map-slim output (header/nstab/stridx/strtab .bin)."""
import struct, sys, os, csv, io
def load(d):
    r = lambda n: open(os.path.join(d, n), 'rb').read()
    si, st, ns = r('stridx.bin'), r('strtab.bin'), r('nstab.bin')
    offs = struct.unpack('<%di' % (len(si)//4), si); ns = list(struct.unpack('<%di' % (len(ns)//4), ns))
    strs = [st[o:st.index(b'\0', o)].decode('utf-8', 'replace') if o >= 0 else '' for o in offs]
    return strs, ns
def sid(g, ns):
    tot = sum(ns)
    if g < ns[0]: return g
    if g >= tot: return g - tot + ns[0]
    k = 1; base = ns[0]
    while g >= base + ns[k]: base += ns[k]; k += 1
    return (k << 19) | (g - base)
def rows(d):
    strs, ns = load(d); total = 0
    for g, s in enumerate(strs):
        total += len(s.encode('utf-8')) + 1
        yield g, total, sid(g, ns), s
if __name__ == '__main__':
    out = io.StringIO(); w = csv.writer(out, lineterminator='\r\n'); w.writerow(['Index', 'Total', 'ID', 'String'])
    for r in rows(sys.argv[1]): w.writerow(r)
    open(sys.argv[2], 'w', encoding='utf-8', newline='').write(out.getvalue())
