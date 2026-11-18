# Step 1 (plain python): cut the views out of the concept sheet into a 2048 atlas (colour, normal, smdi)
# usage: prep.py <sheet.png> <name> <out dir>
import json, sys
import numpy as np
from PIL import Image, ImageFilter
from cfg import CFG

sheet, name, out = sys.argv[1], sys.argv[2], sys.argv[3]
c = CFG[name]
img = Image.open(sheet).convert("RGB")
if "patch" in c:
    # cover loose bits painted over the case (the Monolith rosary) with the mirrored opposite corner
    src, dst = c["patch"]
    img.paste(img.crop(src).transpose(Image.FLIP_TOP_BOTTOM), dst)
S = 2048
atlas = Image.new("RGB", (S, S), (30, 30, 28))
regions = {}


def place(key, im, box):
    bx, by, bw, bh = box
    atlas.paste(im.resize((bw, bh), Image.LANCZOS), (bx, by))
    regions[key] = [bx / S, by / S, bw / S, bh / S]


A = np.asarray(img).astype(np.int32)


def tight(box):
    # trim the background around a small view: bbox of pixels that differ from the crop border
    x0, y0, x1, y1 = box
    sub = A[y0:y1, x0:x1]
    border = np.concatenate([sub[0], sub[-1], sub[:, 0], sub[:, -1]])
    m = np.abs(sub - np.median(border, axis=0)).sum(axis=2) > 45
    rows = np.nonzero(m.sum(axis=1) > m.shape[1] * 0.15)[0]; cols = np.nonzero(m.sum(axis=0) > m.shape[0] * 0.15)[0]
    if len(rows) < 4 or len(cols) < 4:
        return box
    return (x0 + cols[0], y0 + rows[0], x0 + cols[-1] + 1, y0 + rows[-1] + 1)


for k in ("left", "right", "top", "bottom"):
    c[k] = tight(c[k])
    print(name, k, c[k])
c["back"] = c["backbody"]
front = img.crop(c["front"]); back = img.crop(c["back"]); top = img.crop(c["top"]); bottom = img.crop(c["bottom"])
left = img.crop(c["left"]).transpose(Image.ROTATE_90); right = img.crop(c["right"]).transpose(Image.ROTATE_90)
def hgt(im, w):
    return int(w * im.height / im.width)
FW = S
while True:
    fh = hgt(front, FW); y = fh + 8
    rows = max(hgt(back, 1024), hgt(top, 1016) + hgt(bottom, 1016) + 8)
    if y + rows + 8 + max(hgt(left, 1016), hgt(right, 1016)) <= S:
        break
    FW -= 64
place("front", front, (0, 0, FW, fh))
place("back", back, (0, y, 1024, hgt(back, 1024)))
place("top", top, (1032, y, 1016, hgt(top, 1016)))
place("bottom", bottom, (1032, y + hgt(top, 1016) + 8, 1016, hgt(bottom, 1016)))
yy = y + rows + 8
place("left", left, (0, yy, 1016, hgt(left, 1016)))
place("right", right, (1032, yy, 1016, hgt(right, 1016)))
atlas.save(out + "/atlas_co.png")

# normal map from luminance (OpenGL green; flipped to DirectX when copied into the mod)
L = np.asarray(atlas.convert("L").filter(ImageFilter.GaussianBlur(1.0))).astype(np.float32) / 255.0
gy, gx = np.gradient(L)
n = np.dstack([-gx * 7.0, gy * 7.0, np.ones_like(L)])
n /= np.linalg.norm(n, axis=2, keepdims=True)
Image.fromarray(((n * 0.5 + 0.5) * 255).astype(np.uint8)).save(out + "/atlas_normal.png")
Lr = np.asarray(atlas.convert("L")).astype(np.float32)
smdi = np.dstack([np.full_like(Lr, 255), np.clip(Lr * 0.45, 10, 140), np.clip(60 + Lr * 0.35, 60, 150)]).astype(np.uint8)
Image.fromarray(smdi).save(out + "/atlas_smdi.png")
json.dump(dict(name=name, regions=regions, boxes={k: list(map(int, c[k])) for k in ("left", "right", "top", "bottom", "back")}), open(out + "/prep.json", "w"))
print(name, "atlas ok, front width", FW)
