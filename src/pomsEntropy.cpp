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

// Compute entropy of whole map
//
// >0 - all cell positions realized
//  0 - success
// <0 - at least one cell has no tiles
//
int POMS::computeCellEntropy() {
  int32_t block[3][2];

  block[0][0]=0; block[1][0]=0; block[2][0]=0;
  block[0][1]=m_size[0];
  block[1][1]=m_size[1];
  block[2][1]=m_size[2];

  return computeCellEntropyWithinBlock(block, 1);
}

// Compute entropy restricted to `m_block`
//
// >0 - all cell positions realized
//  0 - success
// <0 - at least one cell has no tiles
//
int POMS::computeCellEntropyWithinBlock(int32_t block[][2], int dirty_opt) {
  int64_t cell;
  int32_t x,y,z,
          tile_idx,
          tile,
          tile_n;
  double S, lg2, R, p;
  int ret = 1;

  lg2 = log(2.0);

  for (z=block[2][0]; z<block[2][1]; z++) {
    for (y=block[1][0]; y<block[1][1]; y++) {
      for (x=block[0][0]; x<block[0][1]; x++) {
        cell = xyz2cell(x,y,z);
        if (cell<0) { continue; }

        //EXPERIMENT
        if (m_cell_pin[cell] != 0) { continue; }

        if (dirty_opt &&
            (!m_ac4_dirty[m_plane][cell])) {
          continue;
        }


        tile_n = cellSize(m_plane, cell);
        if (tile_n <  1) { return -1; }
        if (tile_n == 1) {
          m_entropy[cell] = 0.0;
          continue;
        }

        ret = 0;

        S = 0.0;
        R = 0.0;
        for (tile_idx=0; tile_idx < tile_n; tile_idx++) {
          tile = cellTile( m_plane, cell, tile_idx );

          //p = G(tile);
          p = G_cb(cell,tile);

          if (p > m_zero) {
            S += -p * log(p) / lg2;
            R += p;
          }
        }

        if (R > m_zero) {
          S /= R;
          S += log(R) / lg2;
        }

        m_entropy[cell] = S;

      }
    }
  }

  return ret;
}

// Compute entropy restricted to `m_block`
//
// >0 - all cell positions realized
//  0 - success
// <0 - at least one cell has no tiles
//
int POMS::computeCellEntropyWithinBlock() {
  return computeCellEntropyWithinBlock(m_block, 1);
}

//---
//

// pick minimum entropy cell
//
// return:
//
//  1 - all entries fixed
//  0 - success
// <0 - error
//
int POMS::pickCellMinEntropyWithinBlock(int64_t *min_cell,
                                        int32_t *tile_idx,
                                        double *min_entropy,
                                        int32_t block[][2]) {
  int64_t cell,
          _min_cell;
  int32_t x,y,z,
          idx,
          _tile_idx,
          tile,
          tile_n;
  double s_min=-1.0, s=0,
         p_max=-1.0, p=0,
         count=0.0, tcount=0.0,
         R, prv,
         prob_cdf=0.0;
  int ret = 1;

  ret = computeCellEntropyWithinBlock(block, 1);
  if (ret != 0) { return ret; }

  for (z=block[2][0]; z<block[2][1]; z++) {
    for (y=block[1][0]; y<block[1][1]; y++) {
      for (x=block[0][0]; x<block[0][1]; x++) {
        cell = xyz2cell(x,y,z);

        //EXPERIMENT
        if (m_cell_pin[cell] != 0) { continue; }

        tile_n = cellSize(m_plane, cell);
        if (tile_n<1)   { return -1; }
        if (tile_n==1)  { continue; }

        s = m_entropy[cell];

        // add noise to selection, if we've specified a noise
        // parameter
        //
        if (m_entropy_rand_coefficient > m_zero) {
          s += noisePowerLaw(m_entropy_rand_coefficient, m_entropy_rand_exponent);
        }

        if (s_min < -m_zero) { s_min = s; }

        if ( (s - s_min) < -m_zero ) {
          _min_cell = cell;
          s_min = s;
          count = 1.0;
          ret = 0;
        }

        // if entropies are equal, choose
        // randomly between the identical
        // choices
        //
        else if ( fabs(s-s_min) < m_zero ) {
          p = rnd();
          count += 1.0;
          if ( p < (1.0 / count) ) {
            _min_cell = cell;
          }
        }

      }
    }
  }

  if (min_cell)     { *min_cell = _min_cell; }
  if (min_entropy)  { *min_entropy = s_min; }
  if (tile_idx) {

    if (m_tile_choice_policy == POMS_TILE_CHOICE_MAX) {

      tile_n = cellSize( m_plane, _min_cell );
      for (idx=0; idx<tile_n; idx++) {
        tile = cellTile( m_plane, _min_cell, idx );

        //p = G(tile);
        p = G_cb(_min_cell,tile);

        if ( (idx==0) ||
             ((p - p_max) > m_zero) ) {
          _tile_idx = idx;
          p_max = p;
          tcount = 1.0;
          continue;
        }

        else {
          tcount += 1.0;
          p = rnd();
          if ( p < (1.0 / tcount) ) {
            _tile_idx = idx;
          }
        }

      }

      *tile_idx = _tile_idx;
    }

    else if (m_tile_choice_policy == POMS_TILE_CHOICE_PROB) {

      R = 0.0;
      tile_n = cellSize( m_plane, _min_cell );
      for (idx=0; idx<tile_n; idx++) {
        tile = cellTile( m_plane, _min_cell, idx );

        //R += G(tile);
        R += G_cb(_min_cell,tile);

      }

      if (R > m_zero) {

        p = rnd();

        prob_cdf = 0.0;
        for (_tile_idx=0; _tile_idx<(tile_n-1); _tile_idx++) {
          tile = cellTile( m_plane, _min_cell, _tile_idx );

          //if ( (p > prob_cdf) && (p < (prob_cdf + (G(tile)/R))) ) {
          if ( (p > prob_cdf) && (p < (prob_cdf + (G_cb(_min_cell,tile)/R))) ) {
            break;
          }

          //prob_cdf += G(tile) / R;
          prob_cdf += G_cb(_min_cell,tile) / R;

        }
      }
      else { _tile_idx = irnd(tile_n); }

      *tile_idx = _tile_idx;
    }

  }

  return ret;
}

