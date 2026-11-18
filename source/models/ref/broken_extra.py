# broken КПК-7 extra geometry: the brick, smashed (exec'd inside build_v4 before the join)
def bxy(px, py):
    bb = c["backbody"]
    return (W / 2 - (px - bb[0]) / (bb[2] - bb[0]) * W, H / 2 - (py - bb[1]) / (bb[3] - bb[1]) * H)


def brect(r):
    xa, ya = bxy(r[0], r[3]); xb, yb = bxy(r[2], r[1])
    return min(xa, xb), min(ya, yb), max(xa, xb), max(ya, yb)


# dents: spheres pressed into the case (front corner, side, back)
for (px_, py_, z_, rad) in [(392, 600, ZF + 0.0072, 0.0085), (412, 300, 0.0, 0.0075), (150, 230, -ZF - 0.0068, 0.009), (260, 662, ZF * 0.3, 0.0070)]:
    x_, y_ = P(px_, py_)
    if px_ >= 410:
        x_ = W / 2 + 0.0062
    if py_ >= 660:
        y_ = -H / 2 - 0.0058
    bpy.ops.mesh.primitive_uv_sphere_add(segments=16, ring_count=10, radius=rad, location=(x_, y_, z_))
    sp_ = bpy.context.active_object
    bo = case.modifiers.new("dent", 'BOOLEAN'); bo.operation = 'DIFFERENCE'; bo.object = sp_; bo.solver = 'EXACT'
    apply_all(case); bpy.data.objects.remove(sp_)
# the new dent faces need material attributes like the rest of the case
am_ = case.data.attributes["mat"]; ad_ = case.data.attributes["dec"]

# rubber strips: left whole, right torn (short piece left)
for (a, cc, y0p, y1p) in [(94, 108, 220, 540), (404, 418, 220, 330)]:
    x0, y0, x1, y1 = rect_m((a, y0p, cc, y1p))
    parts.append(paint(prism("rail", rounded(x0, y0, x1, y1, 0.0025, 4), -ZF - 0.0008, ZF + 0.0008, 0.0016, 3, 30), TRIM))
# bezel around the screen
ox0, oy0, ox1, oy1 = rect_m(c["screen"], 11)
ring = prism("bezel", rounded(ox0, oy0, ox1, oy1, 0.004, 5), ZF - 0.0004, ZF + 0.0016, 0.0007, 3, 30)
cut = prism("bc", rounded(sx0 - 0.0016, sy0 - 0.0016, sx1 + 0.0016, sy1 + 0.0016, 0.0041), ZF - 0.01, ZF + 0.01)
bo = ring.modifiers.new("c", 'BOOLEAN'); bo.operation = 'DIFFERENCE'; bo.object = cut; bo.solver = 'EXACT'
apply_all(ring); bpy.data.objects.remove(cut)
parts.append(paint(ring, BODY))
# antenna snapped off: boss and a bent stub
ax, _ = P(176, 0)
bpy.ops.mesh.primitive_cylinder_add(vertices=16, radius=0.0062, depth=0.008, location=(ax, H / 2 + 0.003, 0), rotation=(math.radians(90), 0, 0))
parts.append(paint(bpy.context.active_object, TRIM))
bpy.ops.mesh.primitive_cylinder_add(vertices=12, radius=0.0034, depth=0.013, location=(ax + 0.002, H / 2 + 0.012, 0.001), rotation=(math.radians(90), math.radians(18), 0))
parts.append(paint(bpy.context.active_object, TRIM))
# empty key sockets where keys were knocked out (dark holes in the pad)
for (cx, cy) in [(256, 492), (192, 580), (320, 536), (336, 404)]:
    hw = 26 if cy != 404 else 34; hh = 18 if cy != 404 else 13
    x0, y0, x1, y1 = rect_m((cx - hw + 4, cy - hh + 4, cx + hw - 4, cy + hh - 4))
    parts.append(paint(prism("hole", rounded(x0, y0, x1, y1, 0.0012, 3), ZF + 0.0002, ZF + 0.00065), TRIM))
# connector, battery cover (one screw missing)
x0, _ = P(214, 0); x1, _ = P(298, 0)
parts.append(paint(prism("conn", rounded(x0, -H / 2 - 0.0035, x1, -H / 2 + 0.003, 0.0015, 3), -D * 0.22, D * 0.22, 0.0008, 2), TRIM))
x0, y0, x1, y1 = brect((130, 300, 382, 620))
parts.append(paint(prism("batt", rounded(x0, y0, x1, y1, 0.004, 5), -ZF - 0.0008, -ZF + 0.0004, 0.0006, 3), BODY))
for (sx_, sy_) in [(142, 312), (370, 312), (142, 608)]:
    cx_, cy_ = bxy(sx_, sy_)
    parts.append(paint(prism("bscr", circle(cx_, cy_, 0.0017, 12), -ZF - 0.0015, -ZF - 0.0006, 0.0003, 2), METAL))

# cracks: zigzag grooves cut into the case (front from the dented corner, back across the cover)
def crack(pts_px, z_face, depth, width=0.0007):
    for (a_, b_) in zip(pts_px[:-1], pts_px[1:]):
        xa, ya = P(*a_); xb, yb = P(*b_)
        if z_face < 0:
            xa, ya = bxy(*a_); xb, yb = bxy(*b_)
        dx, dy = xb - xa, yb - ya; L_ = (dx * dx + dy * dy) ** 0.5 or 1e-6
        nx, ny = -dy / L_ * width, dx / L_ * width
        q = [(xa - nx, ya - ny), (xb - nx, yb - ny), (xb + nx, yb + ny), (xa + nx, ya + ny)]
        z0_, z1_ = (z_face - depth, z_face + 0.01) if z_face > 0 else (z_face - 0.01, z_face + depth)
        cut_ = prism("crk", q, z0_, z1_)
        bo = case.modifiers.new("crk", 'BOOLEAN'); bo.operation = 'DIFFERENCE'; bo.object = cut_; bo.solver = 'EXACT'
        apply_all(case); bpy.data.objects.remove(cut_)


crack([(410, 596), (392, 584), (384, 560), (370, 548), (362, 520), (350, 506)], ZF, 0.0012)
crack([(112, 280), (140, 300), (156, 296), (178, 318), (204, 322), (230, 346), (252, 350)], -ZF, 0.0010)
for p in case.data.polygons:
    if case.data.attributes["mat"].data[p.index].value not in (BODY, PHOTO):
        case.data.attributes["mat"].data[p.index].value = BODY
