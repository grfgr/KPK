# Layer 2 (Blender): bake one unique texture for the whole case: tiling materials + edge wear (worn to
# bare metal on edges / corners, found with the Bevel node) + grime in cavities (AO). Photo faces
# (screen, icons, labels) stay on their own sharp atlas. Rewrites mesh.json / mesh_lod1.json.
# usage: blender python bake_v4.py -- <name> <dir> [res]
import bpy, sys, json, math, os
args = sys.argv[sys.argv.index("--") + 1:]
name, out = args[0], args[1]; RES = int(args[2]) if len(args) > 2 else 2048
bpy.ops.wm.open_mainfile(filepath=out + "/kpk.blend")
ob = bpy.data.objects["KPK"]; me = ob.data
bpy.context.view_layer.objects.active = ob; ob.select_set(True)
sc = bpy.context.scene; sc.render.engine = 'CYCLES'; sc.cycles.samples = 24; sc.cycles.device = 'CPU'
PHOTO = [i for i, m in enumerate(me.materials) if m.name.startswith("photo")][0]

# ---- bake UV for every face (photo faces get squeezed into a corner, unused)
src = me.uv_layers["UV"]
bk = me.uv_layers.new(name="BAKE"); me.uv_layers.active = bk
bpy.ops.object.mode_set(mode='EDIT'); bpy.ops.mesh.select_all(action='SELECT')
bpy.ops.uv.smart_project(angle_limit=math.radians(60), island_margin=0.004, area_weight=0.0, scale_to_bounds=True)
bpy.ops.object.mode_set(mode='OBJECT')

img = bpy.data.images.new("baked", RES, RES, alpha=False); img.colorspace_settings.name = "sRGB"
edge_img = bpy.data.images.new("edge", RES, RES, alpha=False); edge_img.colorspace_settings.name = "Non-Color"
WEAR = {"body": (0.50, 0.47, 0.43), "trim": (0.66, 0.63, 0.58), "metal": (0.78, 0.76, 0.72), "weave": (0.55, 0.52, 0.40), "cloth": (0.5, 0.46, 0.38)}
DARK = {"body": 0.9, "trim": 0.85, "metal": 0.9, "weave": 0.85, "cloth": 0.85}


