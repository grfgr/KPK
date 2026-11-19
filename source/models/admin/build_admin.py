# Admin KPK «Куратор»: a simple rugged GPS-like unit, landscape: dark blue-grey plastic body, rubber corner
# bumpers, wide screen on the left, d-pad + keys on the right, orange stripe with the name, stub antenna
# on the top-left corner. Writes mesh.json / mesh_lod1.json (write_p3d5 format: screen +Z, antenna +Y),
# kpk.blend and the front ortho render for the interface frame.
# usage: blender python build_admin.py -- <out dir> <atlas.png>
import bpy, bmesh, sys, json, math, os
from mathutils import Vector
a = sys.argv[sys.argv.index("--") + 1:]
OUT, ATLAS = a[0], a[1]
bpy.ops.wm.read_factory_settings(use_empty=True)

W, H, D = 0.150, 0.080, 0.026
ZF = D / 2
# screen (left), key column (right)
SX0, SX1, SY0, SY1 = -0.057, 0.040, -0.022, 0.031
# atlas regions (u0, v0, u1, v1) in 0..1, v up
R = {"body": (0.0, 0.0, 0.5, 0.5), "rubber": (0.5, 0.0, 0.75, 0.25), "accent": (0.75, 0.0, 1.0, 0.25),
     "key": (0.5, 0.25, 0.75, 0.5), "dark": (0.75, 0.25, 1.0, 0.5), "screen": (0.0, 0.5, 0.5, 1.0),
     "plate": (0.5, 0.5, 1.0, 0.75), "metal": (0.5, 0.75, 0.75, 1.0)}
parts = []


def box(name, x0, y0, z0, x1, y1, z1, bevel=0.0, seg=3, reg="body", exact=False):
    bpy.ops.mesh.primitive_cube_add(location=((x0 + x1) / 2, (y0 + y1) / 2, (z0 + z1) / 2))
    o = bpy.context.active_object; o.name = name
    o.scale = ((x1 - x0) / 2, (y1 - y0) / 2, (z1 - z0) / 2)
    bpy.ops.object.transform_apply(scale=True)
    if bevel > 0:
        m = o.modifiers.new("b", 'BEVEL'); m.width = bevel; m.segments = seg; m.limit_method = 'ANGLE'
        bpy.ops.object.modifier_apply(modifier="b")
    parts.append((o, reg, exact)); return o


def cyl(name, x, y, z0, z1, r, reg, axis="z", seg=20, bevel=0.0):
    depth = z1 - z0
    if axis == "z":
        bpy.ops.mesh.primitive_cylinder_add(vertices=seg, radius=r, depth=depth, location=(x, y, (z0 + z1) / 2))
    else:   # along y
        bpy.ops.mesh.primitive_cylinder_add(vertices=seg, radius=r, depth=depth, location=(x, (z0 + z1) / 2, y), rotation=(math.radians(90), 0, 0))
    o = bpy.context.active_object; o.name = name
    if bevel > 0:
        m = o.modifiers.new("b", 'BEVEL'); m.width = bevel; m.segments = 2; m.limit_method = 'ANGLE'
        bpy.ops.object.modifier_apply(modifier="b")
    parts.append((o, reg, False)); return o


# body: main shell, slightly thinner back plate
box("body", -W / 2, -H / 2, -ZF + 0.002, W / 2, H / 2, ZF, 0.006, 4)
box("back", -W / 2 + 0.006, -H / 2 + 0.006, -ZF, W / 2 - 0.006, H / 2 - 0.006, -ZF + 0.004, 0.003, 3)
# rubber corner bumpers
for sx in (-1, 1):
    for sy in (-1, 1):
        cx, cy = sx * (W / 2 - 0.007), sy * (H / 2 - 0.007)
        box("bump", cx - 0.0095, cy - 0.0095, -ZF - 0.0015, cx + 0.0095, cy + 0.0095, ZF + 0.0006, 0.004, 3, "rubber")
# rubber side grips
for sx in (-1, 1):
    box("grip", sx * W / 2 - 0.003, -0.018, -ZF + 0.003, sx * W / 2 + 0.003, 0.018, ZF - 0.003, 0.002, 2, "rubber")
