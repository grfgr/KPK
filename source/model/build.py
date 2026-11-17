# Builds the KPK model, bakes worn textures, exports FBX/OBJ and mesh data for the P3D writer.
# Blender coords while modelling: X width, Y thickness (screen faces -Y), Z length (antenna up).
import bpy, bmesh, math, os, sys, json
from mathutils import Vector, Matrix

OUT = sys.argv[sys.argv.index("--") + 1]
RES = int(sys.argv[sys.argv.index("--") + 2]) if len(sys.argv) > sys.argv.index("--") + 2 else 2048
W, L, T = 0.078, 0.140, 0.028

bpy.ops.wm.read_factory_settings(use_empty=True)
scene = bpy.context.scene

# ------------------------------------------------------------------ geometry helpers

def apply_mods(ob):
    bpy.context.view_layer.objects.active = ob
    for m in list(ob.modifiers):
        bpy.ops.object.modifier_apply(modifier=m.name)

def box(name, size, loc, bevel=0.0, segs=2, mat=None):
    bpy.ops.mesh.primitive_cube_add(size=1, location=loc)
    ob = bpy.context.active_object
    ob.name = name
    ob.scale = size
    bpy.ops.object.transform_apply(scale=True)
    if bevel > 0:
        m = ob.modifiers.new("bev", 'BEVEL')
        m.width = bevel
        m.segments = segs
        m.limit_method = 'ANGLE'
        apply_mods(ob)
    if mat:
        ob.data.materials.append(mat)
    return ob

def cyl(name, r, depth, loc, axis='Z', verts=20, mat=None, bevel=0.0):
    bpy.ops.mesh.primitive_cylinder_add(vertices=verts, radius=r, depth=depth, location=loc)
    ob = bpy.context.active_object
    ob.name = name
    if axis == 'Y':
        ob.rotation_euler = (math.radians(90), 0, 0)
        bpy.ops.object.transform_apply(rotation=True)
    if bevel > 0:
        m = ob.modifiers.new("bev", 'BEVEL')
        m.width = bevel
        m.segments = 2
        m.limit_method = 'ANGLE'
        apply_mods(ob)
    if mat:
        ob.data.materials.append(mat)
    return ob

def cut(target, cutter):
    m = target.modifiers.new("cut", 'BOOLEAN')
    m.operation = 'DIFFERENCE'
    m.object = cutter
    m.solver = 'EXACT'
    m.material_mode = 'INDEX'   # cut faces take the target's material, not an empty slot
    apply_mods(target)
    bpy.data.objects.remove(cutter, do_unlink=True)

# ------------------------------------------------------------------ materials

def node(nt, kind, x=0, y=0, **props):
    n = nt.nodes.new(kind)
    n.location = (x, y)
    for k, v in props.items():
        setattr(n, k, v)
    return n

def math_node(nt, op, a=None, b=None, clamp=False):
    n = nt.nodes.new('ShaderNodeMath')
    n.operation = op
    n.use_clamp = clamp
    for i, v in enumerate((a, b)):
        if v is None:
            continue
        if isinstance(v, (int, float)):
            n.inputs[i].default_value = v
        else:
            nt.links.new(v, n.inputs[i])
    return n.outputs[0]

def mix(nt, fac, a, b):
    n = nt.nodes.new('ShaderNodeMix')
    n.data_type = 'RGBA'
    for sock, v in ((n.inputs[0], fac), (n.inputs[6], a), (n.inputs[7], b)):
        if isinstance(v, (int, float)):
            sock.default_value = v
        elif isinstance(v, tuple):
            sock.default_value = v
        else:
            nt.links.new(v, sock)
    return n.outputs[2]

def mixf(nt, fac, a, b):
    n = nt.nodes.new('ShaderNodeMix')
    n.data_type = 'FLOAT'
    for sock, v in ((n.inputs[0], fac), (n.inputs[2], a), (n.inputs[3], b)):
        if isinstance(v, (int, float)):
            sock.default_value = v
        else:
            nt.links.new(v, sock)
    return n.outputs[0]

