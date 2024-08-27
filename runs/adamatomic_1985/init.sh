#!/bin/bash

node ../../src.js/img2tile.js \
  -E ./data/1985_example.png \
  -P ./data/1985_poms.json \
  -T ./data/1985_tileset.png \
  -t ./data/1985_flat_tileset.png \
  -M ./data/1985_tilemap.json \
  -m ./data/1985_flat_tilemap.json \
  -s 8 \
  -w 16 \
  -D 128,128,1 \
  -q 128,128,1 \
  -W flat

