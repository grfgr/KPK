# Broken KPK: the brick, burnt and smashed (snapped antenna, cracked screen, soot)
import sys, math, bpy
sys.path.insert(0, sys.path[0] if sys.path[0] else ".")
import kpklib as K
out, res = sys.argv[sys.argv.index("--") + 1], int(sys.argv[sys.argv.index("--") + 2])
W, L, T = 0.078, 0.140, 0.028
K.init(out, res, W, L, T, (-0.028, 0.056, 0.008, 0.048))
box, cyl, cut = K.box, K.cyl, K.cut
PAINT = K.mat_paint("paint", (0.055, 0.060, 0.036, 1))
BEZEL = K.mat_paint("bezel", (0.035, 0.036, 0.030, 1))
RUBBER = K.mat_rubber()
METAL = K.mat_metal()
SCREEN = K.mat_screen()
LED = K.mat_rubber_c("deadled", (0.03, 0.03, 0.03, 1), (0.05, 0.05, 0.05, 1))
parts = []
body = box("body", (W, T, L), (0, 0, 0), bevel=0.004, segs=3, mat=PAINT)
cut(body, box("c_screen", (0.058, 0.006, 0.050), (0, -T / 2, 0.032)))
cut(body, box("c_keys", (0.060, 0.003, 0.058), (0, -T / 2, -0.030)))
parts.append(body)

# raised bezel ring around the screen
ring = box("bezel", (0.066, 0.0024, 0.058), (0, -T / 2 - 0.0006, 0.032), bevel=0.0008, mat=BEZEL)
cut(ring, box("c_ring", (0.058, 0.01, 0.050), (0, -T / 2, 0.032)))
parts.append(ring)

glass = box("glass", (0.0566, 0.001, 0.0486), (0, -T / 2 + 0.0026, 0.032), mat=SCREEN)
parts.append(glass)

# rubber corner bumpers
for sx in (-1, 1):
    for sz in (-1, 1):
        parts.append(box("bump", (0.017, T + 0.003, 0.017), (sx * (W / 2 - 0.0065), 0, sz * (L / 2 - 0.0065)), bevel=0.003, segs=2, mat=RUBBER))

# side rubber grips
for sx in (-1, 1):
    parts.append(box("grip", (0.004, 0.018, 0.074), (sx * (W / 2 + 0.0006), 0, 0), bevel=0.0012, mat=RUBBER))

# keys: function row + 3x4 keypad
for i in range(3):
    parts.append(box("fkey", (0.016, 0.003, 0.006), (-0.020 + i * 0.020, -T / 2, -0.006), bevel=0.0012, mat=RUBBER))
for i in range(12):
    c, r = i % 3, i // 3
    parts.append(box("key", (0.012, 0.003, 0.008), (-0.016 + c * 0.016, -T / 2, -0.017 - r * 0.011), bevel=0.0013, mat=RUBBER))

# LED
parts.append(cyl("led", 0.0018, 0.0016, (-0.032, -T / 2 - 0.0002, 0.0635), axis='Y', verts=12, mat=LED))

# antenna, connector on the top end
parts.append(cyl("ant_base", 0.0058, 0.005, (-0.020, 0, L / 2 + 0.0015), verts=6, mat=METAL, bevel=0.0006))
# antenna snapped off: a short jagged stub
stub = cyl("antenna", 0.0034, 0.012, (-0.020, 0, L / 2 + 0.010), verts=7, mat=RUBBER)
stub.rotation_euler = (0.25, 0.15, 0); bpy.ops.object.transform_apply(rotation=True)
parts.append(stub)
parts.append(cyl("ant_ring", 0.0039, 0.0025, (-0.020, 0, L / 2 + 0.008), verts=14, mat=RUBBER))
parts.append(cyl("conn", 0.0062, 0.006, (0.016, 0, L / 2 + 0.002), verts=16, mat=METAL, bevel=0.0006))
parts.append(cyl("conn_cap", 0.0068, 0.003, (0.016, 0, L / 2 + 0.0055), verts=16, mat=RUBBER, bevel=0.0008))

# strap lug at the bottom
lug = box("lug", (0.022, 0.007, 0.009), (0, 0.004, -L / 2 - 0.0035), bevel=0.0015, mat=METAL)
cut(lug, box("c_lug", (0.016, 0.02, 0.0045), (0, 0.004, -L / 2 - 0.0045)))
parts.append(lug)

# battery hatch with screws on the back
parts.append(box("hatch", (0.056, 0.0018, 0.082), (0, T / 2 + 0.0005, -0.004), bevel=0.0008, mat=PAINT))
for sx in (-1, 1):
    for sz in (-1, 1):
        parts.append(cyl("screw", 0.0022, 0.0012, (sx * 0.023, T / 2 + 0.0015, -0.004 + sz * 0.035), axis='Y', verts=12, mat=METAL, bevel=0.0004))


K.finalize(parts, body)
