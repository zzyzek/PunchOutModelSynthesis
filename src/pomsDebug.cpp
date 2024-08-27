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

//---
//
void POMS::printDebugCellSize(void) {
  int32_t x,y,z;
  int64_t cell;

  for (z=0; z<m_size[2]; z++) {
    printf("#[.,.,%i]\n", (int)z);
    for (y=0; y<m_size[1]; y++) {
      for (x=0; x<m_size[0]; x++) {
        cell = xyz2cell(x,y,z);
        printf(" %3i", (int)cellSize( m_plane, cell ) );
      }
      printf("\n");
    }
    printf("\n");
  }
  printf("\n");
}

void POMS::printDebugCellSize(int32_t *order) {
  int i;
  int32_t x,y,z;
  int64_t cell;
  int32_t tile, n_tile;

  int32_t v[3], p[3];

  int32_t default_order[3];
  int32_t a,b,c;
  char dim_code[3], pfx;

  default_order[0] = 2;
  default_order[1] = 1;
  default_order[2] = 0;

  dim_code[0] = 'x';
  dim_code[1] = 'y';
  dim_code[2] = 'z';

  if (!order) { order = default_order; }

  a = order[0];
  b = order[1];
  c = order[2];

  for (p[a]=0; p[a]<m_size[a]; p[a]++) {

    printf("# %c:%i,%c:.,%c:.\n", dim_code[a], p[a], dim_code[b], dim_code[c]);
    for (p[b]=(m_size[b]-1); p[b]>=0; p[b]--) {
      for (p[c]=0; p[c]<m_size[c]; p[c]++) {

        cell = vec2cell(p);
        tile = cellTile(m_plane, cell, 0);
        n_tile = cellSize(m_plane, cell);

        printf(" %2i", (int)n_tile);
      }
      printf("\n");
    }
    printf("\n");
  }

  printf("\n");
}



/*
void POMS::printDebugConflict(void) {
  int32_t p[3];
  cell2vec(p, m_conflict_cell);
  printf("conflict [%i,%i,%i]{%i} tile:%i idir:%i type:%i\n",
      (int)p[0], (int)p[1], (int)p[2], (int)m_conflict_cell,
      (int)m_conflict_tile, (int)m_conflict_idir,
      (int)m_conflict_type);

}
*/

void POMS::printDebugConflict(void) {

  int32_t v[3], tile_idx=0;
  std::string s;

  s = "";
  switch (m_conflict_type) {
    case POMS_CONFLICT_BOUNDARY: s = "boundary"; break;
    case POMS_CONFLICT_NO_SUPPORT: s = "no-support"; break;
    case POMS_CONFLICT_EMPTY: s = "empty"; break;
    case POMS_CONFLICT_MISMATCH: s = "mismatch"; break;
    case POMS_CONFLICT_NEGATIVE_SUPPORT_SANITY_ERROR: s = "sanity"; break;
    default: s = "?"; break;
  }

  cell2vec(v, m_conflict_cell);
  if ((m_conflict_tile >= 0) &&
      (m_conflict_tile < m_tile_name.size())) {
    tile_idx = m_conflict_tile;
  }

  printf("conflict{cell:[%i,%i,%i](%i), tile:%s(%i), idir:%i, type:%s(%i)}",
      (int)v[0], (int)v[1], (int)v[2], (int)m_conflict_cell,
      m_tile_name[tile_idx].c_str(), (int)m_conflict_tile,
      (int)m_conflict_idir,
      s.c_str(), (int)m_conflict_type);

}

