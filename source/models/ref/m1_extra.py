# КПК-М1 extra geometry (exec'd inside build_v4 before the join; sheet px = front view, back px = back view)
def bxy(px, py):
    bb = c["backbody"]
    return (W / 2 - (px - bb[0]) / (bb[2] - bb[0]) * W, H / 2 - (py - bb[1]) / (bb[3] - bb[1]) * H)


def brect(r):
    xa, ya = bxy(r[0], r[3]); xb, yb = bxy(r[2], r[1])
    return min(xa, xb), min(ya, yb), max(xa, xb), max(ya, yb)


# bumps along the top edge: rubber-capped blocks, through most of the depth
for k, (a, b) in enumerate([(230, 268), (280, 321), (333, 376), (387, 428), (440, 478)]):
    x0, _ = P(a, 0); x1, _ = P(b, 0)
    parts.append(paint(prism("bump%d" % k, rounded(x0, H / 2 - 0.004, x1, H / 2 + 0.0016, 0.0022, 4), -D * 0.36, D * 0.40, 0.0012, 3, 30), TRIM))
# raised bezel around the screen (frame step), the screen glass sits below it
bz0, by0_, bz1, by1_ = rect_m(c["screen"], 10)
ring = prism("bezel", rounded(bz0, by0_, bz1, by1_, 0.006, 6), ZF - 0.0004, ZF + 0.0009, 0.0007, 3, 30)
cut = prism("bc", rounded(sx0 - 0.0016, sy0 - 0.0016, sx1 + 0.0016, sy1 + 0.0016, 0.0041), ZF - 0.01, ZF + 0.01)
bo = ring.modifiers.new("c", 'BOOLEAN'); bo.operation = 'DIFFERENCE'; bo.object = cut; bo.solver = 'EXACT'
apply_all(ring); bpy.data.objects.remove(cut)
parts.append(paint(ring, BODY))
# control panel under the right-hand keys
px0, py0, px1, py1 = rect_m((656, 142, 744, 340))
parts.append(paint(prism("kpanel", rounded(px0, py0, px1, py1, 0.004, 5), ZF - 0.0004, ZF + 0.0008, 0.0006, 3), TRIM))
# LED housing
lx0, ly0, lx1, ly1 = rect_m((96, 96, 123, 153))
parts.append(paint(prism("ledbox", rounded(lx0, ly0, lx1, ly1, 0.003, 5), ZF - 0.0004, ZF + 0.0007, 0.0005, 2), TRIM))
# boss under the lanyard ring
cxr, cyr = P(95, 355)
parts.append(paint(prism("boss", circle(cxr, cyr, 27 * MPP, 24), ZF - 0.0004, ZF + 0.0006, 0.0005, 2), TRIM))
# back: two panels with a groove between them, screws in their corners
for k, r in enumerate([(662, 522, 793, 700), (799, 522, 1036, 700)]):
    x0, y0, x1, y1 = brect(r)
    parts.append(paint(prism("bpan%d" % k, rounded(x0, y0, x1, y1, 0.004, 5), -ZF - 0.0008, -ZF + 0.0004, 0.0006, 3), BODY))
    for sx_, sy_ in ((r[0] + 9, r[1] + 9), (r[2] - 9, r[1] + 9), (r[0] + 9, r[3] - 9), (r[2] - 9, r[3] - 9)):
        cx_, cy_ = bxy(sx_, sy_); rad = 0.0016
        parts.append(paint(prism("bscr", circle(cx_, cy_, rad, 12), -ZF - 0.0014, -ZF - 0.0006, 0.0003, 2), METAL))
        for ang in (0, 90):
            ca, sa = math.cos(math.radians(ang)), math.sin(math.radians(ang)); lw_, ll = rad * 0.13, rad * 0.75
            q = [(cx_ - ca * ll - sa * lw_, cy_ - sa * ll + ca * lw_), (cx_ + ca * ll - sa * lw_, cy_ + sa * ll + ca * lw_),
                 (cx_ + ca * ll + sa * lw_, cy_ + sa * ll - ca * lw_), (cx_ - ca * ll + sa * lw_, cy_ - sa * ll - ca * lw_)]
            parts.append(paint(prism("bscx", q, -ZF - 0.00155, -ZF - 0.0013), TRIM))
# side: slot cover on the left, ribbed grip strip on the right
_, yc = P(CX, 300)
for k in range(5):
    yy = yc + (k - 2) * 0.0035
    parts.append(paint(prism("grip%d" % k, rounded(W / 2 - 0.0008, yy - 0.0008, W / 2 + 0.0009, yy + 0.0008, 0.0006, 2), -D * 0.25, D * 0.25, 0.0003, 1), TRIM))
