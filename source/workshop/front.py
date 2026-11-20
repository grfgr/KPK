# Front 3/4 render of a KPK with a transparent background, model upright (for compositions like a fan).
# usage: blender python front.py -- <kpk_baked.blend> <out.png>
import bpy, sys, math
from mathutils import Vector
a = sys.argv[sys.argv.index("--") + 1:]
bpy.ops.wm.open_mainfile(filepath=a[0])
sc = bpy.context.scene; sc.render.engine = 'CYCLES'; sc.cycles.samples = 128; sc.cycles.use_denoising = True
sc.render.resolution_x, sc.render.resolution_y = 1400, 800
sc.view_settings.view_transform = 'AgX'; sc.view_settings.look = 'AgX - Punchy'
sc.render.film_transparent = True
w = bpy.data.worlds.new("w"); sc.world = w; w.use_nodes = True
w.node_tree.nodes["Background"].inputs[0].default_value = (0.03, 0.035, 0.04, 1); w.node_tree.nodes["Background"].inputs[1].default_value = 0.5
ob = bpy.data.objects["KPK"]
ob.rotation_euler = (math.radians(-12), math.radians(8), 0)
def light(loc, e, size, col):
    d = bpy.data.lights.new("l", 'AREA'); d.energy = e; d.size = size; d.color = col
    o = bpy.data.objects.new("l", d); o.location = loc
    o.rotation_euler = (-Vector(loc)).to_track_quat('-Z', 'Y').to_euler(); sc.collection.objects.link(o)
light((0.25, -0.25, 0.45), 26, 0.3, (1.0, 0.85, 0.65))
light((-0.35, 0.2, 0.2), 16, 0.2, (0.6, 0.8, 1.0))
light((0.0, -0.4, 0.1), 5, 0.6, (1, 1, 1))
cam = bpy.data.cameras.new("c"); cam.type = 'ORTHO'
cam.ortho_scale = max(ob.dimensions.x, ob.dimensions.y) * 1.12
co = bpy.data.objects.new("c", cam); sc.collection.objects.link(co); sc.camera = co
co.location = (0, 0, 0.6); co.rotation_euler = (0, 0, 0)
sc.render.filepath = a[1]; bpy.ops.render.render(write_still=True)
print("FRONT ok")