//---
//
void POMS::printDebug(int32_t show_rule) {

  int32_t i, j, k, idir;
  int32_t fold_name = 4;

  int32_t tile,
          tile_idx,
          n_tile;
  int64_t cell, _cell;
  std::vector< int32_t > v;

  int show_constraint = 1;

  printf("m_tile_name[%i]{m_tile_weight[%i],m_tile_group[%i]}:\n",
      (int)m_tile_count, (int)m_tile_weight.size(), (int)m_tile_group.size());
  for (i=0; i<m_tile_count; i++) {
    if (i==0) { printf(" "); }
    if ((i>0) && ((i%fold_name)==0)) {
      printf("\n ");
    }
    printf(" %3i(%3s){%0.2f,%i}",
        i, m_tile_name[i].c_str(),
        (double)m_tile_weight[i],
        (int)m_tile_group[i] );

    if (i < m_objMap.size()) {
      printf(" (%s)", m_objMap[i].c_str());
    }
  }
  printf("\n");

  if (show_rule) {
    printf("rule[6][%i][%i]:\n", (int)m_tile_count, (int)m_tile_count);
    for (idir=0; idir<6; idir++) {
      for (i=0; i<m_tile_count; i++) {
        printf("tile[%3i](%s):", i, m_dir_desc[idir].c_str());
        for (j=0; j<m_tile_count; j++) {
          if (F(i,j,idir) > m_zero) {
            printf(" %i", j);
          }
        }
        printf("\n");
      }
      printf("\n");
    }

    printf("m_tileAdj[%i]:\n", (int)m_tileAdj.size());

    for (idir=0; idir<6; idir++) {
      for (tile=0; tile<m_tile_count; tile++) {
        printf("m_tileAdj[%i][](tile:%i,dir:%i):",
            (int)(idir*m_tile_count + tile), (int)tile, (int)idir);
        for (i=0; i<m_tileAdj[ idir*m_tile_count + tile ].size(); i++) {
          printf(" %i", (int)m_tileAdj[ idir*m_tile_count + tile ][i]);
        }
        printf("\n");
      }
    }


  }

  if (show_constraint) {
    printf("{ \"constraint\" : [\n");
    for (i=0; i<m_constraint.size(); i++) {
      if (i>0) { printf(",\n"); }
      printf("  {\"type\":%c, \"tile_range\":[%i,%i], \"size_range\":[[%i,%i],[%i,%i],[%i,%i]]}",
          m_constraint[i].type,
          (int)m_constraint[i].tile_range[0], (int)m_constraint[i].tile_range[1],
          (int)m_constraint[i].size_range[0][0], (int)m_constraint[i].size_range[0][1],
          (int)m_constraint[i].size_range[1][0], (int)m_constraint[i].size_range[1][1],
          (int)m_constraint[i].size_range[2][0], (int)m_constraint[i].size_range[2][1]);
    }
    printf("\n]}\n");
  }

  printf("m_size: [%i,%i,%i]\n",
      (int)m_size[0],
      (int)m_size[1],
      (int)m_size[2]);

  printf("m_block_size: [%i,%i,%i]\n",
      (int)m_block_size[0],
      (int)m_block_size[1],
      (int)m_block_size[2]);

  printf("m_soften_size: [%i,%i,%i]\n",
      (int)m_soften_size[0],
      (int)m_soften_size[1],
      (int)m_soften_size[2]);

  printf("m_seed: %i\n", (int)m_seed);
  printf("m_retry_max: %i\n", (int)m_retry_max);

  v.resize(3);

  printf("[plane.cell](x,y,z){cellSize} [%i]:\n", (int)m_cell_count);
  for (cell=0; cell<m_cell_count; cell++) {
    cell2vec(v, cell);

    printf("[%i.%i](%i,%i,%i){n:%i}:",
        (int)m_plane, (int)cell,
        (int)v[0], (int)v[1], (int)v[2],
        (int)cellSize(m_plane, cell));

    n_tile = cellSize(m_plane, cell);
    for (tile_idx=0; tile_idx<n_tile; tile_idx++) {
      printf(" %i", (int)cellTile(m_plane, cell, tile_idx));
    }
    printf("\n");
  }

}

void POMS::printDebugBlock() {
  int64_t cell, n_tile, tile_idx;
  int32_t v[3];

  printf("[plane.cell](x,y,z){cellSize} [%i]:\n", (int)m_cell_count);
  for (cell=0; cell<m_cell_count; cell++) {
    cell2vec(v, cell);

    printf("[%i.%i](%i,%i,%i){n:%i}:",
        (int)m_plane, (int)cell,
        (int)v[0], (int)v[1], (int)v[2],
        (int)cellSize(m_plane, cell));

    n_tile = cellSize(m_plane, cell);
    for (tile_idx=0; tile_idx<n_tile; tile_idx++) {
      printf(" %i", (int)cellTile(m_plane, cell, tile_idx));
    }
    printf("\n");
  }

}


void POMS::printDebugGrid(void) {
  int32_t x,y,z;
  int64_t cell;
  int32_t tile;


  for (z=0; z<m_size[2]; z++) {
    printf("#z:%i\n", (int)z);
    for (y=0; y<m_size[1]; y++) {
      for (x=0; x<m_size[0]; x++) {
        cell = xyz2cell(x,y,z);
        tile = cellTile(m_plane, cell, 0);

        printf(" %3i", (int)tile);
      }
      printf("\n");
    }
    printf("\n");
  }

  printf("\n");

}

