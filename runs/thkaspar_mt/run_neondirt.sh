#!/bin/bash


N='58,58,1'
Q='128,128,1'
B='32'

../../bin/poms \
  -s $N \
  -q $Q \
  -C ./data/neondirt_poms.json \
  -b 1 -B $B \
  -w 1 -E -1.5 \
  -S 1337 \
  -P min  \
  -O patch-policy=pending \
  -V 2 \
  -O viz_step=300 \
  -8 ./data/neondirt_snapshot.json \
  -1 ./data/neondirt_128x128.json
