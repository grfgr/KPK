# v3 (Blender): KPK modelled from scratch after the concept sheet. Case with rounded rims, stepped
# screen recess, glass, bumpers, real buttons, ports, clips, handles, straps, plates; every part has
# its own material: tiling body / trim / cloth / metal / weave, and "photo" for screen, back panel,
# button tops, plates and printed decals. Writes mesh.json (+materials), mesh_lod1.json, kpk.blend.
# usage: blender python build_v3.py -- <name> <out dir>
import bpy, bmesh, sys, json, math, os
from mathutils import Vector
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from cfg_v4 import CFG

args = sys.argv[sys.argv.index("--") + 1:]
name, out = args[0], args[1]
c = CFG[name]
REG = json.load(open(out + "/v3.json"))["regions"]
bpy.ops.wm.read_factory_settings(use_empty=True)

SLOTS = ["body", "trim", "photo", "metal", "weave", "cloth"]
BODY, TRIM, PHOTO, METAL, WEAVE, CLOTH = range(6)
bx0, by0, bx1, by1 = c["body"]
MPP = c["W"] / 1000.0 / (bx1 - bx0)
CX, CY = (bx0 + bx1) / 2.0, (by0 + by1) / 2.0
W = c["W"] / 1000.0; H = (by1 - by0) * MPP; D = c["D"] / 1000.0; ZF = D / 2
kind, rpx = c["corner"]
RAD = max(rpx * MPP, 0.009)
DEC = []   # photo regions: (atlas key, sheet box, "front" | "back")


def P(px, py):
    return ((px - CX) * MPP, (CY - py) * MPP)


def rect_m(r, grow=0):
    a, b, cc, d = r
    x0, y0 = P(a - grow, d + grow); x1, y1 = P(cc + grow, b - grow)
    return x0, y0, x1, y1


def outline(x0, y0, x1, y1, kind_, r, steps=7):
    pts = []
    for cxx, cyy, a0 in ((x1, y0, -90), (x1, y1, 0), (x0, y1, 90), (x0, y0, 180)):
        sx = -1 if cxx == x1 else 1; sy = -1 if cyy == y1 else 1
        ox, oy = cxx + sx * r, cyy + sy * r
        st = 1 if kind_ == "chamfer" else steps
        for i in range(st + 1):
            a = math.radians(a0 + 90.0 * i / st)
            pts.append((ox + r * math.cos(a), oy + r * math.sin(a)))
    return pts


def rounded(x0, y0, x1, y1, r, steps=5):
    r = min(r, (x1 - x0) / 2 * 0.99, (y1 - y0) / 2 * 0.99)
    return outline(x0, y0, x1, y1, "round", r, steps)


def circle(cx_, cy_, rad, n=16):
    return [(cx_ + rad * math.cos(2 * math.pi * i / n), cy_ + rad * math.sin(2 * math.pi * i / n)) for i in range(n)]


def clip(poly, x0, y0, x1, y1):
    def cut(pts, inside, inter):
        o = []
        for i in range(len(pts)):
            a, b = pts[i - 1], pts[i]
            if inside(b):
                if not inside(a): o.append(inter(a, b))
                o.append(b)
            elif inside(a):
                o.append(inter(a, b))
        return o
    ix = lambda a, b, x: (x, a[1] + (x - a[0]) / (b[0] - a[0]) * (b[1] - a[1]))
    iy = lambda a, b, y: (a[0] + (y - a[1]) / (b[1] - a[1]) * (b[0] - a[0]), y)
    p = list(poly)
    for f in (lambda q: cut(q, lambda v: v[0] >= x0, lambda a, b: ix(a, b, x0)), lambda q: cut(q, lambda v: v[0] <= x1, lambda a, b: ix(a, b, x1)),
              lambda q: cut(q, lambda v: v[1] >= y0, lambda a, b: iy(a, b, y0)), lambda q: cut(q, lambda v: v[1] <= y1, lambda a, b: iy(a, b, y1))):
        if p: p = f(p)
    return p


def link(me, nm):
    ob = bpy.data.objects.new(nm, me); bpy.context.scene.collection.objects.link(ob); return ob


