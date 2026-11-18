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


# strap pose (attached to a backpack / vest slot): the model.cfg animation "kpkStrap" moves the hand pose
# to: centred on the slot, long side along the strap (roll), back against it (ZBACK towards the outside)
ROLL = math.radians(float(sys.argv[11]) if len(sys.argv) > 11 else 90.0)
ZBACK = float(sys.argv[12]) if len(sys.argv) > 12 else -0.02


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


def visual_lod(path, resolution):
    m = json.load(open(path))
    points = [to_p3d(v) for v in m["verts"]]
    normals, faces = [], []
    for tri in m["tris"]:
        verts = []
        corners = list(reversed(tri)) if REVERSE else list(tri)
        for vi, n, uv in corners:
            pn = turn(n[0], n[1], -n[2])
            if INVERT:
                pn = tuple(-c for c in pn)
            normals.append(pn)
            verts.append((vi, len(normals) - 1, uv[0], 1.0 - uv[1]))
        faces.append((verts, TEX, MAT))
    taggs = selection("camo", len(points), len(faces)) + selection("kpk_strap_r1", len(points), len(faces))
    # the parent bones of the strap animation have no vertices of their own
    taggs += selection("kpk_strap_r2", len(points), len(faces), points=[], faces=[])
    taggs += selection("kpk_strap_move", len(points), len(faces), points=[], faces=[])
    return lod(points, normals, faces, resolution, taggs), points


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
    else:
        taggs += selection("kpk_strap_r1", 8, len(faces))
    return lod(pts, [(0.0, 1.0, 0.0)], faces, resolution, taggs)


lod0, pts = visual_lod(src + "/mesh.json", 1.0)
lod1, _ = visual_lod(src + "/mesh_lod1.json", 2.0)

# collision / view / fire geometry: the body with bumpers, without the antenna
xs = [p[0] for p in pts]
ys = [p[1] for p in pts]
zs = [p[2] for p in pts]
mn = (min(xs), min(ys), min(zs))
mx = (max(xs), min(max(ys), YMAX), max(zs))   # antenna (above y=0.074) left out
print("bbox", [round(c, 4) for c in mn], [round(c, 4) for c in mx], "points", len(pts))

# memory points: centre, and "invView" - the side the inventory preview camera looks from (the screen, -Z)
# boundingbox_min/max frame the inventory / inspect preview: it centres on them and rotates around
# their middle, so the model turns around its own centre even though its origin is at the grip
vx = [p[0] for p in pts]
vy = [p[1] for p in pts]
c = ((min(vx) + max(vx)) / 2, (min(ys) + max(ys)) / 2, (min(zs) + max(zs)) / 2)
memory_pts = [c, (c[0], c[1], -0.35), (min(vx), min(ys), min(zs)), (max(vx), max(ys), max(zs))]

# ---- strap animation: hand pose q = R(p + T) + Z0  ->  strap pose Roll(p - C) + ZB
# done as two half turns (child bone r1, then r2: a 180 degree turn looks the same either way round,
# so the engine's rotation sign cannot flip it) and a translation (parent bone)
def mat_turn():
    cols = [turn(1, 0, 0), turn(0, 1, 0), turn(0, 0, 1)]
    return [[cols[j][i] for j in range(3)] for i in range(3)]


def mul(a, b):
    return [[sum(a[i][k] * b[k][j] for k in range(3)) for j in range(3)] for i in range(3)]


def app(m, v):
    return tuple(sum(m[i][k] * v[k] for k in range(3)) for i in range(3))


def axis_rot(n, ang):
    x, y, z = n
    c, s_, t = math.cos(ang), math.sin(ang), 1 - math.cos(ang)
    return [[t*x*x + c, t*x*y - s_*z, t*x*z + s_*y], [t*x*y + s_*z, t*y*y + c, t*y*z - s_*x], [t*x*z - s_*y, t*y*z + s_*x, t*z*z + c]]


def norm(v):
    l = math.sqrt(sum(c * c for c in v))
    return tuple(c / l for c in v)


def cross(a, b):
    return (a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0])


R = mat_turn()
RT = [[R[j][i] for j in range(3)] for i in range(3)]
ROLLM = axis_rot((0, 0, 1), ROLL)
M = mul(ROLLM, RT)
src_pts = [(v[0], v[1], -v[2]) for v in json.load(open(src + "/mesh.json"))["verts"]]
C = tuple((min(p[i] for p in src_pts) + max(p[i] for p in src_pts)) / 2 for i in range(3))
T = (XOFF, YOFF, 0.0)
Z0 = (0.0, 0.0, ZOFF)
mz = app(M, Z0)
rt = app(ROLLM, (T[0] + C[0], T[1] + C[1], T[2] + C[2]))
CT = (-mz[0] - rt[0], -mz[1] - rt[1], -mz[2] - rt[2] + ZBACK)
# axis-angle of M
ang = math.acos(max(-1.0, min(1.0, (M[0][0] + M[1][1] + M[2][2] - 1) / 2)))
if ang < 1e-6:
    n = (0.0, 0.0, 1.0)
else:
    n = norm((M[2][1] - M[1][2], M[0][2] - M[2][0], M[1][0] - M[0][1]))
helper = (1.0, 0.0, 0.0) if abs(n[0]) < 0.9 else (0.0, 1.0, 0.0)
m1 = norm(cross(n, helper))
m2 = app(axis_rot(n, ang / 2), m1)
# check: half turn about m1, then about m2, then move = strap pose
H1, H2 = axis_rot(m1, math.pi), axis_rot(m2, math.pi)
err = 0.0
for p in src_pts[::50]:
    q = turn(p[0] + XOFF, p[1] + YOFF, p[2]); q = (q[0], q[1], q[2] + ZOFF)
    r = app(H2, app(H1, q)); r = (r[0] + CT[0], r[1] + CT[1], r[2] + CT[2])
    w = app(ROLLM, (p[0] - C[0], p[1] - C[1], p[2] - C[2])); w = (w[0], w[1], w[2] + ZBACK)
    err = max(err, max(abs(r[i] - w[i]) for i in range(3)))
print("strap check, max error", round(err, 6))
assert err < 1e-5

memory_pts += [(0, 0, 0), tuple(c * 0.1 for c in m1), (0, 0, 0), tuple(c * 0.1 for c in m2), (0, 0, 0), CT]
NP = len(memory_pts)
memory = lod(memory_pts, [(0.0, 1.0, 0.0)], [], 1e15,
             selection("ce_center", NP, 0, points=[0]) + selection("invView", NP, 0, points=[1])
             + selection("boundingbox_min", NP, 0, points=[2]) + selection("boundingbox_max", NP, 0, points=[3])
             + selection("kpk_strap_r1_axis", NP, 0, points=[4, 5]) + selection("kpk_strap_r2_axis", NP, 0, points=[6, 7])
             + selection("kpk_strap_move_axis", NP, 0, points=[8, 9]))
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
