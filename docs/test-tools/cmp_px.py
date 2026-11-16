# Raw pixel samples (Blender reader, linear floats -> also sRGB-encoded bytes) of a few textures at fixed top-left coords.
import bpy, os, numpy as np, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
D = r"D:\Steam\steamapps\common\HREK\data\objects\characters\elite_noble\bitmaps"
out = []
def enc(l): return np.where(l <= 0.0031308, l * 12.92, 1.055 * np.power(np.maximum(l, 1e-9), 1 / 2.4) - 0.055)
try:
    for n in ('ch_elite_honor_armor_01', 'ch_elite_honor_armor_01_grey', 'ch_helmet_councilor_01_diff'):
        im = bpy.data.images.load(os.path.join(D, n + '.tif'), check_existing=False)
        w, h = im.size
        a = np.empty(w * h * 4, dtype=np.float32); im.pixels.foreach_get(a); a = a.reshape(h, w, 4)
        out.append('%s %dx%d colorspace=%s alpha_mode=%s depth=%s' % (n, w, h, im.colorspace_settings.name, im.alpha_mode, im.depth))
        for (x, y) in ((w // 2, h // 2), (w // 4, h // 4), (3 * w // 4, h // 3), (w // 3, 2 * h // 3)):
            p = a[h - 1 - y, x]
            out.append('  (%d,%d) lin %s -> byte %s' % (x, y, np.round(p, 3).tolist(), np.round(np.concatenate([enc(p[:3]), p[3:]]) * 255).astype(int).tolist()))
        bpy.data.images.remove(im)
except Exception:
    import traceback; out.append(traceback.format_exc())
open(os.path.join(T, 'cmp_px.txt'), 'w').write('\n'.join(out))