def apply_all(ob):
    bpy.context.view_layer.objects.active = ob
    for m in list(ob.modifiers):
        bpy.ops.object.modifier_apply(modifier=m.name)


def prism(nm, pts, z0, z1, bevel=0.0, segs=3, angle=35):
    me = bpy.data.meshes.new(nm); bm = bmesh.new()
    f = bm.faces.new([bm.verts.new((x, y, z0)) for x, y in pts])
    bmesh.ops.recalc_face_normals(bm, faces=[f])
    if f.normal.z > 0: f.normal_flip()
    ext = bmesh.ops.extrude_face_region(bm, geom=[f])
    bmesh.ops.translate(bm, verts=[v for v in ext["geom"] if isinstance(v, bmesh.types.BMVert)], vec=(0, 0, z1 - z0))
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces[:])
    bm.to_mesh(me); bm.free()
    ob = link(me, nm)
    if bevel > 0:
        m = ob.modifiers.new("b", 'BEVEL'); m.width = bevel; m.segments = segs; m.limit_method = 'ANGLE'
        m.angle_limit = math.radians(angle); m.profile = 0.5
    apply_all(ob)
    return ob


def paint(ob, mat, top_mat=None, dec=0, top_dir=1):
    """face attributes: mat slot, decal index (photo on the faces pointing top_dir: 1 front, -1 back)"""
    me = ob.data
    am = me.attributes.get("mat") or me.attributes.new("mat", 'INT', 'FACE')
    ad = me.attributes.get("dec") or me.attributes.new("dec", 'INT', 'FACE')
    for p in me.polygons:
        top = top_mat is not None and p.normal.z * top_dir > 0.8
        am.data[p.index].value = top_mat if top else mat
        ad.data[p.index].value = dec if top else 0
    return ob


def decal(key, box, side="front"):
    DEC.append((key, box, side)); return len(DEC)


parts = []

# ------------------------------------------------------------------ case
case = prism("KPK", outline(-W / 2, -H / 2, W / 2, H / 2, kind, RAD), -ZF, ZF, c["edge"] / 1000.0, 5, 40)
# stepped screen recess: a 1 mm frame step, then the glass well
sx0, sy0, sx1, sy1 = rect_m(c["screen"])
def scr_shape(x0, y0, x1, y1, r):
    if c.get("screen_kind") == "chamfer":
        return outline(x0, y0, x1, y1, "chamfer", r * 2.2)
    return rounded(x0, y0, x1, y1, r)


for grow, depth in ((0.0016, 0.0007), (0.0, 0.0016)):
    cut = prism("cut", scr_shape(sx0 - grow, sy0 - grow, sx1 + grow, sy1 + grow, 0.0025 + grow), ZF - depth, ZF + 0.01)
    bo = case.modifiers.new("c", 'BOOLEAN'); bo.operation = 'DIFFERENCE'; bo.object = cut; bo.solver = 'EXACT'
    apply_all(case); bpy.data.objects.remove(cut)
screen_dec = decal("screen", c["screen"])
me = case.data
am = me.attributes.new("mat", 'INT', 'FACE'); ad = me.attributes.new("dec", 'INT', 'FACE')
inset = c["edge"] / 1000.0
for p in me.polygons:
    cen = p.center; n = p.normal
    m, d = BODY, 0
    if n.z > 0.9 and sx0 - 1e-4 <= cen.x <= sx1 + 1e-4 and sy0 - 1e-4 <= cen.y <= sy1 + 1e-4 and cen.z < ZF - 0.0012:
        m, d = PHOTO, screen_dec
    am.data[p.index].value = m; ad.data[p.index].value = d
parts.append(case)

