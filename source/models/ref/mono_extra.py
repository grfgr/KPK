# Монолит ПИК №ОС-031 extra geometry (exec'd inside build_v4 before the join)
def bxy(px, py):
    bb = c["backbody"]
    return (W / 2 - (px - bb[0]) / (bb[2] - bb[0]) * W, H / 2 - (py - bb[1]) / (bb[3] - bb[1]) * H)


def brect(r):
    xa, ya = bxy(r[0], r[3]); xb, yb = bxy(r[2], r[1])
    return min(xa, xb), min(ya, yb), max(xa, xb), max(ya, yb)


def screw(cx_, cy_, rad, z, sgn=1):
    z0_, z1_ = (z - 0.0003, z + 0.0007) if sgn > 0 else (z - 0.0007, z + 0.0003)
    parts.append(paint(prism("scr", circle(cx_, cy_, rad, 12), z0_, z1_, 0.0003, 2), METAL))


# massive two-step frame around the chamfered screen
for grow, hgt, mat in ((26, 0.0018, BODY), (10, 0.0028, TRIM)):
    ox0, oy0, ox1, oy1 = rect_m(c["screen"], grow)
    ring = prism("frame%d" % grow, scr_shape(ox0, oy0, ox1, oy1, 0.004 + grow * MPP * 0.3), ZF - 0.0004, ZF + hgt, 0.0009, 3, 30)
    cut = prism("fc", scr_shape(sx0 - 0.0016, sy0 - 0.0016, sx1 + 0.0016, sy1 + 0.0016, 0.0041), ZF - 0.01, ZF + 0.01)
    bo = ring.modifiers.new("c", 'BOOLEAN'); bo.operation = 'DIFFERENCE'; bo.object = cut; bo.solver = 'EXACT'
    apply_all(ring); bpy.data.objects.remove(cut)
    parts.append(paint(ring, mat))

# segmented blocks on the top and bottom edges (wrap over the edge)
for (a, cc) in [(122, 228), (240, 346), (358, 464), (476, 582)]:
    x0, _ = P(a, 0); x1, _ = P(cc, 0)
    parts.append(paint(prism("tseg", outline(x0, H / 2 - 0.006, x1, H / 2 + 0.0014, "chamfer", 0.002), -ZF - 0.0008, ZF + 0.0012, 0.0008, 2, 30), TRIM))
for (a, cc) in [(130, 316), (444, 600)]:
    x0, _ = P(a, 0); x1, _ = P(cc, 0)
    parts.append(paint(prism("bseg", outline(x0, -H / 2 - 0.0014, x1, -H / 2 + 0.005, "chamfer", 0.002), -ZF - 0.0008, ZF + 0.0012, 0.0008, 2, 30), TRIM))

# key column plate
x0, y0, x1, y1 = rect_m((612, 142, 678, 372))
parts.append(paint(prism("kcol", outline(x0, y0, x1, y1, "chamfer", 0.003), ZF - 0.0004, ZF + 0.0009, 0.0006, 2, 30), TRIM))

# rosary: beads hanging along the left side, dog tag at the end
import math as _m
pts_ = []
for k in range(34):
    t = k / 33.0
    pts_.append((62 - 14 * _m.sin(t * _m.pi) , 58 + 250 * t))
for k, (bx_, by_) in enumerate(pts_):
    x_, y_ = P(bx_, by_)
    bpy.ops.mesh.primitive_uv_sphere_add(segments=8, ring_count=5, radius=0.0021 if k % 6 else 0.0028, location=(x_, y_, ZF * 0.55))
    parts.append(paint(bpy.context.active_object, TRIM))
tx0, ty0, tx1, ty1 = rect_m((34, 300, 70, 352))
tag = prism("tag", outline(tx0, ty0, tx1, ty1, "chamfer", 0.002), ZF * 0.4, ZF * 0.4 + 0.0012, 0.0004, 2)
parts.append(paint(tag, METAL))

# cyan indicator slot on the left side face
_, yc = P(0, 300)
for k in range(3):
    yy = yc + (k - 1) * 0.004
    parts.append(paint(prism("sled", rounded(-W / 2 - 0.0009, yy - 0.0012, -W / 2 + 0.0005, yy + 0.0012, 0.0005, 2), -D * 0.12, D * 0.12, 0.0002, 1), TRIM))

# more front screws
for (sx_, sy_) in [(110, 70), (110, 388), (590, 70), (590, 388)]:
    screw(*P(sx_, sy_), 7 * MPP * 0.8, ZF)

# back: raised emblem panel and two crossing cloth strips over the right part
x0, y0, x1, y1 = brect((792, 505, 990, 690))
parts.append(paint(prism("bpan", outline(x0, y0, x1, y1, "chamfer", 0.005), -ZF - 0.0006, -ZF + 0.0004, 0.0005, 2), BODY))
for kk, (p0, p1) in enumerate([((925, 478), (1045, 712)), ((1045, 478), (930, 712))]):
    xa, ya = bxy(*p0); xb, yb = bxy(*p1)
    dx, dy = xb - xa, yb - ya; L_ = (dx * dx + dy * dy) ** 0.5; nx, ny = -dy / L_ * 0.006, dx / L_ * 0.006
    q = [(xa - nx, ya - ny), (xb - nx, yb - ny), (xb + nx, yb + ny), (xa + nx, ya + ny)]
    parts.append(paint(prism("bcl", q, -ZF - 0.0016 - kk * 0.0012, -ZF + 0.002, 0.0008, 2), CLOTH))
