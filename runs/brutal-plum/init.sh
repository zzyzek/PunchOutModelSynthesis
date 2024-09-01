#!/bin/bash

echo "# generating nopath"
mkdir -p brutal-plum_tile
node js/brutal-plum_path.js nopath > data/stickem_brutal-plum.conf
node js/stickem.js data/stickem_brutal-plum.conf brutal-plum_tile/ > ./data/brutal-plum_poms.json

echo "# generating 1path"
mkdir -p brutal-plum_tile_1path
node js/brutal-plum_path.js 1path > data/stickem_brutal-plum_1path.conf
node js/stickem.js data/stickem_brutal-plum_1path.conf brutal-plum_tile_1path/ > ./data/brutal-plum_poms_1path.json

echo "# generating 2path"
mkdir -p brutal-plum_tile_2path
node js/brutal-plum_path.js 2path > data/stickem_brutal-plum_2path.conf
node js/stickem.js data/stickem_brutal-plum_2path.conf brutal-plum_tile_2path/ > ./data/brutal-plum_poms_2path.json
