/*
 * To the extent possible under law, the person who associated CC0 with
 * this file has waived all copyright and related or neighboring rights
 * to this file.
 *      
 * You should have received a copy of the CC0 legalcode along with this
 * work. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 *
 */

#ifndef POMS_HPP
#define POMS_HPP

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

#include <sys/time.h>

#include <math.h>

#include <string>
#include <vector>

#include "ac4_tier.hpp"

#define POMS_VERSION "0.23.0"

#define POMS_VERBOSE_ERROR      -2
#define POMS_VERBOSE_WARNING    -1
#define POMS_VERBOSE_NONE        0
#define POMS_VERBOSE_ITER        1
#define POMS_VERBOSE_RUN         2
#define POMS_VERBOSE_STEP        3
#define POMS_VERBOSE_INTRASTEP   4
#define POMS_VERBOSE_DEBUG       5
#define POMS_VERBOSE_DEBUG0      6
#define POMS_VERBOSE_DEBUG1      7
#define POMS_VERBOSE_DEBUG2      8
#define POMS_VERBOSE_DEBUG3      9

#define POMS_CONSTRAINT_ADD 'a'
#define POMS_CONSTRAINT_DEL 'd'
#define POMS_CONSTRAINT_FIX 'f'

#define POMS_CONFLICT_BOUNDARY    -1
#define POMS_CONFLICT_NO_SUPPORT  -2
#define POMS_CONFLICT_EMPTY       -3
#define POMS_CONFLICT_MISMATCH    -4
#define POMS_CONFLICT_NEGATIVE_SUPPORT_SANITY_ERROR -5

#define POMS_STATE_INIT       0
#define POMS_STATE_CONSISTENT 1
#define POMS_STATE_SUCCESS    2
#define POMS_STATE_CONFLICT   3

#define POMS_TILE_CHOICE_MAX   0
#define POMS_TILE_CHOICE_PROB  1

#define POMS_BLOCK_CHOICE_SEQUENTIAL    0
#define POMS_BLOCK_CHOICE_MAX_ENTROPY   1
#define POMS_BLOCK_CHOICE_MIN_ENTROPY   2
#define POMS_BLOCK_CHOICE_WAVEFRONT     3

#define POMS_HEMP_POLICY_STATIC 0
#define POMS_HEMP_POLICY_COM    1

#define POMS_OPT_DISTANCE_XYZ         0
#define POMS_OPT_DISTANCE_CONE        1
#define POMS_OPT_DISTANCE_X           2
#define POMS_OPT_DISTANCE_Y           3
#define POMS_OPT_DISTANCE_Z           4
#define POMS_OPT_DISTANCE_MINUS_XYZ   5
#define POMS_OPT_DISTANCE_MINUS_CONE  6
#define POMS_OPT_DISTANCE_MINUS_X     7
#define POMS_OPT_DISTANCE_MINUS_Y     8
#define POMS_OPT_DISTANCE_MINUS_Z     9
#define POMS_OPT_DISTANCE_PLANE       10
#define POMS_OPT_DISTANCE_MINUS_PLANE 11

#define POMS_OPTIMIZATION_TILE_NONE 0
#define POMS_OPTIMIZATION_TILE_BITVEC 1

#define POMS_OPTIMIZATION_AC4_NONE      0
#define POMS_OPTIMIZATION_AC4_FLAT      1
#define POMS_OPTIMIZATION_AC4_TIER4     2
#define POMS_OPTIMIZATION_AC4_TIER4_M   3
#define POMS_OPTIMIZATION_AC4_TIER4_M2  4
#define POMS_OPTIMIZATION_AC4_TIER6     5
#define POMS_OPTIMIZATION_AC4_TIER6_M   6

#define POMS_TILE_INT int16_t
//#define POMS_TILE_INT int32_t

typedef enum {
  POMS_PHASE_UNDEF=-1,
  POMS_PHASE_CREATE=0,
  POMS_PHASE_PREFATORY,
  POMS_PHASE_START,
  POMS_PHASE_INIT,
  POMS_PHASE_STEP_BEGIN,
  POMS_PHASE_STEP_TICK,
  POMS_PHASE_STEP_END,
  POMS_PHASE_FINISH,
} POMS_PHASE;

typedef enum {
  POMS_PROF_AC4INIT=0,
  POMS_PROF_AC4UPDATE,        //1
  POMS_PROF_SAVEGRID,         //2
  POMS_PROF_SAVECELLS,        //3
  POMS_PROF_RESTOREGRID,      //4
  POMS_PROF_RESTORECELLS,     //5
} POMS_PROF;

typedef struct poms_constraint_type {
  char type;
  int32_t tile_range[2];
  int32_t size_range[3][2];

  int32_t implicit_size[3][2];
  int32_t implicit_tile[2];
} poms_constraint_t;

typedef struct tileset_ctx_type {
  int32_t imagewidth,
          imageheight,
          tilewidth,
          tileheight,
          rows,
          columns,
          tilecount;
  std::string image;
  std::string name;
} tileset_ctx_t;

typedef struct poms_quilt_patch_type {
  int32_t m_region[3][2];
  std::vector< int32_t > tile;
} poms_quilt_patch_t;

static void _init_tileset_ctx( tileset_ctx_t &ctx )  {
  ctx.imagewidth = -1;
  ctx.imageheight = -1;
  ctx.tilewidth = -1;
  ctx.tileheight = -1;
  ctx.rows = -1;
  ctx.columns = -1;
  ctx.tilecount = -1;
}

//---

static inline int32_t _tile_support_flat(void *_ac4, int32_t idir, int64_t cell, int32_t tile, int32_t val) {
  ac4_flat_t *ac4 = (ac4_flat_t *)_ac4;
  return ac4->tileSupport(idir, cell, tile, val);
}

//---

static inline int32_t _tile_support_tier4(void *_ac4, int32_t idir, int64_t cell, int32_t tile, int32_t val) {
  ac4_tier4_t *ac4 = (ac4_tier4_t *)_ac4;
  return ac4->tileSupport(idir, cell, tile, val);
}

static inline int32_t _tile_support_tier4_m1(void *_ac4, int32_t idir, int64_t cell, int32_t tile, int32_t val) {
  ac4_tier4_m1_t *ac4 = (ac4_tier4_m1_t *)_ac4;
  return ac4->tileSupport(idir, cell, tile, val);
}

static inline int32_t _tile_support_tier4_m2(void *_ac4, int32_t idir, int64_t cell, int32_t tile, int32_t val) {
  ac4_tier4_m2_t *ac4 = (ac4_tier4_m2_t *)_ac4;
  return ac4->tileSupport(idir, cell, tile, val);
}

//---

