#!/bin/bash

N='42,42,1'
Q='128,128,1'
B='8:32'

../../bin/poms \
  -s $N \
  -q $Q \
  -C ./data/kyst_poms.json \
  -b 1 -B $B \
  -w 1 -E -1.5 \
  -P min \
  -O patch-policy=pending \
  -S 1337 \
  -V 2 \
  -O viz_step=100 \
  -8 ./data/kyst_snapshot.json \
  -1 ./data/kyst_128x128.json
