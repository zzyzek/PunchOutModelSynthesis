#!/bin/bash

node ../../src.js/img2tile.js \
  -E ./data/kyst_example.png \
  -P ./data/kyst_poms.json \
  -T ./data/kyst_tileset.png \
  -t ./data/kyst_flat_tileset.png \
  -M ./data/kyst_tilemap.json \
  -m ./data/kyst_flat_tilemap.json \
  -s 8 \
  -w 16 \
  -D 128,128,1 \
  -q 128,128,1 \
  -W uniform

