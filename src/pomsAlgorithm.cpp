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
int POMS::renew(void) {
  int r;
  int64_t sz,
          cell;
  int32_t xyz,
          tile_idx,
          tile,
          idir;

  int32_t plane,
          idx,
          n_nei,
          max_nei;

  std::vector< int32_t > tile_max_support[6];

  m_tile[0].clear();
  m_tile[1].clear();

  m_tile_size[0].clear();
  m_tile_size[1].clear();

  //EXPERIMENTAL
  //

  m_tile_support[0].clear();
  m_tile_support[1].clear();

  m_ac4_flat[0].clear();
  m_ac4_flat[1].clear();

  m_ac4_tier4[0].clear();
  m_ac4_tier4[1].clear();

  m_ac4_tier4_m1[0].clear();
  m_ac4_tier4_m1[1].clear();

  m_ac4_tier4_m2[0].clear();
  m_ac4_tier4_m2[1].clear();

  m_ac4_tier6[0].clear();
  m_ac4_tier6[1].clear();

  m_ac4_tier6_m1[0].clear();
  m_ac4_tier6_m1[1].clear();

  if (m_tile_support_option == POMS_OPTIMIZATION_AC4_NONE) {
    //m_tile_support[0].clear();
    //m_tile_support[1].clear();
  }
  else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_FLAT) {
    m_ac4_class_p[0] = &(m_ac4_flat[0]);
    m_ac4_class_p[1] = &(m_ac4_flat[1]);
  }

  else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER4) {
    m_ac4_class_p[0] = &(m_ac4_tier4[0]);
    m_ac4_class_p[1] = &(m_ac4_tier4[1]);
  }
  else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER4_M) {
    m_ac4_class_p[0] = &(m_ac4_tier4_m1[0]);
    m_ac4_class_p[1] = &(m_ac4_tier4_m1[1]);
  }
  else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER4_M2) {
    m_ac4_class_p[0] = &(m_ac4_tier4_m2[0]);
    m_ac4_class_p[1] = &(m_ac4_tier4_m2[1]);
  }

  else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER6) {
    m_ac4_class_p[0] = &(m_ac4_tier6[0]);
    m_ac4_class_p[1] = &(m_ac4_tier6[1]);
  }
  else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER6_M) {
    m_ac4_class_p[0] = &(m_ac4_tier6_m1[0]);
    m_ac4_class_p[1] = &(m_ac4_tier6_m1[1]);
  }

  //
  //EXPERIMENTAL



  if ((m_size[0] <= 0) ||
      (m_size[1] <= 0) ||
      (m_size[2] <= 0)) { return -1; }

  if (m_quilt_size[0] < 0) { m_quilt_size[0] = m_size[0]; }
  if (m_quilt_size[1] < 0) { m_quilt_size[1] = m_size[1]; }
  if (m_quilt_size[2] < 0) { m_quilt_size[2] = m_size[2]; }

  m_quilt_cell_count = m_quilt_size[0]*m_quilt_size[1]*m_quilt_size[2];

  m_extent[0] = m_size[0];
  m_extent[1] = m_size[1];
  m_extent[2] = m_size[2];

  m_dir_cell_incr[0] = 1;
  m_dir_cell_incr[1] = m_size[0];
  m_dir_cell_incr[2] = m_size[0]*m_size[1];

  m_plane = 0;

  m_cell_count = m_size[0] * m_size[1] * m_size[2];
  sz = m_cell_count * m_tile_count;

  m_tile[0].resize(sz);
  m_tile[1].resize(sz);

  m_tile_bp[0].resize(sz);
  m_tile_bp[1].resize(sz);

  m_tile_size[0].resize(m_cell_count);
  m_tile_size[1].resize(m_cell_count);

  //EXPERIMENTAL
  //
  //m_tile_support[0].resize(6*sz, -1);
  //m_tile_support[1].resize(6*sz, -1);

  if (m_tile_support_option == POMS_OPTIMIZATION_AC4_NONE) {
    m_tile_support[0].resize(6*sz, -1);
    m_tile_support[1].resize(6*sz, -1);
  }
  else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_FLAT) {
    r = m_ac4_flat[0].init(m_cell_count, m_tile_count);
    if (r<0) { return r; }
    r = m_ac4_flat[1].init(m_cell_count, m_tile_count);
    if (r<0) { return r; }

    m_ac4_class_p[0] = &(m_ac4_flat[0]);
    m_ac4_class_p[1] = &(m_ac4_flat[1]);
    m_tile_support_cb = _tile_support_flat;
  }

  else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER4) {

    r = m_ac4_tier4[0].pre_init(m_cell_count, m_tile_count);
    if (r<0) { return r; }
    r = m_ac4_tier4[1].pre_init(m_cell_count, m_tile_count);
    if (r<0) { return r; }

    for (tile=0; tile<m_tile_count; tile++) {
      max_nei = -1;
      for (idir=0; idir<6; idir++) {
        n_nei = tileAdjIdxSize(tile, idir);
        if (n_nei > max_nei) { max_nei = n_nei; }
      }

      for (plane=0; plane<2; plane++) {
        if      (max_nei < 2)     { m_ac4_tier4[plane].init_tile_vec(tile, 1); }
        else if (max_nei < 256)   { m_ac4_tier4[plane].init_tile_vec(tile, 8); }
        else if (max_nei < 65536) { m_ac4_tier4[plane].init_tile_vec(tile, 16); }
        else                      { m_ac4_tier4[plane].init_tile_vec(tile, 64); }
      }
    }

    m_ac4_class_p[0] = &(m_ac4_tier4[0]);
    m_ac4_class_p[1] = &(m_ac4_tier4[1]);
    m_tile_support_cb = _tile_support_tier4;
  }

  else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER4_M) {

    r = m_ac4_tier4_m1[0].pre_init(m_cell_count, m_tile_count);
    if (r<0) { return r; }
    r = m_ac4_tier4_m1[1].pre_init(m_cell_count, m_tile_count);
    if (r<0) { return r; }

    for (tile=0; tile<m_tile_count; tile++) {
      max_nei = -1;
      for (idir=0; idir<6; idir++) {
        n_nei = tileAdjIdxSize(tile, idir);
        if (n_nei > max_nei) { max_nei = n_nei; }
      }

      for (plane=0; plane<2; plane++) {
        if      (max_nei < 2)     { m_ac4_tier4_m1[plane].init_tile_vec(tile, 1); }
        else if (max_nei < 256)   { m_ac4_tier4_m1[plane].init_tile_vec(tile, 8); }
        else if (max_nei < 65536) { m_ac4_tier4_m1[plane].init_tile_vec(tile, 16); }
        else                      { m_ac4_tier4_m1[plane].init_tile_vec(tile, 64); }
      }
    }

    m_ac4_class_p[0] = &(m_ac4_tier4_m1[0]);
    m_ac4_class_p[1] = &(m_ac4_tier4_m1[1]);
    m_tile_support_cb = _tile_support_tier4_m1;
  }

  else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER4_M2) {

    r = m_ac4_tier4_m2[0].pre_init(m_cell_count, m_tile_count);
    if (r<0) { return r; }
    r = m_ac4_tier4_m2[1].pre_init(m_cell_count, m_tile_count);
    if (r<0) { return r; }

    for (tile=0; tile<m_tile_count; tile++) {
      max_nei = -1;
      for (idir=0; idir<6; idir++) {
        n_nei = tileAdjIdxSize(tile, idir);
        if (n_nei > max_nei) { max_nei = n_nei; }
      }

      for (plane=0; plane<2; plane++) {
        if      (max_nei < 2)     { m_ac4_tier4_m2[plane].init_tile_vec(tile, 1); }
        else if (max_nei < 256)   { m_ac4_tier4_m2[plane].init_tile_vec(tile, 8); }
        else if (max_nei < 65536) { m_ac4_tier4_m2[plane].init_tile_vec(tile, 16); }
        else                      { m_ac4_tier4_m2[plane].init_tile_vec(tile, 64); }
      }
    }

    m_ac4_class_p[0] = &(m_ac4_tier4_m2[0]);
    m_ac4_class_p[1] = &(m_ac4_tier4_m2[1]);
    m_tile_support_cb = _tile_support_tier4_m2;
  }

  else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER6) {
    r = m_ac4_tier6[0].pre_init(m_cell_count, m_tile_count);
    if (r<0) { return r; }
    r = m_ac4_tier6[1].pre_init(m_cell_count, m_tile_count);
    if (r<0) { return r; }

    for (tile=0; tile<m_tile_count; tile++) {
      max_nei = -1;
      for (idir=0; idir<6; idir++) {
        n_nei = tileAdjIdxSize(tile, idir);
        if (n_nei > max_nei) { max_nei = n_nei; }
      }

      for (plane=0; plane<2; plane++) {
        if      (max_nei < 2)     { m_ac4_tier6[plane].init_tile_vec(tile, 1); }
        else if (max_nei < 4)     { m_ac4_tier6[plane].init_tile_vec(tile, 2); }
        else if (max_nei < 16)    { m_ac4_tier6[plane].init_tile_vec(tile, 4); }
        else if (max_nei < 256)   { m_ac4_tier6[plane].init_tile_vec(tile, 8); }
        else if (max_nei < 65536) { m_ac4_tier6[plane].init_tile_vec(tile, 16); }
        else                      { m_ac4_tier6[plane].init_tile_vec(tile, 64); }
      }

    }

    m_ac4_class_p[0] = &(m_ac4_tier6[0]);
    m_ac4_class_p[1] = &(m_ac4_tier6[1]);
    m_tile_support_cb = _tile_support_tier6;
  }

  else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER6_M) {

    r = m_ac4_tier6_m1[0].pre_init(m_cell_count, m_tile_count);
    if (r<0) { return r; }
    r = m_ac4_tier6_m1[1].pre_init(m_cell_count, m_tile_count);
    if (r<0) { return r; }

    for (tile=0; tile<m_tile_count; tile++) {
      max_nei = -1;
      for (idir=0; idir<6; idir++) {
        n_nei = tileAdjIdxSize(tile, idir);
        if (n_nei > max_nei) { max_nei = n_nei; }
      }

      for (plane=0; plane<2; plane++) {
        if      (max_nei < 2)     { m_ac4_tier6_m1[plane].init_tile_vec(tile, 1); }
        else if (max_nei < 4)     { m_ac4_tier6_m1[plane].init_tile_vec(tile, 2); }
        else if (max_nei < 16)    { m_ac4_tier6_m1[plane].init_tile_vec(tile, 4); }
        else if (max_nei < 256)   { m_ac4_tier6_m1[plane].init_tile_vec(tile, 8); }
        else if (max_nei < 65536) { m_ac4_tier6_m1[plane].init_tile_vec(tile, 16); }
        else                      { m_ac4_tier6_m1[plane].init_tile_vec(tile, 64); }
      }

    }

    m_ac4_class_p[0] = &(m_ac4_tier6_m1[0]);
    m_ac4_class_p[1] = &(m_ac4_tier6_m1[1]);
    m_tile_support_cb = _tile_support_tier6_m1;
  }

  //
  //EXPERIMENTAL

  m_cell_pin.resize( m_cell_count, 0 );

  //m_cell_uid.resize( m_cell_count, 0 );
  //m_cell_uid_count = 0;
  //for (idir=0; idir<6; idir++) {
  //  m_ac4_memoize[idir].clear();
  //  m_ac4_memoize[idir].resize(m_tile_count, -1);
  //}
  //resetMemoize();

  //---
  //---

  for (xyz=0; xyz<3; xyz++) {
    if (m_patch_region[xyz][0] < 0) { m_patch_region[xyz][0] = 0; }
    if (m_patch_region[xyz][1] < 0) { m_patch_region[xyz][1] = m_size[xyz]; }
  }
  if (m_quilt_tile.size() != m_quilt_cell_count) {
    m_quilt_tile.resize( m_quilt_cell_count, -1 );
  }
  if (m_quilt_pin.size() != m_quilt_cell_count) {
    m_quilt_pin.resize( m_quilt_cell_count, 0 );
  }

  //---
  //---

  for (cell=0; cell<m_cell_count; cell++) {
    setCellSize( 0, cell, m_tile_count );
    setCellSize( 1, cell, m_tile_count );

    for (tile_idx=0; tile_idx<m_tile_count; tile_idx++) {
      tile = tile_idx;
      setCellTile( 0, cell, tile_idx, tile );
      setCellTile( 1, cell, tile_idx, tile );

      for (idir=0; idir<6; idir++) {
        tileSupport(0, idir, cell, tile_idx, 0);
        tileSupport(1, idir, cell, tile_idx, 0);
      }
    }
  }

  m_ac4_dirty[0].clear();
  m_ac4_dirty[1].clear();

  m_ac4_dirty[0].resize(m_cell_count, 0);
  m_ac4_dirty[1].resize(m_cell_count, 0);

  //---

  m_prefatory.clear();
  m_prefatory.resize( m_cell_count * m_tile_count );

  m_prefatory_size.clear();
  m_prefatory_size.resize( m_cell_count );

  m_entropy.clear();
  m_entropy.resize( m_cell_count );

  m_block_entropy.clear();
  m_block_entropy.resize( m_cell_count );

  m_distance_modifier.clear();
  m_distance_modifier.resize( m_cell_count );

  //---

  //memset(m_block_size,  0, sizeof(int32_t)*3);
  memset(m_block,       0, sizeof(int32_t)*3*2);

  //memset(m_soften_size, 0, sizeof(int32_t)*3);
  memset(m_soften_block,      0, sizeof(int32_t)*3*2);
  memset(m_soften_pos,        0, sizeof(int32_t)*3);

  //---

  m_visited[0].clear();
  m_visited[1].clear();

  m_visited[0].resize( m_cell_count, 0 );
  m_visited[1].resize( m_cell_count, 0 );

  //---

  m_cell_queue[0].clear();
  m_cell_queue[1].clear();

  m_cell_queue[0].resize( m_cell_count );
  m_cell_queue[1].resize( m_cell_count );

  m_cell_queue_size[0] = 0;
  m_cell_queue_size[1] = 0;

  //---

  m_cell_tile_queue[0].clear();
  //m_cell_tile_queue[1].clear();

  m_cell_tile_queue[0].resize( m_cell_count * m_tile_count * 2 );
  //m_cell_tile_queue[1].resize( m_cell_count * m_tile_count * 2 );

  m_cell_tile_queue_size[0] = 0;
  //m_cell_tile_queue_size[1] = 0;

  m_cell_tile_visited[0].clear();
  m_cell_tile_visited[1].clear();

  m_cell_tile_visited[0].resize( m_cell_count * m_tile_count );
  m_cell_tile_visited[1].resize( m_cell_count * m_tile_count );

  //---

  m_phase = POMS_PHASE_INIT;

  //---

  m_cell_filter.resize( m_cell_count );

  //---

  // get actual constraints from implicit constraints
  //
  refreshConstraints();

  return 0;
}

