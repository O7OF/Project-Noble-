# Mean base-texture brightness inside the change-colour mask areas. PAIRS = "label=base_path;cc_path|..." (absolute paths, tif/tga).
# cc: R = primary mask, G = secondary mask. Reports mean RGB(byte/255) of base overall, weighted by cc.R, weighted by cc.G, and mean cc.R / cc.G.
import bpy, os, numpy as np, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []
def load(p, size=None):
    im = bpy.data.images.load(p, check_existing=False)
    if size and tuple(im.size) != tuple(size): im.scale(size[0], size[1])
    w, h = im.size
    a = np.empty(w * h * 4, dtype=np.float32); im.pixels.foreach_get(a)
    r = (a.reshape(h, w, 4), (w, h)); bpy.data.images.remove(im); return r
try:
    for spec in os.environ['PAIRS'].split('|'):
        label, paths = spec.split('=', 1); bp, cp = paths.split(';')
        try:
            base, sz = load(bp); cc, _ = load(cp, sz)
            lum = base[..., :3].mean(axis=2)
            R = cc[..., 0]; G = cc[..., 1]
            wr = (lum * R).sum() / max(R.sum(), 1e-6); wg = (lum * G).sum() / max(G.sum(), 1e-6)
            out.append('%-34s %dx%d base mean %.3f | in primary mask %.3f | in secondary mask %.3f | mask R mean %.2f G mean %.2f | base alpha %.2f' % (label, sz[0], sz[1], lum.mean(), wr, wg, R.mean(), G.mean(), base[..., 3].mean()))
        except Exception as e:
            out.append('%s: %s' % (label, e))
except Exception:
    out.append(traceback.format_exc())
open(os.path.join(T, os.environ.get('OUT', 'maskstats.txt')), 'w').write('\n'.join(out))
