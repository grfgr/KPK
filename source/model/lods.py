import bpy, sys, json
args = sys.argv[sys.argv.index("--") + 1:]
bpy.ops.wm.open_mainfile(filepath=args[0])
kpk = bpy.data.objects["KPK"]
bpy.context.view_layer.objects.active = kpk
def dump(ob, path):
    me = ob.evaluated_get(bpy.context.evaluated_depsgraph_get()).to_mesh()
    me.calc_loop_triangles()
    uvl = me.uv_layers.active.data
    cn = me.corner_normals
    data = {"verts": [list(v.co) for v in me.vertices], "tris": []}
    for tri in me.loop_triangles:
        data["tris"].append([[me.loops[li].vertex_index, list(cn[li].vector), list(uvl[li].uv)] for li in tri.loops])
    json.dump(data, open(path, "w"))
    print(path, len(data["tris"]))
m = kpk.modifiers.new("dec", 'DECIMATE')
m.ratio = 0.3
dump(kpk, args[1] + "/mesh_lod1.json")
