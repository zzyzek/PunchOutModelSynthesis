#!/bin/bash

../../bin/poms \
  -C ./data/1985_poms.json \
  -b 1 -B 16:32 \
  -w 4 -E -1.5 \
  -P 'min' \
  -S 1337 \
  -V 2 \
  -1 ./data/1985_128x128.json
