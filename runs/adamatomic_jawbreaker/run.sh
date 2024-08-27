#!/bin/bash

../../bin/poms \
  -C ./data/jawbreaker_poms.json \
  -b 1 -B 8 \
  -w 2 -E -1.7 \
  -S 1337 \
  -V 2 \
  -1 ./data/jawbreaker_128x128.json
