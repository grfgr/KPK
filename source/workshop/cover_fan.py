# Workshop cover v2 (1024x1024): three KPKs (КПК-М1, ВПК-7М, ПИК №042) in a fan over a dark PDA-screen
# backdrop, title, and a red diagonal author ribbon in the bottom-left corner.
# usage: cover_fan.py <front_m1.png> <front_mil.png> <front_nii.png> <out.jpg>
import sys, math, random
from PIL import Image, ImageDraw, ImageFilter, ImageFont
N = 1024; random.seed(11)
FB = "/usr/share/fonts/opentype/inter/Inter-Black.otf"
FM = "/usr/share/fonts/truetype/jetbrains-mono/JetBrainsMono-Bold.ttf"
FMR = "/usr/share/fonts/truetype/jetbrains-mono/JetBrainsMono-Regular.ttf"

# backdrop: dark green radial glow, grid, map contour rings, faint log lines like a PDA screen
bg = Image.new("RGB", (N, N), (6, 10, 8))
glow = Image.new("L", (N, N), 0)
ImageDraw.Draw(glow).ellipse([N * 0.05, N * 0.0, N * 0.95, N * 0.9], fill=255)
glow = glow.filter(ImageFilter.GaussianBlur(160))
bg = Image.composite(Image.new("RGB", (N, N), (22, 40, 28)), bg, glow).convert("RGBA")
ov = Image.new("RGBA", (N, N), (0, 0, 0, 0)); od = ImageDraw.Draw(ov)
for x in range(0, N, 40):
    od.line([(x, 0), (x, N)], fill=(110, 200, 120, 12))
    od.line([(0, x), (N, x)], fill=(110, 200, 120, 12))
cx, cy = N * 0.5, N * 0.42
for k in range(9):
    r = 70 + k * 70
    od.ellipse([cx - r * 1.25, cy - r, cx + r * 1.25, cy + r], outline=(130, 220, 140, 22), width=2)
fm = ImageFont.truetype(FMR, 17)
names = ["Сталкер-17", "Дегтярёв", "Лис", "Фагот", "Сыч", "Шрам", "Бармен", "Волк"]
for i in range(30):
    y = 14 + i * 34
    t = "[%02d:%02d] %s → метка  %04d %04d  сигнал %d%%" % (random.randint(0, 23), random.randint(0, 59), random.choice(names), random.randint(1000, 9999), random.randint(1000, 9999), random.randint(20, 99))
    od.text((22, y), t, font=fm, fill=(120, 200, 120, 26))
bg = Image.alpha_composite(bg, ov)

# the fan: three renders rotated around a point below the frame
def prep(path, w):
    im = Image.open(path).convert("RGBA")
    im = im.crop(im.getbbox())
    h = int(im.height * w / im.width)
    return im.resize((w, h), Image.LANCZOS)
items = [(sys.argv[1], -24, -205, 40), (sys.argv[3], 24, 205, 40), (sys.argv[2], 0, 0, 0)]   # back ones first, mil on top
layer = Image.new("RGBA", (N, N), (0, 0, 0, 0))
for path, ang, dx, dy in items:
    im = prep(path, 560).rotate(-ang, resample=Image.BICUBIC, expand=True)
    x = int(N / 2 - im.width / 2 + dx); y = int(N * 0.40 - im.height / 2 + dy)
    sh = Image.new("RGBA", (N, N), (0, 0, 0, 0)); sh.paste((0, 0, 0, 190), (x + 18, y + 26), im.split()[3])
    layer = Image.alpha_composite(layer, sh.filter(ImageFilter.GaussianBlur(22)))
    layer.alpha_composite(im, (x, y))
halo = Image.new("RGBA", (N, N), (0, 0, 0, 0)); halo.paste((110, 230, 120, 70), (0, 0), layer.split()[3])
bg = Image.alpha_composite(bg, halo.filter(ImageFilter.GaussianBlur(40)))
bg = Image.alpha_composite(bg, layer)

# title band
band = Image.new("RGBA", (N, N), (0, 0, 0, 0)); bd = ImageDraw.Draw(band)
for y in range(N - 280, N):
    bd.line([(0, y), (N, y)], fill=(3, 6, 4, int(min(235, (y - (N - 280)) * 2.0))))
bg = Image.alpha_composite(bg, band); d = ImageDraw.Draw(bg)
ft = ImageFont.truetype(FB, 200)
title = "КПК"
tw = d.textlength(title, font=ft); tx = (N - tw) / 2 + 90; ty = N - 300
gl = Image.new("RGBA", (N, N), (0, 0, 0, 0)); ImageDraw.Draw(gl).text((tx, ty), title, font=ft, fill=(120, 255, 130, 150))
bg = Image.alpha_composite(bg, gl.filter(ImageFilter.GaussianBlur(14))); d = ImageDraw.Draw(bg)
d.text((tx, ty), title, font=ft, fill=(226, 236, 214))
fs = ImageFont.truetype(FM, 34)
sub = "КАРТА · ГРУППА · ЗВОНКИ"
sw = d.textlength(sub, font=fs); d.text(((N - sw) / 2 + 120, N - 66), sub, font=fs, fill=(214, 60, 48))

# scanlines
sl = Image.new("RGBA", (N, N), (0, 0, 0, 0)); sd = ImageDraw.Draw(sl)
for y in range(0, N, 4):
    sd.line([(0, y), (N, y)], fill=(0, 0, 0, 20))
bg = Image.alpha_composite(bg, sl)

# author ribbon: red band across the bottom-left corner, dark edges, white text with a black outline
RL = 640
R = Image.new("RGBA", (RL, 104), (0, 0, 0, 0)); rd = ImageDraw.Draw(R)
rd.rectangle([0, 0, RL, 104], fill=(20, 8, 8, 255))
rd.rectangle([0, 8, RL, 96], fill=(206, 36, 30, 255))
fr = ImageFont.truetype(FM, 54)
txt = "ZloyDed.dll"
w_ = rd.textlength(txt, font=fr)
rd.text(((RL - w_) / 2, 52), txt, font=fr, fill=(250, 250, 250, 255), anchor="lm", stroke_width=4, stroke_fill=(15, 15, 15, 255))
Rr = R.rotate(-45, resample=Image.BICUBIC, expand=True)
# centre of the band over the corner diagonal, its ends cut by the left and bottom edges
RC = 168
pos = (int(RC - Rr.width / 2), int(N - RC - Rr.height / 2))
rs = Image.new("RGBA", (N, N), (0, 0, 0, 0)); rs.paste((0, 0, 0, 150), pos, Rr.split()[3])
bg = Image.alpha_composite(bg, rs.filter(ImageFilter.GaussianBlur(10)))
bg.alpha_composite(Rr, pos) if pos[0] >= 0 and pos[1] >= 0 else None
if pos[0] < 0 or pos[1] < 0:
    tmp = Image.new("RGBA", (N, N), (0, 0, 0, 0)); tmp.paste(Rr, pos, Rr); bg = Image.alpha_composite(bg, tmp)
bg.convert("RGB").save(sys.argv[4], quality=92)
print("ok")
