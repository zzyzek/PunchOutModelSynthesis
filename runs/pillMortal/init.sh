#!/bin/bash

node ../../src.js/img2tile.js \
  -E ./data/pill_mortal_map0.png \
  -P ./data/pillMortal_poms.json \
  -T ./data/pillMortal_tileset.png \
  -t ./data/pillMortal_flat_tileset.png \
  -M ./data/pillMortal_tilemap.json \
  -m ./data/pillMortal_flat_tilemap.json \
  -s 8,8 \
  -w 16,16 \
  -D 32,48 \
  -V 1

