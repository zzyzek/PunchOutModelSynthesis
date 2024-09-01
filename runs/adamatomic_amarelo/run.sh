#!/bin/bash

## image
../../bin/poms \
  -C ./data/amarelo_poms.json \
  -s 128,128,1 \
  -q 128,128,1 \
  -b 1 \
  -B 24:32  \
  -J 10000 \
  -w 1.5  \
  -E -1.25 \
  -P 'wf=xyz' \
  -O 'viz_step=50' \
  -O 'patch-policy=pending' \
  -S 1337 \
  -V 1 \
  -1 ./data/amarelo_128x128.json \
  -8 ./data/amarelo_snapshot.json


