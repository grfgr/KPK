import bpy, sys, math
from mathutils import Vector
args = sys.argv[sys.argv.index("--") + 1:]
blend, out = args[0], args[1]
bpy.ops.wm.open_mainfile(filepath=blend)
sc = bpy.context.scene
sc.render.engine = 'CYCLES'
sc.cycles.samples = 48
sc.cycles.use_denoising = False
sc.render.resolution_x, sc.render.resolution_y = 1100, 900
sc.render.film_transparent = False
sc.view_settings.view_transform = 'Standard'
world = bpy.data.worlds.new("w"); sc.world = world
world.use_nodes = True
world.node_tree.nodes["Background"].inputs[0].default_value = (0.06, 0.065, 0.06, 1)
world.node_tree.nodes["Background"].inputs[1].default_value = 0.6
def light(loc, energy, size=0.2):
    d = bpy.data.lights.new("l", 'AREA'); d.energy = energy; d.size = size
    o = bpy.data.objects.new("l", d); o.location = loc
    o.rotation_euler = (Vector((0, 0, 0)) - Vector(loc)).to_track_quat('-Z', 'Y').to_euler()
    sc.collection.objects.link(o)
light((0.25, -0.2, 0.35), 5)
light((-0.3, 0.1, 0.2), 1.5)
light((0.0, 0.3, -0.1), 2)
cam = bpy.data.cameras.new("c"); cam.lens = 70
co = bpy.data.objects.new("c", cam); sc.collection.objects.link(co); sc.camera = co
views = {"front": (0.05, -0.30, 0.42), "back": (-0.12, 0.20, -0.42), "angle": (0.36, -0.22, 0.26)}
target = Vector((0, 0.0, 0))
for name, loc in views.items():
    co.location = loc
    co.rotation_euler = (target - Vector(loc)).to_track_quat('-Z', 'Y').to_euler()
    sc.render.filepath = out + "/preview_" + name + ".png"
    bpy.ops.render.render(write_still=True)
print("previews ok")
