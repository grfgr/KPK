# Step 2 (Blender): case from the concept proportions, recessed screen, raised buttons, model parts,
# UVs projected onto the atlas views (front / back / sides / top / bottom). Writes mesh.json,
# mesh_lod1.json (for write_p3d5.py) and kpk.blend (previews).
# usage: blender python build_ref.py -- <name> <out dir>
import bpy, bmesh, sys, json, math, os
from mathutils import Vector
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from cfg import CFG

args = sys.argv[sys.argv.index("--") + 1:]
name, out = args[0], args[1]
c = CFG[name]
prep = json.load(open(out + "/prep.json"))
R = prep["regions"]

bpy.ops.wm.read_factory_settings(use_empty=True)
bx0, by0, bx1, by1 = c["body"]
MPP = c["W"] / 1000.0 / (bx1 - bx0)           # metres per sheet pixel
CX, CY = (bx0 + bx1) / 2.0, (by0 + by1) / 2.0
W = c["W"] / 1000.0
H = (by1 - by0) * MPP
D = c["D"] / 1000.0
print(name, "size mm", round(W * 1000), round(H * 1000), round(D * 1000))


def P(px, py):
    return ((px - CX) * MPP, (CY - py) * MPP)


def outline(x0, y0, x1, y1, kind, r):
    # rectangle in metres with chamfered or rounded corners, counter-clockwise
    pts = []
    corners = [(x1, y0, -90), (x1, y1, 0), (x0, y1, 90), (x0, y0, 180)]
    for cxx, cyy, a0 in corners:
        sx = -1 if cxx == x1 else 1
        sy = -1 if cyy == y1 else 1
        ox, oy = cxx + sx * r, cyy + sy * r
        steps = 1 if kind == "chamfer" else 6
        for i in range(steps + 1):
            a = math.radians(a0 + 90.0 * i / steps)
            pts.append((ox + r * math.cos(a), oy + r * math.sin(a)))
    return pts


def prism(name_, pts, z0, z1, bevel, segs=2):
    me = bpy.data.meshes.new(name_)
    bm = bmesh.new()
    vs = [bm.verts.new((x, y, z0)) for x, y in pts]
    f = bm.faces.new(vs)
    bmesh.ops.recalc_face_normals(bm, faces=[f])
    if f.normal.z > 0:
        f.normal_flip()
    ext = bmesh.ops.extrude_face_region(bm, geom=[f])
    top = [v for v in ext["geom"] if isinstance(v, bmesh.types.BMVert)]
    bmesh.ops.translate(bm, verts=top, vec=(0, 0, z1 - z0))
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces[:])
    bm.to_mesh(me); bm.free()
    ob = bpy.data.objects.new(name_, me)
    bpy.context.scene.collection.objects.link(ob)
    if bevel > 0:
        m = ob.modifiers.new("b", 'BEVEL'); m.width = bevel; m.segments = segs; m.limit_method = 'ANGLE'
        m.angle_limit = math.radians(40); m.profile = 0.5
    return ob


def box(name_, px0, py0, px1, py1, z0, z1, bevel=0.0006):
    x0, y0 = P(px0, py1); x1, y1 = P(px1, py0)
    return prism(name_, [(x0, y0), (x1, y0), (x1, y1), (x0, y1)], z0, z1, bevel)


def apply_all(ob):
    bpy.context.view_layer.objects.active = ob
    for m in list(ob.modifiers):
        bpy.ops.object.modifier_apply(modifier=m.name)


# ---- case
kind, rpx = c["corner"]
x0, y0 = -W / 2, -H / 2
# rounded edges all around (tablet-like): wide round bevel on the front and back rims
case = prism("KPK", outline(x0, y0, -x0, -y0, kind, max(rpx * MPP, 0.009)), -D / 2, D / 2, c["edge"] / 1000.0, 5)
apply_all(case)

# ---- recessed screen
sx0, sy0, sx1, sy1 = c["screen"]
cut = box("cut", sx0, sy0, sx1, sy1, D / 2 - 0.0013, D / 2 + 0.01, 0)
apply_all(cut)
bo = case.modifiers.new("cut", 'BOOLEAN'); bo.operation = 'DIFFERENCE'; bo.object = cut; bo.solver = 'EXACT'
apply_all(case)
bpy.data.objects.remove(cut)

parts = [case]


