# v4 decals, all drawn here (no photo cut-outs): button icons, LEDs, labels, emblems, stripes,
# radiation signs, barcodes, on a background cut from the very material they sit on.
# The only photo piece is the screen picture. usage: gen_v4.py <sheet.png> <name> <v3 dir>
import json, sys, math, random, os
from PIL import ImageFilter as _IF
import numpy as np
from PIL import Image, ImageDraw, ImageFont, ImageFilter
from cfg_v4 import CFG

sheet, name, out = sys.argv[1], sys.argv[2], sys.argv[3]
c = CFG[name]
random.seed(7)
FB = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"
FC = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf"
MM = c["W"] / (c["body"][2] - c["body"][0])                   # mm per front px
BB = c["backbody"]; MMB = c["W"] / (BB[2] - BB[0])            # mm per back px
PXMM = 12                                                      # texture px per mm
TILES = {k: Image.open(f"{out}/{k}_co.png").convert("RGB") for k in c["mats"]}
TILE_MM = {"body": 90.0, "trim": 60.0, "cloth": 30.0}


def bg_from(key, wpx, hpx, color=None):
    if color is not None and not isinstance(color, str):
        base = Image.new("RGB", (wpx, hpx), tuple(color))
        n = np.asarray(TILES["trim"].resize((wpx, hpx)).convert("L")).astype(np.float32)
        a = np.asarray(base).astype(np.float32) * (0.85 + (n / max(n.mean(), 1)) * 0.15)[..., None]
        return Image.fromarray(np.clip(a, 0, 255).astype(np.uint8))
    tile = TILES[key]; s = tile.width / (TILE_MM[key] * PXMM)    # tile px per texture px
    w_, h_ = max(2, int(wpx * s)), max(2, int(hpx * s))
    x0, y0 = random.randint(0, tile.width - min(w_, tile.width)), random.randint(0, tile.height - min(h_, tile.height))
    big = Image.new("RGB", (tile.width * 2, tile.height * 2)); [big.paste(tile, (i * tile.width, j * tile.height)) for i in (0, 1) for j in (0, 1)]
    return big.crop((x0, y0, x0 + w_, y0 + h_)).resize((wpx, hpx), Image.LANCZOS)


def font(path, size):
    return ImageFont.truetype(path, max(6, int(size)))


def fit_text(d, text, box, path, fill):
    x0, y0, x1, y1 = box; size = (y1 - y0) * 0.9
    while size > 6:
        f = font(path, size); tb = d.textbbox((0, 0), text, font=f)
        if tb[2] - tb[0] <= (x1 - x0) * 0.96 and tb[3] - tb[1] <= (y1 - y0) * 0.95:
            break
        size *= 0.92
    tb = d.textbbox((0, 0), text, font=f)
    d.text(((x0 + x1 - (tb[2] - tb[0])) / 2 - tb[0], (y0 + y1 - (tb[3] - tb[1])) / 2 - tb[1]), text, font=f, fill=fill)


