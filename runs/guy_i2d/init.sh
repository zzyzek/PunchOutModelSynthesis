#!/bin/bash

node ../../src.js/img2tile.js \
  -E ./data/sample1.png \
  -P ./data/i2d_poms.json \
  -T ./data/i2d_tileset.png \
  -t ./data/i2d_flat_tileset.png \
  -M ./data/i2d_tilemap.json \
  -m ./data/i2d_flat_tilemap.json \
  -u \
  -s 17 \
  -w 34 \
  -D 128,128,1 \
  -q 128,128,1 \
  -W flat

