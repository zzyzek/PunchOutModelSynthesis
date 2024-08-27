#!/bin/bash

node ../../src.js/img2tile.js \
  -E ./data/wangblob_sample.png \
  -P ./data/wangblob_poms.json \
  -T ./data/wangblob_tileset.png \
  -t ./data/wangblob_flat_tileset.png \
  -M ./data/wangblob_tilemap.json \
  -m ./data/wangblob_flat_tilemap.json \
  -u \
  -s 16 \
  -w 32 \
  -D 128,128,1 \
  -q 128,128,1 \
  -W image