// pick minimum entropy cell
//
// return:
//
//  1 - all entries fixed
//  0 - success
// <0 - error
//
int POMS::pickCellMinEntropyWithinBlock(int64_t *min_cell,
                                        int32_t *tile_idx,
                                        double *min_entropy) {
  return pickCellMinEntropyWithinBlock(min_cell, tile_idx, min_entropy, m_block);
}

// pick minimum entropy cell
//
// return:
//
//  1 - all entries fixed
//  0 - success
// <0 - error
//
int POMS::pickCellMinEntropy(int64_t *min_cell,
                             int32_t *tile_idx,
                             double *min_entropy) {
  int32_t block[3][2];

  block[0][0]=0;
  block[1][0]=0;
  block[2][0]=0;

  block[0][1]=m_size[0];
  block[1][1]=m_size[1];
  block[2][1]=m_size[2];

  return pickCellMinEntropyWithinBlock(min_cell, tile_idx, min_entropy, block);
}

int POMS::computeBlockEntropy(int32_t reuse_cell_entropy) {
  return computeBlockEntropy( m_block_size, reuse_cell_entropy );
}

//----
//----
//----

static int _wf_push( POMS &poms, int _p, int64_t cell, double v ) {
  int64_t qidx, pidx;
  int64_t qcell, pcell;
  int64_t t;

  if ( poms.m_visited[_p][cell] != 0 ) { return -1; }
  if ( poms.m_cell_queue_size[_p] >= poms.m_cell_count ) { return -1; }

  poms.m_visited[_p][cell] = 1;
  poms.m_distance_modifier[cell] = v;

  qidx = poms.m_cell_queue_size[_p];
  poms.m_cell_queue[_p][qidx] = cell;
  poms.m_cell_queue_size[_p]++;

  while (qidx > 0) {
    pidx = qidx/2;

    qcell = poms.m_cell_queue[_p][qidx];
    pcell = poms.m_cell_queue[_p][pidx];
    if (poms.m_distance_modifier[pcell] <= poms.m_distance_modifier[qcell]) {
      break;
    }

    t = poms.m_cell_queue[_p][qidx];
    poms.m_cell_queue[_p][qidx] = poms.m_cell_queue[_p][pidx];
    poms.m_cell_queue[_p][pidx] = t;

    qidx = pidx;
  }

  return 0;
}


static int _wf_pop( POMS &poms, int _p, int64_t *ret_cell ) {
  int64_t qidx, lidx, ridx;
  int64_t qcell, lcell, rcell;
  int64_t t, nxt_idx, nq;
  double v;

  nq = poms.m_cell_queue_size[_p];
  if (nq < 1) { return -1; }
  nq--;
  poms.m_cell_queue_size[_p] = nq;

  *ret_cell = poms.m_cell_queue[_p][0];
  poms.m_cell_queue[_p][0] = poms.m_cell_queue[_p][nq];

  qidx = 0;
  while (qidx < nq) {
    lidx = 2*qidx + 1;
    ridx = 2*qidx + 2;

    qcell = poms.m_cell_queue[_p][qidx];
    v = poms.m_distance_modifier[qcell];

    nxt_idx = -1;

    if (lidx < nq) {
      lcell = poms.m_cell_queue[_p][lidx];
      if ( poms.m_distance_modifier[lcell] < v) {
        v = poms.m_distance_modifier[lcell];
        nxt_idx = lidx;
      }
    }

    if (ridx < nq) {
      rcell = poms.m_cell_queue[_p][ridx];
      if (poms.m_distance_modifier[rcell] < v) {
        v = poms.m_distance_modifier[rcell];
        nxt_idx = ridx;
      }
    }

    if (nxt_idx < 0) { break; }

    t = poms.m_cell_queue[_p][qidx];
    poms.m_cell_queue[_p][qidx] = poms.m_cell_queue[_p][nxt_idx];
    poms.m_cell_queue[_p][nxt_idx] = t;

    qidx = nxt_idx;
  }

  return 0;
}

