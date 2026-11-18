# Sheet pixel boxes (x0, y0, x1, y1) on the 1536 x 1024 concept sheets.
# front: the whole front view incl. handles (texture), body: the main case (geometry + scale),
# screen: recessed screen, buttons: raised 1.2 mm, extra parts per model.
CFG = {
    "mil": dict(W=208, D=36, corner=("chamfer", 40), front=(36, 30, 774, 414), body=(86, 44, 726, 408),
                screen=(172, 100, 626, 350),
                buttons=[(110, 145, 150, 185), (110, 197, 150, 238), (108, 255, 152, 332), (640, 150, 705, 250), (648, 265, 702, 322)],
                handles=True,
                back=(620, 470, 1092, 714), left=(50, 483, 103, 723), right=(162, 483, 223, 723),
                top=(262, 488, 598, 566), bottom=(262, 630, 598, 708)),
    "mono": dict(W=182, D=30, corner=("chamfer", 30), front=(74, 36, 748, 420), body=(80, 40, 745, 416),
                 screen=(158, 90, 603, 368),
                 buttons=[(620, 150, 670, 200), (620, 205, 670, 255), (620, 262, 670, 312), (620, 318, 670, 365)],
                 back=(648, 480, 1048, 710), left=(38, 495, 112, 697), right=(172, 495, 236, 697),
                 top=(272, 495, 605, 548), bottom=(272, 632, 600, 686)),
    "nii": dict(W=198, D=32, corner=("round", 26), front=(40, 36, 754, 448), body=(42, 52, 752, 446),
                screen=(150, 100, 680, 391),
                buttons=[(88, 220, 128, 262), (697, 155, 733, 215), (697, 235, 733, 295)],
                clips=[(255, 38, 335, 60), (515, 38, 595, 60)],
                back=(655, 510, 1077, 737), left=(42, 515, 105, 745), right=(160, 515, 225, 745),
                top=(268, 510, 617, 570), bottom=(268, 665, 617, 725)),
    "m1": dict(W=178, D=28, corner=("round", 30), front=(60, 48, 746, 446), body=(62, 52, 745, 436),
               screen=(150, 102, 640, 370),
               buttons=[(668, 152, 702, 198), (666, 205, 746, 275), (668, 285, 702, 330)],
               back=(648, 512, 1050, 728), left=(50, 512, 107, 744), right=(172, 512, 240, 744),
               top=(265, 535, 607, 600), bottom=(265, 665, 607, 730)),
}

