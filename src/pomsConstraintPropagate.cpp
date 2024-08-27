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

// Remove tile from cell position (index).
//
// Return:
//
//  0 - removed tile from cell (valid if 0 tiles left @ cell)
// <0 - cell size is 0 before attempted removal
//
int POMS::removeTileIdx( int plane, int64_t cell, int32_t tile_idx ) {

  int32_t tile_keep,
          tile_discard,
          n;

  n = cellSize( plane, cell );
  if (n==0) { return -1; }

  tile_discard  = cellTile( plane, cell, tile_idx );
  tile_keep     = cellTile( plane, cell, n-1 );
  setCellTile( plane, cell, tile_idx, tile_keep );
  setCellTile( plane, cell, n-1, tile_discard );
  setCellSize( plane, cell, n-1 );

  return 0;
}

// Remove tile from cell.
//
// Return:
//
//  0 - tile removed (success)
// <0 - cell size is 0 before attempted removal
// >0 - tile not found, no action taken
//
int POMS::removeTile( int plane, int64_t cell, int32_t tile ) {
  int32_t tile_idx,
          tile_n,
          tile_tmp;
  int r;

  tile_idx = cellTileIndex(plane, cell, tile);
  if ((tile_idx < 0) || (tile_idx >= cellSize(plane, cell))) { return -1; }
  if ( cellTile(plane, cell, tile_idx) != tile )  { return -2; }
  return removeTileIdx(plane, cell, tile_idx);
}



// vestigial ac3 helper functions?
//
// queue cell to m_cell_queue, setting m_visited
// If cell already queued, do nothing
//
// return:
//
// >0 - cell already visited, ignoring
//  0 - cell queued
// <0 - error
//
int POMS::queueCell( int64_t cell ) {
  if (m_visited[m_plane][cell]) { return 1; }

  if (m_cell_queue_size[m_plane] >= m_cell_count) {

    printf("SANITY ERROR queueCell trying to enqueue past m_cell_count %i\n",
        (int)m_cell_count);

    return -1;
  }

  m_visited[m_plane][cell] = 1;
  m_cell_queue[m_plane][ m_cell_queue_size[m_plane] ] = cell;
  m_cell_queue_size[m_plane]++;

  return 0;
}

// queue cell neighbors
//
int POMS::queueCellNeighbors( int64_t cell ) {
  int32_t idir;
  int64_t nei_cell;
  int r;

  for (idir=0; idir<6; idir++) {
    nei_cell = neiCell( cell, idir );
    if (nei_cell < 0) { continue; }
    r = queueCell( nei_cell );
    if (r<0) { return r; }
  }

  return 0;
}

// only unwind the visited
//
int POMS::unwindVisited(void) {
  int64_t idx,
          cell;
  for (idx=0; idx<m_cell_queue_size[m_plane]; idx++) {
    cell = m_cell_queue[m_plane][idx];
    m_visited[m_plane][cell] = 0;
  }
  return 0;
}

// To avoid a full blank of visited,
// go through individual cells to
// blank out m_visited entries.
//
int POMS::unwindQueue(void) {
  int64_t idx,
          cell;
  int32_t tile,
          tile_idx;

  for (idx=0; idx<m_cell_queue_size[m_plane]; idx++) {
    cell = m_cell_queue[m_plane][idx];
    m_visited[m_plane][cell] = 0;
  }

  m_cell_queue_size[m_plane]=0;

  return 0;
}

//---------
//---------
//---------

//int POMS::cullSweep_ac4() { return AC4Init(); }
//int POMS::cullSweep()     { return cullSweep_ac4(); }
int POMS::cullSweep()     { return AC4Init(); }

//---------
//---------
//---------

