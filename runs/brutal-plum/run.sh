#!/bin/bash

logfn=brutal-plum_nopath.log
pomsfn="./data/brutal-plum_poms.json"
patchfn="./data/brutal-plum_patch.json"
stlfn="./data/brutal-plum.stl"

blockpolicy="min"
patchpolicy="pending"


s="22,22,22"
q="32,32,32"

B="1:8"

JIter=` echo "$s" | sed 's/,/\*/g' | sed 's/$/\*10/' | bc`

echo "# info { blockpolicy:$blockpolicy, patchpolicy:$patchpolicy, b:1, B:$B, s:$s, q:$q, J:$JIter }"
echo "# log: $logfn"
echo "# po48ms: $pomsfn"
echo "# patch: $patchfn"
echo "# stl: $stlfn"

../../bin/poms \
  -s "$s" \
  -q "$q" \
  -b 1 \
  -B "$B" \
  -J "$JIter" \
  -w 1.5 -E -1.85 \
  -P "$blockpolicy" \
  -O "patch-policy=$patchpolicy" \
  -C "$pomsfn" \
  -2 $stlfn \
  -6 $patchfn \
  -S 1337 \
  -O "viz_step=100" \
  -V 1