# ------------------------------------------------------------------ details
for i, pt in enumerate(c["parts"]):
    t = pt["t"]; h = pt.get("h", 1.0) / 1000.0; nm = "%s%d" % (t, i)
    if t == "btn":
        x0, y0, x1, y1 = rect_m(pt["r"]); mm = min(x1 - x0, y1 - y0)
        dec = decal("p%d" % i, pt["r"])
        if pt.get("base", True):
            parts.append(paint(prism(nm + "b", rounded(x0 - 0.001, y0 - 0.001, x1 + 0.001, y1 + 0.001, mm * 0.25), ZF - 0.0006, ZF + 0.0007, 0.0003, 2), TRIM))
        if pt["shape"] == "cross":
            cx_, cy_ = (x0 + x1) / 2, (y0 + y1) / 2; aw = mm * 0.2
            pts = [(cx_ - aw, y0), (cx_ + aw, y0), (cx_ + aw, cy_ - aw), (x1, cy_ - aw), (x1, cy_ + aw), (cx_ + aw, cy_ + aw),
                   (cx_ + aw, y1), (cx_ - aw, y1), (cx_ - aw, cy_ + aw), (x0, cy_ + aw), (x0, cy_ - aw), (cx_ - aw, cy_ - aw)]
            ob = prism(nm, pts, ZF - 0.0003, ZF + h, mm * 0.05, 3)
        else:
            ins = mm * 0.08; rr = mm * (0.42 if pt["shape"] == "round" else 0.18)
            ob = prism(nm, rounded(x0 + ins, y0 + ins, x1 - ins, y1 - ins, rr), ZF - 0.0003, ZF + h, min(mm * 0.12, h * 0.6), 3)
        parts.append(paint(ob, METAL if pt.get("mat") == "metal" else TRIM, PHOTO, dec))
    elif t == "plate":
        x0, y0, x1, y1 = rect_m(pt["r"])
        parts.append(paint(prism(nm, rounded(x0, y0, x1, y1, min(x1 - x0, y1 - y0) * 0.15, 3), ZF - 0.0004, ZF + h, 0.0004, 2), TRIM))
    elif t == "guard":
        g = 0.0013
        grown = outline(-W / 2 - g, -H / 2 - g, W / 2 + g, H / 2 + g, kind, RAD + g)
        x0, y0, x1, y1 = rect_m(pt["r"], 3)
        poly = clip(grown, x0, y0, x1, y1)
        if len(poly) >= 3:
            parts.append(paint(prism(nm, poly, -ZF - h * 0.6, ZF + h, min(0.003, h * 0.9), 4, 30), TRIM))
    elif t == "led":
        if "c" in pt:
            cx_, cy_ = P(*pt["c"]); rad = pt["rad"] * MPP
            dec = decal("p%d" % i, None)
            parts.append(paint(prism(nm, circle(cx_, cy_, rad), ZF - 0.0003, ZF + h, rad * 0.6, 3), TRIM, PHOTO, dec))
        else:
            x0, y0, x1, y1 = rect_m(pt["r"]); mm = min(x1 - x0, y1 - y0)
            dec = decal("p%d" % i, pt["r"])
            parts.append(paint(prism(nm, rounded(x0, y0, x1, y1, mm * 0.45), ZF - 0.0003, ZF + h, min(mm * 0.3, h * 0.7), 3), TRIM, PHOTO, dec))
    elif t in ("grille", "bgrille"):
        # frame plate with raised ribs (vent slots between them)
        if t == "grille":
            x0, y0, x1, y1 = rect_m(pt["r"]); zb, sgn = ZF, 1
        else:
            a, b, cc, d = pt["r"]; bb = c["backbody"]
            bxy = lambda px, py: (W / 2 - (px - bb[0]) / (bb[2] - bb[0]) * W, H / 2 - (py - bb[1]) / (bb[3] - bb[1]) * H)
            xa, ya = bxy(a, d); xb, yb = bxy(cc, b); x0, x1 = min(xa, xb), max(xa, xb); y0, y1 = min(ya, yb), max(ya, yb); zb, sgn = -ZF, -1
        za, zz = sorted((zb - sgn * 0.0004, zb + sgn * h * 0.45))
        parts.append(paint(prism(nm, rounded(x0, y0, x1, y1, min(x1 - x0, y1 - y0) * 0.1, 3), za, zz, 0.0004, 2), TRIM))
        nrib = pt["n"]; m_ = min(x1 - x0, y1 - y0) * 0.12
        for k in range(nrib):
            f0 = (k + 0.25) / nrib; f1 = (k + 0.75) / nrib
            if pt["dir"] == "h":
                rx0, rx1 = x0 + m_, x1 - m_; ry0, ry1 = y1 - (y1 - y0 - 2 * m_) * f1 - m_, y1 - (y1 - y0 - 2 * m_) * f0 - m_
            else:
                ry0, ry1 = y0 + m_, y1 - m_; rx0, rx1 = x0 + m_ + (x1 - x0 - 2 * m_) * f0, x0 + m_ + (x1 - x0 - 2 * m_) * f1
            za2, zz2 = sorted((zz if sgn > 0 else za, (zz if sgn > 0 else za) + sgn * h * 0.55))
            parts.append(paint(prism(nm + "r%d" % k, rounded(rx0, ry0, rx1, ry1, min(rx1 - rx0, ry1 - ry0) * 0.45, 3), za2 - sgn * 0.0002, zz2, min(rx1 - rx0, ry1 - ry0) * 0.3, 2), BODY))
    elif t in ("label", "blabel"):
        dec = decal("p%d" % i, pt["r"], "front" if t == "label" else "backpx")
        if t == "label":
            x0, y0, x1, y1 = rect_m(pt["r"]); z0_, z1_, dirn = ZF - 0.0003, ZF + h, 1
        else:
            a, b, cc, d = pt["r"]; bb = c["backbody"]
            bxy = lambda px, py: (W / 2 - (px - bb[0]) / (bb[2] - bb[0]) * W, H / 2 - (py - bb[1]) / (bb[3] - bb[1]) * H)
            xa, ya = bxy(a, d); xb, yb = bxy(cc, b); x0, x1 = min(xa, xb), max(xa, xb); y0, y1 = min(ya, yb), max(ya, yb)
            z0_, z1_, dirn = -ZF - h, -ZF + 0.0003, -1
        shape = circle((x0 + x1) / 2, (y0 + y1) / 2, min(x1 - x0, y1 - y0) / 2, 24) if pt.get("round") else rounded(x0, y0, x1, y1, min(0.0012, min(x1 - x0, y1 - y0) * 0.2), 3)
        side = BODY if pt.get("bg") == "body" else TRIM
        parts.append(paint(prism(nm, shape, z0_, z1_, min(0.0003, h * 0.4), 1), side, PHOTO, dec, dirn))
    elif t == "buckle":
        # metal frame with a centre bar, over the back strap
        a, b, cc, d = pt["r"]; bb = c["backbody"]
        bxy = lambda px, py: (W / 2 - (px - bb[0]) / (bb[2] - bb[0]) * W, H / 2 - (py - bb[1]) / (bb[3] - bb[1]) * H)
        xa, ya = bxy(a, d); xb, yb = bxy(cc, b); x0, x1 = min(xa, xb), max(xa, xb); y0, y1 = min(ya, yb), max(ya, yb)
        zc = -ZF - 0.0035; r_ = 0.0012
        cu = bpy.data.curves.new(nm, 'CURVE'); cu.dimensions = '3D'; cu.bevel_depth = r_; cu.bevel_resolution = 3
        sp = cu.splines.new('POLY'); pts_ = rounded(x0, y0, x1, y1, 0.003, 3); sp.points.add(len(pts_) - 1); sp.use_cyclic_u = True
        for p_, (x_, y_) in zip(sp.points, pts_):
            p_.co = (x_, y_, zc, 1)
        ob = link(cu, nm); bpy.context.view_layer.objects.active = ob; ob.select_set(True); bpy.ops.object.convert(target='MESH'); ob.select_set(False)
        parts.append(paint(bpy.context.view_layer.objects.active, METAL))
        bpy.ops.mesh.primitive_cylinder_add(vertices=12, radius=r_ * 1.1, depth=(x1 - x0) * 0.9, location=((x0 + x1) / 2, (y0 + y1) / 2, zc), rotation=(0, math.radians(90), 0))
        parts.append(paint(bpy.context.active_object, METAL))
    elif t == "strap":
        x0, y0, x1, y1 = rect_m(pt["r"])
        parts.append(paint(prism(nm, [(x0, y0), (x1, y0), (x1, y1), (x0, y1)], ZF - 0.002, ZF + h, 0.0004, 2), WEAVE))
    elif t == "screw":
        cx_, cy_ = P(*pt["c"]); rad = pt["rad"] * MPP * 0.8
        parts.append(paint(prism(nm, circle(cx_, cy_, rad, 12), ZF - 0.0003, ZF + 0.0007, 0.00035, 2), METAL))
        # slot
        parts.append(paint(prism(nm + "s", [(cx_ - rad * 0.8, cy_ - rad * 0.12), (cx_ + rad * 0.8, cy_ - rad * 0.12), (cx_ + rad * 0.8, cy_ + rad * 0.12), (cx_ - rad * 0.8, cy_ + rad * 0.12)], ZF + 0.0006, ZF + 0.00075), TRIM))
    elif t == "lip":
        a, b, cc, d = c["screen"]; g = pt["w"]
        ox0, oy0, ox1, oy1 = rect_m((a, b, cc, d), g + 2)
        ring = scr_shape(ox0, oy0, ox1, oy1, 0.004)
        ob = prism(nm, ring, ZF - 0.0004, ZF + h, 0.0004, 2)
        cut = prism("lc", scr_shape(sx0 - 0.0016, sy0 - 0.0016, sx1 + 0.0016, sy1 + 0.0016, 0.0041), ZF - 0.01, ZF + 0.01)
        bo = ob.modifiers.new("c", 'BOOLEAN'); bo.operation = 'DIFFERENCE'; bo.object = cut; bo.solver = 'EXACT'
        apply_all(ob); bpy.data.objects.remove(cut)
        parts.append(paint(ob, TRIM))
    elif t == "clip":
        x0, y0, x1, y1 = rect_m(pt["r"])
        parts.append(paint(prism(nm, rounded(x0, y0 - 0.004, x1, y1, (x1 - x0) * 0.15, 3), -D * 0.36, D * 0.44, 0.0009, 3), TRIM))
    elif t == "ring":
        cx_, cy_ = P(*pt["c"])
        bpy.ops.mesh.primitive_torus_add(major_radius=pt["rad"] * MPP, minor_radius=pt["tube"] * MPP * 0.8, major_segments=20, minor_segments=8,
                                         location=(cx_, cy_, ZF + pt["tube"] * MPP * 0.3))
        parts.append(paint(bpy.context.active_object, METAL))
    elif t in ("bplate", "bstrap"):
        a, b, cc, d = pt["r"]; bb = c["backbody"]
        bxy = lambda px, py: (W / 2 - (px - bb[0]) / (bb[2] - bb[0]) * W, H / 2 - (py - bb[1]) / (bb[3] - bb[1]) * H)
        xa, ya = bxy(a, d); xb, yb = bxy(cc, b)
        x0, x1 = min(xa, xb), max(xa, xb); y0, y1 = min(ya, yb), max(ya, yb)
        if t == "bstrap":
            pts = [(x0, -H / 2 - h), (x1, -H / 2 - h), (x1, H / 2 + h), (x0, H / 2 + h)]
            parts.append(paint(prism(nm, pts, -ZF - h, -ZF + 0.004, 0.0006, 2), WEAVE))
        else:
            dec = decal("back", c["backbody"], "back")
            parts.append(paint(prism(nm, rounded(x0, y0, x1, y1, min(x1 - x0, y1 - y0) * 0.12, 3), -ZF - h, -ZF + 0.0004, 0.0004, 2), TRIM, PHOTO, dec, -1))