// do a sanity check on the the m_cell_tile_queue and m_cell_tile_visited structures:
//
// make sure every (cell,tile) pair in m_cell_tile_queue has its visited flag set
// in m_cell_tile_visited.
//
// make sure the count of set m_cell_tile_visited matches the queue size
//
// Only works on m_plane.
//
// Returns:
//
//  0 - sanity ok
// -1 - queue size not even
// -2 - visited flag not set
// -3 - visited count doesn't match queue element count
//
int POMS::sanityCellTileQueueVisited() {
  int64_t cell, idx, qpos, n_queue;
  int32_t tile;
  int64_t count=0;

  n_queue = cellTileQueueSize(m_plane);
  if ((n_queue % 2) == 1) { return -1; }
  for (qpos=0; qpos<n_queue; qpos+=2) {
    cellTileQueuePeek(m_plane, qpos, &cell, &tile);
    if (cellTileVisited(m_plane, cell, tile)==0) { return -2; }
  }

  for (cell=0; cell<m_cell_count; cell++) {
    for (tile=0; tile<m_tile_count; tile++) {
      if (cellTileVisited(m_plane, cell, tile) == 0) { continue; }
      count++;
    }
  }

  if (count != (qpos/2)) { return -3; }

  return 0;
}

// do a sweep to confirm arc consistency
//
// Return:
//
//  0 - map is arc consistent
// -1 - a conflict exists in the graph
//
int POMS::sanityArcConsistency() {
  int64_t src_cell,
          nei_cell;
  int32_t src_tile_idx,
          src_n_tile,
          src_tile,

          nei_tile_idx,
          nei_n_tile,
          nei_tile,

          tile_valid,
          boundary_tile = 0,
          idir;

  for (src_cell=0; src_cell < m_cell_count; src_cell++) {

    if (m_cell_pin[src_cell] != 0) { continue; }

    for (idir=0; idir<6; idir++) {

      nei_cell = neiCell( src_cell, idir );
      src_n_tile = cellSize( m_plane, src_cell );
      for (src_tile_idx=0; src_tile_idx < src_n_tile; src_tile_idx++) {

        src_tile = cellTile( m_plane, src_cell, src_tile_idx );

        // hard coded boundary tile for now.
        // if rule from current cell to boundary tile
        // in direction is invalid, not arc consistent
        //
        if (nei_cell < 0) {
          if ( F(src_tile, 0, idir) < m_zero ) {
            m_conflict_cell = src_cell;
            m_conflict_tile = src_tile;
            m_conflict_idir = idir;
            m_conflict_type = POMS_CONFLICT_BOUNDARY;
            return -1;
          }
          continue;
        }

        tile_valid = 0;

        // loop through neighbor tiles.
        // If we find at least one valid connection from
        // source tile to neighbor, arc concsistent
        // (for thet direction, source cell and tile)
        //

        nei_n_tile = cellSize( m_plane, nei_cell );
        for (nei_tile_idx=0; nei_tile_idx < nei_n_tile; nei_tile_idx++) {
          nei_tile = cellTile( m_plane, nei_cell, nei_tile_idx );
          if ( F(src_tile, nei_tile, idir) > m_zero ) {
            tile_valid = 1;
            break;
          }
        }

        if (tile_valid==0) {
          m_conflict_cell = src_cell;
          m_conflict_tile = src_tile;
          m_conflict_idir = idir;
          m_conflict_type = POMS_CONFLICT_NO_SUPPORT;
          return -2;
        }
      }
    }
  }

  return 0;
}

int POMS::AC4Consistency() {
  int64_t src_cell,
          nei_cell;
  int32_t src_tile_n,
          nei_tile_n,
          src_tile_idx,
          nei_tile_idx,
          src_tile,
          nei_tile;
  int32_t boundary_tile = 0,
          support_count=-1;
  int32_t idir, rdir;

  for (src_cell=0; src_cell<m_cell_count; src_cell++) {

    if (m_cell_pin[src_cell] != 0) { continue; }

    src_tile_n = cellSize(m_plane, src_cell);
    for (src_tile_idx=0; src_tile_idx<src_tile_n; src_tile_idx++) {
      src_tile = cellTile(m_plane, src_cell, src_tile_idx);

      for (idir=0; idir<6; idir++) {
        support_count=0;

        nei_cell = neiCell(src_cell, idir);
        rdir = m_dir_oppo[idir];

        if (nei_cell<0) {
          if (F(boundary_tile, src_tile, rdir) > m_zero) {
            support_count++;
          }
        }
        else {
          nei_tile_n = cellSize(m_plane, nei_cell);
          for (nei_tile_idx=0; nei_tile_idx<nei_tile_n; nei_tile_idx++) {
            nei_tile = cellTile(m_plane, nei_cell, nei_tile_idx);

            if (F(nei_tile,src_tile, rdir) > m_zero) { support_count++; }
          }
        }

        if (tileSupport(m_plane, idir, src_cell, src_tile) != support_count) {
          m_conflict_cell = src_cell;
          m_conflict_tile = src_tile;
          m_conflict_idir = idir;
          m_conflict_type = POMS_CONFLICT_MISMATCH;
          return -1;
        }

      }

    }

  }

  return 0;
}

