from PIL import Image, ImageDraw, ImageFont
import os, glob
shots = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11\armor-fx-pri-shots"
out = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11\armor-fx-pri-contact-sheet.png"
files = sorted(glob.glob(os.path.join(shots, "armor-fx-pri-??-*.png")))
cols, rows = 6, 5
tw, th = 312, 246
pad = 4
sheet = Image.new("RGB", (cols*tw + (cols+1)*pad, rows*tth + (rows+1)*pad) if False else (cols*tw + (cols+1)*pad, rows*(th+18) + (rows+1)*pad), (18,20,24))
draw = ImageDraw.Draw(sheet)
for i, p in enumerate(files[:30]):
    img = Image.open(p).convert("RGB")
    crop = img.crop((180, 80, 740, 600))
    scale = max(tw/crop.width, th/crop.height)
    r = crop.resize((int(crop.width*scale+0.5), int(crop.height*scale+0.5)), Image.LANCZOS)
    left = (r.width-tw)//2; top=(r.height-th)//2
    r = r.crop((left,top,left+tw,top+th))
    col = i % cols; row = i // cols
    x = pad + col*(tw+pad); y = pad + row*(th+18+pad)
    sheet.paste(r, (x,y))
    name = os.path.basename(p).replace("armor-fx-pri-","").replace(".png","")
    draw.text((x+4,y+th+2), name, fill=(220,220,220))
sheet.save(out)
print(out, sheet.size, os.path.getsize(out))
