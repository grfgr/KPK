# Export a KPK model with its baked textures as .glb (Sketchfab / Steam preview).
# usage: blender python export_glb.py -- <kpk_baked.blend> <out.glb>
import bpy, sys
a = sys.argv[sys.argv.index("--") + 1:]
bpy.ops.wm.open_mainfile(filepath=a[0])
for o in list(bpy.data.objects):
    if o.type != 'MESH':
        bpy.data.objects.remove(o)
bpy.ops.export_scene.gltf(filepath=a[1], export_format='GLB', export_image_format='JPEG', export_apply=True)
print("GLB ok")
