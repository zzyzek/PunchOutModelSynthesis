#!/bin/bash


N='58,58,1'
Q='128,128,1'
N="$Q"
B='8:32'

../../bin/poms \
  -s $N \
  -q $Q \
  -C ./data/minirogue_poms.json \
  -b 1 -B $B \
  -w 1.5 -E -1.75 \
  -S 1337 \
  -P min \
  -O patch-policy=pending \
  -V 2 \
  -O viz_step=300 \
  -8 ./data/minirogue_snapshot.json \
  -1 ./data/minirogue_128x128.json