def setup(mat, target, mode):
    nt = mat.node_tree; N = nt.nodes; L = nt.links
    for n in list(N):
        if n.bl_idname not in ("ShaderNodeTexImage",) or n.name.startswith("BAKE"):
            N.remove(n)
    tex = [n for n in N if n.bl_idname == "ShaderNodeTexImage"]
    uvn = N.new("ShaderNodeUVMap"); uvn.uv_map = "UV"
    outn = N.new("ShaderNodeOutputMaterial")
    emi = N.new("ShaderNodeEmission")
    L.new(emi.outputs[0], outn.inputs[0])
    key = mat.name.split(".")[0]
    if tex:
        L.new(uvn.outputs[0], tex[0].inputs[0]); base = tex[0].outputs[0]
    else:
        rgb = N.new("ShaderNodeRGB"); rgb.outputs[0].default_value = (0.4, 0.4, 0.4, 1); base = rgb.outputs[0]
    # edge mask: 1 - N.bevelN
    bev = N.new("ShaderNodeBevel"); bev.inputs[0].default_value = 0.0035; bev.samples = 12
    geo = N.new("ShaderNodeNewGeometry")
    dot = N.new("ShaderNodeVectorMath"); dot.operation = 'DOT_PRODUCT'
    L.new(bev.outputs[0], dot.inputs[0]); L.new(geo.outputs["Normal"], dot.inputs[1])
    inv = N.new("ShaderNodeMath"); inv.operation = 'SUBTRACT'; inv.inputs[0].default_value = 1.0; L.new(dot.outputs["Value"], inv.inputs[1])
    amp = N.new("ShaderNodeMath"); amp.operation = 'MULTIPLY'; amp.inputs[1].default_value = 4.0; amp.use_clamp = True; L.new(inv.outputs[0], amp.inputs[0])
    # chipping noise breaks the edge line
    tc = N.new("ShaderNodeTexCoord")
    noi = N.new("ShaderNodeTexNoise"); noi.inputs["Scale"].default_value = 70.0; noi.inputs["Detail"].default_value = 10.0; noi.inputs["Roughness"].default_value = 0.65
    L.new(tc.outputs["Object"], noi.inputs["Vector"])
    ramp = N.new("ShaderNodeValToRGB"); ramp.color_ramp.elements[0].position = 0.38; ramp.color_ramp.elements[1].position = 0.58; ramp.color_ramp.elements[0].color = (0.25, 0.25, 0.25, 1)
    L.new(noi.outputs["Fac"], ramp.inputs[0])
    chip = N.new("ShaderNodeMath"); chip.operation = 'MULTIPLY'; chip.use_clamp = True
    L.new(amp.outputs[0], chip.inputs[0]); L.new(ramp.outputs[0], chip.inputs[1])
    # cavity: ambient occlusion
    ao = N.new("ShaderNodeAmbientOcclusion"); ao.inputs["Distance"].default_value = 0.008; ao.only_local = True; ao.samples = 12
    # big grime blotches
    noi2 = N.new("ShaderNodeTexNoise"); noi2.inputs["Scale"].default_value = 18.0; noi2.inputs["Detail"].default_value = 4.0
    L.new(tc.outputs["Object"], noi2.inputs["Vector"])
    # colour
    darken = N.new("ShaderNodeMixRGB"); darken.blend_type = 'MULTIPLY'; darken.inputs[0].default_value = 1.0
    L.new(base, darken.inputs[1]); d = DARK.get(key, 0.8); darken.inputs[2].default_value = (d, d, d, 1)
    blot = N.new("ShaderNodeMixRGB"); blot.blend_type = 'MULTIPLY'
    L.new(darken.outputs[0], blot.inputs[1]); blot.inputs[2].default_value = (0.62, 0.56, 0.48, 1)
    bl = N.new("ShaderNodeMath"); bl.operation = 'MULTIPLY'; bl.inputs[1].default_value = 0.55; L.new(noi2.outputs["Fac"], bl.inputs[0])
    bl2 = N.new("ShaderNodeMath"); bl2.operation = 'SUBTRACT'; bl2.use_clamp = True; L.new(bl.outputs[0], bl2.inputs[0]); bl2.inputs[1].default_value = 0.18
    L.new(bl2.outputs[0], blot.inputs[0])
    wear = N.new("ShaderNodeMixRGB"); wear.blend_type = 'MIX'
    L.new(chip.outputs[0], wear.inputs[0]); L.new(blot.outputs[0], wear.inputs[1]); w = WEAR.get(key, (0.6, 0.6, 0.6)); wear.inputs[2].default_value = (*w, 1)
    grime = N.new("ShaderNodeMixRGB"); grime.blend_type = 'MIX'
    cav = N.new("ShaderNodeMath"); cav.operation = 'SUBTRACT'; cav.inputs[0].default_value = 1.0; L.new(ao.outputs["AO"], cav.inputs[1])
    cav2 = N.new("ShaderNodeMath"); cav2.operation = 'MULTIPLY'; cav2.inputs[1].default_value = 2.2; cav2.use_clamp = True; L.new(cav.outputs[0], cav2.inputs[0])
    L.new(cav2.outputs[0], grime.inputs[0]); L.new(wear.outputs[0], grime.inputs[1]); grime.inputs[2].default_value = (0.06, 0.05, 0.035, 1)
    if mode == "color":
        L.new(grime.outputs[0], emi.inputs[0])
    else:
        L.new(chip.outputs[0], emi.inputs[0])
    t = N.new("ShaderNodeTexImage"); t.name = "BAKE"; t.image = target
    uvb = N.new("ShaderNodeUVMap"); uvb.uv_map = "BAKE"; L.new(uvb.outputs[0], t.inputs[0])
    N.active = t; t.select = True