//---
//
int POMS::saveGrid(int save_plane, int save_full) {
  int r;
  int32_t idir;
  int64_t cell;
  int64_t process_count=0;
  //size_t _s;

  //_s = (size_t)m_tile_support_data_size;

  if (save_plane < 0) {
    save_plane = (m_plane + 1)%m_plane_count;
  }

  if (save_full != 0) {

    _prof_start(POMS_PROF_SAVEGRID);

    memcpy( &(m_tile[save_plane][0]),
            &(m_tile[   m_plane][0]),
            m_tile_data_size*m_tile_count*m_cell_count );

    memcpy( &(m_tile_bp[save_plane][0]),
            &(m_tile_bp[   m_plane][0]),
            m_tile_data_size*m_tile_count*m_cell_count );

    memcpy( &(m_tile_size[save_plane][0]),
            &(m_tile_size[   m_plane][0]),
            m_tile_data_size*m_cell_count );

    //memcpy( &(m_tile_support[save_plane][0]),
    //        &(m_tile_support[   m_plane][0]),
    //        m_tile_support_data_size*m_tile_count*m_cell_count*6 );
    if (m_tile_support_option == POMS_OPTIMIZATION_AC4_NONE) {
      memcpy( &(m_tile_support[save_plane][0]),
              &(m_tile_support[   m_plane][0]),
              m_tile_support_data_size*m_tile_count*m_cell_count*6 );
    }
    else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_FLAT) {
      r = m_ac4_flat[save_plane].set( &(m_ac4_flat[m_plane]) );
      if (r<0) { return r; }
    }

    else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER4) {
      r = m_ac4_tier4[save_plane].set( &(m_ac4_tier4[m_plane]) );
      if (r<0) { return r; }
    }
    else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER4_M) {
      r = m_ac4_tier4_m1[save_plane].set( &(m_ac4_tier4_m1[m_plane]) );
      if (r<0) { return r; }
    }
    else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER4_M2) {
      r = m_ac4_tier4_m2[save_plane].set( &(m_ac4_tier4_m2[m_plane]) );
      if (r<0) { return r; }
    }

    else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER6) {
      r = m_ac4_tier6[save_plane].set( &(m_ac4_tier6[m_plane]) );
      if (r<0) { return r; }
    }
    else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER6_M) {
      r = m_ac4_tier6_m1[save_plane].set( &(m_ac4_tier6_m1[m_plane]) );
      if (r<0) { return r; }
    }

    _prof_end(POMS_PROF_SAVEGRID);

    return 0;
  }

  _prof_start(POMS_PROF_SAVECELLS);

  for (cell=0; cell<m_cell_count; cell++) {


    if (!m_ac4_dirty[m_plane][cell]) { continue; }

    process_count++;

    memcpy( &(m_tile[save_plane][ cell*m_tile_count ]),
            &(m_tile[   m_plane][ cell*m_tile_count ]),
            m_tile_data_size*m_tile_count );

    memcpy( &(m_tile_bp[save_plane][ cell*m_tile_count ]),
            &(m_tile_bp[   m_plane][ cell*m_tile_count ]),
            m_tile_data_size*m_tile_count );

    m_tile_size[save_plane][cell] = m_tile_size[m_plane][cell];

    //for (idir=0; idir<6; idir++)  {
    //  memcpy( &(m_tile_support[save_plane][ (idir*m_cell_count*m_tile_count) + (cell*m_tile_count) ]),
    //          &(m_tile_support[   m_plane][ (idir*m_cell_count*m_tile_count) + (cell*m_tile_count) ]),
    //          m_tile_support_data_size*m_tile_count );
    //}

    if (m_tile_support_option == POMS_OPTIMIZATION_AC4_NONE) {
      for (idir=0; idir<6; idir++)  {
        memcpy( &(m_tile_support[save_plane][ (idir*m_cell_count*m_tile_count) + (cell*m_tile_count) ]),
                &(m_tile_support[   m_plane][ (idir*m_cell_count*m_tile_count) + (cell*m_tile_count) ]),
                m_tile_support_data_size*m_tile_count );
      }
    }
    else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_FLAT) {
      r = m_ac4_flat[save_plane].setCell( &(m_ac4_flat[m_plane]), cell );
      if (r<0) { return r; }
    }

    else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER4) {
      r = m_ac4_tier4[save_plane].setCell( &(m_ac4_tier4[m_plane]), cell );
      if (r<0) { return r; }
    }
    else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER4_M) {
      r = m_ac4_tier4_m1[save_plane].setCell( &(m_ac4_tier4_m1[m_plane]), cell );
      if (r<0) { return r; }
    }
    else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER4_M2) {
      r = m_ac4_tier4_m2[save_plane].setCell( &(m_ac4_tier4_m2[m_plane]), cell );
      if (r<0) { return r; }
    }

    else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER6) {
      r = m_ac4_tier6[save_plane].setCell( &(m_ac4_tier6[m_plane]), cell );
      if (r<0) { return r; }
    }
    else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER6_M) {
      r = m_ac4_tier6_m1[save_plane].setCell( &(m_ac4_tier6_m1[m_plane]), cell );
      if (r<0) { return r; }
    }

  }

  _prof_end(POMS_PROF_SAVECELLS);

  if (m_verbose >= POMS_VERBOSE_RUN) {
    printf("# saveGrid: ac4_dirty count: %i\n", (int)process_count);
  }

  return 0;
}

//---
//
int POMS::restoreGrid(int save_plane, int restore_full) {
  int r;
  int32_t idir;
  int64_t cell;
  int64_t process_count=0;

  if (save_plane < 0) {
    save_plane = (m_plane + 1)%m_plane_count;
  }

  if (restore_full != 0) {


    _prof_start(POMS_PROF_RESTOREGRID);

    memcpy( &(m_tile[   m_plane][0]),
            &(m_tile[save_plane][0]),
            m_tile_data_size*m_tile_count*m_cell_count );

    memcpy( &(m_tile_bp[   m_plane][0]),
            &(m_tile_bp[save_plane][0]),
            m_tile_data_size*m_tile_count*m_cell_count );

    memcpy( &(m_tile_size[   m_plane][0]),
            &(m_tile_size[save_plane][0]),
            m_tile_data_size*m_cell_count );

    //memcpy( &(m_tile_support[   m_plane][0]),
    //        &(m_tile_support[save_plane][0]),
    //        m_tile_support_data_size*m_tile_count*m_cell_count*6 );
    if (m_tile_support_option == POMS_OPTIMIZATION_AC4_NONE) {
      memcpy( &(m_tile_support[   m_plane][0]),
              &(m_tile_support[save_plane][0]),
              m_tile_support_data_size*m_tile_count*m_cell_count*6 );
    }
    else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_FLAT) {
      r = m_ac4_flat[m_plane].set( &(m_ac4_flat[save_plane]) );
      if (r<0) { return r; }
    }

    else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER4) {
      r = m_ac4_tier4[m_plane].set( &(m_ac4_tier4[save_plane]) );
      if (r<0) { return r; }
    }
    else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER4_M) {
      r = m_ac4_tier4_m1[m_plane].set( &(m_ac4_tier4_m1[save_plane]) );
      if (r<0) { return r; }
    }
    else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER4_M2) {
      r = m_ac4_tier4_m2[m_plane].set( &(m_ac4_tier4_m2[save_plane]) );
      if (r<0) { return r; }
    }

    else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER6) {
      r = m_ac4_tier6[m_plane].set( &(m_ac4_tier6[save_plane]) );
      if (r<0) { return r; }
    }
    else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER6_M) {
      r = m_ac4_tier6_m1[m_plane].set( &(m_ac4_tier6_m1[save_plane]) );
      if (r<0) { return r; }
    }

    _prof_end(POMS_PROF_RESTOREGRID);

    return 0;
  }

  _prof_start(POMS_PROF_RESTORECELLS);


  for (cell=0; cell<m_cell_count; cell++) {

    if (!m_ac4_dirty[m_plane][cell]) { continue; }

    process_count++;

    memcpy( &(m_tile[   m_plane][ cell*m_tile_count ]),
            &(m_tile[save_plane][ cell*m_tile_count ]),
            m_tile_data_size*m_tile_count );

    memcpy( &(m_tile_bp[   m_plane][ cell*m_tile_count ]),
            &(m_tile_bp[save_plane][ cell*m_tile_count ]),
            m_tile_data_size*m_tile_count );

    m_tile_size[m_plane][cell] = m_tile_size[save_plane][cell];

    //for (idir=0; idir<6; idir++)  {
    //  memcpy( &(m_tile_support[   m_plane][ (idir*m_cell_count*m_tile_count) + (cell*m_tile_count) ]),
    //          &(m_tile_support[save_plane][ (idir*m_cell_count*m_tile_count) + (cell*m_tile_count) ]),
    //          m_tile_support_data_size*m_tile_count );
    //}
    if (m_tile_support_option == POMS_OPTIMIZATION_AC4_NONE) {
      for (idir=0; idir<6; idir++)  {
        memcpy( &(m_tile_support[   m_plane][ (idir*m_cell_count*m_tile_count) + (cell*m_tile_count) ]),
                &(m_tile_support[save_plane][ (idir*m_cell_count*m_tile_count) + (cell*m_tile_count) ]),
                m_tile_support_data_size*m_tile_count );
      }
    }
    else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_FLAT) {
      r = m_ac4_flat[m_plane].setCell( &(m_ac4_flat[save_plane]), cell );
      if (r<0) { return r; }
    }

    else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER4) {
      r = m_ac4_tier4[m_plane].setCell( &(m_ac4_tier4[save_plane]), cell );
      if (r<0) { return r; }
    }
    else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER4_M) {
      r = m_ac4_tier4_m1[m_plane].setCell( &(m_ac4_tier4_m1[save_plane]), cell );
      if (r<0) { return r; }
    }
    else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER4_M2) {
      r = m_ac4_tier4_m2[m_plane].setCell( &(m_ac4_tier4_m2[save_plane]), cell );
      if (r<0) { return r; }
    }

    else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER6) {
      r = m_ac4_tier6[m_plane].setCell( &(m_ac4_tier6[save_plane]), cell );
      if (r<0) { return r; }
    }
    else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER6_M) {
      r = m_ac4_tier6_m1[m_plane].setCell( &(m_ac4_tier6_m1[save_plane]), cell );
      if (r<0) { return r; }
    }


  }

  _prof_end(POMS_PROF_RESTORECELLS);

  if (m_verbose >= POMS_VERBOSE_RUN) {
    printf("# restoreGrid: ac4_dirty count: %i\n", (int)process_count);
  }

  return 0;
}

//---
//
int64_t POMS::resolvedCount(void) {
  int64_t cell, count=0;
  int32_t tile_n;

  for (cell=0; cell<m_cell_count; cell++) {

    //EXPERIMENT
    if (m_cell_pin[cell] != 0) {
      count++;
      continue;
    }

    tile_n = cellSize(m_plane, cell);
    if (tile_n < 1) { return -1; }
    if (tile_n == 1) { count++; }
  }
  return count;
}


//---
//

int POMS::savePrefatory(void) {
  int64_t cell;
  int32_t tile_idx;

  memcpy( &(m_prefatory[0]),
          &(m_tile[m_plane][0]),
          m_tile_data_size*m_cell_count*m_tile_count );
  memcpy( &(m_prefatory_size[0]),
          &(m_tile_size[m_plane][0]),
          m_tile_data_size*m_cell_count );

  return 0;
}

//---
// restore block, stored in soften_block, to prefatory state.
//
// `wblock` holds the soften_block and extra boundary to account
// for the need to update the boundary tiles for AC4 updates.
// `wblock` can go past the grid boundary so this needs to be
// skipped over in the AC4InitBlock call, which should correctly
// handle these bounds checks.
//
int POMS::soften_ac4(int32_t soften_block[][2]) {
  int r;
  int64_t cell;
  int32_t tile_idx,
          tile,
          tile_n,
          x,y,z,
          xyz;
  int32_t wblock[3][2];

  for (xyz=0; xyz<3; xyz++) {
    wblock[xyz][0] = soften_block[xyz][0]-1;
    wblock[xyz][1] = soften_block[xyz][1]+1;

    if (wblock[xyz][0] < 0)           { wblock[xyz][0] = 0; }
    if (wblock[xyz][1] > m_size[xyz]) { wblock[xyz][1] = m_size[xyz]; }
  }

  for (z=soften_block[2][0]; z<soften_block[2][1]; z++) {
    for (y=soften_block[1][0]; y<soften_block[1][1]; y++) {
      for (x=soften_block[0][0]; x<soften_block[0][1]; x++) {

        cell = xyz2cell(x,y,z);

        //EXPERIMENT
        if (m_cell_pin[cell] != 0) { continue; }

        tile_n = cellBufSize( &(m_prefatory_size[0]), cell );
        setCellSize( m_plane, cell, tile_n );

        for (tile_idx=0; tile_idx<tile_n; tile_idx++) {
          tile = cellBufTile( &(m_prefatory[0]), cell, tile_idx );
          setCellTile( m_plane, cell, tile_idx, tile );
        }

        markAC4Dirty(m_plane, cell);

      }
    }
  }

  r = AC4InitBlock( wblock );
  return r;
}

//---
// restore block, stored in m_soften_block, to prefatory
// state
//
int POMS::soften(int32_t soften_block[][2]) { return soften_ac4(soften_block); }
int POMS::soften(void)                      { return soften(m_soften_block); }


//---
// __      _____ ___
// \ \    / / __/ __|
//  \ \/\/ /| _| (__
//   \_/\_/ |_| \___|
//

int POMS::WFCBlock(int32_t block[][2], int64_t wfc_step) { return WFCBlock_ac4(m_block, wfc_step); }
int POMS::WFCBlock(int64_t wfc_step) { return WFCBlock(m_block, wfc_step); }

int POMS::WFC(int64_t wfc_step) {
  int32_t block[3][2];

  block[0][0]=0; block[1][0]=0; block[2][0]=0;
  block[0][1]=m_size[0];
  block[1][1]=m_size[1];
  block[2][1]=m_size[2];

  return WFCBlock(block, wfc_step);
}


//--------------
//--------------
//--------------