static inline int32_t _tile_support_tier6(void *_ac4, int32_t idir, int64_t cell, int32_t tile, int32_t val) {
  ac4_tier6_t *ac4 = (ac4_tier6_t *)_ac4;
  return ac4->tileSupport(idir, cell, tile, val);
}

static inline int32_t _tile_support_tier6_m1(void *_ac4, int32_t idir, int64_t cell, int32_t tile, int32_t val) {
  ac4_tier6_m1_t *ac4 = (ac4_tier6_m1_t *)_ac4;
  return ac4->tileSupport(idir, cell, tile, val);
}

//---

class POMS {
  public:

    POMS() {

      memset(m_dir_vec_incr, 0, sizeof(int32_t)*6*3);
      m_dir_vec_incr[0][0] =  1;
      m_dir_vec_incr[1][0] = -1;
      m_dir_vec_incr[0][1] =  1;
      m_dir_vec_incr[1][1] = -1;
      m_dir_vec_incr[0][2] =  1;
      m_dir_vec_incr[1][2] = -1;

      m_dir_oppo[0] = 1;
      m_dir_oppo[1] = 0;

      m_dir_oppo[2] = 3;
      m_dir_oppo[3] = 2;

      m_dir_oppo[4] = 5;
      m_dir_oppo[5] = 4;

      m_dir_desc.push_back( "x+" );
      m_dir_desc.push_back( "x-" );
      m_dir_desc.push_back( "y+" );
      m_dir_desc.push_back( "y-" );
      m_dir_desc.push_back( "z+" );
      m_dir_desc.push_back( "z-" );

      m_tile_count=0;

      //m_zero = 1.0/(1024.0*1024.0);
      //m_eps = 1.0/(1024.0*1024.0);

      m_zero = 1.0/(1024.0*1024.0*1024.0);
      m_eps = 1.0/(1024.0*1024.0*1024.0);

      m_size[0] = -1;
      m_size[1] = -1;
      m_size[2] = -1;

      m_extent[0] = -1;
      m_extent[1] = -1;
      m_extent[2] = -1;

      m_plane = 0;
      m_plane_count = 2;

      m_init_seed = 0;
      //m_seed = 0xf1c2;
      m_seed = 0;

      m_verbose = 0;

      memset(m_block, 0, sizeof(int32_t)*3*2);
      memset(m_block_size, 0, sizeof(int32_t)*3);

      memset(m_soften_block, 0, sizeof(int32_t)*3*2);
      memset(m_soften_size, 0, sizeof(int32_t)*3);

      m_retry_max = 10;
      m_retry_count = 0;

      m_rally_point[0] = -1.0;
      m_rally_point[1] = -1.0;
      m_rally_point[2] = -1.0;

      m_entropy_rand_exponent = -2.0;
      m_entropy_rand_coefficient = 0.0;

      m_prob_t = 0.0;

      m_tile_choice_policy = POMS_TILE_CHOICE_PROB;
      m_block_choice_policy = POMS_BLOCK_CHOICE_SEQUENTIAL;

      //---

      m_state_descr.push_back("POMS_STATE_INIT");
      m_state_descr.push_back("POMS_STATE_CONSISTENT");
      m_state_descr.push_back("POMS_STATE_SUCCESS");
      m_state_descr.push_back("POMS_STATE_CONFLICT");

      m_undef_str = "undef";

      _init_tileset_ctx( m_tileset_ctx );

      m_phase       = POMS_PHASE_UNDEF;
      m_hemp_policy = POMS_HEMP_POLICY_COM;

      m_tile_opt  = POMS_OPTIMIZATION_TILE_NONE;
      //m_ac4_opt   = POMS_OPTIMIZATION_AC4_NONE;

      m_tile_support_option = POMS_OPTIMIZATION_AC4_NONE;

      m_distance_modifier_opt = POMS_OPT_DISTANCE_XYZ;
      m_distance_v[0] = 1.0;
      m_distance_v[1] = 1.0;
      m_distance_v[2] = 1.0;

      m_distance_p[0] = 0;
      m_distance_p[1] = 0;
      m_distance_p[2] = 0;

      m_distance_coef = 1.0;

      m_viz_cb = NULL;

      m_constraint.clear();
      m_constraint_start_count=0;

      //m_tile_support_data_size  = (int32_t)sizeof(int16_t);
      m_tile_support_data_size  = (int32_t)sizeof(POMS_TILE_INT);
      m_tile_support_cb = _tile_support_flat;
      m_ac4_class_p[0] = &(m_ac4_flat[0]);
      m_ac4_class_p[1] = &(m_ac4_flat[1]);

      //m_tile_data_size          = (int32_t)sizeof(int16_t);
      m_tile_data_size          = (int32_t)sizeof(POMS_TILE_INT);
      //m_tile_data_size          = (int32_t)sizeof(int32_t);

      m_cell_tile_queue_data_size = (int32_t)sizeof(int32_t);

      m_g_cb = NULL;

      m_quilt_size[0] = -1;
      m_quilt_size[1] = -1;
      m_quilt_size[2] = -1;

      m_patch_region[0][0] = -1;
      m_patch_region[0][1] = -1;

      m_patch_region[1][0] = -1;
      m_patch_region[1][1] = -1;

      m_patch_region[2][0] = -1;
      m_patch_region[2][1] = -1;

      m_quilt_stitch_size[0] = 1;
      m_quilt_stitch_size[1] = 1;
      m_quilt_stitch_size[2] = 1;
    }

    std::string m_undef_str;
    const char *stateDescr(int32_t state) {
      if (state<0) { return m_undef_str.c_str(); }
      if (state>POMS_STATE_CONFLICT) { return m_undef_str.c_str(); }
      return m_state_descr[state].c_str();
    }

    //---
    // pomsExport.cpp
    //

    int loadJSONFile(std::string &fn);
    int loadJSONString(std::string &json_buf);
    int exportTiledJSON( std::string &fn, int32_t multilayer=0, int export_quilt=0 );
    int exportTiledJSON( const char *fn, int32_t multilayer=0, int export_quilt=0 );
    int exportTiledJSON( FILE *fp, int32_t multilayer=0, int export_quilt=0 );

    int loadPOMS(POMS &src_poms);
    int exportPOMSBlock(const char *fn);
    int exportPOMSGrid(const char *fn);

    int refreshConstraints();

    //---
    // pomsAlgorithm.cpp
    //

    int renew(void);

    int savePrefatory(void);

    int saveGrid(int save_plane=-1, int save_full=0);
    int restoreGrid(int save_plane=-1, int restore_full=0);

    int soften(void);
    int soften(int32_t soften_block[][2]);
    int soften_ac4(int32_t soften_block[][2]);

    int saveQuiltPatchRegion(void);
    int setupQuiltPatch(void);

