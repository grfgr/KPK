#!/bin/bash
# quick 256 smoke test of all four, then the 1024 bakes
cd "$(dirname "$0")"
PY=../bvenv/bin/python
for m in mil mono nii broken; do
  mkdir -p final_$m && cp out_$m/*.png final_$m/
  $PY build_$m.py -- $(pwd)/final_$m ${RES:-1024} > log_$m.txt 2>&1 && $PY preview5.py -- final_$m/kpk.blend final_$m >> log_$m.txt 2>&1
  echo "$m done: $(grep -c 'export ok' log_$m.txt)"
done
