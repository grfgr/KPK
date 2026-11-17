# sanity check: parse the MLOD back
import struct, sys
b = open(sys.argv[1], "rb").read()
o = 0
def rd(fmt):
    global o
    v = struct.unpack_from(fmt, b, o); o += struct.calcsize(fmt); return v
def rz():
    global o
    e = b.index(b"\0", o); s = b[o:e].decode(); o = e + 1; return s
sig, ver, n = b[0:4], *struct.unpack_from("<II", b, 4); o = 12
print(sig, ver, n)
for i in range(n):
    s = b[o:o+4]; o += 4
    hs, mv, np_, nn, nf, fl = rd("<IIIIII")
    o += np_ * 16 + nn * 12
    texs = set()
    for f in range(nf):
        rd("<I"); o += 4 * 16; rd("<I"); texs.add(rz()); rz()
    assert b[o:o+4] == b"TAGG", b[o:o+4]; o += 4
    tags = []
    while True:
        act = b[o]; o += 1; name = rz(); size, = rd("<I"); o += size; tags.append(name)
        if name == "#EndOfFile#": break
    res, = rd("<f")
    print(s, "res=%g" % res, "points", np_, "faces", nf, "tags", tags, "tex", texs)
print("bytes left:", len(b) - o)