    // ac4 is currently default here
    //
    int WFC(int64_t wfc_step=0);
    int WFCBlock(int64_t wfc_step=0);
    int WFCBlock(int32_t block[][2], int64_t wfc_step=0);
    int WFCBlock_ac4(int32_t block[][2], int64_t wfc_step=0);
    int WFC_HEMP(int32_t block[][2], int64_t wfc_step=0);

    int _WFCBlock_ac4(int32_t block[][2], int64_t wfc_step=0);

    double acceptHEMPProb(void);

    int MMS(int64_t mms_step=0);
    int MMSInit(void);
    int MMSBegin(void);
    int MMSStep(void);
    int MMSEnd(void);

    int BMS(int64_t max_rounds=0, int64_t bms_step=0);
    int BMSInit(void);
    int BMSBegin(void);
    int BMSStep(void);
    int BMSEnd(void);

    int POMSGo(int64_t poms_step=0);
    int POMSInit(void);
    int POMSBegin(void);
    int POMSStep(void);
    int POMSEnd(void);

    int MCMC(int64_t mcmc_step=0);
    int MCMCInit(void);
    int MCMCBegin(void);
    int MCMCStep(void);
    int MCMCEnd(void);

    int chooseBlock_sequential(int32_t block[][2], int64_t seq);
    int chooseBlock_maxEntropyBlock(int32_t block[][2], int64_t seq);
    int chooseBlock_minEntropyBlock(int32_t block[][2], int64_t seq);
    int chooseBlock_waveFront(int32_t block[][2], int64_t seq);
    int chooseBlock(int32_t block[][2], int64_t seq);

    int64_t blockSequenceCount();

    int sanityBlockEntropy(int32_t *block_size);
    int sanityQuilt(void);

    int64_t resolvedCount();
    int64_t quiltResolvedCount();

    int execPhase(int32_t phase);

    int copyState(POMS &src_poms);

    int transplantBlock(int32_t soften_block[][2], int32_t copy_block[][2], POMS &src);
    int transplantBlock_union(int32_t soften_block[][2], int32_t copy_block[][2], POMS &src);
    int transplantBlock_hard(int32_t soften_block[][2], int32_t copy_block[][2], POMS &src);

    //---
    // pomsDebug.cpp
    //

    void printDebug(int32_t show_rule=0);
    void printDebugAC4(void);
    void printDebugCellTileQueue(void);
    void printDebugBlockEntropy(void);
    void printDebugCellEntropy(void);
    void printDebugTileSizeBlock(int32_t block[][2]);

    void printDebugBlock(void);
    void printDebugGrid(void);
    void printDebugGrid(int32_t *order);
    void printDebugQuiltGrid(int32_t *order=NULL);
    void printDebugCellSize(void);
    void printDebugCellSize(int32_t *order);

    void printDebugCellFilter(void);
    void printDebugDistanceModifier(void);

    void printDebugAC4Dirty(int32_t plane);
    void printDebugFStat(void);
    void printDebugMemStat(void);

    void printDebugQuilt(void);
    void printDebugPin(void);
    void printDebugCellUID(void);

    void printDebugSpotCheck(void);

    void printDebugConflict(void);

    int64_t countClusters(void);


    //---
    // pomsConstraintPropagate.cpp
    //

    int removeTileIdx(int plane, int64_t cell, int32_t tile_idx);

    int queueCell(int64_t cell);
    int queueCellNeighbors( int64_t cell );

    int unwindQueue(void);
    int unwindVisited(void);
    int sanityArcConsistency();
    int sanityCellTileQueueVisited();

    int64_t realizedCellsWithinBlock(int32_t b[][2]);
    int64_t realizedCellsWithinBlock(void);
    int64_t realizedCells(void);

    int removeTile(int plane, int64_t cell, int32_t tile);
    int addTile(int plane, int64_t cell, int32_t tile);
    int forceTile(int plane, int64_t cell, int32_t tile);

    //EXPERIMENT
    int pinCell(int plane, int64_t cell, int32_t tile);

    int applyConstraints(int apply_start_constraints=0, int apply_quilt_constraints=0);
    int applyStartConstraints(void);
    int applyQuiltConstraints(void);

    //int cullSweep_ac4();
    int cullSweep();

    int AC4Init(int memoize_opt=0);
    int AC4InitBlock(int32_t block[][2], int memoize_opt=0);
    int AC4Update(void);

    int AC4Consistency(void);

    //---
    // pomsEntropy.cpp
    //

    int computeCellEntropy(void);
    int computeCellEntropyWithinBlock(void);
    int computeCellEntropyWithinBlock(int32_t block[][2], int dirty_opt=0);

    int computeBlockEntropy(int32_t reuse_cell_entropy=0);
    int computeBlockEntropy(int32_t *block_size, int32_t reuse_cell_entropy=0);

    int computeDistanceModifier_resolved(void);

    int computeDistanceModifier_xyz(double a, double b, double c);
    int computeDistanceModifier_xyz(double a, double b, double c, double cx, double cy, double cz, double coef=1.0);
    int computeDistanceModifier_cone(void);
    int computeDistanceModifier_minus_cone(void);
    int computeDistanceModifier(void);

    int pickCellMinEntropy(int64_t *min_cell,
                           int32_t *tile_idx,
                           double *min_entropy);
    int pickCellMinEntropyWithinBlock(int64_t *min_cell,
                                      int32_t *tile_idx,
                                      double *min_entropy);
    int pickCellMinEntropyWithinBlock(int64_t *min_cell,
                                      int32_t *tile_idx,
                                      double *min_entropy,
                                      int32_t block[][2]);

    int entropyCenterOfMass(double *com,
                            int32_t block[][2]);

    int entropyBlock(double *S, int32_t block[][2]);
    int entropy(double *S);

    int dist( double *d, POMS &B );
    int distJaccard( double *d, POMS &B );
    int distCosine( double *d, POMS &B );

    //---

    int64_t vec2cell( int32_t *v, int32_t *_size ) {
      if ((v[0] < 0) || (v[0] >= _size[0]) ||
          (v[1] < 0) || (v[1] >= _size[1]) ||
          (v[2] < 0) || (v[2] >= _size[2])) {
        return -1;
      }
      return (v[2]*_size[0]*_size[1]) + (v[1]*_size[0]) + v[0];
    }
    int64_t xyz2cell(int32_t x, int32_t y, int32_t z, int32_t *_size) {
      int32_t v[3];
      v[0] = x; v[1] = y; v[2] = z;
      return vec2cell(v, _size);
    }


