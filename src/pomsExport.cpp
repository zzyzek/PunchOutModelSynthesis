/*
 * To the extent possible under law, the person who associated CC0 with
 * this file has waived all copyright and related or neighboring rights
 * to this file.
 *      
 * You should have received a copy of the CC0 legalcode along with this
 * work. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 *
 */

#include "poms.hpp"

int POMS::exportPOMSBlock(const char *fn) {
  int32_t i, j, _fold_w=16, idir;
  int64_t count, cell;

  int32_t xyz[3];

  FILE *fp;

  int use_config_constraint = 1;

  fp = fopen(fn, "w");
  if (!fp) { return -1; }

  fprintf(fp, "{\n");

  fprintf(fp, "  \"rule\": [\n");

  count=0;
  for (i=0; i<m_tile_count; i++) {
    for (j=0; j<m_tile_count; j++) {
      for (idir=0; idir<6; idir++) {
        if (F(i,j,idir) > 0) {
          if (count>0) { fprintf(fp, ","); }
          fprintf(fp, "[%i,%i,%i,%f]",
              i, j, idir, F(i,j,idir));
          if ((count % _fold_w)==0) { fprintf(fp, "\n"); }
          count++;
        }
      }
    }
  }
  fprintf(fp, "\n  ],\n");

  fprintf(fp, "  \"name\": [\n");
  for (i=0; i<m_tile_count; i++) {
    if (i>0) { fprintf(fp, ","); }
    fprintf(fp, "\"%s\"", m_tile_name[i].c_str());
    if ((i % _fold_w)==0) { fprintf(fp, "\n"); }
  }
  fprintf(fp, "\n  ],\n");

  fprintf(fp, "  \"weight\": [\n");
  for (i=0; i<m_tile_count; i++) {
    if (i>0) { fprintf(fp, ","); }
    fprintf(fp, "%f", m_tile_weight[i]);
    if ((i % _fold_w)==0) { fprintf(fp, "\n"); }
  }
  fprintf(fp, "  ],\n");


  if (m_objMap.size() > 0) {
    for (i=0; i<m_objMap.size(); i++) {
      if (i>0) { fprintf(fp, ","); }
      fprintf(fp, "\"%s\"", m_objMap[i].c_str());
      if ((i % _fold_w)==0) { fprintf(fp, "\n"); }
    }
    fprintf(fp, "  ],\n");
  }

  if (m_tileset_ctx.tilecount > 0) {
    fprintf(fp, "  \"tileset\": {\n");
    fprintf(fp, "    \"image\": \"%s\",\n", m_tileset_ctx.image.c_str());
    fprintf(fp, "    \"tilecount\": %i,\n", m_tileset_ctx.tilecount);
    fprintf(fp, "    \"imageheight\": %i,\n", m_tileset_ctx.imageheight);
    fprintf(fp, "    \"imagewidth\": %i,\n", m_tileset_ctx.imagewidth);
    fprintf(fp, "    \"tileheight\": %i,\n", m_tileset_ctx.tileheight);
    fprintf(fp, "    \"tilewidth\": %i\n", m_tileset_ctx.tilewidth);
    fprintf(fp, "  },\n");
  }

  //fprintf(fp, "  \"flatMap\": [\n");
  //fprintf(fp, "  ],\n");
  //fprintf(fp, "  \"flatTileset\": [\n");
  //fprintf(fp, "  ],\n");

  fprintf(fp, "  \"boundaryConditions\": [\n");
  fprintf(fp, "    \"{\"x+\":{\"type\":\"tile\",\"value\":0},\n");
  fprintf(fp, "    \"{\"x-\":{\"type\":\"tile\",\"value\":0},\n");
  fprintf(fp, "    \"{\"y+\":{\"type\":\"tile\",\"value\":0},\n");
  fprintf(fp, "    \"{\"y-\":{\"type\":\"tile\",\"value\":0},\n");
  fprintf(fp, "    \"{\"z+\":{\"type\":\"tile\",\"value\":0},\n");
  fprintf(fp, "    \"{\"z-\":{\"type\":\"tile\",\"value\":0}\n");
  fprintf(fp, "  ],\n");

  fprintf(fp, "  \"constraint\": [\n");

  if (use_config_constraint) {
    for (i=0; i<m_constraint.size(); i++) {
      fprintf(fp, "  {");

      fprintf(fp, "\"type\":\"%c\",", m_constraint[i].type);
      fprintf(fp, "\"range\":{");
      fprintf(fp, " \"tile\":[%i,%i],", m_constraint[i].tile_range[0], m_constraint[i].tile_range[1]);
      fprintf(fp, " \"x\":[%i,%i],", m_constraint[i].size_range[0][0], m_constraint[i].size_range[0][1]);
      fprintf(fp, " \"y\":[%i,%i],", m_constraint[i].size_range[1][0], m_constraint[i].size_range[1][1]);
      fprintf(fp, " \"z\":[%i,%i]", m_constraint[i].size_range[2][0], m_constraint[i].size_range[2][1]);
      fprintf(fp, "}");



      fprintf(fp, "}");
      if (i<(m_constraint.size()-1)) { fprintf(fp, ","); }
      fprintf(fp, "\n");
    }
  }
  else {

    for (cell=0; cell<m_cell_count; cell++) {
      if (m_cell_pin[cell] == 0) { continue; }

      if (cellSize(m_plane, cell) == 1) {
        fprintf(fp, "  {\"type\":\"pin\", \"range\"{\"tile\":[%i,%i], \"x\":[%i,%i], \"y\":[%i,%i], \"z\":[%i,%i]}}",
            cellTile(m_plane,cell,0), cellTile(m_plane,cell,0)+1,
            xyz[0], xyz[0]+1,
            xyz[1], xyz[1]+1,
            xyz[2], xyz[2]+1);
        fprintf(fp, "  {\"type\":\"force\", \"range\"{\"tile\":[%i,%i], \"x\":[%i,%i], \"y\":[%i,%i], \"z\":[%i,%i]}}",
            cellTile(m_plane,cell,0), cellTile(m_plane,cell,0)+1,
            xyz[0], xyz[0]+1,
            xyz[1], xyz[1]+1,
            xyz[2], xyz[2]+1);
      }

    }
  }

  fprintf(fp, "  ],\n");

  fprintf(fp, "  \"size\":[%i,%i,%i]\n",
      (int)m_size[0], (int)m_size[1], (int)m_size[2]);

  fprintf(fp, "}\n");

  fclose(fp);
  return 0;
}

