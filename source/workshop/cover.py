# Workshop cover 1024x1024: hero render of the broken KPK over a dark PDA-screen backdrop (radar rings,
# grid, scanlines, faint radiation sign), title at the bottom. usage: cover.py <hero.png> <out.jpg>
import sys, math, random
from PIL import Image, ImageDraw, ImageFilter, ImageFont
N = 1024; random.seed(3)
bg = Image.new("RGB", (N, N), (8, 12, 10)); d = ImageDraw.Draw(bg)
for y in range(N):   # vignette gradient
    for_ = 1 - abs(y - N * 0.42) / N
    d.line([(0, y), (N, y)], fill=(int(10 + 14 * for_), int(16 + 26 * for_), int(12 + 16 * for_)))
g = Image.new("RGBA", (N, N), (0, 0, 0, 0)); gd = ImageDraw.Draw(g)
for x in range(0, N, 48):
    gd.line([(x, 0), (x, N)], fill=(120, 220, 120, 14))
    gd.line([(0, x), (N, x)], fill=(120, 220, 120, 14))
cx, cy = N * 0.5, N * 0.44
for r in range(80, 760, 85):
    gd.ellipse([cx - r, cy - r, cx + r, cy + r], outline=(140, 230, 140, 30), width=2)
for k in range(3):   # radiation trefoil, faint
    a0 = -90 + k * 120 - 30
    gd.pieslice([cx - 360, cy - 360, cx + 360, cy + 360], a0, a0 + 60, fill=(230, 170, 60, 16))
gd.ellipse([cx - 70, cy - 70, cx + 70, cy + 70], fill=(8, 14, 10, 255))
gd.ellipse([cx - 45, cy - 45, cx + 45, cy + 45], fill=(230, 170, 60, 16))
for _ in range(40):   # map marks
    x, y = random.randint(40, N - 40), random.randint(40, N - 260)
    gd.text((x, y), random.choice(["+", "·", "×", "◦"]), fill=(150, 240, 150, 45))
bg = Image.alpha_composite(bg.convert("RGBA"), g)
hero = Image.open(sys.argv[1]).convert("RGBA")
s = 0.86; hero = hero.resize((int(N * s), int(N * s)), Image.LANCZOS)
hx, hy = (N - hero.width) // 2, 10
sh = Image.new("RGBA", (N, N), (0, 0, 0, 0)); sh.paste((0, 0, 0, 200), (hx + 26, hy + 34), hero.split()[3])
bg = Image.alpha_composite(bg, sh.filter(ImageFilter.GaussianBlur(26)))
glow = Image.new("RGBA", (N, N), (0, 0, 0, 0)); glow.paste((110, 230, 120, 90), (hx, hy), hero.split()[3])
bg = Image.alpha_composite(bg, glow.filter(ImageFilter.GaussianBlur(40)))
bg.alpha_composite(hero, (hx, hy))
# title band
band = Image.new("RGBA", (N, N), (0, 0, 0, 0)); bd = ImageDraw.Draw(band)
for y in range(N - 250, N):
    bd.line([(0, y), (N, y)], fill=(4, 8, 6, int(min(235, (y - (N - 250)) * 2.2))))
bg = Image.alpha_composite(bg, band); d = ImageDraw.Draw(bg)
fb = ImageFont.truetype("/usr/share/fonts/opentype/inter/Inter-Black.otf", 190)
fm = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf", 34)
title = "КПК"
tw = d.textlength(title, font=fb); tx = (N - tw) / 2; ty = N - 262
gl = Image.new("RGBA", (N, N), (0, 0, 0, 0)); ImageDraw.Draw(gl).text((tx, ty), title, font=fb, fill=(120, 255, 130, 160))
bg = Image.alpha_composite(bg, gl.filter(ImageFilter.GaussianBlur(14))); d = ImageDraw.Draw(bg)
d.text((tx, ty), title, font=fb, fill=(214, 240, 200))
sub = "КАРМАННЫЙ ПЕРСОНАЛЬНЫЙ КОМПЬЮТЕР"
sw = d.textlength(sub, font=fm); d.text(((N - sw) / 2, N - 62), sub, font=fm, fill=(230, 170, 60))
# scanlines
sl = Image.new("RGBA", (N, N), (0, 0, 0, 0)); sd = ImageDraw.Draw(sl)
for y in range(0, N, 4):
    sd.line([(0, y), (N, y)], fill=(0, 0, 0, 22))
bg = Image.alpha_composite(bg, sl)
bg.convert("RGB").save(sys.argv[2], quality=92)
print("ok")