def tube(nm, pts_px, radius, z=0.0):
    cu = bpy.data.curves.new(nm, 'CURVE'); cu.dimensions = '3D'
    cu.bevel_depth = radius; cu.bevel_resolution = 3; cu.use_fill_caps = True
    sp = cu.splines.new('POLY'); sp.points.add(len(pts_px) - 1)
    for p, (px, py) in zip(sp.points, pts_px):
        x, y = P(px, py); p.co = (x, y, z, 1)
    ob = link(cu, nm); bpy.context.view_layer.objects.active = ob; ob.select_set(True)
    bpy.ops.object.convert(target='MESH'); ob.select_set(False)
    return bpy.context.view_layer.objects.active


for i, pt in enumerate(c["parts"]):
    if pt["t"] == "loop":
        a, b, cc, d = pt["r"]
        parts.append(paint(tube("loop%d" % i, [(a, b), (a, d - 4), (a + 4, d), (cc - 4, d), (cc, d - 4), (cc, b)], 0.0015, ZF - 0.002), METAL))

if c.get("handles"):
    mx = 2 * CX
    left = [(85, 92), (58, 92), (45, 112), (45, 352), (58, 374), (85, 374)]
    parts.append(paint(tube("hl", left, 0.0024), METAL))
    parts.append(paint(tube("hr", [(mx - x, y) for x, y in left], 0.0024), METAL))
    for gx in (57, mx - 57):
        x, ya = P(gx, 150); _, yb = P(gx, 300)
        bpy.ops.mesh.primitive_cylinder_add(vertices=16, radius=0.0056, depth=abs(ya - yb), location=(x, (ya + yb) / 2, 0), rotation=(math.radians(90), 0, 0))
        parts.append(paint(bpy.context.active_object, WEAVE))
    parts.append(paint(tube("ht", [(287, 47), (294, 36), (307, 31), (489, 31), (502, 36), (509, 47)], 0.0021), METAL))

