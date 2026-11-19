# Screen pictures of the full models, drawn like the brick's: a KPK program window in the colours of
# the model's interface theme, with a content picture of its own per faction.  usage: gen_screens.py
import math, random, os
from PIL import Image, ImageDraw, ImageFont, ImageFilter
from cfg_v4 import CFG
FB = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"
FM = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf"
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "screens")
TH = {
    "m1":   dict(bg=(26, 22, 14), panel=(40, 33, 20), text=(232, 200, 140), dim=(150, 124, 80), acc=(240, 170, 70),
                 os="СТАЛКЕР-ОС · КПК-М1", tabs=["ПРОФИЛЬ", "КАРТА", "ГРУППА", "ГРУППИРОВКИ", "ЗАДАНИЯ", "ЗАМЕТКИ"], page="КАРТА"),
    "mil":  dict(bg=(14, 20, 12), panel=(26, 34, 20), text=(190, 210, 150), dim=(110, 130, 80), acc=(220, 200, 90),
                 os="ОКСОП · ЗАКРЫТЫЙ КАНАЛ", tabs=["ЛИЧНОЕ ДЕЛО", "ОПЕРКАРТА", "ОТДЕЛЕНИЕ", "ПОДРАЗДЕЛЕНИЯ", "ПРИКАЗЫ", "ЖУРНАЛ"], page="ОПЕРКАРТА"),
    "nii":  dict(bg=(12, 18, 24), panel=(22, 32, 42), text=(200, 225, 240), dim=(110, 140, 160), acc=(120, 200, 255),
                 os="НИИ · ПИК №042", tabs=["СОТРУДНИК", "КАРТА ПОЛЕЙ", "ГРУППА", "ГРУППИРОВКИ", "ПРОТОКОЛЫ", "НАБЛЮДЕНИЯ"], page="КАРТА ПОЛЕЙ"),
    "mono": dict(bg=(6, 16, 16), panel=(10, 28, 28), text=(150, 240, 230), dim=(70, 140, 135), acc=(120, 255, 240),
                 os="# # #  ПИК №ОС-031", tabs=["[ I ]", "[ II ]", "[ III ]", "[ IV ]", "[ V ]", "[ VI ]"], page="[ II ]"),
}


def f(path, size):
    return ImageFont.truetype(path, int(size))


