#!/bin/bash

## image
../../bin/poms \
  -s 256,44,1 \
  -q 256,44,1 \
  -C ./data/amarelo_poms.json \
  -b 1 -B 12:48 \
  -w 2.5 -E -1.25 \
  -S 1337 \
  -V 2 \
  -1 ./amarelo_256x44.json

exit

###
../../bin/poms \
  -C ./data/amarelo_poms.json \
  -b 1 -B 32 \
  -w 1 -E -1.5 \
  -S 1337 \
  -V 2 \
  -1 ./amarelo_128x128.json
