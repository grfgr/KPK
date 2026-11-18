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
# faces nobody sees: photo faces (own atlas) and faces covered by another part (ray from the face centre
# along its normal hits something within 0.3 mm)
from mathutils.bvhtree import BVHTree
import bmesh as _bm
bmv = _bm.new(); bmv.from_mesh(me); bmv.faces.ensure_lookup_table()
tree = BVHTree.FromBMesh(bmv)
hidden = set()
for f in bmv.faces:
    if f.material_index == PHOTO:
        hidden.add(f.index); continue
    c_ = f.calc_center_median(); n_ = f.normal
    hit = tree.ray_cast(c_ + n_ * 0.00002, n_, 0.0004)
    if hit[0] is not None:
        hidden.add(f.index)
bmv.free()
print("hidden faces", len(hidden), "of", len(me.polygons))
bk = me.uv_layers["BAKE"]
for p in me.polygons:
    if p.index in hidden:
        for li in p.loop_indices:
            bk.data[li].uv = (0.999, 0.999)
bpy.ops.object.mode_set(mode='EDIT'); bpy.ops.mesh.select_all(action='DESELECT')
bpy.ops.object.mode_set(mode='OBJECT')
for p in me.polygons:
    p.select = p.index not in hidden
bpy.ops.object.mode_set(mode='EDIT')
bpy.context.scene.tool_settings.use_uv_select_sync = True
bpy.ops.uv.select_all(action='SELECT')
bpy.ops.uv.pack_islands(rotate=True, margin=0.003)
bpy.ops.object.mode_set(mode='OBJECT')

