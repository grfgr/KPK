# Decal images projected onto the device in the bake (front / back / screen), drawn with PIL.
import random, math
from PIL import Image, ImageDraw, ImageFont, ImageFilter
OUT = __import__('sys').argv[1]
F = "/usr/share/fonts/truetype/dejavu/"
def font(name, size): return ImageFont.truetype(F + name, size)
random.seed(7)

# device size in mm: width 78, length 140 -> 10 px / mm
W, H = 780, 1400
def mm(x, z):  # device coords (mm, origin centre, z up) -> image px
    return (int((x + 39) * 10), int((70 - z) * 10))

# ---------------- screen (56 x 48 mm), the KPK UI as a still image
SW, SH = 1120, 960
scr = Image.new("RGB", (SW, SH), (6, 18, 9))
d = ImageDraw.Draw(scr)
g, gd, am = (150, 225, 120), (70, 120, 60), (230, 170, 60)
d.rectangle([0, 0, SW, 60], fill=(20, 48, 26))
d.text((20, 12), "ВЕСТНИК ОС 2.11 — КАРТА", font=font("DejaVuSans-Bold.ttf", 30), fill=(215, 232, 200))
d.text((SW - 190, 14), "18:42", font=font("DejaVuSansMono-Bold.ttf", 30), fill=g)
# map area with grid, contour lines and markers
d.rectangle([20, 80, 760, 880], outline=gd, width=3)
for i in range(1, 8):
    d.line([20 + i * 92, 80, 20 + i * 92, 880], fill=(30, 60, 32), width=2)
    d.line([20, 80 + i * 100, 760, 80 + i * 100], fill=(30, 60, 32), width=2)
cont = Image.new("RGB", (SW, SH), (6, 18, 9))
dc = ImageDraw.Draw(cont)
for k in range(9):
    pts = []
    for t in range(0, 361, 10):
        a = math.radians(t)
        r = 60 + k * 38 + 18 * math.sin(a * 3 + k)
        pts.append((390 + r * math.cos(a) * 1.2, 480 + r * math.sin(a)))
    dc.line(pts, fill=(45, 95, 48), width=2)
scr.paste(cont.crop((23, 83, 758, 878)), (23, 83))
for i in range(1, 8):
    d.line([20 + i * 92, 80, 20 + i * 92, 880], fill=(30, 60, 32), width=2)
    d.line([20, 80 + i * 100, 760, 80 + i * 100], fill=(30, 60, 32), width=2)
d.line([(120, 820), (260, 640), (330, 600), (480, 420), (620, 300), (700, 150)], fill=(90, 150, 80), width=5)
for (x, y, c) in [(260, 640, am), (480, 420, (220, 60, 50)), (620, 300, am), (330, 260, (80, 160, 240))]:
    d.polygon([(x, y - 18), (x + 16, y + 12), (x - 16, y + 12)], outline=c, width=4)
d.ellipse([370, 500, 400, 530], outline=(235, 235, 220), width=4)
d.text((410, 498), "Я", font=font("DejaVuSans-Bold.ttf", 28), fill=(235, 235, 220))
# side list
d.text((790, 90), "МЕТКИ", font=font("DejaVuSans-Bold.ttf", 28), fill=am)
for i, t in enumerate(["Тайник у моста", "Аномалия", "Лагерь", "Радиация", "Точка сбора"]):
    d.rectangle([785, 140 + i * 62, 1100, 190 + i * 62], fill=(14, 34, 17))
    d.text((800, 150 + i * 62), t, font=font("DejaVuSans.ttf", 24), fill=g)
d.rectangle([0, SH - 56, SW, SH], fill=(12, 28, 15))
d.text((18, SH - 46), "> Сигнал: устойчивый_", font=font("DejaVuSansMono.ttf", 26), fill=g)
d.text((SW - 330, SH - 46), "ПИТ 63%  GPS OK", font=font("DejaVuSansMono.ttf", 26), fill=gd)
# scanlines + blur
px = scr.load()
for y in range(0, SH, 4):
    for x in range(SW):
        r, gg, b = px[x, y]
        px[x, y] = (int(r * 0.6), int(gg * 0.6), int(b * 0.6))
scr = scr.filter(ImageFilter.GaussianBlur(1.2))
scr.save(OUT + "/screen.png")