void POMS::printDebugGrid(int32_t *order) {
  int i;
  int32_t x,y,z;
  int64_t cell;
  int32_t tile, n_tile;

  int32_t v[3], p[3];

  int32_t default_order[3];
  int32_t a,b,c;
  char dim_code[3], pfx;

  default_order[0] = 2;
  default_order[1] = 1;
  default_order[2] = 0;

  dim_code[0] = 'x';
  dim_code[1] = 'y';
  dim_code[2] = 'z';

  if (!order) { order = default_order; }

  a = order[0];
  b = order[1];
  c = order[2];

  for (p[a]=0; p[a]<m_size[a]; p[a]++) {
    printf("#%c:%i [%c:%c]\n", dim_code[a], (int)p[a], dim_code[b], dim_code[c]);
    //for (p[b]=0; p[b]<m_size[b]; p[b]++) {
    for (p[b]=(m_size[b]-1); p[b]>=0; p[b]--) {
      for (p[c]=0; p[c]<m_size[c]; p[c]++) {

        cell = vec2cell(p);
        tile = cellTile(m_plane, cell, 0);
        n_tile = cellSize(m_plane, cell);

        pfx = ' ';
        if (m_cell_pin[cell]) { pfx = '@'; }

        if      (n_tile == 0) { printf("    !%c", pfx); }
        else if (n_tile  > 1) { printf("    *%c", pfx); }
        else                  { printf(" %4i%c", (int)tile, pfx); }
      }
      printf("\n");
    }
    printf("\n");
  }

  printf("\n");
}

void POMS::printDebugQuiltGrid(int32_t *order) {
  int i;
  int32_t x,y,z;
  int64_t cell;
  int32_t tile;

  int32_t v[3], p[3];

  int32_t default_order[3];
  int32_t a,b,c;
  char dim_code[3];

  default_order[0] = 2;
  default_order[1] = 1;
  default_order[2] = 0;

  dim_code[0] = 'x';
  dim_code[1] = 'y';
  dim_code[2] = 'z';

  if (!order) { order = default_order; }

  a = order[0];
  b = order[1];
  c = order[2];

  for (p[a]=0; p[a]<m_quilt_size[a]; p[a]++) {
    printf("#%c:%i [%c:%c]\n", dim_code[a], (int)p[a], dim_code[b], dim_code[c]);
    //for (p[b]=0; p[b]<m_quilt_size[b]; p[b]++) {
    for (p[b]=(m_quilt_size[b]-1); p[b]>=0; p[b]--) {
      for (p[c]=0; p[c]<m_quilt_size[c]; p[c]++) {

        cell = vec2cell(p, m_quilt_size);
        tile = m_quilt_tile[cell];

        if (tile < 0) { printf("    ."); }
        else          { printf(" %4i", (int)tile); }
      }
      printf("\n");
    }
    printf("\n");
  }

  printf("\n");

}

void POMS::printDebugAC4Dirty(int32_t plane) {
  int64_t cell;
  int32_t x,y,z;

  for (z=0; z<m_size[2]; z++) {
    for (y=0; y<m_size[1]; y++) {
      for (x=0; x<m_size[0]; x++) {
        cell = xyz2cell(x,y,z);
        printf(" %i", m_ac4_dirty[plane][cell]);
      }
      printf("\n");
    }
    printf("\n");
  }

}

