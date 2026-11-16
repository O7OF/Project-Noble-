import math
def qmul(a,b):
    ax,ay,az,aw=a; bx,by,bz,bw=b
    return (aw*bx+ax*bw+ay*bz-az*by, aw*by-ax*bz+ay*bw+az*bx, aw*bz+ax*by-ay*bx+az*bw, aw*bw-ax*bx-ay*by-az*bz)
def qconj(q): return (-q[0],-q[1],-q[2],q[3])
def qrot(q,v):
    p=qmul(qmul(q,(v[0],v[1],v[2],0.0)),qconj(q)); return p[:3]
def qnorm(q):
    n=math.sqrt(sum(c*c for c in q)); return tuple(c/n for c in q)
def absolute(nodes, conj=False):
    out=[None]*len(nodes)
    for i,n in enumerate(nodes):
        q=qnorm(n['rot']); q=qconj(q) if conj else q
        if n['parent']<0: out[i]=(n['pos'],q)
        else:
            pp,pq=out[n['parent']]; assert pp is not None
            t=qrot(pq,n['pos']); out[i]=(tuple(pp[k]+t[k] for k in range(3)), qmul(pq,q))
    return out
