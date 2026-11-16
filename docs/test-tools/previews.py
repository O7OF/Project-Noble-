import bpy, os, glob
T = r'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11'
D = r'D:\Steam\steamapps\common\HREK\data\objects\characters\elite_noble\bitmaps'
OUT = os.path.join(T, 'prev'); os.makedirs(OUT, exist_ok=True)
files = [os.path.join(D, n + '.tif') for n in ('ch_helmet_councilor_01_diff', 'elite_ossoona_helmet_armor_diffuse_4k', 'mp_newelite_diffuse',
         'elite_cc', 'ch_elite_armor_02', 'ch_elite_honor_armor_01', 'ch_elite_honor_armor_01_classic', 'ch_elite_honor_armor_01_em')]
files += glob.glob(os.path.join(T, 'stockref', '*.tga'))
log = []
for f in files:
    try:
        im = bpy.data.images.load(f)
        w, h = im.size
        im.scale(384, max(1, int(384 * h / w)))
        im.filepath_raw = os.path.join(OUT, os.path.splitext(os.path.basename(f))[0] + '.png')
        im.file_format = 'PNG'; im.save()
        # channel stats: mean of R,G,B,A
        px = list(im.pixels); n = len(px) // 4
        means = [round(sum(px[c::4]) / n, 3) for c in range(4)]
        log.append('%s %dx%d means RGBA %s' % (os.path.basename(f), w, h, means))
    except Exception as e:
        log.append('FAIL %s %s' % (f, e))
open(os.path.join(T, 'prev', 'log.txt'), 'w').write('\n'.join(log))