static int _wf_debugprint( POMS &poms ) {
  int64_t cell;
  int i;

  printf("queue[%i]:", (int)poms.m_cell_queue_size[0]);
  for (i=0; i<poms.m_cell_queue_size[0]; i++) {
    cell = poms.m_cell_queue[0][i];
    printf(" [%i]{idx:%i,v:%i}",
        (int)i,
        (int)cell,
        (int)poms.m_distance_modifier[cell]);
  }
  printf("\n");

  return 0;
}


// NEEDS UPDATING
// bagging on it for now.
// I suspect this can be deleted but I need to review before I make a
// definitive solution.
//
// I'm giong to be using the (now hopefully unused) m_cell_queue data
// structures to help fill the distance modifier grid.
//
int POMS::computeDistanceModifier_resolved(void) {
  int64_t cell,
          nei_cell,
          qidx;
  int32_t x,y,z, xyz,
          idir,
          tile_val, tile_n, tile_idx;
  int r;
  int _p = 0;

  double cell_val = 0.0,
         cell_dv = 1.0 / (double)m_cell_count;

  //experiment
  cell_dv = (double)(m_cell_count * m_tile_count);

  memset( &(m_distance_modifier[0]), 0, sizeof(double) * m_distance_modifier.size() );

  memset( &(m_visited[0][0]), 0, sizeof(int8_t) * m_visited[0].size() );
  memset( &(m_visited[1][0]), 0, sizeof(int8_t) * m_visited[1].size() );
  m_cell_queue_size[0] = 0;
  m_cell_queue_size[1] = 0;

  // find all starting points of our distance flood fill (resolved cells)
  //
  for (cell=0; cell<m_cell_count; cell++) {
    tile_n = cellSize( m_plane, cell );
    if (tile_n == 0) { return -1; }
    if (tile_n == 1) {
      _wf_push( *this, 0, cell, 0.0 );
      continue;
    }
  }

  // no resolved cells is alright, distance_modifier just all 0s
  //
  if (m_cell_queue_size[0] == 0) { return 0; }

  // initially get islands of resolved cells and fill them an
  // increasing value per region.
  //
  cell_val = 0.0;
  for (qidx=0; qidx<m_cell_queue_size[0]; qidx++) {
    cell = m_cell_queue[0][qidx];
    if (m_visited[1][cell]) { continue; }

    _wf_push( *this, 1, cell, cell_val );
    while (m_cell_queue_size[1] > 0) {
      r = _wf_pop( *this, 1, &cell );
      if (r<0){ return r; }

      for (idir=0; idir<6; idir++) {
        nei_cell = neiCell(cell, idir);
        if (nei_cell < 0) { continue; }
        if (m_visited[1][nei_cell]) { continue; }

        tile_n = cellSize( m_plane, nei_cell );
        if (tile_n != 1) { continue; }

        r = _wf_push( *this, 1, nei_cell, cell_val );
      }

    }
    cell_val += cell_dv;
  }


  //DEBUG
  //_wf_debugprint( *this );

  // now go through and compute the distance of unresolved cells
  //
  while ( m_cell_queue_size[_p] > 0 ) {

    r = _wf_pop( *this, _p, &cell );
    if (r<0){ return r; }

    //DEBUG
    //printf("\n\n---\n");
    //printf("popped cell:%i\n", (int)cell);
    //_wf_debugprint( *this );
    //printDebugDistanceModifier();

    for (idir=0; idir<6; idir++) {
      nei_cell = neiCell( cell, idir );
      if (nei_cell < 0) { continue; }
      if (m_visited[_p][nei_cell]) { continue; }

      r = _wf_push( *this, _p, nei_cell, m_distance_modifier[cell]+1.0 );
      if (r<0) { return r; }

      //DEBUG
      //printf("pushed cell:%i\n", (int)nei_cell);
      //_wf_debugprint( *this );

    }
  }

  memset( &(m_visited[0][0]), 0, sizeof(int8_t) * m_visited[0].size() );
  memset( &(m_visited[1][0]), 0, sizeof(int8_t) * m_visited[1].size() );
  m_cell_queue_size[0] = 0;
  m_cell_queue_size[1] = 0;

  return 0;
}

