# Decals (front, back, screen) for the military, Monolith and NII KPKs and the broken brick.
# usage: python decals5.py <model> <out_dir>    model = mil | mono | nii | broken
import math, random, sys, os
from PIL import Image, ImageDraw, ImageFont, ImageFilter

MODEL, OUT = sys.argv[1], sys.argv[2]
os.makedirs(OUT, exist_ok=True)
F = "/usr/share/fonts/truetype/dejavu/"
def font(n, s): return ImageFont.truetype(F + n, s)
B, BM, R, RM = "DejaVuSans-Bold.ttf", "DejaVuSansMono-Bold.ttf", "DejaVuSans.ttf", "DejaVuSansMono.ttf"
random.seed(hash(MODEL) & 0xFFFF)

def canvas(w_mm, h_mm, ppm=10):
    return Image.new("RGBA", (int(w_mm * ppm), int(h_mm * ppm)), (0, 0, 0, 0))

def wear(img, amount, seed):
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

def ctext(d, cx, y, t, f, fill):
    w = d.textlength(t, font=f)
    d.text((cx - w / 2, y), t, font=f, fill=fill)

def mast(d, cx, base, h, col, width=4, arcs=True):
    """radio mast emblem with signal arcs"""
    top = base - h
    d.line([cx - h * 0.18, base, cx, top, cx + h * 0.18, base], fill=col, width=width)
    for k in range(1, 5):
        y = top + k * h * 0.2
        hw = h * 0.18 * k * 0.2
        d.line([cx - hw, y, cx + hw, y], fill=col, width=max(1, width - 1))
        d.line([cx - hw, y, cx + hw * 0.6, y + h * 0.18], fill=col, width=max(1, width - 2))
    if arcs:
        for k in (1, 2):
            r = h * (0.35 + 0.22 * k)
            d.arc([cx - r, top + h * 0.15 - r, cx + r, top + h * 0.15 + r], 200, 340, fill=col, width=width)

def scanlines(img, dark=0.8, blur=1.0, noise=8):
    img = img.convert("RGB")
    px = img.load()
    w, h = img.size
    for y in range(h):
        k = dark if y % 4 == 0 else 1.0
        for x in range(w):
            r, g, b = px[x, y]
            n = random.randint(-noise, noise)
            px[x, y] = (max(0, min(255, int(r * k) + n)), max(0, min(255, int(g * k) + n)), max(0, min(255, int(b * k) + n)))
    return img.filter(ImageFilter.GaussianBlur(blur))

def status_icons(d, w, col):
    for i in range(5):
        d.rectangle([40 + i * 16, 70 - i * 9, 50 + i * 16, 80], fill=col)
    d.rectangle([w - 150, 52, w - 70, 82], outline=col, width=4)
    d.rectangle([w - 70, 60, w - 62, 74], fill=col)
    for i in range(4):
        d.rectangle([w - 144 + i * 19, 58, w - 130 + i * 19, 76], fill=col)

def barcode(d, x, y, n, h, col, step=6):
    for i in range(n):
        if random.random() < 0.62:
            d.rectangle([x + i * step, y, x + i * step + random.choice([2, 3, 4]), y + h], fill=col)

def stripes(d, x, y, n, col, w=26, h=60, gap=16):
    for i in range(n):
        x0 = x + i * (w + gap)
        d.polygon([(x0 + h * 0.5, y), (x0 + h * 0.5 + w, y), (x0 + w, y + h), (x0, y + h)], fill=col)

def save(front, back, screen):
    front.save(OUT + "/front.png")
    back.transpose(Image.FLIP_LEFT_RIGHT).save(OUT + "/back.png")
    screen.save(OUT + "/screen.png")
    print(MODEL, "decals ok")

