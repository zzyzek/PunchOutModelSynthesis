#!/bin/bash
#
# LICENSE: CC0
#

node ../../src.js/img2tile.js \
  -E ./data/thkaspar_forestmicro.png \
  -P ./data/forestmicro_poms.json \
  -T ./data/forestmicro_tileset.png \
  -t ./data/forestmicro_flat_tileset.png \
  -M ./data/forestmicro_tilemap.json \
  -m ./data/forestmicro_flat_tilemap.json \
  -s 16 \
  -w 32 \
  -D 128,128,1 \
  -q 128,128,1 \
  -W uniform

node ../../src.js/img2tile.js \
  -E ./data/thkaspar_neondungeon.png \
  -P ./data/neondungeon_poms.json \
  -T ./data/neondungeon_tileset.png \
  -t ./data/neondungeon_flat_tileset.png \
  -M ./data/neondungeon_tilemap.json \
  -m ./data/neondungeon_flat_tilemap.json \
  -s 16 \
  -w 32 \
  -D 128,128,1 \
  -q 128,128,1 \
  -W flat

node ../../src.js/img2tile.js \
  -E ./data/thkaspar_neondirt.png \
  -P ./data/neondirt_poms.json \
  -T ./data/neondirt_tileset.png \
  -t ./data/neondirt_flat_tileset.png \
  -M ./data/neondirt_tilemap.json \
  -m ./data/neondirt_flat_tilemap.json \
  -s 8 \
  -w 16 \
  -D 128,128,1 \
  -q 128,128,1 \
  -W flat

node ../../src.js/img2tile.js \
  -E ./data/thkaspar_neonsnow.png \
  -P ./data/neonsnow_poms.json \
  -T ./data/neonsnow_tileset.png \
  -t ./data/neonsnow_flat_tileset.png \
  -M ./data/neonsnow_tilemap.json \
  -m ./data/neonsnow_flat_tilemap.json \
  -s 16 \
  -w 32 \
  -D 128,128,1 \
  -q 128,128,1 \
  -W flat

