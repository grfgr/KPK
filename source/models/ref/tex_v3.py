# v3 textures: seamless procedural materials (colour from the sheet) and a photo atlas (screen, back,
# button tops, plates, decals).  usage: tex_v3.py <sheet.png> <name> <out dir>
import json, sys, random
import numpy as np
from PIL import Image, ImageDraw, ImageFilter
from cfg_v3 import CFG, SHARED

sheet, name, out = sys.argv[1], sys.argv[2], sys.argv[3]
c = CFG[name]
img = Image.open(sheet).convert("RGB")
if "patch" in c:
    src, dst = c["patch"]
    img.paste(img.crop(src).transpose(Image.FLIP_TOP_BOTTOM), dst)
A = np.asarray(img).astype(np.float32)
rng = np.random.default_rng(hash(name) % 2 ** 31)
random.seed(hash(name) % 1000)


def tnoise(size, cells):
    # seamless value noise: random cells tiled 3x3, smooth-resized, centre cut out
    r = rng.random((cells, cells)).astype(np.float32)
    big = Image.fromarray((np.tile(r, (3, 3)) * 255).astype(np.uint8)).resize((size * 3, size * 3), Image.BICUBIC)
    return np.asarray(big).astype(np.float32)[size:2 * size, size:2 * size] / 255.0


def fbm(size, base=4, octaves=5):
    v = np.zeros((size, size), np.float32); amp = 1.0; tot = 0
    for o in range(octaves):
        v += tnoise(size, base * 2 ** o) * amp; tot += amp; amp *= 0.5
    return v / tot


def scratches(size, count, length, width=1):
    im = Image.new("L", (size * 3, size * 3), 0); d = ImageDraw.Draw(im)
    for _ in range(count):
        x, y = random.uniform(0, size), random.uniform(0, size)
        a = random.uniform(0, 3.1416); l = random.uniform(length * 0.3, length)
        dx, dy = np.cos(a) * l, np.sin(a) * l
        val = random.randint(90, 255)
        for ox in (0, size, 2 * size):
            for oy in (0, size, 2 * size):
                d.line([(x + ox, y + oy), (x + ox + dx, y + oy + dy)], fill=val, width=width)
    return np.asarray(im).astype(np.float32)[size:2 * size, size:2 * size] / 255.0


def normal_from(h, k):
    gx = (np.roll(h, -1, 1) - np.roll(h, 1, 1)) * 0.5; gy = (np.roll(h, -1, 0) - np.roll(h, 1, 0)) * 0.5
    n = np.dstack([-gx * k, gy * k, np.ones_like(h)]); n /= np.linalg.norm(n, axis=2, keepdims=True)
    n = (n * 0.5 + 0.5) * 255
    n[..., 1] = 255 - n[..., 1]          # DirectX green, as DayZ wants
    return Image.fromarray(n.astype(np.uint8))


def sample(box):
    x0, y0, x1, y1 = box
    return np.median(A[y0:y1, x0:x1].reshape(-1, 3), axis=0)