// populate `m_distance_modifier` array
//
int POMS::computeDistanceModifier_xyz(double a, double b, double c) {
  int64_t cell;
  int32_t v[3];
  double cell_dv = (double)m_tile_count, x,y,z, xyz;
  memset( &(m_distance_modifier[0]), 0, sizeof(double) * m_distance_modifier.size() );
  for (cell=0; cell<m_cell_count; cell++) {

    if (m_cell_pin[cell] != 0) { continue; }

    cell2vec(v, cell);
    x = (double)v[0];
    y = (double)v[1];
    z = (double)v[2];
    xyz = (a*x) + (b*y) + (c*z);
    m_distance_modifier[cell] = cell_dv * ((double)xyz);
  }
  return 0;
}

// populate `m_distance_modifier` array
//
int POMS::computeDistanceModifier_xyz(double a, double b, double c, double cx, double cy, double cz, double coef) {
  int64_t cell, cell_processed=0;
  int32_t v[3];
  double cell_dv = (double)m_tile_count, x,y,z, xyz;
  double _min_val=0;

  static int first = 1;

  memset( &(m_distance_modifier[0]), 0, sizeof(double) * m_distance_modifier.size() );
  for (cell=0; cell<m_cell_count; cell++) {

    if (m_cell_pin[cell] != 0) { continue; }

    cell2vec(v, cell);
    x = (double)v[0];
    y = (double)v[1];
    z = (double)v[2];
    x -= cx;
    y -= cy;
    z -= cz;
    xyz = coef*fabs((a*x) + (b*y) + (c*z));
    m_distance_modifier[cell] = cell_dv * ((double)xyz);

    if ((cell_processed==0) ||
        (_min_val > m_distance_modifier[cell])) {
      _min_val = m_distance_modifier[cell];
    }

    cell_processed++;
  }

  for (cell=0; cell<m_cell_count; cell++) {
    if (m_cell_pin[cell] != 0) { continue; }
    m_distance_modifier[cell] -= _min_val;
  }

  //DEBUG
  if (first) {
    int32_t ix,iy,iz;
    first = 0;

    printf("## COMPUTEDISTANCEMODIFIER_XYZ\n");
    printf("## abc: (%f,%f,%f), cxyz:(%f,%f,%f) coef:%f\n",
        a,b,c,cx,cy,cz,coef);

    printf("## m_distance_modifier[z,y,x]:\n");
    for (iz=0; iz<m_size[2]; iz++) {
      for (iy=0; iy<m_size[1]; iy++) {
        printf("## [z%i,y%i,x:]", (int)iz, (int)iy);
        for (ix=0; ix<m_size[0]; ix++) {

          cell = xyz2cell(ix,iy,iz);
          printf(" %f", m_distance_modifier[cell]);
        }
        printf("\n");
      }
      printf("##\n");
    }
    printf("\n");

  }

  return 0;
}

int POMS::computeDistanceModifier_cone(void) {
  int64_t cell;
  int32_t v[3];
  double cell_dv = (double)m_tile_count;
  double dx,dy,dz, cx,cy,cz, d;

  cx = ((double)m_size[0])/2.0;
  cy = ((double)m_size[1])/2.0;
  cz = ((double)m_size[2])/2.0;

  memset( &(m_distance_modifier[0]), 0, sizeof(double) * m_distance_modifier.size() );

  for (cell=0; cell<m_cell_count; cell++) {

    if (m_cell_pin[cell] != 0) { continue; }

    cell2vec(v, cell);
    dx = ((double)v[0]) - cx;
    dy = ((double)v[1]) - cy;
    dz = ((double)v[2]) - cz;

    d = sqrt( (dx*dx) + (dy*dy) + (dz*dz) );

    m_distance_modifier[cell] = cell_dv * d;
  }
  return 0;
}

int POMS::computeDistanceModifier_minus_cone(void) {
  int64_t cell, cell_processed=0;
  int32_t v[3];
  double cell_dv = (double)m_tile_count;
  double dx,dy,dz, cx,cy,cz, d;

  double _min_val=0;

  cx = ((double)m_size[0])/2.0;
  cy = ((double)m_size[1])/2.0;
  cz = ((double)m_size[2])/2.0;

  memset( &(m_distance_modifier[0]), 0, sizeof(double) * m_distance_modifier.size() );

  for (cell=0; cell<m_cell_count; cell++) {
    if (m_cell_pin[cell] != 0) { continue; }

    cell2vec(v, cell);
    dx = ((double)v[0]) - cx;
    dy = ((double)v[1]) - cy;
    dz = ((double)v[2]) - cz;

    d = sqrt( (dx*dx) + (dy*dy) + (dz*dz) );

    m_distance_modifier[cell] = -cell_dv * d;

    if ((cell_processed==0) ||
        (_min_val > m_distance_modifier[cell])) {
      _min_val = m_distance_modifier[cell];
    }

    cell_processed++;
  }

  for (cell=0; cell<m_cell_count; cell++) {
    if (m_cell_pin[cell] != 0) { continue; }
    m_distance_modifier[cell] -= _min_val;
  }


  return 0;
}