# =====================================================================================================
if MODEL == "mil":
    W, H = 208, 126
    def mm(x, z): return (int((x + W / 2) * 10), int((H / 2 - z) * 10))
    def mv(v, z): return mm(v, z)          # back drawn as seen from behind, flipped on save
    paint = (205, 196, 160, 235)
    # screen 120 x 68 mm
    SW, SH = 1800, 1020
    s = Image.new("RGB", (SW, SH), (20, 30, 28))
    d = ImageDraw.Draw(s)
    sky = Image.new("RGB", (SW, SH), (0, 0, 0))
    for y in range(SH):
        c = int(20 + 40 * (1 - y / SH))
        ImageDraw.Draw(sky).line([0, y, SW, y], fill=(c - 6, c + 6, c))
    s.paste(sky)
    d = ImageDraw.Draw(s)
    col = (190, 205, 190)
    status_icons(d, SW, col)
    mast(d, SW // 2, 345, 175, (200, 210, 195), 6)
    # watchtower, fence, buildings silhouette
    sil = (10, 16, 15)
    d.rectangle([260, 300, 380, 360], fill=sil)
    for lx in (270, 360):
        d.line([lx, 360, lx - 30, 640], fill=sil, width=10)
    d.line([300, 480, 340, 560], fill=sil, width=6)
    d.rectangle([0, 600, SW, SH], fill=sil)
    for x in range(0, SW, 60):
        d.line([x, 640, x, 560], fill=sil, width=5)
    d.line([0, 575, SW, 575], fill=sil, width=3)
    d.rectangle([1150, 420, 1500, 620], fill=sil)
    d.rectangle([1300, 360, 1360, 420], fill=sil)
    f1 = font(B, 70)
    ctext(d, SW // 2, 430, "ОБЪЕДИНЁННЫЙ КОНТИНГЕНТ", f1, (225, 230, 220))
    ctext(d, SW // 2, 515, "СИЛ ОХРАНЫ ПЕРИМЕТРА", f1, (225, 230, 220))
    d.polygon([(SW // 2, 615), (SW // 2 + 14, 629), (SW // 2, 643), (SW // 2 - 14, 629)], outline=col, width=3)
    d.line([SW // 2 - 300, 629, SW // 2 - 40, 629], fill=col, width=2)
    d.line([SW // 2 + 40, 629, SW // 2 + 300, 629], fill=col, width=2)
    f2 = font(B, 56)
    ctext(d, SW // 2, 670, "ВОЕННЫЙ ПЕРСОНАЛЬНЫЙ", f2, col)
    ctext(d, SW // 2, 740, "КОМПЬЮТЕР", f2, col)
    ctext(d, SW // 2, 820, "ВПК-7М №3184", font(B, 66), (225, 230, 220))
    screen = scanlines(s)

    fr = canvas(W, H)
    d = ImageDraw.Draw(fr)
    # button icons
    ix = mm(-73, 28); d.arc([ix[0] - 26, ix[1] - 26, ix[0] + 26, ix[1] + 26], -60, 240, fill=paint, width=6); d.line([ix[0], ix[1] - 32, ix[0], ix[1] - 4], fill=paint, width=6)
    ix = mm(-73, 10); d.ellipse([ix[0] - 12, ix[1] - 12, ix[0] + 12, ix[1] + 12], outline=paint, width=5)
    for k in range(8):
        a = k * math.pi / 4
        d.line([ix[0] + math.cos(a) * 20, ix[1] + math.sin(a) * 20, ix[0] + math.cos(a) * 30, ix[1] + math.sin(a) * 30], fill=paint, width=5)
    ix = mm(73, -8)
    for k in range(3):
        d.line([ix[0] - 24, ix[1] - 16 + k * 16, ix[0] + 24, ix[1] - 16 + k * 16], fill=paint, width=6)
    for (dx, dz, pts) in [(0, 9, [(0, -14), (-12, 6), (12, 6)]), (0, -9, [(0, 14), (-12, -6), (12, -6)]),
                          (-9, 0, [(-14, 0), (6, -12), (6, 12)]), (9, 0, [(14, 0), (-6, -12), (-6, 12)])]:
        bx, by = mm(73 + dx, 22 + dz)
        d.polygon([(bx + p[0], by + p[1]) for p in pts], fill=paint)
    stripes(d, *mm(58, -48), 4, (200, 170, 110, 220))
    fr = wear(fr, 3000, 1)

    bk = canvas(W, H)
    d = ImageDraw.Draw(bk)
    mast(d, *mv(2, 38), 150, (205, 196, 160, 230), 5)
    ctext(d, mv(2, 0)[0], mv(0, 16)[1], "ОБЪЕДИНЁННЫЙ КОНТИНГЕНТ", font(B, 54), paint)
    ctext(d, mv(2, 0)[0], mv(0, 9)[1], "СИЛ ОХРАНЫ ПЕРИМЕТРА", font(B, 54), paint)
    ctext(d, mv(2, 0)[0], mv(0, -2)[1], "ВПК-7М №3184", font(B, 80), paint)
    d.rectangle([*mv(-22, -16), *mv(26, -24)], outline=paint, width=4)
    d.text(mv(-20, -17), "Инв. № 027645-30", font=font(BM, 42), fill=paint)
    barcode(d, *mv(-20, -22.5), 50, 30, paint)
    stripes(d, *mv(30, -38), 4, (205, 196, 160, 210))
    bk = wear(bk, 6000, 2)
    save(fr, bk, screen)

# =====================================================================================================
elif MODEL == "mono":
    W, H = 182, 108
    def mm(x, z): return (int((x + W / 2) * 10), int((H / 2 - z) * 10))
    mv = mm
    bone = (215, 210, 195, 235)
    def emblem(d, cx, cy, h, col, w=5):
        # obelisk with arcs and a cross-hair ring
        d.polygon([(cx, cy - h * 0.55), (cx + h * 0.12, cy + h * 0.35), (cx - h * 0.12, cy + h * 0.35)], outline=col, width=w)
        d.line([cx - h * 0.3, cy + h * 0.4, cx + h * 0.3, cy + h * 0.4], fill=col, width=w)
        d.arc([cx - h * 0.45, cy - h * 0.45, cx + h * 0.45, cy + h * 0.45], 200, 340, fill=col, width=w)
        d.arc([cx - h * 0.6, cy - h * 0.6, cx + h * 0.6, cy + h * 0.6], 210, 330, fill=col, width=max(1, w - 2))
        for a in (-0.8, -0.5, -0.2, 0.2, 0.5, 0.8):
            d.line([cx - h * 0.3 + a * 30, cy + h * 0.4, cx + a * h * 0.5, cy + h * 0.55], fill=col, width=max(1, w - 2))
    SW, SH = 1800, 1050
    s = Image.new("RGB", (SW, SH), (6, 14, 18))
    d = ImageDraw.Draw(s)
    cy = (90, 220, 220)
    status_icons(d, SW, cy)
    ctext(d, SW // 2, 50, "МОНОЛИТ", font(B, 80), (150, 230, 230))
    d.line([300, 95, 700, 95], fill=(60, 140, 140), width=3); d.line([1100, 95, 1500, 95], fill=(60, 140, 140), width=3)
    # glowing brain of veins
    glow = Image.new("RGB", (SW, SH), (0, 0, 0))
    g = ImageDraw.Draw(glow)
    for k in range(260):
        a = random.uniform(0, 2 * math.pi)
        r = random.uniform(0, 1)
        x, y = SW // 2 + math.cos(a) * 330 * r, 430 + math.sin(a) * 220 * r
        for _ in range(4):
            nx, ny = x + random.uniform(-45, 45), y + random.uniform(-35, 35)
            if ((nx - SW / 2) / 340) ** 2 + ((ny - 430) / 230) ** 2 < 1:
                g.line([x, y, nx, ny], fill=(80, 220, 230), width=3)
                x, y = nx, ny
    glow = glow.filter(ImageFilter.GaussianBlur(2))
    s = Image.composite(glow, s, glow.convert("L").point(lambda v: min(255, v * 3)))
    d = ImageDraw.Draw(s)
    mast(d, SW // 2, 700, 260, (5, 10, 12), 9, arcs=False)
    d.rectangle([0, 690, SW, SH], fill=(4, 8, 10))
    for x in (220, 420, 1380, 1580):
        d.rectangle([x, 380, x + 8, 690], fill=(40, 120, 125))
    ctext(d, SW // 2, 760, "ПЕРСОНАЛЬНЫЙ ТЕРМИНАЛ СВЯЗИ", font(B, 64), (140, 225, 225))
    ctext(d, SW // 2, 840, "С ОСОЗНАНИЕМ", font(B, 64), (140, 225, 225))
    # interference
    for _ in range(40):
        y = random.randrange(SH)
        d.line([0, y, SW, y], fill=(30, 80, 85), width=random.choice([1, 2]))
    screen = scanlines(s, 0.7, 1.2, 14)

    fr = canvas(W, H)
    d = ImageDraw.Draw(fr)
    # emblem on the cloth wrap, right end
    lay = canvas(W, H); ld = ImageDraw.Draw(lay)
    emblem(ld, *mm(80, 8), 190, bone, 7)
    fr.alpha_composite(wear(lay, 9000, 3))
    # tag on the bead chain (left, hanging)
    emblem(d, *mm(-93, -36), 70, (60, 55, 45, 255), 3)
    # button icons
    for (z, kind) in ((30, "up"), (12, "stop"), (-6, "down"), (-26, "power")):
        x, y = mm(62, z)
        c = (200, 200, 190, 230) if kind != "power" else (140, 240, 240, 255)
        if kind == "up": d.line([x - 14, y + 7, x, y - 7, x + 14, y + 7], fill=c, width=5)
        if kind == "down": d.line([x - 14, y - 7, x, y + 7, x + 14, y - 7], fill=c, width=5)
        if kind == "stop": d.rectangle([x - 10, y - 10, x + 10, y + 10], outline=c, width=5)
        if kind == "power":
            d.arc([x - 18, y - 18, x + 18, y + 18], -60, 240, fill=c, width=5); d.line([x, y - 22, x, y - 2], fill=c, width=5)
    fr = wear(fr, 1500, 4)

    bk = canvas(W, H)
    lay = canvas(W, H); ld = ImageDraw.Draw(lay)
    emblem(ld, *mv(18, 22), 150, (190, 185, 170, 220), 5)
    ctext(ld, mv(18, 0)[0], mv(0, -2)[1], "МОНОЛИТ", font(B, 76), (190, 185, 170, 220))
    ctext(ld, mv(18, 0)[0], mv(0, -16)[1], "ПИК №ОС-031", font(BM, 60), (190, 185, 170, 200))
    barcode(ld, *mv(2, -27), 50, 24, (190, 185, 170, 160))
    bk.alpha_composite(wear(lay, 9000, 5))
    save(fr, bk, screen)

# =====================================================================================================
elif MODEL == "nii":
    W, H = 198, 112
    def mm(x, z): return (int((x + W / 2) * 10), int((H / 2 - z) * 10))
    mv = mm
    ink = (40, 42, 44, 240)
    def logo(d, cx, cy, s, col, w=6, text=True):
        mast(d, cx, cy + s * 0.2, s * 0.9, col, w, arcs=False)
        for k in (1, 2):
            r = s * (0.35 + 0.18 * k)
            d.arc([cx - r, cy - r * 0.9, cx + r, cy + r * 0.9], 190, 350, fill=col, width=w)
        if text:
            ctext(d, cx, cy + s * 0.05, "НИИ", font(B, int(s * 0.55)), col)
    SW, SH = 1950, 1050
    s = Image.new("RGB", (SW, SH), (28, 38, 44))
    d = ImageDraw.Draw(s)
    for y in range(SH):
        c = int(28 + 26 * (1 - y / SH))
        d.line([0, y, SW, y], fill=(c - 6, c + 2, c + 8))
    col = (215, 225, 230)
    status_icons(d, SW, col)
    d.text((SW - 330, 50), "100%", font=font(B, 40), fill=col)
    logo(d, 720, 320, 300, col, 7)
    d.line([960, 220, 960, 420], fill=col, width=3)
    for i, t in enumerate(["НАУЧНО-", "ИССЛЕДОВАТЕЛЬСКИЙ", "ИНСТИТУТ"]):
        d.text((990, 230 + i * 62), t, font=font(B, 52), fill=col)
    sil = (14, 20, 24)
    for x in range(0, SW, 34):
        h = random.randint(40, 110)
        d.polygon([(x, 640), (x + 17, 640 - h), (x + 34, 640)], fill=sil)
    d.rectangle([0, 630, SW, SH], fill=sil)
    d.rectangle([760, 560, 1100, 640], fill=sil); d.rectangle([880, 520, 930, 560], fill=sil)
    d.ellipse([1380, 500, 1520, 600], fill=sil); d.line([1450, 600, 1450, 640], fill=sil, width=10)
    ctext(d, SW // 2, 690, "ПЕРСОНАЛЬНЫЙ", font(B, 64), col)
    ctext(d, SW // 2, 770, "ИССЛЕДОВАТЕЛЬСКИЙ КОМПЬЮТЕР", font(B, 64), col)
    ctext(d, SW // 2, 860, "№042", font(B, 96), (235, 240, 245))
    screen = scanlines(s, 0.85, 0.9, 6)

    fr = canvas(W, H)
    d = ImageDraw.Draw(fr)
    d.text(mm(-62, -46.5), "НИИ", font=font(B, 66), fill=ink)
    stripes(d, *mm(30, -46), 2, (210, 110, 30, 240), w=22, h=50, gap=12)
    d.text(mm(50, -47.5), "ПИК №042", font=font(B, 44), fill=ink)
    # button icons
    x, y = mm(-83, 0); c = (200, 205, 205, 235)
    d.arc([x - 18, y - 18, x + 18, y + 18], -60, 240, fill=c, width=5); d.line([x, y - 22, x, y - 2], fill=c, width=5)
    x, y = mm(83, 18); d.line([x - 14, y + 7, x, y - 7, x + 14, y + 7], fill=c, width=5)
    x, y = mm(83, -4)
    for k in range(3):
        d.line([x - 16, y - 12 + k * 12, x + 16, y - 12 + k * 12], fill=c, width=5)
    fr = wear(fr, 1200, 6)

    bk = canvas(W, H)
    d = ImageDraw.Draw(bk)
    logo(d, *mv(-8, 22), 130, ink, 5)
    for i, t in enumerate(["НАУЧНО-", "ИССЛЕДОВАТЕЛЬСКИЙ", "ИНСТИТУТ"]):
        d.text(mv(4, 30 - i * 6), t, font=font(B, 40), fill=ink)
    for i, t in enumerate(["ПЕРСОНАЛЬНЫЙ", "ИССЛЕДОВАТЕЛЬСКИЙ", "КОМПЬЮТЕР №042"]):
        d.text(mv(-22, 4 - i * 6), t, font=font(B, 40), fill=ink)
    barcode(d, *mv(-22, -16), 36, 40, ink, 5)
    q0 = mv(18, -12)
    for i in range(12):
        for j in range(12):
            if random.random() < 0.5 or (i < 3 and j < 3) or (i > 8 and j < 3) or (i < 3 and j > 8):
                d.rectangle([q0[0] + i * 6, q0[1] + j * 6, q0[0] + i * 6 + 5, q0[1] + j * 6 + 5], fill=ink)
    stripes(d, *mv(40, 28), 2, (210, 110, 30, 240), w=22, h=50, gap=12)
    bk = wear(bk, 2000, 7)
    save(fr, bk, screen)

# =====================================================================================================
elif MODEL == "broken":
    # the brick KPK (78 x 140 mm, portrait), burnt and smashed. Its own front/back come from model/.
    BASE = sys.argv[3]
    W, H = 78, 140
    fr = Image.open(BASE + "/front.png").convert("RGBA")
    bk = Image.open(BASE + "/back.png").convert("RGBA")
    def soot(img, n, seed):
        rnd = random.Random(seed)
        lay = Image.new("RGBA", img.size, (0, 0, 0, 0))
        d = ImageDraw.Draw(lay)
        w, h = img.size
        for _ in range(n):
            x, y = rnd.randrange(w), rnd.randrange(int(h * 0.45), h)
            r = rnd.randrange(20, 120)
            d.ellipse([x - r, y - r, x + r, y + r], fill=(12, 10, 8, rnd.randrange(60, 140)))
        lay = lay.filter(ImageFilter.GaussianBlur(25))
        img.alpha_composite(lay)
        return img
    fr = soot(fr, 26, 8)
    bk = soot(bk, 18, 9)
    fr.save(OUT + "/front.png")
    bk.save(OUT + "/back.png")       # already flipped in the brick's decal set
    # smashed screen: dead dark glass with a frozen, faded UI and cracks from an impact point
    scr = Image.open(BASE + "/screen.png").convert("RGB")
    scr = Image.blend(scr, Image.new("RGB", scr.size, (4, 6, 5)), 0.82)
    d = ImageDraw.Draw(scr)
    w, h = scr.size
    ox, oy = int(w * 0.62), int(h * 0.38)
    rnd = random.Random(10)
    for k in range(22):
        a = k * 2 * math.pi / 22 + rnd.uniform(-0.12, 0.12)
        x, y = ox, oy
        for seg in range(9):
            nx = x + math.cos(a) * rnd.uniform(40, 110)
            ny = y + math.sin(a) * rnd.uniform(40, 110)
            d.line([x, y, nx, ny], fill=(150, 165, 155), width=rnd.choice([1, 2, 2, 3]))
            x, y = nx, ny
            a += rnd.uniform(-0.3, 0.3)
    for r in (30, 75, 140):
        d.ellipse([ox - r, oy - r, ox + r, oy + r], outline=(120, 135, 125), width=2)
    d.ellipse([ox - 14, oy - 14, ox + 14, oy + 14], fill=(200, 210, 205))
    scr = scr.filter(ImageFilter.GaussianBlur(0.6))
    scr.save(OUT + "/screen.png")
    print(MODEL, "decals ok")