def material(key, m, size=1024):
    col = sample(m["box"]) if "box" in m else np.array(m["color"], np.float32)
    kind = m["kind"]
    f = fbm(size, 4); fine = fbm(size, 64, 3)
    if kind == "cloth" or kind == "weave":
        yy, xx = np.mgrid[0:size, 0:size].astype(np.float32)
        p = 2 * np.pi * 24 / size
        warp = (np.sin(xx * p) * 0.5 + 0.5) * (np.sin(yy * p * 0.5) > 0)
        weft = (np.sin(yy * p) * 0.5 + 0.5) * (np.sin(xx * p * 0.5) <= 0)
        h = np.maximum(warp, weft) * 0.7 + fine * 0.3
        shade = 0.72 + 0.4 * h + (f - 0.5) * 0.35
        rgb = col[None, None, :] * shade[..., None]
        spec, gloss = 12, 30
    else:
        sc = scratches(size, int(260 * m.get("scratch", 0.5)), size * 0.12) if kind != "rubber" else np.zeros((size, size))
        rust = np.clip((fbm(size, 6) - 0.62) * 4, 0, 1) * m.get("rust", 0)
        grime = np.clip((f - 0.45) * 1.6, 0, 1)
        shade = 0.88 + (fine - 0.5) * 0.18 + (f - 0.5) * 0.22 - grime * 0.12
        rgb = col[None, None, :] * shade[..., None]
        rgb = rgb * (1 - rust[..., None]) + np.array([95, 58, 34])[None, None, :] * rust[..., None]
        bright = {"metal": 85, "paint": 60, "rubber": 15}[kind]
        rgb = rgb + sc[..., None] * bright
        h = fine * 0.6 + f * 0.3 - sc * 0.5 + rust * 0.3
        spec = {"metal": 70, "paint": 40, "rubber": 10}[kind]
        gloss = {"metal": 110, "paint": 80, "rubber": 35}[kind]
    rgb = np.clip(rgb, 0, 255).astype(np.uint8)
    Image.fromarray(rgb).save(f"{out}/{key}_co.png")
    normal_from(h, 3.0 if kind != "rubber" else 1.5).save(f"{out}/{key}_nohq.png")
    lum = np.asarray(Image.fromarray(rgb).convert("L")).astype(np.float32)
    smdi = np.dstack([np.full_like(lum, 255), np.clip(spec + (lum - lum.mean()) * 0.4, 5, 200), np.full_like(lum, gloss)])
    Image.fromarray(smdi.astype(np.uint8)).save(f"{out}/{key}_smdi.png")
    print(key, kind, "colour", col.astype(int))


for key, m in c["mats"].items():
    material(key, m)
if len(sys.argv) > 4:
    for key, m in SHARED.items():
        material(key, m)

# ---------------- photo atlas: shelf packing of crops
crops = {"screen": img.crop(c["screen"]), "back": img.crop(c["backbody"])}
fx0, fy0 = c["front"][0], c["front"][1]
for i, pt in enumerate(c["parts"]):
    if pt["t"] in ("btn", "plate", "led"):
        r = pt.get("r")
        if pt["t"] == "led":
            cx, cy = pt["c"]; rr = pt["rad"]; r = (cx - rr, cy - rr, cx + rr, cy + rr)
        crops["p%d" % i] = img.crop(r)
for i, r in enumerate(c["decals"]):
    crops["d%d" % i] = img.crop(r)
if c.get("cloth"):
    crops["emblem"] = img.crop(c["cloth"]["emblem"])
S = 2048
atlas = Image.new("RGB", (S, S), (40, 40, 38))
regions = {}
order = sorted(crops, key=lambda k: -crops[k].height * crops[k].width)
x = y = shelf = 0
for k in order:
    im = crops[k]
    scale = {"screen": 1500 / im.width, "back": 1100 / im.width}.get(k, min(4.0, 300 / max(im.width, im.height)))
    w, h = max(8, int(im.width * scale)), max(8, int(im.height * scale))
    if x + w > S:
        x = 0; y += shelf + 6; shelf = 0
    atlas.paste(im.resize((w, h), Image.LANCZOS), (x, y))
    regions[k] = [x / S, y / S, w / S, h / S]
    x += w + 6; shelf = max(shelf, h)
assert y + shelf <= S, "photo atlas overflow"
atlas.save(out + "/photo_co.png")
L = np.asarray(atlas.convert("L").filter(ImageFilter.GaussianBlur(1.0))).astype(np.float32) / 255
normal_from(L, 4.0).save(out + "/photo_nohq.png")
lum = np.asarray(atlas.convert("L")).astype(np.float32)
Image.fromarray(np.dstack([np.full_like(lum, 255), np.clip(lum * 0.4, 10, 120), np.full_like(lum, 90)]).astype(np.uint8)).save(out + "/photo_smdi.png")
json.dump(dict(regions=regions), open(out + "/v3.json", "w"))
print("photo atlas ok", len(regions))
