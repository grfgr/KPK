# v3: modelled from scratch, the sheets only give proportions, colours and the printed bits.
from cfg import CFG

# materials: colour sampled from the sheet box (median), kind = procedural pattern
MATS = {
    "mil":  {"body": dict(color=(80, 74, 50), kind="paint", scratch=0.4, rust=0.5, soft=True),
             "trim": dict(color=(56, 55, 52), kind="metal", scratch=0.3, rust=0.25, soft=True)},
    "mono": {"body": dict(color=(86, 84, 78), kind="paint", scratch=0.4, rust=0.2, soft=True),
             "trim": dict(color=(62, 61, 57), kind="metal", scratch=0.3, rust=0.1, soft=True),
             "cloth": dict(color=(96, 85, 67), kind="cloth")},
    "nii":  {"body": dict(color=(196, 194, 190), kind="paint", scratch=0.6, rust=0.0),
             "trim": dict(color=(32, 33, 32), kind="rubber")},
    "m1":   {"body": dict(color=(66, 60, 53), kind="metal", scratch=0.35, rust=0.45, soft=True),
             "trim": dict(color=(96, 92, 86), kind="metal", scratch=0.25, rust=0.15, soft=True)},
}
# shared by all models
SHARED = {"kpk_metal": dict(color=(150, 148, 144), kind="metal", scratch=0.8, rust=0.15),
          "kpk_weave": dict(color=(104, 96, 64), kind="weave")}

# printed / photographed bits put on thin plates (front view px); "back" = whole back panel
DECALS = {
    "mil":  [(380, 380, 462, 400)],
    "mono": [],
    "nii":  [(238, 413, 302, 441), (488, 413, 642, 441)],
    "m1":   [(628, 70, 704, 97), (78, 222, 152, 302)],
}

# extra 3D: rubber port covers on the sides (y px of the front view, length px)
PORTS = {"mil": [("right", 225, 60), ("left", 225, 40)], "mono": [("left", 300, 40)],
         "nii": [("left", 245, 60), ("right", 245, 60)], "m1": [("right", 250, 60), ("left", 260, 40)]}

# Monolith cloth band around the right part of the case (front px x range) + emblem plate
CLOTH = {"mono": dict(x=(684, 742), emblem=(690, 150, 742, 330))}

for k in CFG:
    CFG[k]["mats"] = MATS[k]
    CFG[k]["decals"] = DECALS[k]
    CFG[k]["ports"] = PORTS[k]
    CFG[k]["cloth"] = CLOTH.get(k)
