import struct, pickle, sys
from reachmap import Map
def open_map(D):
    m=Map(D)
    def sid(v):
        if v==0: return ''
        ns=(v>>19)&0xFF; i=v&0x7FFFF
        g = (i if i<m.ns[0] else sum(m.ns)+(i-m.ns[0])) if ns==0 else sum(m.ns[:ns])+i
        return m.strs[g] if g<len(m.strs) else '<%08X>'%v
    m.sid=sid
    o=m.off(m.idx); ng,_,ga,nt,_,ta=struct.unpack_from('<iIQiIQ',m.tags,o)
    groups=[m.tags[m.off(ga)+i*16:m.off(ga)+i*16+4][::-1].decode('latin1') for i in range(ng)]
    m.taglist=[]
    for i in range(nt):
        g,salt,p=struct.unpack_from('<hHI',m.tags,m.off(ta)+i*8)
        m.taglist.append((groups[g] if g>=0 else None, m.ptr(p), m.names[i]))
    return m
def block(m,a,o):
    c,p,_=struct.unpack_from('<iII',m.tags,m.off(a)+o); return c,m.ptr(p)
def dump_mode(m,ti,out=print):
    g,a,n=m.taglist[ti]
    out('### %d %s %s name=%s'%(ti,g,n,m.sid(m.u32(a))))
    rc,ra=block(m,a,0x0C)
    regions=[]
    for r in range(rc):
        ro=ra+r*16; rn=m.sid(m.u32(ro)); pc,pa=block(m,ro,4)
        perms=[]
        for p in range(pc):
            po=pa+p*24; pn=m.sid(m.u32(po)); mi,mc=struct.unpack_from('<hH',m.tags,m.off(po)+4)
            perms.append((pn,mi,mc))
        regions.append((rn,perms))
        out('  region %-22s %d perms: '%(rn,pc)+', '.join('%s[%d+%d]'%p for p in perms))
    return regions
if __name__=='__main__':
    m=open_map(sys.argv[1])
    for t in sys.argv[2:]: dump_mode(m,int(t))