def ramp(nt, inp, p0, p1):
    n = nt.nodes.new('ShaderNodeMapRange')
    n.clamp = True
    n.inputs[1].default_value = p0
    n.inputs[2].default_value = p1
    nt.links.new(inp, n.inputs[0])
    return n.outputs[0]

def image_node(nt, path, colorspace='sRGB'):
    img = bpy.data.images.load(path)
    img.colorspace_settings.name = colorspace
    n = nt.nodes.new('ShaderNodeTexImage')
    n.image = img
    n.extension = 'CLIP'
    return n

def planar_uv(nt, x0, w, z0, h):
    """object coords -> (u, v) on the XZ plane"""
    tc = nt.nodes.new('ShaderNodeTexCoord')
    sep = nt.nodes.new('ShaderNodeSeparateXYZ')
    nt.links.new(tc.outputs['Object'], sep.inputs[0])
    u = math_node(nt, 'DIVIDE', math_node(nt, 'SUBTRACT', sep.outputs[0], x0), w)
    v = math_node(nt, 'DIVIDE', math_node(nt, 'SUBTRACT', sep.outputs[2], z0), h)
    comb = nt.nodes.new('ShaderNodeCombineXYZ')
    nt.links.new(u, comb.inputs[0])
    nt.links.new(v, comb.inputs[1])
    return comb.outputs[0]

def common_masks(nt, chip_scale=600.0, edge_width=0.0012):
    """edge wear, cavity dirt, scratches; returns sockets"""
    geo = nt.nodes.new('ShaderNodeNewGeometry')
    bev = nt.nodes.new('ShaderNodeBevel')
    bev.samples = 8
    bev.inputs['Radius'].default_value = edge_width
    dot = nt.nodes.new('ShaderNodeVectorMath')
    dot.operation = 'DOT_PRODUCT'
    nt.links.new(bev.outputs[0], dot.inputs[0])
    nt.links.new(geo.outputs['Normal'], dot.inputs[1])
    edge = ramp(nt, math_node(nt, 'SUBTRACT', 1.0, dot.outputs['Value']), 0.015, 0.09)

    tc = nt.nodes.new('ShaderNodeTexCoord')
    noise = nt.nodes.new('ShaderNodeTexNoise')
    noise.inputs['Scale'].default_value = chip_scale
    noise.inputs['Detail'].default_value = 8
    noise.inputs['Roughness'].default_value = 0.65
    nt.links.new(tc.outputs['Object'], noise.inputs['Vector'])
    big = nt.nodes.new('ShaderNodeTexNoise')
    big.inputs['Scale'].default_value = 60
    big.inputs['Detail'].default_value = 4
    nt.links.new(tc.outputs['Object'], big.inputs['Vector'])

    # chipped paint: edges plus a few random chips on flat areas
    chip_in = math_node(nt, 'ADD', math_node(nt, 'MULTIPLY', edge, 0.9), math_node(nt, 'MULTIPLY', big.outputs['Fac'], 0.35))
    chip_in = math_node(nt, 'ADD', chip_in, math_node(nt, 'MULTIPLY', noise.outputs['Fac'], 0.55))
    wear = ramp(nt, chip_in, 0.92, 0.98)

    ao = nt.nodes.new('ShaderNodeAmbientOcclusion')
    ao.samples = 16
    ao.only_local = True
    ao.inputs['Distance'].default_value = 0.005
    dirt = ramp(nt, math_node(nt, 'SUBTRACT', 1.0, ao.outputs['AO']), 0.05, 0.6)
    dirt = math_node(nt, 'MULTIPLY', dirt, ramp(nt, big.outputs['Fac'], 0.3, 0.7))

    # scratches: stretched voronoi edges in two directions
    def scratch_layer(scale, rot):
        mp = nt.nodes.new('ShaderNodeMapping')
        mp.inputs['Scale'].default_value = scale
        mp.inputs['Rotation'].default_value = rot
        nt.links.new(tc.outputs['Object'], mp.inputs['Vector'])
        vo = nt.nodes.new('ShaderNodeTexVoronoi')
        vo.feature = 'DISTANCE_TO_EDGE'
        vo.inputs['Scale'].default_value = 1.0
        nt.links.new(mp.outputs[0], vo.inputs['Vector'])
        line = ramp(nt, vo.outputs['Distance'], 0.01, 0.0)
        gate = nt.nodes.new('ShaderNodeTexNoise')
        gate.inputs['Scale'].default_value = 25
        nt.links.new(tc.outputs['Object'], gate.inputs['Vector'])
        return math_node(nt, 'MULTIPLY', line, ramp(nt, gate.outputs['Fac'], 0.5, 0.6))
    # every layer is stretched along a different axis, so each face gets long thin strokes
    s1 = scratch_layer((900, 900, 45), (0.3, 0.2, 0.4))
    s2 = scratch_layer((45, 900, 900), (0.2, -0.4, 0.3))
    s3 = scratch_layer((900, 45, 900), (-0.5, 0.3, 0.1))
    scratch = math_node(nt, 'MAXIMUM', math_node(nt, 'MAXIMUM', s1, s2), s3)
    return {'edge': edge, 'wear': wear, 'dirt': dirt, 'scratch': scratch, 'noise': noise.outputs['Fac'], 'big': big.outputs['Fac'], 'geo': geo}