def tube(name_, pts_px, radius, z=0.0):
    cu = bpy.data.curves.new(name_, 'CURVE'); cu.dimensions = '3D'
    cu.bevel_depth = radius; cu.bevel_resolution = 3; cu.use_fill_caps = True
    sp = cu.splines.new('POLY'); sp.points.add(len(pts_px) - 1)
    for p, (px, py) in zip(sp.points, pts_px):
        x, y = P(px, py); p.co = (x, y, z, 1)
    ob = bpy.data.objects.new(name_, cu); bpy.context.scene.collection.objects.link(ob)
    bpy.context.view_layer.objects.active = ob
    ob.select_set(True)
    bpy.ops.object.convert(target='MESH')
    ob.select_set(False)
    return bpy.context.view_layer.objects.active


def cylinder(name_, px, py0, py1, radius):
    x, ya = P(px, py0); _, yb = P(px, py1)
    bpy.ops.mesh.primitive_cylinder_add(vertices=16, radius=radius, depth=abs(ya - yb), location=(x, (ya + yb) / 2, 0),
                                        rotation=(math.radians(90), 0, 0))
    ob = bpy.context.active_object; ob.name = name_
    return ob




def tag(ob, mode):
    # uvmode per face: 0 = by normal, 1 = front view, 2 = back view
    at = ob.data.attributes.get("uvmode") or ob.data.attributes.new("uvmode", 'INT', 'FACE')
    for d in at.data:
        d.value = mode
    return ob


def rounded(x0, y0, x1, y1, r, steps=5):
    pts = []
    for cx_, cy_, a0 in ((x1 - r, y0 + r, -90), (x1 - r, y1 - r, 0), (x0 + r, y1 - r, 90), (x0 + r, y0 + r, 180)):
        for i in range(steps + 1):
            a = math.radians(a0 + 90.0 * i / steps)
            pts.append((cx_ + r * math.cos(a), cy_ + r * math.sin(a)))
    return pts


def rect_m(r, grow=0):
    a, b, cc, d = r
    x0, y0 = P(a - grow, d + grow); x1, y1 = P(cc + grow, b - grow)
    return x0, y0, x1, y1


def solid(name_, pts, z0, z1, bevel, segs=2, mode=1):
    ob = prism(name_, pts, z0, z1, 0)
    if bevel > 0:
        m = ob.modifiers.new("b", 'BEVEL'); m.width = bevel; m.segments = segs; m.limit_method = 'ANGLE'
        m.angle_limit = math.radians(30)
    apply_all(ob)
    return tag(ob, mode)


def circle(cx_, cy_, rad, n=14):
    return [(cx_ + rad * math.cos(2 * math.pi * i / n), cy_ + rad * math.sin(2 * math.pi * i / n)) for i in range(n)]


def back_xy(bx, by):
    # back view pixel -> model x, y (the back view is mirrored left / right)
    bb = c["backbody"]
    return (W / 2 - (bx - bb[0]) / (bb[2] - bb[0]) * W, H / 2 - (by - bb[1]) / (bb[3] - bb[1]) * H)


