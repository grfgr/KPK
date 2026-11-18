# copy a baked model into the mod: kpk_<name>_baked (+_photo), rvmat, remove the old tiling materials
# usage: integrate.py <name>
import shutil, os, sys
import numpy as np
from PIL import Image, ImageFilter
name = sys.argv[1]
R = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../../../KPK/data/"); B = os.path.join(os.path.dirname(os.path.abspath(__file__)), "v3_%s/" % name)
col = Image.open(B + "baked_color.png").convert("RGB"); col.save(R + "kpk_%s_baked_co.png" % name)
edge = np.asarray(Image.open(B + "baked_edge.png").convert("L")).astype(np.float32)
lum = np.asarray(col.convert("L")).astype(np.float32)
Image.fromarray(np.dstack([np.full_like(lum, 255), np.clip(25 + edge * 0.55 + lum * 0.15, 0, 230), np.clip(55 + edge * 0.4, 0, 200)]).astype(np.uint8)).save(R + "kpk_%s_baked_smdi.png" % name)
L = np.asarray(col.convert("L").filter(ImageFilter.GaussianBlur(0.8))).astype(np.float32) / 255
gx = (np.roll(L, -1, 1) - np.roll(L, 1, 1)) * 0.5; gy = (np.roll(L, -1, 0) - np.roll(L, 1, 0)) * 0.5
n = np.dstack([-gx * 2.0, gy * 2.0, np.ones_like(L)]); n /= np.linalg.norm(n, axis=2, keepdims=True); n = (n * 0.5 + 0.5) * 255; n[..., 1] = 255 - n[..., 1]
Image.fromarray(n.astype(np.uint8)).save(R + "kpk_%s_baked_nohq.png" % name)
for suf in ["_co", "_nohq", "_smdi"]:
    shutil.copy(B + "photo" + suf + ".png", R + "kpk_%s_photo%s.png" % (name, suf))
for old in ["body", "trim", "cloth"]:
    for suf in ["_co.png", "_nohq.png", "_smdi.png", ".rvmat"]:
        f = R + "kpk_%s_%s%s" % (name, old, suf)
        if os.path.exists(f):
            os.remove(f)
tpl = open(R + "kpk_m1_baked.rvmat").read()
open(R + "kpk_%s_baked.rvmat" % name, "w").write(tpl.replace("kpk_m1_baked", "kpk_%s_baked" % name))
if not os.path.exists(R + "kpk_%s_photo.rvmat" % name):
    open(R + "kpk_%s_photo.rvmat" % name, "w").write(open(R + "kpk_m1_photo.rvmat").read().replace("kpk_m1_photo", "kpk_%s_photo" % name))
print("integrated", name)