# printed decals on thin plates
for i, r in enumerate(c["decals"]):
    x0, y0, x1, y1 = rect_m(r)
    parts.append(paint(prism("dec%d" % i, rounded(x0, y0, x1, y1, 0.0008, 2), ZF - 0.0003, ZF + 0.00035), TRIM, PHOTO, decal("d%d" % i, r)))

# rubber port covers on the sides
for side, ypx, lpx in c["ports"]:
    _, yc = P(CX, ypx); half = lpx * MPP / 2
    xs = W / 2 if side == "right" else -W / 2
    x0, x1 = (xs - 0.0015, xs + 0.0012) if side == "right" else (xs - 0.0012, xs + 0.0015)
    parts.append(paint(prism("port", rounded(x0, yc - half, x1, yc + half, 0.0012, 3), -D * 0.22, D * 0.22, 0.0005, 2), TRIM))

# Monolith cloth band
if c.get("cloth"):
    cl = c["cloth"]; xa, _ = P(cl["x"][0], 0); xb, _ = P(cl["x"][1], 0)
    band = prism("cloth", rounded(xa, -H / 2 - 0.0014, W / 2 + 0.0014, H / 2 + 0.0014, 0.006, 4), -ZF - 0.0014, ZF + 0.0014, 0.0035, 4, 30)
    parts.append(paint(band, CLOTH))

