from PIL import Image
import os, glob, shutil
shots = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11\armor-fx-pri-shots"
dst = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\ui\reach\armory\armor_effects"
dist = r"D:\Project-Noble-Closed-Source\dist\Project-Noble\data\ui\reach\armory\armor_effects"
os.makedirs(dst, exist_ok=True)
os.makedirs(dist, exist_ok=True)
made = 0
for p in sorted(glob.glob(os.path.join(shots, "armor-fx-pri-*-*.png"))):
    name = os.path.basename(p)
    if name.startswith("thumb_test"): continue
    # armor-fx-pri-01-steel.png -> id steel
    parts = name[len("armor-fx-pri-"):-len(".png")].split("-", 1)
    if len(parts) != 2: continue
    vid = parts[1]
    img = Image.open(p).convert("RGB")
    crop = img.crop((180, 80, 740, 600))
    tw, th = 312, 246
    scale = max(tw / crop.width, th / crop.height)
    resized = crop.resize((int(crop.width * scale + 0.5), int(crop.height * scale + 0.5)), Image.LANCZOS)
    left = (resized.width - tw) // 2
    top = (resized.height - th) // 2
    resized = resized.crop((left, top, left + tw, top + th))
    target = os.path.join(dst, f"flame_pri_{vid}.png")
    resized.save(target)
    shutil.copy2(target, os.path.join(dist, f"flame_pri_{vid}.png"))
    made += 1
print("thumbnails made", made)
