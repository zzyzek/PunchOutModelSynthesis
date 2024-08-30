/*
 * To the extent possible under law, the person who associated CC0 with
 * this file has waived all copyright and related or neighboring rights
 * to this file.
 *      
 * You should have received a copy of the CC0 legalcode along with this
 * work.  If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 *
 */

#ifndef TILED_EXPORT_HPP
#define TILED_EXPORT_HPP

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdint.h>

#include <vector>
#include <string>

typedef struct tiled_layer_type {
  std::vector< int64_t > data;
  std::string name;
  double opacity;
  std::string type;
  bool visible;
  int64_t height,
          width,
          x,
          y;

  //std::vector< tiled_layer_object_t > objects;

  tiled_layer_type() {
    name = "";
    opacity = 1.0;
    type = "tilelayer";
    visible = true;
    height = 0;
    width = 0;
    x = 0;
    y = 0;
  }

} tiled_layer_t;

typedef struct tiled_tileset_type {
  int64_t firstgid,
          columns;
  std::string name,
              image;
  int64_t imageheight,
          imagewidth,
          margin,
          spacing,
          tilecount,
          tileheight,
          tilewidth;

  tiled_tileset_type() {
    firstgid = 1;
    columns = 0;
    name = "";
    image = "";
    imageheight = 0;
    imagewidth = 0;
    margin = 0;
    spacing = 0;
    tilecount = 0;
    tileheight = 0;
    tilewidth = 0;
  }

} tiled_tileset_t;

typedef struct tiled_export_type {
  std::string backgroundcolor;
  int64_t height, width;

  std::vector< tiled_layer_type > layers;

  int64_t nextobjectid;
  std::string orientation;
  std::vector< std::string > properties;
  std::string renderorder;

  int64_t tileheight;
  int64_t tilewidth;

  std::vector< tiled_tileset_type > tilesets;

  int64_t version;

  tiled_export_type() {
    backgroundcolor = "#ffffff";
    height = 0;
    width = 0;
    nextobjectid = 1;
    orientation = "orthogonal";
    renderorder = "right-down";
    tileheight = 0;
    tilewidth = 0;
    version = 1;
  }

} tiled_export_t;

int exportTiledJSON(FILE *fp, tiled_export_t &T);

#endif
