# КПК-7 «Вестник» extra geometry (exec'd inside build_v4 before the join)
def bxy(px, py):
    bb = c["backbody"]
    return (W / 2 - (px - bb[0]) / (bb[2] - bb[0]) * W, H / 2 - (py - bb[1]) / (bb[3] - bb[1]) * H)


def brect(r):
    xa, ya = bxy(r[0], r[3]); xb, yb = bxy(r[2], r[1])
    return min(xa, xb), min(ya, yb), max(xa, xb), max(ya, yb)


# rubber strips along the long sides
for (a, cc) in [(94, 108), (404, 418)]:
    x0, y0, x1, y1 = rect_m((a, 220, cc, 540))
    parts.append(paint(prism("rail", rounded(x0, y0, x1, y1, 0.0025, 4), -ZF - 0.0008, ZF + 0.0008, 0.0016, 3, 30), TRIM))
# raised bezel around the screen (frame step)
ox0, oy0, ox1, oy1 = rect_m(c["screen"], 11)
ring = prism("bezel", rounded(ox0, oy0, ox1, oy1, 0.004, 5), ZF - 0.0004, ZF + 0.0016, 0.0007, 3, 30)
cut = prism("bc", rounded(sx0 - 0.0016, sy0 - 0.0016, sx1 + 0.0016, sy1 + 0.0016, 0.0041), ZF - 0.01, ZF + 0.01)
bo = ring.modifiers.new("c", 'BOOLEAN'); bo.operation = 'DIFFERENCE'; bo.object = cut; bo.solver = 'EXACT'
apply_all(ring); bpy.data.objects.remove(cut)
parts.append(paint(ring, BODY))
# antenna: rubber whip on a boss, top left
ax, _ = P(176, 0)
parts.append(paint(prism("aboss", circle(ax, H / 2 + 0.002, 0.0062, 16), -0.0045, 0.0045, 0.0008, 2), TRIM))
bpy.ops.mesh.primitive_cylinder_add(vertices=16, radius=0.0062, depth=0.008, location=(ax, H / 2 + 0.003, 0), rotation=(math.radians(90), 0, 0))
parts.append(paint(bpy.context.active_object, TRIM))
bpy.ops.mesh.primitive_cylinder_add(vertices=14, radius=0.0034, depth=0.046, location=(ax, H / 2 + 0.03, 0), rotation=(math.radians(90), 0, 0))
ant = bpy.context.active_object
m_ = ant.modifiers.new("b", 'BEVEL'); m_.width = 0.0012; m_.segments = 2; apply_all(ant)
parts.append(paint(ant, TRIM))
# connector at the bottom end
x0, _ = P(214, 0); x1, _ = P(298, 0)
parts.append(paint(prism("conn", rounded(x0, -H / 2 - 0.0035, x1, -H / 2 + 0.003, 0.0015, 3), -D * 0.22, D * 0.22, 0.0008, 2), TRIM))
parts.append(paint(prism("slot", rounded(x0 + 0.003, -H / 2 - 0.0037, x1 - 0.003, -H / 2 - 0.002, 0.0006, 2), -D * 0.08, D * 0.08), METAL))
# back: battery cover with screws
x0, y0, x1, y1 = brect((130, 300, 382, 620))
parts.append(paint(prism("batt", rounded(x0, y0, x1, y1, 0.004, 5), -ZF - 0.0008, -ZF + 0.0004, 0.0006, 3), BODY))
for (sx_, sy_) in [(142, 312), (370, 312), (142, 608), (370, 608)]:
    cx_, cy_ = bxy(sx_, sy_)
    parts.append(paint(prism("bscr", circle(cx_, cy_, 0.0017, 12), -ZF - 0.0015, -ZF - 0.0006, 0.0003, 2), METAL))