def decal_layer(nt, masks):
    """front/back decal images projected along Y; returns (color, alpha)"""
    uv = planar_uv(nt, -W / 2, W, -L / 2, L)
    fr = image_node(nt, OUT + "/front.png")
    bk = image_node(nt, OUT + "/back.png")
    nt.links.new(uv, fr.inputs[0])
    nt.links.new(uv, bk.inputs[0])
    sepn = nt.nodes.new('ShaderNodeSeparateXYZ')
    nt.links.new(masks['geo'].outputs['Normal'], sepn.inputs[0])
    front = ramp(nt, sepn.outputs[1], -0.6, -0.8)
    back = ramp(nt, sepn.outputs[1], 0.6, 0.8)
    a_f = math_node(nt, 'MULTIPLY', fr.outputs['Alpha'], front)
    a_b = math_node(nt, 'MULTIPLY', bk.outputs['Alpha'], back)
    col = mix(nt, a_b, mix(nt, a_f, (0, 0, 0, 1), fr.outputs['Color']), bk.outputs['Color'])
    alpha = math_node(nt, 'MAXIMUM', a_f, a_b)
    # decals wear off with the paint and get scratched
    alpha = math_node(nt, 'MULTIPLY', alpha, math_node(nt, 'SUBTRACT', 1.0, masks['wear']))
    alpha = math_node(nt, 'MULTIPLY', alpha, ramp(nt, masks['noise'], 0.25, 0.4))
    alpha = math_node(nt, 'MULTIPLY', alpha, math_node(nt, 'SUBTRACT', 1.0, masks['scratch']))
    return col, alpha

def finish(nt, color, rough, metal, height, spec_mask):
    out = nt.nodes.new('ShaderNodeOutputMaterial')
    bsdf = nt.nodes.new('ShaderNodeBsdfPrincipled')
    nt.links.new(color, bsdf.inputs['Base Color'])
    for name, v in (('Roughness', rough), ('Metallic', metal)):
        if isinstance(v, (int, float)):
            bsdf.inputs[name].default_value = v
        else:
            nt.links.new(v, bsdf.inputs[name])
    bump = nt.nodes.new('ShaderNodeBump')
    bump.inputs['Strength'].default_value = 1.0
    bump.inputs['Distance'].default_value = 0.0008
    nt.links.new(height, bump.inputs['Height'])
    nt.links.new(bump.outputs['Normal'], bsdf.inputs['Normal'])
    nt.links.new(bsdf.outputs[0], out.inputs['Surface'])
    # spec mask is kept on a dedicated emission node for the EMIT bake
    em = nt.nodes.new('ShaderNodeEmission')
    em.name = "SPEC_EMIT"
    if isinstance(spec_mask, (int, float)):
        em.inputs['Color'].default_value = (spec_mask, spec_mask, spec_mask, 1)
    else:
        nt.links.new(spec_mask, em.inputs['Color'])
    out.name = "OUT"
    bsdf.name = "BSDF"