// `m_cell_tile_queue` and `m_cell_tile_queue_size` should
// be populated with the `(cell,tile)` pairs to be removed.
//
// For every `(cell,tile)` pair removed, remove it's support
// for neighboring `(neiCell,neiTile)` pairs. If any neighboring
// `(neiCell,neiTile)` pair's support goes to 0 (in any of
// the directions), add it to the queue.
//
// `m_cell_tile_visited` structures is used to not double queue
// `(cell,tile)` pairs.
//
// Return:
//
//   0 - queue processed, all nodes considered in an arc consistent state
//  -1 - contradiction occured (cell has 0 support)
// <-1 - sanity error (shouldn't happen)
//
// the `m_conflict_*` variables will be populated accordingly in
// the case of a conflict.
//
int POMS::AC4Update(void) {
  int64_t src_cell,
          nei_cell,
          pos,
          nei_pos,
          qpos;
  int32_t src_tile_n,
          src_tile_idx,
          src_tile,
          nei_tile_n,
          nei_tile_idx,
          nei_tile,
          idir,
          rdir;
  int32_t ii,
          supported_tile = 0,
          _tile_support = 0;

  int32_t v[3], u[3];
  int32_t _v[3], _u[3];

  int32_t __tmpxyz[3];

  int32_t nei_v_n,
          nei_v_idx;

  int r;

  _prof_start(POMS_PROF_AC4UPDATE);

  while (cellTileQueueSize(m_plane) > 0) {

    r = cellTileQueuePop(m_plane, &src_cell, &src_tile);
    if (r<0) { return -5; }

    cellTileVisited(m_plane, src_cell, src_tile, 0);

    // if tile has at least one direction with no support, we need to remove it
    //
    supported_tile = 1;
    for (idir=0; idir<6; idir++) {
      if (tileSupport(m_plane, idir, src_cell, src_tile) == 0) {
        supported_tile = 0;
        break;
      }
    }
    if (supported_tile) { continue; }

    // Go through each direction, updating neighboring support counts.
    // If the neighboring tile is off the boundary, we can skip over it.
    //
    for (idir=0; idir<6; idir++) {
      nei_cell = neiCell(src_cell, idir);
      if (nei_cell < 0) { continue; }

      if (m_cell_pin[nei_cell] != 0) { continue; }

      nei_tile_n  = cellSize(m_plane, nei_cell);
      nei_v_n     = tileAdjIdxSize(src_tile, idir);

      rdir        = m_dir_oppo[idir];

      if (nei_tile_n < nei_v_n) {

        for (nei_tile_idx=0; nei_tile_idx<nei_tile_n; nei_tile_idx++) {

          nei_tile  = cellTile(m_plane, nei_cell, nei_tile_idx);

          // no connection, needs no update, skip
          //
          if ( F(src_tile, nei_tile, idir) < m_zero ) { continue; }

          // Negative tile support should only happen if the ac4 tile
          // support counts have got mangled. I don't see a legitimate
          // case for thie to be true.
          // Things can go haywire if tile support goes negative
          // and its a simple enough check to do to ensure sanity,
          // so its kept in.
          //
          _tile_support = tileSupport(m_plane, rdir, nei_cell, nei_tile);
          if (_tile_support == 0) {

            cell2vec(v, nei_cell);
            cell2vec(u, src_cell);

            m_conflict_idir = rdir;
            m_conflict_cell = nei_cell;
            m_conflict_tile = nei_tile;
            m_conflict_type = POMS_CONFLICT_NEGATIVE_SUPPORT_SANITY_ERROR;

            return -2;
          }

          _tile_support--;
          tileSupport(m_plane, rdir, nei_cell, nei_tile, _tile_support);
          markAC4Dirty(m_plane, nei_cell);

          // reduce support from direction, adding it to queue if it goes to zero
          //
          if (_tile_support == 0) {

            if (cellTileVisited(m_plane, nei_cell, nei_tile) == 0) {
              cellTileVisited(m_plane, nei_cell, nei_tile, 1);
              cellTileQueuePush(m_plane, nei_cell, nei_tile);
            }

          }

        }

      }

      // nei_v_n <= nei_tile_n
      //
      else {

        // here, we only consider valid tile to tile connections.
        // We iteration through the possible tile neighbors, in
        // the appropriate idir, skipping over any tiles
        // that don't exist in the neighboring cell.
        //
        // If it exists in the neighboring cell, we decrement
        // the support and update accordingly.
        //
        for (nei_v_idx=0; nei_v_idx<nei_v_n; nei_v_idx++) {

          nei_tile = tileAdjIdx( src_tile, idir, nei_v_idx );

          r = cellHasTile( m_plane, nei_cell, nei_tile );
          if (r == 0) { continue; }

          //if (r <  0) { return r; }
          if (r <  0) { return -10; }

          _tile_support = tileSupport(m_plane, rdir, nei_cell, nei_tile);
          if (_tile_support==0) {

            m_conflict_idir = rdir;
            m_conflict_cell = nei_cell;
            m_conflict_tile = nei_tile;
            m_conflict_type = POMS_CONFLICT_NEGATIVE_SUPPORT_SANITY_ERROR;

            return -3;
          }

          _tile_support--;
          tileSupport(m_plane, rdir, nei_cell, nei_tile, _tile_support);

          markAC4Dirty(m_plane, nei_cell);

          if (_tile_support == 0) {
            if (cellTileVisited(m_plane, nei_cell, nei_tile) == 0) {
              cellTileVisited(m_plane, nei_cell, nei_tile, 1);
              cellTileQueuePush(m_plane, nei_cell, nei_tile);
            }
          }


        }

      }

    }

    // once we're done processing the (cell,tile) pair from
    // the queue, remove the tile from the cell.
    //
    removeTile( m_plane, src_cell, src_tile );
    markAC4Dirty(m_plane, src_cell);

    if (m_verbose >= POMS_VERBOSE_DEBUG) {

      cell2vec(__tmpxyz, src_cell);

      printf("AC4Update: remove tile:%i(%s)@cell:%i(%i,%i,%i) support{%i,%i, %i,%i, %i,%i}\n",
          (int)src_tile,
          m_tile_name[src_tile].c_str(),
          (int)src_cell,
          (int)__tmpxyz[0], (int)__tmpxyz[1], (int)__tmpxyz[2],
          tileSupport(m_plane, 0, src_cell, src_tile),
          tileSupport(m_plane, 1, src_cell, src_tile),
          tileSupport(m_plane, 2, src_cell, src_tile),
          tileSupport(m_plane, 3, src_cell, src_tile),
          tileSupport(m_plane, 4, src_cell, src_tile),
          tileSupport(m_plane, 5, src_cell, src_tile)
          );
    }

    // contradiction...
    //
    if (cellSize(m_plane, src_cell) == 0) {

      if (m_verbose >= POMS_VERBOSE_DEBUG) {
        cell2vec(_v, src_cell);
        printf("AC4Update: conflict @cell:[%i,%i,%i](%i)\n",
            _v[0], _v[1], _v[2],
            (int)src_cell);
      }

      m_conflict_cell = src_cell;
      m_conflict_tile = src_tile;
      m_conflict_idir = -1;
      m_conflict_type = POMS_CONFLICT_NO_SUPPORT;
      return -1;
    }

  }

  _prof_end(POMS_PROF_AC4UPDATE);

  return 0;
}

