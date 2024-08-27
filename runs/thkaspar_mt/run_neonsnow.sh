#!/bin/bash

../../bin/poms \
  -C ./data/neonsnow_poms.json \
  -b 1 -B 32 \
  -w 1 -E -1.5 \
  -S 1337 \
  -V 2 \
  -P min \
  -1 ./data/neonsnow_128x128.json