def new_mat(name):
    m = bpy.data.materials.new(name)
    m.use_nodes = True
    m.node_tree.nodes.clear()
    return m, m.node_tree

def to_rgb(nt, f):
    c = nt.nodes.new('ShaderNodeCombineColor')
    for i in range(3):
        nt.links.new(f, c.inputs[i])
    return c.outputs[0]

# painted metal body, olive drab, chipped to dark steel
def mat_paint(name, base):
    m, nt = new_mat(name)
    k = common_masks(nt)
    var = ramp(nt, k['big'], 0.2, 0.8)
    paint = mix(nt, var, base, tuple(min(1, c * 1.35) for c in base[:3]) + (1,))
    # sun-bleached top
    paint = mix(nt, math_node(nt, 'MULTIPLY', ramp(nt, k['noise'], 0.45, 0.7), 0.15), paint, (0.22, 0.22, 0.17, 1))
    steel = mix(nt, k['noise'], (0.05, 0.05, 0.05, 1), (0.13, 0.12, 0.11, 1))
    col = mix(nt, k['wear'], paint, steel)
    dcol, dalpha = decal_layer(nt, k)
    col = mix(nt, dalpha, col, dcol)
    col = mix(nt, math_node(nt, 'MULTIPLY', k['scratch'], 0.7), col, (0.32, 0.31, 0.28, 1))
    col = mix(nt, math_node(nt, 'MULTIPLY', k['dirt'], 0.85), col, (0.035, 0.03, 0.02, 1))
    rough = mixf(nt, k['wear'], 0.62, 0.38)
    rough = mixf(nt, k['dirt'], rough, 0.9)
    metal = math_node(nt, 'MAXIMUM', k['wear'], math_node(nt, 'MULTIPLY', k['scratch'], 0.8))
    height = math_node(nt, 'SUBTRACT', math_node(nt, 'MULTIPLY', k['noise'], 0.15),
                       math_node(nt, 'ADD', math_node(nt, 'MULTIPLY', k['wear'], 0.5), math_node(nt, 'MULTIPLY', k['scratch'], 0.8)))
    spec = to_rgb(nt, math_node(nt, 'ADD', 0.12, math_node(nt, 'MULTIPLY', metal, 0.7)))
    finish(nt, col, rough, metal, height, spec)
    return m

def mat_rubber():
    m, nt = new_mat("rubber")
    k = common_masks(nt, chip_scale=900)
    base = mix(nt, k['noise'], (0.016, 0.016, 0.015, 1), (0.032, 0.032, 0.03, 1))
    col = mix(nt, math_node(nt, 'MULTIPLY', k['edge'], 0.45), base, (0.09, 0.087, 0.08, 1))  # rubbed edges
    dcol, dalpha = decal_layer(nt, k)
    col = mix(nt, dalpha, col, dcol)
    col = mix(nt, math_node(nt, 'MULTIPLY', k['dirt'], 0.4), col, (0.11, 0.095, 0.07, 1))  # dust in cracks
    col = mix(nt, math_node(nt, 'MULTIPLY', k['scratch'], 0.4), col, (0.09, 0.09, 0.085, 1))
    height = math_node(nt, 'MULTIPLY', k['noise'], 0.3)
    finish(nt, col, 0.88, 0.0, height, 0.05)
    return m