//---
// Run WFC on `block` (AC4).
// Only cells within `block` will be fixed but constraints
//   will propagate outside of block.
//
// for input `wfc_step`:
//
//  >0 - only run for `wfc_step` steps before returning
// <=0 - run until conflict or block is fully resolved
//
// Return:
//
//  1 - block not fully resolved but no conflict has been encoutered
//  0 - every cell within `m_block` is resolved and the `(cell,tile)`
//      pairs touched are in an arc consistent state
// -1 - conflict was encountered
//
// On a conflict, the `m_conflict_*` variables will be populated
// accordingly.
//
// unoptimized
//
int POMS::_WFCBlock_ac4(int32_t block[][2], int64_t wfc_step) {
  int64_t cell=-1;
  int32_t x,y,z,
          tile_idx=-1,
          tile;
  double p, s=-1.0;
  int _r, ret;
  int64_t block_cell_count=0,
          realized_count=0;
  int64_t max_iter=-1,
          iter=0;

  int32_t _v[3],
          _idx,
          _tile,
          _tile_n;

  int32_t tile_n,
          rem_tile,
          rem_tile_idx;
  //int64_t qpos=0;

  int64_t nei_cell,
          nei_pos;
  int32_t nei_tile_n,
          nei_tile_idx,
          nei_tile;
  int32_t idir, rdir;

  int32_t _tile_support=0;

  int32_t __i32_0, __i32_1;
  int64_t __i64_0, __i64_1;


  ret = -5;

  max_iter = wfc_step;
  if (max_iter <= 0) {
    max_iter = m_size[0]*m_size[1]*m_size[2];
  }

  if (m_verbose >= POMS_VERBOSE_DEBUG) {
    printf("wfcBlock_ac4: wfc_step:%i, max_iter:%i block:[[%i,%i],[%i,%i],[%i,%i]]\n",
        (int)wfc_step, (int)max_iter,
        (int)block[0][0], (int)block[0][1],
        (int)block[1][0], (int)block[1][1],
        (int)block[2][0], (int)block[2][1]);
  }

  while (iter<max_iter) {

    _r = computeCellEntropyWithinBlock(block, 1);
    if (_r==1) { return 0; }
    if (_r <0) { return _r; }


    if (m_verbose >= POMS_VERBOSE_DEBUG) {
      printf("wfcBlock_ac4: computeEntropyWithinBlock got %i\n", (int)_r);
    }

    _r = pickCellMinEntropyWithinBlock( &cell, &tile_idx, &s, block );

    //DEBUG
    //if (m_cell_pin[cell] != 0) {
    //  printf("SANITY wfc ac4, picked pinned cell %i\n", (int)cell);
    //  return -1;
    //}


    // We really shouldn't get an error here as we should have already
    // figured out there was a contradiction. Kept in for paranoia.
    //
    if (_r <  0) { ret=-1; break; }
    if (_r == 1) { ret= 0; break; }

    // we assume a continuation. If a conflict occurs,
    // ret will be set appropriately below or we'll
    // return with the appropriate value.
    //
    if (_r==0) { ret=1; }

    if (m_verbose >= POMS_VERBOSE_DEBUG) {
      _tile_n = cellSize(m_plane, cell);
      _tile = cellTile(m_plane, cell, tile_idx);
      cell2vec(_v, cell);
      printf("wfcBlock_ac4: picking cell:%i[%i,%i,%i], tile_idx:%i/%i tile:%i(%s), e:%f"
         " (block:[[%i,%i],[%i,%i],[%i,%i]])\n",
         (int)cell, (int)_v[0], (int)_v[1], (int)_v[2],
         (int)tile_idx, (int)_tile_n,
         (int)_tile, m_tile_name[_tile].c_str(),
         (double)s,
         (int)block[0][0], (int)block[0][1],
         (int)block[1][0], (int)block[1][1],
         (int)block[2][0], (int)block[2][1]);

      _tile_n = -1;
    }

    // tile to keep (fix)
    //
    tile = cellTile( m_plane, cell, tile_idx );

    tile_n = cellSize( m_plane, cell );

    // for each tile we're removing, decrement the tile support
    // for neighboring (nei_cell,nei_tile) values that have arcs into it
    // from the current (cell,tile)
    //
    for (rem_tile_idx=0; rem_tile_idx<tile_n; rem_tile_idx++) {
      if (rem_tile_idx == tile_idx) { continue; }
      rem_tile = cellTile( m_plane, cell, rem_tile_idx );

      for (idir=0; idir<6; idir++) {
        nei_cell = neiCell( cell, idir );
        if (nei_cell < 0) { continue; }

        //EXPERIMENT
        if (m_cell_pin[nei_cell] != 0) { continue; }

        nei_tile_n = cellSize( m_plane, nei_cell );
        for ( nei_tile_idx=0; nei_tile_idx < nei_tile_n; nei_tile_idx++ ) {
          nei_tile = cellTile( m_plane, nei_cell, nei_tile_idx );

          rdir = m_dir_oppo[idir];
          if ( F(nei_tile, rem_tile, rdir) < m_zero ) { continue; }

          nei_pos = (rdir*m_cell_count*m_tile_count) + (nei_cell*m_tile_count) + nei_tile;
          _tile_support = tileSupport(m_plane, rdir, nei_cell, nei_tile);

          if (_tile_support == 0) {

            // sanity error
            //
            if (m_verbose >= POMS_VERBOSE_DEBUG) {
              cell2vec(_v, nei_cell);

              __i32_0 = tileSupport(m_plane, rdir, nei_cell, nei_tile);

              printf("SANITY ERROR: tile:%i@cell:%i(%i,%i,%i) has negative support (%i)\n",
                  (int)nei_tile, (int)nei_cell,
                  (int)_v[0], (int)_v[1], (int)_v[2],
                  (int)__i32_0);
              fprintf(stderr, "SANITY ERROR: tile:%i@cell:%i(%i,%i,%i) has negative support (%i)\n",
                  (int)nei_tile, (int)nei_cell,
                  (int)_v[0], (int)_v[1], (int)_v[2],
                  (int)__i32_0);
            }
            return -2;
          }

          _tile_support--;
          tileSupport(m_plane, rdir, nei_cell, nei_tile, _tile_support);
          markAC4Dirty(m_plane, nei_cell);

          // support has gone to 0, enqueue
          //
          if (tileSupport(m_plane, rdir, nei_cell, nei_tile) == 0) {

            if (cellTileVisited(m_plane, nei_cell, nei_tile) == 0) {
              cellTileQueuePush(m_plane, nei_cell, nei_tile);
              cellTileVisited(m_plane, nei_cell, nei_tile, 1);
            }

          }

        }

      }


    }

    // once we've updated tile support, remove all tiles
    // and keep the one we're resolving
    //
    forceTile( m_plane, cell, tile );
    markAC4Dirty(m_plane, cell);

    _r = AC4Update();
    if (_r<0) {
      ret=-1;
      break;
    }

    iter++;
  }

  if (ret < 0) {return ret; }

  if (iter >= max_iter) {
    block_cell_count =
      (block[0][1] - block[0][0]) *
      (block[1][1] - block[1][0]) *
      (block[2][1] - block[2][0]);

    realized_count = realizedCellsWithinBlock(block);

    ret = 1;
    if (block_cell_count == realized_count) { ret = 0; }

    if (m_verbose >= POMS_VERBOSE_DEBUG) {
      printf("WFCBlock_ac4: ret:%i, iter:%i/%i, block_cell_count:%i, realized_count:%i\n",
          (int)ret,
          (int)iter, (int)max_iter, (int)block_cell_count, (int)realized_count);
    }

  }

  return ret;
}

//---
// Run WFC on `block` (AC4).
// Only cells within `block` will be fixed but constraints
//   will propagate outside of block.
//
// for input `wfc_step`:
//
//  >0 - only run for `wfc_step` steps before returning
// <=0 - run until conflict or block is fully resolved
//
// Return:
//
//  1 - block not fully resolved but no conflict has been encoutered
//  0 - every cell within `m_block` is resolved and the `(cell,tile)`
//      pairs touched are in an arc consistent state
// -1 - conflict was encountered
//
// On a conflict, the `m_conflict_*` variables will be populated
// accordingly.
//
// OPTIMIZED
// optimization looks to be minor (maybe 10-20%) but considering
// wfc is run repeatedly, seems a shame not to do it.
// Same optimization as the AC4Update and AC4Init by taking
// the minimum of tiles left in cell vs. size of neighboring tile
// rule list.
//
int POMS::WFCBlock_ac4(int32_t block[][2], int64_t wfc_step) {
  int64_t cell=-1;
  int32_t x,y,z,
          tile_idx=-1,
          tile;
  double p, s=-1.0;
  int _r, ret;
  int64_t block_cell_count=0,
          realized_count=0;
  int64_t max_iter=-1,
          iter=0;

  int32_t _v[3],
          _idx,
          _tile,
          _tile_n;

  int32_t tile_n,
          nei_v_n,
          nei_v_idx,
          rem_tile,
          rem_tile_idx;
  //int64_t qpos=0;

  int64_t nei_cell,
          nei_pos;
  int32_t nei_tile_n,
          nei_tile_idx,
          nei_tile;
  int32_t idir, rdir;

  int32_t _tile_support=0;

  int32_t __i32_0, __i32_1;
  int64_t __i64_0, __i64_1;


  ret = -5;

  max_iter = wfc_step;
  if (max_iter <= 0) {
    max_iter = m_size[0]*m_size[1]*m_size[2];
  }

  if (m_verbose >= POMS_VERBOSE_DEBUG) {
    printf("wfcBlock_ac4: wfc_step:%i, max_iter:%i block:[[%i,%i],[%i,%i],[%i,%i]]\n",
        (int)wfc_step, (int)max_iter,
        (int)block[0][0], (int)block[0][1],
        (int)block[1][0], (int)block[1][1],
        (int)block[2][0], (int)block[2][1]);
  }

  while (iter<max_iter) {

    _r = computeCellEntropyWithinBlock(block, 1);
    if (_r==1) { return 0; }
    if (_r <0) { return _r; }


    if (m_verbose >= POMS_VERBOSE_DEBUG) {
      printf("wfcBlock_ac4: computeEntropyWithinBlock got %i\n", (int)_r);
    }

    _r = pickCellMinEntropyWithinBlock( &cell, &tile_idx, &s, block );

    //DEBUG
    //if (m_cell_pin[cell] != 0) {
    //  printf("SANITY wfc ac4, picked pinned cell %i\n", (int)cell);
    //  return -1;
    //}


    // We really shouldn't get an error here as we should have already
    // figured out there was a contradiction. Kept in for paranoia.
    //
    if (_r <  0) { ret=-1; break; }
    if (_r == 1) { ret= 0; break; }

    // we assume a continuation. If a conflict occurs,
    // ret will be set appropriately below or we'll
    // return with the appropriate value.
    //
    if (_r==0) { ret=1; }

    if (m_verbose >= POMS_VERBOSE_DEBUG) {
      _tile_n = cellSize(m_plane, cell);
      _tile = cellTile(m_plane, cell, tile_idx);
      cell2vec(_v, cell);
      printf("wfcBlock_ac4: picking cell:%i[%i,%i,%i], tile_idx:%i/%i tile:%i(%s), e:%f"
         " (block:[[%i,%i],[%i,%i],[%i,%i]])\n",
         (int)cell, (int)_v[0], (int)_v[1], (int)_v[2],
         (int)tile_idx, (int)_tile_n,
         (int)_tile, m_tile_name[_tile].c_str(),
         (double)s,
         (int)block[0][0], (int)block[0][1],
         (int)block[1][0], (int)block[1][1],
         (int)block[2][0], (int)block[2][1]);

      _tile_n = -1;
    }

    // tile to keep (fix)
    //
    tile = cellTile( m_plane, cell, tile_idx );

    tile_n = cellSize( m_plane, cell );

    // for each tile we're removing, decrement the tile support
    // for neighboring (nei_cell,nei_tile) values that have arcs into it
    // from the current (cell,tile)
    //
    for (rem_tile_idx=0; rem_tile_idx<tile_n; rem_tile_idx++) {
      if (rem_tile_idx == tile_idx) { continue; }
      rem_tile = cellTile( m_plane, cell, rem_tile_idx );

      for (idir=0; idir<6; idir++) {
        nei_cell = neiCell( cell, idir );
        if (nei_cell < 0) { continue; }

        //EXPERIMENT
        if (m_cell_pin[nei_cell] != 0) { continue; }

        nei_tile_n  = cellSize(m_plane, nei_cell);
        nei_v_n     = tileAdjIdxSize(rem_tile, idir);
        rdir        = m_dir_oppo[idir];

        if (nei_tile_n < nei_v_n ) {

          for ( nei_tile_idx=0; nei_tile_idx < nei_tile_n; nei_tile_idx++ ) {
            nei_tile = cellTile( m_plane, nei_cell, nei_tile_idx );

            if ( F(nei_tile, rem_tile, rdir) < m_zero ) { continue; }

            nei_pos = (rdir*m_cell_count*m_tile_count) + (nei_cell*m_tile_count) + nei_tile;
            _tile_support = tileSupport(m_plane, rdir, nei_cell, nei_tile);
            if (_tile_support == 0) { return -2; }

            _tile_support--;
            tileSupport(m_plane, rdir, nei_cell, nei_tile, _tile_support);
            markAC4Dirty(m_plane, nei_cell);

            // support has gone to 0, enqueue
            //
            if (tileSupport(m_plane, rdir, nei_cell, nei_tile) == 0) {
              if (cellTileVisited(m_plane, nei_cell, nei_tile) == 0) {
                cellTileQueuePush(m_plane, nei_cell, nei_tile);
                cellTileVisited(m_plane, nei_cell, nei_tile, 1);
              }
            }
          }

          continue;
        }

        // nei_v_n <= nei_tile_n
        //
        for (nei_v_idx=0; nei_v_idx<nei_v_n; nei_v_idx++) {
          nei_tile = tileAdjIdx( rem_tile, idir, nei_v_idx );
          _r = cellHasTile(m_plane, nei_cell, nei_tile);
          if (_r==0) { continue; }
          if (_r<0) { return _r; }

          _tile_support = tileSupport(m_plane, rdir, nei_cell, nei_tile);
          if (_tile_support==0) {
            m_conflict_idir = rdir;
            m_conflict_cell = nei_cell;
            m_conflict_tile = nei_tile;
            m_conflict_type = POMS_CONFLICT_NEGATIVE_SUPPORT_SANITY_ERROR;
            return -4;
          }

          _tile_support--;
          tileSupport(m_plane, rdir, nei_cell, nei_tile, _tile_support);
          markAC4Dirty(m_plane, nei_cell);

          if (_tile_support==0) {
            if (cellTileVisited(m_plane, nei_cell, nei_tile) == 0) {
              cellTileVisited(m_plane, nei_cell, nei_tile, 1);
              cellTileQueuePush(m_plane, nei_cell, nei_tile);
            }
          }


        }

      }


    }

    // once we've updated tile support, remove all tiles
    // and keep the one we're resolving
    //
    forceTile( m_plane, cell, tile );
    markAC4Dirty(m_plane, cell);

    _r = AC4Update();
    if (_r<0) {
      ret=-1;
      break;
    }

    iter++;
  }

  if (ret < 0) {return ret; }

  if (iter >= max_iter) {
    block_cell_count =
      (block[0][1] - block[0][0]) *
      (block[1][1] - block[1][0]) *
      (block[2][1] - block[2][0]);

    realized_count = realizedCellsWithinBlock(block);

    ret = 1;
    if (block_cell_count == realized_count) { ret = 0; }

    if (m_verbose >= POMS_VERBOSE_DEBUG) {
      printf("WFCBlock_ac4: ret:%i, iter:%i/%i, block_cell_count:%i, realized_count:%i\n",
          (int)ret,
          (int)iter, (int)max_iter, (int)block_cell_count, (int)realized_count);
    }

  }

  return ret;
}

double POMS::acceptHEMPProb(void) {
  double f, p,
         p_beg=0.0,
         p_end=0.5;

  f = (double)resolvedCount() / ((double)m_cell_count);
  return ((p_end - p_beg)*f) + p_beg;
}

