from PIL import Image, ImageFilter
import numpy as np, os, colorsys, json

BUILD = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\ui\reach\armory\armor_effects"
DIST  = r"D:\Project-Noble-Closed-Source\dist\Project-Noble\data\ui\reach\armory\armor_effects"
OUT   = r"D:\Project-Noble-Closed-Source\docs\evidence\armor-fx-primary-2026-11-17\helmet-thumbs"
os.makedirs(OUT, exist_ok=True)
os.makedirs(DIST, exist_ok=True)

# Primary palette, same order/values as src/common/ArmorEffects.hpp kPrimaryFlames.
COLORS = [
 ("steel",(0.349,0.349,0.349)),("silver",(0.608,0.608,0.608)),("white",(0.847,0.847,0.847)),
 ("brown",(0.427,0.310,0.220)),("tan",(0.620,0.494,0.376)),("khaki",(0.780,0.671,0.541)),
 ("sage",(0.345,0.408,0.302)),("olive",(0.490,0.569,0.400)),("drab",(0.643,0.725,0.533)),
 ("forest",(0.165,0.369,0.212)),("green",(0.267,0.545,0.314)),("sea_foam",(0.467,0.729,0.506)),
 ("teal",(0.165,0.431,0.459)),("aqua",(0.278,0.608,0.635)),("cyan",(0.471,0.788,0.808)),
 ("blue",(0.176,0.302,0.514)),("cobalt",(0.290,0.455,0.694)),("ice",(0.518,0.678,0.875)),
 ("violet",(0.310,0.227,0.498)),("orchid",(0.459,0.365,0.678)),("lavender",(0.651,0.565,0.855)),
 ("maroon",(0.467,0.161,0.200)),("brick",(0.678,0.251,0.278)),("rose",(0.863,0.478,0.494)),
 ("rust",(0.557,0.231,0.133)),("coral",(0.784,0.376,0.239)),("peach",(0.941,0.588,0.451)),
 ("gold",(0.639,0.439,0.141)),("yellow",(0.859,0.651,0.220)),("pale",(0.961,0.816,0.447)),
]

base = Image.open(os.path.join(BUILD,"legendary.png")).convert("RGBA")
eternal = Image.open(os.path.join(BUILD,"eternal.png")).convert("RGBA").resize(base.size, Image.LANCZOS)
B = np.asarray(base).astype(np.float32)/255.0
E = np.asarray(eternal).astype(np.float32)/255.0
alpha = B[...,3]
# Flame aura = pixels that differ between the orange and blue templates, keep the helmet interior out.
diff = (np.abs(B[...,:3]-E[...,:3]).max(axis=2) > 0.08) & (alpha > 0.08)
mask_img = Image.fromarray((diff*255).astype(np.uint8)).filter(ImageFilter.MinFilter(5))  # erode 2px
mask = np.asarray(mask_img) > 127
# The helmet/visor is identical in both templates (diff false), so the difference mask recolours every
# coloured flame pixel without touching the helmet. Erosion avoids the antialiased helmet/flame boundary.
mask = diff & (alpha > 0.08)

def tinted(color):
    r,g,b = color
    h,s,v = colorsys.rgb_to_hsv(r,g,b)
    out = B.copy()
    rgb = out[...,:3]
    mx = rgb.max(axis=2); mn = rgb.min(axis=2)
    base_v = mx
    base_s = np.where(mx>1e-4, (mx-mn)/np.maximum(mx,1e-4), 0)
    # Keep the flame's luminance shading, swap hue/saturation toward the primary colour.
    target_s = max(s, 0.05)
    new_s = np.clip(target_s * (0.55 + 0.55*base_s), 0, 1)
    # White-hot core: keep it white/very bright so the flame still reads as fire.
    core = np.clip((base_v-0.78)/0.22,0,1) * np.clip(1.0-base_s*2.2,0,1)
    new_s = np.clip(new_s*(1.0-core), 0, 1)
    new_v = np.clip(base_v*1.05+0.02,0,1)
    # HSV -> RGB vectorized
    i = np.floor(h*6.0).astype(np.int32)
    f = h*6.0 - i
    p = new_v*(1-new_s); q = new_v*(1-f*new_s); t = new_v*(1-(1-f)*new_s)
    rr = np.select([i%6==0,i%6==1,i%6==2,i%6==3,i%6==4], [new_v,q,p,p,t], default=new_v)
    gg = np.select([i%6==0,i%6==1,i%6==2,i%6==3,i%6==4], [t,new_v,new_v,q,p], default=q)
    bb = np.select([i%6==0,i%6==1,i%6==2,i%6==3,i%6==4], [p,t,q,new_v,new_v], default=p)
    flame = np.stack([rr,gg,bb],axis=2)
    out[...,:3] = np.where(mask[...,None], flame, rgb)
    return Image.fromarray((np.clip(out,0,1)*255).astype(np.uint8), "RGBA")

thumbs=[]
for name,color in COLORS:
    im = tinted(color)
    path = os.path.join(BUILD, f"flame_pri_{name}.png")
    im.save(path)
    os.makedirs(DIST, exist_ok=True)
    im.save(os.path.join(DIST, f"flame_pri_{name}.png"))
    im.save(os.path.join(OUT, f"flame_pri_{name}.png"))
    thumbs.append((name, im.copy()))

# Contact sheet on a dark blue background (same feel as the Armory tiles)
cw,ch=312,246; cols=6; rows=5
sheet=Image.new("RGB",(cw*cols,ch*rows),(18,26,42))
for idx,(name,im) in enumerate(thumbs):
    x=(idx%cols)*cw; y=(idx//cols)*ch
    sheet.paste(im,(x,y),im)
    from PIL import ImageDraw
    d=ImageDraw.Draw(sheet); d.text((x+8,y+8), name.upper(), fill=(235,235,235))
sheet.save(os.path.join(OUT,"helmet-thumbs-contact.png"))
print("made",len(thumbs),"thumbnails; mask px",int(mask.sum()))