def mat_metal():
    m, nt = new_mat("metal")
    k = common_masks(nt)
    steel = mix(nt, k['noise'], (0.20, 0.20, 0.19, 1), (0.32, 0.31, 0.29, 1))
    rust_mask = ramp(nt, math_node(nt, 'ADD', k['big'], math_node(nt, 'MULTIPLY', k['dirt'], 0.6)), 0.62, 0.8)
    col = mix(nt, rust_mask, steel, (0.20, 0.08, 0.025, 1))
    col = mix(nt, math_node(nt, 'MULTIPLY', k['dirt'], 0.8), col, (0.03, 0.025, 0.02, 1))
    rough = mixf(nt, rust_mask, 0.35, 0.9)
    metal = math_node(nt, 'SUBTRACT', 1.0, rust_mask)
    height = math_node(nt, 'ADD', math_node(nt, 'MULTIPLY', rust_mask, 0.4), math_node(nt, 'MULTIPLY', k['noise'], 0.1))
    spec = to_rgb(nt, math_node(nt, 'MULTIPLY', metal, 0.85))
    finish(nt, col, rough, metal, height, spec)
    return m

def mat_screen():
    m, nt = new_mat("screen")
    k = common_masks(nt)
    uv = planar_uv(nt, -0.028, 0.056, 0.008, 0.048)
    img = image_node(nt, OUT + "/screen.png")
    nt.links.new(uv, img.inputs[0])
    col = mix(nt, 0.15, img.outputs['Color'], (0.0, 0.02, 0.01, 1))
    col = mix(nt, math_node(nt, 'MULTIPLY', k['scratch'], 0.5), col, (0.25, 0.3, 0.27, 1))
    col = mix(nt, math_node(nt, 'MULTIPLY', ramp(nt, k['big'], 0.55, 0.8), 0.35), col, (0.16, 0.15, 0.12, 1))  # dust
    height = math_node(nt, 'MULTIPLY', k['scratch'], -0.5)
    finish(nt, col, 0.08, 0.0, height, 0.9)
    return m

def mat_led():
    m, nt = new_mat("led")
    k = common_masks(nt)
    finish(nt, mix(nt, k['noise'], (0.15, 0.75, 0.15, 1), (0.25, 0.9, 0.25, 1)), 0.2, 0.0, math_node(nt, 'MULTIPLY', k['noise'], 0.0), 0.8)
    return m

PAINT = mat_paint("paint", (0.085, 0.098, 0.055, 1))
BEZEL = mat_paint("bezel", (0.06, 0.065, 0.05, 1))
RUBBER = mat_rubber()
METAL = mat_metal()
SCREEN = mat_screen()
LED = mat_led()

# ------------------------------------------------------------------ the device

parts = []
body = box("body", (W, T, L), (0, 0, 0), bevel=0.004, segs=3, mat=PAINT)
cut(body, box("c_screen", (0.058, 0.006, 0.050), (0, -T / 2, 0.032)))
cut(body, box("c_keys", (0.060, 0.003, 0.058), (0, -T / 2, -0.030)))
parts.append(body)

# raised bezel ring around the screen
ring = box("bezel", (0.066, 0.0024, 0.058), (0, -T / 2 - 0.0006, 0.032), bevel=0.0008, mat=BEZEL)
cut(ring, box("c_ring", (0.058, 0.01, 0.050), (0, -T / 2, 0.032)))
parts.append(ring)

glass = box("glass", (0.0566, 0.001, 0.0486), (0, -T / 2 + 0.0026, 0.032), mat=SCREEN)
parts.append(glass)

# rubber corner bumpers
for sx in (-1, 1):
    for sz in (-1, 1):
        parts.append(box("bump", (0.017, T + 0.003, 0.017), (sx * (W / 2 - 0.0065), 0, sz * (L / 2 - 0.0065)), bevel=0.003, segs=2, mat=RUBBER))

# side rubber grips
for sx in (-1, 1):
    parts.append(box("grip", (0.004, 0.018, 0.074), (sx * (W / 2 + 0.0006), 0, 0), bevel=0.0012, mat=RUBBER))

# keys: function row + 3x4 keypad
for i in range(3):
    parts.append(box("fkey", (0.016, 0.003, 0.006), (-0.020 + i * 0.020, -T / 2, -0.006), bevel=0.0012, mat=RUBBER))
