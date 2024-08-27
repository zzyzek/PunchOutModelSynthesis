/*
 * To the extent possible under law, the person who associated CC0 with
 * this file has waived all copyright and related or neighboring rights
 * to this file.
 *
 * You should have received a copy of the CC0 legalcode along with this
 * work.  If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdint.h>

#include "../TiledExport.hpp"

int main(int argc, char **argv) {
  int i;
  int64_t W, H,
          tile_w, tile_h;
  tiled_export_t T;

  tiled_tileset_t ts;
  tiled_layer_t tl;

  W = 31;
  H = 28;

  tile_w = 8;
  tile_h = 8;

  T.height = H;
  T.width = W;

  tl.width = W;
  tl.height = H;

  for (i=0; i<(tl.width*tl.height); i++) {
    tl.data.push_back( (i%32) + 1 );
  }

  T.tileheight = tile_h;
  T.tilewidth = tile_w;

  ts.tilecount = 189;
  ts.tilewidth = tile_w;
  ts.tileheight = tile_h;
  ts.image = "./pillMortal_tileset.png";

  T.layers.push_back(tl);
  T.tilesets.push_back(ts);

  exportTiledJSON(stdout, T);

}
