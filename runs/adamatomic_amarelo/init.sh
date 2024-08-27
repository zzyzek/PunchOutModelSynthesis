#!/bin/bash

node ../../src.js/img2tile.js \
  -E ./data/amarelo_sample.png \
  -P ./data/amarelo_poms.json \
  -T ./data/amarelo_tileset.png \
  -t ./data/amarelo_flat_tileset.png \
  -M ./data/amarelo_tilemap.json \
  -m ./data/amarelo_flat_tilemap.json \
  -s 32 \
  -w 64 \
  -D 128,128,1 \
  -q 128,128,1 \
  -W flat

