#!/bin/bash

N='48,48,1'
Q='128,128,1'
B='12:24'

../../bin/poms \
  -s $N \
  -q $Q \
  -C ./data/mccaves_poms.json \
  -b 1 -B $B  \
  -w 2 -E -1.7 \
  -P min \
  -O patch-policy=pending \
  -S 1337  \
  -V 2 \
  -O viz_step=100 \
  -8 ./data/mccaves_snapshot.json \
  -1 ./data/mccaves_128x128.json