// Initialize the AC4 tile support structure restricted to a block.
// Block bounds need not be restricted to the grid as this will
// skip invalid cell positions.
//
// Return:
//
//  0 - ac4 structure initialized and all tiles have support
// -1 - at least one conflict encountered
//
// On error, the m_confict_... variables will be populated
//
int POMS::AC4InitBlock(int32_t block[][2], int memoize_opt) {
  int64_t src_cell,
          nei_cell,
          idx,
          pos,
          qpos;
  int32_t src_tile_idx,
          src_tile,
          src_tile_n,

          nei_tile_idx,
          nei_tile,
          nei_tile_n;
  int32_t idir,
          rdir,
          boundary_tile=0,
          support_count=0;
  int32_t nei_v_n,
          nei_v_idx;
  int ret=-1, r;

  int32_t x,y,z;

  int32_t _u[3]={0}, _v[3] = {0}, _r=-1;

  int64_t boyer_moore_c,
          _memoize_uid=-1,
          _counter=0;

  // To make sure we're starting in a fresh state,
  // zero out visited array and queue structure.
  // This can be sped up if need be but doing the naive
  // thing for now until it's shown to actually matter
  // for runtime.
  //
  cellTileVisitedClear(m_plane);
  cellTileQueueClear(m_plane);

  if (m_verbose >= POMS_VERBOSE_DEBUG) {
    printf("# AC4InitBlock: init (qsize:%i) (_counter:%i) (queue sanity: %i, sanity ac: %i)\n",
        (int)cellTileQueueSize(m_plane), (int)_counter,
        sanityCellTileQueueVisited(), sanityArcConsistency());
  }


  // boyer moore max freq. algorithm.
  // This will only reliably pick out an element
  // that occurs with freq > N/2.
  // Since this is a heuristic optimization,
  // we don't care if it picks a random value in
  // the case there's not an element that appears greater
  // than half the time.
  //
  // Pick out cell uid that has some high frequency and
  // use that as the basis for the memoize optimization
  // below. src and nei cell have to have same uid, chosen
  // here, in order to re-use the value.
  //
  /*
  if (memoize_opt) {
    boyer_moore_c = 0;
    _memoize_uid = -1;
    for (src_cell=0; src_cell<m_cell_count; src_cell++) {
      if (m_cell_uid[src_cell] <= 0) { continue; }

      if (boyer_moore_c == 0) {
        _memoize_uid = m_cell_uid[src_cell];
      }
      else if (_memoize_uid == m_cell_uid[src_cell]) {
        boyer_moore_c++;
      }
      else {
        boyer_moore_c--;
      }

    }
  }
  */

  // populate tile support
  //
  for (z=block[2][0]; z<block[2][1]; z++) {
    for (y=block[1][0]; y<block[1][1]; y++) {
      for (x=block[0][0]; x<block[0][1]; x++) {
        src_cell = xyz2cell(x,y,z);

        if (src_cell < 0) { continue; }

        if (m_cell_pin[src_cell] != 0) { continue; }

        src_tile_n = cellSize(m_plane, src_cell);
        for (src_tile_idx=0; src_tile_idx<src_tile_n; src_tile_idx++) {
          src_tile = cellTile(m_plane, src_cell, src_tile_idx);

          for (idir=0; idir<6; idir++) {

            support_count=0;
            nei_cell = neiCell(src_cell, idir);

            rdir = m_dir_oppo[idir];

            /*
            if (memoize_opt) {

              // optimization to try and take advantage of homogenous initial
              // state
              //
              if ( (m_cell_pin[nei_cell] == 0) &&
                   (m_cell_uid[src_cell] > 0) &&
                   (m_cell_uid[nei_cell] > 0) &&
                   (m_cell_uid[src_cell] == _memoize_uid) &&
                   (m_cell_uid[nei_cell] == _memoize_uid) &&
                   (m_ac4_memoize[idir][src_tile] >= 0) ) {

                tileSupport(m_plane, idir, src_cell, src_tile, m_ac4_memoize[idir][src_tile]);
                markAC4Dirty(m_plane, src_cell);
                _counter++;
                continue;
              }

            }
            */

            // connection on boundary, do appropriate checks and
            // continue on as there's no more work to be done on this
            // tile
            //
            if (nei_cell<0) {
              if (F(boundary_tile, src_tile, rdir) > m_zero) { support_count++; }
              tileSupport(m_plane, idir, src_cell, src_tile, support_count);
              markAC4Dirty(m_plane, src_cell);
              continue;
            }

            nei_tile_n = cellSize(m_plane, nei_cell);
            nei_v_n = tileAdjIdxSize(src_tile, idir);

            // "unoptimized" case, just cycle through tiles in neighbor cell.
            //
            if (nei_tile_n < nei_v_n) {

              for (nei_tile_idx=0; nei_tile_idx<nei_tile_n; nei_tile_idx++) {
                nei_tile = cellTile(m_plane, nei_cell, nei_tile_idx);

                if ( F(nei_tile, src_tile, rdir) > m_zero ) {
                  support_count++;
                }
              }

              tileSupport(m_plane, idir, src_cell, src_tile, support_count);
              markAC4Dirty(m_plane, src_cell);

              /*
              if (memoize_opt) {
                if ((m_cell_uid[src_cell] == _memoize_uid) &&
                    (m_cell_uid[nei_cell] == _memoize_uid) &&
                    (m_ac4_memoize[idir][src_tile] < 0)) {
                  m_ac4_memoize[idir][src_tile] = support_count;
                }
              }
              */

              continue;
            }

            // otherwise, nei_v_n <= nei_tile_n
            // "optimized" case.
            // cycle through F neighbors of the tile, check to
            // see if they're in the neighboring cell
            //
            for (nei_v_idx=0; nei_v_idx<nei_v_n; nei_v_idx++) {
              nei_tile = tileAdjIdx( src_tile, idir, nei_v_idx );
              r = cellHasTile( m_plane, nei_cell, nei_tile );
              if (r == 0) { continue; }
              if (r <  0) { return r; }

              support_count++;
            }


            tileSupport(m_plane, idir, src_cell, src_tile, support_count);
            markAC4Dirty(m_plane, src_cell);

            /*
            if (memoize_opt) {
              if ((m_cell_uid[src_cell] == _memoize_uid) &&
                  (m_cell_uid[nei_cell] == _memoize_uid) &&
                  (m_ac4_memoize[idir][src_tile] < 0)) {
                m_ac4_memoize[idir][src_tile] = support_count;
              }
            }
            */

          } // for idir
        }   /// for src_tile_idx

      }
    }
  }

  if (m_verbose >= POMS_VERBOSE_DEBUG) {
    printf("# AC4InitBlock: tile support calculated, adding to queue... (queue sanity: %i, sanity ac: %i)\n",
        sanityCellTileQueueVisited(), sanityArcConsistency());
  }


  // add appropriate (cell,tile) to m_cell_tile_queue if
  // there's no support in every direction, restricted
  // to the block.
  //
  for (z=block[2][0]; z<block[2][1]; z++) {
    for (y=block[1][0]; y<block[1][1]; y++) {
      for (x=block[0][0]; x<block[0][1]; x++) {

        src_cell = xyz2cell(x,y,z);
        if (src_cell < 0) { continue; }

        if (m_cell_pin[src_cell] != 0) { continue; }

        src_tile_n = cellSize(m_plane, src_cell);
        for (src_tile_idx=0; src_tile_idx < src_tile_n; src_tile_idx++) {
          src_tile = cellTile(m_plane, src_cell, src_tile_idx);

          for (idir=0; idir<6; idir++) {

            if (tileSupport(m_plane, idir, src_cell, src_tile) == 0) {

              if (!cellTileVisited(m_plane, src_cell, src_tile)) {
                cellTileVisited(m_plane, src_cell, src_tile, 1);
                cellTileQueuePush(m_plane, src_cell, src_tile);
              }

            }

          }
        }

      }
    }
  }

  if (m_verbose >= POMS_VERBOSE_DEBUG) {
    printf("# AC4InitBlock: about to run ac4update (qsize:%i) (_counter:%i) (queue sanity: %i, sanity ac: %i)\n",
        (int)cellTileQueueSize(m_plane), (int)_counter,
        sanityCellTileQueueVisited(), sanityArcConsistency());

    printDebug(1);
    printDebugAC4();
  }

  ret = AC4Update();
  if (ret < 0) { return ret; }

  return ret;
}