    int64_t vec2cell( int32_t *v ) {
      if ((v[0] < 0) || (v[0] >= m_size[0]) ||
          (v[1] < 0) || (v[1] >= m_size[1]) ||
          (v[2] < 0) || (v[2] >= m_size[2])) {
        return -1;
      }
      return (v[2]*m_size[0]*m_size[1]) + (v[1]*m_size[0]) + v[0];
    }
    int64_t vec2cell( std::vector< int32_t > &v ) { return vec2cell( &(v[0]) ); }
    int64_t xyz2cell(int32_t x, int32_t y, int32_t z) {
      int32_t v[3];
      v[0] = x; v[1] = y; v[2] = z;
      return vec2cell(v);
    }


    int cell2vec( int32_t *v, int64_t cell, int32_t *_size ) {
      int64_t _s;
      _s = ((int64_t)_size[0])*((int64_t)_size[1])*((int64_t)_size[2]);
      if ((cell < 0) || (cell >= _s)) { return -1; }
      v[2] = (int32_t)(cell / (_size[0]*_size[1]));
      cell -= v[2]*_size[0]*_size[1];
      v[1] = (int32_t)(cell / _size[0]);
      cell -= v[1]*_size[0];
      v[0] = (int32_t)cell;
      return 0;
    }

    int cell2vec( int32_t *v, int64_t cell ) {
      if ((cell < 0) ||
          (cell >= m_cell_count)) { return -1; }
      v[2] = (int32_t)(cell / (m_size[0]*m_size[1]));
      cell -= v[2]*m_size[0]*m_size[1];
      v[1] = (int32_t)(cell / m_size[0]);
      cell -= v[1]*m_size[0];
      v[0] = (int32_t)cell;
      return 0;
    }
    int cell2vec( std::vector< int32_t > &v, int64_t cell ) { return cell2vec( &(v[0]), cell ); }

    int64_t neiCell( int64_t cell, int32_t idir ) {
      int64_t nei_cell=-1;
      int32_t v[3] = {0};

      if (cell2vec(v, cell) < 0) { return -1; }
      switch (idir) {
        case 0: v[0]++; break;
        case 1: v[0]--; break;
        case 2: v[1]++; break;
        case 3: v[1]--; break;
        case 4: v[2]++; break;
        case 5: v[2]--; break;
      };
      nei_cell = vec2cell( v );

      return nei_cell;
    }

    int64_t neiCell( int64_t cell, int32_t idir, int32_t *_size ) {
      int64_t nei_cell=-1;
      int32_t v[3] = {0};

      if (cell2vec(v, cell, _size) < 0) { return -1; }
      switch (idir) {
        case 0: v[0]++; break;
        case 1: v[0]--; break;
        case 2: v[1]++; break;
        case 3: v[1]--; break;
        case 4: v[2]++; break;
        case 5: v[2]--; break;
      };
      nei_cell = vec2cell( v, _size );

      return nei_cell;
    }


    //-------------
    //-------------
    // rule functions
    //

    void _setF_i8(int32_t tile_src, int32_t tile_dst, int32_t idir, double val ) {
      int32_t t = m_tile_count,
              tt = m_tile_count*m_tile_count;
      //m_tile_rule[ (idir*tt) + (tile_src*t) + tile_dst ] = val;
      //m_tile_rule[ (idir*tt) + (tile_src*t) + tile_dst ] = (int8_t)val;
      m_tile_rule[ (idir*tt) + (tile_src*t) + tile_dst ] = (uint8_t)val;
    }

    double  _F_i8(int32_t tile_src, int32_t tile_dst, int32_t idir ) {
      int32_t t = m_tile_count,
              tt = m_tile_count*m_tile_count;
      return (double)m_tile_rule[ (idir*tt) + (tile_src*t) + tile_dst ];
    }

    double  F_i8(int32_t tile_src, int32_t tile_dst, int32_t idir, double val = -1.0 ) {
      int32_t t = m_tile_count,
              tt = m_tile_count*m_tile_count;
      //if (val >= -m_zero) { m_tile_rule[ (idir*tt) + (tile_src*t) + tile_dst ] = (int8_t)val; }
      return (double)m_tile_rule[ (idir*tt) + (tile_src*t) + tile_dst ];
    }

    double  setF_bv(int32_t tile_src, int32_t tile_dst, int32_t idir, double val) {
      int32_t t = m_tile_count,
              tt = m_tile_count*m_tile_count;
      int32_t n_q, n_r, p;
      uint8_t u, v;

      p = ((idir*tt) + (tile_src*t) + tile_dst);
      n_q = p / 8;
      n_r = p % 8;

      u = (1<<n_r);
      v = m_tile_rule[n_q];
      if (val < m_zero) {
        v = v & (((uint8_t)0xff) ^ u);
      }
      else {
        v = v | u;
      }
      m_tile_rule[n_q] = v;

      if (val > m_zero) { return 1.0; }
      return 0.0;
    }

    double  F_bv(int32_t tile_src, int32_t tile_dst, int32_t idir) {
      int32_t t = m_tile_count,
              tt = m_tile_count*m_tile_count;
      int32_t n_q, n_r, p;
      uint8_t u;

      p = ((idir*tt) + (tile_src*t) + tile_dst);
      n_q = p / 8;
      n_r = p % 8;

      u = (1<<n_r);
      if (m_tile_rule[n_q] & u) { return 1.0; }
      return 0.0;

      //return (double)m_tile_rule[ (idir*tt) + (tile_src*t) + tile_dst ];
    }

    double  setF(int32_t tile_src, int32_t tile_dst, int32_t idir, double val) {
      return setF_bv(tile_src, tile_dst, idir, val);
    }

    double  F(int32_t tile_src, int32_t tile_dst, int32_t idir) {
      return F_bv(tile_src, tile_dst, idir);
    }


    //-------------
    //-------------


    void setG(int32_t tile, double p)   {        m_tile_weight[ tile ] = p; }
    double  G(int32_t tile)             { return m_tile_weight[ tile ]; }

    double G_cb(int32_t tile) {
      if (m_g_cb) { return m_g_cb(this, -1, tile); }
      return G(tile);
    }

    double G_cb(int64_t cell, int32_t tile) {
      if (m_g_cb) { return m_g_cb(this, cell, tile); }
      return G(tile);
    }


    //---

    int64_t cellTileQueuePush(int32_t plane, int64_t cell, int32_t tile) {
      int64_t n_queue;
      n_queue = m_cell_tile_queue_size[plane];
      if (n_queue >= (m_tile_count*m_cell_count*2)) { return -1; }
      if ((n_queue % 2) != 0) { return -2; }
      m_cell_tile_queue[plane][n_queue+0] = cell;
      m_cell_tile_queue[plane][n_queue+1] = (int64_t)tile;
      m_cell_tile_queue_size[plane] += 2;
      return n_queue+2;
    }

