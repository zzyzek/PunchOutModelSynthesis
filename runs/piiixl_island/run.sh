#!/bin/bash

N='32,32,1'
Q='128,128,1'
B='8:16'

../../bin/poms \
  -s $N \
  -q $Q \
  -C ./data/island_poms.json \
  -b 1 -B $B \
  -P min \
  -O patch-policy=pending \
  -w 2 -E -1.7 \
  -S 1337 \
  -V 2 \
  -O viz_step=100 \
  -8 ./data/island_snapshot.json \
  -1 ./data/island_128x128.json