// NEEDS UPDATING (pin, etc.)
//---
// Run WFC on `block` (AC4).
// Only cells within `block` will be fixed but constraints
//   will propagate outside of block.
//
// for input `wfc_step`:
//
//  >0 - only run for `wfc_step` steps before returning
// <=0 - run until conflict or block is fully resolved
//
// Return:
//
//  1 - block not fully resolved but no conflict has been encoutered
//  0 - every cell within `m_block` is resolved and the `(cell,tile)`
//      pairs touched are in an arc consistent state
// -1 - conflict was encountered
//
// On a conflict, the `m_conflict_*` variables will be populated
// accordingly.
//
int POMS::WFC_HEMP(int32_t block[][2], int64_t wfc_step) {
  int     _r, ret;
  int64_t cell=-1,
          block_cell_count=0,
          realized_count=0,
          iter=0, max_iter=-1,
          nei_cell, nei_pos,
          qpos=0;
  int32_t x,y,z,
          tile,
          tile_idx=-1,
          tile_n,
          _tile, _tile_n,
          _v[3], _idx,
          rem_tile,
          rem_tile_idx,
          nei_tile_n,
          nei_tile_idx,
          nei_tile,
          _tile_support=0,
          idir, rdir;
  double  _cur_com[3],
          _vec_d0[3],
          _vec_d1[3],
          _vec_[3],
          _d0, _d1,
          p, q, s=-1.0,
          f, p_beg=0.0,
          //p_end=0.5;
          p_end=0.25;

  ret = -5;

  max_iter = wfc_step;
  if (max_iter <= 0) {
    max_iter = m_size[0]*m_size[1]*m_size[2];
  }

  if (m_verbose >= POMS_VERBOSE_DEBUG) {
    printf("wfcBlock_ac4: wfc_step:%i, max_iter:%i block:[[%i,%i],[%i,%i],[%i,%i]]\n",
        (int)wfc_step, (int)max_iter,
        (int)block[0][0], (int)block[0][1],
        (int)block[1][0], (int)block[1][1],
        (int)block[2][0], (int)block[2][1]);
  }

  while (iter<max_iter) {

    // HEMP heuristic.
    // calculate the current entropy center of mass
    // for this block and if it's closer to the rally
    // point than the saved center of mass from the
    // beginning of the attempt, probabilistically
    // accept this block in the still unrealized
    // state
    //
    _r = entropyCenterOfMass( _cur_com, block );
    if (_r== 1) { return 0; }
    if (_r < 0) { return _r; }

    _vec_d0[0] = m_rally_point[0] - _cur_com[0];
    _vec_d0[1] = m_rally_point[1] - _cur_com[1];
    _vec_d0[2] = m_rally_point[2] - _cur_com[2];

    _vec_d1[0] = m_rally_point[0] - m_block_hemp_point[0];
    _vec_d1[1] = m_rally_point[1] - m_block_hemp_point[1];
    _vec_d1[2] = m_rally_point[2] - m_block_hemp_point[2];

    _d0 = sqrt( _vec_d0[0]*_vec_d0[0] + _vec_d0[1]*_vec_d0[1] + _vec_d0[2]*_vec_d0[2] );
    _d1 = sqrt( _vec_d1[0]*_vec_d1[0] + _vec_d1[1]*_vec_d1[1] + _vec_d1[2]*_vec_d1[2] );


    _r = entropyBlock( &s, block );
    if (_r<0) { return _r; }

    if (( _d0 < _d1 ) &&
        ( s < m_orig_block_entropy )) {
      f = (double)resolvedCount() / ((double)m_cell_count);
      q = ((p_end - p_beg)*f) + p_beg;

      p = rnd();
      if ( p < q ) {

        if (m_verbose >= POMS_VERBOSE_DEBUG) {
          printf("wfc_hemp: p:%f < q:%f, d0:%f < d1:%f\n",
              p, q, _d0, _d1);
        }

        return 0;
      }
    }

    //---
    // otherwise continue with normal wfc


    _r = computeCellEntropyWithinBlock(block);
    if (_r==1) { return 0; }
    if (_r <0) { return _r; }

    if (m_verbose >= POMS_VERBOSE_DEBUG) {
      printf("wfc_hemp: computeEntropyWithinBlock got %i\n", (int)_r);
    }

    _r = pickCellMinEntropyWithinBlock( &cell, &tile_idx, &s, block );

    // We really shouldn't get an error here as we should have already
    // figured out there was a contradiction. Kept in for paranoia.
    //
    if (_r <0) { ret=-1; break; }
    if (_r==1) { ret=0; break; }

    // we assume a continuation. If a conflict occurs,
    // ret will be set appropriately below or we'll
    // return with the appropriate value.
    //
    if (_r==0) { ret=1; }

    if (m_verbose >= POMS_VERBOSE_DEBUG) {
      _tile_n = cellSize(m_plane, cell);
      _tile = cellTile(m_plane, cell, tile_idx);
      cell2vec(_v, cell);
      printf("wfc_hemp: picking cell:%i[%i,%i,%i], tile_idx:%i/%i tile:%i(%s), e:%f"
         " (block:[[%i,%i],[%i,%i],[%i,%i]])\n",
         (int)cell, (int)_v[0], (int)_v[1], (int)_v[2],
         (int)tile_idx, (int)_tile_n,
         (int)_tile, m_tile_name[_tile].c_str(),
         (double)s,
         (int)block[0][0], (int)block[0][1],
         (int)block[1][0], (int)block[1][1],
         (int)block[2][0], (int)block[2][1]);

      _tile_n = -1;
    }

    // tile to keep (fix)
    //
    tile = cellTile( m_plane, cell, tile_idx );

    tile_n = cellSize( m_plane, cell );

    // for each tile we're removing, decrement the tile support
    // for neighboring (nei_cell,nei_tile) values that have arcs into it
    // from the current (cell,tile)
    //
    for (rem_tile_idx=0; rem_tile_idx<tile_n; rem_tile_idx++) {
      if (rem_tile_idx == tile_idx) { continue; }
      rem_tile = cellTile( m_plane, cell, rem_tile_idx );

      for (idir=0; idir<6; idir++) {
        nei_cell = neiCell( cell, idir );
        if (nei_cell < 0) { continue; }

        nei_tile_n = cellSize( m_plane, nei_cell );
        for ( nei_tile_idx=0; nei_tile_idx < nei_tile_n; nei_tile_idx++ ) {
          nei_tile = cellTile( m_plane, nei_cell, nei_tile_idx );

          rdir = m_dir_oppo[idir];
          if ( F(nei_tile, rem_tile, rdir) < m_zero ) { continue; }

          nei_pos = (rdir*m_cell_count*m_tile_count) + (nei_cell*m_tile_count) + nei_tile;

          _tile_support = tileSupport(m_plane, rdir, nei_cell, nei_tile);

          if (_tile_support == 0) {

            // sanity error
            //
            if (m_verbose >= POMS_VERBOSE_DEBUG) {
              cell2vec(_v, nei_cell);
              printf("SANITY ERROR: tile:%i@cell:%i(%i,%i,%i) has negative support (%i)\n",
                  (int)nei_tile, (int)nei_cell,
                  (int)_v[0], (int)_v[1], (int)_v[2],
                  (int)tileSupport(m_plane, rdir, nei_cell, nei_tile));
              fprintf(stderr, "SANITY ERROR: tile:%i@cell:%i(%i,%i,%i) has negative support (%i)\n",
                  (int)nei_tile, (int)nei_cell,
                  (int)_v[0], (int)_v[1], (int)_v[2],
                  (int)tileSupport(m_plane, rdir, nei_cell, nei_tile));
            }
            return -2;
          }

          _tile_support--;
          tileSupport(m_plane, rdir, nei_cell, nei_tile, _tile_support);


          // support has gone to 0, enqueue
          //
          if (tileSupport(m_plane, rdir, nei_cell, nei_tile) == 0) {

            if (cellTileVisited(m_plane, nei_cell, nei_tile) == 0) {
              cellTileVisited(m_plane, nei_cell, nei_tile, 1);
              cellTileQueuePush(m_plane, nei_cell, nei_tile);
            }

          }

        }
      }


    }

    // once we've updated tile support , remove all tiles
    // and keep the one we're resolving
    //
    forceTile(m_plane, cell, tile);

    _r = AC4Update();
    if (_r<0) {
      ret=-1;
      break;
    }

    iter++;
  }

  if (ret < 0) {return ret; }

  if (iter >= max_iter) {
    block_cell_count =
      (block[0][1] - block[0][0]) *
      (block[1][1] - block[1][0]) *
      (block[2][1] - block[2][0]);

    realized_count = realizedCellsWithinBlock(block);

    ret = 1;
    if (block_cell_count == realized_count) { ret = 0; }

    if (m_verbose >= POMS_VERBOSE_DEBUG) {
      printf("WFC_HEMP: ret:%i, iter:%i/%i, block_cell_count:%i, realized_count:%i\n",
          (int)ret,
          (int)iter, (int)max_iter, (int)block_cell_count, (int)realized_count);
    }

  }

  return ret;
}

//---
//---
//---

// Choose a sequential overlapping block schedule.
//
// overall number of choices in each dimension is:
//
// 2*floor( grid_size / block_size ) - ((grid_size % block_size) ? 0 : 1)
//
// The last bit is to catch the overflow when grid size is not an exact
// multiple of block size.
//
// `block` will be populated with the block choice
//
// Return:
//
//  0 - success
// <0 - error
//
int POMS::chooseBlock_sequential(int32_t block[][2], int64_t seq) {
  int32_t nz,ny,nx;
  int32_t seq_x, seq_y, seq_z;
  int32_t n_b[3];

  n_b[0] = m_block_size[0];
  n_b[1] = m_block_size[1];
  n_b[2] = m_block_size[2];

  if (n_b[0]<1) { n_b[0] = 1; }
  if (n_b[1]<1) { n_b[1] = 1; }
  if (n_b[2]<1) { n_b[2] = 1; }

  nx = (2*m_size[0]) / n_b[0];
  ny = (2*m_size[1]) / n_b[1];
  nz = (2*m_size[2]) / n_b[2];

  // m_size exact multiple of m_block,
  // no overflow to account for,
  // reduce overall count
  //
  if (((2*m_size[0]) % n_b[0]) == 0) { nx--; }
  if (((2*m_size[1]) % n_b[1]) == 0) { ny--; }
  if (((2*m_size[2]) % n_b[2]) == 0) { nz--; }

  seq = seq % (nx*ny*nz);

  seq_z  = seq / (nx*ny);
  seq   -= seq_z * nx * ny;

  seq_y  = seq / nx;
  seq   -= seq_y * nx;

  seq_x  = seq;

  if (seq_x == (nx-1)) { block[0][0] = m_size[0] - n_b[0]; }
  else                 { block[0][0] = seq_x*n_b[0]/2; }

  if (seq_y == (ny-1)) { block[1][0] = m_size[1] - n_b[1]; }
  else                 { block[1][0] = seq_y*n_b[1]/2; }

  if (seq_z == (nz-1)) { block[2][0] = m_size[2] - n_b[2]; }
  else                 { block[2][0] = seq_z*n_b[2]/2; }

  block[0][1] = block[0][0] + n_b[0];
  block[1][1] = block[1][0] + n_b[1];
  block[2][1] = block[2][0] + n_b[2];

  return 0;
}

// Return the number of blocks for a sequential and
// half overlapping block schedule.
//
int64_t POMS::blockSequenceCount(void) {
  int32_t nz,ny,nx;
  int32_t n_b[3];

  n_b[0] = m_block_size[0];
  n_b[1] = m_block_size[1];
  n_b[2] = m_block_size[2];

  if (n_b[0]<1) { n_b[0] = 1; }
  if (n_b[1]<1) { n_b[1] = 1; }
  if (n_b[2]<1) { n_b[2] = 1; }

  nx = (2*m_size[0]) / n_b[0];
  ny = (2*m_size[1]) / n_b[1];
  nz = (2*m_size[2]) / n_b[2];

  // m_size exact multiple of m_block,
  // no overflow to account for,
  // reduce overall count
  //
  if (((2*m_size[0]) % n_b[0]) == 0) { nx--; }
  if (((2*m_size[1]) % n_b[1]) == 0) { ny--; }
  if (((2*m_size[2]) % n_b[2]) == 0) { nz--; }

  return nx*ny*nz;
}


// Choose a max entropy block.
//
// Return:
//
//  0 - success
// <0 - error
//
int POMS::chooseBlock_maxEntropyBlock(int32_t block[][2], int64_t seq) {
  int r=-1, ret=-1;
  int32_t nz,ny,nx,
          mz,my,mx,
          ix,iy,iz;
  int32_t seq_x, seq_y, seq_z;
  int32_t n_b[3];

  double p,
         max_block_val = -1.0,
         block_val = -1.0,
         dup_count=1.0,
         noise=0.0;
  double *B, *M;

  r = computeCellEntropy();
  if (r<0) { return r; }

  r = computeBlockEntropy();
  if (r<0) { return r; }

  if (m_verbose >= POMS_VERBOSE_DEBUG) {
    printf("BLOCK_ENTROPY:\n");
    printDebugBlockEntropy();

    printf("CELL_ENTROPY:\n");
    printDebugCellEntropy();
  }

  n_b[0] = m_size[0] - m_block_size[0] + 1;
  n_b[1] = m_size[1] - m_block_size[1] + 1;
  n_b[2] = m_size[2] - m_block_size[2] + 1;

  mx = m_size[0];
  my = m_size[1];
  mz = m_size[2];

  B = &(m_block_entropy[0]);

  block[0][0] = 0;
  block[1][0] = 0;
  block[2][0] = 0;

  for (iz=0; iz<n_b[2]; iz++) {
    for (iy=0; iy<n_b[1]; iy++) {
      for (ix=0; ix<n_b[0]; ix++) {

        block_val = B[ (iz*mx*my) + (iy*mx) + ix ];

        if (block_val < m_zero) { continue; }

        noise = noisePowerLaw( m_entropy_rand_coefficient, m_entropy_rand_exponent );
        block_val += noise;

        if ((max_block_val < 0.0) ||
            (max_block_val < (block_val - m_zero))) {
          max_block_val = block_val;
          block[0][0] = ix;
          block[1][0] = iy;
          block[2][0] = iz;
          dup_count = 1.0;
        }
        else if (fabs(max_block_val-block_val) < m_zero) {
          dup_count += 1.0;

          p = rnd();
          if ( p < (1.0 / dup_count) ) {
            block[0][0] = ix;
            block[1][0] = iy;
            block[2][0] = iz;
          }
          else { }

        }

      }
    }
  }

  block[0][1] = block[0][0] + m_block_size[0];
  block[1][1] = block[1][0] + m_block_size[1];
  block[2][1] = block[2][0] + m_block_size[2];

  if (max_block_val < -m_zero) { return -1; }

  return 0;
}