def tower(d, box, fill, w):
    x0, y0, x1, y1 = box; cx = (x0 + x1) / 2; hh = y1 - y0
    d.line([(cx, y0 + hh * 0.05), (cx - hh * 0.18, y1), ], fill=fill, width=w); d.line([(cx, y0 + hh * 0.05), (cx + hh * 0.18, y1)], fill=fill, width=w)
    for k in range(1, 5):
        yy = y0 + hh * (0.05 + 0.95 * k / 5); dx = hh * 0.18 * k / 5
        d.line([(cx - dx, yy), (cx + dx, yy)], fill=fill, width=max(1, w // 2))
    for r in (0.45, 0.62):
        d.arc([cx - hh * r * 1.4, y0 + hh * 0.15, cx + hh * r * 1.4, y0 + hh * 0.15 + hh * r * 2.0], 200, 340, fill=fill, width=max(1, w // 2))


def obelisk(d, box, fill, w):
    x0, y0, x1, y1 = box; cx = (x0 + x1) / 2; hh = y1 - y0
    d.polygon([(cx, y0), (cx + hh * 0.1, y1 - hh * 0.15), (cx - hh * 0.1, y1 - hh * 0.15)], fill=fill)
    d.polygon([(cx - hh * 0.3, y1), (cx + hh * 0.3, y1), (cx + hh * 0.12, y1 - hh * 0.16), (cx - hh * 0.12, y1 - hh * 0.16)], fill=fill)
    for r in (0.3, 0.42):
        d.arc([cx - hh * r, y0 + hh * 0.25, cx + hh * r, y0 + hh * 0.25 + hh * r * 2], 150, 210, fill=fill, width=max(1, w // 2))
        d.arc([cx - hh * r, y0 + hh * 0.25, cx + hh * r, y0 + hh * 0.25 + hh * r * 2], 330, 30, fill=fill, width=max(1, w // 2))


def barcode(d, box, fill):
    x0, y0, x1, y1 = box; x = x0
    while x < x1:
        bw = random.choice((1, 1, 2, 3)) * max(1, (x1 - x0) / 90)
        if random.random() > 0.4:
            d.rectangle([x, y0, min(x1, x + bw), y1], fill=fill)
        x += bw * 1.6


def radiation(d, box, fill):
    x0, y0, x1, y1 = box; cx, cy = (x0 + x1) / 2, (y0 + y1) / 2; r = min(x1 - x0, y1 - y0) / 2 * 0.92
    for a in (90, 210, 330):
        d.pieslice([cx - r, cy - r, cx + r, cy + r], a - 30, a + 30, fill=fill)
    d.ellipse([cx - r * 0.42, cy - r * 0.42, cx + r * 0.42, cy + r * 0.42], fill=None)
    d.ellipse([cx - r * 0.18, cy - r * 0.18, cx + r * 0.18, cy + r * 0.18], fill=fill)


def bg_of(pt):
    return pt.get("bg", "trim")


def make(pt, wmm, hmm):
    S = 3   # supersampling
    k_ = min(1.0, 700.0 / max(wmm * PXMM, hmm * PXMM))      # keep the aspect when capping the size
    wpx, hpx = max(16, int(wmm * PXMM * k_)), max(16, int(hmm * PXMM * k_))
    bgk = bg_of(pt)
    im = bg_from(bgk if isinstance(bgk, str) else "trim", wpx * S, hpx * S, None if isinstance(bgk, str) else bgk)
    if pt["t"] == "btn" and "color" in pt:
        im = bg_from("trim", wpx * S, hpx * S, pt["color"])
    d = ImageDraw.Draw(im); W_, H_ = im.size; lw = max(2, int(min(W_, H_) * 0.07))
    fg = tuple(pt.get("fg", (205, 205, 200)))
    t = pt["t"]
    if t == "btn":
        ic = pt.get("icon", "none"); g = tuple(pt.get("glow", (210, 210, 205)))
        cx, cy, r = W_ / 2, H_ / 2, min(W_, H_) * 0.26
        if ic == "power":
            d.arc([cx - r, cy - r, cx + r, cy + r], -60, 240, fill=g, width=lw); d.line([(cx, cy - r * 1.2), (cx, cy + r * 0.1)], fill=g, width=lw)
        elif ic == "sun":
            d.ellipse([cx - r * 0.45, cy - r * 0.45, cx + r * 0.45, cy + r * 0.45], outline=g, width=lw)
            for k in range(8):
                a = k * math.pi / 4; d.line([(cx + math.cos(a) * r * 0.75, cy + math.sin(a) * r * 0.75), (cx + math.cos(a) * r * 1.15, cy + math.sin(a) * r * 1.15)], fill=g, width=lw)
        elif ic == "menu":
            for k in (-1, 0, 1):
                d.line([(cx - r, cy + k * r * 0.6), (cx + r, cy + k * r * 0.6)], fill=g, width=lw)
        elif ic in ("up", "down"):
            s = -1 if ic == "up" else 1
            d.line([(cx - r, cy - s * r * 0.45), (cx, cy + s * r * 0.45), (cx + r, cy - s * r * 0.45)], fill=g, width=lw, joint="curve")
        elif ic == "square":
            d.rectangle([cx - r * 0.6, cy - r * 0.6, cx + r * 0.6, cy + r * 0.6], fill=g)
        elif ic in ("dpad", "dpad4"):
            k = 0.68 if ic == "dpad" else 0.62; a = min(W_, H_) * 0.07
            for ang in (0, 90, 180, 270):
                ca, sa = math.cos(math.radians(ang)), math.sin(math.radians(ang))
                px, py = cx + ca * W_ * k / 2 * 0.9, cy + sa * H_ * k / 2 * 0.9
                d.polygon([(px + ca * a, py + sa * a), (px - sa * a - ca * a * 0.4, py + ca * a - sa * a * 0.4), (px + sa * a - ca * a * 0.4, py - ca * a - sa * a * 0.4)], fill=g)
            d.ellipse([cx - a, cy - a, cx + a, cy + a], fill=g)
        elif ic.startswith("t:"):
            fit_text(d, ic[2:], (W_ * 0.12, H_ * 0.14, W_ * 0.88, H_ * 0.86), FB, g)
        elif ic == "call_c":
            fit_text(d, "С", (cx - r * 1.6, cy - r * 1.6, cx + r * 1.6, cy + r * 1.6), FB, (200, 50, 40))
        elif ic == "call_ok":
            fit_text(d, "☎", (cx - r * 1.6, cy - r * 1.6, cx + r * 1.6, cy + r * 1.6), "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", (70, 180, 70))
    elif t == "led":
        col = np.array(pt["color"], np.float32)
        yy, xx = np.mgrid[0:H_, 0:W_]; dd = np.maximum(np.abs(xx - W_ / 2) / (W_ / 2), np.abs(yy - H_ / 2) / (H_ / 2)) if "r" in pt else np.hypot(xx - W_ / 2, yy - H_ / 2) / (min(W_, H_) / 2)
        glow = np.clip(1.15 - dd, 0, 1)[..., None]
        a = col * (0.55 + 0.45 * glow) + 255 * np.clip(0.6 - dd, 0, 1)[..., None] * 0.5
        im = Image.fromarray(np.clip(a, 0, 255).astype(np.uint8))
    elif t in ("label", "blabel", "emblem"):
        dr = pt.get("draw", "text")
        if dr == "plate":
            d.rectangle([lw, lw, W_ - lw, H_ - lw], outline=tuple(int(v * 0.6) for v in (pt["bg"] if not isinstance(pt["bg"], str) else (90, 90, 90))), width=max(1, lw // 2))
        if dr in ("text", "plate"):
            lines = pt["lines"]; weights = [2.2 if l.startswith("@") and l not in ("@barcode",) else (0.9 if l == "@barcode" else 1.0) for l in lines]
            tot = sum(weights); y = H_ * 0.06; usable = H_ * 0.88
            for l, wgt in zip(lines, weights):
                hh = usable * wgt / tot; pad = 0.2 if dr == "plate" else 0.08
                box = (W_ * 0.08, y + hh * pad, W_ * 0.92, y + hh * (1 - pad))
                sw = max(2, int(min(W_, H_) * 0.018))
                if l == "@tower": tower(d, box, fg, sw)
                elif l == "@obelisk": obelisk(d, box, fg, sw)
                elif l == "@barcode": barcode(d, (W_ * 0.25, box[1], W_ * 0.75, box[3]), fg)
                elif l == "@nii":
                    tower(d, (box[0], box[1], box[0] + (box[2] - box[0]) * 0.45, box[3]), fg, sw)
                    fit_text(d, "НИИ", (box[0] + (box[2] - box[0]) * 0.4, box[1], box[2], box[3]), FB, fg)
                else:
                    fit_text(d, l, box, FB if pt.get("bold", True) else FC, fg)
                y += hh
        elif dr == "stripes" or dr == "stripes_text":
            acc = tuple(pt.get("accent", fg)); sw = H_ * 0.32; x = -H_ if dr == "stripes" else W_ * 0.02
            end = W_ + H_ if dr == "stripes" else W_ * 0.3
            while x < end:
                d.polygon([(x, H_), (x + sw, H_), (x + sw + H_ * 0.6, 0), (x + H_ * 0.6, 0)], fill=acc); x += sw * 2.1
            if dr == "stripes_text":
                fit_text(d, pt["lines"][0], (W_ * 0.38, H_ * 0.15, W_ * 0.98, H_ * 0.85), FB, fg)
        elif dr == "nii_back":
            # logo + institute name on top, three lines of text, barcode and QR code below, thin frame
            d.rectangle([lw, lw, W_ - lw, H_ - lw], outline=(150, 150, 150), width=max(1, lw // 3))
            sw = max(2, int(H_ * 0.012))
            tower(d, (W_ * 0.06, H_ * 0.08, W_ * 0.22, H_ * 0.44), fg, sw)
            fit_text(d, "НИИ", (W_ * 0.17, H_ * 0.2, W_ * 0.36, H_ * 0.46), FB, fg)
            d.line([(W_ * 0.39, H_ * 0.1), (W_ * 0.39, H_ * 0.44)], fill=fg, width=max(1, sw // 2))
            for k, t_ in enumerate(["НАУЧНО-", "ИССЛЕДОВАТЕЛЬСКИЙ", "ИНСТИТУТ"]):
                fit_text(d, t_, (W_ * 0.42, H_ * (0.1 + k * 0.11), W_ * 0.78, H_ * (0.2 + k * 0.11)), FB, fg)
            for k, t_ in enumerate(["ПЕРСОНАЛЬНЫЙ", "ИССЛЕДОВАТЕЛЬСКИЙ", "КОМПЬЮТЕР №042"]):
                fit_text(d, t_, (W_ * 0.06, H_ * (0.56 + k * 0.11), W_ * 0.4, H_ * (0.65 + k * 0.11)), FB, fg)
            barcode(d, (W_ * 0.45, H_ * 0.6, W_ * 0.7, H_ * 0.84), fg)
            q0, q1 = W_ * 0.75, H_ * 0.56; qs = H_ * 0.3; cell = qs / 21
            rnd = random.Random(42)
            for i_ in range(21):
                for j_ in range(21):
                    fin = (i_ < 7 and j_ < 7) or (i_ < 7 and j_ > 13) or (i_ > 13 and j_ < 7)
                    on = fin and not (1 <= i_ % 14 <= 5 and 1 <= j_ % 14 <= 5 and not (2 <= i_ % 14 <= 4 and 2 <= j_ % 14 <= 4)) if fin else rnd.random() > 0.55
                    if on:
                        d.rectangle([q0 + j_ * cell, q1 + i_ * cell, q0 + (j_ + 1) * cell, q1 + (i_ + 1) * cell], fill=fg)
        elif dr == "scratch":
            # scratched in with a knife: thin wobbly strokes, each letter cut two or three times
            txt = pt["lines"][0]
            size = H_ * 0.8
            while size > 8:
                f = font("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", size)
                tb = d.textbbox((0, 0), txt, font=f)
                if tb[2] - tb[0] <= W_ * 0.9 and tb[3] - tb[1] <= H_ * 0.8:
                    break
                size *= 0.93
            tb = d.textbbox((0, 0), txt, font=f); tx = (W_ - (tb[2] - tb[0])) / 2 - tb[0]; ty = (H_ - (tb[3] - tb[1])) / 2 - tb[1]
            tmp = Image.new("L", (W_, H_), 0); td = ImageDraw.Draw(tmp)
            rnd = random.Random(5)
            for k in range(3):
                td.text((tx + rnd.uniform(-1.5, 1.5) * 3, ty + rnd.uniform(-1.5, 1.5) * 3), txt, font=f, fill=255)
            # keep only a thin core of the strokes (erode), break them a bit
            core = tmp.filter(ImageFilter.MinFilter(3))
            m_ = np.asarray(core).astype(np.float32) / 255
            brk = np.asarray(Image.fromarray((np.random.default_rng(2).random((H_ // 6 + 1, W_ // 6 + 1)) * 255).astype(np.uint8)).resize((W_, H_))).astype(np.float32) / 255
            m_ = m_ * np.clip((brk - 0.15) * 3, 0, 1)
            base = np.asarray(im).astype(np.float32)
            col_ = np.array(fg, np.float32)
            sh_ = np.clip(np.roll(m_, 3, axis=0) - m_, 0, 1)[..., None]
            out_ = base * (1 - m_[..., None]) + col_ * m_[..., None]
            out_ = out_ * (1 - sh_ * 0.6)
            im = Image.fromarray(np.clip(out_, 0, 255).astype(np.uint8)); d = ImageDraw.Draw(im)
        elif dr == "finger":
            sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
            from fingerprint import mask as _fmask
            base = np.asarray(im).astype(np.float32)
            m_ = _fmask(W_, H_, 7, 15.0) * 0.85
            col_ = np.array(fg, np.float32)
            im = Image.fromarray(np.clip(base * (1 - m_[..., None]) + col_ * m_[..., None], 0, 255).astype(np.uint8)); d = ImageDraw.Draw(im)
        elif dr == "radiation":
            radiation(d, (W_ * 0.08, H_ * 0.08, W_ * 0.92, H_ * 0.92), fg)
    im = im.resize((wpx, hpx), Image.LANCZOS)
    if t in ("label", "blabel"):
        # a little wear on prints
        n = np.asarray(TILES["body"].resize((wpx, hpx)).convert("L")).astype(np.float32) / 255
        im = Image.fromarray(np.clip(np.asarray(im).astype(np.float32) * (0.9 + n[..., None] * 0.15), 0, 255).astype(np.uint8))
    return im


img = Image.open(sheet).convert("RGB")
_scr = os.path.join(os.path.dirname(os.path.abspath(__file__)), "screens", name + ".png")
items = {"screen": Image.open(_scr).convert("RGB") if os.path.exists(_scr) else img.crop(c["screen"])}
for i, pt in enumerate(c["parts"]):
    t = pt["t"]
    if t in ("btn", "led", "label", "blabel"):
        if "c" in pt:
            wmm = hmm = pt["rad"] * 2 * MM
        else:
            a, b, cc, dd = pt["r"]; k = MMB if t == "blabel" else MM
            wmm, hmm = (cc - a) * k, (dd - b) * k
        items["p%d" % i] = make(pt, wmm, hmm)
if c.get("cloth"):
    e = c["cloth"]["emblem"]
    items["emblem"] = make(dict(t="emblem", draw="text", bg="cloth", fg=(118, 106, 84), lines=["@obelisk"]), (e[2] - e[0]) * MM, (e[3] - e[1]) * MM)

S = 2048
atlas = Image.new("RGB", (S, S), (40, 40, 38)); regions = {}
x = y = shelf = 0
for k in sorted(items, key=lambda k: -items[k].height):
    im = items[k]
    if k == "screen":
        im = im.resize((1600, int(1600 * im.height / im.width)), Image.LANCZOS)
    if x + im.width > S:
        x = 0; y += shelf + 4; shelf = 0
    atlas.paste(im, (x, y)); regions[k] = [x / S, y / S, im.width / S, im.height / S]
    x += im.width + 4; shelf = max(shelf, im.height)
assert y + shelf <= S, "atlas overflow"
atlas.save(out + "/photo_co.png")
L = np.asarray(atlas.convert("L").filter(ImageFilter.GaussianBlur(0.8))).astype(np.float32) / 255
gx = (np.roll(L, -1, 1) - np.roll(L, 1, 1)) * 0.5; gy = (np.roll(L, -1, 0) - np.roll(L, 1, 0)) * 0.5
n = np.dstack([-gx * 2.5, gy * 2.5, np.ones_like(L)]); n /= np.linalg.norm(n, axis=2, keepdims=True); n = (n * 0.5 + 0.5) * 255; n[..., 1] = 255 - n[..., 1]
Image.fromarray(n.astype(np.uint8)).save(out + "/photo_nohq.png")
lum = np.asarray(atlas.convert("L")).astype(np.float32)
Image.fromarray(np.dstack([np.full_like(lum, 255), np.clip(lum * 0.35, 10, 110), np.full_like(lum, 80)]).astype(np.uint8)).save(out + "/photo_smdi.png")
json.dump(dict(regions=regions), open(out + "/v3.json", "w"))
print(name, "decals", len(regions))
