#!/bin/bash
# after the bakes: LOD, p3d, game textures, rvmat, previews -> repo
set -e
cd "$(dirname "$0")"
PY=../bvenv/bin/python
R=/home/user/kpk
declare -A NAME=( [mil]=kpk_mil [mono]=kpk_mono [nii]=kpk_nii [broken]=kpk_broken )
declare -A MASS=( [mil]=0.7 [mono]=0.5 [nii]=0.55 [broken]=0.28 )
declare -A YMAX=( [mil]=9 [mono]=9 [nii]=9 [broken]=0.074 )
for m in ${MODELS:-mil mono nii broken}; do
  d=final_$m; n=${NAME[$m]}
  $PY ../model/lods.py -- $d/kpk.blend $d > /dev/null 2>&1
  python3 write_p3d5.py $d $d/$n.p3d $n ${MASS[$m]} ${YMAX[$m]} | grep -E "bbox|written"
  $PY - "$d" "$n" <<'PYEOF'
import sys
from PIL import Image
import numpy as np
d, n = sys.argv[1], sys.argv[2]
Image.open(d + "/bake_color.png").convert("RGB").save(f"{d}/{n}_co.png")
a = np.asarray(Image.open(d + "/bake_normal.png").convert("RGB")).copy(); a[..., 1] = 255 - a[..., 1]
Image.fromarray(a).save(f"{d}/{n}_nohq.png")
r = np.asarray(Image.open(d + "/bake_rough.png").convert("L")).astype(float)
sp = np.asarray(Image.open(d + "/bake_spec.png").convert("L")).astype(float)
o = np.zeros(r.shape + (3,), np.uint8); o[..., 0] = 255; o[..., 1] = np.clip(sp, 0, 255); o[..., 2] = np.clip(255 - r, 0, 255)
Image.fromarray(o).save(f"{d}/{n}_smdi.png")
for v in ("front", "back", "angle"):
    Image.open(f"{d}/preview_{v}.png").convert("RGB").save(f"{d}/{n}_{v}.jpg", quality=88)
PYEOF
  cp $d/$n.p3d $d/${n}_co.png $d/${n}_nohq.png $d/${n}_smdi.png $R/KPK/data/
  sed "s/kpk_nohq/${n}_nohq/; s/kpk_smdi/${n}_smdi/" $R/KPK/data/kpk.rvmat > $R/KPK/data/$n.rvmat
  mkdir -p $R/source/models/$m/preview $R/source/models/$m/textures_src
  cp build_$m.py $R/source/models/$m/ ; cp $d/kpk.blend $R/source/models/$m/$n.blend; cp $d/kpk.fbx $R/source/models/$m/$n.fbx
  cp out_$m/*.png $R/source/models/$m/textures_src/
  cp $d/${n}_front.jpg $d/${n}_back.jpg $d/${n}_angle.jpg $R/source/models/$m/preview/
  echo "$m -> repo"
done