// Choose a min entropy block.
//
// Return:
//
//  0 - success
// <0 - error
//
int POMS::chooseBlock_minEntropyBlock(int32_t block[][2], int64_t seq) {
  int r=-1, ret=-1;
  int32_t nz,ny,nx,
          mz,my,mx,
          ix,iy,iz;
  int32_t seq_x, seq_y, seq_z;
  int32_t n_b[3];

  double p,
         min_block_val = -1.0,
         block_val = -1.0,
         dup_count=1.0,
         noise=0.0;
  double *B, *M;

  r = computeCellEntropy();
  if (r<0) { return r; }

  r = computeBlockEntropy();
  if (r<0) { return r; }

  if (m_verbose >= POMS_VERBOSE_DEBUG) {
    printf("BLOCK_ENTROPY:\n");
    printDebugBlockEntropy();

    printf("CELL_ENTROPY:\n");
    printDebugCellEntropy();
  }

  n_b[0] = m_size[0] - m_block_size[0] + 1;
  n_b[1] = m_size[1] - m_block_size[1] + 1;
  n_b[2] = m_size[2] - m_block_size[2] + 1;

  mx = m_size[0];
  my = m_size[1];
  mz = m_size[2];

  B = &(m_block_entropy[0]);

  block[0][0] = 0;
  block[1][0] = 0;
  block[2][0] = 0;

  for (iz=0; iz<n_b[2]; iz++) {
    for (iy=0; iy<n_b[1]; iy++) {
      for (ix=0; ix<n_b[0]; ix++) {

        block_val = B[ (iz*mx*my) + (iy*mx) + ix ];

        if (block_val < m_zero) { continue; }

        noise = noisePowerLaw( m_entropy_rand_coefficient, m_entropy_rand_exponent );
        block_val += noise;

        if ((min_block_val < 0.0) ||
            (min_block_val > (block_val + m_zero))) {
          min_block_val = block_val;
          block[0][0] = ix;
          block[1][0] = iy;
          block[2][0] = iz;
          dup_count = 1.0;
        }
        else if (fabs(min_block_val-block_val) < m_zero) {
          dup_count+=1.0;

          p = rnd();
          if ( p < (1.0 / dup_count) ) {
            block[0][0] = ix;
            block[1][0] = iy;
            block[2][0] = iz;
          }
          else { }

        }

      }
    }
  }

  block[0][1] = block[0][0] + m_block_size[0];
  block[1][1] = block[1][0] + m_block_size[1];
  block[2][1] = block[2][0] + m_block_size[2];

  //DEBUG
  //DEBUG
  //DEBUG
  //printf("chooseblock_meb: min_block_val: %f, -m_zero: %f, ? %i\n",
  //    min_block_val, -m_zero, min_block_val < -m_zero );

  if (min_block_val < -m_zero) { return -1; }

  return 0;
}

//---
//---
//---

// Choose a block based on entropy and distance.
//
// Return:
//
//  0 - success
// <0 - error
//


int POMS::chooseBlock_waveFront(int32_t block[][2], int64_t seq) {
  int r=-1, ret=-1;
  int32_t nz,ny,nx,
          mz,my,mx,
          ix,iy,iz;
  int32_t seq_x, seq_y, seq_z;
  int32_t n_b[3];

  double p,
         min_block_val = -1.0,
         block_val = -1.0,
         dup_count=1.0,
         noise=0.0;
  double *B, *M;
  double *R,
         _F=1.0,
         dist_mod,
         alpha,
         alpha_s = 0.75,
         alpha_e = 0.98;

  int64_t cell,
          resolved_count;

  //_F = 1.0;
  //_F = (double)(m_cell_count * m_tile_count);

  r = computeCellEntropy();
  if (r<0) { return r; }

  r = computeBlockEntropy(1);
  if (r<0) { return r; }

  r = computeDistanceModifier();
  if (r<0) { return r; }

  if (m_verbose >= POMS_VERBOSE_DEBUG) {
    printf("BLOCK_ENTROPY:\n");
    printDebugBlockEntropy();

    printf("CELL_ENTROPY:\n");
    printDebugCellEntropy();
  }

  n_b[0] = m_size[0] - m_block_size[0] + 1;
  n_b[1] = m_size[1] - m_block_size[1] + 1;
  n_b[2] = m_size[2] - m_block_size[2] + 1;

  mx = m_size[0];
  my = m_size[1];
  mz = m_size[2];

  B = &(m_block_entropy[0]);

  block[0][0] = 0;
  block[1][0] = 0;
  block[2][0] = 0;

  R = &(m_distance_modifier[0]);

  _F = (double)(m_cell_count * m_tile_count);

  resolved_count=0;
  for (cell=0; cell<m_cell_count; cell++) {
    if (cellSize( m_plane, cell )==1) {
      resolved_count++;
    }
  }
  alpha = (double)resolved_count / (double)m_cell_count;
  if ((alpha > alpha_s) && (alpha < alpha_e)) {
    _F = (double)(m_cell_count - resolved_count) / (double)m_cell_count;
    _F *= (double)(m_cell_count * m_tile_count);
    _F *= (alpha_e - alpha_s);
  }
  else if (alpha >= alpha_e) {
    _F = 0.0;
  }

  for (iz=0; iz<n_b[2]; iz++) {
    for (iy=0; iy<n_b[1]; iy++) {
      for (ix=0; ix<n_b[0]; ix++) {

        cell = xyz2cell(ix,iy,iz);
        if ( cellSize(m_plane, cell) <= 1 ) { continue; }
        if ( m_cell_pin[cell] != 0 ) { continue; }

        dist_mod = _F * R[ (iz*mx*my) + (iy*mx) + ix ];

        block_val = B[ (iz*mx*my) + (iy*mx) + ix ];
        block_val += dist_mod;

        if (block_val < m_zero) { continue; }

        noise = noisePowerLaw( m_entropy_rand_coefficient, m_entropy_rand_exponent );
        block_val += noise;

        if ((min_block_val < 0.0) ||
            (min_block_val > (block_val + m_zero))) {
          min_block_val = block_val;
          block[0][0] = ix;
          block[1][0] = iy;
          block[2][0] = iz;
          dup_count = 1.0;
        }
        else if (fabs(min_block_val-block_val) < m_zero) {
          dup_count+=1.0;

          p = rnd();
          if ( p < (1.0 / dup_count) ) {
            block[0][0] = ix;
            block[1][0] = iy;
            block[2][0] = iz;
          }
          else { }

        }

      }
    }
  }

  block[0][1] = block[0][0] + m_block_size[0];
  block[1][1] = block[1][0] + m_block_size[1];
  block[2][1] = block[2][0] + m_block_size[2];

  if (min_block_val < -m_zero) { return -1; }

  return 0;
}


// Wrapper function to choose a block.
// Currently only sequential blocks will be chosen (for MMS)
//
int POMS::chooseBlock(int32_t block[][2], int64_t seq) {
  int ret=-1;

  switch (m_block_choice_policy) {
    case POMS_BLOCK_CHOICE_SEQUENTIAL:
      ret = chooseBlock_sequential(block, seq);
      break;
    case POMS_BLOCK_CHOICE_MAX_ENTROPY:
      ret = chooseBlock_maxEntropyBlock(block, seq);
      break;
    case POMS_BLOCK_CHOICE_MIN_ENTROPY:
      ret = chooseBlock_minEntropyBlock(block, seq);
      break;
    case POMS_BLOCK_CHOICE_WAVEFRONT:
      ret = chooseBlock_waveFront(block, seq);
      break;
    default:
      ret=-1;
      break;
  };

  return ret;
}

//---
//  __  __ __  __ ___
// |  \/  |  \/  / __|
// | |\/| | |\/| \__ \
// |_|  |_|_|  |_|___/
//

// Initialize MMS.
// Run AC4 for arc consistency, initializing it here.
// Check to make sure grid is in fully realized state as
// MMS requires a fully realized 'ground state'.
//
// Initalize:
//
// - `m_seq` = 0
// - `m_state` = POMS_STATE_INIT
//
// Return:
//
//  0 - success
// <0 - failure
//
// Failure can occur if the initial grid cannot be put
// into an arc consistent state (AC4Init failure) or
// if the initial grid is not fully realized.
//
int POMS::MMSInit(void) {
  int64_t cell;
  int r;
  m_state = POMS_STATE_INIT;
  m_seq = 0;

  r = AC4Init();
  if (r<0) { return r; }

  for (cell=0; cell<m_cell_count; cell++) {
    if (cellSize(m_plane, cell) != 1) { r=-2; break; }
  }

  return r;
}

// Begin of an MMS block run.
//
// Save the grid in case we need to restore after a conflict.
// Choose a block from the block schedule, using `m_seq` to
// tell how far along we are.
// Fuzz/soften the block to it's prefatory state
// Put `m_state` into init.
//
// Return:
//
//  0 - success, ready to run
// <0 - failure
//
// Given a valid initial grid, this should not fail.
//
int POMS::MMSBegin(void) {
  int r;
  int save_plane = (m_plane + 1)%m_plane_count;

  r = saveGrid();
  if (r<0) { return r; }

  r = chooseBlock(m_block, m_seq);
  if (r<0) { return r; }

  r = soften(m_block);
  if (r<0) { return r; }

  m_state = POMS_STATE_INIT;

  return r;
}

// Resolution algorithm on block
//
int POMS::MMSStep(void) {
  int r;

  m_state = POMS_STATE_CONSISTENT;

  r = WFCBlock_ac4( m_block, 1 );
  if (r<0) {
    m_state = POMS_STATE_CONFLICT;
    return r;
  }

  if (r==0) { m_state = POMS_STATE_SUCCESS; }

  return r;
}

// Finalize block
//
// If block has been found, keep it.
// If block has failed, restore to state before
//
// increment `m_eq`
//
// Return:
//
//  0 - success
// <0 - error
//
// An error here is a bug as `m_state` should only ever be
// success or conflict.
//
int POMS::MMSEnd(void) {
  int r=0;
  m_seq++;

  // success  - don't need to don anything (Keep block)
  // conflict - restore to previous state
  // other    - error
  //
  if      (m_state == POMS_STATE_SUCCESS)   { }
  else if (m_state == POMS_STATE_CONFLICT)  { r = restoreGrid(); }
  else                                      { r=-1; }

  return r;
}

// UNTESTED
//
// skeleton to run MMS
//
// Return:
//
//  0 - success
// -1 - error
//
// Unless there are lurking bugs, the only way an error
// should every occur is if there's a setup problem
// (MMSInit failed). Otherwise, this should only
// every return success.
//
int POMS::MMS(int64_t mms_step) {
  int64_t step,
          wfc_step,
          max_wfc_step;
  int32_t block[3][2];
  int r, ret=1;

  max_wfc_step = m_block_size[0]*m_block_size[1]*m_block_size[2];

  r = MMSInit();
  if (r<0) { return r; }

  for (step=0; step<mms_step; step++) {
    r = MMSBegin();
    if (r<0) { ret=r; break; }

    for (wfc_step=0; wfc_step < max_wfc_step; wfc_step++) {
      r = MMSStep();
      if (r<=0) { break; }
    }

    r = MMSEnd();
    if (r<0) { ret=r; break; }

  }

  if ((step==mms_step) && (ret>=0)) { ret=0; }

  return ret;
}

//---
//  ___ __  __ ___
// | _ )  \/  / __|
// | _ \ |\/| \__ \
// |___/_|  |_|___/
//

int POMS::BMS(int64_t max_rounds, int64_t bms_step) {
  int r,
      ret = 1;
  int64_t it, n_it, step;
  int32_t _v[3];

  r = BMSInit();
  if (r<0) { return r; }

  if (m_viz_cb!=NULL) { m_viz_cb(); }

  n_it = max_rounds;
  if (n_it <= 0) { n_it = blockSequenceCount(); }

  if (bms_step<=0) {
    bms_step = m_block_size[0]*m_block_size[1]*m_block_size[2] + 1;
  }

  for (it=0; (it<n_it) && (ret>=0); it++) {

    r = BMSBegin();
    if (r<0) {

      if (m_verbose >= POMS_VERBOSE_DEBUG) {
        cell2vec(_v, m_conflict_cell);
        printf("bms: begin fail (conflict cell:%i[%i,%i,%i], tile:%i, idir:%i, type:%i)\n",
            (int)m_conflict_cell,
            (int)_v[0], (int)_v[1], (int)_v[2],
            (int)m_conflict_tile,
            (int)m_conflict_idir,
            (int)m_conflict_type );
      }

      return r;
    };
    if (m_viz_cb!=NULL) { m_viz_cb(); }

    for (step=0; step<bms_step; step++) {
      r = BMSStep();
      if (r<=0) { break; }

      if (m_viz_cb!=NULL) { m_viz_cb(); }
    }

    r = BMSEnd();
    if (r<=0) {

      //if (r<0)  { printf("bms: end fail\n"); }
      //else      { printf("bms: end success\n"); }

      return r;
    }

    if (m_viz_cb!=NULL) { m_viz_cb(); }
  }

  return -1;
}

// Initialize BMS run, including initializing
// `m_state`, `m_seq` and the AC4 structures
// for arc consistency.
//
// m_state  - POMS_STATE_INIT
// m_seq    - 0
//
// Return:
//
//  0 - success
// <0 - error
//
// AC4Init can fail if constraint propagation finds
// a conflict. On conflict, the `m_conflict_*` variables
// will be set accordingly.
//
int POMS::BMSInit(void) {
  int64_t cell;
  int r=-1, ret=-1;

  m_state = POMS_STATE_INIT;
  m_seq = 0;
  m_retry_count = 0;

  // default soften window size is 3x block size
  //
  if (m_soften_size[0] < 1) { m_soften_size[0] = 3*m_block_size[0]; }
  if (m_soften_size[1] < 1) { m_soften_size[1] = 3*m_block_size[1]; }
  if (m_soften_size[2] < 1) { m_soften_size[2] = 3*m_block_size[2]; }
  clampParameters();

  r = AC4Init();
  if (r<0) { ret=r; }

  ret = r;

  if (m_verbose >= POMS_VERBOSE_RUN) {
    printf("BMSInit: block_size:[%i,%i,%i], soften_size:[%i,%i,%i]\n",
        (int)m_block_size[0], (int)m_block_size[1], (int)m_block_size[2],
        (int)m_soften_size[0], (int)m_soften_size[1], (int)m_soften_size[2] );
  }

  if (m_verbose >= POMS_VERBOSE_DEBUG) {
    printf("# BMSInit m_state:%i, m_seq:%i (ac4init:%i)\n",
        (int)m_state, (int)m_seq, ret);
  }

  // probably redundant because of ac4init above, but shouldn't hurt
  //
  for (cell=0; cell<m_cell_count; cell++) {

    //EXPERIMENT
    if (m_cell_pin[cell] != 0) { continue; }

    markAC4Dirty(m_plane, cell);
  }

  return ret;
}

