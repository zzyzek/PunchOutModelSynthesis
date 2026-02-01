#!/bin/bash

../../bin/poms \
  -C ./scripts/i2d_poms.json \
  -b 1 -B 16 \
  -w 1 -E -1.5 \
  -S 1337 \
  -V 1 \
  -1 ./scripts/i2d_128x128.json