# screen bezel (dark) and screen
box("bezel", SX0 - 0.005, SY0 - 0.005, ZF - 0.001, SX1 + 0.005, SY1 + 0.005, ZF + 0.0012, 0.0025, 3, "dark")
box("screen", SX0, SY0, ZF + 0.0006, SX1, SY1, ZF + 0.0014, 0, 1, "screen", True)
# orange stripe with the name under the screen
box("plate", SX0 + 0.004, -0.0362, ZF - 0.0005, SX1 + 0.002, -0.0282, ZF + 0.0008, 0.0008, 2, "plate", True)
# key column: d-pad ring, centre key, two keys above, power key below
KX = (SX1 + W / 2) / 2 + 0.001
cyl("dpad", KX, 0.002, ZF - 0.001, ZF + 0.0022, 0.0115, "dark", bevel=0.0008)
cyl("ok", KX, 0.002, ZF, ZF + 0.0032, 0.005, "key", bevel=0.0007)
for dx, dy in ((0, 0.0083), (0, -0.0083), (0.0083, 0), (-0.0083, 0)):
    box("arrow", KX + dx - 0.0022, 0.002 + dy - 0.0022, ZF + 0.002, KX + dx + 0.0022, 0.002 + dy + 0.0022, ZF + 0.003, 0.0008, 2, "key")
for dx in (-0.0075, 0.0075):
    box("key", KX + dx - 0.0055, 0.021, ZF - 0.0005, KX + dx + 0.0055, 0.029, ZF + 0.0028, 0.0015, 3, "key")
box("power", KX - 0.007, -0.027, ZF - 0.0005, KX + 0.007, -0.019, ZF + 0.0028, 0.0015, 3, "accent")
# speaker slots
for k in range(3):
    box("slot", KX - 0.008, -0.0345 + k * 0.0022, ZF - 0.0003, KX + 0.008, -0.0335 + k * 0.0022, ZF + 0.0003, 0, 1, "dark")
# antenna: base, rubber stub, tip, on the top-left corner
AX = -W / 2 + 0.020
cyl("abase", AX, 0.0, H / 2 - 0.002, H / 2 + 0.006, 0.0055, "metal", axis="y")
cyl("astub", AX, 0.0, H / 2 + 0.006, H / 2 + 0.046, 0.0034, "rubber", axis="y", seg=14)
bpy.ops.mesh.primitive_uv_sphere_add(segments=12, ring_count=7, radius=0.0042, location=(AX, H / 2 + 0.047, 0))
parts.append((bpy.context.active_object, "rubber", False))
# lanyard loop bottom-right
box("loop", W / 2 - 0.030, -H / 2 - 0.004, -0.005, W / 2 - 0.018, -H / 2 + 0.002, 0.005, 0.0015, 2, "rubber")

# UVs: every face projected along its dominant axis into the part's atlas region (exact parts: their front
# face fills the region, so the screen picture / plate artwork map one to one)
tris_out, verts_out = [], []
from mathutils import Matrix
for o, reg, exact in parts:
    me = o.data
    me.transform(o.matrix_world); o.matrix_world = Matrix.Identity(4)   # world coordinates in the export
    bm = bmesh.new(); bm.from_mesh(me); bmesh.ops.triangulate(bm, faces=bm.faces[:]); bm.to_mesh(me); bm.free()
    for p in me.polygons:
        p.use_smooth = True
    me.set_sharp_from_angle(angle=math.radians(40))
    while len(me.uv_layers):
        me.uv_layers.remove(me.uv_layers[0])
    uvl = me.uv_layers.new(name="uv")
    xs = [v.co.x for v in me.vertices]; ys = [v.co.y for v in me.vertices]; zs = [v.co.z for v in me.vertices]
    bb = (min(xs), min(ys), min(zs), max(xs), max(ys), max(zs))
    u0, v0, u1, v1 = R[reg]
    pad = 0.0 if exact else 0.01
    for p in me.polygons:
        n = p.normal
        ax = max(range(3), key=lambda i: abs(n[i]))
        for li in p.loop_indices:
            co = me.vertices[me.loops[li].vertex_index].co
            if ax == 2:
                s = (co.x - bb[0]) / max(1e-6, bb[3] - bb[0]); t = (co.y - bb[1]) / max(1e-6, bb[4] - bb[1])
            elif ax == 0:
                s = (co.y - bb[1]) / max(1e-6, bb[4] - bb[1]); t = (co.z - bb[2]) / max(1e-6, bb[5] - bb[2])
            else:
                s = (co.x - bb[0]) / max(1e-6, bb[3] - bb[0]); t = (co.z - bb[2]) / max(1e-6, bb[5] - bb[2])
            if exact and ax != 2:
                s, t = 0.02, 0.02   # sides of the screen / plate: a corner pixel
            uvl.data[li].uv = (u0 + pad + (u1 - u0 - 2 * pad) * s, v0 + pad + (v1 - v0 - 2 * pad) * t)
    base = len(verts_out)
    verts_out += [list(v.co) for v in me.vertices]
    cn = me.corner_normals
    for p in me.polygons:
        tri = []
        for li in p.loop_indices:
            nn = cn[li].vector
            tri.append([base + me.loops[li].vertex_index, [nn.x, nn.y, nn.z], list(uvl.data[li].uv)])
        tri.append(0)
        tris_out.append(tri)
