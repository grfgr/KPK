# ВПК-7М (ОКСОП): armoured olive military PDA, 208 x 126 x 36 mm
import sys, math, bpy
sys.path.insert(0, sys.path[0] if sys.path[0] else ".")
import kpklib as K
out, res = sys.argv[sys.argv.index("--") + 1], int(sys.argv[sys.argv.index("--") + 2])
W, H, T = 0.208, 0.126, 0.036
SX, SZ, SW, SH = 0.0, 0.004, 0.120, 0.068
K.init(out, res, W, H, T, (SX - SW / 2, SW, SZ - SH / 2, SH))
OLIVE = K.mat_paint("olive", (0.075, 0.072, 0.040, 1))
DARK = K.mat_steel("dark", (0.030, 0.030, 0.027, 1))
RUBBER = K.mat_rubber()
METAL = K.mat_metal()
SCREEN = K.mat_screen()
CORD = K.mat_fabric("cord", (0.11, 0.10, 0.05, 1), 1400)
STRAP = K.mat_fabric("strap", (0.07, 0.065, 0.03, 1), 900)
MESH = K.mat_fabric("mesh", (0.05, 0.045, 0.025, 1), 2200)
RED = K.mat_glow("red", (0.55, 0.06, 0.03, 1))
box, cyl, cut = K.box, K.cyl, K.cut
parts = []
body = box("body", (W - 0.016, T - 0.006, H - 0.014), (0, 0, 0), bevel=0.006, segs=3, mat=OLIVE)
cut(body, box("c_scr", (SW + 0.004, 0.012, SH + 0.004), (SX, -T / 2, SZ)))
parts.append(body)
ring = box("ring", (SW + 0.02, 0.006, SH + 0.02), (SX, -T / 2 + 0.002, SZ), bevel=0.002, mat=DARK)
cut(ring, box("c_ring", (SW + 0.002, 0.03, SH + 0.002), (SX, -T / 2, SZ)))
parts.append(ring)
parts.append(box("glass", (SW + 0.003, 0.001, SH + 0.003), (SX, -T / 2 + 0.004, SZ), mat=SCREEN))
# armoured corner blocks and edge plates
for sx in (-1, 1):
    for sz in (-1, 1):
        parts.append(box("corner", (0.046, T, 0.036), (sx * (W / 2 - 0.026), 0, sz * (H / 2 - 0.02)), bevel=0.005, segs=2, mat=DARK))
        parts.append(K.cyl("bolt", 0.0026, 0.002, (sx * (W / 2 - 0.012), -T / 2 - 0.0008, sz * (H / 2 - 0.01)), axis='Y', verts=6, mat=METAL))
for sz in (-1, 1):
    parts.append(box("edge", (0.08, T - 0.004, 0.012), (0, 0, sz * (H / 2 - 0.006)), bevel=0.003, segs=2, mat=OLIVE))
# side handles with cord-wrapped grips
for sx in (-1, 1):
    x = sx * (W / 2 + 0.006)
    parts.append(cyl("hbar", 0.0042, 0.070, (x, 0, 0), verts=12, mat=DARK, bevel=0.001))
    for sz in (-1, 1):
        c = cyl("hpost", 0.0038, 0.014, (x - sx * 0.006, 0, sz * 0.035), verts=12, mat=DARK)
        c.rotation_euler = (0, math.radians(90), 0); bpy.ops.object.transform_apply(rotation=True); parts.append(c)
    parts.append(cyl("grip", 0.0072, 0.044, (x, 0, 0), verts=16, mat=CORD))
# top carry handle and latches
parts.append(cyl("thandle", 0.0035, 0.07, (0, 0, H / 2 + 0.010), verts=12, mat=DARK))
bpy.context.active_object.rotation_euler = (0, math.radians(90), 0); bpy.ops.object.transform_apply(rotation=True)
for sx in (-1, 1):
    parts.append(cyl("tpost", 0.0032, 0.012, (sx * 0.035, 0, H / 2 + 0.004), verts=10, mat=DARK))
    parts.append(box("latch", (0.022, 0.012, 0.008), (sx * 0.062, 0, H / 2 - 0.001), bevel=0.0015, mat=DARK))
# left column: power, brightness, guarded red switch
for z in (0.028, 0.010):
    parts.append(box("btn", (0.016, 0.004, 0.014), (-0.073, -T / 2 - 0.0005, z), bevel=0.002, mat=DARK))
parts.append(box("guard", (0.02, 0.004, 0.026), (-0.073, -T / 2 - 0.0005, -0.016), bevel=0.0015, mat=DARK))
parts.append(box("switch", (0.011, 0.005, 0.018), (-0.073, -T / 2 - 0.0016, -0.016), bevel=0.0012, mat=RED))
# right: d-pad and menu key
parts.append(box("dpadwell", (0.03, 0.003, 0.03), (0.073, -T / 2 - 0.0002, 0.022), bevel=0.003, mat=RUBBER))
for dx, dz in ((0, 0.009), (0, -0.009), (-0.009, 0), (0.009, 0), (0, 0)):
    parts.append(box("dpad", (0.008, 0.004, 0.008), (0.073 + dx, -T / 2 - 0.0012, 0.022 + dz), bevel=0.0012, mat=DARK))
parts.append(box("menu", (0.018, 0.004, 0.016), (0.073, -T / 2 - 0.0005, -0.008), bevel=0.002, mat=DARK))
# speaker grilles at the bottom
for sx in (-1, 1):
    parts.append(box("grille", (0.036, 0.003, 0.008), (sx * 0.042, -T / 2 - 0.0002, -0.050), bevel=0.001, mat=MESH))
# back: plate, vents, strap with a buckle, service connector at the bottom
plate = box("plate", (0.12, 0.002, 0.08), (0.01, T / 2 + 0.0008, 0), bevel=0.0015, mat=OLIVE)
for i in range(7):
    cut(plate, box("vent", (0.022, 0.008, 0.0028), (0.052, T / 2, 0.018 - i * 0.006)))
parts.append(plate)
parts.append(box("strap", (0.022, 0.004, H - 0.012), (-0.062, T / 2 + 0.003, 0), bevel=0.0008, mat=STRAP))
buckle = box("buckle", (0.028, 0.006, 0.01), (-0.062, T / 2 + 0.004, 0.012), bevel=0.001, mat=METAL)
cut(buckle, box("c_buckle", (0.022, 0.02, 0.005), (-0.062, T / 2 + 0.004, 0.012)))
parts.append(buckle)
parts.append(box("conn", (0.04, 0.008, 0.003), (0, 0, -H / 2 + 0.0004), bevel=0.0008, mat=DARK))
parts.append(box("usb", (0.004, 0.012, 0.02), (W / 2 - 0.004, 0, 0.0), bevel=0.001, mat=RUBBER))
K.finalize(parts, body)
