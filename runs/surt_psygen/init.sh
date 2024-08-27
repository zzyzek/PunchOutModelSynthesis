#!/bin/bash

node ../../src.js/img2tile.js \
  -E ./data/psygen_mockup.png \
  -P ./data/psygen_poms.json \
  -T ./data/psygen_tileset.png \
  -t ./data/psygen_flat_tileset.png \
  -M ./data/psygen_tilemap.json \
  -m ./data/psygen_flat_tilemap.json \
  -s 16 \
  -w 32 \
  -D 128,128,1 \
  -q 128,128,1 \
  -W uniform

