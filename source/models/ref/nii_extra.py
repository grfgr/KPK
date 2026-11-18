# НИИ ПИК №042 extra geometry (exec'd inside build_v4 before the join)
def bxy(px, py):
    bb = c["backbody"]
    return (W / 2 - (px - bb[0]) / (bb[2] - bb[0]) * W, H / 2 - (py - bb[1]) / (bb[3] - bb[1]) * H)


def brect(r):
    xa, ya = bxy(r[0], r[3]); xb, yb = bxy(r[2], r[1])
    return min(xa, xb), min(ya, yb), max(xa, xb), max(ya, yb)


def screw(cx_, cy_, rad, z, sgn=1):
    z0_, z1_ = (z - 0.0003, z + 0.0007) if sgn > 0 else (z - 0.0007, z + 0.0003)
    parts.append(paint(prism("scr", circle(cx_, cy_, rad, 12), z0_, z1_, 0.0003, 2), METAL))
    zt = z1_ if sgn > 0 else z0_
    for ang in (45, 135):
        ca, sa = math.cos(math.radians(ang)), math.sin(math.radians(ang)); lw_, ll = rad * 0.13, rad * 0.75
        q = [(cx_ - ca * ll - sa * lw_, cy_ - sa * ll + ca * lw_), (cx_ + ca * ll - sa * lw_, cy_ + sa * ll + ca * lw_),
             (cx_ + ca * ll + sa * lw_, cy_ + sa * ll - ca * lw_), (cx_ - ca * ll + sa * lw_, cy_ - sa * ll - ca * lw_)]
        parts.append(paint(prism("scx", q, zt - 0.00012, zt + 0.00012), TRIM))


# black inner bezel around the screen
ox0, oy0, ox1, oy1 = rect_m(c["screen"], 14)
ring = prism("bezel", rounded(ox0, oy0, ox1, oy1, 0.0065, 6), ZF - 0.0004, ZF + 0.0013, 0.0007, 3, 30)
cut = prism("bc", rounded(sx0 - 0.0016, sy0 - 0.0016, sx1 + 0.0016, sy1 + 0.0016, 0.0041), ZF - 0.01, ZF + 0.01)
bo = ring.modifiers.new("c", 'BOOLEAN'); bo.operation = 'DIFFERENCE'; bo.object = cut; bo.solver = 'EXACT'
apply_all(ring); bpy.data.objects.remove(cut)
parts.append(paint(ring, TRIM))

# rubber rails along the short sides (wrap the side face)
for (a, cc) in [(36, 60), (736, 760)]:
    x0, y0, x1, y1 = rect_m((a, 128, cc, 372))
    parts.append(paint(prism("rail", rounded(x0, y0, x1, y1, 0.003, 4), -ZF - 0.0009, ZF + 0.0009, 0.0018, 3, 30), TRIM))
    # three grip ribs on each rail front
    for k in range(3):
        _, yy = P(0, 200 + k * 50)
        parts.append(paint(prism("rib", rounded(x0 + 0.001, yy - 0.0012, x1 - 0.001, yy + 0.0012, 0.0008, 2), ZF + 0.0007, ZF + 0.0016, 0.0003, 1), TRIM))

# black housings: LEDs, power key, right keys, camera
for r in [(87, 131, 130, 198), (84, 211, 132, 271), (692, 147, 738, 223), (692, 227, 738, 303), (392, 56, 427, 83)]:
    x0, y0, x1, y1 = rect_m(r)
    parts.append(paint(prism("hous", rounded(x0, y0, x1, y1, 0.0025, 4), ZF - 0.0004, ZF + 0.0009, 0.0005, 2), TRIM))

# second tier on the corner bumpers + a screw in each
for r in [(40, 50, 106, 116), (688, 50, 754, 116), (40, 384, 106, 448), (688, 384, 754, 448)]:
    x0, y0, x1, y1 = rect_m((r[0] + 6, r[1] + 6, r[2] - 6, r[3] - 6))
    g = 0.0007
    grown = outline(-W / 2 - g, -H / 2 - g, W / 2 + g, H / 2 + g, kind, RAD + g)
    poly = clip(grown, x0, y0, x1, y1)
    if len(poly) >= 3:
        parts.append(paint(prism("bump2", poly, -ZF - 0.0034, ZF + 0.0036, 0.0016, 3, 30), TRIM))

# D-rings on the top clips
for (a, cc) in [(262, 330), (520, 590)]:
    lp = tube("dring", [(a + 6, 44), (a + 6, 30), (a + 14, 24), (cc - 14, 24), (cc - 6, 30), (cc - 6, 44)], 0.0011, 0.0)
    parts.append(paint(lp, METAL))

# back: raised panel seam + corner screws
x0, y0, x1, y1 = brect((688, 528, 1040, 712))
parts.append(paint(prism("bpan", rounded(x0, y0, x1, y1, 0.004, 5), -ZF - 0.0006, -ZF + 0.0004, 0.0005, 2), BODY))
for (sx_, sy_) in [(700, 540), (1028, 540), (700, 700), (1028, 700)]:
    cx_, cy_ = bxy(sx_, sy_)
    screw(cx_, cy_, 0.0018, -ZF - 0.0006, -1)
# front: fourth screw (top right)
screw(*P(712, 106), 8 * MPP * 0.8, ZF)
# bottom: grip slot block
x0, _ = P(380, 0); x1, _ = P(500, 0)
parts.append(paint(prism("slot", rounded(x0, -H / 2 - 0.0008, x1, -H / 2 + 0.003, 0.001, 3), -D * 0.16, D * 0.16, 0.0004, 2), TRIM))
