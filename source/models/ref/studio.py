# studio renders of a KPK blend: 4 views on one sheet. usage: blender python studio.py -- <blend> <out.jpg>
import bpy, sys, math
from mathutils import Vector
a = sys.argv[sys.argv.index("--") + 1:]
bpy.ops.wm.open_mainfile(filepath=a[0])
sc = bpy.context.scene; sc.render.engine = 'CYCLES'; sc.cycles.samples = 64; sc.cycles.use_denoising = True
try:
    sc.cycles.denoiser = 'OPENIMAGEDENOISE'
except Exception:
    pass
sc.render.resolution_x, sc.render.resolution_y = 900, 700
sc.view_settings.view_transform = 'AgX'
w = bpy.data.worlds.new("w"); sc.world = w; w.use_nodes = True
bgn = w.node_tree.nodes["Background"]; bgn.inputs[0].default_value = (0.045, 0.047, 0.05, 1); bgn.inputs[1].default_value = 0.8
def light(loc, e, size, col=(1, 1, 1)):
    d = bpy.data.lights.new("l", 'AREA'); d.energy = e; d.size = size; d.color = col
    o = bpy.data.objects.new("l", d); o.location = loc
    o.rotation_euler = (-Vector(loc)).to_track_quat('-Z', 'Y').to_euler(); sc.collection.objects.link(o)
light((0.35, -0.25, 0.45), 9, 0.35, (1, 0.96, 0.9))
light((-0.4, 0.05, 0.25), 3, 0.5, (0.85, 0.9, 1))
light((0.0, 0.45, 0.2), 4, 0.3)
light((0.0, 0.0, -0.5), 1.5, 0.6)
cam = bpy.data.cameras.new("c"); cam.lens = 85
co = bpy.data.objects.new("c", cam); sc.collection.objects.link(co); sc.camera = co
ob_ = bpy.data.objects["KPK"]
k_ = max(ob_.dimensions.x, ob_.dimensions.y) / 0.19
views = [(n_, tuple(v * k_ for v in l_)) for n_, l_ in [("front", (0.0, -0.06, 0.48)), ("q_left", (-0.28, -0.22, 0.30)), ("back", (0.12, 0.18, -0.42)), ("top", (0.05, 0.35, 0.16))]]
import os
tmp = []
for nm, loc in views:
    co.location = loc
    co.rotation_euler = (Vector((0, 0, 0)) - Vector(loc)).to_track_quat('-Z', 'Y').to_euler()
    p = os.path.splitext(a[1])[0] + "_" + nm + ".png"; sc.render.filepath = p; bpy.ops.render.render(write_still=True); tmp.append(p)
print("STUDIO", ";".join(tmp))
