#!/bin/bash

N='52,52,1'
Q='128,128,1'
B='24:32'

../../bin/poms \
  -s $N \
  -q $Q \
  -C ./data/psygen_poms.json \
  -b 1 -B $B \
  -w 1 -E -1.5 \
  -P min \
  -O patch-policy=cone-\
  -S 1337 \
  -V 2 \
  -O viz_step=200 \
  -8 ./data/psygen_snaphost.json \
  -1 ./data/psygen_128x128.json