for mode, target in (("color", img), ("edge", edge_img)):
    for i, mat in enumerate(me.materials):
        if i != PHOTO:
            setup(mat, target, mode)
        else:
            nt = mat.node_tree
            t = nt.nodes.get("BAKE") or nt.nodes.new("ShaderNodeTexImage"); t.name = "BAKE"; t.image = target; nt.nodes.active = t
    sc.render.bake.margin = 8; sc.render.bake.use_clear = True
    bpy.ops.object.bake(type='EMIT')
    target.filepath_raw = out + "/baked_%s.png" % mode; target.file_format = 'PNG'; target.save()
    print("baked", mode)

# ---- final UVs: baked faces -> BAKE, photo faces keep their atlas UVs
fin = me.uv_layers.new(name="FINAL")
src = me.uv_layers["UV"]; bk = me.uv_layers["BAKE"]; fin = me.uv_layers["FINAL"]
for p in me.polygons:
    use = src if p.material_index == PHOTO else bk
    for li in p.loop_indices:
        fin.data[li].uv = use.data[li].uv
me.uv_layers.active = fin


def dump(path, ratio=None):
    if ratio:
        m_ = ob.modifiers.new("dec", 'DECIMATE'); m_.ratio = ratio
    m = ob.evaluated_get(bpy.context.evaluated_depsgraph_get()).to_mesh(); m.calc_loop_triangles()
    uvl = m.uv_layers["FINAL"].data; cn = m.corner_normals
    mats = ["kpk_%s_baked" % name, "kpk_%s_photo" % name]
    data = {"verts": [list(v.co) for v in m.vertices], "tris": [], "mats": mats}
    for tri in m.loop_triangles:
        data["tris"].append([[m.loops[li].vertex_index, list(cn[li].vector), list(uvl[li].uv)] for li in tri.loops] + [1 if tri.material_index == PHOTO else 0])
    json.dump(data, open(path, "w"))
    if ratio:
        ob.modifiers.remove(ob.modifiers["dec"])
    print(path, len(data["tris"]))


dump(out + "/mesh.json"); dump(out + "/mesh_lod1.json", 0.5)
# preview material: baked image on BAKE uv
for i, mat in enumerate(me.materials):
    nt = mat.node_tree; N = nt.nodes; L = nt.links
    for n in list(N): N.remove(n)
    o_ = N.new("ShaderNodeOutputMaterial"); b = N.new("ShaderNodeBsdfPrincipled"); L.new(b.outputs[0], o_.inputs[0])
    t = N.new("ShaderNodeTexImage"); u = N.new("ShaderNodeUVMap"); u.uv_map = "FINAL"; L.new(u.outputs[0], t.inputs[0])
    t.image = bpy.data.images.load(out + "/photo_co.png") if i == PHOTO else img
    L.new(t.outputs[0], b.inputs["Base Color"])
    b.inputs["Roughness"].default_value = 0.6
    if i != PHOTO:
        e = N.new("ShaderNodeTexImage"); e.image = edge_img; L.new(u.outputs[0], e.inputs[0])
        r = N.new("ShaderNodeMath"); r.operation = 'SUBTRACT'; r.inputs[0].default_value = 0.72; L.new(e.outputs[0], r.inputs[1])
        L.new(r.outputs[0], b.inputs["Roughness"])
        mm = N.new("ShaderNodeMath"); mm.operation = 'MULTIPLY'; mm.inputs[1].default_value = 0.8; L.new(e.outputs[0], mm.inputs[0]); L.new(mm.outputs[0], b.inputs["Metallic"])
bpy.ops.file.pack_all()
bpy.ops.wm.save_as_mainfile(filepath=out + "/kpk_baked.blend")
print("bake ok")
