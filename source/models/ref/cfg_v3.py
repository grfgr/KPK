# v3: modelled from scratch, the sheets only give proportions, colours and the printed bits.
from cfg import CFG

# materials: colour sampled from the sheet box (median), kind = procedural pattern
MATS = {
    "mil":  {"body": dict(color=(92, 85, 58), kind="paint", scratch=0.9, rust=0.5),
             "trim": dict(color=(60, 59, 56), kind="metal", scratch=0.7, rust=0.3)},
    "mono": {"body": dict(color=(98, 94, 86), kind="paint", scratch=1.0, rust=0.2),
             "trim": dict(color=(72, 70, 64), kind="metal", scratch=0.6, rust=0.1),
             "cloth": dict(color=(96, 85, 67), kind="cloth")},
    "nii":  {"body": dict(color=(196, 194, 190), kind="paint", scratch=0.6, rust=0.0),
             "trim": dict(color=(32, 33, 32), kind="rubber")},
    "m1":   {"body": dict(color=(84, 74, 64), kind="metal", scratch=1.0, rust=0.6),
             "trim": dict(color=(122, 114, 106), kind="metal", scratch=0.5, rust=0.2)},
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
