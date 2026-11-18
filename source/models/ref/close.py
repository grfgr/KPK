import bpy, sys
from mathutils import Vector
a = sys.argv[sys.argv.index("--") + 1:]
bpy.ops.wm.open_mainfile(filepath=a[0])
sc = bpy.context.scene; sc.render.engine = 'CYCLES'; sc.cycles.samples = 40
sc.render.resolution_x, sc.render.resolution_y = 800, 600
w = bpy.data.worlds.new("w"); sc.world = w; w.use_nodes = True
w.node_tree.nodes["Background"].inputs[0].default_value = (0.06, 0.065, 0.06, 1)
w.node_tree.nodes["Background"].inputs[1].default_value = 0.5
for loc, e in (((0.3, -0.1, 0.3), 6), ((-0.2, 0.2, 0.15), 1.5)):
    d = bpy.data.lights.new("l", 'AREA'); d.energy = e; d.size = 0.15
    o = bpy.data.objects.new("l", d); o.location = loc
    o.rotation_euler = (-Vector(loc)).to_track_quat('-Z', 'Y').to_euler(); sc.collection.objects.link(o)
cam = bpy.data.cameras.new("c"); cam.lens = 60
co = bpy.data.objects.new("c", cam); sc.collection.objects.link(co); sc.camera = co
tx = float(a[2])
co.location = (tx + 0.10, -0.07, 0.07)
co.rotation_euler = (Vector((tx, 0, 0.01)) - co.location).to_track_quat('-Z', 'Y').to_euler()
sc.render.filepath = a[1]; bpy.ops.render.render(write_still=True)
