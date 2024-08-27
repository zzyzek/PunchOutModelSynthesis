#!/bin/bash

poms="../../bin/poms"

s="48,48,1"
q='128,128,1'

## viz_step=1 slows the run down significantly
## but without it we can't get nice animations.
##

$poms \
  -C ./data/pillMortal_poms.json \
  -s "$s" \
  -q "$q" \
  -b 1 \
  -B 8,8,1 \
  -J 10000 \
  -w 1.0 \
  -E -1.95 \
  -1 ./data/pillMortal_tiled_128x128.json \
  -8 ./data/pillMortal_snapshot.json \
  -P min \
  -O patch-policy=pending \
  -S 1337 \
  -V 2

