#!/bin/bash

../../bin/poms \
  -C ./data/wangblob_poms.json \
  -b 1 -B 16 \
  -w 1 -E -1.5 \
  -S 1337 \
  -V 2 \
  -1 ./data/wangblob_128x128.json
