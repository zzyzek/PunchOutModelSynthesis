#!/bin/bash



w=256
h=256
Q="$w,$h,1"
N="50,70,1"

B='12:24'

opt="wUbW3x3"
logfn="oarpgo_${opt}.log"

ppolicy="cone-"

echo "# oarpgo $opt"
echo "# Q: $Q"
echo "# N: $N"
echo "# patch policy: $ppolicy"
echo "# snapshot: data/oarpgo_snapshot_${opt}.json"
echo "# log: $logfn"
echo "# tiled output: data/oarpgo_${w}x${h}_${opt}.json"

../../bin/poms \
  -s $N \
  -q $Q \
  -C ./data/oarpgo_poms_${opt}.json \
  -b 1 -B $B \
  -P min \
  -O "patch-policy=$ppolicy" \
  -w 1.5 -E -1.75 \
  -S 1337 \
  -V 1 \
  -O viz_step=50 \
  -8 ./data/oarpgo_snapshot_${opt}.json \
  -1 ./data/oarpgo_${w}x${h}_${opt}.json > $logfn