    /*
    int64_t cellTileQueuePush(int32_t plane, int64_t cell, int32_t tile) {
      int64_t n_queue, ns;

      if (m_ac4_opt < 2) { return _cellTileQueuePush(plane, cell, tile); }

      n_queue = m_cell_tile_queue_size[plane];
      if (n_queue >= (m_tile_count*m_cell_count*2)) { return -1; }
      if (n_queue >= m_cell_tile_queue[plane].size()) {

        ns = (2+n_queue)*2;
        if (ns > (m_tile_count*m_cell_count*2)) {
          ns = m_tile_count*m_cell_count*2;
        }

        m_cell_tile_queue[plane].resize( ns );

        //DEBUG
        //printf("### queue now %in", (int)m_cell_tile_queue[plane].size());
      }

      return _cellTileQueuePush(plane, cell, tile);
    }
    */

    int64_t cellTileQueuePop(int32_t plane, int64_t *cell, int32_t *tile) {
      int64_t n_queue, c, t;
      n_queue = m_cell_tile_queue_size[plane];
      if (n_queue < 2) { return -1; }
      if ((n_queue % 2) != 0) { return -2; }

      n_queue -= 2;
      if (cell) { *cell = m_cell_tile_queue[plane][n_queue+0]; }
      if (tile) { *tile = (int32_t)m_cell_tile_queue[plane][n_queue+1]; }
      m_cell_tile_queue_size[plane] -= 2;
      return n_queue;
    }

    int cellTileQueueClear(int32_t plane) {
      m_cell_tile_queue_size[plane] = 0;
      return 0;
    }

    int cellTileQueuePeek(int32_t plane, int64_t qpos, int64_t *cell, int32_t *tile) {
      *cell = (int64_t)m_cell_tile_queue[plane][qpos+0];
      *tile = (int32_t)m_cell_tile_queue[plane][qpos+1];
      return 0;
    }

    int64_t cellTileQueueSize(int32_t plane) {
      return m_cell_tile_queue_size[plane];
    }

    //--
    //--

    int cellQueuePop(int32_t plane, int64_t *cell) {
      int64_t n_queue, c, t;
      n_queue = m_cell_queue_size[plane];
      if (n_queue < 1) { return -1; }
      n_queue--;
      if (cell) { *cell = m_cell_queue[plane][n_queue]; }
      m_cell_queue_size[plane] --;
      return n_queue;
    }

    int64_t cellQueuePush(int32_t plane, int64_t cell) {
      int64_t n_queue;
      n_queue = m_cell_queue_size[plane];
      if (n_queue >= m_cell_count) { return -1; }
      m_cell_queue[plane][n_queue] = cell;
      m_cell_queue_size[plane]++;
      return n_queue+1;
    }


    int cellQueuePeek(int32_t plane, int64_t qpos, int64_t *cell) {
      *cell = (int64_t)m_cell_queue[plane][qpos+0];
      return 0;
    }

    int64_t cellQueueSize(int32_t plane) {
      return m_cell_queue_size[plane];
    }

    int8_t cellVisited(int32_t plane, int64_t cell, int8_t val=-1) {
      if (val>=0) {
        m_visited[plane][cell] = val;
      }
      return m_visited[plane][cell];
    }

    int8_t cellVisitedClear(int32_t plane) {
      memset( &(m_visited[plane][0]), 0, sizeof(int8_t)*m_cell_count);
      return 0;
    }

    //--
    //--

    int8_t cellTileVisited(int32_t plane, int64_t cell, int32_t tile, int8_t val=-1) {
      if (val >= 0) {
        m_cell_tile_visited[plane][ (cell*m_tile_count) + tile ] = val;
      }
      return m_cell_tile_visited[plane][ (cell*m_tile_count) + tile ];
    }

    //--

    int cellTileVisitedClear(int32_t plane) {
      memset( &(m_cell_tile_visited[plane][0]), 0, sizeof(int8_t)*m_cell_count*m_tile_count);
      return 0;
    }


    //--

    int32_t _tileSupport(int32_t plane, int64_t pos, int32_t val=-1) {
      if (val >= 0) { m_tile_support[plane][pos] = val; }
      return (int32_t)m_tile_support[plane][pos];
    }

    int32_t _tileSupport(int32_t plane, int32_t idir, int64_t cell, int32_t tile, int32_t val=-1) {
      int64_t p;
      p = (idir*m_cell_count*m_tile_count) + (cell*m_tile_count) + tile;
      return _tileSupport(plane, p, val);
    }

    int32_t tileSupport(int32_t plane, int32_t idir, int64_t cell, int32_t tile, int32_t val=-1) {
      if (m_tile_support_option == POMS_OPTIMIZATION_AC4_NONE) {
        return _tileSupport(plane, idir, cell, tile, val);
      }
      return m_tile_support_cb( (void *)(m_ac4_class_p[plane]), idir, cell, tile, val );
    }

    int32_t (*m_tile_support_cb)( void *, int32_t, int64_t, int32_t, int32_t );

    void            *m_ac4_class_p[2];
    ac4_flat_t      m_ac4_flat[2];
    ac4_tier4_t     m_ac4_tier4[2];
    ac4_tier4_m1_t  m_ac4_tier4_m1[2];
    ac4_tier4_m2_t  m_ac4_tier4_m2[2];
    ac4_tier6_t     m_ac4_tier6[2];
    ac4_tier6_m1_t  m_ac4_tier6_m1[2];


    //---

    int resetAC4Dirty(int32_t plane) {
      memset( &(m_ac4_dirty[plane][0]), 0, sizeof(int8_t)*m_cell_count );
      return 0;
    }

    int markAC4Dirty(int32_t plane, int64_t cell) {
      m_ac4_dirty[m_plane][cell] = 1;
      return 0;
    }

    //---

    // populate `m_tile_cdf` with cumulative distribution function
    // for individual tile probabilities.
    //
    // Return:
    //
    //  0 - success
    // <0 - error
    //
    // Error indicates either `m_tile_count` is 0 or
    // normalization constant is 0.
    //
    // pretty useless function
    //
    int computeTileCDF(void) {
      int32_t tile;
      double R=0.0;
      if (m_tile_count < 1) { return -1; }
      m_tile_cdf.resize( m_tile_count );
      for (tile=0; tile<m_tile_count; tile++) {

        //R += G(tile);
        R += G_cb(tile);

        m_tile_cdf[tile] = R;
      }
      if (R < m_zero) { return -2; }
      for (tile=0; tile<m_tile_count; tile++) {
        m_tile_cdf[tile] /= R;
      }
      m_tile_cdf[m_tile_count-1] = 1.0;
      return 0;
    }


    //---

    // POMS_OPTIMIZATION_TILE_NONE
    //

    void setCellTile( int32_t plane, int64_t cell, int32_t tile_idx, int32_t tile_val ) {
      m_tile[plane][ (cell*m_tile_count) + tile_idx ] = tile_val;
      m_tile_bp[plane][ (cell*m_tile_count) + tile_val ] = tile_idx;
    }

