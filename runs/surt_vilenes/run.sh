#!/bin/bash

../../bin/poms \
  -C ./data/vilenes_poms.json \
  -b 1 -B 8:32 \
  -w 15.0 -E -1.0 \
  -S 1339 \
  -1 ./data/vilenes_128x128.json \
  -V 2 \
  -P wf \
  -8 ./data/vilenes_snapshot.json