extra = os.path.join(os.path.dirname(os.path.abspath(__file__)), name + "_extra.py")
if os.path.exists(extra):
    exec(open(extra).read())

# ------------------------------------------------------------------ join, materials, UVs
bpy.ops.object.select_all(action='DESELECT')
for ob in parts:
    ob.select_set(True)
bpy.context.view_layer.objects.active = case
bpy.ops.object.join()
kpk = bpy.context.active_object; kpk.name = "KPK"; me = kpk.data
while me.uv_layers:
    me.uv_layers.remove(me.uv_layers[0])
uv = me.uv_layers.new(name="UV"); me.uv_layers.active = uv; uv.active_render = True
am = me.attributes["mat"]; ad = me.attributes["dec"]
TILE = 0.06   # metres per repeat of the tiling materials
for p in me.polygons:
    m = am.data[p.index].value; d = ad.data[p.index].value
    p.material_index = m
    n = p.normal; ax = max(range(3), key=lambda i: abs(n[i]))
    for li in p.loop_indices:
        co = me.vertices[me.loops[li].vertex_index].co
        if m == PHOTO and d > 0:
            key, box, side = DEC[d - 1]
            u0, v0, rw, rh = REG[key]
            if side == "backpx":
                bb = c["backbody"]
                px = bb[0] + (W / 2 - co.x) / W * (bb[2] - bb[0]); py = bb[1] + (H / 2 - co.y) / H * (bb[3] - bb[1])
                fu, fv = (px - box[0]) / (box[2] - box[0]), (py - box[1]) / (box[3] - box[1])
            elif key == "back" or side == "back":
                fu, fv = (W / 2 - co.x) / W, (H / 2 - co.y) / H
            else:
                if box is None:
                    pt = c["parts"][int(key[1:])]; cx_, cy_ = pt["c"]; rr = pt["rad"]; box = (cx_ - rr, cy_ - rr, cx_ + rr, cy_ + rr)
                px = co.x / MPP + CX; py = CY - co.y / MPP
                fu, fv = (px - box[0]) / (box[2] - box[0]), (py - box[1]) / (box[3] - box[1])
            fu = min(1, max(0, fu)); fv = min(1, max(0, fv))
            uv.data[li].uv = (u0 + fu * rw, 1 - (v0 + fv * rh))
        else:
            a, b = [(1, 2), (0, 2), (0, 1)][ax]
            tl = {BODY: 0.09, TRIM: 0.06, METAL: 0.04, WEAVE: 0.022, CLOTH: 0.03}.get(m, TILE)
            uv.data[li].uv = (co[a] / tl, co[b] / tl)