ZF = D / 2
for i, pt in enumerate(c.get("parts", [])):
    t = pt["t"]; h = pt.get("h", 1.0) / 1000.0; nm = "%s%d" % (t, i)
    if t == "btn":
        x0, y0, x1, y1 = rect_m(pt["r"])
        w_, h_ = x1 - x0, y1 - y0; m_ = min(w_, h_)
        if pt.get("base", True):
            parts.append(solid(nm + "b", rounded(x0 - 0.0008, y0 - 0.0008, x1 + 0.0008, y1 + 0.0008, m_ * 0.22), ZF - 0.0005, ZF + 0.0006, 0.0002))
        if pt["shape"] == "cross":
            cx_, cy_ = (x0 + x1) / 2, (y0 + y1) / 2; aw = m_ * 0.2
            cross = [(cx_ - aw, y0), (cx_ + aw, y0), (cx_ + aw, cy_ - aw), (x1, cy_ - aw), (x1, cy_ + aw), (cx_ + aw, cy_ + aw),
                     (cx_ + aw, y1), (cx_ - aw, y1), (cx_ - aw, cy_ + aw), (x0, cy_ + aw), (x0, cy_ - aw), (cx_ - aw, cy_ - aw)]
            parts.append(solid(nm, cross, ZF - 0.0003, ZF + h, m_ * 0.06, 3))
        else:
            rr = m_ * (0.42 if pt["shape"] == "round" else 0.16)
            ins = m_ * 0.08
            parts.append(solid(nm, rounded(x0 + ins, y0 + ins, x1 - ins, y1 - ins, rr), ZF - 0.0003, ZF + h, min(m_ * 0.14, h * 0.7), 3))
    elif t == "plate":
        x0, y0, x1, y1 = rect_m(pt["r"])
        parts.append(solid(nm, rounded(x0, y0, x1, y1, min(x1 - x0, y1 - y0) * 0.15, 2), ZF - 0.0004, ZF + h, 0.0004))
    elif t == "guard" or t == "wrap":
        x0, y0, x1, y1 = rect_m(pt["r"], 3)
        x0 = max(x0, -W / 2 - 0.0012); x1 = min(x1, W / 2 + 0.0012); y0 = max(y0, -H / 2 - 0.0012); y1 = min(y1, H / 2 + 0.0012)
        if t == "wrap":
            y0, y1 = -H / 2 - h, H / 2 + h
        parts.append(solid(nm, rounded(x0, y0, x1, y1, min(x1 - x0, y1 - y0) * 0.3, 4), -ZF - h, ZF + h, 0.0028, 4, 0))
    elif t == "led":
        cx_, cy_ = P(*pt["c"]); rad = pt["rad"] * MPP
        parts.append(solid(nm, circle(cx_, cy_, rad), ZF - 0.0003, ZF + h, rad * 0.6, 3))
    elif t == "screw":
        cx_, cy_ = P(*pt["c"]); rad = pt["rad"] * MPP
        parts.append(solid(nm, circle(cx_, cy_, rad, 10), ZF - 0.0003, ZF + 0.0006, 0.0002, 1))
    elif t == "lip":
        a, b, cc, d = c["screen"]; g = pt["w"]
        ox0, oy0, ox1, oy1 = rect_m((a, b, cc, d), g); ix0, iy0, ix1, iy1 = rect_m((a, b, cc, d), 0)
        frame = [(ox0, oy0, ox1, iy0), (ox0, iy1, ox1, oy1), (ox0, iy0, ix0, iy1), (ix1, iy0, ox1, iy1)]
        for k, (q0, q1, q2, q3) in enumerate(frame):
            parts.append(solid(nm + str(k), [(q0, q1), (q2, q1), (q2, q3), (q0, q3)], ZF - 0.0004, ZF + h, 0.0003, 1))
    elif t == "clip":
        x0, y0, x1, y1 = rect_m(pt["r"])
        parts.append(solid(nm, rounded(x0, y0 - 0.004, x1, y1, (x1 - x0) * 0.12, 2), -D * 0.32, D * 0.42, 0.0008, 2, 0))
    elif t == "ring":
        cx_, cy_ = P(*pt["c"])
        bpy.ops.mesh.primitive_torus_add(major_radius=pt["rad"] * MPP, minor_radius=pt["tube"] * MPP, major_segments=18, minor_segments=8,
                                         location=(cx_, cy_, ZF + pt["tube"] * MPP * 0.4))
        parts.append(tag(bpy.context.active_object, 1))
    elif t == "loop":
        a, b, cc, d = pt["r"]
        lp = tube(nm, [(a, b), (a, d - 4), (a + 4, d), (cc - 4, d), (cc, d - 4), (cc, b)], 0.0016, ZF - 0.002)
        parts.append(tag(lp, 1))
    elif t in ("bplate", "bstrap"):
        a, b, cc, d = pt["r"]
        xa, ya = back_xy(a, d); xb, yb = back_xy(cc, b)
        x0, x1 = min(xa, xb), max(xa, xb); y0, y1 = min(ya, yb), max(ya, yb)
        if t == "bstrap":
            y0, y1 = -H / 2 - h, H / 2 + h
            parts.append(solid(nm, [(x0, y0), (x1, y0), (x1, y1), (x0, y1)], -ZF - h, -ZF + 0.004, 0.0005, 1, 2))
        else:
            parts.append(solid(nm, rounded(x0, y0, x1, y1, min(x1 - x0, y1 - y0) * 0.12, 2), -ZF - h, -ZF + 0.0004, 0.0004, 2, 2))


if c.get("handles"):
    mx = 2 * CX
    left = [(88, 92), (60, 92), (46, 112), (46, 352), (60, 374), (88, 374)]
    parts.append(tag(tube("hl", left, 0.0024), 0))
    parts.append(tube("hr", [(mx - x, y) for x, y in left], 0.0024))
    parts.append(cylinder("gl", 57, 150, 300, 0.0058))
    parts.append(cylinder("gr", mx - 57, 150, 300, 0.0058))
    parts.append(tube("ht", [(300, 47), (308, 36), (322, 32), (612, 32), (628, 36), (636, 47)], 0.0021))