// trying to get a handle on memory usage
//
void POMS::printDebugMemStat(void) {
  int64_t i, s, tot_s=0;
  double d_gb = 0.0;

  printf("# mem stat estimates:\n");

  s = 0;
  for (i=0; i<m_tile_name.size(); i++) { s += m_tile_name[i].size(); }
  d_gb = (double)s/(1024.0*1024.0*1024.0);
  printf("# size(m_tile_name[%i]): %i (%f)\n",
      (int)m_tile_name.size(), (int)s, d_gb);
  tot_s += s;

  s=0;
  for (i=0; i<m_tileAdj.size(); i++) { s += m_tileAdj[i].size()*sizeof(int32_t); }
  d_gb = (double)s/(1024.0*1024.0*1024.0);
  printf("# size(m_tileAdj[%i]): %i (%f)\n", (int)m_tileAdj.size(), (int)s, d_gb);
  tot_s += s;

  s= m_tile_rule.size()*sizeof(int8_t) ;
  d_gb = (double)s/(1024.0*1024.0*1024.0);
  printf("# size(m_tile_rule): %i (%f)\n", (int)s, d_gb);
  tot_s += s;

  s = m_tile_weight.size()*sizeof(double);
  d_gb = (double)s/(1024.0*1024.0*1024.0);
  printf("# size(m_tile_weight): %i (%f)\n", (int)s, d_gb);
  tot_s += s;

  s = m_tile_group.size()*sizeof(int32_t);
  d_gb = (int32_t)s/(1024.0*1024.0*1024.0);
  printf("# size(m_tile_group): %i (%f)\n", (int)s, d_gb);
  tot_s += s;

  s = 0;
  for (i=0; i<m_objMap.size(); i++) { s += m_objMap[i].size(); }
  d_gb = (double)s/(1024.0*1024.0*1024.0);
  printf("# size(m_objMap[%i]): %i (%f)\n", (int)m_objMap.size(), (int)s, d_gb);
  tot_s += s;

  s = 0;
  //for (i=0; i<3; i++) { s += m_tile[i].size()*sizeof(int32_t); }
  //for (i=0; i<2; i++) { s += m_tile[i].size()*sizeof(int32_t); }
  for (i=0; i<2; i++) { s += m_tile[i].size()*m_tile_data_size; }
  d_gb = (double)s/(1024.0*1024.0*1024.0);
  printf("# size(m_tile[%i]): %i (%f)\n", 2, (int)s, d_gb);
  tot_s += s;

  s = 0;
  //for (i=0; i<3; i++) { s += m_tile_bp[i].size()*sizeof(int32_t); }
  //for (i=0; i<2; i++) { s += m_tile_bp[i].size()*sizeof(int32_t); }
  for (i=0; i<2; i++) { s += m_tile_bp[i].size()*m_tile_data_size; }
  d_gb = (double)s/(1024.0*1024.0*1024.0);
  printf("# size(m_tile_bp[%i]): %i (%f)\n", 2, (int)s, d_gb);
  tot_s += s;

  s = 0;
  //for (i=0; i<3; i++) { s += m_tile_size[i].size()*sizeof(int32_t); }
  //for (i=0; i<2; i++) { s += m_tile_size[i].size()*sizeof(int32_t); }
  for (i=0; i<2; i++) { s += m_tile_size[i].size()*m_tile_data_size; }
  d_gb = (double)s/(1024.0*1024.0*1024.0);
  printf("# size(m_tile_size[%i]): %i (%f)\n", 2, (int)s, d_gb);
  tot_s += s;

  s = 0;
  s += m_quilt_tile.size()*m_tile_data_size;
  d_gb = (double)s/(1024.0*1024.0*1024.0);
  printf("# size(m_quilt_tile): %i (%f)\n", (int)s, d_gb);
  tot_s += s;

  s = 0;
  for (i=0; i<2; i++) { s += m_visited[i].size()*sizeof(int8_t); }
  d_gb = (double)s/(1024.0*1024.0*1024.0);
  printf("# size(m_visited[%i]): %i (%f)\n", 2, (int)s, d_gb);
  tot_s += s;

  s = 0;
  for (i=0; i<2; i++) { s += m_cell_queue[i].size()*sizeof(int64_t); }
  d_gb = (double)s/(1024.0*1024.0*1024.0);
  printf("# size(m_cell_queue[%i]): %i (%f)\n", 2, (int)s, d_gb);
  tot_s += s;

  s = 0;
  //for (i=0; i<3; i++) { s += m_tile_support[i].size()*sizeof(int32_t); }
  //for (i=0; i<2; i++) { s += m_tile_support[i].size()*sizeof(int32_t); }
  //for (i=0; i<2; i++) { s += m_tile_support[i].size()*m_tile_support_data_size; }
  for (i=0; i<2; i++) {
    if (m_tile_support_option == POMS_OPTIMIZATION_AC4_NONE) {
      s += m_ac4_flat[i].size_estimate();
    }
    else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER4) {
      s += m_ac4_tier4[i].size_estimate();
    }
    else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER6) {
      s += m_ac4_tier6[i].size_estimate();
    }
  }
  d_gb = (double)s/(1024.0*1024.0*1024.0);
  printf("# size(tileSupport{%i}[%i]): %i (%f)\n", m_tile_support_option, 2, (int)s, d_gb);
  tot_s += s;

  s = 0;
  for (i=0; i<2; i++) { s += m_cell_tile_visited[i].size()*sizeof(int8_t); }
  d_gb = (double)s/(1024.0*1024.0*1024.0);
  printf("# size(m_cell_tile_visited[%i]): %i (%f)\n", 2, (int)s, d_gb);
  tot_s += s;

  s = 0;
  //for (i=0; i<2; i++) { s += m_cell_tile_queue[i].size()*sizeof(int64_t); }
  //for (i=0; i<1; i++) { s += m_cell_tile_queue[i].size()*sizeof(int64_t); }
  for (i=0; i<1; i++) { s += m_cell_tile_queue[i].size()*m_cell_tile_queue_data_size; }
  d_gb = (double)s/(1024.0*1024.0*1024.0);
  printf("# size(m_cell_tile_queue[%i]): %i (%f)\n", 1, (int)s, d_gb);
  tot_s += s;

  s = m_prefatory.size()*sizeof(int32_t);
  d_gb = (double)s/(1024.0*1024.0*1024.0);
  printf("# size(m_prefatory): %i (%f)\n", (int)s, d_gb);
  tot_s += s;

  s = m_prefatory_size.size()*sizeof(int32_t);
  d_gb = (double)s/(1024.0*1024.0*1024.0);
  printf("# size(m_prefatory_size): %i (%f)\n", (int)s, d_gb);
  tot_s += s;

  s = m_entropy.size()*sizeof(double);
  d_gb = (double)s/(1024.0*1024.0*1024.0);
  printf("# size(m_entropy): %i (%f)\n", (int)s, d_gb);
  tot_s += s;

  s = m_block_entropy.size()*sizeof(double);
  d_gb = (double)s/(1024.0*1024.0*1024.0);
  printf("# size(m_block_entropy): %i (%f)\n", (int)s, d_gb);
  tot_s += s;

  s = m_distance_modifier.size()*sizeof(double);
  d_gb = (double)s/(1024.0*1024.0*1024.0);
  printf("# size(m_distance_modifier): %i (%f)\n", (int)s, d_gb);
  tot_s += s;

  d_gb = (double)tot_s/(1024.0*1024.0*1024.0);
  printf("# tot_s: %lli (%f)\n", (long long int)tot_s, d_gb);

}

