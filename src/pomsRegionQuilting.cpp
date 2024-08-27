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

// WIP
// ABSOLUTELY NOT READY
// THIS HAS BUGS, WON'T COMPILE, ETC.


// linked list by array position
//
typedef struct _region_tree_type {
  std::vector< int64_t > parent;
  std::vector< int64_t > child;
  std::vector< int64_t > sibling;

  void init(int64_t n) {
    parent.resize(n, -1);
    child.resize(n, -1);
    sibling.resize(n, -1);
  }

  void addChild(int64_t p_id, int64_t c_id) {
    int64_t id, prv_child_id;

    parent[c_id] = p_id;

    prv_child_id = child[p_id];
    child[p_id] = c_id;
    sibling[c_id] = prv_child_id;

    //for (id = child[p_id]; id>=0; id = sibling[id] );
    //sibling[id] = c_id;

  }

  void debugPrint(void) {
    int64_t i;
    for (i=0; i<parent.size(); i++) {
      printf("node: %i, parent: %i, sibling: %i, child: %i\n",
          (int)i, (int)parent[i], (int)sibling[i], (int)child[i]);
    }
  }

} _region_tree_t;

void _region_tree_init( _region_tree_t &rt, int64_t n) {
  rt.parent.resize(n, -1);
  rt.child.resize(n, -1);
  rt.sibling.resize(n, -1);
}

void _region_tree_add_child( _region_tree_t &rt, int64_t p_id, int64_t c_id) {
  int64_t id;

  rt.parent[c_id] = p_id;
  rt.sibling[c_id] = -1;

  for (id = rt.child[p_id]; id>=0; id = rt.sibling[id] );
  rt.sibling[id] = c_id;

}

// partition into inside/outside regions
void POMS::entropyFilter(double threshold, int reuse_cell_entropy) {
  int64_t cell;
  //std::vector< int8_t > m_cell_filter;
  //double m_cell_filter_value;

  if (!reuse_cell_entropy) { computeCellEntropy(); }

  for (cell=0; cell<m_cell_count; cell++) {
    m_cell_filter[cell] = 0;
    if (m_entropy[cell] > threshold) {
      m_cell_filter[cell] = 1;
    }
  }

}

void _printDebugRegion( POMS &poms, std::vector< int64_t > m_cell_region_id, int32_t m_size[3]) {
  int64_t cell;
  int32_t x,y,z;

  for (z=0; z<m_size[2]; z++) {
    for (y=0; y<m_size[1]; y++) {
      for (x=0; x<m_size[0]; x++) {
        cell = poms.xyz2cell(x,y,z);
        printf(" %3i", (int)m_cell_region_id[cell]);
      }
      printf("\n");
    }
    printf("\n");
  }
  printf("\n");
}