# ---- one object
bpy.ops.object.select_all(action='DESELECT')
for ob in parts:
    ob.select_set(True)
bpy.context.view_layer.objects.active = case
bpy.ops.object.join()
kpk = bpy.context.active_object
kpk.name = "KPK"

# ---- UVs: each face takes the view it faces
fx0, fy0, fx1, fy1 = c["front"]
me = kpk.data
while me.uv_layers:
    me.uv_layers.remove(me.uv_layers[0])
uv = me.uv_layers.new(name="UV")
me.uv_layers.active = uv
uv.active_render = True


def reg(key, fu, fv):
    u0, v0, rw, rh = R[key]
    fu = min(1.0, max(0.0, fu)); fv = min(1.0, max(0.0, fv))
    return (u0 + fu * rw, 1.0 - (v0 + fv * rh))


def front_uv(x, y):
    return ((x / MPP + CX - fx0) / (fx1 - fx0), (CY - y / MPP - fy0) / (fy1 - fy0))


FXW = (fx1 - fx0) * MPP
FXL = (fx0 - CX) * MPP
modes = me.attributes.get("uvmode")
for poly in me.polygons:
    n = poly.normal
    ax = max(range(3), key=lambda i: abs(n[i]))
    mode = modes.data[poly.index].value if modes else 0
    if mode == 1:
        ax = 2; n = Vector((0, 0, 1))
    elif mode == 2:
        ax = 2; n = Vector((0, 0, -1))
    for li in poly.loop_indices:
        co = me.vertices[me.loops[li].vertex_index].co
        x, y, z = co
        if ax == 2 and n.z > 0:
            fu, fv = front_uv(x, y); t = reg("front", fu, fv)
        elif ax == 2:
            t = reg("back", (W / 2 - x) / W, (H / 2 - y) / H)
        elif ax == 0:
            U = (H / 2 - y) / H
            if n.x < 0:
                t = reg("left", U, 1.0 - (z + D / 2) / D)
            else:
                t = reg("right", U, 1.0 - (D / 2 - z) / D)
        else:
            fu = (x + W / 2) / W
            if n.y > 0:
                t = reg("top", fu, (z + D / 2) / D)
            else:
                t = reg("bottom", fu, (D / 2 - z) / D)
        uv.data[li].uv = t

bpy.context.view_layer.objects.active = kpk
kpk.select_set(True)
bpy.ops.object.shade_smooth_by_angle(angle=math.radians(35))

# ---- material for previews
mat = bpy.data.materials.new("kpk"); mat.use_nodes = True
nt = mat.node_tree; bsdf = nt.nodes["Principled BSDF"]
tex = nt.nodes.new("ShaderNodeTexImage"); tex.image = bpy.data.images.load(out + "/atlas_co.png")
nt.links.new(tex.outputs[0], bsdf.inputs["Base Color"])
nrm = nt.nodes.new("ShaderNodeTexImage"); nrm.image = bpy.data.images.load(out + "/atlas_normal.png")
nrm.image.colorspace_settings.name = "Non-Color"
nm = nt.nodes.new("ShaderNodeNormalMap"); nm.inputs[0].default_value = 0.6
nt.links.new(nrm.outputs[0], nm.inputs[1]); nt.links.new(nm.outputs[0], bsdf.inputs["Normal"])
bsdf.inputs["Roughness"].default_value = 0.7
kpk.data.materials.clear()
kpk.data.materials.append(mat)
for poly in kpk.data.polygons:
    poly.material_index = 0
bpy.ops.file.pack_all()
print('material', [m.name if m else None for m in kpk.data.materials], len(kpk.material_slots))


def dump(ob, path):
    m = ob.evaluated_get(bpy.context.evaluated_depsgraph_get()).to_mesh()
    m.calc_loop_triangles()
    uvl = m.uv_layers.active.data
    cn = m.corner_normals
    data = {"verts": [list(v.co) for v in m.vertices], "tris": []}
    for tri in m.loop_triangles:
        data["tris"].append([[m.loops[li].vertex_index, list(cn[li].vector), list(uvl[li].uv)] for li in tri.loops])
    json.dump(data, open(path, "w"))
    print(path, "tris", len(data["tris"]), "verts", len(data["verts"]))


dump(kpk, out + "/mesh.json")
dec = kpk.modifiers.new("dec", 'DECIMATE'); dec.ratio = 0.45
dump(kpk, out + "/mesh_lod1.json")
kpk.modifiers.remove(dec)
bpy.ops.wm.save_as_mainfile(filepath=out + "/kpk.blend")
print("build ok")