def screen(name):
    c = CFG[name]; t = TH[name]
    sx0, sy0, sx1, sy1 = c["screen"]
    W = 1400; H = int(W * (sy1 - sy0) / (sx1 - sx0))
    rnd = random.Random(hash(name) % 997)
    im = Image.new("RGB", (W, H), t["bg"]); d = ImageDraw.Draw(im)
    u = H / 100.0
    # title, menu, status bars
    d.rectangle([0, 0, W, 6 * u], fill=t["panel"]); d.rectangle([1.2 * u, 1.6 * u, 3 * u, 4.4 * u], fill=t["acc"])
    d.text((4.2 * u, 1.0 * u), t["os"] + "  —  " + t["page"], font=f(FB, 3.6 * u), fill=t["text"])
    d.text((W - 22 * u, 1.0 * u), "20.09.2026  18:42", font=f(FB, 3.2 * u), fill=t["dim"])
    d.rectangle([0, 6 * u, W, 10.5 * u], fill=tuple(int(v * 0.8) for v in t["panel"]))
    for i, m in enumerate(["Файл", "Сеть", "Карта", "Сервис", "Справка"]):
        d.text((2 * u + i * 14 * u, 6.6 * u), m, font=f(FB, 2.8 * u), fill=t["dim"])
    d.rectangle([0, 93 * u, W, H], fill=t["panel"])
    status = {"m1": "> Удачной охоты, сталкер._", "mil": "> Доложите обстановку._", "nii": "> Синхронизация данных: 100%_", "mono": "> . . . МОНОЛИТ СЛЫШИТ . . ._"}[name]
    d.text((2 * u, 94.2 * u), status, font=f(FB, 3.2 * u), fill=t["text"])
    d.text((W - 44 * u, 94.4 * u), "СИГН ||||   ПИТ 87%   GPS: OK", font=f(FB, 2.8 * u), fill=t["dim"])
    # sidebar with modules
    sw = 22 * u
    d.rectangle([0, 10.5 * u, sw, 93 * u], fill=tuple(int(v * 0.9) for v in t["panel"]))
    d.text((2 * u, 12 * u), "МОДУЛИ" if name != "mono" else "# # #", font=f(FB, 2.8 * u), fill=t["acc"])
    for i, tb in enumerate(t["tabs"]):
        y = 17 * u + i * 8.2 * u
        act = tb == t["page"]
        if act:
            d.rectangle([1 * u, y - 0.8 * u, sw - 1 * u, y + 5.6 * u], fill=tuple(min(255, int(v * 1.8)) for v in t["panel"]))
        lbl = tb if name == "mono" else "[%d] %s" % (i + 1, tb)
        d.text((2 * u, y), lbl, font=f(FB, 2.3 * u), fill=t["acc"] if act else t["text"])
    # content area
    x0, y0, x1, y1 = sw + 2 * u, 13 * u, W - 26 * u, 91 * u
    d.rectangle([x0, y0, x1, y1], outline=t["dim"], width=2)
    cx, cy = (x0 + x1) / 2, (y0 + y1) / 2
    if name in ("m1", "nii"):
        # terrain map: contour lines, grid, a road and markers
        for k in range(9):
            r = (k + 1) * (y1 - y0) / 18
            pts = []
            for a in range(0, 361, 8):
                rr = r * (1 + 0.12 * math.sin(math.radians(a) * 3 + k))
                pts.append((cx - 6 * u + math.cos(math.radians(a)) * rr * 1.6, cy + math.sin(math.radians(a)) * rr))
            d.line(pts, fill=tuple(int(v * 0.55) for v in t["text"]), width=2)
        for gx in range(int(x0), int(x1), int(9 * u)):
            d.line([(gx, y0), (gx, y1)], fill=tuple(int(v * 0.35) for v in t["dim"]), width=1)
        for gy in range(int(y0), int(y1), int(9 * u)):
            d.line([(x0, gy), (x1, gy)], fill=tuple(int(v * 0.35) for v in t["dim"]), width=1)
        road = [(x0 + 5 * u, y1 - 6 * u), (cx - 10 * u, cy + 6 * u), (cx + 4 * u, cy - 2 * u), (x1 - 6 * u, y0 + 5 * u)]
        d.line(road, fill=t["acc"], width=5)
        if name == "m1":
            marks = [((cx - 18 * u, cy - 14 * u), (230, 60, 50), "radiation"), ((cx + 14 * u, cy + 12 * u), (240, 200, 60), "art"),
                     ((cx + 22 * u, cy - 18 * u), (90, 150, 240), "camp")]
            lst = ["Радиация", "Артефакт", "Лагерь", "Тайник у моста", "Точка сбора"]
        else:
            marks = [((cx - 16 * u, cy - 10 * u), (120, 200, 255), "field"), ((cx + 12 * u, cy + 14 * u), (255, 120, 80), "field"),
                     ((cx + 24 * u, cy - 16 * u), (200, 240, 255), "probe")]
            lst = ["Поле «Жарка»", "Электра", "Зонд №3", "Пункт забора проб", "Бункер учёных"]
        for (mx, my), col, kind in marks:
            if kind in ("field",):
                for rr in (6 * u, 4 * u, 2 * u):
                    d.ellipse([mx - rr, my - rr, mx + rr, my + rr], outline=col, width=3)
            else:
                d.polygon([(mx, my - 3 * u), (mx + 2.6 * u, my + 2 * u), (mx - 2.6 * u, my + 2 * u)], outline=col, width=4)
        d.ellipse([cx - 1.6 * u, cy - 1.6 * u, cx + 1.6 * u, cy + 1.6 * u], outline=t["text"], width=3)
        d.text((cx + 2.4 * u, cy - 2 * u), "Я", font=f(FB, 3 * u), fill=t["text"])
    elif name == "mil":
        # operations map: grid squares with numbers, unit boxes, an arrow and a perimeter line
        step = 12 * u
        for i, gx in enumerate(range(int(x0), int(x1), int(step))):
            d.line([(gx, y0), (gx, y1)], fill=tuple(int(v * 0.45) for v in t["dim"]), width=1)
            d.text((gx + 0.6 * u, y0 + 0.4 * u), "%02d" % (40 + i), font=f(FM, 2.2 * u), fill=t["dim"])
        for j, gy in enumerate(range(int(y0), int(y1), int(step))):
            d.line([(x0, gy), (x1, gy)], fill=tuple(int(v * 0.45) for v in t["dim"]), width=1)
        per = [(x0 + 6 * u, y0 + 30 * u), (cx - 6 * u, y0 + 22 * u), (cx + 16 * u, y0 + 34 * u), (x1 - 4 * u, y0 + 26 * u)]
        d.line(per, fill=(220, 80, 60), width=4)
        for k in range(len(per)):
            px, py = per[k]; d.line([(px - 1.5 * u, py - 1.5 * u), (px + 1.5 * u, py + 1.5 * u)], fill=(220, 80, 60), width=3)
        for (ux, uy, lbl) in [(cx - 20 * u, cy + 10 * u, "ОТД-1"), (cx + 6 * u, cy + 18 * u, "ОТД-2"), (cx + 22 * u, cy - 4 * u, "БП-3")]:
            d.rectangle([ux - 3.4 * u, uy - 2.2 * u, ux + 3.4 * u, uy + 2.2 * u], outline=t["acc"], width=3)
            d.line([(ux - 3.4 * u, uy - 2.2 * u), (ux + 3.4 * u, uy + 2.2 * u)], fill=t["acc"], width=2)
            d.text((ux - 3.4 * u, uy + 2.8 * u), lbl, font=f(FB, 2.4 * u), fill=t["text"])
        d.line([(cx - 20 * u, cy + 6 * u), (cx - 6 * u, cy - 8 * u)], fill=t["acc"], width=5)
        d.polygon([(cx - 6 * u, cy - 8 * u), (cx - 9.5 * u, cy - 7 * u), (cx - 7.5 * u, cy - 4 * u)], fill=t["acc"])
        lst = ["КПП «Север»", "Вышка №2", "Сектор 41-07", "Нарушитель: 1", "Патруль ОТД-2"]
    else:
        # Monolith: rings around the obelisk, interference and glyphs
        for k in range(7):
            r = (k + 1) * (y1 - y0) / 15
            d.ellipse([cx - r * 1.4, cy - r, cx + r * 1.4, cy + r], outline=tuple(int(v * (0.9 - k * 0.1)) for v in t["acc"]), width=2)
        d.polygon([(cx, cy - 26 * u), (cx + 3 * u, cy + 14 * u), (cx - 3 * u, cy + 14 * u)], fill=t["acc"])
        d.polygon([(cx - 9 * u, cy + 18 * u), (cx + 9 * u, cy + 18 * u), (cx + 4 * u, cy + 13 * u), (cx - 4 * u, cy + 13 * u)], fill=t["acc"])
        for _ in range(60):
            gx = rnd.uniform(x0, x1); gy = rnd.uniform(y0, y1)
            d.text((gx, gy), rnd.choice("#%&@$?!/<>=+*01"), font=f(FM, 2.6 * u), fill=tuple(int(v * rnd.uniform(0.3, 0.9)) for v in t["text"]))
        for _ in range(14):
            gy = rnd.uniform(y0, y1); gw = rnd.uniform(10, 40) * u; gx = rnd.uniform(x0, x1 - gw)
            d.rectangle([gx, gy, gx + gw, gy + rnd.uniform(0.3, 1.2) * u], fill=tuple(int(v * 0.6) for v in t["acc"]))
        lst = ["ОСОЗНАНИЕ", "ЗОВ", "ЧАС", ". . .", "ИДИ"]
    # list on the right
    lx = W - 24 * u
    d.text((lx, 13 * u), "МЕТКИ" if name != "mono" else "# # #", font=f(FB, 3 * u), fill=t["acc"])
    for i, it in enumerate(lst):
        y = 18 * u + i * 6.6 * u
        d.rectangle([lx - 0.6 * u, y - 0.6 * u, W - 1.6 * u, y + 5 * u], fill=tuple(int(v * 1.25) for v in t["bg"]))
        d.text((lx, y), it, font=f(FB, 2.6 * u), fill=t["text"])
    # scanlines and a little glow
    sl = Image.new("L", (W, H), 0); sd = ImageDraw.Draw(sl)
    for yy in range(0, H, 4):
        sd.line([(0, yy), (W, yy)], fill=40)
    glow = im.filter(ImageFilter.GaussianBlur(4))
    im = Image.blend(im, glow, 0.25)
    im = Image.composite(Image.new("RGB", (W, H), (0, 0, 0)), im, sl)
    im.save(os.path.join(OUT, name + ".png"))
    print(name, im.size)


for n in ["m1", "mil", "nii", "mono"]:
    screen(n)