bpy.context.view_layer.objects.active = kpk; kpk.select_set(True)
bpy.ops.object.shade_smooth_by_angle(angle=math.radians(35))

TEX = {"body": out + "/body", "trim": out + "/trim", "photo": out + "/photo", "metal": out + "/../v3_shared/kpk_metal",
       "weave": out + "/../v3_shared/kpk_weave", "cloth": out + "/cloth"}
GAME = {"body": "kpk_%s_body" % name, "trim": "kpk_%s_trim" % name, "photo": "kpk_%s_photo" % name, "metal": "kpk_metal",
        "weave": "kpk_weave", "cloth": "kpk_%s_cloth" % name}
used = sorted(set(am.data[p.index].value for p in me.polygons))
mats_idx = [p.material_index for p in me.polygons]
kpk.data.materials.clear()
for s_ in SLOTS:
    mat = bpy.data.materials.new(s_); mat.use_nodes = True
    nt = mat.node_tree; bsdf = nt.nodes["Principled BSDF"]
    if os.path.exists(TEX[s_] + "_co.png"):
        tx = nt.nodes.new("ShaderNodeTexImage"); tx.image = bpy.data.images.load(TEX[s_] + "_co.png")
        nt.links.new(tx.outputs[0], bsdf.inputs["Base Color"])
    bsdf.inputs["Roughness"].default_value = 0.65 if s_ != "metal" else 0.4
    bsdf.inputs["Metallic"].default_value = 0.6 if s_ in ("metal",) else 0.0
    kpk.data.materials.append(mat)
for p, mi in zip(kpk.data.polygons, mats_idx):
    p.material_index = mi
print("slots", [m.name if m else None for m in kpk.data.materials])
bpy.ops.file.pack_all()


def dump(ob, path):
    m = ob.evaluated_get(bpy.context.evaluated_depsgraph_get()).to_mesh()
    m.calc_loop_triangles()
    uvl = m.uv_layers.active.data; cn = m.corner_normals
    data = {"verts": [list(v.co) for v in m.vertices], "tris": [], "mats": [GAME[s] for s in SLOTS]}
    for tri in m.loop_triangles:
        data["tris"].append([[m.loops[li].vertex_index, list(cn[li].vector), list(uvl[li].uv)] for li in tri.loops] + [tri.material_index])
    json.dump(data, open(path, "w"))
    print(path, "tris", len(data["tris"]))


dump(kpk, out + "/mesh.json")
dec_ = kpk.modifiers.new("dec", 'DECIMATE'); dec_.ratio = 0.5
dump(kpk, out + "/mesh_lod1.json")
kpk.modifiers.remove(dec_)
bpy.ops.wm.save_as_mainfile(filepath=out + "/kpk.blend")
print("build ok, materials used", [SLOTS[u] for u in used])
