# Writes an editable MLOD .p3d (the format Object Builder saves) with all LODs a DayZ item needs.
# Blender (x, y, z), Z up  ->  P3D (x, z, y), Y up. Face winding is reversed (P3D front faces are clockwise).
import json, struct, sys

src, out = sys.argv[1], sys.argv[2]
# variant flags for testing in game: reverse face winding, invert stored normals
# tested in game: variant D (original winding, normals as they are) renders correctly
REVERSE = False
INVERT = False
# usage: write_p3d5.py <dir> <out.p3d> <name> <mass> [ymax] [yoff] [xoff] [yaw] [pitch] [zoff]   name -> kpk\data\<name>_co.paa / <name>.rvmat
NAME = sys.argv[3]
TEX = "kpk\\data\\" + NAME + "_co.paa"
MAT = "kpk\\data\\" + NAME + ".rvmat"
MASS = float(sys.argv[4])
YMAX = float(sys.argv[5]) if len(sys.argv) > 5 else 9.0
# lift landscape models so the hand (vanilla GPS hold) grips the bottom edge instead of the screen
YOFF = float(sys.argv[6]) if len(sys.argv) > 6 else 0.0
# shift sideways: landscape models are held by the side bezel, so the thumb rests beside the screen
XOFF = float(sys.argv[7]) if len(sys.argv) > 7 else 0.0
# turn the screen towards the player around the grip (degrees): yaw brings the far (left) side closer,
# pitch tips the top away so the screen looks up at the eyes; keeps the fingers out of the screen
import math
YAW = math.radians(float(sys.argv[8]) if len(sys.argv) > 8 else 0.0)
PITCH = math.radians(float(sys.argv[9]) if len(sys.argv) > 9 else 0.0)
ZOFF = float(sys.argv[10]) if len(sys.argv) > 10 else 0.0


# strap pose (attached to a backpack / vest slot): a second copy of the mesh, centred on the slot,
# long side along the strap (roll), back against it (ZBACK towards the outside)
ROLL = math.radians(float(sys.argv[11]) if len(sys.argv) > 11 else 90.0)
ZBACK = float(sys.argv[12]) if len(sys.argv) > 12 else -0.02
STRAP_UP = float(sys.argv[13]) if len(sys.argv) > 13 else 0.0
# "same": on a strap the model looks as in the hands (portrait models: brick, broken)
STRAP_SAME = len(sys.argv) > 14 and sys.argv[14] == "same"


def turn(x, y, z):
    x, z = x * math.cos(YAW) - z * math.sin(YAW), x * math.sin(YAW) + z * math.cos(YAW)
    y, z = y * math.cos(PITCH) - z * math.sin(PITCH), y * math.sin(PITCH) + z * math.cos(PITCH)
    return (x, y, z)


def asciiz(s):
    return s.encode("ascii") + b"\0"


def tag(name, data):
    return b"\1" + asciiz(name) + struct.pack("<I", len(data)) + data


def prop(name, value):
    return tag("#Property#", name.encode().ljust(64, b"\0") + value.encode().ljust(64, b"\0"))


def selection(name, n_points, n_faces, points=None, faces=None):
    pw = bytearray(n_points)
    fw = bytearray(n_faces)
    for i in (range(n_points) if points is None else points):
        pw[i] = 1
    for i in (range(n_faces) if faces is None else faces):
        fw[i] = 1
    return tag(name, bytes(pw) + bytes(fw))


def lod(points, normals, faces, resolution, taggs=b""):
    """points: [(x,y,z)] in P3D space; faces: [[(point, normal, u, v), ...3 or 4], texture, material]"""
    b = bytearray()
    b += b"P3DM" + struct.pack("<IIIIII", 28, 256, len(points), len(normals), len(faces), 0)
    for p in points:
        b += struct.pack("<fffI", p[0], p[1], p[2], 0)
    for n in normals:
        b += struct.pack("<fff", n[0], n[1], n[2])
    for verts, tex, mat in faces:
        b += struct.pack("<I", len(verts))
        for i in range(4):
            if i < len(verts):
                pi, ni, u, v = verts[i]
            else:
                pi, ni, u, v = 0, 0, 0.0, 0.0
            b += struct.pack("<IIff", pi, ni, u, v)
        b += struct.pack("<I", 0)
        b += asciiz(tex) + asciiz(mat)
    b += b"TAGG"
    b += taggs
    b += tag("#EndOfFile#", b"")
    b += struct.pack("<f", resolution)
    return bytes(b)


def to_p3d(v):
    # mesh.json: lying flat, screen +Z, antenna +Y (Blender). Held in hands the player looks at the
    # item's -Z side, so P3D: antenna up (+Y), screen towards -Z. One axis flips: P3D is left-handed.
    x, y, z = turn(v[0] + XOFF, v[1] + YOFF, -v[2])
    return (x, y, z + ZOFF)


def strap_pose(v):
    # attached to a strap / vest slot: centred, long side along the strap (roll), back on the strap
    x, y, z = v[0] - C[0], v[1] - C[1], -v[2] - C[2]
    c, s_ = math.cos(ROLL), math.sin(ROLL)
    return (x * c - y * s_, x * s_ + y * c + STRAP_UP, z + ZBACK)


def strap_normal(n):
    c, s_ = math.cos(ROLL), math.sin(ROLL)
    return (n[0] * c - n[1] * s_, n[0] * s_ + n[1] * c, -n[2])


