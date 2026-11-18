# ПИК №ОС-031 (Монолит): worn grey stone-like case, cloth wrap, bead chain with a tag, 182 x 108 x 30 mm
import sys, math, bpy
sys.path.insert(0, sys.path[0] if sys.path[0] else ".")
import kpklib as K
out, res = sys.argv[sys.argv.index("--") + 1], int(sys.argv[sys.argv.index("--") + 2])
W, H, T = 0.182, 0.108, 0.030
SX, SZ, SW, SH = -0.008, 0.003, 0.120, 0.070
K.init(out, res, W, H, T, (SX - SW / 2, SW, SZ - SH / 2, SH))
STONE = K.mat_steel("stone", (0.085, 0.080, 0.070, 1))
DARK = K.mat_steel("darkstone", (0.040, 0.037, 0.033, 1))
METAL = K.mat_metal()
SCREEN = K.mat_screen()
CLOTH = K.mat_fabric("cloth", (0.045, 0.036, 0.026, 1), 600)
BEAD = K.mat_metal()
CYAN = K.mat_glow("cyan", (0.25, 0.85, 0.85, 1))
box, cyl, cut = K.box, K.cyl, K.cut
parts = []
body = box("body", (W, T, H), (0, 0, 0), bevel=0.008, segs=3, mat=STONE)
cut(body, box("c_scr", (SW + 0.004, 0.01, SH + 0.004), (SX, -T / 2, SZ)))
parts.append(body)
ring = box("ring", (SW + 0.02, 0.005, SH + 0.02), (SX, -T / 2 - 0.0005, SZ), bevel=0.002, mat=DARK)
cut(ring, box("c_ring", (SW + 0.002, 0.03, SH + 0.002), (SX, -T / 2, SZ)))
parts.append(ring)
parts.append(box("glass", (SW + 0.003, 0.001, SH + 0.003), (SX, -T / 2 + 0.003, SZ), mat=SCREEN))
# chunky frame segments
for sz in (-1, 1):
    for sx in (-1, 0, 1):
        parts.append(box("seg", (0.04, T + 0.002, 0.012), (SX + sx * 0.048, 0, sz * (H / 2 - 0.004)), bevel=0.003, mat=STONE))
for sx in (-1, 1):
    for sz in (-1, 1):
        parts.append(box("corner", (0.03, T + 0.003, 0.03), (sx * (W / 2 - 0.013), 0, sz * (H / 2 - 0.013)), bevel=0.005, mat=DARK))
# right buttons, power glowing
for z in (0.030, 0.012, -0.006):
    parts.append(box("btn", (0.014, 0.004, 0.013), (0.062, -T / 2 - 0.0006, z), bevel=0.002, mat=DARK))
parts.append(box("power", (0.014, 0.004, 0.013), (0.062, -T / 2 - 0.0006, -0.026), bevel=0.002, mat=CYAN))
# glowing strip at the bottom and a small indicator top left
parts.append(box("strip", (0.03, 0.002, 0.003), (SX, -T / 2 - 0.0005, -0.049), bevel=0.0008, mat=CYAN))
parts.append(box("ind", (0.006, 0.002, 0.004), (-0.08, -T / 2 - 0.0005, 0.044), bevel=0.001, mat=CYAN))
# cloth wrapped around the right end (front and back), slightly crooked
# one straight band: crossing boxes overlapped and baked black patches
parts.append(box("wrap", (0.022, T + 0.004, H + 0.004), (0.081, 0, 0), bevel=0.003, segs=3, mat=CLOTH))
# bead chain hanging on the left with a tag
for i in range(16):
    t = i / 15.0
    x = -W / 2 - 0.004 - 0.006 * math.sin(t * math.pi)
    z = 0.045 - t * 0.075
    bpy.ops.mesh.primitive_uv_sphere_add(radius=0.0028, segments=10, ring_count=6, location=(x, -T / 2 + 0.004, z))
    b = bpy.context.active_object; b.data.materials.append(BEAD); parts.append(b)
parts.append(box("tag", (0.012, 0.002, 0.018), (-W / 2 - 0.003, -T / 2 + 0.003, -0.039), bevel=0.0012, mat=METAL))
# back: plate with vents
plate = box("plate", (0.12, 0.002, 0.075), (-0.01, T / 2 + 0.0006, 0), bevel=0.0015, mat=STONE)
for i in range(6):
    cut(plate, box("vent", (0.022, 0.008, 0.0028), (-0.052, T / 2, 0.015 - i * 0.006)))
parts.append(plate)
parts.append(box("usb", (0.004, 0.012, 0.02), (W / 2, 0, 0.0), bevel=0.001, mat=DARK))
K.finalize(parts, body)
