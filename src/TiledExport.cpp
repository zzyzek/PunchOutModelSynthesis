/*
 * To the extent possible under law, the person who associated CC0 with
 * this file has waived all copyright and related or neighboring rights
 * to this file.
 *      
 * You should have received a copy of the CC0 legalcode along with this
 * work.  If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 *
 */

#include "TiledExport.hpp"

int exportTiledJSON(FILE *fp, tiled_export_t &T, int hint_col) {
  size_t i, j;

  hint_col = ((hint_col <= 0) ? 32 : hint_col);

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
          ((j % hint_col) == 0) ? '\n' : ' ',
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
