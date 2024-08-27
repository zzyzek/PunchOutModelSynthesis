#!/bin/bash

node ../../src.js/img2tile.js \
  -E ./data/island_map.png \
  -P ./data/island_poms.json \
  -T ./data/island_tileset.png \
  -t ./data/island_flat_tileset.png \
  -M ./data/island_tilemap.json \
  -m ./data/island_flat_tilemap.json \
  -s 8 \
  -w 16 \
  -D 128,128,1 \
  -q 128,128,1 \
  -W flat