// Initialize the AC4 tile support structure
//
// Return:
//
//  0 - all tiles have support
// -1 - at least one conflict encountered
//
// On error, the m_confict_... variables will be populated
//
int POMS::AC4Init(int memoize_opt) {
  int r;
  int32_t block[3][2];

  _prof_start(POMS_PROF_AC4INIT);

  block[0][0] = 0; block[0][1] = m_size[0];
  block[1][0] = 0; block[1][1] = m_size[1];
  block[2][0] = 0; block[2][1] = m_size[2];

  r = AC4InitBlock(block, memoize_opt);

  _prof_end(POMS_PROF_AC4INIT);

  return r;
}

//---
//---
//---

// Apply user constraints.
// Does not do constraint propagation.
// The map can be in an indeterminate state
// after a call to applyConstraints.
//
// There are two types of constraints, normal (lower-case) constraints
// and 'start' (upper-case) constraints. The idea is that normal
// constraints apply all throughout the lifetime of the problem,
// being present when restorting to a prefatory state.
// Start constraints only apply at the beginning of the problem run.
//
// This allows for persistent constraints while still allowing transient
// constraints, to seed a solution space, for example, if desired.
//
// codes:
//
// d/D - delete
// a/A - add
// f/F - force
// p   - pin (dosn't make sense for start constraints)
//
/// UPDATE: There are now three types of constraints,
//   - normal constraints (region)
//   - start constraints
//   - quilt constraints
// in the future, I think these should be consolidated but for
// now they're separate.
//
// Quilt constraints are:
//   `=` - pin
//   `+` - add
//   `-` - remove
//
// They operate on the quilt region and are mapped to the current patch
// region, as stored in m_patch_region.
//
//
//
// Return:
//
//  0 - success
//
int POMS::applyConstraints(int apply_start_constraints, int apply_quilt_constraints) {
  int64_t cell, qcell;
  int i, ii, jj;
  int32_t x,y,z,
          tile,
          v[3], qv[3];

  for (i=0; i<m_constraint.size(); i++) {

    if (m_verbose >= POMS_VERBOSE_DEBUG) {
      printf("# applyConstraints: constraint[%i] type:%c tile:[%i,%i] [[%i,%i],[%i,%i],[%i,%i]] (apply_start_constraints:%i)\n",
          i, m_constraint[i].type,
          m_constraint[i].tile_range[0], m_constraint[i].tile_range[1],
          m_constraint[i].size_range[0][0], m_constraint[i].size_range[0][1],
          m_constraint[i].size_range[1][0], m_constraint[i].size_range[1][1],
          m_constraint[i].size_range[2][0], m_constraint[i].size_range[2][1],
          (int)apply_start_constraints);
    }

    for (z=m_constraint[i].size_range[2][0]; z<m_constraint[i].size_range[2][1]; z++) {
      for (y=m_constraint[i].size_range[1][0]; y<m_constraint[i].size_range[1][1]; y++) {
        for (x=m_constraint[i].size_range[0][0]; x<m_constraint[i].size_range[0][1]; x++) {

          for (tile=m_constraint[i].tile_range[0]; tile<m_constraint[i].tile_range[1]; tile++) {

            if (apply_quilt_constraints) {
              qv[0] = x; qv[1] = y; qv[2] = z;
              qcell = vec2cell(qv, m_quilt_size);
              if (qcell < 0) { continue; }

              if (m_constraint[i].type == '=') { m_quilt_pin[qcell] = 1; }

              if ((qv[0] <  m_patch_region[0][0]) ||
                  (qv[0] >= m_patch_region[0][1]) ||
                  (qv[1] <  m_patch_region[1][0]) ||
                  (qv[1] >= m_patch_region[1][1]) ||
                  (qv[2] <  m_patch_region[2][0]) ||
                  (qv[2] >= m_patch_region[2][1])) {
                continue;
              }

              v[0] = qv[0];
              v[1] = qv[1];
              v[2] = qv[2];

              v[0] = qv[0] - m_patch_region[0][0];
              v[1] = qv[1] - m_patch_region[1][0];
              v[2] = qv[2] - m_patch_region[2][0];

              cell = vec2cell(v);
              if (cell < 0) { continue; }

              if      (m_constraint[i].type == '-') { removeTile(m_plane, cell, tile); }
              else if (m_constraint[i].type == '+') { addTile(m_plane, cell, tile); }
              else if (m_constraint[i].type == '!') { forceTile(m_plane, cell, tile); }
              else if (m_constraint[i].type == '=') { pinCell(m_plane, cell, tile); }

              continue;
            }

            v[0] = x; v[1] = y; v[2] = z;
            cell = vec2cell(v);
            if (cell < 0) { continue; }

            if      (m_constraint[i].type == 'd') { removeTile(m_plane, cell, tile); }
            else if (m_constraint[i].type == 'a') { addTile(m_plane, cell, tile); }
            else if (m_constraint[i].type == 'f') { forceTile(m_plane, cell, tile); }
            else if (m_constraint[i].type == 'p') { pinCell(m_plane, cell, tile); }

            if (apply_start_constraints) {
              if      (m_constraint[i].type == 'D') { removeTile(m_plane, cell, tile); }
              else if (m_constraint[i].type == 'A') { addTile(m_plane, cell, tile); }
              else if (m_constraint[i].type == 'F') { forceTile(m_plane, cell, tile); }
            }

          }

        }
      }
    }

  }

  return 0;
}