# ---------------- front decals (RGBA)
fr = Image.new("RGBA", (W, H), (0, 0, 0, 0))
d = ImageDraw.Draw(fr)
paint = (205, 200, 175, 235)
# stencil name above the screen
d.text(mm(-25, 68.5), "КПК-7", font=font("DejaVuSans-Bold.ttf", 46), fill=paint)
d.text(mm(4, 67.5), "«ВЕСТНИК»", font=font("DejaVuSans-Bold.ttf", 36), fill=paint)
# key captions under the keys
cap = font("DejaVuSans-Bold.ttf", 30)
for i, t in enumerate(["МЕНЮ", "OK", "ВЫКЛ"]):
    x = -20 + i * 20
    d.text(mm(x - 6, -10.3), t, font=font("DejaVuSans-Bold.ttf", 22), fill=paint)
keys = ["1", "2", "3", "4", "5", "6", "7", "8", "9", "*", "0", "#"]
for i, t in enumerate(keys):
    c, r = i % 3, i // 3
    x, z = -16 + c * 16, -17 - r * 11
    d.text(mm(x - 1.6, z + 3), t, font=cap, fill=(200, 195, 170, 235))
# warning triangle and plate
d.polygon([mm(27, 62), mm(33, 52), mm(21, 52)], outline=(225, 175, 40, 240), width=6)
d.text(mm(25.8, 60), "!", font=font("DejaVuSans-Bold.ttf", 54), fill=(225, 175, 40, 240))
# small sticker at the bottom, crooked
st = Image.new("RGBA", (420, 110), (205, 196, 160, 255))
sd = ImageDraw.Draw(st)
sd.rectangle([0, 0, 420, 16], fill=(170, 30, 30, 255))
sd.text((14, 24), "ПРОВЕРЕНО  ОТК-4", font=font("DejaVuSans-Bold.ttf", 34), fill=(40, 34, 25, 255))
sd.text((14, 66), "НИИ ЧАЗ  1986", font=font("DejaVuSans.ttf", 30), fill=(60, 52, 40, 255))
for _ in range(900):  # dirt and wear on the paper
    x, y = random.randrange(420), random.randrange(110)
    a = random.randrange(10, 80)
    sd.point((x, y), fill=(90, 75, 50, 255) if random.random() < 0.7 else (230, 225, 200, 255))
sd.polygon([(360, 110), (420, 50), (420, 110)], fill=(0, 0, 0, 0))  # torn corner
st = st.rotate(3, expand=True, resample=Image.BICUBIC)
fr.alpha_composite(st, (int(16.5 * 10) + 30, int((70 + 62) * 10) - 40))
fr.save(OUT + "/front.png")

# ---------------- back decals (seen from behind: x mirrored when projected)
bk = Image.new("RGBA", (W, H), (0, 0, 0, 0))
d = ImageDraw.Draw(bk)
d.text(mm(-24, 30), "БАТАРЕЯ  9В", font=font("DejaVuSans-Bold.ttf", 54), fill=paint)
d.text(mm(-24, 22), "+  ВСТАВЛЯТЬ  ▲", font=font("DejaVuSans-Bold.ttf", 34), fill=paint)
d.text(mm(-30, -40), "ИНВ. № 07-0347", font=font("DejaVuSansMono-Bold.ttf", 44), fill=paint)
d.text(mm(-30, -47), "ЗАВ. № 860412", font=font("DejaVuSansMono-Bold.ttf", 36), fill=(190, 185, 160, 200))
# seal sticker, half torn off
seal = Image.new("RGBA", (300, 120), (190, 30, 30, 255))
sd = ImageDraw.Draw(seal)
sd.text((18, 14), "ПЛОМБА", font=font("DejaVuSans-Bold.ttf", 40), fill=(235, 220, 200, 255))
sd.text((18, 64), "НЕ ВСКРЫВАТЬ", font=font("DejaVuSans-Bold.ttf", 30), fill=(235, 220, 200, 255))
mask = Image.new("L", seal.size, 255)
md = ImageDraw.Draw(mask)
md.polygon([(170, 0), (300, 0), (300, 120), (120, 120), (200, 60)], fill=0)
seal.putalpha(mask)
bk.alpha_composite(seal.rotate(-4, expand=True), (mm(-6, 4)))
# a name scratched in with a knife
scr_col = (215, 210, 195, 200)
d.text(mm(-28, -58), "СЕМЁНОВ", font=font("DejaVuSans.ttf", 44), fill=scr_col)
for _ in range(3):
    y0 = mm(0, -55.5)[1] + random.randint(-8, 8)
    d.line([mm(-30, 0)[0], y0, mm(4, 0)[0], y0 + random.randint(-6, 6)], fill=scr_col, width=3)
bk = bk.transpose(Image.FLIP_LEFT_RIGHT)
bk.save(OUT + "/back.png")
print("decals ok")
