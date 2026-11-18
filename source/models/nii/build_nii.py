# ПИК №042 (НИИ): white research PDA with black rubber bumpers, 198 x 112 x 32 mm
import sys, math, bpy
sys.path.insert(0, sys.path[0] if sys.path[0] else ".")
import kpklib as K
out, res = sys.argv[sys.argv.index("--") + 1], int(sys.argv[sys.argv.index("--") + 2])
W, H, T = 0.198, 0.112, 0.032
SX, SZ, SW, SH = 0.0, 0.004, 0.130, 0.070
K.init(out, res, W, H, T, (SX - SW / 2, SW, SZ - SH / 2, SH))
WHITE = K.mat_paint("white", (0.50, 0.50, 0.47, 1))
BLACK = K.mat_rubber_c("black", (0.012, 0.012, 0.012, 1), (0.024, 0.024, 0.023, 1))
METAL = K.mat_metal()
SCREEN = K.mat_screen()
GREEN = K.mat_glow("green", (0.2, 0.8, 0.15, 1))
YELLOW = K.mat_glow("yellow", (0.85, 0.65, 0.08, 1))
box, cyl, cut = K.box, K.cyl, K.cut
parts = []
body = box("body", (W - 0.01, T - 0.004, H - 0.01), (0, 0, 0), bevel=0.007, segs=3, mat=WHITE)
cut(body, box("c_scr", (SW + 0.004, 0.01, SH + 0.004), (SX, -T / 2, SZ)))
parts.append(body)
ring = box("ring", (SW + 0.014, 0.004, SH + 0.014), (SX, -T / 2, SZ), bevel=0.002, mat=BLACK)
cut(ring, box("c_ring", (SW + 0.002, 0.03, SH + 0.002), (SX, -T / 2, SZ)))
parts.append(ring)
parts.append(box("glass", (SW + 0.003, 0.001, SH + 0.003), (SX, -T / 2 + 0.003, SZ), mat=SCREEN))
# black rubber corner bumpers and side strips
for sx in (-1, 1):
    for sz in (-1, 1):
        parts.append(box("bump", (0.03, T + 0.002, 0.03), (sx * (W / 2 - 0.012), 0, sz * (H / 2 - 0.012)), bevel=0.006, segs=3, mat=BLACK))
    parts.append(box("side", (0.006, T, H - 0.05), (sx * (W / 2 - 0.002), 0, 0), bevel=0.002, mat=BLACK))
# top latches and the camera bump
for sx in (-1, 1):
    parts.append(box("latch", (0.022, 0.016, 0.009), (sx * 0.05, 0, H / 2 - 0.001), bevel=0.002, mat=BLACK))
parts.append(box("cam", (0.012, 0.004, 0.006), (0, -T / 2 - 0.0005, H / 2 - 0.006), bevel=0.0015, mat=BLACK))
parts.append(cyl("lens", 0.0018, 0.002, (0, -T / 2 - 0.0026, H / 2 - 0.006), axis='Y', verts=12, mat=METAL))
# left: status LEDs and power key
for z, m in ((0.028, GREEN), (0.020, YELLOW), (0.012, GREEN)):
    parts.append(box("led", (0.010, 0.0025, 0.004), (-0.083, -T / 2 - 0.0006, z), bevel=0.0015, mat=m))
parts.append(box("pwr", (0.014, 0.004, 0.013), (-0.083, -T / 2 - 0.0008, 0.0), bevel=0.003, mat=BLACK))
# right keys
for z in (0.018, -0.004):
    parts.append(box("key", (0.012, 0.004, 0.014), (0.083, -T / 2 - 0.0008, z), bevel=0.003, mat=BLACK))
# screws
for x, z in ((-0.088, 0.046), (-0.088, -0.044), (0.088, -0.044)):
    parts.append(cyl("screw", 0.0022, 0.0014, (x, -T / 2 - 0.0004, z), axis='Y', verts=12, mat=METAL, bevel=0.0004))
# back: label plate, vents
plate = box("plate", (0.13, 0.0015, 0.074), (0.0, T / 2 + 0.0004, 0), bevel=0.001, mat=WHITE)
for i in range(7):
    cut(plate, box("vent", (0.02, 0.008, 0.0026), (0.05, T / 2, 0.018 - i * 0.0055)))
parts.append(plate)
parts.append(box("port", (0.03, 0.008, 0.003), (0, 0, -H / 2 + 0.0035), bevel=0.0008, mat=BLACK))
parts.append(box("usb", (0.004, 0.012, 0.02), (W / 2 + 0.001, 0, 0.0), bevel=0.001, mat=BLACK))
K.finalize(parts, body)