    int32_t cellTile( int32_t plane, int64_t cell, int32_t tile_idx ) {
      return m_tile[plane][ (cell*m_tile_count) + tile_idx ];
    }

    int32_t cellTileIndex( int32_t plane, int64_t cell, int32_t tile_val ) {
      return m_tile_bp[plane][ (cell*m_tile_count) + tile_val ];
    }


    // POMS_OPTIMIZATION_TILE_BITVEC

    /*
    void setCellTile_bv( int32_t plane, int64_t cell, int32_t tile, uint8_t val ) {
      uint8_t u8, p;
      int64_t idx, byte_idx;

      idx = ((cell*m_tile_count) + tile);
      byte_idx = idx / 8;
      p = (uint8_t)( idx % 8 );
      u8 = m_tile_bv[plane][byte_idx];
      if (val==0) { u8 &= ~(1<<p); }
      m_tile[plane][ (cell*m_tile_count) + tile_idx ] = tile_val;
      m_tile_bp[plane][ (cell*m_tile_count) + tile_val ] = tile_idx;
    }

    int32_t cellTile_bv( int32_t plane, int64_t cell, int32_t tile_idx ) {
      return m_tile[plane][ (cell*m_tile_count) + tile_idx ];
    }

    int32_t cellTileIndex_bv( int32_t plane, int64_t cell, int32_t tile_val ) {
      return m_tile_bp[plane][ (cell*m_tile_count) + tile_val ];
    }
    */



    //--

    void setCellSize( int32_t plane, int64_t cell, int32_t size ) {
      m_tile_size[plane][ cell ] = size;
    }

    int32_t cellSize( int32_t plane, int64_t cell ) {
      return m_tile_size[plane][ cell ];
    }

    //--

    int32_t tileAdjIdxSize( int32_t tile, int32_t idir ) {
      return (int32_t)m_tileAdj[(idir*m_tile_count) + tile].size();
    }

    int32_t tileAdjIdx( int32_t tile, int32_t idir, int32_t nei_tile_idx ) {
      return m_tileAdj[(idir*m_tile_count) + tile][ nei_tile_idx ];
    }

    //--

    void setCellBufTile( int32_t *buf, int64_t cell, int32_t tile_idx, int32_t tile_val ) {
      buf[ (cell*m_tile_count) + tile_idx ] = tile_val;
    }

    int32_t _cellBufTile32( int32_t *buf, int64_t cell, int32_t tile_idx ) {
      return buf[ (cell*m_tile_count) + tile_idx ];
    }
    int32_t _cellBufTile16( int16_t *buf, int64_t cell, int32_t tile_idx ) {
      return buf[ (cell*m_tile_count) + tile_idx ];
    }

    //int32_t cellBufTile( int16_t *buf, int64_t cell, int32_t tile_idx ) {
    int32_t cellBufTile( POMS_TILE_INT *buf, int64_t cell, int32_t tile_idx ) {
      return buf[ (cell*m_tile_count) + tile_idx ];
    }


    void setCellBufSize( int32_t *buf, int64_t cell, int32_t size ) {
      buf[ cell ] = size;
    }

    int32_t cellBufSize32( int32_t *buf, int64_t cell ) { return buf[ cell ]; }
    int32_t cellBufSize16( int16_t *buf, int64_t cell ) { return buf[ cell ]; }
    //int32_t cellBufSize  ( int16_t *buf, int64_t cell ) { return buf[ cell ]; }
    int32_t cellBufSize  ( POMS_TILE_INT *buf, int64_t cell ) { return buf[ cell ]; }

    //---

    void maximizeBlock(void) {
      m_block[0][0] = 0;
      m_block[1][0] = 0;
      m_block[2][0] = 0;

      m_block[0][1] = m_size[0];
      m_block[1][1] = m_size[1];
      m_block[2][1] = m_size[2];

      m_block_size[0] = m_size[0];
      m_block_size[1] = m_size[1];
      m_block_size[2] = m_size[2];
    }

    void clampParameters(void) {
      if (m_size[0] > m_quilt_size[0]) { m_size[0] = m_quilt_size[0]; }
      if (m_size[1] > m_quilt_size[1]) { m_size[1] = m_quilt_size[1]; }
      if (m_size[2] > m_quilt_size[2]) { m_size[2] = m_quilt_size[2]; }

      if (m_block_size[0] > m_size[0]) { m_block_size[0] = m_size[0]; }
      if (m_block_size[1] > m_size[1]) { m_block_size[1] = m_size[1]; }
      if (m_block_size[2] > m_size[2]) { m_block_size[2] = m_size[2]; }

      if (m_soften_size[0] > m_size[0]) { m_soften_size[0] = m_size[0]; }
      if (m_soften_size[1] > m_size[1]) { m_soften_size[1] = m_size[1]; }
      if (m_soften_size[2] > m_size[2]) { m_soften_size[2] = m_size[2]; }
    }

    //---

    void rnd_seed(unsigned long _seed) {
      m_init_seed = 1;
      m_seed = _seed;
      srand48(m_seed);
    }

    double rnd(void) {
      if (!m_init_seed) { rnd_seed(m_seed); }
      return drand48();
    }

    int32_t irnd(int32_t N=2) {
      if (!m_init_seed) { rnd_seed(m_seed); }
      return (int32_t) ((double)N*drand48());
    }

    double noiseUniform(double x1) { return x1*rnd(); }
    double noiseUniform(double x0, double x1) { return x0 + (x1-x0)*rnd(); }
    double _noisePowerLaw(double C=1.0, double E=-2.0, double x0=(1.0/1024.0), double x1=(1024.0*1024.0)) {
      double y, z, x0en1, x1en1;
      y = noiseUniform(1.0);
      x0en1 = pow(x0, E+1.0);
      x1en1 = pow(x1, E+1.0);
      z = (y*(x1en1 - x0en1)) + x0en1;
      z = pow(z, 1.0 / (E+1.0));
      z *= C;
      return z;
    }

    double noisePowerLaw(double C=1.0, double E=-2.0) {
      static double x0en1=0.0, x1en1=0.0, Esave=0.0;
      double y, z;
      double x0=(1.0/1024.0), x1=(1024.0*1024.0);

      double de;

      de = Esave - E;
      if (de<0) { de = -de; }

      if (de > m_zero) {
        Esave = E;
        x0en1 = pow(x0, E+1.0);
        x1en1 = pow(x1, E+1.0);
      }
      y = noiseUniform(1.0);
      z = (y*(x1en1 - x0en1)) + x0en1;
      z = pow(z, 1.0 / (E+1.0));
      z *= C;
      return z;
    }

    //--

