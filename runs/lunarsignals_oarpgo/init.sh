#!/bin/bash

# This creates a 3x3 tiled window for this tileset:
#
# -u              rule wrap, will automatically create frame restrictions in POMS file
# -s 16           base tile size in pixels (16x16 px)
# -w 48           window size in pixels (48x48 px)
# -o 16           offset in window, in pixels (16x16 px), so in center of the 48x48 px window
# -B 32           supertile band 32px widw (full 48px high) which will be suitably rotated
# -D ...          'size' dimension of block. We'll be using our own so just set to grid/quilt size
# -q ...          grid/quilt size
# -W uniform      uniform weighting of tiles. From inspection, this gives nice aesthetic results
#


sfx="wUbW3x3"

echo "creating 'oarpgo_poms_${sfx}'"
node ../../src.js/img2tile.js \
  -E ./data/lunarsignals_oarpgo.png \
  -P ./data/oarpgo_poms_${sfx}.json \
  -T ./data/oarpgo_tileset_${sfx}.png \
  -t ./data/oarpgo_flat_tileset_${sfx}.png \
  -M ./data/oarpgo_tilemap_${sfx}.json \
  -m ./data/oarpgo_flat_tilemap_${sfx}.json \
  -u \
  -s 16 \
  -o 16 \
  -w 48 \
  -B 32 \
  -D 128,128,1 \
  -q 128,128,1 \
  -W uniform


