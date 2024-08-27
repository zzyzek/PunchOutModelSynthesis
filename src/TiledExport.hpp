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

/*
typedef struct tiled_property_type {

  std::string name,
              type,
              propertytype;


} tiled_property_t;

typedef struct tiled_layer_object_type {
  int8_t  ellipse,
          point,
          visible;

  std::string name;
  int64_t id;

  double  width, height,
          x, y,
          rotation;

  std::vector< double > polygon;
  std::vector< double > polyline;
  std::vector< tiled_property_t > properties;

  std::string _template,
              type;

  tiled_text_t  text;

  int64_t gid;

} tiled_layer_object_t;
*/

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

int exportTiledJSON(FILE *fp, tiled_export_t &T) {
  size_t i, j;

  fprintf(fp, "{\n");
  fprintf(fp, "  \"backgroundcolor\":\"%s\",\n", T.backgroundcolor.c_str());
  fprintf(fp, "  \"height\": %i,\n", (int)T.height);
  fprintf(fp, "  \"width\": %i,\n", (int)T.width);

  fprintf(fp, "  \"layers\": [\n");
  for (i=0; i<T.layers.size(); i++) {
    fprintf(fp, "    {\n");

    fprintf(fp, "      \"data\": [\n");
    for (j=0; j<T.layers[i].data.size(); j++) {
      if (j>0) { fprintf(fp, ","); }
      fprintf(fp, "%c%i",
          ((j%32)==0) ? '\n' : ' ',
          (int)T.layers[i].data[j]);
    }
    fprintf(fp, "      ],\n");

    fprintf(fp, "    \"name\":\"%s\",\n", T.layers[i].name.c_str());
    fprintf(fp, "    \"opacity\":%f,\n", T.layers[i].opacity);
    fprintf(fp, "    \"type\":\"%s\",\n", T.layers[i].type.c_str());
    fprintf(fp, "    \"visible\": %s,\n", (T.layers[i].visible ? "true" : "false") );
    fprintf(fp, "    \"height\": %i,\n", (int)T.layers[i].height);
    fprintf(fp, "    \"width\": %i,\n", (int)T.layers[i].width);
    fprintf(fp, "    \"x\": %i,\n", (int)T.layers[i].x);
    fprintf(fp, "    \"y\": %i\n", (int)T.layers[i].y);
    fprintf(fp, "  }");
    if (i < (T.layers.size()-1)) { fprintf(fp, ","); }
    fprintf(fp, "\n");
  }
  fprintf(fp, "  ],\n");


  fprintf(fp, "  \"nextobjectid\" : %i,\n", (int)T.nextobjectid);
  fprintf(fp, "  \"orientation\" : \"%s\",\n", T.orientation.c_str());

  fprintf(fp, "  \"properties\" : [\n");
  for (i=0; i<T.properties.size(); i++) {
    if (i>0) { fprintf(fp, ","); }
    fprintf(fp, "\"%s\"", T.properties[i].c_str());
  }
  fprintf(fp, "  ],\n");

  fprintf(fp, "  \"renderorder\" : \"%s\",\n", T.renderorder.c_str());

  fprintf(fp, "  \"tileheight\": %i,\n", (int)T.tileheight);
  fprintf(fp, "  \"tilewidth\": %i,\n", (int)T.tilewidth);


  fprintf(fp, "  \"tilesets\": [\n");
  for (i=0; i<T.tilesets.size(); i++) {
    fprintf(fp, "    {\n");
    fprintf(fp, "      \"firstgid\": %i,\n", (int)T.tilesets[i].firstgid);
    //fprintf(fp, "      \"rows\": %i,\n", (int)T.tilesets[i].rows);
    fprintf(fp, "      \"columns\": %i,\n", (int)T.tilesets[i].columns);
    fprintf(fp, "      \"name\": \"%s\",\n", T.tilesets[i].name.c_str());
    fprintf(fp, "      \"image\": \"%s\",\n", T.tilesets[i].image.c_str());
    fprintf(fp, "      \"imageheight\": %i,\n", (int)T.tilesets[i].imageheight);
    fprintf(fp, "      \"imagewidth\": %i,\n", (int)T.tilesets[i].imagewidth);
    fprintf(fp, "      \"margin\": %i,\n", (int)T.tilesets[i].margin);
    fprintf(fp, "      \"spacing\": %i,\n", (int)T.tilesets[i].spacing);
    fprintf(fp, "      \"tilecount\": %i,\n", (int)T.tilesets[i].tilecount);
    fprintf(fp, "      \"tileheight\": %i,\n", (int)T.tilesets[i].tileheight);
    fprintf(fp, "      \"tilewidth\": %i\n", (int)T.tilesets[i].tilewidth);
    fprintf(fp, "    }");
    if (i<(T.tilesets.size()-1)) { fprintf(fp, ","); }
    fprintf(fp, "\n");
  }


  fprintf(fp, "  ],\n");

  fprintf(fp, "  \"version\": %i\n", (int)T.version);
  fprintf(fp, "}\n");

  return 0;
}

#endif