void POMS::printDebugFStat(void) {
  int32_t src_tile, dst_tile, n_tile;
  int32_t idir,
          mod_print=4;
  int32_t t, n_v_tile, n_v_idx;

  for (src_tile=0; src_tile<m_tile_count; src_tile++) {

    if ((src_tile%mod_print)==0) { printf("\n"); }

    printf("# %4i:{", src_tile);

    for (idir=0; idir<6; idir++) {
      n_tile = 0;
      for (dst_tile=0; dst_tile<m_tile_count; dst_tile++) {
        if (F(src_tile, dst_tile, idir) > m_zero) { n_tile++; }
      }

      if (idir>0) { printf(","); }
      printf("%4i%s", n_tile, (n_tile == tileAdjIdxSize(src_tile, idir)) ? "" : "!!!!");

      if (n_tile != tileAdjIdxSize(src_tile, idir)) {

        n_v_tile = tileAdjIdxSize(src_tile, idir);
        for (n_v_idx=0; n_v_idx<n_v_tile; n_v_idx++) {
          t = tileAdjIdx(src_tile, idir, n_v_idx);
          if (F(src_tile, t, idir) < m_zero) {
            printf("# xxx tile:%i has tileAdjIdx neighbor %i (idir:%i) but no F value (%i)\n",
                src_tile, t, idir, (int)F(src_tile, t, idir));
          }
        }

      }


    }
    printf("}");
  }
  printf("\n");
}

void POMS::printDebugAC4(void) {
  int64_t cell,
          pos;
  int32_t x,y,z,
          tile_n,
          tile_idx,
          tile,
          idir;
  int w = 2;


  printf("AC4 tile_support[x:%i,y:%i,z:%i](cell:%i): ...[tile_n]\n",
      (int)m_size[0], (int)m_size[1], (int)m_size[2],
      (int)m_cell_count);

  for (z=0; z<m_size[2]; z++) {
    for (y=0; y<m_size[1]; y++) {
      for (x=0; x<m_size[0]; x++) {
        cell = xyz2cell(x,y,z);

        tile_n = cellSize(m_plane, cell);
        printf("\n[x:%3i,y:%3i,z:%2i](cell:%4i): [tile_n:%2i]:",
            (int)x, (int)y, (int)z, (int)cell,
            (int)tile_n);

        for (tile_idx=0; tile_idx<tile_n; tile_idx++) {
          if ((tile_idx % w) == 0) { printf("\n    "); }

          tile = cellTile(m_plane, cell, tile_idx);

          printf(" %3i{", (int)tile);
          for (idir=0; idir<6; idir++) {
            pos = (idir*m_cell_count*m_tile_count) + (cell*m_tile_count) + tile;
            if (idir > 0) { printf(", "); }
            printf("%s:%2i",
                m_dir_desc[idir].c_str(),
                (int)tileSupport(m_plane, idir, cell, tile));
                //m_tile_support[m_plane][pos]);
          }
          printf("}");


        }
        printf("\n");

      }
    }
  }

  printf("\n----\n");

}

void POMS::printDebugCellTileQueue(void) {
  int64_t cell,
          pos,
          idx;

  int32_t x,y,z,
          tile_n,
          tile_idx,
          tile,
          idir;
  int w = 2;

  printf("m_cell_tile_queue[%i]:\n", (int)m_cell_tile_queue_size[m_plane]);

  for (idx=0; idx<m_cell_tile_queue_size[m_plane]; idx+=2) {
    printf("[%i] cell:%i, tile:%i\n",
        (int)idx,
        (int)m_cell_tile_queue[m_plane][idx+0],
        (int)m_cell_tile_queue[m_plane][idx+1]);
  }

}

