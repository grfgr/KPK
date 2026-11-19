# ВПК-7М extra geometry (exec'd inside build_v4 before the join)
def bxy(px, py):
    bb = c["backbody"]
    return (W / 2 - (px - bb[0]) / (bb[2] - bb[0]) * W, H / 2 - (py - bb[1]) / (bb[3] - bb[1]) * H)


def brect(r):
    xa, ya = bxy(r[0], r[3]); xb, yb = bxy(r[2], r[1])
    return min(xa, xb), min(ya, yb), max(xa, xb), max(ya, yb)


def screw(cx_, cy_, rad, z, sgn=1):
    parts.append(paint(prism("scr", circle(cx_, cy_, rad, 12), z - sgn * 0.0003 if sgn > 0 else z - 0.0007, z + 0.0007 if sgn > 0 else z + 0.0003, 0.0003, 2), METAL))
    for ang in (45, 135):
        ca, sa = math.cos(math.radians(ang)), math.sin(math.radians(ang)); lw_, ll = rad * 0.13, rad * 0.75
        q = [(cx_ - ca * ll - sa * lw_, cy_ - sa * ll + ca * lw_), (cx_ + ca * ll - sa * lw_, cy_ + sa * ll + ca * lw_),
             (cx_ + ca * ll + sa * lw_, cy_ + sa * ll - ca * lw_), (cx_ - ca * ll + sa * lw_, cy_ - sa * ll - ca * lw_)]
        zt = z + 0.0007 if sgn > 0 else z - 0.0007
        parts.append(paint(prism("scx", q, zt - 0.00015 * sgn if sgn > 0 else zt - 0.0001, zt + 0.0001 if sgn > 0 else zt + 0.00015), TRIM))


# raised armoured frame around the chamfered screen: two steps
for grow, hgt, mat in ((28, 0.0016, BODY), (12, 0.0026, TRIM)):
    ox0, oy0, ox1, oy1 = rect_m(c["screen"], grow)
    ring = prism("frame%d" % grow, scr_shape(ox0, oy0, ox1, oy1, 0.004 + grow * MPP * 0.3), ZF - 0.0004, ZF + hgt, 0.0008, 3, 30)
    cut = prism("fc", scr_shape(sx0 - 0.0016, sy0 - 0.0016, sx1 + 0.0016, sy1 + 0.0016, 0.0041), ZF - 0.01, ZF + 0.01)
    bo = ring.modifiers.new("c", 'BOOLEAN'); bo.operation = 'DIFFERENCE'; bo.object = cut; bo.solver = 'EXACT'
    apply_all(ring); bpy.data.objects.remove(cut)
    parts.append(paint(ring, mat))

# second tier on the corner armour + screw
for r in [(84, 42, 166, 106), (646, 42, 728, 106), (84, 346, 166, 410), (646, 346, 728, 410)]:
    x0, y0, x1, y1 = rect_m((r[0] + 14, r[1] + 12, r[2] - 14, r[3] - 12))
    parts.append(paint(prism("arm2", outline(x0, y0, x1, y1, "chamfer", min(x1 - x0, y1 - y0) * 0.28), ZF + 0.0015, ZF + 0.0042, 0.0007, 2, 30), TRIM))
    cx_, cy_ = (x0 + x1) / 2, (y0 + y1) / 2
    screw(cx_, cy_, 0.0019, ZF + 0.0042)

# mounts of the top handle and of the side handles
for r in [(276, 34, 304, 60), (492, 34, 520, 60)]:
    x0, y0, x1, y1 = rect_m(r)
    parts.append(paint(prism("hm", rounded(x0, y0 - 0.003, x1, y1, 0.0015, 3), -D * 0.22, D * 0.3, 0.0008, 2), TRIM))
mx = 2 * CX
for (a, b, cc, d) in [(64, 80, 90, 106), (64, 360, 90, 386)]:
    for aa, cc2 in ((a, cc), (mx - cc, mx - a)):
        x0, y0, x1, y1 = rect_m((aa, b, cc2, d))
        parts.append(paint(prism("sm", rounded(x0, y0, x1, y1, 0.0015, 3), -D * 0.24, D * 0.24, 0.0008, 2), TRIM))

# feet under the bottom edge
for (a, cc) in [(262, 300), (497, 535)]:
    x0, _ = P(a, 0); x1, _ = P(cc, 0)
    parts.append(paint(prism("foot", rounded(x0, -H / 2 - 0.0022, x1, -H / 2 + 0.004, 0.0015, 3), -D * 0.34, D * 0.34, 0.0008, 2), TRIM))
# service connector on the bottom face: dark block with a row of pins
x0, _ = P(345, 0); x1, _ = P(462, 0)
blk = prism("conn", rounded(x0, -H / 2 - 0.0009, x1, -H / 2 + 0.003, 0.001, 3), -D * 0.14, D * 0.14, 0.0004, 2)
parts.append(paint(blk, TRIM))
for k in range(12):
    px_ = x0 + (x1 - x0) * (k + 0.5) / 12
    parts.append(paint(prism("pin", circle(px_, -H / 2 - 0.0009, 0.0006, 8), -0.0006, 0.0006, 0), METAL))
# pins lie in XY, turn them: tiny round studs pointing down are drawn as short boxes instead
# guard rails of the red switch
for (a, cc) in [(110, 117), (143, 150)]:
    x0, y0, x1, y1 = rect_m((a, 258, cc, 330))
    parts.append(paint(prism("swg", rounded(x0, y0, x1, y1, 0.0006, 2), ZF - 0.0003, ZF + 0.0042, 0.0004, 2), TRIM))

# back: corner screws and the raised centre panel under the stencil
# (inside the back outline: on the corner armour, not on the very corner)
for (sx_, sy_) in [(674, 514), (1031, 514), (674, 685), (1031, 685)]:
    cx_, cy_ = bxy(sx_, sy_)
    screw(cx_, cy_, 0.002, -ZF, -1)
x0, y0, x1, y1 = brect((782, 500, 992, 700))
parts.append(paint(prism("bpan", outline(x0, y0, x1, y1, "chamfer", 0.006), -ZF - 0.0007, -ZF + 0.0004, 0.0006, 2), BODY))