// Start of a block realization
//
// - If this is the first try, save the grid
// - Choose block from block scheduler (e.g. max entropy block)
// - fuzz block and perform constraint propagation (AC4)
//
// m_state - POMS_STATE_INIT
//
// Return:
//
//  0 - success
// <0 - failure
//
// Other than a sanity error in saveGrid or chooseBlock, the
// only error that can occur is if the initial fuzz and constraint
// propagation fails. If we get here, this shouldn't happen
// as fuzzing only ever relaxes constraints on the grid.
//
int POMS::BMSBegin(void) {
  int r;

  if (m_retry_count==0) {
    r = saveGrid();
    if (r<0) { return r; }
  }
  else {

    if (m_verbose >= POMS_VERBOSE_DEBUG) {
      printf("# BMSBegin.retry: before soften AC4Consistency: %i\n", AC4Consistency());
    }

  }

  r = chooseBlock(m_block, m_seq);

  //DEBUG
  //DEBUG
  //DEBUG
  //DEBUG
  //printf("BMSBegin: chooseBlock: %i\n", r);

  if (r<0) { return r; }

  resetAC4Dirty(m_plane);

  r = soften(m_block);
  if (r<0) { return r; }

  m_state = POMS_STATE_INIT;

  if (m_verbose >= POMS_VERBOSE_RUN) {
    printf("BMSBegin: retry:%i/%i, chose block:[[%i,%i],[%i,%i],[%i,%i]\n",
        (int)m_retry_count, (int)m_retry_max,
        (int)m_block[0][0], (int)m_block[0][1],
        (int)m_block[1][0], (int)m_block[1][1],
        (int)m_block[2][0], (int)m_block[2][1]);
  }


  if (m_verbose >= POMS_VERBOSE_DEBUG) {
    printf("blockentropy sanity:%i\n",
        sanityBlockEntropy(m_block_size));
    printf("# BMSBegin m_state:%i, m_retry_count:%i\n",
        (int)m_state, (int)m_retry_count);

    printf("# BMSBegin: AC4Consistency: %i\n", AC4Consistency());
  }

  return r;
}

// Process a step of block realization
//
// - Run the solver (WFC on a block using AC4)
//   to choose a cell to fix, fix it, propagate
//   constraints, etc.
//
// Note that constraint propagation will spill
// outside of the block into the wider grid.
// A conflict can occur inside or outside of
// the grid. Should such a conflict occur,
// this function will return <0.
//
// The expected use of this function is to run
// it until it returns either a negative number or
// 0. Returning a positive number indicates a 'continuation'
// condition, where the block is partially realized
// but has no conflicts.
//
// Return:
//
//  0 - success, block fully realized without conflicts
// <0 - failure, grid has conflicts
// >0 - continuation, block partially resolved without
//      conflicts
//
// In each of the above, `m_state` will be:
//
// POMS_STATE_SUCCESS     - return 0, block fully realized
// POMS_STATE_CONFLICT    - return <0, block has conflicts
// POMS_STATE_CONSISTENT  - return >0, block not fully realized
//                                     but has no conflicts
//
// In the case of a conflict, the `m_conflict_*` variables
// will be set accordingly.
//
int POMS::BMSStep(void) {
  int r=-1;

  if (m_verbose >= POMS_VERBOSE_STEP) {
    printf("BMSStep: block[[%i,%i],[%i,%i],[%i,%i]]\n",
        (int)m_block[0][0], (int)m_block[0][1],
        (int)m_block[1][0], (int)m_block[1][1],
        (int)m_block[2][0], (int)m_block[2][1]);
  }


  m_state = POMS_STATE_CONSISTENT;

  r = WFCBlock_ac4( m_block, 1 );
  if (r<0) {
    m_state = POMS_STATE_CONFLICT;

    if (m_verbose >= POMS_VERBOSE_DEBUG) {
      printf("# BMSStep, conflict, m_state:%i, r:%i\n",
          (int)m_state, (int)r);
    }

    return r;
  }

  if (r==0) { m_state = POMS_STATE_SUCCESS; }

  if (m_verbose >= POMS_VERBOSE_DEBUG) {
    printf("# BMSStep, m_state:%i, r:%i\n",
        (int)m_state, (int)r);
  }

  return r;
}

// Post processing after BMSStep has completed.
//
// If BMSStep successfully resolved the block,
// reset `m_retry_count` so that we can continue
// as normal.
//
// If BMSStep encountered a conflict, increment
// `m_retry_count` and if it's below the maximum
// `m_retry_max`, restore the grid to the previously
// known good state so that we can try again.
//
// If the `m_retry_max` limit is hit, soften
// the block and it's neighboring region as
// given by `m_soften_size[]`.
// Once softened, reset `m_retry_count`.
//
// `m_seq` incremented here.
//
// Return:
//
// >0 - continuation
//  0 - success
// <0 - error
//
// This function should not fail but propagates
// any errors it encounters from restoreGrid and soften_ac4
// or if `m_state` is invalid (not one of POMS_STATE_SUCCESS
// or POMS_STATE_CONFLICT).
//
int POMS::BMSEnd(void) {
  float f;
  int r=0, ret=-1;
  int64_t rez;

  m_seq++;

  if (m_verbose >= POMS_VERBOSE_RUN) {
    printf("BMSEnd: m_state:%s(%i), m_retry_count:%i/%i\n",
        stateDescr(m_state),
        (int)m_state, (int)m_retry_count, (int)m_retry_max);
  }

  if (m_verbose >= POMS_VERBOSE_DEBUG) {
    printf("# BMSEnd: AC4Consistency: %i\n", AC4Consistency());
  }


  if (m_state == POMS_STATE_SUCCESS)  {

    if (m_verbose >= POMS_VERBOSE_RUN) {
      rez = resolvedCount();
      f = (float)rez / (float)m_cell_count;
      printf("BMSEnd: success, keeping block [[%i,%i],[%i,%i],[%i,%i]] (resolved %i/%i (%f))\n",
          (int)m_block[0][0], (int)m_block[0][1],
          (int)m_block[1][0], (int)m_block[1][1],
          (int)m_block[2][0], (int)m_block[2][1],
          (int)rez, (int)m_cell_count, f );
    }

    m_retry_count=0;

    ret = ( (resolvedCount() == m_cell_count) ? 0 : 1 );
  }

  else if (m_state == POMS_STATE_CONFLICT) {

    if (m_retry_count < m_retry_max) {

      if (m_verbose >= POMS_VERBOSE_RUN) {
        printf("BMSEnd: reverting block [[%i,%i],[%i,%i],[%i,%i]] (retry)\n",
            (int)m_block[0][0], (int)m_block[0][1],
            (int)m_block[1][0], (int)m_block[1][1],
            (int)m_block[2][0], (int)m_block[2][1]);
      }


      if (m_verbose >= POMS_VERBOSE_DEBUG) {
        printf("m_ac4_dirty[%i][%i]:\n", (int)m_plane, (int)m_ac4_dirty[m_plane].size());
        printDebugAC4Dirty(m_plane);
        printf("\n");

      }


      r = restoreGrid();
      if (r<0) { return r; }

      resetAC4Dirty(m_plane);

      m_retry_count++;

    }
    else {


      m_soften_block[0][0] = m_block[0][0] - m_soften_size[0];
      m_soften_block[0][1] = m_block[0][1] + m_soften_size[0];
      m_soften_block[1][0] = m_block[1][0] - m_soften_size[1];
      m_soften_block[1][1] = m_block[1][1] + m_soften_size[1];
      m_soften_block[2][0] = m_block[2][0] - m_soften_size[2];
      m_soften_block[2][1] = m_block[2][1] + m_soften_size[2];

      if (m_soften_block[0][0] < 0)         { m_soften_block[0][0] = 0; }
      if (m_soften_block[0][1] > m_size[0]) { m_soften_block[0][1] = m_size[0]; }
      if (m_soften_block[1][0] < 0)         { m_soften_block[1][0] = 0; }
      if (m_soften_block[1][1] > m_size[1]) { m_soften_block[1][1] = m_size[1]; }
      if (m_soften_block[2][0] < 0)         { m_soften_block[2][0] = 0; }
      if (m_soften_block[2][1] > m_size[2]) { m_soften_block[2][1] = m_size[2]; }

      if (m_verbose >= POMS_VERBOSE_RUN) {
        printf("BMSEnd: retry limit exceeded, restoring grid and softening block [[%i,%i],[%i,%i],[%i,%i]]\n",
            (int)m_soften_block[0][0], (int)m_soften_block[0][1],
            (int)m_soften_block[1][0], (int)m_soften_block[1][1],
            (int)m_soften_block[2][0], (int)m_soften_block[2][1]);
      }

      // We might have propagated constraints outside of even the soften block radius.
      // To account for this possibility, restore the grid to previously known good
      // state and then soften.
      //
      r = restoreGrid();
      if (r<0) { return r; }

      resetAC4Dirty(m_plane);

      r = soften(m_soften_block);
      if (r<0) { return r; }

      m_retry_count=0;
    }

    ret = 1;

  }
  else { ret = -1; }

  return ret;
}

//---
//  ___  ___  __  __ ___
// | _ \/ _ \|  \/  / __|
// |  _/ (_) | |\/| \__ \
// |_|  \___/|_|  |_|___/
//

int POMS::POMSGo(int64_t poms_step) {
  return -1;
}

//---
//
int POMS::POMSInit(void) {
  int64_t cell;
  int r=-1, ret=-1;

  m_state = POMS_STATE_INIT;
  m_seq = 0;

  // default soften window size is 3x block size
  //
  if (m_soften_size[0] < 1) { m_soften_size[0] = 3*m_block_size[0]; }
  if (m_soften_size[1] < 1) { m_soften_size[1] = 3*m_block_size[1]; }
  if (m_soften_size[2] < 1) { m_soften_size[2] = 3*m_block_size[2]; }
  clampParameters();

  r=0;
  switch (m_hemp_policy) {
    case POMS_HEMP_POLICY_STATIC:
      if (m_rally_point[0] < 0) { m_rally_point[0] = ((double)m_size[0])/2.0; }
      if (m_rally_point[1] < 0) { m_rally_point[1] = ((double)m_size[1])/2.0; }
      if (m_rally_point[2] < 0) { m_rally_point[2] = ((double)m_size[2])/2.0; }
      break;
    case POMS_HEMP_POLICY_COM:
      break;
    default:
      r=-1;
      break;
  }
  if (r<0) { return r; }

  r = AC4Init();
  if (r<0) { ret=r; }

  ret = r;

  if (m_verbose >= POMS_VERBOSE_RUN) {
    printf("POMSInit: block_size:[%i,%i,%i], soften_size:[%i,%i,%i]\n",
        (int)m_block_size[0], (int)m_block_size[1], (int)m_block_size[2],
        (int)m_soften_size[0], (int)m_soften_size[1], (int)m_soften_size[2] );
  }

  if (m_verbose >= POMS_VERBOSE_DEBUG) {
    printf("# POMSInit m_state:%i, m_seq:%i (ac4init:%i)\n",
        (int)m_state, (int)m_seq, ret);
  }

  return ret;
}

//---
//
int POMS::POMSBegin(void) {
  int r;
  int32_t grid_block[3][2];

  if (m_retry_count==0) {
    r = saveGrid();
    if (r<0) { return r; }
  }

  switch (m_hemp_policy) {
    case POMS_HEMP_POLICY_COM:
      grid_block[0][0] = 0; grid_block[0][1] = m_size[0];
      grid_block[1][0] = 0; grid_block[1][1] = m_size[1];
      grid_block[2][0] = 0; grid_block[2][1] = m_size[2];

      r = entropyCenterOfMass( m_rally_point, grid_block );
      break;
    case POMS_HEMP_POLICY_STATIC:
      r = 0;
      break;
    default:
      r = -1;
      break;
  }
  if (r<0) { return r; }

  r = chooseBlock(m_block, m_seq);
  if (r<0) { return r; }

  r = entropyCenterOfMass( m_block_hemp_point, m_block );
  if (r<0) { return r; }

  r = entropyBlock( &m_orig_block_entropy, m_block );
  if (r<0) { return r; }

  r = soften(m_block);
  if (r<0) { return r; }

  m_state = POMS_STATE_INIT;

  if (m_verbose >= POMS_VERBOSE_RUN) {
    printf("POMSBegin: retry:%i/%i, chose block:[[%i,%i],[%i,%i],[%i,%i], block_com(%f,%f,%f), rally_point(%f,%f,%f)\n",
        (int)m_retry_count, (int)m_retry_max,
        (int)m_block[0][0], (int)m_block[0][1],
        (int)m_block[1][0], (int)m_block[1][1],
        (int)m_block[2][0], (int)m_block[2][1],
        m_block_hemp_point[0], m_block_hemp_point[1], m_block_hemp_point[2],
        m_rally_point[0], m_rally_point[1], m_rally_point[2]);
    printf("blockentropy sanity:%i\n",
        sanityBlockEntropy(m_block_size));
  }

  if (m_verbose >= POMS_VERBOSE_DEBUG) {
    printf("# POMSBegin m_state:%i, m_retry_count:%i\n",
        (int)m_state, (int)m_retry_count);
  }

  return r;
}

//---
//
int POMS::POMSStep(void) {
  int r=-1;

  if (m_verbose >= POMS_VERBOSE_STEP) {
    printf("POMSStep: block[[%i,%i],[%i,%i],[%i,%i]]\n",
        (int)m_block[0][0], (int)m_block[0][1],
        (int)m_block[1][0], (int)m_block[1][1],
        (int)m_block[2][0], (int)m_block[2][1]);
  }

  m_state = POMS_STATE_CONSISTENT;

  r = WFC_HEMP( m_block, 1 );
  if (r<0) {
    m_state = POMS_STATE_CONFLICT;

    if (m_verbose >= POMS_VERBOSE_DEBUG) {
      printf("# POMSStep, conflict, m_state:%i, r:%i\n",
          (int)m_state, (int)r);
    }

    return r;
  }

  if (r==0) { m_state = POMS_STATE_SUCCESS; }

  if (m_verbose >= POMS_VERBOSE_DEBUG) {
    printf("# POMSStep, m_state:%i, r:%i\n",
        (int)m_state, (int)r);
  }

  return r;
}

//---
//
int POMS::POMSEnd(void) {
  int r=0,
      ret=-1;
  m_seq++;

  if (m_verbose >= POMS_VERBOSE_RUN) {
    printf("POMSEnd: m_state:%s(%i), m_retry_count:%i/%i\n",
        stateDescr(m_state),
        (int)m_state, (int)m_retry_count, (int)m_retry_max);
  }

  if (m_state == POMS_STATE_SUCCESS)  {

    if (m_verbose >= POMS_VERBOSE_RUN) {
      printf("POMSEnd: success, keeping block [[%i,%i],[%i,%i],[%i,%i]]\n",
          (int)m_block[0][0], (int)m_block[0][1],
          (int)m_block[1][0], (int)m_block[1][1],
          (int)m_block[2][0], (int)m_block[2][1]);
    }

    m_retry_count=0;

    ret = ( (resolvedCount() == m_cell_count) ? 0 : 1 );
  }

  else if (m_state == POMS_STATE_CONFLICT) {

    if (m_retry_count < m_retry_max) {

      if (m_verbose >= POMS_VERBOSE_RUN) {
        printf("POMSEnd: reverting block [[%i,%i],[%i,%i],[%i,%i]] (retry)\n",
            (int)m_block[0][0], (int)m_block[0][1],
            (int)m_block[1][0], (int)m_block[1][1],
            (int)m_block[2][0], (int)m_block[2][1]);
      }

      r = restoreGrid();
      if (r<0) { return r; }

      m_retry_count++;

    }
    else {

      m_soften_block[0][0] = m_block[0][0] - m_soften_size[0];
      m_soften_block[0][1] = m_block[0][1] + m_soften_size[0];
      m_soften_block[1][0] = m_block[1][0] - m_soften_size[1];
      m_soften_block[1][1] = m_block[1][1] + m_soften_size[1];
      m_soften_block[2][0] = m_block[2][0] - m_soften_size[2];
      m_soften_block[2][1] = m_block[2][1] + m_soften_size[2];

      if (m_soften_block[0][0] < 0)         { m_soften_block[0][0] = 0; }
      if (m_soften_block[0][1] > m_size[0]) { m_soften_block[0][1] = m_size[0]; }
      if (m_soften_block[1][0] < 0)         { m_soften_block[1][0] = 0; }
      if (m_soften_block[1][1] > m_size[1]) { m_soften_block[1][1] = m_size[1]; }
      if (m_soften_block[2][0] < 0)         { m_soften_block[2][0] = 0; }
      if (m_soften_block[2][1] > m_size[2]) { m_soften_block[2][1] = m_size[2]; }

      if (m_verbose >= POMS_VERBOSE_RUN) {
        printf("POMSEnd: retry limit exceeded, restoring grid and softening block [[%i,%i],[%i,%i],[%i,%i]]\n",
            (int)m_soften_block[0][0], (int)m_soften_block[0][1],
            (int)m_soften_block[1][0], (int)m_soften_block[1][1],
            (int)m_soften_block[2][0], (int)m_soften_block[2][1]);
      }

      // We might have propagated constraints outside of even the soften block radius.
      // To account for this possibility, restore the grid to previously known good
      // state and then soften.
      //
      r = restoreGrid();
      if (r<0) { return r; }

      r = soften(m_soften_block);
      if (r<0) { return r; }

      m_retry_count=0;
    }

    ret = 1;

  }
  else { ret = -1; }

  if (m_verbose >= POMS_VERBOSE_RUN) {
    int64_t rescount= resolvedCount();
    double pct = (double)rescount/(double)m_cell_count;
    printf("POMSEnd: %f(%i) clusterCount: %i\n",
        pct, (int)rescount, (int)countClusters());

  }

  return ret;
}


