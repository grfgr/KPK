# Writes an editable MLOD .p3d (the format Object Builder saves) with all LODs a DayZ item needs.
# Blender (x, y, z), Z up  ->  P3D (x, z, y), Y up. Face winding is reversed (P3D front faces are clockwise).
import json, struct, sys

src, out = sys.argv[1], sys.argv[2]
# variant flags for testing in game: reverse face winding, invert stored normals
# tested in game: variant D (original winding, normals as they are) renders correctly
REVERSE = sys.argv[3] == "1" if len(sys.argv) > 3 else False
INVERT = sys.argv[4] == "1" if len(sys.argv) > 4 else False
TEX = "kpk\\data\\kpk_m1_co.paa"
MAT = "kpk\\data\\kpk_m1.rvmat"
MASS = 0.45


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
    return (v[0], v[1], -v[2])


def visual_lod(path, resolution):
    m = json.load(open(path))
    points = [to_p3d(v) for v in m["verts"]]
    normals, faces = [], []
    for tri in m["tris"]:
        verts = []
        corners = list(reversed(tri)) if REVERSE else list(tri)
        for vi, n, uv in corners:
            pn = to_p3d(n)
            if INVERT:
                pn = tuple(-c for c in pn)
            normals.append(pn)
            verts.append((vi, len(normals) - 1, uv[0], 1.0 - uv[1]))
        faces.append((verts, TEX, MAT))
    taggs = selection("camo", len(points), len(faces))
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
    return lod(pts, [(0.0, 1.0, 0.0)], faces, resolution, taggs)


lod0, pts = visual_lod(src + "/mesh.json", 1.0)
lod1, _ = visual_lod(src + "/mesh_lod1.json", 2.0)

# collision / view / fire geometry: the body with bumpers, without the antenna
xs = [p[0] for p in pts]
ys = [p[1] for p in pts]
zs = [p[2] for p in pts]
mn = (min(xs), min(ys), min(zs))
mx = (max(xs), max(ys), max(zs))   # antenna (above y=0.074) left out
print("bbox", [round(c, 4) for c in mn], [round(c, 4) for c in mx], "points", len(pts))

# memory points: centre, and "invView" - the side the inventory preview camera looks from (the screen, -Z)
memory_pts = [(0.0, 0.0, 0.0), (0.0, 0.0, -0.35)]
memory = lod(memory_pts, [(0.0, 1.0, 0.0)], [], 1e15,
             selection("ce_center", 2, 0, points=[0]) + selection("invView", 2, 0, points=[1]))
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
