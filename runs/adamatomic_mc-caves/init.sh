#!/bin/bash

node ../../src.js/img2tile.js \
  -E ./data/bw.png \
  -P ./data/mccaves_poms.json \
  -T ./data/mccaves_tileset.png \
  -t ./data/mccaves_flat_tileset.png \
  -M ./data/mccaves_tilemap.json \
  -m ./data/mccaves_flat_tilemap.json \
  -s 8 \
  -w 16 \
  -D 128,128,1 \
  -q 128,128,1 \
  -W uniform