//---
//  __  __  ___ __  __  ___
// |  \/  |/ __|  \/  |/ __|
// | |\/| | (__| |\/| | (__
// |_|  |_|\___|_|  |_|\___|
//

int POMS::MCMC(int64_t mcmc_step) {
  return -1;
}

int POMS::MCMCInit() {
  int64_t cell,
          nei_cell;
  int32_t idir,
          tile_idx,
          tile,
          tile_n;
  int32_t src_tile,
          dst_tile;

  int32_t x,y,z,
          boundary_tile = 0;

  for (cell=0; cell<m_cell_count; cell++) {
    tile = irnd( m_tile_count );
    //setCellSize( m_plane, cell, 1 );
    //setCellTile( m_plane, cell, 0, tile );
    forceTile(m_plane, cell, tile);
  }

  m_conflict_count=0;

  for (z=0; z<m_size[2]; z++) {
    for (y=0; y<m_size[1]; y++) {
      for (x=0; x<m_size[0]; x++) {
        cell = xyz2cell(x,y,z);

        src_tile = cellTile( m_plane, cell, 0 );
        for (idir=0; idir<6; idir++) {
          nei_cell = neiCell( cell, idir );

          dst_tile = boundary_tile;
          if (nei_cell>=0) {
            dst_tile = cellTile( m_plane, nei_cell, 0);
          }

          if (F(src_tile, dst_tile, idir) <= m_zero) {
            m_conflict_count++;
          }
        }

      }
    }
  }

  return 0;
}

int POMS::MCMCBegin() { return 0; }

int POMS::MCMCStep() {
  int64_t cell,
          nei_cell;
  int32_t idir,
          tile_nei,
          tile_orig,
          tile_attempt;

  int32_t conflict_orig=0,
          conflict_attempt=0;
  double E_orig,
         E_attempt;

  int32_t boundary_tile = 0;

  cell = (int64_t)irnd( m_cell_count );
  tile_orig = cellTile( m_plane, cell, 0 );
  tile_attempt = irnd( m_tile_count );

  for (idir=0; idir<6; idir++) {
    nei_cell = neiCell( cell, idir );

    tile_nei = boundary_tile;
    if (nei_cell >= 0) {
      tile_nei = cellTile( m_plane, nei_cell, 0 );
    }

    if (F(tile_orig, tile_nei, idir) < m_zero) {
      conflict_orig++;
    }

    if (F(tile_attempt, tile_nei, idir) < m_zero) {
      conflict_attempt++;
    }

  }

  // accept
  //
  if ( (conflict_attempt <= conflict_orig) ||
       (rnd() < exp( m_beta * ((double)(conflict_attempt - conflict_orig)) )) ) {
    m_conflict_count += (conflict_attempt - conflict_orig);
    setCellTile( m_plane, cell, 0, tile_attempt );
  }

  if (m_conflict_count==0) { return 0; }
  return 1;
}

int POMS::MCMCEnd() {
  if (m_conflict_count==0) { return 0; }
  return 1;
}

//---

int POMS::execPhase(int32_t phase) {
  int r;

  switch (phase) {
    case POMS_PHASE_CREATE:
      return 0;
      break;

    case POMS_PHASE_PREFATORY:
      r = applyConstraints();
      if (r<0) { return r; }
      r = cullSweep();
      if (r<0) { return r; }
      r = savePrefatory();
      if (r<0) { return r; }
      break;

    case POMS_PHASE_START:
      r = applyStartConstraints();
      if (r<0) { return r; }
      r = cullSweep();
      if (r<0) { return r; }
      break;

    case POMS_PHASE_INIT:
      // r = algInit();
      // if (r<0) { return r; }
      return -1;
      break;

    case POMS_PHASE_STEP_BEGIN:
      // r = algBegin();
      // if (r<0) { return r; }
      return -1;
      break;

    case POMS_PHASE_STEP_TICK:
      // r = algStep();
      // if (r<0) { return r; }
      return -1;
      break;

    case POMS_PHASE_STEP_END:
      // r = algEnd();
      // if (r<0) { return r; }
      return -1;
      break;

    case POMS_PHASE_FINISH:
      // r = algFinish();
      // if (r<0) { return r; }
      return -1;
      break;

    case POMS_PHASE_UNDEF:
    default:
      return -1;
      break;
  }

  return 0;
}

int POMS::copyState(POMS &src_poms) {
  int i, r;
  size_t _s, _ts;

  _s = m_tile_support_data_size;
  _ts = m_tile_data_size;

  if (src_poms.m_cell_count != m_cell_count) { return -1; }
  if (src_poms.m_tile_count != m_tile_count) { return -1; }

  //memcpy( &(m_prefatory[0]), &(src_poms.m_prefatory[0]), sizeof(int32_t)*src_poms.m_prefatory.size() );
  memcpy( &(m_prefatory[0]), &(src_poms.m_prefatory[0]), _ts*src_poms.m_prefatory.size() );
  //memcpy( &(m_prefatory_size[0]), &(src_poms.m_prefatory_size[0]), sizeof(int32_t)*src_poms.m_prefatory_size.size() );
  memcpy( &(m_prefatory_size[0]), &(src_poms.m_prefatory_size[0]), _ts*src_poms.m_prefatory_size.size() );

  //memcpy( &(m_tile_support[0][0]),
  //        &(src_poms.m_tile_support[0][0]),
  //        _s*src_poms.m_tile_support[0].size() );
  //memcpy( &(m_tile_support[1][0]),
  //        &(src_poms.m_tile_support[1][0]),
  //        _s*src_poms.m_tile_support[1].size() );
  if (m_tile_support_option == POMS_OPTIMIZATION_AC4_NONE) {
    memcpy( &(m_tile_support[0][0]),
            &(src_poms.m_tile_support[0][0]),
            _s*src_poms.m_tile_support[0].size() );
    memcpy( &(m_tile_support[1][0]),
            &(src_poms.m_tile_support[1][0]),
            _s*src_poms.m_tile_support[1].size() );
  }
  else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_FLAT) {
    r = m_ac4_flat[0].set( &(src_poms.m_ac4_flat[0]) );
    if (r<0) { return r; }
    r = m_ac4_flat[1].set( &(src_poms.m_ac4_flat[1]) );
    if (r<0) { return r; }
  }

  else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER4) {
    r = m_ac4_tier4[0].set( &(src_poms.m_ac4_tier4[0]) );
    if (r<0) { return r; }
    r = m_ac4_tier4[1].set( &(src_poms.m_ac4_tier4[1]) );
    if (r<0) { return r; }
  }
  else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER4_M) {
    r = m_ac4_tier4_m1[0].set( &(src_poms.m_ac4_tier4_m1[0]) );
    if (r<0) { return r; }
    r = m_ac4_tier4_m1[1].set( &(src_poms.m_ac4_tier4_m1[1]) );
    if (r<0) { return r; }
  }
  else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER4_M2) {
    r = m_ac4_tier4_m2[0].set( &(src_poms.m_ac4_tier4_m2[0]) );
    if (r<0) { return r; }
    r = m_ac4_tier4_m2[1].set( &(src_poms.m_ac4_tier4_m2[1]) );
    if (r<0) { return r; }
  }

  else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER6) {
    r = m_ac4_tier6[0].set( &(src_poms.m_ac4_tier6[0]) );
    if (r<0) { return r; }
    r = m_ac4_tier6[1].set( &(src_poms.m_ac4_tier6[1]) );
    if (r<0) { return r; }
  }
  else if (m_tile_support_option == POMS_OPTIMIZATION_AC4_TIER6_M) {
    r = m_ac4_tier6_m1[0].set( &(src_poms.m_ac4_tier6_m1[0]) );
    if (r<0) { return r; }
    r = m_ac4_tier6_m1[1].set( &(src_poms.m_ac4_tier6_m1[1]) );
    if (r<0) { return r; }
  }


  //memcpy( &(m_tile[0][0]), &(src_poms.m_tile[0][0]), sizeof(int32_t)*src_poms.m_tile[0].size() );
  //memcpy( &(m_tile[1][0]), &(src_poms.m_tile[1][0]), sizeof(int32_t)*src_poms.m_tile[1].size() );

  memcpy( &(m_tile[0][0]), &(src_poms.m_tile[0][0]), _ts*src_poms.m_tile[0].size() );
  memcpy( &(m_tile[1][0]), &(src_poms.m_tile[1][0]), _ts*src_poms.m_tile[1].size() );

  //memcpy( &(m_tile_size[0][0]), &(src_poms.m_tile_size[0][0]), sizeof(int32_t)*src_poms.m_tile_size[0].size() );
  //memcpy( &(m_tile_size[1][0]), &(src_poms.m_tile_size[1][0]), sizeof(int32_t)*src_poms.m_tile_size[1].size() );

  memcpy( &(m_tile_size[0][0]), &(src_poms.m_tile_size[0][0]), _ts*src_poms.m_tile_size[0].size() );
  memcpy( &(m_tile_size[1][0]), &(src_poms.m_tile_size[1][0]), _ts*src_poms.m_tile_size[1].size() );

  //memcpy( &(m_tile_bp[0][0]), &(src_poms.m_tile_bp[0][0]), sizeof(int32_t)*src_poms.m_tile_bp[0].size() );
  //memcpy( &(m_tile_bp[1][0]), &(src_poms.m_tile_bp[1][0]), sizeof(int32_t)*src_poms.m_tile_bp[1].size() );

  memcpy( &(m_tile_bp[0][0]), &(src_poms.m_tile_bp[0][0]), _ts*src_poms.m_tile_bp[0].size() );
  memcpy( &(m_tile_bp[1][0]), &(src_poms.m_tile_bp[1][0]), _ts*src_poms.m_tile_bp[1].size() );

  m_tileAdj.resize( src_poms.m_tileAdj.size() );
  for (i=0; i<m_tileAdj.size(); i++) { m_tileAdj[i] = src_poms.m_tileAdj[i]; }

  memcpy( &(m_cell_pin[0]), &(src_poms.m_cell_pin[0]), sizeof(int8_t)*m_cell_pin.size() );

  return 0;
}

//
// * soften `soften_block` bounds
// * copy `copy_block` from src to this
// * run AC4InitBlock on `soften_block[][+-1]`
//
// If destination and source cells are fully
// realized and are identical, transfer it
// without softening.
//
// Retrun (AC4InitBlock):
//
//  0 - grid in arc consistent state
// -1 - conflict occured
// 
int POMS::transplantBlock(int32_t soften_block[][2], int32_t copy_block[][2], POMS &src) {
  int32_t xyz,x,y,z,cell;

  int32_t src_orig_tile_n,
          dst_orig_tile_n,
          tile_idx,

          src_tile_n,
          src_tile_idx,
          src_tile,

          dst_tile_n,
          dst_tile_idx,
          dst_tile,

          tile_n,
          tile_val;

  int32_t wblock[3][2];


  // soften without calling ac4initblock as we'll do that
  // after we copy the inner block after we soften.
  //
  for (xyz=0; xyz<3; xyz++) {
    wblock[xyz][0] = ( (copy_block[xyz][0] < soften_block[xyz][0]) ? (copy_block[xyz][0]-1) : (soften_block[xyz][0]-1) );
    wblock[xyz][1] = ( (copy_block[xyz][1] > soften_block[xyz][1]) ? (copy_block[xyz][1]+1) : (soften_block[xyz][1]+1) );

    if (wblock[xyz][0] < 0)           { wblock[xyz][0] = 0; }
    if (wblock[xyz][1] > m_size[xyz]) { wblock[xyz][1] = m_size[xyz]; }
  }

  for (z=soften_block[2][0]; z<soften_block[2][1]; z++) {
    for (y=soften_block[1][0]; y<soften_block[1][1]; y++) {
      for (x=soften_block[0][0]; x<soften_block[0][1]; x++) {

        cell = xyz2cell(x,y,z);

        // We want to keep cells that are fully
        // realized and identical from source
        // and destination
        //
        src_orig_tile_n = src.cellSize(src.m_plane, cell);
        dst_orig_tile_n =     cellSize(    m_plane, cell);

        if ((src_orig_tile_n == 1) &&
            (src_orig_tile_n == dst_orig_tile_n) &&
            (src.cellTile( src.m_plane, cell, 0 ) == cellTile( m_plane, cell, 0 ))) {
          continue;
        }

        // NEEDS UPDATING
        //
        tile_n = cellBufSize( &(m_prefatory_size[0]), cell );
        setCellSize( m_plane, cell, tile_n );

        for (tile_idx=0; tile_idx<tile_n; tile_idx++) {
          tile_val = cellBufTile( &(m_prefatory[0]), cell, tile_idx );
          setCellTile( m_plane, cell, tile_idx, tile_val );
        }

      }
    }
  }

  // copy inner block
  //
  for (z=copy_block[2][0]; z<copy_block[2][1]; z++) {
    for (y=copy_block[1][0]; y<copy_block[1][1]; y++) {
      for (x=copy_block[0][0]; x<copy_block[0][1]; x++) {
        cell = xyz2cell(x,y,z);

        tile_n = src.cellSize(src.m_plane, cell);
        for (tile_idx=0; tile_idx<tile_n; tile_idx++) {
          tile_val = src.cellTile( src.m_plane, cell, tile_idx );
          setCellTile( m_plane, cell, tile_idx, tile_val );
        }
        setCellSize( m_plane, cell, tile_n );

      }
    }
  }

  // There's a potential for speedup here by keeping delta changes instead of
  // re-running ac4 initilization on the whole block, but this is the simple way
  // to do it (profile to make sure there's an actual speedup to be had before
  // optimizing).
  //
  return AC4InitBlock( wblock );
}

