#!/bin/bash

node ../../src.js/img2tile.js \
  -E ./data/jawbreaker_fixed.png \
  -P ./data/jawbreaker_poms.json \
  -T ./data/jawbreaker_tileset.png \
  -t ./data/jawbreaker_flat_tileset.png \
  -M ./data/jawbreaker_tilemap.json \
  -m ./data/jawbreaker_flat_tilemap.json \
  -s 8 \
  -w 16 \
  -D 128,128,1 \
  -q 128,128,1 \
  -W flat