int POMS::computeDistanceModifier(void) {
  switch (m_distance_modifier_opt) {

    case POMS_OPT_DISTANCE_XYZ:
      return computeDistanceModifier_xyz(1,1,1);
      break;

    case POMS_OPT_DISTANCE_CONE:
      return computeDistanceModifier_cone();
      break;

    case POMS_OPT_DISTANCE_X:
      return computeDistanceModifier_xyz(1,0,0);
      break;

    case POMS_OPT_DISTANCE_Y:
      return computeDistanceModifier_xyz(0,1,0);
      break;

    case POMS_OPT_DISTANCE_Z:
      return computeDistanceModifier_xyz(0,0,1);
      break;

    case POMS_OPT_DISTANCE_MINUS_XYZ:
      return computeDistanceModifier_xyz(-1,-1,-1,
          (double)m_size[0], (double)m_size[1], (double)m_size[2]);
      break;

    case POMS_OPT_DISTANCE_MINUS_CONE:
      return computeDistanceModifier_minus_cone();
      break;

    case POMS_OPT_DISTANCE_MINUS_X:
      return computeDistanceModifier_xyz(-1,0,0,
          (double)m_size[0], (double)m_size[1], (double)m_size[2]);
      break;

    case POMS_OPT_DISTANCE_MINUS_Y:
      return computeDistanceModifier_xyz(0,-1,0,
          (double)m_size[0], (double)m_size[1], (double)m_size[2]);
      break;

    case POMS_OPT_DISTANCE_MINUS_Z:
      return computeDistanceModifier_xyz(0,0,-1,
          (double)m_size[0], (double)m_size[1], (double)m_size[2]);
      break;

    case POMS_OPT_DISTANCE_PLANE:
      return computeDistanceModifier_xyz( m_distance_v[0], m_distance_v[1], m_distance_v[2],
                                          m_distance_p[0], m_distance_p[1], m_distance_p[2],
                                          1);
      break;
    case POMS_OPT_DISTANCE_MINUS_PLANE:
      return computeDistanceModifier_xyz( m_distance_v[0], m_distance_v[1], m_distance_v[2],
                                          m_distance_p[0], m_distance_p[1], m_distance_p[2],
                                          -1);
      break;

    default:
      return -1;
  }
  return -1;
}

//----
//----
//----

// untested
//
int POMS::sanityBlockEntropy(int32_t *block_size) {
  int64_t cell;
  int32_t tile_idx,
          tile_n;
  int32_t x,y,z,
          xx,yy,zz;
  double S, lg2, R, p;

  double *B, *M;
  int32_t nx,ny,nz,
          mx,my,mz,
          sx,sy,sz,
          bx,by,bz;
  int32_t c_idx;

  double _eps = 1.0/1024.0,
         max_eps = -1.0,
         d;

  computeBlockEntropy(block_size);

  nx = m_size[0] - block_size[0] + 1;
  ny = m_size[1] - block_size[1] + 1;
  nz = m_size[2] - block_size[2] + 1;

  sx = block_size[0];
  sy = block_size[1];
  sz = block_size[2];

  B = &(m_block_entropy[0]);
  M = &(m_entropy[0]);

  mx = m_size[0];
  my = m_size[1];
  mz = m_size[2];

  for (z=0; z<nz; z++) {
    for (y=0; y<ny; y++) {
      for (x=0; x<nx; x++) {

        cell = (z*mx*my) + y*mx + x;

        R = 0.0;
        for (zz=0; zz<sz; zz++) {
          for (yy=0; yy<sy; yy++) {
            for (xx=0; xx<sx; xx++) {
              R += M[ (z+zz)*(mx*my) + (y+yy)*(mx) + (x+xx) ];
            }
          }
        }

        d = fabs(R - B[ z*mx*my + y*mx + x ]);

        if (d > max_eps) { max_eps = d; }

        if (d > _eps) {

          //printf("block sanity error@[%i,%i,%i], size:(%i,%i,%i), calc:%f, stored:%f\n",
          //    (int)x, (int)y, (int)z,
          //    (int)block_size[0], (int)block_size[1], (int)block_size[2],
          //    R, B[ z*mx*my + y*mx + x ]);

          return -1;
        }

      }
    }
  }

  //printf("sanityblockentropy max_eps:%f\n", max_eps);

  return 0;
}