void POMS::printDebugCellEntropy(void) {
  int64_t cell,
          pos,
          idx;

  int32_t x,y,z,
          tile_n,
          tile_idx,
          tile,
          idir;
  int w = 2;
  int32_t n_b[3];

  int32_t mx, my, mz;
  double *M;

  n_b[0] = m_size[0];
  n_b[1] = m_size[1];
  n_b[2] = m_size[2];

  mx = m_size[0];
  my = m_size[1];
  mz = m_size[2];

  M = &(m_entropy[0]);

  printf("cell entropy n_b[%i,%i,%i]\n",
      (int)n_b[0], (int)n_b[1], (int)n_b[2]);


  for (z=0; z<n_b[2]; z++) {
    printf("#z:%i,y:.,x:.\n", (int)z);
    for (y=0; y<n_b[1]; y++) {
      for (x=0; x<n_b[0]; x++) {
        printf(" %4.2f", M[ (z*mx*my) + (y*mx) + x ]);
      }
      printf("\n");
    }
    printf("\n");
  }

}

void POMS::printDebugTileSizeBlock(int32_t block[][2]) {
  int32_t ix,iy,iz;
  int64_t cell;

  printf("debugTileSizeBlock:\n");
  for (iz=block[2][0]; iz<block[2][1]; iz++) {
    for (iy=block[1][0]; iy<block[1][1]; iy++) {
      for (ix=block[0][0]; ix<block[0][1]; ix++) {
        if ((iz<0) || (iz>=m_size[2])) { printf("  ."); continue; }
        if ((ix<0) || (ix>=m_size[0])) { printf("  ."); continue; }
        if ((iy<0) || (iy>=m_size[1])) { printf("  ."); continue; }
        cell = xyz2cell(ix,iy,iz);
        printf(" %2i", (int)cellSize(m_plane, cell));
      }
      printf("\n");
    }
    printf("\n");
  }
  printf("\n");
}

void POMS::printDebugBlockEntropy(void) {
  int64_t cell,
          pos,
          idx;

  int32_t x,y,z,
          tile_n,
          tile_idx,
          tile,
          idir;
  int w = 2;
  int32_t n_b[3];

  int32_t nx, ny, nz;
  double *B;

  n_b[0] = m_size[0] - m_block_size[0] + 1;
  n_b[1] = m_size[1] - m_block_size[1] + 1;
  n_b[2] = m_size[2] - m_block_size[2] + 1;

  nx = m_size[0];
  ny = m_size[1];
  nz = m_size[2];

  B = &(m_block_entropy[0]);

  printf("block entropy n_b[%i,%i,%i]\n",
      (int)n_b[0], (int)n_b[1], (int)n_b[2]);


  for (z=0; z<n_b[2]; z++) {
    printf("#z:%i,y:.,x:.\n", (int)z);
    for (y=0; y<n_b[1]; y++) {
      for (x=0; x<n_b[0]; x++) {
        printf(" %4.2f", B[ (z*nx*ny) + (y*nx) + x ]);
      }
      printf("\n");
    }
    printf("\n");
  }

}

//---
//

int64_t POMS::countClusters(void) {
  static int64_t _cell_count=-1;
  static std::vector< int64_t >   grid,
                                  cell_queue,
                                  boundary_queue;
  static std::vector< int8_t >    visited;
  int64_t nei_cell,
          cluster_count=0,
          cur_cluster = 0,
          cell=0,
          cell_queue_size=0,
          boundary_queue_size=0;
  int64_t nei_id=-1;
  int idir;

  int32_t x,y,z;
  int32_t v[3];

  if (_cell_count != m_cell_count) {
    grid.resize( m_cell_count, -1 );
    cell_queue.resize( m_cell_count );
    visited.resize( m_cell_count, 0 );
    boundary_queue.resize( 6*m_cell_count );
    _cell_count = m_cell_count;
  }

  cell_queue_size=0;
  boundary_queue_size = 0;
  cur_cluster = 0;
  cluster_count = 0;

  // queue all relevant cells for processing
  //
  for (cell=0; cell<m_cell_count; cell++) {
    grid[cell] = -1;
    visited[cell] = 0;
    if (cellSize(m_plane, cell) <= 1) { continue; }
    cell_queue[ cell_queue_size ] = cell;
    cell_queue_size++;
  }

  while (cell_queue_size > 0) {

    cell_queue_size--;
    cell = cell_queue[cell_queue_size];

    //DEBUG
    /*
    cell2vec(v, cell);
    printf("considering cell:%i(%i,%i,%i), visited[%i]:%i, cluster:%i\n",
        (int)cell,
        (int)v[0], (int)v[1], (int)v[2],
        (int)cell, visited[cell], (int)cluster_count);
        */

    if (visited[cell]) { continue; }

    // sanity error
    nei_id = -1;
    for (idir=0; idir<6; idir++) {
      nei_cell = neiCell( cell, idir );
      if (nei_cell < 0) { continue; }
      if (visited[nei_cell]) { continue; }
      if (grid[nei_cell] >= 0) {
        nei_id = grid[nei_cell];
        break;
      }
    }
    if (nei_id >= 0) {
      //DEBUG
      //printf("sanity error, should not have neighbor with non negative id cell:%i, nei_id:%i\n", (int)cell, (int)nei_id);
      return -1;
    }

    boundary_queue[0] = cell;
    boundary_queue_size = 1;

    while (boundary_queue_size > 0) {

      boundary_queue_size--;
      cell = boundary_queue[ boundary_queue_size ];
      if (visited[cell]) { continue; }

      grid[cell] = cluster_count;
      visited[cell] = 1;

      //DEBUG
      /*
      cell2vec(v, cell);
      printf("  grid[%i](%i,%i,%i) = cluster:%i\n",
          (int)cell,
          (int)v[0], (int)v[1], (int)v[2],
          (int)grid[cell]);
          */

      for (idir=0; idir<6; idir++) {
        nei_cell = neiCell( cell, idir );
        if (nei_cell < 0) { continue; }
        if (visited[nei_cell]) { continue; }
        if (cellSize(m_plane, nei_cell) <= 1) { continue; }
        boundary_queue[boundary_queue_size] = nei_cell;
        boundary_queue_size++;
      }

    }

    cluster_count++;

  }

  //DEBUG
  //printf("countClusters\n");
  //fgetc(stdin);


  return cluster_count;
}