for i in range(12):
    c, r = i % 3, i // 3
    parts.append(box("key", (0.012, 0.003, 0.008), (-0.016 + c * 0.016, -T / 2, -0.017 - r * 0.011), bevel=0.0013, mat=RUBBER))

# LED
parts.append(cyl("led", 0.0018, 0.0016, (-0.032, -T / 2 - 0.0002, 0.0635), axis='Y', verts=12, mat=LED))

# antenna, connector on the top end
parts.append(cyl("ant_base", 0.0058, 0.005, (-0.020, 0, L / 2 + 0.0015), verts=6, mat=METAL, bevel=0.0006))
parts.append(cyl("antenna", 0.0034, 0.046, (-0.020, 0, L / 2 + 0.027), verts=14, mat=RUBBER, bevel=0.0012))
for k in range(3):
    parts.append(cyl("ant_ring", 0.0039, 0.0025, (-0.020, 0, L / 2 + 0.008 + k * 0.004), verts=14, mat=RUBBER))
parts.append(cyl("conn", 0.0062, 0.006, (0.016, 0, L / 2 + 0.002), verts=16, mat=METAL, bevel=0.0006))
parts.append(cyl("conn_cap", 0.0068, 0.003, (0.016, 0, L / 2 + 0.0055), verts=16, mat=RUBBER, bevel=0.0008))

# strap lug at the bottom
lug = box("lug", (0.022, 0.007, 0.009), (0, 0.004, -L / 2 - 0.0035), bevel=0.0015, mat=METAL)
cut(lug, box("c_lug", (0.016, 0.02, 0.0045), (0, 0.004, -L / 2 - 0.0045)))
parts.append(lug)

# battery hatch with screws on the back
parts.append(box("hatch", (0.056, 0.0018, 0.082), (0, T / 2 + 0.0005, -0.004), bevel=0.0008, mat=PAINT))
for sx in (-1, 1):
    for sz in (-1, 1):
        parts.append(cyl("screw", 0.0022, 0.0012, (sx * 0.023, T / 2 + 0.0015, -0.004 + sz * 0.035), axis='Y', verts=12, mat=METAL, bevel=0.0004))

# join
bpy.ops.object.select_all(action='DESELECT')
for p in parts:
    p.select_set(True)
bpy.context.view_layer.objects.active = body
bpy.ops.object.join()
kpk = bpy.context.active_object
kpk.name = "KPK"
for i in reversed(range(len(kpk.data.materials))):
    if kpk.data.materials[i] is None:
        kpk.active_material_index = i
        bpy.ops.object.material_slot_remove()
bpy.ops.object.shade_auto_smooth(angle=math.radians(40))
bpy.ops.object.modifier_apply(modifier=kpk.modifiers[0].name) if kpk.modifiers else None

# UVs
bpy.ops.object.mode_set(mode='EDIT')
bpy.ops.mesh.select_all(action='SELECT')
bpy.ops.uv.smart_project(angle_limit=math.radians(60), island_margin=0.006)
bpy.ops.object.mode_set(mode='OBJECT')
print("tris:", sum(len(p.vertices) - 2 for p in kpk.data.polygons))

# ------------------------------------------------------------------ bake

scene.render.engine = 'CYCLES'
scene.cycles.device = 'CPU'
scene.cycles.samples = 24
scene.render.bake.margin = 12