//
//
int POMS::computeBlockEntropy(int32_t *block_size, int32_t reuse_cell_entropy) {
  int64_t cell;
  int32_t tile_idx,
          tile_n;
  int32_t x,y,z,
          xx,yy,zz;
  double S, R, p;

  double *B, *M;
  int32_t nx,ny,nz,
          mx,my,mz,
          sx,sy,sz,
          bx,by,bz;
  int32_t c_idx;

  //              0  1  2  3  4  5  6  7
  //              +  -  -  +  -  +  +  -
  int coef[8] = { 1,-1,-1, 1,-1, 1, 1,-1 };
  int d_idx[8][3] = {
    { -1, -1, -1 },
    { -1, -1,  0 },
    { -1,  0, -1 },
    { -1,  0,  0 },
    {  0, -1, -1 },
    {  0, -1,  0 },
    {  0,  0, -1 },
    {  0,  0,  0 }
  };

  if (!reuse_cell_entropy) { computeCellEntropy(); }

  // init boundaries of B
  //

  nx = m_size[0] - block_size[0] + 1;
  ny = m_size[1] - block_size[1] + 1;
  nz = m_size[2] - block_size[2] + 1;

  sx = block_size[0];
  sy = block_size[1];
  sz = block_size[2];

  B = &(m_block_entropy[0]);
  M = &(m_entropy[0]);

  mx = m_size[0];
  my = m_size[1];
  mz = m_size[2];

  //DEBUG
  //printf("blockentropy: nxyz(%i,%i,%i), sxyz(%i,%i,%i), mxyz(%i,%i,%i)\n",
  //    (int)nx, (int)ny, (int)nz,
  //    (int)sx, (int)sy, (int)sz,
  //    (int)mx, (int)my, (int)mz);

  // first 0,0,0
  // O( s * s )
  //
  B[0] = 0;
  for (z=0; z<sz; z++) {
    for (y=0; y<sy; y++) {
      for (x=0; x<sx; x++) {
        B[0] += M[ (z*mx*my) + (y*mx) + x ];
      }
    }
  }

  // z axis init
  // O( n_b * s * s )
  //
  x = 0;
  y = 0;
  for (z=1; z<nz; z++) {
    B[ z*mx*my + y*mx + x ] = B[ (z-1)*mx*my + y*mx + x ];
    for (yy=0; yy<sy; yy++) {
      for (xx=0; xx<sx; xx++) {
        B[ z*mx*my + y*mx + x ] += M[ (z+sz-1)*mx*my + yy*mx + xx ] - M[ (z-1)*mx*my + yy*mx + xx ];
      }
    }
  }

  // y axis init
  // O( n_b * s * s )
  //
  x = 0;
  z = 0;
  for (y=1; y<ny; y++) {
    B[ z*mx*my + y*mx + x ] = B[ z*my*mx + (y-1)*mx + x ];
    for (zz=0; zz<sz; zz++) {
      for (xx=0; xx<sx; xx++) {
        B[ z*mx*my + y*mx + x ] += M[ zz*mx*my + (y+sy-1)*mx + xx ] - M[ zz*mx*my + (y-1)*mx + xx ];
      }
    }
  }

  // x axis init
  // O( n_b * s * s )
  //
  y = 0;
  z = 0;
  for (x=1; x<nx; x++) {
    B[ z*mx*my + y*mx + x ] = B[ z*mx*my + y*mx + (x-1) ];
    for (zz=0; zz<sz; zz++) {
      for (yy=0; yy<sy; yy++) {
        B[ z*mx*my + y*mx + x ] += M[ zz*mx*my + yy*mx + (x+sx-1) ] - M[ zz*mx*my + yy*mx + (x-1) ];
      }
    }
  }

  // xy,z=0 plane init
  // O( n_b * n_b * s )
  //
  z = 0;
  for (y=1; y<ny; y++) {
    for (x=1; x<nx; x++) {
      B[ z*mx*my + y*mx + x ] =
          B[ z*mx*my + (y-1)*mx + x     ]
        + B[ z*mx*my + (y  )*mx + (x-1) ]
        - B[ z*mx*my + (y-1)*mx + (x-1) ];

      for (zz=0; zz<sz; zz++) {
        B[ z*mx*my + y*mx + x ] +=
            M[ zz*mx*my +    (y-1)*mx +    (x-1) ]
          - M[ zz*mx*my + (y+sy-1)*mx +    (x-1) ]
          - M[ zz*mx*my +    (y-1)*mx + (x+sx-1) ]
          + M[ zz*mx*my + (y+sy-1)*mx + (x+sx-1) ];

      }
    }
  }

  // xz, y=0 plane init
  // O( n_b * n_b *s )
  //
  y = 0;
  for (z=1; z<nz; z++) {
    for (x=1; x<nx; x++) {
      B[ z*mx*my + y*mx + x ] =
          B[ (z-1)*mx*my + y*mx + x     ]
        + B[   (z)*mx*my + y*mx + (x-1) ]
        - B[ (z-1)*mx*my + y*mx + (x-1) ];

      for (yy=0; yy<sy; yy++) {
        B[ z*mx*my + y*mx + x ] +=
            M[    (z-1)*mx*my + yy*mx +    (x-1) ]
          - M[ (z+sz-1)*mx*my + yy*mx +    (x-1) ]
          - M[    (z-1)*mx*my + yy*mx + (x+sx-1) ]
          + M[ (z+sz-1)*mx*my + yy*mx + (x+sx-1) ];

      }
    }
  }

  // yz, x=0 plane init
  // O( n_b * n_b *s )
  //
  x = 0;
  for (z=1; z<nz; z++) {
    for (y=1; y<ny; y++) {
      B[ z*mx*my + y*mx + x ] =
          B[ (z-1)*mx*my +   (y)*mx + x ]
        + B[   (z)*mx*my + (y-1)*mx + x ]
        - B[ (z-1)*mx*my + (y-1)*mx + x ];

      for (xx=0; xx<sx; xx++) {
        B[ z*mx*my + y*mx + x ] +=
            M[    (z-1)*mx*my +    (y-1)*mx + xx ]
          - M[ (z+sz-1)*mx*my +    (y-1)*mx + xx ]
          - M[    (z-1)*mx*my + (y+sy-1)*mx + xx ]
          + M[ (z+sz-1)*mx*my + (y+sy-1)*mx + xx ];

      }
    }
  }


  // rest of the grid.
  //
  // O( n_b * n_b * n_b )
  //
  for (z=1; z<nz; z++) {

    for (y=1; y<ny; y++) {
      for (x=1; x<nx; x++) {

        B[ z*mx*my + y*mx + x ] = 0;
        for (c_idx=0; c_idx<8; c_idx++) {
          bx = x + d_idx[c_idx][0];
          by = y + d_idx[c_idx][1];
          bz = z + d_idx[c_idx][2];

          if (c_idx < 7) {
            B[ z*mx*my + y*mx + x ] += coef[c_idx]*B[ bz*mx*my + by*mx + bx ];
          }

          xx = x - 1 + sx*(d_idx[c_idx][0]+1);
          yy = y - 1 + sy*(d_idx[c_idx][1]+1);
          zz = z - 1 + sz*(d_idx[c_idx][2]+1);

          B[ z*mx*my + y*mx + x ] += -coef[c_idx]*M[ zz*mx*my + yy*mx + xx ];

        }
      }
    }
  }

  return 0;
}