int POMS::applyStartConstraints(void) {
  return applyConstraints(1);
}

int POMS::applyQuiltConstraints(void) {
  return applyConstraints(0,1);
}

// Add tile to cell.
// This does an O(n) search through all tile entries to make
// sure it's not adding the tile again.
//
// Return:
//
//  0 - tile added (success)
// <0 - cell size is m_tile_count before addition
// >0 - tile already found, no action taken
//
int POMS::addTile( int plane, int64_t cell, int32_t tile ) {
  int r;
  int32_t tile_idx,
          tile_n,
          tile_tmp;

  tile_n = cellSize( plane, cell );
  if (tile_n==m_tile_count) { return -1; }

  for (tile_idx=0; tile_idx < tile_n; tile_idx++) {
    if ( cellTile( plane, cell, tile_idx ) == tile ) {
      break;
    }
  }
  if (tile_idx != tile_n) { return 1; }

  setCellTile( plane, cell, tile_n, tile );
  setCellSize( plane, cell, tile_n+1 );

  return 0;
}

// Force cell to have only `tile` tile
//
// Return:
//
//  0 - success
// <0 - error (?)
//
int POMS::forceTile( int plane, int64_t cell, int32_t tile ) {
  int r;
  int32_t tile_idx,
          tile_n,
          tile_tmp;

  setCellTile( plane, cell, 0, tile );
  setCellSize( plane, cell, 1);
  return 0;
}

