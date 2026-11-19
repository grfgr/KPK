# atlas of the admin KPK «Куратор» (1024x1024): painted plastic, rubber, keys, screen artwork, name stripe.
# Also writes the textures for the game (kpk_admin_baked_co / _nohq / _smdi) into KPK/data.
# usage: gen_atlas.py <atlas.png>
import sys, os, random
import numpy as np
from PIL import Image, ImageDraw, ImageFont, ImageFilter
random.seed(5); rng = np.random.default_rng(5)
N = 1024
A = np.zeros((N, N, 3), np.float32)
FB = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"; FM = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf"


def region(u0, v0, u1, v1):   # atlas uv (v up) -> pixel box
    return int(u0 * N), int((1 - v1) * N), int(u1 * N), int((1 - v0) * N)


def fill(box, col, noise=0.04, blur=1.5):
    x0, y0, x1, y1 = box
    h, w = y1 - y0, x1 - x0
    n = rng.normal(0, 1, (h // 4 + 1, w // 4 + 1))
    n = np.array(Image.fromarray(((n * 40) + 128).clip(0, 255).astype(np.uint8)).resize((w, h), Image.BICUBIC).filter(ImageFilter.GaussianBlur(blur)), np.float32) / 128 - 1
    fine = rng.normal(0, 1, (h, w)) * 0.35
    A[y0:y1, x0:x1] = np.array(col, np.float32)[None, None, :] * (1 + (n[..., None] + fine[..., None]) * noise)


fill(region(0, 0, 0.5, 0.5), (52, 60, 72), 0.08)        # body: dark blue-grey plastic
fill(region(0.5, 0, 0.75, 0.25), (24, 24, 25), 0.10)     # rubber
fill(region(0.75, 0, 1, 0.25), (214, 96, 30), 0.06)      # accent orange
fill(region(0.5, 0.25, 0.75, 0.5), (70, 74, 80), 0.06)   # keys
fill(region(0.75, 0.25, 1, 0.5), (16, 18, 22), 0.05)     # dark bezel
fill(region(0.5, 0.75, 0.75, 1), (150, 152, 150), 0.10)  # metal
img = Image.fromarray(A.clip(0, 255).astype(np.uint8))
d = ImageDraw.Draw(img)

# screen: admin terminal (dark navy, red-orange accents): title bar, player list, map with a crosshair
x0, y0, x1, y1 = region(0, 0.5, 0.5, 1.0)
sc = Image.new("RGB", (1024, 512), (6, 10, 20)); sd = ImageDraw.Draw(sc)
f1 = ImageFont.truetype(FM, 26); f2 = ImageFont.truetype(FM, 20); f3 = ImageFont.truetype(FB, 22)
sd.rectangle([0, 0, 1024, 40], fill=(18, 30, 54)); sd.text((14, 7), "КУРАТОР · АДМИН-ТЕРМИНАЛ СЕТИ ЗОНЫ", font=f1, fill=(240, 150, 70))
sd.text((850, 9), "РЕЖИМ: ROOT", font=f2, fill=(255, 90, 70))
sd.rectangle([0, 40, 300, 512], fill=(10, 16, 30)); sd.line([300, 40, 300, 512], fill=(60, 90, 140), width=2)
for i, (nm, st) in enumerate([("Сталкер-17", "онлайн"), ("Дегтярёв", "онлайн"), ("Лис", "оффлайн"), ("Фагот", "онлайн"), ("Сыч", "онлайн"), ("Бармен", "оффлайн"), ("Шрам", "онлайн")]):
    y = 56 + i * 52
    if i == 1:
        sd.rectangle([6, y - 6, 294, y + 40], fill=(40, 60, 100))
    sd.text((16, y), nm, font=f3, fill=(210, 225, 245)); sd.text((16, y + 22), st, font=f2, fill=(110, 200, 120) if st == "онлайн" else (120, 120, 140))
for gx in range(300, 1024, 48):
    sd.line([gx, 40, gx, 470], fill=(20, 34, 60))
for gy in range(40, 470, 48):
    sd.line([300, gy, 1024, gy], fill=(20, 34, 60))
for r in (40, 90, 150, 220):
    sd.ellipse([660 - r, 250 - r, 660 + r, 250 + r], outline=(50, 80, 130), width=2)
sd.line([600, 250, 720, 250], fill=(255, 110, 70), width=3); sd.line([660, 190, 660, 310], fill=(255, 110, 70), width=3)
for _ in range(12):
    px, py = random.randint(330, 990), random.randint(60, 450)
    sd.ellipse([px - 6, py - 6, px + 6, py + 6], fill=(110, 200, 120))
sd.rectangle([0, 470, 1024, 512], fill=(14, 22, 40)); sd.text((14, 480), "> просмотр: Дегтярёв   группа: ОКСОП   заметок: 4", font=f2, fill=(180, 200, 230))
img.paste(sc.resize((x1 - x0, y1 - y0), Image.LANCZOS), (x0, y0))

# name stripe (plate is ~110 x 8 mm: draw at that aspect and squeeze into the 2:1 region)
x0, y0, x1, y1 = region(0.5, 0.5, 1.0, 0.75)
pl = Image.new("RGB", (1650, 120), (206, 92, 28)); pd = ImageDraw.Draw(pl)
for k in range(0, 1650, 40):
    pd.line([k, 0, k + 30, 120], fill=(196, 86, 26), width=6)
pf = ImageFont.truetype(FB, 64)
pd.text((40, 22), "КПК-А «КУРАТОР»", font=pf, fill=(20, 20, 22))
pd.text((830, 34), "АДМИНИСТРАТОР СЕТИ", font=ImageFont.truetype(FB, 44), fill=(30, 24, 20))
img.paste(pl.resize((x1 - x0, y1 - y0), Image.LANCZOS), (x0, y0))

# grime: a soft darkening noise over the whole atlas except the screen
g = Image.fromarray((rng.normal(0, 1, (64, 64)) * 30 + 128).clip(0, 255).astype(np.uint8)).resize((N, N), Image.BICUBIC).filter(ImageFilter.GaussianBlur(8))
gm = np.array(g, np.float32) / 128 - 1
arr = np.array(img, np.float32)
mask = np.ones((N, N), np.float32); sx0, sy0, sx1, sy1 = region(0, 0.5, 0.5, 1.0); mask[sy0:sy1, sx0:sx1] = 0
arr *= (1 + gm[..., None] * 0.10 * mask[..., None])
img = Image.fromarray(arr.clip(0, 255).astype(np.uint8))
img.save(sys.argv[1])

R = os.path.join(os.path.dirname(os.path.abspath(__file__)), "../../../KPK/data/")
img.save(R + "kpk_admin_baked_co.png")
Image.new("RGB", (N, N), (128, 128, 255)).save(R + "kpk_admin_baked_nohq.png")
# smdi: rubber matte, plastic a bit glossy, screen glossy
sm = Image.new("RGB", (N, N), (255, 70, 40)); smd = ImageDraw.Draw(sm)
smd.rectangle(region(0.5, 0, 0.75, 0.25), fill=(255, 20, 20))
smd.rectangle(region(0, 0.5, 0.5, 1.0), fill=(255, 200, 120))
smd.rectangle(region(0.5, 0.75, 0.75, 1), fill=(255, 160, 90))
sm.save(R + "kpk_admin_baked_smdi.png")
print("atlas ok")