def free_pose(v):
    # in the inventory, on the ground, in a bag: centred on the origin, so the inspect view turns it
    # around its middle (the preview always rotates around the model origin)
    return (v[0] - C[0], v[1] - C[1], -v[2] - C[2])


def visual_lod(path, resolution):
    """three copies of the mesh: in the hands ("kpk_hand", origin at the grip), on a strap ("kpk_strap")
    and free ("kpk_free", centred); the model.cfg hide animations show one, the script sets the phases"""
    m = json.load(open(path))
    nv = len(m["verts"])
    hand = [to_p3d(v) for v in m["verts"]]
    strap = hand if STRAP_SAME else [strap_pose(v) for v in m["verts"]]
    points = hand + strap + [free_pose(v) for v in m["verts"]]
    normals, faces = [], []
    for copy in (0, 1, 2):
        for tri in m["tris"]:
            verts = []
            corners = list(reversed(tri)) if REVERSE else list(tri)
            for vi, n, uv in corners:
                if copy == 0 or (copy == 1 and STRAP_SAME):
                    pn = turn(n[0], n[1], -n[2])
                elif copy == 1:
                    pn = strap_normal(n)
                else:
                    pn = (n[0], n[1], -n[2])
                if INVERT:
                    pn = tuple(-c for c in pn)
                normals.append(pn)
                verts.append((vi + nv * copy, len(normals) - 1, uv[0], 1.0 - uv[1]))
            faces.append((verts, TEX, MAT))
    nf = len(m["tris"])
    taggs = selection("camo", len(points), len(faces))
    for k, name in enumerate(("kpk_hand", "kpk_strap", "kpk_free")):
        taggs += selection(name, len(points), len(faces), points=range(nv * k, nv * (k + 1)), faces=range(nf * k, nf * (k + 1)))
    return lod(points, normals, faces, resolution, taggs), points[2 * nv:]


def box_lod(mn, mx, resolution, mass=None, component=True, triangles=False):
    x0, y0, z0 = mn
    x1, y1, z1 = mx
    pts = [(x0, y0, z0), (x1, y0, z0), (x1, y1, z0), (x0, y1, z0),
           (x0, y0, z1), (x1, y0, z1), (x1, y1, z1), (x0, y1, z1)]
    # quads, clockwise when seen from outside (P3D convention)
    quads = [(0, 3, 2, 1), (4, 5, 6, 7), (0, 1, 5, 4), (2, 3, 7, 6), (1, 2, 6, 5), (0, 4, 7, 3)]
    faces = []
    for q in quads:
        polys = [(q[0], q[1], q[2]), (q[0], q[2], q[3])] if triangles else [q]
        for poly in polys:
            faces.append(([(i, 0, 0.0, 0.0) for i in poly], "", ""))
    taggs = b""
    if mass is not None:
        taggs += tag("#Mass#", struct.pack("<8f", *([mass / 8.0] * 8)))
    if component:
        taggs += selection("Component01", 8, len(faces))
    return lod(pts, [(0.0, 1.0, 0.0)], faces, resolution, taggs)


_src = json.load(open(src + "/mesh.json"))["verts"]
C = tuple((min(v[i] * (-1 if i == 2 else 1) for v in _src) + max(v[i] * (-1 if i == 2 else 1) for v in _src)) / 2 for i in range(3))
lod0, pts = visual_lod(src + "/mesh.json", 1.0)
lod1, _ = visual_lod(src + "/mesh_lod1.json", 2.0)

# collision / view / fire geometry: the body with bumpers, without the antenna
xs = [p[0] for p in pts]
ys = [p[1] for p in pts]
zs = [p[2] for p in pts]
mn = (min(xs), min(ys), min(zs))
mx = (max(xs), min(max(ys), YMAX - C[1]), max(zs))   # antenna (above YMAX in mesh units) left out
print("bbox", [round(c, 4) for c in mn], [round(c, 4) for c in mx], "points", len(pts))

# memory points: centre, and "invView" - the side the inventory preview camera looks from (the screen, -Z)
# boundingbox_min/max frame the inventory / inspect preview (around the free, centred copy)
vx = [p[0] for p in pts]
vy = [p[1] for p in pts]
c = ((min(vx) + max(vx)) / 2, (min(ys) + max(ys)) / 2, (min(zs) + max(zs)) / 2)
memory_pts = [c, (c[0], c[1], -0.35), (min(vx), min(ys), min(zs)), (max(vx), max(ys), max(zs))]

memory = lod(memory_pts, [(0.0, 1.0, 0.0)], [], 1e15,
             selection("ce_center", 4, 0, points=[0]) + selection("invView", 4, 0, points=[1])
             + selection("boundingbox_min", 4, 0, points=[2]) + selection("boundingbox_max", 4, 0, points=[3]))
view_geo = box_lod(mn, mx, 6e15)
fire_geo = box_lod(mn, mx, 7e15)
shadow = box_lod(mn, mx, 10000.0, component=False, triangles=True)

# geometry LOD: mass, convex component, autocenter off
gx = box_lod(mn, mx, 1e13, mass=MASS)
end = gx.rfind(b"\1#EndOfFile#")
gx = gx[:end] + prop("autocenter", "0") + gx[end:]

lods = [lod0, lod1, gx, memory, view_geo, fire_geo, shadow]
with open(out, "wb") as f:
    f.write(b"MLOD" + struct.pack("<II", 257, len(lods)))
    for l in lods:
        f.write(l)
print("written", out)
