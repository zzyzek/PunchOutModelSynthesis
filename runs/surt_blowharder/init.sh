#!/bin/bash

node ../../src.js/img2tile.js \
  -E ./data/blowhard_sms_map_0.png \
  -P ./data/blowharder_poms.json \
  -T ./data/blowharder_tileset.png \
  -t ./data/blowharder_flat_tileset.png \
  -M ./data/blowharder_tilemap.json \
  -m ./data/blowharder_flat_tilemap.json \
  -s 16 \
  -w 32 \
  -D 128,128,1 \
  -q 128,128,1 \
  -W uniform

