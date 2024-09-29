#!/bin/bash
#
# LICENSE: CC0
#

node ../../src.js/img2tile.js \
  -E ./data/kingel_minirogue.png \
  -P ./data/minirogue_poms.json \
  -T ./data/minirogue_tileset.png \
  -t ./data/minirogue_flat_tileset.png \
  -M ./data/minirogue_tilemap.json \
  -m ./data/minirogue_flat_tilemap.json \
  -s 8 \
  -w 16 \
  -u \
  -D 128,128,1 \
  -q 128,128,1 \
  -W image