//----

void POMS::printDebugCellFilter() {
  int64_t cell;
  int32_t x,y,z;

  for (z=0; z<m_size[2]; z++) {
    for (y=0; y<m_size[1]; y++) {
      for (x=0; x<m_size[0]; x++) {
        cell = xyz2cell(x,y,z);
        if (m_cell_filter[cell] == 0) { printf(" ."); }
        else { printf(" %i", (int)m_cell_filter[cell]); }
      }
      printf("\n");
    }
    printf("\n");
  }
  printf("\n");
}


//----

void POMS::printDebugDistanceModifier() {
  int64_t cell;
  int32_t x,y,z;
  int print_full=1;

  for (z=0; z<m_size[2]; z++) {
    for (y=0; y<m_size[1]; y++) {

      for (x=0; x<m_size[0]; x++) {
        cell = xyz2cell(x,y,z);

        if (print_full) {
          printf(" %f", m_distance_modifier[cell]);
        }
        else {
          if (m_distance_modifier[cell] < m_zero) { printf("    .  "); continue; }
          if (m_distance_modifier[cell] < 10.0) { printf(" "); }
          if (m_distance_modifier[cell] < 100.0) { printf(" "); }
          printf(" %0.2f", m_distance_modifier[cell]);
        }

      }
      printf("\n");
    }
    printf("\n");
  }
  printf("\n");
}

void POMS::printDebugQuilt() {
  int32_t x,y,z;
  int64_t cell;

  for (z=0; z<m_quilt_size[2]; z++) {

    for (x=0; x<m_quilt_size[0]; x++) { printf(" %3i", x); }
    printf("\n");

    for (y=0; y<m_quilt_size[1]; y++) {

      for (x=0; x<m_quilt_size[0]; x++) {
        cell = xyz2cell(x,y,z,m_quilt_size);
        if (m_quilt_tile[cell] < 0) { printf("   ."); }
        else { printf(" %3i", m_quilt_tile[cell]); }
      }
      printf("\n");
    }
    printf("\n");
  }
  printf("\n");
}

void POMS::printDebugPin() {
  int32_t x,y,z;
  int64_t cell;

  for (z=0; z<m_size[2]; z++) {


    printf("    ");
    for (x=0; x<m_size[0]; x++) { printf(" %2i", x); }
    printf("\n");

    for (y=0; y<m_size[1]; y++) {

      printf("%2i |", y);

      for (x=0; x<m_size[0]; x++) {
        cell = xyz2cell(x,y,z);
        printf("  %c", (m_cell_pin[cell] > 0) ? '1' : '.');
      }
      printf("\n");
    }
    printf("\n");
  }
  printf("\n");

}

/*
void POMS::printDebugCellUID() {
  int32_t x,y,z;
  int64_t cell;

  for (z=0; z<m_size[2]; z++) {

    //printf("    ");
    //for (x=0; x<m_size[0]; x++) { printf(" %2i", x); }
    //printf("\n");

    for (y=0; y<m_size[1]; y++) {

     // printf("%2i |", y);

      for (x=0; x<m_size[0]; x++) {
        cell = xyz2cell(x,y,z);
        printf(" %2i", (int)m_cell_uid[cell]);
      }
      printf("\n");
    }
    printf("\n");
  }
  printf("\n");

}
*/

