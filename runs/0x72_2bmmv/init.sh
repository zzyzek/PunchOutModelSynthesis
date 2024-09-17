#!/bin/bash

node ../../src.js/img2tile.js \
  -E ./data/2bit_micro_metroidvania.png \
  -P ./data/2bmmv_poms.json \
  -T ./data/2bmmv_tileset.png \
  -t ./data/2bmmv_flat_tileset.png \
  -M ./data/2bmmv_tilemap.json \
  -m ./data/2bmmv_flat_tilemap.json \
  -s 24 \
  -w 48 \
  -D 128,128,1 \
  -q 128,128,1 \
  -W uniform

