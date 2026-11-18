# Cuts the screen window out of the front render (alpha 0, chamfered / rounded like the model) and fades
# the brick out below the keys; writes KPK/data/kpk_ui_<name>_ca.png. usage: ui_cut.py <scratch ui dir>
import json, sys, os
import numpy as np
from PIL import Image, ImageDraw, ImageFilter
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from cfg_v4 import CFG
U = sys.argv[1]; R = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../../../KPK/data/")
out = {}
for n in ["m1", "mil", "nii", "mono", "brick"]:
    im = Image.open(U + "/%s.png" % n).convert("RGBA"); j = json.load(open(U + "/%s.json" % n)); c = CFG[n]
    W, H = im.size; u, v, w, h = j["screen"]
    sw_mm = (c["screen"][2] - c["screen"][0]) * c["W"] / (c["body"][2] - c["body"][0])
    pxmm = w * W / sw_mm
    x0, y0, x1, y1 = u * W, v * H, (u + w) * W, (v + h) * H
    hole = Image.new("L", (W * 2, H * 2), 0); d = ImageDraw.Draw(hole)
    box = [x0 * 2, y0 * 2, x1 * 2, y1 * 2]
    if c.get("screen_kind") == "chamfer":
        k = 5.5 * pxmm * 2
        d.polygon([(box[0] + k, box[1]), (box[2] - k, box[1]), (box[2], box[1] + k), (box[2], box[3] - k), (box[2] - k, box[3]), (box[0] + k, box[3]), (box[0], box[3] - k), (box[0], box[1] + k)], fill=255)
    else:
        d.rounded_rectangle(box, radius=2.5 * pxmm * 2, fill=255)
    hole = hole.resize((W, H), Image.LANCZOS)
    a = np.asarray(im).astype(np.float32)
    alpha = a[..., 3] * (1 - np.asarray(hole).astype(np.float32) / 255)
    if "fade" in j:
        # in game the brick picture is 1.82 screen heights tall and runs off the bottom: fade out
        # just above the bottom edge of the screen (around the first key row)
        top_ = 0.06 - 0.13158 * 1.8
        f0, f1 = (0.9 - top_) / 1.8 * H, (1.0 - top_) / 1.8 * H
        yy = np.arange(H, dtype=np.float32)[:, None]
        alpha = alpha * np.clip((f1 - yy) / (f1 - f0), 0, 1)
    a[..., 3] = alpha
    Image.fromarray(np.clip(a, 0, 255).astype(np.uint8)).save(R + "kpk_ui_%s_ca.png" % n)
    out[n] = dict(aspect=j["aspect"], screen=j["screen"])
    print(n, "screen aspect", round(w * W / (h * H), 3))
json.dump(out, open(U + "/ui_all.json", "w"), indent=1)
