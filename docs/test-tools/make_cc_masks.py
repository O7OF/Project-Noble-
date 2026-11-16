import bpy, os, numpy as np, traceback
T = r'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11'
D = r'D:\Steam\steamapps\common\HREK\data\objects\characters\elite_noble\bitmaps'
LOG = os.path.join(T, 'cc_masks_log.txt'); log = []
def L(s):
    log.append(str(s)); open(LOG, 'w').write('\n'.join(log))

def load(name, maxdim=2048):
    im = bpy.data.images.load(os.path.join(D, name + '.tif'))
    w, h = im.size
    s = min(1.0, maxdim / max(w, h))
    if s < 1.0: im.scale(int(w * s), int(h * s)); w, h = im.size
    a = np.empty(w * h * 4, dtype=np.float32); im.pixels.foreach_get(a)
    return a.reshape(h, w, 4), w, h

def save(arr, w, h, name):
    im = bpy.data.images.new(name, w, h, alpha=True, float_buffer=False)
    im.pixels.foreach_set(arr.astype(np.float32).ravel())
    im.filepath_raw = os.path.join(D, name + '.tif'); im.file_format = 'TIFF'; im.save()
    L('saved %s %dx%d' % (name, w, h))

def smooth(x, e0, e1):
    t = np.clip((x - e0) / max(e1 - e0, 1e-6), 0, 1); return t * t * (3 - 2 * t)

try:
    # armour masks: R = primary on the main plates, G = secondary on darker trim / recesses
    for src in ('ch_helmet_councilor_01_diff', 'elite_ossoona_helmet_armor_diffuse_4k', 'ch_elite_armor_02', 'ch_elite_honor_armor_01'):
        px, w, h = load(src)
        lum = 0.299 * px[..., 0] + 0.587 * px[..., 1] + 0.114 * px[..., 2]
        # blur a little so the mask follows shapes, not scratches
        k = 5
        pad = np.pad(lum, k, mode='edge')
        blur = sum(pad[k + dy:k + dy + h, k + dx:k + dx + w] for dy in (-k, 0, k) for dx in (-k, 0, k)) / 9.0
        p25, p50 = np.percentile(blur, 25), np.percentile(blur, 50)
        sec = 1.0 - smooth(blur, p25 - 0.03, p25 + 0.03)     # darker quarter -> secondary
        out = np.zeros((h, w, 4), np.float32)
        out[..., 0] = 1.0 - sec; out[..., 1] = sec; out[..., 3] = 1.0
        save(out, w, h, src + '_cc')
        L('  %s p25=%.3f p50=%.3f secondary share=%.2f' % (src, p25, p50, float(sec.mean())))
    # greyscale copy of the gold honor armour base (keep alpha = specular)
    px, w, h = load('ch_elite_honor_armor_01')
    lum = 0.299 * px[..., 0] + 0.587 * px[..., 1] + 0.114 * px[..., 2]
    g = px.copy(); g[..., 0] = g[..., 1] = g[..., 2] = lum
    save(g, w, h, 'ch_elite_honor_armor_01_grey')
    # plain primary mask for the glowing pieces
    out = np.zeros((64, 64, 4), np.float32); out[..., 0] = 1; out[..., 3] = 1
    save(out, 64, 64, 'cc_full_primary')
except Exception:
    L('FATAL\n' + traceback.format_exc())
L('DONE')