//---
//

int POMS::entropy(double *S) {
  int32_t block[3][2];

  block[0][0] = 0; block[0][1] = m_size[0];
  block[1][0] = 0; block[1][1] = m_size[1];
  block[2][0] = 0; block[2][1] = m_size[2];

  return entropyBlock(S, block);
}

int POMS::entropyBlock(double *S, int32_t block[][2]) {
  int64_t cell;
  int32_t tile_idx, tile, tile_n;
  int32_t x,y,z;

  double s, p, _S, R;

  computeCellEntropyWithinBlock(block, 1);

  _S = 0.0;
  for (z=block[2][0]; z<block[2][1]; z++) {
    for (y=block[1][0]; y<block[1][1]; y++) {
      for (x=block[0][0]; x<block[0][1]; x++) {
        cell = xyz2cell(x,y,z);
        if (cell<0) { continue; }

        //EXPERIMENT
        if (m_cell_pin[cell] != 0) { continue; }

        tile_n = cellSize(m_plane, cell);
        if (tile_n < 1) { return -1; }

        s = m_entropy[cell];
        if (s < m_zero) { continue; }

        _S += s;
      }
    }
  }

  *S = 0.0;
  if (_S < m_zero) { return 1; }

  *S = _S;
  return 0;
}

//---
// Calculate the "entropy center of mass".
//
// Return:
//
// >0 - grid fully realized
//  0 - success, valid center of mass
// <0 - error
//
// In the case of success, com will hold the center of mass.
// If the grid is in a contradictory state or if it's fully
// realized, com will not be populated
//
int POMS::entropyCenterOfMass(double *com, int32_t block[][2]) {
  int64_t cell;
  int32_t tile_idx, tile, tile_n;
  int32_t x,y,z;

  double s, p, S, R, lg2;
  double vec[3];

  lg2 = log(2.0);

  computeCellEntropyWithinBlock(block);

  vec[0] = 0.0;
  vec[1] = 0.0;
  vec[2] = 0.0;

  R = 0.0;

  for (z=block[2][0]; z<block[2][1]; z++) {
    for (y=block[1][0]; y<block[1][1]; y++) {
      for (x=block[0][0]; x<block[0][1]; x++) {

        cell = xyz2cell(x,y,z);
        if (cell<0) { continue; }

        //EXPERIMENT
        if (m_cell_pin[cell] != 0) { continue; }

        tile_n = cellSize(m_plane, cell);
        if (tile_n < 1) { return -1; }

        s = m_entropy[cell];
        if (s < m_zero) { continue; }

        R += s;
        vec[0] += ((double)x)*s;
        vec[1] += ((double)y)*s;
        vec[2] += ((double)z)*s;

      }
    }
  }

  if (R < m_zero) { return 1; }

  com[0] = vec[0]/R;
  com[1] = vec[1]/R;
  com[2] = vec[2]/R;

  return 0;
}

//----

