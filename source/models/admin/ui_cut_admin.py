# interface frame of the admin KPK: the front render with the screen window cut out (soft 1 px edge)
# usage: ui_cut_admin.py <out dir>  -> KPK/data/kpk_ui_admin_ca.png
import sys, os, json
from PIL import Image, ImageDraw, ImageFilter
o = sys.argv[1]
im = Image.open(o + "/ui_front.png").convert("RGBA"); j = json.load(open(o + "/ui.json"))
W, H = im.size; u, v, w, h = j["screen"]
hole = Image.new("L", (W, H), 0)
ImageDraw.Draw(hole).rounded_rectangle([u * W, v * H, (u + w) * W, (v + h) * H], radius=6, fill=255)
hole = hole.filter(ImageFilter.GaussianBlur(0.8))
a = im.split()[3]
a = Image.composite(Image.new("L", (W, H), 0), a, hole)
im.putalpha(a)
R = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../../../KPK/data/")
im.save(R + "kpk_ui_admin_ca.png")
print("screen", [round(x, 5) for x in j["screen"]])