    // returns:
    //
    //  0 : cell does not have tile
    //  1 : cell has tile
    // <0 : error
    //
    int cellHasTile(int plane, int64_t cell, int32_t tile) {
      int32_t t, p;

      if ((tile < 0) || (tile >= m_tile_count)) { return -1; }
      if ((cell < 0) || (cell >= m_cell_count)) { return -2; }

      p = m_tile_bp[plane][ (cell*m_tile_count) + tile ];
      if ((p < 0) || (p >= m_tile_count)) { return -3; }

      if (p >= m_tile_size[plane][cell]) { return 0; }
      if (m_tile[plane][ (cell*m_tile_count) + p ] == tile) { return 1; }
      return 0;
    }

    //---

    int             m_init_seed;
    unsigned long   m_seed;
    double          m_zero,
                    m_eps;

    int32_t                     m_plane;
    int32_t                     m_plane_count;

    std::vector< std::string >  m_dir_desc;

    int32_t                     m_dir_oppo[6];
    int64_t                     m_dir_cell_incr[6];
    int32_t                     m_dir_vec_incr[6][3];

    // `m_size` is the 'admissible' region to find solutions.
    // `m_extent` is the physical workable area that includes
    //   boundary conditions explicitely
    //
    // Just using `m_size` for now...need to think if
    //   there's a good way to incorporate `m_extent`
    //
    int32_t                     m_size[3];
    int32_t                     m_extent[3];

    int32_t                     m_tile_count;
    std::vector< std::string >  m_tile_name;

    int64_t                     m_cell_count;

    // originally, m_tile_rule was meant to be a double
    // array but for tilesets which are 10k+, this
    // turns into upwards of a Gb+ just to hold the $D^2$
    // relations. Often, we only want a straight unweighted
    // indication, so int8_t is a fine compromise for this.
    //
    // Note that this is in addition to global tile weights
    // (the `m_tile_weight` arrya below and the `G` function
    // above).
    //
    // commented out double as a reminder.
    //
    //std::vector< double >       m_tile_rule;
    //std::vector< int8_t >       m_tile_rule;
    std::vector< uint8_t >       m_tile_rule;

    // global weighting of tiles (`G`)
    //
    std::vector< double >       m_tile_weight;

    std::vector< int32_t >      m_tile_group;

    //---

    // for STL output, maps tile index to object file location
    //
    std::vector< std::string >  m_objMap;

    //---

    // m_tile             - tile values at each cell position. |m_tile[cell]| = m_tile_size[cell] (|cell| * |tile|)
    // m_tile_bp          - index of tile in m_tile (|cell| * |tile|)
    // m_tile_size        - number of tiles at cell for m_tile (|cell|)
    //
    //std::vector< int16_t >      m_tile[2];
    //std::vector< int16_t >      m_tile_bp[2];
    //std::vector< int16_t >      m_tile_size[2];

    std::vector< POMS_TILE_INT >      m_tile[2];
    std::vector< POMS_TILE_INT >      m_tile_bp[2];
    std::vector< POMS_TILE_INT >      m_tile_size[2];

    std::vector< uint8_t >      m_bv_tile[2];
    std::vector< uint8_t >      m_bv_tile_count[2];
    int32_t                     m_bv_tile_count_stride;

    double (*m_g_cb)(POMS *, int64_t, int32_t);

    //-------
    //- >>quilt
    //-------

    // `m_quilt_tile`       : store resolved tiles for out of core patch and stitch.
    // `m_quilt_size`       : size of quilt region (at least as big as `m_size`)
    // `m_quilt_stitch_size`: size of stitch band. For now, this should be 1 in
    //                        each dimension
    //
    // |m_quilt_tile| = m_quilt_size[0] * m_quilt_size[1] * m_quilt_size[2]
    //
    // m_quilt_tile[quilt_cell]:
    //
    //  -1 : element unassigned
    // >=0 : tile assigned
    //
    //std::vector< int16_t >      m_quilt_tile;
    std::vector< POMS_TILE_INT >      m_quilt_tile;
    std::vector< int8_t >       m_quilt_pin;
    int32_t                     m_quilt_size[3];
    int32_t                     m_quilt_stitch_size[3];

    int64_t                     m_quilt_cell_count;

    std::vector< poms_quilt_patch_t > m_quilt_patch;

    // current working region as it fits into quilt
    //
    // `m_patch_region` : [
    //   [ Xstart, Xend ],
    //   [ Ystart, Yend ],
    //   [ Zstart, Zend ]
    // ]
    //
    // [XYZ]end are non inclusive
    //
    // patch region is inclusive of stitch if the region
    // does not fall on an larger quilt edge boundary.
    // Stitch buffer region will be of `m_quilt_stitch_size`
    // on each edge that does not fall on a boundary.
    // If patch region falls on edge boundary, stitches will
    // not exist.
    //
    // Here's some ASCII art to illustrate:
    //
    // .-----   .----    .-----
    // |sss     | sss    |  
    // |s..     | ...    | ...
    // |s..     | ...    | ...
    // |        |        | ...
    //
    // `s` are stitched pinned cells
    // `.` are free cells
    //
    int32_t                     m_patch_region[3][2];

    // map of cells to pin and leave untouched by:
    // - support
    // - resolution
    // - etc.
    //
    //  0 : unpinned
    //  1 : pinned
    //
    std::vector< int8_t >       m_cell_pin;

    //-------
    //- <<quilt
    //-------


    //std::vector< int32_t >      m_prefatory;
    //std::vector< int16_t >      m_prefatory;
    //std::vector< int16_t >      m_prefatory_size;

    std::vector< POMS_TILE_INT >  m_prefatory;
    std::vector< POMS_TILE_INT >  m_prefatory_size;

    int32_t                     m_tile_data_size;

    // m_visited          - indicator of whether cell has been visited (|cell|)
    // m_cell_queue       - array of (cell) values to process (|cell|)
    // m_cell_queue_size  - size of m_cell_queue
    //
    std::vector< int8_t >       m_visited[2];
    std::vector< int64_t >      m_cell_queue[2];
    int64_t                     m_cell_queue_size[2];

    // >>>ac4
    //
    // AC4 supporting structures
    //
    // m_tile_support         - support for tile at cell for each direction (|cell| * |tile| * |dir|)
    // m_cell_tile_visited    - indicator whether (cell,tile,dir) has been visited (|cell| * |tile| * |dir|)
    // m_cell_tile_queue      - array of (cell,tile) values to process (|cell| * |tile|)
    // m_cell_tile_queue_size - size of m_cell_tile_queue
    //
    //std::vector< int16_t >      m_tile_support[2];
    std::vector< POMS_TILE_INT >  m_tile_support[2];
    std::vector< int8_t >         m_cell_tile_visited[2];

