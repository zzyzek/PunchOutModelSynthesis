#!/bin/bash

poms="../../bin/poms"

## viz_step=1 slows the run down significantly
## but without it we can't get nice animations.
#
#  -O viz_step=1 \
##

$poms \
  -C ./data/forestmicro_poms.json \
  -s 48,48,1 \
  -q 128,128,1 \
  -b 1 \
  -B 8,8,1 \
  -J 10000 \
  -w 1.0 \
  -E -1.95 \
  -1 data/forestmicro_128x128.json \
  -8 data/forestmicro_snapshot.json \
  -P wf \
  -O patch-policy=wf \
  -S 1337 \
  -V 1