int POMS::exportPOMSGrid(const char *fn) {

  return 0;
}

int POMS::exportTiledJSON(const char *fn, int32_t multilayer, int export_quilt) {
  std::string _fn;
  _fn = fn;
  return exportTiledJSON(_fn, multilayer, export_quilt);
}

int POMS::exportTiledJSON(std::string &fn, int32_t multilayer, int export_quilt) {
  FILE *fp;
  int ret;

  fp = fopen(fn.c_str(), "w");
  if (!fp) { return -1; }

  ret = exportTiledJSON(fp, multilayer, export_quilt);
  if (ret!=0) { return ret; }

  return fclose(fp);
}

int POMS::exportTiledJSON(FILE *fp, int32_t multilayer, int export_quilt) {

  int64_t cell;

  int32_t x,y,z;
  int32_t max_tile_n=-1,
          t,
          tile_val=0;

  double opacity = 1.0,
         opacity_f=1.0;

  fprintf(fp, "{\n");
  fprintf(fp, "  \"backgroundcolor\":\"#ffffff\",\n");

  if (export_quilt) {
    fprintf(fp, "  \"height\": %i,\n", (int)m_quilt_size[1]);
    fprintf(fp, "  \"width\": %i,\n", (int)m_quilt_size[0]);
  }
  else {
    fprintf(fp, "  \"height\": %i,\n", (int)m_size[1]);
    fprintf(fp, "  \"width\": %i,\n", (int)m_size[0]);
  }

  if (multilayer==0) {

    fprintf(fp, "  \"layers\": [\n");

    fprintf(fp, "   {\n");
    fprintf(fp, "   \"data\": [");

    if (export_quilt) {
      for (cell=0; cell<m_quilt_cell_count; cell++) {
        if (cell>0) { fprintf(fp, ","); }
        fprintf(fp, "%c%i",
            ((cell%32)==0) ? '\n' : ' ',
            (m_quilt_tile[cell] >= 0) ? ((int)m_quilt_tile[cell]) : 0);
      }
    }
    else {
      for (cell=0; cell<m_cell_count; cell++) {
        if (cell>0) { fprintf(fp, ","); }
        fprintf(fp, "%c%i",
            ((cell%32)==0) ? '\n' : ' ',
            (cellSize(m_plane,cell)>0) ? ((int)cellTile(m_plane, cell, 0)) : 0);
      }
    }
    fprintf(fp, "    ],\n");

    fprintf(fp, "    \"name\":\"poms_layer\",\n");
    fprintf(fp, "    \"opacity\":1,\n");
    fprintf(fp, "    \"type\":\"tilelayer\",\n");
    fprintf(fp, "    \"visible\":true,\n");

    if (export_quilt) {
      fprintf(fp, "    \"height\": %i,\n", (int)m_quilt_size[1]);
      fprintf(fp, "    \"width\": %i,\n", (int)m_quilt_size[0]);
    }
    else {
      fprintf(fp, "    \"height\": %i,\n", (int)m_size[1]);
      fprintf(fp, "    \"width\": %i,\n", (int)m_size[0]);
    }

    fprintf(fp, "    \"x\": 0,\n");
    fprintf(fp, "    \"y\": 0\n");
    fprintf(fp, "    }");
    fprintf(fp, "  ],\n");

  }

  // layers are z height, so xy cross sections
  // of the fully realized 3d grid
  //
  else if (multilayer==1) {

    fprintf(fp, "  \"layers\": [\n");

    for (z=0; z<m_size[2]; z++) {

      fprintf(fp, "   {\n");
      fprintf(fp, "   \"data\": [");

      for (y=0; y<m_size[1]; y++) {
        for (x=0; x<m_size[0]; x++) {
          cell = xyz2cell(x,y,z);

          if (cellSize(m_plane, cell) > 0) {
            fprintf(fp, " %i%s",
                (int)cellTile(m_plane, cell, 0),
                ((x<(m_size[0]-1)) || (y<(m_size[1]-1))) ? "," : "" );
          }

        }
        fprintf(fp, "\n");
      }
      fprintf(fp, "    ],\n");

      fprintf(fp, "    \"name\":\"zlayer%i\",\n", (int)z);
      fprintf(fp, "    \"opacity\":1,\n");
      fprintf(fp, "    \"type\":\"tilelayer\",\n");
      fprintf(fp, "    \"visible\":true,\n");
      fprintf(fp, "    \"height\": %i,\n", (int)m_size[1]);
      fprintf(fp, "    \"width\": %i,\n", (int)m_size[0]);
      fprintf(fp, "    \"z\": %i,\n", (int)z);
      fprintf(fp, "    \"x\": 0,\n");
      fprintf(fp, "    \"y\": 0\n");
      fprintf(fp, "    }%s\n", ((z<(m_size[2]-1)) ? "," : ""));

    }

    fprintf(fp, "  ],\n");

  }

  // single xy plane (z=0) but multilayer for
  // different tiles in the cell.
  //
  else if (multilayer==2) {

    max_tile_n = cellSize(m_plane, 0);
    for (cell=0; cell<m_cell_count; cell++) {
      if (max_tile_n < cellSize(m_plane, cell)) {
        max_tile_n = cellSize(m_plane, cell);
      }
    }

    fprintf(fp, "  \"layers\": [\n");

    z = 0;
    for (t=0; t<max_tile_n; t++) {

      fprintf(fp, "   {\n");
      fprintf(fp, "   \"data\": [");

      for (y=0; y<m_size[1]; y++) {
        for (x=0; x<m_size[0]; x++) {
          cell = xyz2cell(x,y,z);
          fprintf(fp, " %2i%s",
              (int)((cellSize(m_plane,cell)>0)  ? cellTile(m_plane, cell, t) : -1),
              (((x<(m_size[0]-1)) || (y<(m_size[1]-1))) ? "," : "" ) );
        }
        fprintf(fp, "\n");
      }
      fprintf(fp, "    ],\n");

      opacity = 0.125;

      fprintf(fp, "    \"name\":\"tileLayer%i\",\n", (int)t);
      fprintf(fp, "    \"opacity\":%f,\n", opacity);
      fprintf(fp, "    \"type\":\"tilelayer\",\n");
      fprintf(fp, "    \"visible\":true,\n");
      fprintf(fp, "    \"height\": %i,\n", (int)m_size[1]);
      fprintf(fp, "    \"width\": %i,\n", (int)m_size[0]);
      fprintf(fp, "    \"w\": %i,\n", (int)t);
      fprintf(fp, "    \"x\": 0,\n");
      fprintf(fp, "    \"y\": 0\n");
      fprintf(fp, "    }%s\n", ((t<(max_tile_n-1)) ? "," : ""));

    }

    fprintf(fp, "  ],\n");


  }

  //---

  fprintf(fp, "  \"nextobjectid\" : 1,\n");
  fprintf(fp, "  \"orientation\" : \"orthogonal\",\n");
  fprintf(fp, "  \"properties\" : [],\n");
  fprintf(fp, "  \"renderorder\" : \"right-down\",\n");

  fprintf(fp, "  \"tileheight\": %i,\n", (int)m_tileset_ctx.tileheight);
  fprintf(fp, "  \"tilewidth\": %i,\n", (int)m_tileset_ctx.tilewidth);
  fprintf(fp, "  \"tilesets\": [{\n");
  fprintf(fp, "    \"firstgid\": 1,\n");
  fprintf(fp, "    \"rows\": %i,\n", (int)m_tileset_ctx.rows);
  fprintf(fp, "    \"columns\": %i,\n", (int)m_tileset_ctx.columns);
  fprintf(fp, "    \"name\": \"%s\",\n", m_tileset_ctx.name.c_str());
  fprintf(fp, "    \"image\": \"%s\",\n", m_tileset_ctx.image.c_str());
  fprintf(fp, "    \"imageheight\": %i,\n", (int)m_tileset_ctx.imageheight);
  fprintf(fp, "    \"imagewidth\": %i,\n", (int)m_tileset_ctx.imagewidth);
  fprintf(fp, "    \"margin\": 0,\n");
  fprintf(fp, "    \"spacing\": 0,\n");
  fprintf(fp, "    \"tilecount\": %i,\n", (int)m_tileset_ctx.tilecount);
  fprintf(fp, "    \"tileheight\": %i,\n", (int)m_tileset_ctx.tileheight);
  fprintf(fp, "    \"tilewidth\": %i\n", (int)m_tileset_ctx.tilewidth);
  fprintf(fp, "  }],\n");

  fprintf(fp, "  \"version\": 1\n");
  fprintf(fp, "}\n");

  //return fclose(fp);
  return 0;
}
