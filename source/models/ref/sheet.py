import sys
from PIL import Image
out, ims = sys.argv[1], sys.argv[2:]
I = [Image.open(p).convert("RGB") for p in ims]
w, h = I[0].size
o = Image.new("RGB", (w * 2, h * 2))
for i, im in enumerate(I):
    o.paste(im, ((i % 2) * w, (i // 2) * h))
o.save(out, quality=88)
