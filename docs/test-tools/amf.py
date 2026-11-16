"""Reader for Reclaimer's AMF v2 model files (Adjutant Model Format)."""
import struct, math
class AMF:
    def __init__(s, path):
        b = s.b = open(path, 'rb').read()
        assert b[:4] == b'AMF!', 'not an AMF file'
        s.version = struct.unpack_from('<f', b, 4)[0]
        s.name, o = s.cstr(8)
        nc, na, mc, ma, rc, ra, sc, sa = struct.unpack_from('<8i', b, o)
        s.nodes = []
        o = na
        for _ in range(nc):
            n, o = s.cstr(o)
            par, child, sib = struct.unpack_from('<3h', b, o); o += 6
            pos = struct.unpack_from('<3f', b, o); o += 12
            rot = struct.unpack_from('<4f', b, o); o += 16
            s.nodes.append(dict(name=n, parent=par, pos=pos, rot=rot))
        s.markers = []
        o = ma
        for _ in range(mc):
            n, o = s.cstr(o); c, a = struct.unpack_from('<2i', b, o); o += 8
            for k in range(c):
                r, p, node = struct.unpack_from('<BBh', b, a)
                pos = struct.unpack_from('<3f', b, a + 4); rot = struct.unpack_from('<4f', b, a + 16)
                s.markers.append(dict(name=n, region=r, perm=p, node=node, pos=pos, rot=rot)); a += 32
        s.regions = []
        o = ra
        for _ in range(rc):
            n, o = s.cstr(o); pc, pa = struct.unpack_from('<2i', b, o); o += 8
            perms = []
            for _ in range(pc):
                pn, pa = s.cstr(pa)
                vtype, node = struct.unpack_from('<BB', b, pa); pa += 2
                vc, va, fc, fa, smc, sma = struct.unpack_from('<6i', b, pa); pa += 24
                scale = struct.unpack_from('<f', b, pa)[0]; pa += 4
                mtx = None
                if not math.isnan(scale):
                    mtx = struct.unpack_from('<12f', b, pa); pa += 48
                perms.append(dict(name=pn, vtype=vtype, node=node, vc=vc, va=va, fc=fc, fa=fa, smc=smc, sma=sma, scale=scale, mtx=mtx))
            s.regions.append(dict(name=n, perms=perms))
        s.shaders = []
        o = sa
        for _ in range(sc):
            n, o = s.cstr(o)
            assert not n.startswith('*'), 'terrain shader not handled'
            tex = []
            for _ in range(8):
                t, o = s.cstr(o)
                if t != 'null':
                    tile = struct.unpack_from('<2f', b, o); o += 8
                    tex.append((t, tile))
                else:
                    tex.append(None)
            tints = [tuple(b[o + 4*i:o + 4*i + 4]) for i in range(4)]; o += 16
            transp, cc = b[o], b[o+1]; o += 2
            s.shaders.append(dict(name=n, tex=tex, tints=tints, transparent=transp, cc_only=cc))
    def cstr(s, o):
        e = s.b.index(b'\0', o); return s.b[o:e].decode('latin1'), e + 1
    def mesh(s, p):
        """Vertices [(pos, normal, uv, [(node, weight)])], triangles [(a,b,c)], submeshes [(shader, start, count)]"""
        b = s.b; o = p['va']; verts = []
        for _ in range(p['vc']):
            pos = struct.unpack_from('<3f', b, o); nrm = struct.unpack_from('<3f', b, o + 12); uv = struct.unpack_from('<2f', b, o + 24); o += 32
            inf = []
            if p['vtype'] == 1:
                idx = []
                while len(idx) < 4:
                    v = b[o]; o += 1
                    if v == 255: break
                    idx.append(v)
                w = struct.unpack_from('<%df' % len(idx), b, o); o += 4 * len(idx)
                inf = list(zip(idx, w))
            elif p['vtype'] == 2:
                v = b[o]; o += 1
                assert b[o] == 255; o += 1
                # rigid-boned: the index is into this permutation's node map (one entry), not the skeleton
                inf = [(p['node'] if p['node'] != 255 else v, 1.0)]
            else:
                inf = [(p['node'], 1.0)] if p['node'] != 255 else []
            verts.append((pos, nrm, uv, inf))
        end_v = o
        fmt, sz = ('<3H', 6) if p['vc'] <= 0xFFFF else ('<3i', 12)
        tris = [struct.unpack_from(fmt, b, p['fa'] + i * sz) for i in range(p['fc'])]
        sub = [struct.unpack_from('<hii', b, p['sma'] + i * 10) for i in range(p['smc'])]
        return verts, tris, sub, end_v
