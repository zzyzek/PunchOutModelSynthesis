#!/bin/bash

N='32,32,1'
Q='128,128,1'
B=16

../../bin/poms \
  -s $N \
  -q $Q \
  -C ./data/neondungeon_poms.json \
  -b 1 -B $B \
  -w 1 -E -1.5 \
  -S 1337 \
  -V 2 \
  -P min \
  -O patch-policy=pending \
  -1 ./data/neondungeon_128x128.json