# Volume details (sheet px; h in mm). Types:
#  btn   - button: flat bezel + pillowed cap (shape round / rect / cross), texture from the front view
#  plate - flat raised plate on the front     guard - corner bumper over front, back and sides
#  led   - small dome (c = centre, rad px)     screw - screw head
#  lip   - raised frame around the screen      clip  - block on the top edge
#  wrap  - band around the case (cloth)        ring  - torus on the front (lanyard ring)
#  loop  - metal loop under the bottom edge    bplate / bstrap - plate / strap on the back (back view px)
PARTS = {
    "mil": [
        dict(t="btn", r=(110, 145, 150, 185), h=2.2, shape="round"),
        dict(t="btn", r=(110, 197, 150, 238), h=2.2, shape="round"),
        dict(t="plate", r=(106, 253, 154, 334), h=1.0),
        dict(t="btn", r=(119, 268, 141, 320), h=3.2, shape="rect", base=False),
        dict(t="btn", r=(638, 148, 707, 252), h=2.4, shape="cross"),
        dict(t="btn", r=(648, 265, 702, 322), h=2.2, shape="round"),
        dict(t="guard", r=(84, 42, 166, 106), h=2.6), dict(t="guard", r=(646, 42, 728, 106), h=2.6),
        dict(t="guard", r=(84, 346, 166, 410), h=2.6), dict(t="guard", r=(646, 346, 728, 410), h=2.6),
        dict(t="clip", r=(210, 36, 268, 58)), dict(t="clip", r=(528, 36, 582, 58)),
        dict(t="lip", w=9, h=0.9),
        dict(t="plate", r=(203, 380, 294, 405), h=0.7), dict(t="plate", r=(510, 380, 602, 405), h=0.7),
        dict(t="screw", c=(178, 368), rad=7), dict(t="screw", c=(632, 368), rad=7),
        dict(t="screw", c=(178, 82), rad=7), dict(t="screw", c=(632, 82), rad=7),
        dict(t="bplate", r=(640, 520, 770, 690), h=1.1), dict(t="bplate", r=(818, 645, 922, 692), h=0.8),
        dict(t="bstrap", r=(985, 472, 1037, 712), h=2.0), dict(t="bplate", r=(988, 558, 1036, 594), h=3.4),
    ],
    "mono": [
        dict(t="btn", r=(620, 150, 670, 200), h=2.0, shape="rect"),
        dict(t="btn", r=(620, 205, 670, 255), h=2.0, shape="rect"),
        dict(t="btn", r=(620, 262, 670, 312), h=2.0, shape="rect"),
        dict(t="btn", r=(620, 318, 670, 365), h=2.0, shape="round"),
        dict(t="lip", w=8, h=0.8),
        dict(t="plate", r=(328, 400, 430, 415), h=0.9),
        dict(t="guard", r=(78, 38, 142, 96), h=2.0), dict(t="guard", r=(78, 358, 142, 418), h=2.0),
        dict(t="guard", r=(600, 358, 675, 418), h=2.0),
        dict(t="wrap", r=(682, 38, 750, 418), h=1.6),
        dict(t="clip", r=(316, 36, 346, 56)), dict(t="clip", r=(558, 36, 590, 56)),
        dict(t="screw", c=(110, 70), rad=7), dict(t="screw", c=(110, 388), rad=7),
        dict(t="bplate", r=(655, 505, 785, 690), h=1.0), dict(t="bplate", r=(790, 505, 985, 690), h=0.7),
    ],
    "nii": [
        dict(t="btn", r=(88, 220, 128, 262), h=2.2, shape="round"),
        dict(t="btn", r=(697, 155, 733, 215), h=2.2, shape="rect"),
        dict(t="btn", r=(697, 235, 733, 295), h=2.2, shape="rect"),
        dict(t="btn", r=(92, 138, 125, 151), h=0.9, shape="rect", base=False),
        dict(t="btn", r=(92, 158, 125, 171), h=0.9, shape="rect", base=False),
        dict(t="btn", r=(92, 178, 125, 191), h=0.9, shape="rect", base=False),
        dict(t="guard", r=(40, 50, 106, 116), h=2.4), dict(t="guard", r=(688, 50, 754, 116), h=2.4),
        dict(t="guard", r=(40, 384, 106, 448), h=2.4), dict(t="guard", r=(688, 384, 754, 448), h=2.4),
        dict(t="clip", r=(255, 36, 337, 60)), dict(t="clip", r=(513, 36, 597, 60)),
        dict(t="led", c=(409, 70), rad=9, h=1.2),
        dict(t="lip", w=9, h=0.8),
        dict(t="screw", c=(99, 106), rad=8), dict(t="screw", c=(208, 425), rad=8), dict(t="screw", c=(712, 410), rad=8),
        dict(t="bplate", r=(780, 555, 1050, 700), h=0.8), dict(t="bplate", r=(690, 588, 762, 702), h=0.7),
    ],
    "m1": [
        dict(t="btn", r=(668, 152, 702, 198), h=2.0, shape="rect"),
        dict(t="plate", r=(664, 203, 748, 277), h=1.0),
        dict(t="btn", r=(688, 218, 724, 262), h=2.6, shape="round"),
        dict(t="btn", r=(668, 285, 702, 330), h=2.0, shape="rect"),
        dict(t="led", c=(109, 108), rad=8, h=1.4), dict(t="led", c=(109, 125), rad=8, h=1.4), dict(t="led", c=(109, 141), rad=8, h=1.4),
        dict(t="ring", c=(95, 355), rad=20, tube=6),
        dict(t="plate", r=(688, 372, 728, 408), h=0.6),
        dict(t="plate", r=(152, 70, 248, 102), h=0.5),
        dict(t="loop", r=(243, 408, 292, 444)), dict(t="loop", r=(478, 408, 527, 444)),
        dict(t="plate", r=(292, 414, 478, 442), h=1.4),
        dict(t="screw", c=(128, 78), rad=8), dict(t="screw", c=(705, 78), rad=8),
        dict(t="screw", c=(128, 420), rad=8), dict(t="screw", c=(705, 420), rad=8),
        dict(t="lip", w=8, h=0.8),
        dict(t="bplate", r=(822, 620, 932, 664), h=0.8), dict(t="bplate", r=(722, 588, 772, 672), h=0.6),
    ],
}
for _k, _p in PARTS.items():
    CFG[_k]["parts"] = _p
    CFG[_k].pop("buttons", None)
    CFG[_k].pop("clips", None)