// UNUSED?
// this only does a normalization based on global
// tile pdf, not on a cell basis.
// This was used while exploring idea.s
// It might be tim eto take this out...
//
int POMS::dist( double *d, POMS &B ) {
  int64_t cell;
  int32_t tile;
  double _d=0.0,
         _c = 0.0,
         _r=0.0,
         _R=0.0;

  int u,v;

  if ( (m_cell_count != B.m_cell_count) ||
       (m_tile_count != B.m_tile_count) ) {
    return -1;
  }

  for (tile=0; tile < m_tile_count; tile++) {
    _R += G(tile);
  }
  if (_R < m_zero) { return -2; }

  for (cell=0; cell < m_cell_count; cell++) {
    _c = 0.0;
    _r = 0.0;
    for (tile=0; tile < m_tile_count; tile++) {

      u =   cellHasTile(   m_plane, cell, tile );
      v = B.cellHasTile( B.m_plane, cell, tile );

      if (u!=v) { _d += G(tile); }
    }

  }

  *d = _d / (2.0 * _R * ((double)m_cell_count));

  return 0;
}

// my interpretation
//
// For each cell:
//
// J_{A_c} = \sum_{d \in A_c} p_d
// J_{B_c} = \sum_{d \in B_c} p_d
// J_{A_c,B_c} = \sum{ d \in \{ A_c \cap B_c \} } p_{A_d} \cdot p_{B_d}
//
// J_{c} = \frac{ J_{A_c,B_c} }{ J_{A_c} + J_{B_c} - J_{A_c,B_c} }
//
// J = \frac{1}{N} \sum_{c} J_{c}
//
// Where $N$ is the number of cells.
//
// Return:
//
//  0 - success, *d holds (renormalized) distance
// <0 - fail (normalization <=0)
//
// ---
//
// arg, missnamed...
//
int POMS::distJaccard( double *d, POMS &B ) {
  int64_t cell;
  int32_t tile;
  int u,v;

  double _a, _b,
         _R_a, _R_b;
  double _cell_J_ab = 0.0,
         _cell_J_a  = 0.0,
         _cell_J_b  = 0.0;

  double _S = 0.0;

  if ( (m_cell_count != B.m_cell_count) ||
       (m_tile_count != B.m_tile_count) ) {
    return -1;
  }

  for (cell=0; cell < m_cell_count; cell++) {

    _R_a = 0.0;
    _R_b = 0.0;

    _cell_J_ab= 0.0;
    _cell_J_a = 0.0;
    _cell_J_b = 0.0;
    for (tile=0; tile < m_tile_count; tile++) {

      u =   cellHasTile(   m_plane, cell, tile );
      v = B.cellHasTile( B.m_plane, cell, tile );

      _a = ((u==0) ? 0.0 :  G(tile));
      _b = ((v==0) ? 0.0 :B.G(tile));

      _R_a += _a;
      _R_b += _b;

      _cell_J_ab += _a*_b;
      _cell_J_a  += _a;
      _cell_J_b  += _b;

    }

    if ((_R_a < m_zero) || (_R_b < m_zero)) { return -1; }

    _cell_J_ab  /= _R_a*_R_b;
    _cell_J_a   /= _R_a;
    _cell_J_b   /= _R_b;

    if ( (_cell_J_a + _cell_J_b - _cell_J_ab) < m_zero ) { return -1; }
    _S += (1.0 - (_cell_J_ab / (_cell_J_a + _cell_J_b - _cell_J_ab)));

  }

  *d = (_S / ((double)m_cell_count) );
  return 0;
}

int POMS::distCosine( double *d, POMS &B ) {
  int64_t cell;
  int32_t tile;
  int u,v;

  double _a, _b,
         _R_a, _R_b;
  double _cell_v_ab = 0.0,
         _cell_v_aa = 0.0,
         _cell_v_bb = 0.0;

  double _S = 0.0;

  for (cell=0; cell < m_cell_count; cell++) {
    _R_a = 0.0;
    _R_b = 0.0;

    _cell_v_ab = 0.0;
    _cell_v_aa = 0.0;
    _cell_v_bb = 0.0;

    for (tile=0; tile < m_tile_count; tile++) {
      u =   cellHasTile(   m_plane, cell, tile );
      v = B.cellHasTile( B.m_plane, cell, tile );

      _a = ((u==0) ? 0.0 :  G(tile));
      _b = ((v==0) ? 0.0 :B.G(tile));

      _R_a += _a;
      _R_b += _b;

      _cell_v_ab += _a*_b;
      _cell_v_aa += _a*_a;
      _cell_v_bb += _b*_b;
    }

    if ((_R_a < m_zero) || (_R_b < m_zero)) { return -1; }

    _cell_v_ab /= _R_a*_R_b;
    _cell_v_aa /= _R_a*_R_a;
    _cell_v_bb /= _R_b*_R_b;

    _cell_v_aa = sqrt(_cell_v_aa);
    _cell_v_bb = sqrt(_cell_v_bb);

    if ((_cell_v_aa < m_zero) || (_cell_v_bb < m_zero)) { return -1; }

    _S += _cell_v_ab / (_cell_v_aa * _cell_v_bb);

  }

  *d = (_S / ((double)m_cell_count) );
  return 0;
}
