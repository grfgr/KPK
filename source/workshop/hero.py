# Workshop cover: dramatic render of the broken KPK lying tilted on dark ground, warm key + cold rim.
# usage: blender python hero.py -- <kpk_baked.blend> <out.png>
import bpy, sys, math
from mathutils import Vector
a = sys.argv[sys.argv.index("--") + 1:]
bpy.ops.wm.open_mainfile(filepath=a[0])
sc = bpy.context.scene; sc.render.engine = 'CYCLES'; sc.cycles.samples = 160; sc.cycles.use_denoising = True
sc.render.resolution_x = sc.render.resolution_y = 1024
sc.view_settings.view_transform = 'AgX'; sc.view_settings.look = 'AgX - Punchy'
sc.render.film_transparent = True
w = bpy.data.worlds.new("w"); sc.world = w; w.use_nodes = True
bg = w.node_tree.nodes["Background"]; bg.inputs[0].default_value = (0.02, 0.025, 0.03, 1); bg.inputs[1].default_value = 0.4
ob = bpy.data.objects["KPK"]
ob.rotation_euler = (math.radians(-6), math.radians(10), math.radians(-14))
def light(loc, e, size, col):
    d = bpy.data.lights.new("l", 'AREA'); d.energy = e; d.size = size; d.color = col
    o = bpy.data.objects.new("l", d); o.location = loc
    o.rotation_euler = (-Vector(loc)).to_track_quat('-Z', 'Y').to_euler(); sc.collection.objects.link(o)
light((0.30, -0.30, 0.40), 30, 0.25, (1.0, 0.82, 0.6))     # warm key
light((-0.35, 0.30, 0.15), 22, 0.15, (0.55, 0.75, 1.0))    # cold rim
light((-0.1, -0.4, 0.05), 6, 0.5, (0.9, 0.9, 1.0))       # fill
cam = bpy.data.cameras.new("c"); cam.lens = 70
co = bpy.data.objects.new("c", cam); sc.collection.objects.link(co); sc.camera = co
k = max(ob.dimensions) / 0.11
loc = Vector((0.06, -0.13, 0.215)) * k
co.location = loc; co.rotation_euler = (-loc).to_track_quat('-Z', 'Y').to_euler()
sc.render.filepath = a[1]; bpy.ops.render.render(write_still=True)
print("HERO ok")