    int32_t                     m_tile_support_data_size;


    int32_t                     m_tile_support_option;
    //ac4_tier_t                  m_ac4_tier;

    //std::vector< int64_t >      m_cell_tile_queue[2];
    //int64_t                     m_cell_tile_queue_size[2];

    //std::vector< int64_t >      m_cell_tile_queue[1];
    std::vector< int32_t >      m_cell_tile_queue[1];
    int64_t                     m_cell_tile_queue_size[1];

    int32_t                     m_cell_tile_queue_data_size;

    //
    // <<<ac4

    std::vector< double >       m_entropy;
    std::vector< double >       m_block_entropy;
    std::vector< double >       m_distance_modifier;
    double                      m_distance_v[3],
                                m_distance_p[3],
                                m_distance_coef;

    //----

    int32_t                     m_block_size[3];
    int32_t                     m_block[3][2];

    int32_t                     m_soften_size[3];
    int32_t                     m_soften_block[3][2];

    int32_t                     m_soften_pos[3];

    int64_t                     m_seq;
    int32_t                     m_state;

    int32_t                     m_retry_max;
    int32_t                     m_retry_count;

    double                      m_entropy_rand_coefficient;
    double                      m_entropy_rand_exponent;

    double                      m_rally_point[3];
    double                      m_block_hemp_point[3];
    double                      m_orig_block_entropy;

    // probability interpolation
    //
    double                      m_prob_t;
    std::vector< double >       m_tile_cdf;

    int32_t                     m_block_choice_policy;
    int32_t                     m_tile_choice_policy;
    int32_t                     m_hemp_policy;

    int64_t                     m_conflict_count;

    std::vector< int8_t >       m_ac4_dirty[2];

    // inverse temperature
    //
    double                      m_beta;

    //----
    // region quilting DEFUNKT!!!
    //----

    void  entropyFilter(double threshold, int reuse_cell_entropy=0);
    int   assignRegionIDs(void);

    std::vector< int8_t >       m_cell_filter;
    double                      m_cell_filter_threshold;
    //std::vector< int64_t >      m_cell_region_id;

    //----

    std::vector< poms_constraint_t >  m_constraint;
    int32_t                           m_constraint_start_count;

    //std::vector< int64_t >            m_cell_uid;
    //int64_t                           m_cell_uid_count;

    //std::vector< int16_t >            m_ac4_memoize[6];

    /*
    int resetMemoize() {
      memset( &(m_cell_uid[0]), 0, sizeof(int64_t)*m_cell_count);
      m_cell_uid_count=0;

      memset( &(m_ac4_memoize[0][0]), -1, m_tile_data_size*m_tile_count );
      memset( &(m_ac4_memoize[1][0]), -1, m_tile_data_size*m_tile_count );
      memset( &(m_ac4_memoize[2][0]), -1, m_tile_data_size*m_tile_count );
      memset( &(m_ac4_memoize[3][0]), -1, m_tile_data_size*m_tile_count );
      memset( &(m_ac4_memoize[4][0]), -1, m_tile_data_size*m_tile_count );
      memset( &(m_ac4_memoize[5][0]), -1, m_tile_data_size*m_tile_count );

      return 0;
    }
    */

    //----

    int64_t     m_conflict_cell;
    int32_t     m_conflict_tile;
    int32_t     m_conflict_idir;
    int32_t     m_conflict_type;

    int32_t     m_verbose;
    int32_t     m_phase;

    std::vector< std::vector< int32_t > > m_tileAdj;

    std::vector< std::string > m_state_descr;

    tileset_ctx_t m_tileset_ctx;
    //int32_t     m_ac4_opt;
    int32_t     m_distance_modifier_opt;

    int32_t     m_tile_opt;

    int (*m_viz_cb)(void);

    //---

    double _tv2ms(struct timeval &tv) {
      double dt;

      dt = (tv.tv_sec*1000000.0) + (tv.tv_usec);
      dt /= 1000.0;

      return dt;
    }

    int _prof_find(std::string &s) {
      int i;

      for (i=0; i<m_prof_name.size(); i++) {
        if (m_prof_name[i] == s) { return i; }
      }
      return -1;
    }

    int _prof_resize(int new_size) {
      m_prof_s.resize(new_size);
      m_prof_e.resize(new_size);
      m_prof_name.resize(new_size);
      m_prof_ms.resize(new_size);
      m_prof_count.resize(new_size);
      return 0;
    }

    int _prof_create(std::string &s) {
      struct timeval tv = {0};
      int idx;

      idx = _prof_find(s);
      if (idx >= 0) { return idx; }

      idx = m_prof_s.size();
      _prof_resize( idx+1 );
      m_prof_name[idx] = s;
      return idx;
    }

    int _prof_start(int prof_idx) {
      int r;

      if (prof_idx >= m_prof_s.size()) {
        _prof_resize(prof_idx+1);
      }

      r = gettimeofday( &(m_prof_s[prof_idx]), NULL );
      return r;
    }

    double _prof_end(int prof_idx) {
      int r;
      double dt, prv_n, cur_n;

      if (prof_idx >= m_prof_s.size()) {
        _prof_resize(prof_idx+1);
      }

      r = gettimeofday( &(m_prof_e[prof_idx]), NULL );
      if (r<0) { return -1.0; }

      dt = _tv2ms(m_prof_e[prof_idx]) - _tv2ms(m_prof_s[prof_idx]);
      //dt = ((m_prof_e[prof_idx].tv_sec  - m_prof_s[prof_idx].tv_sec)*1000000.0) +
      //      (m_prof_e[prof_idx].tv_usec - m_prof_s[prof_idx].tv_usec);

      prv_n = (double)m_prof_count[prof_idx];
      m_prof_count[prof_idx]++;
      cur_n = (double)m_prof_count[prof_idx];

      m_prof_ms[prof_idx] = ((m_prof_ms[prof_idx]*prv_n) + dt)/cur_n;

      return m_prof_ms[prof_idx];
    }

    void _prof_print(int idx) {
      printf("# prof[%i]{%s}: %fms (/%lli)\n",
          idx, m_prof_name[idx].c_str(),
          m_prof_ms[idx], (long long int)m_prof_count[idx]);
    }

    void _prof_print(void) {
      int idx;
      for (idx=0; idx<m_prof_ms.size(); idx++) {
        printf("# prof[%i]{%s}: %fms (/%lli)\n",
            idx, m_prof_name[idx].c_str(),
            m_prof_ms[idx], (long long int)m_prof_count[idx]);
      }
    }

    std::vector< struct timeval >   m_prof_s,
                                    m_prof_e;
    std::vector< double >           m_prof_ms;
    std::vector< std::string >      m_prof_name;
    std::vector< int64_t >          m_prof_count;

};


#endif
