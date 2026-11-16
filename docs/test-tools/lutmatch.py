# Histogram-match PH base-texture luminance (inside its primary cc mask) to the stock Reach Elite distribution (stock Minor pieces, inside their primary mask).
# Output lut_<name>.txt = 256 ints (new byte luminance for each old byte luminance), plus lutmatch.txt summary. Run with runpy (factory startup).
# ITEMS = "name=base_path;cc_path|..." (absolute)
import bpy, os, numpy as np, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
B = os.path.join(T, 'stock_bm')
out = []
def load(p, size=None):
    im = bpy.data.images.load(p, check_existing=False)
    if size and tuple(im.size) != tuple(size): im.scale(size[0], size[1])
    w, h = im.size
    a = np.empty(w * h * 4, dtype=np.float32); im.pixels.foreach_get(a)
    r = (a.reshape(h, w, 4), (w, h)); bpy.data.images.remove(im); return r
def lum_in_mask(bp, cp):
    base, sz = load(bp); cc, _ = load(cp, sz)
    lum = base[..., :3].mean(axis=2); m = cc[..., 0] > 0.5
    return lum[m] if m.sum() > 1000 else lum.ravel()
try:
    st = []
    for n in ('chest', 'thigh', 'shoulder', 'helmet'):
        st.append(lum_in_mask('%s\\elite_minor_%s_diffuseelite_minor_%s_diffuse_00_00.tga' % (B, n, n), '%s\\elite_minor_%s_ccelite_minor_%s_cc_00_00.tga' % (B, n, n)))
    st = np.concatenate(st); st.sort()
    out.append('stock pooled: n=%d mean %.3f  p5 %.3f p25 %.3f p50 %.3f p75 %.3f p95 %.3f p99 %.3f' % ((len(st), st.mean()) + tuple(np.percentile(st, [5, 25, 50, 75, 95, 99]))))
    for spec in os.environ['ITEMS'].split('|'):
        name, paths = spec.split('=', 1); bp, cp = paths.split(';')
        ph = lum_in_mask(bp, cp)
        # byte histogram CDF of PH luminance
        hist = np.bincount(np.clip(np.round(ph * 255).astype(int), 0, 255), minlength=256).astype(np.float64)
        cdf = np.cumsum(hist) / hist.sum()
        lut = np.clip(np.round(np.interp(cdf, np.linspace(0, 1, len(st)), st) * 255), 0, 255).astype(int)
        # keep it monotonic, and keep pure black black
        lut = np.maximum.accumulate(lut); lut[0] = 0
        open(os.path.join(T, 'lut_%s.txt' % name), 'w').write(','.join(str(v) for v in lut))
        out.append('%-12s n=%d mean %.3f p5 %.3f p50 %.3f p95 %.3f -> lut sample 32:%d 64:%d 128:%d 192:%d 240:%d' % ((name, len(ph), ph.mean()) + tuple(np.percentile(ph, [5, 50, 95])) + (lut[32], lut[64], lut[128], lut[192], lut[240])))
except Exception:
    out.append(traceback.format_exc())
open(os.path.join(T, 'lutmatch.txt'), 'w').write('\n'.join(out))
