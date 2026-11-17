# Decals for the KPK-M1 (landscape, 178 x 98 mm, 10 px / mm): front, back and the screen.
import math, random, sys
from PIL import Image, ImageDraw, ImageFont, ImageFilter
OUT = sys.argv[1]
F = "/usr/share/fonts/truetype/dejavu/"
def font(n, s): return ImageFont.truetype(F + n, s)
random.seed(11)
W, H = 1780, 980
def mm(x, z): return (int((x + 89) * 10), int((49 - z) * 10))

def radiation(draw, cx, cy, r, col, bg=None):
    if bg:
        draw.ellipse([cx - r * 1.15, cy - r * 1.15, cx + r * 1.15, cy + r * 1.15], fill=bg)
    for k in range(3):
        a0 = -90 + k * 120 - 30
        draw.pieslice([cx - r, cy - r, cx + r, cy + r], a0, a0 + 60, fill=col)
    if bg:
        draw.ellipse([cx - r * 0.3, cy - r * 0.3, cx + r * 0.3, cy + r * 0.3], fill=bg)
    draw.ellipse([cx - r * 0.18, cy - r * 0.18, cx + r * 0.18, cy + r * 0.18], fill=col)

def wear(img, amount, seed):
    """punch random holes into the alpha so the print looks worn"""
    rnd = random.Random(seed)
    a = img.getchannel("A")
    d = ImageDraw.Draw(a)
    w, h = img.size
    for _ in range(amount):
        x, y = rnd.randrange(w), rnd.randrange(h)
        r = rnd.choice([1, 1, 2, 3])
        d.ellipse([x - r, y - r, x + r, y + r], fill=0)
    img.putalpha(a)
    return img

def plant(draw, x0, base, scale, col):
    """power plant silhouette with a vent stack"""
    s = scale
    draw.rectangle([x0, base - 40 * s, x0 + 260 * s, base], fill=col)
    draw.rectangle([x0 + 40 * s, base - 75 * s, x0 + 150 * s, base - 40 * s], fill=col)
    draw.rectangle([x0 + 170 * s, base - 60 * s, x0 + 230 * s, base - 40 * s], fill=col)
    # stack
    draw.polygon([(x0 + 95 * s, base - 75 * s), (x0 + 108 * s, base - 160 * s), (x0 + 118 * s, base - 160 * s), (x0 + 125 * s, base - 75 * s)], fill=col)
    for i in range(4):
        y = base - 85 * s - i * 18 * s
        draw.line([x0 + 100 * s, y, x0 + 122 * s, y - 10 * s], fill=col, width=max(1, int(2 * s)))
    # pylons
    for px in (x0 - 60 * s, x0 + 300 * s):
        draw.line([px, base, px + 15 * s, base - 110 * s, px + 30 * s, base], fill=col, width=max(1, int(3 * s)))
        draw.line([px + 5 * s, base - 70 * s, px + 25 * s, base - 70 * s], fill=col, width=max(1, int(2 * s)))

