#!/bin/bash

N='32,32,1'
Q='128,128,1'
B='8:16'

../../bin/poms \
  -s $N \
  -q $Q \
  -C ./data/blowharder_poms.json \
  -b 1 -B $B \
  -P min \
  -O patch-policy=pending \
  -w 1 -E -1.5 \
  -S 1337 \
  -V 2 \
  -1 ./data/blowharder_128x128.json
