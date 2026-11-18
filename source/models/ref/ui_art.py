# Front "photo" of a KPK for the in-game interface: orthographic render with a transparent background,
# plus where the screen is on it (normalised 0..1, top-left origin). The brick shows its top part down to
# the second key row and fades out below. usage: blender python ui_art.py -- <name> <out.png> <json>
import bpy, sys, json, os
from mathutils import Vector
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from cfg_v4 import CFG
a = sys.argv[sys.argv.index("--") + 1:]
name, outp, outj = a[0], a[1], a[2]
c = CFG[name]
d = os.path.join(os.path.dirname(os.path.abspath(__file__)), "v3_" + name)
bpy.ops.wm.open_mainfile(filepath=d + "/kpk_baked.blend")
sc = bpy.context.scene; sc.render.engine = 'CYCLES'; sc.cycles.samples = 96; sc.cycles.use_denoising = True
sc.render.film_transparent = True; sc.view_settings.view_transform = 'Standard'
brick = name == "brick"
RX, RY = (1024, 1024) if brick else (2048, 1024)
sc.render.resolution_x, sc.render.resolution_y = RX, RY
w = bpy.data.worlds.new("w"); sc.world = w; w.use_nodes = True
w.node_tree.nodes["Background"].inputs[0].default_value = (0.5, 0.5, 0.5, 1); w.node_tree.nodes["Background"].inputs[1].default_value = 0.35
ob = bpy.data.objects["KPK"]
for loc, e, s in (((-0.15, 0.25, 0.6), 7, 0.6), ((0.25, -0.1, 0.5), 3, 0.5), ((0, 0.6, 0.15), 2, 0.4)):
    l = bpy.data.lights.new("l", 'AREA'); l.energy = e; l.size = s
    o = bpy.data.objects.new("l", l); o.location = loc; o.rotation_euler = (-Vector(loc)).to_track_quat('-Z', 'Y').to_euler(); sc.collection.objects.link(o)
bx0, by0, bx1, by1 = c["body"]
MPP = c["W"] / 1000.0 / (bx1 - bx0); CX, CY = (bx0 + bx1) / 2.0, (by0 + by1) / 2.0
P = lambda px, py: ((px - CX) * MPP, (CY - py) * MPP)
# the mesh sits in the build coordinates: same P() as in build_v4
bb = [ob.matrix_world @ Vector(v) for v in ob.bound_box]
xs = [v.x for v in bb]; ys = [v.y for v in bb]
aspect = RX / RY
if brick:
    _, ytop = P(0, 92); _, ybot = P(0, 548)
    hgt = ytop - ybot; cx, cy = (min(xs) + max(xs)) / 2, (ytop + ybot) / 2
    scale = hgt * aspect
else:
    cx, cy = (min(xs) + max(xs)) / 2, (min(ys) + max(ys)) / 2
    scale = max((max(xs) - min(xs)) * 1.03, (max(ys) - min(ys)) * 1.03 * aspect)
cam = bpy.data.cameras.new("c"); cam.type = 'ORTHO'; cam.ortho_scale = scale
co = bpy.data.objects.new("c", cam); sc.collection.objects.link(co); sc.camera = co
co.location = (cx, cy, 0.5); co.rotation_euler = (0, 0, 0)
sc.render.filepath = outp; bpy.ops.render.render(write_still=True)
# screen rectangle on the image
sx0, sy0 = P(c["screen"][0], c["screen"][1]); sx1, sy1 = P(c["screen"][2], c["screen"][3])
def uv(x, y):
    return ((x - cx) / scale + 0.5, 0.5 - (y - cy) / (scale / aspect))
u0, v0 = uv(sx0, sy0); u1, v1 = uv(sx1, sy1)
info = dict(name=name, aspect=aspect, screen=[u0, v0, u1 - u0, v1 - v0])
if brick:
    _, yf0 = P(0, 470); _, yf1 = P(0, 548)
    info["fade"] = [uv(0, yf0)[1], uv(0, yf1)[1]]
json.dump(info, open(outj, "w"))
print("UIART", json.dumps(info))