# ---------------------------------------------------------------- screen (113 x 64 mm)
SW, SH = 1695, 960
scr = Image.new("RGB", (SW, SH), (18, 16, 12))
d = ImageDraw.Draw(scr)
sep = (190, 172, 130)
d.rectangle([40, 40, SW - 40, SH - 40], outline=(90, 80, 60), width=3)
d.line([40, 40, 120, 40], fill=sep, width=5)
radiation(d, SW // 2, 300, 170, (200, 182, 140), bg=(30, 27, 20))
plant(d, SW // 2 - 330, 560, 2.2, (12, 11, 8))
f = font("DejaVuSans-Bold.ttf", 58)
for i, t in enumerate(["КАРМАННЫЙ ПЕРСОНАЛЬНЫЙ", "КОМПЬЮТЕР", "МОДЕЛЬ 1"]):
    w = d.textlength(t, font=f)
    d.text(((SW - w) / 2, 620 + i * 80), t, font=f, fill=(205, 186, 140))
# grain, burn-in, scanlines
px = scr.load()
for y in range(SH):
    k = 0.82 if y % 4 == 0 else 1.0
    for x in range(0, SW, 1):
        r, g, b = px[x, y]
        n = random.randint(-10, 10)
        px[x, y] = (max(0, min(255, int(r * k) + n)), max(0, min(255, int(g * k) + n)), max(0, min(255, int(b * k) + n)))
scr = scr.filter(ImageFilter.GaussianBlur(1.0))
scr.save(OUT + "/screen.png")

# ---------------------------------------------------------------- front
fr = Image.new("RGBA", (W, H), (0, 0, 0, 0))
d = ImageDraw.Draw(fr)
paint = (196, 186, 160, 235)
# stencil model name, top right
d.text(mm(58, 45.2), "КПК-М1", font=font("DejaVuSans-Bold.ttf", 44), fill=paint)
# tower icon, left
lay = Image.new("RGBA", (W, H), (0, 0, 0, 0))
ld = ImageDraw.Draw(lay)
tx, ty = mm(-80, 6)
ld.line([tx, ty + 120, tx + 30, ty, tx + 60, ty + 120], fill=paint, width=5)
for k in range(4):
    yy = ty + 25 + k * 25
    ld.line([tx + 30 - (yy - ty) / 4, yy, tx + 30 + (yy - ty) / 4, yy], fill=paint, width=3)
ld.line([tx + 70, ty + 120, tx + 70, ty + 80, tx + 110, ty + 80, tx + 110, ty + 120], fill=paint, width=4)
fr.alpha_composite(wear(lay, 1500, 3))
# radiation sticker, bottom right
st = Image.new("RGBA", (124, 124), (0, 0, 0, 0))
sd = ImageDraw.Draw(st)
radiation(sd, 62, 62, 48, (30, 26, 18, 255), bg=(214, 168, 40, 255))
fr.alpha_composite(wear(st, 400, 4), mm(62, -31))
# button symbols
d.text(mm(64.5, 34.5), "C", font=font("DejaVuSans-Bold.ttf", 66), fill=(200, 40, 30, 255))
d.text(mm(64, -19), "✆" if False else "", font=font("DejaVuSans-Bold.ttf", 60), fill=(60, 170, 70, 255))
# green phone handset drawn as an arc
cx, cy = mm(69, -16)
d.arc([cx - 30, cy - 30, cx + 30, cy + 30], 110, 250, fill=(60, 170, 70, 255), width=12)
# arrows on the d-pad
for (dx, dz, pts) in [(0, 9, [(0, -14), (-12, 6), (12, 6)]), (0, -9, [(0, 14), (-12, -6), (12, -6)]),
                      (-9, 0, [(-14, 0), (6, -12), (6, 12)]), (9, 0, [(14, 0), (-6, -12), (-6, 12)])]:
    bx, by = mm(73 + dx, 5 + dz)
    d.polygon([(bx + p[0], by + p[1]) for p in pts], fill=(150, 145, 130, 230))
# serial sticker top left, crooked paper
tag = Image.new("RGBA", (290, 90), (200, 190, 160, 255))
td = ImageDraw.Draw(tag)
td.text((18, 14), "НИГ-0473", font=font("DejaVuSansMono-Bold.ttf", 54), fill=(40, 34, 26, 255))
for _ in range(1400):
    x, y = random.randrange(290), random.randrange(90)
    td.point((x, y), fill=(110, 90, 60, 255) if random.random() < 0.75 else (225, 215, 190, 255))
tag = wear(tag, 300, 5).rotate(9, expand=True, resample=Image.BICUBIC)
fr.alpha_composite(tag, mm(-66, 47))
fr.save(OUT + "/front.png")

# ---------------------------------------------------------------- back
# drawn as seen from behind (v = -model x), then flipped so it lands right in the front projection
def mv(v, z): return (int((v + 89) * 10), int((49 - z) * 10))
bk = Image.new("RGBA", (W, H), (0, 0, 0, 0))
lay = Image.new("RGBA", (W, H), (0, 0, 0, 0))
d = ImageDraw.Draw(lay)
radiation(d, *mv(64, 6), 130, (205, 160, 40, 230))
bk.alpha_composite(wear(lay, 5000, 6))
d = ImageDraw.Draw(bk)
pc = (205, 196, 170, 240)
d.text(mv(1.5, -0.5), "КПК-М1", font=font("DejaVuSans-Bold.ttf", 50), fill=pc)
d.text(mv(1.5, -7.5), "№ 0473-11", font=font("DejaVuSansMono-Bold.ttf", 38), fill=pc)
bx, by = mv(23, -7.5)
for i in range(30):
    w = random.choice([2, 3, 4])
    if random.random() < 0.65:
        d.rectangle([bx + i * 6, by, bx + i * 6 + w, by + 55], fill=pc)
d.text(mv(-82, -40), "ОБРАЗЕЦ 1986", font=font("DejaVuSansMono.ttf", 34), fill=(170, 165, 145, 160))
bk = bk.transpose(Image.FLIP_LEFT_RIGHT)
bk.save(OUT + "/back.png")
print("decals ok")
