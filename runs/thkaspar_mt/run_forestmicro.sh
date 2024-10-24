#!/bin/bash

poms="../../bin/poms"

q="128"
s="48"

if [[ "$1" != "" ]] ; then
  q="$1"
fi

if [[ "$2" != "" ]] ; then
  s="$2"
fi

N="$s,$s,1"
Q="$q,$q,1"

seed=1337

## viz_step=1 slows the run down significantly
## but without it we can't get nice animations.
#
#  -O viz_step=1 \
##

echo "#"
echo "# forest micro tile set"
echo "# Q     : $Q"
echo "# N     : $N"
echo "# B     : 8,8,1"
echo "# w     : 1.0"
echo "# E     : -1.95"
echo "# seed  : $seed"
echo "# tiled output: data/forestmicro_${q}x${q}.json"
echo "# snapshot    : data/forestmicro_snapshot.json"
echo "#"
echo ""


$poms \
  -C ./data/forestmicro_poms.json \
  -s "$N" \
  -q "$Q" \
  -b 1 \
  -B 8,8,1 \
  -J 10000 \
  -w 1.0 \
  -E -1.95 \
  -1 data/forestmicro_${q}x${q}.json \
  -8 data/forestmicro_snapshot.json \
  -P wf \
  -O patch-policy=wf \
  -S $seed \
  -V 1


