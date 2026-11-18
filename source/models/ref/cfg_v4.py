# v4: everything modelled and drawn, the sheet gives only proportions and the screen picture.
from cfg_v3 import CFG

# btn icons: power sun menu up down left right square dpad call_c call_ok none
PARTS = {
    "brick": [
        dict(t="plate", r=(140, 382, 372, 606), h=0.6),
        dict(t="btn", r=(144, 393, 208, 415), h=1.6, shape="rect", icon="menu"),
        dict(t="btn", r=(224, 393, 288, 415), h=1.6, shape="rect", icon="t:OK"),
        dict(t="btn", r=(304, 393, 368, 415), h=1.6, shape="rect", icon="t:ВЫХ"),
        dict(t="btn", r=(168, 432, 216, 464), h=1.8, shape="rect", icon="t:1"),
        dict(t="btn", r=(232, 432, 280, 464), h=1.8, shape="rect", icon="t:2"),
        dict(t="btn", r=(296, 432, 344, 464), h=1.8, shape="rect", icon="t:3"),
        dict(t="btn", r=(168, 476, 216, 508), h=1.8, shape="rect", icon="t:4"),
        dict(t="btn", r=(232, 476, 280, 508), h=1.8, shape="rect", icon="t:5"),
        dict(t="btn", r=(296, 476, 344, 508), h=1.8, shape="rect", icon="t:6"),
        dict(t="btn", r=(168, 520, 216, 552), h=1.8, shape="rect", icon="t:7"),
        dict(t="btn", r=(232, 520, 280, 552), h=1.8, shape="rect", icon="t:8"),
        dict(t="btn", r=(296, 520, 344, 552), h=1.8, shape="rect", icon="t:9"),
        dict(t="btn", r=(168, 564, 216, 596), h=1.8, shape="rect", icon="t:*"),
        dict(t="btn", r=(232, 564, 280, 596), h=1.8, shape="rect", icon="t:0"),
        dict(t="btn", r=(296, 564, 344, 596), h=1.8, shape="rect", icon="t:#"),
        dict(t="guard", r=(94, 94, 160, 150), h=2.6), dict(t="guard", r=(352, 94, 418, 150), h=2.6),
        dict(t="guard", r=(94, 610, 160, 666), h=2.6), dict(t="guard", r=(352, 610, 418, 666), h=2.6),
        dict(t="lip", w=8, h=1.0),
        dict(t="label", r=(170, 104, 342, 128), h=0.12, draw="text", bg="body", fg=(226, 220, 196), lines=["КПК-7 «ВЕСТНИК»"], bold=True),
        dict(t="label", r=(172, 614, 340, 652), h=0.35, draw="plate", bg=(222, 214, 192), fg=(60, 30, 26), lines=["ПРОВЕРЕНО ОТК-4", "НИИ ЧАЗ 1986"]),
        dict(t="led", c=(388, 124), rad=6, h=1.2, color=(90, 230, 90)),
        dict(t="screw", c=(152, 372), rad=7), dict(t="screw", c=(360, 372), rad=7),
        dict(t="bgrille", r=(160, 150, 352, 250), h=1.4, n=7, dir="h"),
        dict(t="blabel", r=(170, 430, 342, 510), h=1.5, draw="plate", bg=(176, 172, 160), fg=(40, 38, 34), lines=["КПК-7 «ВЕСТНИК»", "ЗАВ. № 04173", "ПИТАНИЕ 9 В", "@barcode"]),
    ],
    "mil": [
        dict(t="btn", r=(110, 145, 150, 185), h=2.2, shape="round", icon="power"),
        dict(t="btn", r=(110, 197, 150, 238), h=2.2, shape="round", icon="sun"),
        dict(t="plate", r=(106, 253, 154, 334), h=1.0),
        dict(t="btn", r=(119, 268, 141, 320), h=3.2, shape="rect", base=False, icon="none", color=(150, 38, 28)),
        dict(t="btn", r=(638, 148, 707, 252), h=2.4, shape="cross", icon="dpad"),
        dict(t="btn", r=(648, 265, 702, 322), h=2.2, shape="round", icon="menu"),
        dict(t="guard", r=(84, 42, 166, 106), h=2.6), dict(t="guard", r=(646, 42, 728, 106), h=2.6),
        dict(t="guard", r=(84, 346, 166, 410), h=2.6), dict(t="guard", r=(646, 346, 728, 410), h=2.6),
        dict(t="clip", r=(210, 36, 268, 58)), dict(t="clip", r=(528, 36, 582, 58)),
        dict(t="lip", w=9, h=0.9),
        dict(t="grille", r=(203, 380, 294, 405), h=0.9, n=7, dir="v"), dict(t="grille", r=(510, 380, 602, 405), h=0.9, n=7, dir="v"),
        dict(t="label", r=(382, 382, 460, 400), h=0.15, draw="stripes", bg="body", fg=(196, 170, 120)),
        dict(t="screw", c=(178, 368), rad=7), dict(t="screw", c=(632, 368), rad=7),
        dict(t="screw", c=(178, 82), rad=7), dict(t="screw", c=(632, 82), rad=7),
        # back (back view px)
        dict(t="bgrille", r=(640, 520, 770, 690), h=1.1, n=6, dir="h"),
        dict(t="blabel", r=(790, 512, 985, 645), h=0.85, draw="text", bg="body", fg=(200, 180, 140),
             lines=["@tower", "ОБЪЕДИНЁННЫЙ КОНТИНГЕНТ", "СИЛ ОХРАНЫ ПЕРИМЕТРА", "ВПК-7М №3184"]),
        dict(t="blabel", r=(818, 645, 922, 692), h=1.4, draw="plate", bg=(52, 50, 46), fg=(200, 196, 186), lines=["Инв. № 027645-30", "@barcode"]),
        dict(t="blabel", r=(938, 650, 990, 682), h=0.85, draw="stripes", bg="body", fg=(196, 170, 120)),
        dict(t="bstrap", r=(985, 472, 1037, 712), h=2.0),
        dict(t="buckle", r=(986, 556, 1038, 596)),
    ],
    "mono": [
        dict(t="btn", r=(620, 150, 670, 200), h=2.0, shape="rect", icon="up"),
        dict(t="btn", r=(620, 205, 670, 255), h=2.0, shape="rect", icon="square"),
        dict(t="btn", r=(620, 262, 670, 312), h=2.0, shape="rect", icon="down"),
        dict(t="btn", r=(620, 318, 670, 365), h=2.0, shape="round", icon="power", glow=(90, 230, 220)),
        dict(t="lip", w=8, h=0.8),
        dict(t="led", r=(328, 401, 430, 413), h=0.9, color=(120, 245, 235)),
        dict(t="guard", r=(78, 38, 142, 96), h=2.0), dict(t="guard", r=(78, 358, 142, 418), h=2.0),
        dict(t="clip", r=(316, 36, 346, 56)), dict(t="clip", r=(558, 36, 590, 56)),
        dict(t="screw", c=(110, 70), rad=7), dict(t="screw", c=(110, 388), rad=7),
        dict(t="bgrille", r=(660, 512, 782, 685), h=1.4, n=5, dir="h"),
        dict(t="blabel", r=(805, 515, 975, 682), h=0.75, draw="text", bg="body", fg=(150, 146, 136),
             lines=["@obelisk", "МОНОЛИТ", "ПИК №ОС-031", "@barcode"]),
    ],
    "nii": [
        dict(t="btn", r=(88, 220, 128, 262), h=2.2, shape="round", icon="power"),
        dict(t="btn", r=(697, 155, 733, 215), h=2.2, shape="rect", icon="up"),
        dict(t="btn", r=(697, 235, 733, 295), h=2.2, shape="rect", icon="menu"),
        dict(t="led", r=(93, 139, 124, 150), h=0.9, color=(90, 230, 90)),
        dict(t="led", r=(93, 159, 124, 170), h=0.9, color=(235, 200, 50)),
        dict(t="led", r=(93, 179, 124, 190), h=0.9, color=(90, 230, 90)),
        dict(t="guard", r=(40, 50, 106, 116), h=2.4), dict(t="guard", r=(688, 50, 754, 116), h=2.4),
        dict(t="guard", r=(40, 384, 106, 448), h=2.4), dict(t="guard", r=(688, 384, 754, 448), h=2.4),
        dict(t="clip", r=(255, 36, 337, 60)), dict(t="clip", r=(513, 36, 597, 60)),
        dict(t="led", r=(400, 62, 418, 78), h=1.2, color=(30, 34, 40)),
        dict(t="lip", w=9, h=0.8),
        dict(t="label", r=(238, 414, 302, 440), h=0.12, draw="text", bg="body", fg=(40, 40, 42), lines=["НИИ"], bold=True),
        dict(t="label", r=(488, 414, 642, 440), h=0.12, draw="stripes_text", bg="body", fg=(60, 60, 62), accent=(220, 110, 30), lines=["ПИК №042"]),
        dict(t="screw", c=(99, 106), rad=8), dict(t="screw", c=(208, 425), rad=8), dict(t="screw", c=(712, 410), rad=8),
        dict(t="bgrille", r=(700, 588, 762, 700), h=1.8, n=7, dir="h"),
        dict(t="blabel", r=(780, 556, 1012, 698), h=1.3, draw="nii_back", bg=(214, 212, 208), fg=(38, 38, 40)),
        dict(t="blabel", r=(1022, 598, 1050, 662), h=0.75, draw="stripes", bg="body", fg=(220, 110, 30)),
    ],
    "m1": [
        dict(t="btn", r=(668, 152, 702, 198), h=2.0, shape="rect", icon="call_c", mat="trim"),
        dict(t="plate", r=(664, 203, 748, 277), h=1.0),
        dict(t="btn", r=(688, 218, 724, 262), h=2.6, shape="round", icon="dpad4"),
        dict(t="btn", r=(668, 285, 702, 330), h=2.0, shape="rect", icon="call_ok"),
        dict(t="led", c=(109, 108), rad=8, h=1.4, color=(230, 50, 40)),
        dict(t="led", c=(109, 125), rad=8, h=1.4, color=(235, 190, 40)),
        dict(t="led", c=(109, 141), rad=8, h=1.4, color=(70, 210, 60)),
        dict(t="ring", c=(95, 355), rad=20, tube=6),
        dict(t="label", r=(688, 372, 728, 408), h=0.6, draw="radiation", bg=(196, 160, 50), fg=(30, 28, 24), round=True),
        dict(t="label", r=(152, 70, 248, 100), h=1.5, draw="plate", bg=(196, 186, 160), fg=(50, 46, 40), lines=["НИГ-0473"]),
        dict(t="label", r=(626, 70, 706, 96), h=1.05, draw="text", bg="body", fg=(196, 176, 140), lines=["КПК-М1"], bold=True),
        dict(t="loop", r=(243, 408, 292, 444)), dict(t="loop", r=(478, 408, 527, 444)),
        dict(t="strap", r=(292, 414, 478, 442), h=1.4),
        dict(t="screw", c=(128, 78), rad=8), dict(t="screw", c=(705, 78), rad=8),
        dict(t="screw", c=(128, 420), rad=8), dict(t="screw", c=(705, 420), rad=8),
        dict(t="lip", w=8, h=0.8),
        dict(t="bgrille", r=(700, 570, 760, 680), h=2.2, n=8, dir="h"),
        dict(t="blabel", r=(822, 620, 932, 664), h=1.6, draw="plate", bg=(150, 140, 120), fg=(40, 36, 30), lines=["КПК-М1", "№0473-11", "@barcode"]),
        dict(t="blabel", r=(928, 566, 1022, 662), h=0.95, draw="radiation", bg="body", fg=(200, 160, 50), round=True),
    ],
}
# broken KPK: the brick, smashed
_b = []
for _p in PARTS["brick"]:
    _p = dict(_p)
    ic = _p.get("icon", "")
    if ic in ("t:5", "t:*", "t:ВЫХ", "t:9"):
        continue                                  # keys knocked out
    if ic in ("t:2", "t:0", "t:7"):
        _p["h"] = 0.5                             # keys pushed in
    if _p["t"] == "guard" and _p["r"] == (352, 610, 418, 666):
        continue                                  # bumper torn off
    if _p["t"] == "led":
        _p["color"] = (40, 44, 40)                # dead LED
    if _p["t"] == "label" and "ПРОВЕРЕНО ОТК-4" in _p.get("lines", []):
        _p["r"] = (172, 614, 290, 652)            # sticker half torn off
        _p["lines"] = ["ПРОВЕРЕНО", "НИИ ЧАЗ"]
    _b.append(_p)
_b.append(dict(t="blabel", r=(180, 524, 332, 566), h=0.82, draw="scratch", bg="body", fg=(205, 198, 178), flat=True, lines=["ZloyDed.dll"]))
PARTS["broken"] = _b
CFG["broken"]["parts"] = _b
CFG["broken"]["edge"] = 4.5

CFG["m1"]["D"] = 20
CFG["mil"]["D"] = 28
CFG["nii"]["D"] = 22
CFG["brick"]["edge"] = 4.5
CFG["mono"]["D"] = 22
CFG["mono"]["edge"] = 5.5
CFG["mono"]["screen_kind"] = "chamfer"
CFG["nii"]["edge"] = 5.0
CFG["mil"]["edge"] = 6.0
CFG["mil"]["screen_kind"] = "chamfer"
CFG["m1"]["edge"] = 4.6
for k in CFG:
    CFG[k]["parts"] = PARTS[k]
    CFG[k]["decals"] = []
    if CFG[k].get("cloth"):
        CFG[k]["cloth"]["emblem_draw"] = True