def bake(kind, colorspace, fname, pass_filter=None, emit=False):
    img = bpy.data.images.new(fname, RES, RES, alpha=False, float_buffer=False)
    img.colorspace_settings.name = colorspace
    for mat in kpk.data.materials:
        nt = mat.node_tree
        for n in [n for n in nt.nodes if n.name == "BAKE_TARGET"]:
            nt.nodes.remove(n)
        t = nt.nodes.new('ShaderNodeTexImage')
        t.name = "BAKE_TARGET"
        t.image = img
        nt.nodes.active = t
        out = nt.nodes["OUT"]
        if emit:
            nt.links.new(nt.nodes["SPEC_EMIT"].outputs[0], out.inputs['Surface'])
        else:
            nt.links.new(nt.nodes["BSDF"].outputs[0], out.inputs['Surface'])
    bpy.context.view_layer.objects.active = kpk
    kpk.select_set(True)
    if pass_filter:
        bpy.ops.object.bake(type=kind, pass_filter=pass_filter, use_clear=True)
    else:
        bpy.ops.object.bake(type=kind, use_clear=True)
    img.filepath_raw = OUT + "/" + fname + ".png"
    img.file_format = 'PNG'
    img.save()
    print("baked", fname)

bake('DIFFUSE', 'sRGB', "bake_color", pass_filter={'COLOR'})
bake('ROUGHNESS', 'Non-Color', "bake_rough")
bake('NORMAL', 'Non-Color', "bake_normal")
bake('EMIT', 'Non-Color', "bake_spec", emit=True)

# diagnostics: mean baked colour per material
import numpy as _np
_img = bpy.data.images["bake_color"]
_px = _np.array(_img.pixels[:]).reshape(RES, RES, 4)
_uv = kpk.data.uv_layers.active.data
_acc = {}
for poly in kpk.data.polygons:
    u = sum(_uv[li].uv[0] for li in poly.loop_indices) / poly.loop_total
    v = sum(_uv[li].uv[1] for li in poly.loop_indices) / poly.loop_total
    c = _px[min(RES - 1, int(v * RES)), min(RES - 1, int(u * RES))][:3]
    name = kpk.data.materials[poly.material_index].name
    _acc.setdefault(name, []).append(c)
for name, cs in _acc.items():
    print("MAT", name, len(cs), _np.mean(cs, axis=0).round(3))

# preview material: baked textures only (what the game will show)
pm = bpy.data.materials.new("preview")
pm.use_nodes = True
nt = pm.node_tree
bsdf = nt.nodes["Principled BSDF"]
c = nt.nodes.new('ShaderNodeTexImage'); c.image = bpy.data.images["bake_color"]
r = nt.nodes.new('ShaderNodeTexImage'); r.image = bpy.data.images["bake_rough"]
n = nt.nodes.new('ShaderNodeTexImage'); n.image = bpy.data.images["bake_normal"]
nm = nt.nodes.new('ShaderNodeNormalMap')
nt.links.new(c.outputs[0], bsdf.inputs['Base Color'])
nt.links.new(r.outputs[0], bsdf.inputs['Roughness'])
nt.links.new(n.outputs[0], nm.inputs['Color'])
nt.links.new(nm.outputs[0], bsdf.inputs['Normal'])
kpk.data.materials.clear()
kpk.data.materials.append(pm)

# lying flat, like the game item: screen up (+Z), antenna towards +Y
kpk.rotation_euler = (math.radians(-90), 0, 0)
bpy.ops.object.transform_apply(rotation=True)

bpy.ops.wm.save_as_mainfile(filepath=OUT + "/kpk.blend")
bpy.ops.export_scene.fbx(filepath=OUT + "/kpk.fbx", use_selection=True, mesh_smooth_type='FACE', axis_forward='-Z', axis_up='Y')
bpy.ops.wm.obj_export(filepath=OUT + "/kpk.obj", export_selected_objects=True, export_materials=False)

# mesh data for the P3D writer (triangles, corner normals, uvs)
me = kpk.data
me.calc_loop_triangles()
uvl = me.uv_layers.active.data
cn = me.corner_normals
data = {"verts": [list(v.co) for v in me.vertices], "tris": []}
for tri in me.loop_triangles:
    corners = []
    for li in tri.loops:
        l = me.loops[li]
        corners.append([l.vertex_index, list(cn[li].vector), list(uvl[li].uv)])
    data["tris"].append(corners)
json.dump(data, open(OUT + "/mesh.json", "w"))
print("export ok, tris", len(data["tris"]))
