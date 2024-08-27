#!/bin/bash

node ../../src.js/img2tile.js \
  -E ./data/vilenes_example.1.png \
  -P ./data/vilenes_poms.json \
  -T ./data/vilenes_tileset.png \
  -t ./data/vilenes_flat_tileset.png \
  -M ./data/vilenes_tilemap.json \
  -m ./data/vilenes_flat_tilemap.json \
  -s 16 \
  -w 32 \
  -D 128,128,1 \
  -q 128,128,1 \
  -W image


exit

## experimentation

node ../../src.js/img2tile.js \
  -E ./data/vilenes_example.png \
  -P ./data/vilenes_poms.json \
  -T ./data/vilenes_tileset.png \
  -t ./data/vilenes_flat_tileset.png \
  -M ./data/vilenes_tilemap.json \
  -m ./data/vilenes_flat_tilemap.json \
  -s 8 \
  -w 16 \
  -D 128,128,1 \
  -q 128,128,1 \
  -W flat



