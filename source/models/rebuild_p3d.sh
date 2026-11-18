#!/bin/bash
# rewrite the landscape p3d files: held by the right side bezel, preview centred via boundingbox points
# usage: rebuild_p3d.sh <scratch dir with kpk5/final_* and m1/final>
S=$1; R=$(cd "$(dirname "$0")/../.." && pwd)/KPK/data
for spec in "$S/kpk5/final_mil:kpk_mil:0.7" "$S/kpk5/final_mono:kpk_mono:0.5" "$S/kpk5/final_nii:kpk_nii:0.55" "$S/m1/final:kpk_m1:0.45"; do
  IFS=: read d n mass <<<"$spec"
  read xo yo < <(python3 -c "
import json;v=json.load(open('$d/mesh.json'))['verts']
x1=max(p[0] for p in v); y0=min(p[1] for p in v); y1=max(p[1] for p in v)
print(round(-(x1-0.018),4), round(-(y0+y1)/2-0.008,4))")
  python3 "$(dirname "$0")/write_p3d5.py" $d $R/$n.p3d $n $mass 9 $yo $xo ${YAW:-18} ${PITCH:-12} ${ZOFF:--0.006} 90 ${STRAP_BACK:-0.01} ${STRAP_UP:-0.06}
done
# portrait models (brick, broken): held as they are, the same on a strap, centred copy for the inventory
for spec in "$S/model/final:kpk:0.3:0.074" "$S/kpk5/final_broken:kpk_broken:0.28:0.074"; do
  IFS=: read d n mass ymax <<<"$spec"
  python3 "$(dirname "$0")/write_p3d5.py" $d $R/$n.p3d $n $mass $ymax 0 0 0 0 0 0 0 0 same
done