//
// * soften `soften_block` bounds
// * copy `copy_block` from src to this
// * run AC4InitBlock on `soften_block[][+-1]`
//
// If the `soften_identical` flag is *not* set, if
// destination and source cells are fully
// realized and are identical, transfer it.
// That is, if `soften_identical` is set,
// completely soften the cell even if source
// and destination are fully realized and
// have identical tile values.
//
// Code style preference is to keep default parameters 0 .
//
// Retrun (AC4InitBlock):
//
//  0 - grid in arc consistent state
// -1 - conflict occured
// 
int POMS::transplantBlock_union(int32_t soften_block[][2], int32_t copy_block[][2], POMS &src) {
  int32_t xyz,x,y,z,cell;

  int32_t src_orig_tile_n,
          dst_orig_tile_n,
          tile_idx,

          src_tile_n,
          src_tile_idx,
          src_tile,

          dst_tile_n,
          dst_tile_idx,
          dst_tile,

          tile_n,
          tile_val;

  int32_t wblock[3][2];


  // soften without calling ac4initblock as we'll do that
  // after we copy the inner block after we soften.
  //
  for (xyz=0; xyz<3; xyz++) {
    wblock[xyz][0] = ( (copy_block[xyz][0] < soften_block[xyz][0]) ? (copy_block[xyz][0]-1) : (soften_block[xyz][0]-1) );
    wblock[xyz][1] = ( (copy_block[xyz][1] > soften_block[xyz][1]) ? (copy_block[xyz][1]+1) : (soften_block[xyz][1]+1) );

    if (wblock[xyz][0] < 0)           { wblock[xyz][0] = 0; }
    if (wblock[xyz][1] > m_size[xyz]) { wblock[xyz][1] = m_size[xyz]; }
  }

  // A very gentle soften of the outer block, by combining source and destination
  // tiles.
  //
  for (z=soften_block[2][0]; z<soften_block[2][1]; z++) {
    for (y=soften_block[1][0]; y<soften_block[1][1]; y++) {
      for (x=soften_block[0][0]; x<soften_block[0][1]; x++) {

        cell = xyz2cell(x,y,z);

        src_orig_tile_n = src.cellSize(src.m_plane, cell);
        for (src_tile_idx=0; src_tile_idx < src_orig_tile_n; src_tile_idx++) {
          src_tile = src.cellTile( src.m_plane, cell, src_tile_idx );
          if (cellHasTile( m_plane, cell, src_tile)) { continue; }

          // NEEDS UPDATING
          //
          dst_tile_n = cellSize( m_plane, cell );
          setCellTile( m_plane, cell, dst_tile_n, src_tile );
          setCellSize( m_plane, cell, dst_tile_n+1 );
        }

      }
    }
  }

  // copy inner block
  //
  for (z=copy_block[2][0]; z<copy_block[2][1]; z++) {
    for (y=copy_block[1][0]; y<copy_block[1][1]; y++) {
      for (x=copy_block[0][0]; x<copy_block[0][1]; x++) {
        cell = xyz2cell(x,y,z);

        tile_n = src.cellSize(src.m_plane, cell);
        for (tile_idx=0; tile_idx<tile_n; tile_idx++) {
          tile_val = src.cellTile( src.m_plane, cell, tile_idx );
          setCellTile( m_plane, cell, tile_idx, tile_val );
        }
        setCellSize( m_plane, cell, tile_n );

      }
    }
  }

  // There's a potential for speedup here by keeping delta changes instead of
  // re-running ac4 initilization on the whole block, but this is the simple way
  // to do it (profile to make sure there's an actual speedup to be had before
  // optimizing).
  //
  return AC4InitBlock( wblock );
}

//
// * soften `soften_block` bounds
// * copy `copy_block` from src to this
// * run AC4InitBlock on `soften_block[][+-1]`
//
// Retrun (AC4InitBlock):
//
//  0 - grid in arc consistent state
// -1 - conflict occured
// 
int POMS::transplantBlock_hard(int32_t soften_block[][2], int32_t copy_block[][2], POMS &src) {
  int32_t xyz,x,y,z,cell;

  int32_t src_orig_tile_n,
          dst_orig_tile_n;

  int32_t tile_idx, tile_n, tile_val;
  int32_t wblock[3][2];


  // soften without calling ac4initblock as we'll do that
  // after we copy the inner block after we soften.
  //
  for (xyz=0; xyz<3; xyz++) {
    wblock[xyz][0] = ( (copy_block[xyz][0] < soften_block[xyz][0]) ? (copy_block[xyz][0]-1) : (soften_block[xyz][0]-1) );
    wblock[xyz][1] = ( (copy_block[xyz][1] > soften_block[xyz][1]) ? (copy_block[xyz][1]+1) : (soften_block[xyz][1]+1) );

    if (wblock[xyz][0] < 0)           { wblock[xyz][0] = 0; }
    if (wblock[xyz][1] > m_size[xyz]) { wblock[xyz][1] = m_size[xyz]; }
  }

  // Soften outer block, skipping identically realized cells if appropriate
  //
  for (z=soften_block[2][0]; z<soften_block[2][1]; z++) {
    for (y=soften_block[1][0]; y<soften_block[1][1]; y++) {
      for (x=soften_block[0][0]; x<soften_block[0][1]; x++) {

        cell = xyz2cell(x,y,z);

        /*
        // if not soften identical, we want to keep
        // cells that are fully realized and identical
        // from source and destination
        //
        if (!soften_identical) {
          src_orig_tile_n = src.cellSize(src.m_plane, cell);
          dst_orig_tile_n =     cellSize(    m_plane, cell);

          if ((src_orig_tile_n == 1) &&
              (src_orig_tile_n == dst_orig_tile_n) &&
              (src.cellTile( src.m_plane, cell, 0 ) == cellTile( m_plane, cell, 0 ))) {
            continue;
          }
        }
        */


        // NEEDS UPDATING

        tile_n = cellBufSize( &(m_prefatory_size[0]), cell );
        setCellSize( m_plane, cell, tile_n );

        for (tile_idx=0; tile_idx<tile_n; tile_idx++) {
          tile_val = cellBufTile( &(m_prefatory[0]), cell, tile_idx );
          setCellTile( m_plane, cell, tile_idx, tile_val );
        }

      }
    }
  }

  // copy inner block
  //
  for (z=copy_block[2][0]; z<copy_block[2][1]; z++) {
    for (y=copy_block[1][0]; y<copy_block[1][1]; y++) {
      for (x=copy_block[0][0]; x<copy_block[0][1]; x++) {
        cell = xyz2cell(x,y,z);

        tile_n = src.cellSize(src.m_plane, cell);
        for (tile_idx=0; tile_idx<tile_n; tile_idx++) {
          tile_val = src.cellTile( src.m_plane, cell, tile_idx );
          setCellTile( m_plane, cell, tile_idx, tile_val );
        }
        setCellSize( m_plane, cell, tile_n );

      }
    }
  }

  // There's a potential for speedup here by keeping delta changes instead of
  // re-running ac4 initilization on the whole block, but this is the simple way
  // to do it (profile to make sure there's an actual speedup to be had before
  // optimizing).
  //
  return AC4InitBlock( wblock );
}

//----
//----

int64_t POMS::quiltResolvedCount(void) {
  int64_t cell, n=0;

  for (cell=0; cell<m_quilt_cell_count; cell++) {
    if (m_quilt_pin[cell] != 0) { n++; continue; }
    if (m_quilt_tile[cell] < 0) { continue; }
    n++;
  }

  return n;
}

// transfer current region into `m_quilt_tile`
//
// `m_patch_region` holds the current patch region in `m_quilt_tile`.
//
// If cells in current region aren't resolve, don't copy them into
// the quilt.
//
// we assume patch region has the same size as m_size.
// If we want to work on a smaller area, we can do this
// with various pinning techniquse to force an already processed
// portion to stay the same while working on some other portion
// of the match.
//
// return:
//
//  0 : success
// !0 : error (size mismatch)
//
int POMS::saveQuiltPatchRegion(void) {
  int32_t x,y,z, p[3];
  int64_t cell;

  int32_t qx,qy,qz, q[3],
          dqx, dqy, dqz;
  int64_t qcell;

  dqx = m_patch_region[0][1] - m_patch_region[0][0];
  dqy = m_patch_region[1][1] - m_patch_region[1][0];
  dqz = m_patch_region[2][1] - m_patch_region[2][0];

  if ((dqx != m_size[0]) ||
      (dqy != m_size[1]) ||
      (dqz != m_size[2])) {
    return -1;
  }

  for (cell=0; cell<m_cell_count; cell++) {

    cell2vec(p, cell);
    q[0] = p[0] + m_patch_region[0][0];
    q[1] = p[1] + m_patch_region[1][0];
    q[2] = p[2] + m_patch_region[2][0];
    qcell = vec2cell(q, m_quilt_size);

    if (m_quilt_pin[qcell] != 0) { continue; }
    if (cellSize(m_plane, cell) != 1) { continue; }

    m_quilt_tile[qcell] = cellTile(m_plane, cell, 0);
  }

  return 0;
}

int POMS::sanityQuilt(void) {
  int32_t x,y,z, u[3], v[3];
  int64_t cell, nei_cell;
  int32_t idir, rdir;
  int16_t boundary_tile=0;

  for (cell=0; cell<m_quilt_cell_count; cell++) {

    if (m_quilt_tile[cell] < 0) { continue; }
    if (m_quilt_pin[cell] != 0) { continue; }

    for (idir=0; idir<6; idir++) {
      nei_cell = neiCell(cell, idir, m_quilt_size);
      if ((nei_cell >= 0) &&
          (m_quilt_pin[cell] != 0)) { continue; }

      if (nei_cell < 0) {
        if (F(m_quilt_tile[cell], boundary_tile, idir) < m_zero) {

          printf("sanityQuilt: cell:%i -(%i)-> boundary:%i: %f (b)\n",
              (int)cell, (int)idir, (int)boundary_tile, F(m_quilt_tile[cell], boundary_tile, idir));

          return -1;
        }

        /*
        if (F(boundary_tile, m_quilt_tile[cell], rdir) < m_zero) {
          
          printf("sanityQuilt: cell:%i <-(%i)- boundary:%i: %f\n",
              (int)cell,
              (int)rdir,
              (int)boundary_tile,
              F(boundary_tile, m_quilt_tile[cell], rdir));

          return -1; }
          */

        continue;
      }
      if (m_quilt_tile[nei_cell] < 0) { continue; }

      rdir = m_dir_oppo[idir];

      if (F(m_quilt_tile[cell], m_quilt_tile[nei_cell], idir) < m_zero) {

        cell2vec(u, cell, m_quilt_size);
        cell2vec(v, nei_cell, m_quilt_size);

        printf("sanityQuilt: cell:%i[%i,%i,%i]{%i} -(%i)-> nei:%i[%i,%i,%i]{%i}: %f (1)\n",
            (int)cell, u[0], u[1], u[2], (int)m_quilt_tile[cell],
            (int)idir,
            (int)nei_cell, v[0], v[1], v[2], (int)m_quilt_tile[nei_cell],
            F(m_quilt_tile[cell], m_quilt_tile[nei_cell], idir));


       
        return -1; }
      if (F(m_quilt_tile[nei_cell], m_quilt_tile[cell], rdir) < m_zero) {

        printf("sanityQuilt: cell:%i <-(%i)- nei:%i: %f (2)\n",
            (int)cell, (int)idir, (int)nei_cell, F(m_quilt_tile[nei_cell], m_quilt_tile[cell], rdir));

  
        return -1; }

    }
  }

  return 0;
}

// this will setup a new problem region to process by
// using the `m_patch_region` to pin cells appropriately
//
int POMS::setupQuiltPatch(void) {
  int r;

  int32_t x,y,z, p[3];
  int64_t cell;

  int32_t qx,qy,qz, q[3], q_nei[3];
  int64_t qcell, qcell_nei;

  int32_t xyz, idir;

  int32_t _t[3];

  int code;

  int32_t print_order[3] = {1,2,0};

  renew();

  r = refreshConstraints();
  if (r<0) { return r; }

  memset( &(m_cell_pin[0]), 0, sizeof(int8_t)*m_cell_count);
  resetAC4Dirty(m_plane);

  r = applyConstraints();
  if (r<0) { return r; }

  r = applyQuiltConstraints();
  if (r<0) { return r; }


  if (m_verbose >= POMS_VERBOSE_RUN) {
    printf("## setupQuiltPatch: m_patch_region[%i:%i,%i:%i,%i:%i]\n",
        (int)m_patch_region[0][0], (int)m_patch_region[0][1],
        (int)m_patch_region[1][0], (int)m_patch_region[1][1],
        (int)m_patch_region[2][0], (int)m_patch_region[2][1]);
  }

  // setup quilt patch, pinning sides if not on boundary
  //
  for (cell=0; cell<m_cell_count; cell++) {

    cell2vec(p, cell);

    q[0] = p[0] + m_patch_region[0][0];
    q[1] = p[1] + m_patch_region[1][0];
    q[2] = p[2] + m_patch_region[2][0];

    qcell = vec2cell(q, m_quilt_size);

    if (m_quilt_pin[qcell] != 0) {
      m_cell_pin[cell] = 1;
    }

    //TODO: add different sized stitch values (currently 1)
    //
    // if it's on the border of the working region
    // and it's not on the border of the quilt,
    //   then pin it, as it'll need to be stitched into
    //   a neighboring region
    //

    // working region boundary test
    //
    if ((p[0] == 0) || (p[0] == (m_size[0]-1)) ||
        (p[1] == 0) || (p[1] == (m_size[1]-1)) ||
        (p[2] == 0) || (p[2] == (m_size[2]-1))) {

      // look at all neighbors and if is outside of
      // working region
      //
      for (idir=0; idir<6; idir++) {

        // neighbor in current patch
        //
        if (neiCell(cell, idir) >= 0) { continue; }

        // it's on the working region boundary but also
        // on the quilt boundary, so don't explicitly pin
        // it. If the quilt constraints have it pinned,
        // it will be caught above.
        //
        qcell_nei = neiCell(qcell, idir, m_quilt_size);
        if (qcell_nei < 0) { continue; }

        m_cell_pin[cell] = 1;

        break;
      }

      if (m_cell_pin[cell]) {
        if (m_quilt_tile[qcell] >= 0) {
          r = forceTile(m_plane, cell, m_quilt_tile[qcell]);
          if (r<0) { return r; }
        }
      }


    }

  }

  //--

  r = AC4Init();
  if (r<0) {

    if (m_verbose >= POMS_VERBOSE_RUN) {
      printf("# setupQuiltPatch: AC4Init error (%i), ", r);
      printDebugConflict();
      printf("\n");
    }

    return r;
  }

  r = savePrefatory();
  if (r<0) { return r; }

  m_phase = POMS_PHASE_PREFATORY;

  //--

  // this might not even be appropriate for quilting...
  //
  if (m_constraint_start_count>0) {
    r = applyStartConstraints();
    if (r<0) { return r; }

    r = AC4Init();
    if (r<0) { return r; }
  }

  m_phase = POMS_PHASE_START;

  return 0;
}



