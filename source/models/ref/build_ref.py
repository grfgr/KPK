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


def prism(name_, pts, z0, z1, bevel):
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
        m = ob.modifiers.new("b", 'BEVEL'); m.width = bevel; m.segments = 2; m.limit_method = 'ANGLE'
        m.angle_limit = math.radians(40)
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
case = prism("KPK", outline(x0, y0, -x0, -y0, kind, rpx * MPP), -D / 2, D / 2, 0.0022)
apply_all(case)

# ---- recessed screen
sx0, sy0, sx1, sy1 = c["screen"]
cut = box("cut", sx0, sy0, sx1, sy1, D / 2 - 0.0013, D / 2 + 0.01, 0)
apply_all(cut)
bo = case.modifiers.new("cut", 'BOOLEAN'); bo.operation = 'DIFFERENCE'; bo.object = cut; bo.solver = 'EXACT'
apply_all(case)
bpy.data.objects.remove(cut)

parts = [case]
# ---- raised buttons
for i, (a, b, cc, d) in enumerate(c.get("buttons", [])):
    ob = box("btn%d" % i, a, b, cc, d, D / 2 - 0.001, D / 2 + 0.0013)
    apply_all(ob); parts.append(ob)
# ---- clips on the top edge (NII)
for i, (a, b, cc, d) in enumerate(c.get("clips", [])):
    ob = box("clip%d" % i, a, b, cc, d + 30, -D * 0.3, D * 0.42, 0.001)
    apply_all(ob); parts.append(ob)


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


if c.get("handles"):
    mx = 2 * CX
    left = [(88, 92), (60, 92), (46, 112), (46, 352), (60, 374), (88, 374)]
    parts.append(tube("hl", left, 0.0024))
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
for poly in me.polygons:
    n = poly.normal
    ax = max(range(3), key=lambda i: abs(n[i]))
    for li in poly.loop_indices:
        co = me.vertices[me.loops[li].vertex_index].co
        x, y, z = co
        if ax == 2 and n.z > 0:
            fu, fv = front_uv(x, y); t = reg("front", fu, fv)
        elif ax == 2:
            fu, fv = front_uv(x, y); t = reg("back", 1.0 - fu, fv)
        elif ax == 0:
            U = (H / 2 - y) / H
            if n.x < 0:
                t = reg("left", U, 1.0 - (z + D / 2) / D)
            else:
                t = reg("right", U, 1.0 - (D / 2 - z) / D)
        else:
            fu = (x - FXL) / FXW
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