// Pin a cell, tile is ignored.
//
// pinned cells will not be resolved, updated with ac4, etc.
//
// Return:
//
//  0 - success
// <0 - error (?)
//
int POMS::pinCell( int plane, int64_t cell, int32_t tile ) {
  int r;
  int32_t tile_idx,
          tile_n,
          tile_tmp;

  m_cell_pin[cell] = 1;
  return 0;
}

// Return the number of cells realized within
// block. That is, return the number of cells
// in the block that have exactly 1 tile
// remaining.
//
// Return:
//
// >=0 - number of cells realized within block
//  <0 - error: at least one cell has no tiles
//       in it.
//
int64_t POMS::realizedCellsWithinBlock(int32_t block[][2]) {
  int32_t x, y, z,
          tile, tile_n;
  int64_t cell, count;

  count=0;
  for (z=block[2][0]; z<block[2][1]; z++) {
    for (y=block[1][0]; y<block[1][1]; y++) {
      for (x=block[0][0]; x<block[0][1]; x++) {
        cell = xyz2cell(x,y,z);

        tile_n = cellSize( m_plane, cell );
        if (tile_n < 1) { return -1; }
        if (tile_n ==1) { count++; }

      }
    }
  }

  return count;
}

int64_t POMS::realizedCellsWithinBlock(void) {
  return realizedCellsWithinBlock(m_block);
}

int64_t POMS::realizedCells(void) {
  int32_t b[3][2];

  b[0][0]=0; b[1][0]=0; b[2][0]=0;
  b[0][1]=m_size[0];
  b[1][1]=m_size[1];
  b[2][1]=m_size[2];

  return realizedCellsWithinBlock(b);
}