img = bpy.data.images.new("baked", RES, RES, alpha=False); img.colorspace_settings.name = "sRGB"
edge_img = bpy.data.images.new("edge", RES, RES, alpha=False); edge_img.colorspace_settings.name = "Non-Color"
WEAR = {"body": (0.50, 0.47, 0.43), "trim": (0.66, 0.63, 0.58), "metal": (0.78, 0.76, 0.72), "weave": (0.55, 0.52, 0.40), "cloth": (0.5, 0.46, 0.38)}
WEATHER = {"body": 1.0, "trim": 0.8, "metal": 0.5, "weave": 0.3, "cloth": 0.3}
RUST = {"body": 1.0, "trim": 0.6, "metal": 0.4, "weave": 0.0, "cloth": 0.0}
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
    # ---- layer 3: heavier wear
    w3 = WEATHER.get(key, 1.0)
    # big paint chips on edges: bigger noise, shows dark primer / bare steel
    noi3 = N.new("ShaderNodeTexNoise"); noi3.inputs["Scale"].default_value = 22.0; noi3.inputs["Detail"].default_value = 12.0; noi3.inputs["Roughness"].default_value = 0.7
    L.new(tc.outputs["Object"], noi3.inputs["Vector"])
    r3 = N.new("ShaderNodeMapRange"); r3.inputs[1].default_value = 0.52; r3.inputs[2].default_value = 0.6; L.new(noi3.outputs["Fac"], r3.inputs[0])
    edge2 = N.new("ShaderNodeMapRange"); edge2.inputs[1].default_value = 0.04; edge2.inputs[2].default_value = 0.22; L.new(inv.outputs[0], edge2.inputs[0])
    big = N.new("ShaderNodeMath"); big.operation = 'MULTIPLY'; big.use_clamp = True; L.new(r3.outputs[0], big.inputs[0]); L.new(edge2.outputs[0], big.inputs[1])
    bigw = N.new("ShaderNodeMath"); bigw.operation = 'MULTIPLY'; bigw.inputs[1].default_value = w3; L.new(big.outputs[0], bigw.inputs[0])
    chips = N.new("ShaderNodeMixRGB"); L.new(bigw.outputs[0], chips.inputs[0]); L.new(grime.outputs[0], chips.inputs[1]); chips.inputs[2].default_value = (0.13, 0.12, 0.11, 1)
    # rust streaks: from cavities, stretched downwards (object Y)
    mp = N.new("ShaderNodeMapping"); mp.inputs["Scale"].default_value = (28.0, 5.0, 28.0); L.new(tc.outputs["Object"], mp.inputs["Vector"])
    st = N.new("ShaderNodeTexNoise"); st.inputs["Scale"].default_value = 4.0; st.inputs["Detail"].default_value = 10.0; st.inputs["Distortion"].default_value = 0.0; L.new(mp.outputs[0], st.inputs["Vector"])
    stm = N.new("ShaderNodeMapRange"); stm.inputs[1].default_value = 0.42; stm.inputs[2].default_value = 0.7; L.new(st.outputs["Fac"], stm.inputs[0])
    cavs = N.new("ShaderNodeMath"); cavs.operation = 'MULTIPLY'; cavs.inputs[1].default_value = 3.0; cavs.use_clamp = True; L.new(cav.outputs[0], cavs.inputs[0])
    blur = N.new("ShaderNodeMath"); blur.operation = 'ADD'; blur.inputs[1].default_value = 0.0; L.new(cavs.outputs[0], blur.inputs[0])
    rmask = N.new("ShaderNodeMath"); rmask.operation = 'MULTIPLY'; rmask.use_clamp = True; L.new(stm.outputs[0], rmask.inputs[0])
    patch = N.new("ShaderNodeTexNoise"); patch.inputs["Scale"].default_value = 9.0; L.new(tc.outputs["Object"], patch.inputs["Vector"])
    pm = N.new("ShaderNodeMapRange"); pm.inputs[1].default_value = 0.45; pm.inputs[2].default_value = 0.7; L.new(patch.outputs["Fac"], pm.inputs[0])
    pmw = N.new("ShaderNodeMath"); pmw.operation = 'MULTIPLY'; pmw.inputs[1].default_value = 0.0; L.new(pm.outputs[0], pmw.inputs[0])
    mx = N.new("ShaderNodeMath"); mx.operation = 'MAXIMUM'; L.new(cavs.outputs[0], mx.inputs[0]); L.new(pmw.outputs[0], mx.inputs[1])
    L.new(mx.outputs[0], rmask.inputs[1])
    rw = N.new("ShaderNodeMath"); rw.operation = 'MULTIPLY'; rw.inputs[1].default_value = 0.5 * RUST.get(key, 0.0); L.new(rmask.outputs[0], rw.inputs[0])
    rust = N.new("ShaderNodeMixRGB"); L.new(rw.outputs[0], rust.inputs[0]); L.new(chips.outputs[0], rust.inputs[1]); rust.inputs[2].default_value = (0.30, 0.13, 0.05, 1)
    # fine scratches: thin voronoi cell borders
    vor = N.new("ShaderNodeTexVoronoi"); vor.feature = 'DISTANCE_TO_EDGE'; vor.inputs["Scale"].default_value = 140.0
    mpv = N.new("ShaderNodeMapping"); mpv.inputs["Scale"].default_value = (1.0, 3.0, 1.0); mpv.inputs["Rotation"].default_value = (0, 0, 0.6); L.new(tc.outputs["Object"], mpv.inputs["Vector"]); L.new(mpv.outputs[0], vor.inputs["Vector"])
    vr = N.new("ShaderNodeMapRange"); vr.inputs[1].default_value = 0.02; vr.inputs[2].default_value = 0.0; L.new(vor.outputs["Distance"], vr.inputs[0])
    vmask = N.new("ShaderNodeTexNoise"); vmask.inputs["Scale"].default_value = 6.0; L.new(tc.outputs["Object"], vmask.inputs["Vector"])
    vm = N.new("ShaderNodeMapRange"); vm.inputs[1].default_value = 0.5; vm.inputs[2].default_value = 0.65; L.new(vmask.outputs["Fac"], vm.inputs[0])
    scr = N.new("ShaderNodeMath"); scr.operation = 'MULTIPLY'; scr.use_clamp = True; L.new(vr.outputs[0], scr.inputs[0]); L.new(vm.outputs[0], scr.inputs[1])
    scw = N.new("ShaderNodeMath"); scw.operation = 'MULTIPLY'; scw.inputs[1].default_value = 0.0; L.new(scr.outputs[0], scw.inputs[0])
    scc = N.new("ShaderNodeMixRGB"); L.new(scw.outputs[0], scc.inputs[0]); L.new(rust.outputs[0], scc.inputs[1]); scc.inputs[2].default_value = (0.55, 0.52, 0.47, 1)
    # polished by thumbs: both short ends of the front
    sep = N.new("ShaderNodeSeparateXYZ"); L.new(tc.outputs["Object"], sep.inputs[0])
    ax_ = N.new("ShaderNodeMath"); ax_.operation = 'ABSOLUTE'; L.new(sep.outputs[0], ax_.inputs[0])
    gp = N.new("ShaderNodeMapRange"); gp.inputs[1].default_value = 0.074; gp.inputs[2].default_value = 0.09; L.new(ax_.outputs[0], gp.inputs[0])
    gpn = N.new("ShaderNodeMath"); gpn.operation = 'MULTIPLY'; gpn.inputs[1].default_value = 0.07 * w3; L.new(gp.outputs[0], gpn.inputs[0])
    pol = N.new("ShaderNodeMixRGB"); pol.blend_type = 'SCREEN'; L.new(gpn.outputs[0], pol.inputs[0]); L.new(scc.outputs[0], pol.inputs[1]); pol.inputs[2].default_value = (0.35, 0.33, 0.30, 1)
    if mode == "color":
        L.new(pol.outputs[0], emi.inputs[0])
    else:
        sh = N.new("ShaderNodeMath"); sh.operation = 'MAXIMUM'; L.new(chip.outputs[0], sh.inputs[0]); L.new(bigw.outputs[0], sh.inputs[1])
        sh2 = N.new("ShaderNodeMath"); sh2.operation = 'MAXIMUM'; L.new(sh.outputs[0], sh2.inputs[0]); L.new(gpn.outputs[0], sh2.inputs[1])
        L.new(sh2.outputs[0], emi.inputs[0])
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
