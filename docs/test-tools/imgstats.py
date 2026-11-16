# Alpha / channel stats of source textures (Blender bpy, background). DIRS = "dir|dir" (absolute).
import bpy, os, glob, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []
try:
    import numpy as np
    for d in os.environ['DIRS'].split('|'):
        for p in sorted(glob.glob(os.path.join(d, '*.tif')) + glob.glob(os.path.join(d, '*.tga')) + glob.glob(os.path.join(d, '*.png')) + glob.glob(os.path.join(d, '*.tiff'))):
            n = os.path.basename(p).lower()
            if any(k in n for k in ('_nm', 'zbump', 'bump', '_cc', 'mask')): continue
            try:
                im = bpy.data.images.load(p, check_existing=False)
                w, h = im.size; ch = im.channels
                px = np.empty(w * h * 4, dtype=np.float32); im.pixels.foreach_get(px); px = px.reshape(-1, 4)
                a = px[:, 3]; rgb = px[:, :3].mean(axis=1)
                out.append(f"{n:48} {w}x{h} ch{ch} alpha mean {a.mean():.2f} min {a.min():.2f} max {a.max():.2f} | rgb mean {rgb.mean():.2f}")
                bpy.data.images.remove(im)
            except Exception as e: out.append(f"{n}: {e}")
except Exception:
    out.append("ERROR\n" + traceback.format_exc())
open(os.path.join(T, os.environ.get('OUT', 'imgstats.txt')), 'w').write("\n".join(out))