// region_id:                   cell  -> region id assignment
// m_cell_unprocessed:          idx   -> cell still unprocessed
// m_cell_unprocessed_bp:       cell  -> idx into m_cell_unprocessed (-1 if dne)
// m_cell_region_queue:         idx   -> region id queued
// m_cell_region_queue_bp:      rgnid -> idx into m_cell_region_queue (-1 if dne)
//
// m_region_nei:                cell,cell  -> array of neighbor regions
// m_region_nei_bp:             cell,cell  -> bp (?)
//
// unprocessed holds list of cells that have yet to be considered. When doing
// flood fill and we reach the end of the flood, we take a new cell to consider
// from the unprocessed list.
//
// region queue holds the wave front from the region being flooded.
// If the cell is already in the region or has already been processed (that is,
// it's non-existent in the unprocessed list) we don't add it back in.
//
int POMS::assignRegionIDs(void) {
  std::vector< int64_t >  m_cell_region_id,
                          m_cell_unprocessed,
                          m_cell_unprocessed_bp,
                          m_cell_region_queue,
                          m_cell_region_queue_bp;
  std::vector< std::vector< int64_t > > m_region_nei,
                                        m_region_nei_bp;

  int64_t cell,
          idx,
          unprocessed_n,
          region_queue_n,
          n_region_id;
  int8_t  src_region_type,
          dst_region_type;
  int64_t nei_cell,
          src_region,
          dst_region;

  int64_t src_idx,
          dst_idx;

  int32_t idir;

  int64_t i, j, k;

  _region_tree_t rt;

  //DEBUG
  m_cell_region_id.resize( m_cell_count, -1 );
  m_cell_unprocessed.resize( m_cell_count, -1 );
  m_cell_unprocessed_bp.resize( m_cell_count, -1 );
  m_cell_region_queue.resize( m_cell_count, -1 );
  m_cell_region_queue_bp.resize( m_cell_count, -1 );

  //entropyFilter(threshold);

  region_queue_n = 0;
  unprocessed_n = 0;
  for (cell=0; cell<m_cell_count; cell++) {
    m_cell_region_id[cell] = 0;
    m_cell_unprocessed[unprocessed_n] = cell;
    m_cell_unprocessed_bp[cell] = unprocessed_n;
    unprocessed_n++;
  }

  //DEBUG 
  //printf("unprocessed_n: %i\n", (int)unprocessed_n);

  // first, go through and assign unique regions
  // to each contiguous set of connected cells.
  // Regions are identified by the their filter
  // value
  //
  n_region_id = 0;
  while (unprocessed_n>0) {
    n_region_id++;

    //DEBUG
    //printf("\n\n>> n_region_id: %i\n", (int)n_region_id);

    cell = m_cell_unprocessed[unprocessed_n-1];

    region_queue_n = 0;
    m_cell_region_queue[region_queue_n] = cell;
    region_queue_n++;

    //DEBUG
    //printf("## initial unprocessed cell: %i\n", (int)cell);
    //printf("## m_cell_region_queue[%i]:", (int)region_queue_n);
    //for (i=0; i<region_queue_n; i++) { printf(" %i", (int)m_cell_region_queue[i]); }
    //printf("\n");


    while (region_queue_n>0) {

      //DEBUG
      //printf("\n\n");
      //printf("### region_queue[%i]:", (int)region_queue_n);
      //for (i=0; i<region_queue_n; i++) { printf(" %i", (int)m_cell_region_queue[i]); }
      //printf("\n");

      region_queue_n--;
      cell = m_cell_region_queue[region_queue_n];

      // sanity
      //
      if ((m_cell_region_id[cell] > 0) ||
          (m_cell_unprocessed_bp[cell] < 0)) {

        //printf("SANITY ERROR: cell %i: m_cell_region_id[%i]:%i, m_cell_unprocessed_bp[%i]:%i\n",
        //    (int)cell,
        //    (int)cell, (int)m_cell_region_id[cell],
        //    (int)cell, (int)m_cell_unprocessed_bp[cell]);
        return -1;
      }

      //DEBUG
      //printf("### picking cell: %i, region_queue_n now: %i\n", (int)cell, (int)region_queue_n);

      m_cell_region_id[cell] = n_region_id;
      src_region_type = m_cell_filter[cell];

      unprocessed_n--;
      idx = m_cell_unprocessed_bp[cell];
      m_cell_unprocessed[idx] = m_cell_unprocessed[unprocessed_n];
      m_cell_unprocessed_bp[ m_cell_unprocessed[idx] ] = idx;
      m_cell_unprocessed_bp[cell] = -1;

      //printf("### removing cell %i, putting cell %i in it's place (idx:%i)\n",
      //    (int)cell, (int)m_cell_unprocessed[idx], (int)idx);

      // if:
      //   - it's out of bounds, skip
      //   - already in region queue, skip
      //   - already been processed (not in unprocessed), skip
      //   - it's a different region type, skip
      // else:
      //   - add to region queue for processing
      //
      for (idir=0; idir<6; idir++) {
        nei_cell = neiCell(cell, idir);

        //DEBUG
        //printf("#### nei_cell: %i\n", (int)nei_cell);


        if (nei_cell<0) { continue; }

        //DEBUG
        //printf("####  queue_bp[%i]: %i, unprocessed_bp[%i]: %i, filter[%i]: %i\n",
        //    (int)nei_cell, (int)m_cell_region_queue_bp[nei_cell],
        //    (int)nei_cell, (int)m_cell_unprocessed_bp[nei_cell],
        //    (int)nei_cell, (int)m_cell_filter[nei_cell]);

        if (m_cell_region_queue_bp[nei_cell] >= 0) { continue; }
        if (m_cell_unprocessed_bp[nei_cell] < 0) { continue; }

        dst_region_type = m_cell_filter[nei_cell];
        if (src_region_type != dst_region_type) { continue; }

        m_cell_region_queue[region_queue_n] = nei_cell;
        m_cell_region_queue_bp[ nei_cell ] = region_queue_n;
        region_queue_n++;

      }
    }

  }
  n_region_id++;

  //printf("n_region_id:%i\n", (int)n_region_id);

  printf("n_region_id: %i\n", (int)n_region_id);
  _printDebugRegion( *this, m_cell_region_id, m_size );

  m_region_nei.resize( n_region_id );
  m_region_nei_bp.resize( n_region_id );

  for (src_region=0; src_region<n_region_id; src_region++) {
    m_region_nei_bp[src_region].resize(n_region_id, -1);
  }

  // regions have been calculatd, now find neighbors of each
  // region
  //
  for (cell=0; cell<m_cell_count; cell++) {
    for (idir=0; idir<6; idir++) {
      nei_cell = neiCell(cell, idir);
      if (nei_cell < 0) {
        if ((idir<4) && (m_region_nei_bp[src_region][0] < 0)) {
          m_region_nei_bp[src_region][0] = m_region_nei[src_region].size();
          m_region_nei[src_region].push_back(0);

          m_region_nei_bp[0][src_region] = m_region_nei[0].size();
          m_region_nei[0].push_back(src_region);
        }
        continue;
      }
      if (m_cell_region_id[cell] != m_cell_region_id[nei_cell]) {

        src_region = m_cell_region_id[cell];
        dst_region = m_cell_region_id[nei_cell];

        if (m_region_nei_bp[src_region][ dst_region ] < 0) {
          m_region_nei_bp[src_region][ dst_region ] = m_region_nei[src_region].size();
          m_region_nei[src_region].push_back(dst_region);
        }

        if (m_region_nei_bp[dst_region][ src_region ] < 0) {
          m_region_nei_bp[dst_region][ src_region ] = m_region_nei[dst_region].size();
          m_region_nei[dst_region].push_back(src_region);
        }

      }
    }
  }


  //DEBUG
  for (idx=0; idx<n_region_id; idx++) {
    printf("region{%i}:", (int)(idx));
    for (i=0; i<m_region_nei[idx].size(); i++) {
      printf(" %i", (int)m_region_nei[idx][i]);

      for (j=(i+1); j<m_region_nei[idx].size(); j++) {
        if (m_region_nei[idx][i] == m_region_nei[idx][j]) {
          printf("DUP!!!");
        }
      }
    }
    printf("\n");
  }

  // now that we have the neighbor list, we can work from the outside
  // in to figure out heirarchical containment.
  // The 0 region is assumed to be on the boundary, so all regions that
  // are neighbors to 0 are siblings of each other and children of the boundary.
  //
  // forall regions, r, that have 0 as a neighbor
  //   add r as a child to 0 to tree
  //
  // region_queue = all naighbors of children of 0, minus children of 0
  //
  // while region_queue non empty
  //   r = pop(region_queue)
  //   if r in tree, skip
  //
  //   
  //

  //NOPE
  rt.init(n_region_id);
  for (idx=0; idx<m_region_nei[0].size(); idx++) {
    rt.addChild(0, m_region_nei[0][idx]);
  }

  for (src_region=0; src_region < n_region_id; src_region++) {
    for (idx=0; idx<m_region_nei[src_region].size(); idx++) {
      dst_region = m_region_nei[src_region][idx];
      rt.addChild(src_region, dst_region);
    }
  }

  rt.debugPrint();


  // consolidate regions (outer boundary)
  //

  return 0;

}