void POMS::printDebugSpotCheck() {
  int32_t x, y, z, qx, qy, qz;
  int32_t cell, qcell;

  int64_t spot_check_sum;

  int32_t plane;
  int32_t tile, tile_idx, idir;


  // m_quilt_tile
  // m_quilt_pin


  plane = m_plane;

  //---

  printf("# spot_check: m_cell_queue_size[0,1]: %lli %lli\n",
      (long long int)m_cell_queue_size[0], (long long int)m_cell_queue_size[1]);

  printf("# spot_check: m_cell_tile_queue_size: %lli\n",
      (long long int)m_cell_tile_queue_size[0]);

  spot_check_sum=0;
  for (cell=0; cell<m_cell_count; cell++) {
    spot_check_sum += m_visited[plane][cell];
  }
  printf("# spot_check_sum: m_visited[%i]: %lli\n", plane, (long long int)spot_check_sum);

  spot_check_sum=0;
  for (cell=0; cell<m_cell_count; cell++) {
    for (tile_idx=0; tile_idx < m_tile_count; tile_idx++) {
      spot_check_sum += cellTileVisited(plane, cell, tile_idx);
    }
  }
  printf("# spot_check_sum: m_cell_tile_visited[%i]: %lli\n", plane, (long long int)spot_check_sum);

  spot_check_sum=0;
  for (cell=0; cell<m_cell_count; cell++) {
    spot_check_sum += m_ac4_dirty[plane][cell];
  }
  printf("# spot_check_sum: m_ac4_dirty[%i]: %lli\n", plane, (long long int)spot_check_sum);

  spot_check_sum=0;
  for (cell=0; cell<m_cell_count; cell++) {
    spot_check_sum += m_cell_pin[cell];
  }
  printf("# spot_check_sum: m_cell_pin: %lli\n", (long long int)spot_check_sum);


  //---

  spot_check_sum=0;
  for (cell=0; cell<m_quilt_cell_count; cell++) {
    spot_check_sum += m_quilt_tile[cell];
  }
  printf("# spot_check_sum: m_quilt_tile: %lli\n", (long long int)spot_check_sum);

  spot_check_sum=0;
  for (cell=0; cell<m_quilt_cell_count; cell++) {
    spot_check_sum += m_quilt_pin[cell];
  }
  printf("# spot_check_sum: m_quilt_pin: %lli\n", (long long int)spot_check_sum);

  //---

  spot_check_sum=0;
  for (cell=0; cell<m_cell_count; cell++) {
    spot_check_sum += m_tile_size[plane][cell];
  }
  printf("# spot_check_sum: m_tile_size[%i]: %lli\n", plane, (long long int)spot_check_sum);

  spot_check_sum=0;
  for (cell=0; cell<m_cell_count; cell++) {
    for (tile_idx=0; tile_idx<m_tile_size[plane][cell]; tile_idx++) {
      spot_check_sum += m_tile[plane][cell];
    }
  }
  printf("# spot_check_sum: m_tile[%i]: %lli\n", plane, (long long int)spot_check_sum);

  spot_check_sum=0;
  for (cell=0; cell<m_cell_count; cell++) {
    for (tile_idx=0; tile_idx<m_tile_size[plane][cell]; tile_idx++) {
      spot_check_sum += m_tile_bp[plane][cell];
    }
  }
  printf("# spot_check_sum: m_tile_bp[%i]: %lli\n", plane, (long long int)spot_check_sum);

  //---

  spot_check_sum=0;
  for (cell=0; cell<m_cell_count; cell++) {
    spot_check_sum += m_prefatory_size[cell];
  }
  printf("# spot_check_sum: m_prefatory_size: %lli\n", (long long int)spot_check_sum);

  spot_check_sum=0;
  for (cell=0; cell<m_cell_count; cell++) {
    for (tile_idx=0; tile_idx<m_prefatory_size[cell]; tile_idx++) {
      spot_check_sum += m_prefatory[cell];
    }
  }
  printf("# spot_check_sum: m_prefatory: %lli\n", (long long int)spot_check_sum);

  //---

  spot_check_sum=0;
  for (cell=0; cell<m_cell_count; cell++) {
    for (tile_idx=0; tile_idx<m_tile_size[plane][cell]; tile_idx++) {
      tile = cellTile(plane, cell, tile_idx);
      for (idir=0; idir<6; idir++) {
        spot_check_sum += tileSupport(plane, idir, cell, tile);
      }
    }
  }
  printf("# spot_check_sum: tileSupport[%i]: %lli\n", plane, (long long int)spot_check_sum);



}

//----
//

// bagging on this for now
// We're still suspicious of the AC4 initialization.
// There are some noloz configurationst hat look to be fialing and I'm not sure
// if it's just because it's so overconstrainted or whether there's a bug.
// To check, we can export state of the grid and then load it up to try and figure
// out what's going on but this is a heavy weight operation as we need to dump a lot
// of informatio and then have a system for bringing it back in.
// The constraints could theoretically hold this but we're going to need N*M
// constraints which is sure to bog things down.
//
//int POMS::debugExportBlock(char *fn) { }