mesh = {"verts": verts_out, "tris": tris_out, "mats": ["kpk_admin_baked"]}
os.makedirs(OUT, exist_ok=True)
json.dump(mesh, open(OUT + "/mesh.json", "w"))
json.dump(mesh, open(OUT + "/mesh_lod1.json", "w"))
print("tris", len(tris_out), "verts", len(verts_out))

# material for the renders
mat = bpy.data.materials.new("atlas"); mat.use_nodes = True
nt = mat.node_tree; bsdf = nt.nodes["Principled BSDF"]
img = nt.nodes.new("ShaderNodeTexImage"); img.image = bpy.data.images.load(ATLAS)
nt.links.new(img.outputs["Color"], bsdf.inputs["Base Color"])
bsdf.inputs["Roughness"].default_value = 0.55
for o, reg, exact in parts:
    o.data.materials.append(mat)
bpy.ops.object.select_all(action='DESELECT')
for o, _, _ in parts:
    o.select_set(True)
bpy.context.view_layer.objects.active = parts[0][0]
bpy.ops.object.join(); ob = bpy.context.active_object; ob.name = "KPK"
bpy.ops.wm.save_as_mainfile(filepath=OUT + "/kpk_baked.blend")

# front ortho render for the interface frame: device body framed 2:1
sc = bpy.context.scene; sc.render.engine = 'CYCLES'; sc.cycles.samples = 64; sc.cycles.use_denoising = True
sc.render.film_transparent = True; sc.view_settings.view_transform = 'Standard'
sc.render.resolution_x, sc.render.resolution_y = 2048, 1024
w = bpy.data.worlds.new("w"); sc.world = w; w.use_nodes = True
w.node_tree.nodes["Background"].inputs[0].default_value = (0.5, 0.5, 0.5, 1); w.node_tree.nodes["Background"].inputs[1].default_value = 0.25
def light(loc, e, size):
    d = bpy.data.lights.new("l", 'AREA'); d.energy = e; d.size = size
    lo = bpy.data.objects.new("l", d); lo.location = loc
    lo.rotation_euler = (-Vector(loc)).to_track_quat('-Z', 'Y').to_euler(); sc.collection.objects.link(lo)
light((0.1, 0.15, 0.4), 3.5, 0.4); light((-0.2, -0.1, 0.3), 1.5, 0.5)
cam = bpy.data.cameras.new("c"); cam.type = 'ORTHO'
FW = 0.164; cam.ortho_scale = FW
co = bpy.data.objects.new("c", cam); sc.collection.objects.link(co); sc.camera = co
CY = 0.0
co.location = (0, CY, 0.5); co.rotation_euler = (0, 0, 0)
sc.render.filepath = OUT + "/ui_front.png"; bpy.ops.render.render(write_still=True)
FH = FW / 2
scr = [(SX0 + FW / 2) / FW, (CY + FH / 2 - SY1) / FH, (SX1 - SX0) / FW, (SY1 - SY0) / FH]
json.dump({"screen": scr, "aspect": 2.0}, open(OUT + "/ui.json", "w"))
print("UI", scr)
