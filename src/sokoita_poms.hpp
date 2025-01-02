/*
 * To the extent possible under law, the person who associated CC0 with
 * this file has waived all copyright and related or neighboring rights
 * to this file.
 *      
 * You should have received a copy of the CC0 legalcode along with this
 * work.  If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 *
 */

#ifndef MAIN_POMS_HPP
#define MAIN_POMS_HPP

#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>
#include <getopt.h>

#include "poms.hpp"

#include "auxExport.hpp"
#include "TiledExport.hpp"

#define SOKOITA_POMS_BIN_VERSION "0.18.15"

enum NOISE_TYPE {
  NOISE_TYPE_NONE = 0,
  NOISE_TYPE_LINEAR,
  NOISE_TYPE_THRESHOLD,
  NOISE_TYPE_TIERED,
};

enum NOISE_PRESET_TYPE {
  NOISE_PRESET_TYPE_NONE = 0,
  NOISE_PRESET_TYPE_SIMPLE,
  NOISE_PRESET_TYPE_VECTOR,
};

typedef struct g_ctx_type {
  POMS *poms;
  tiled_export_t T;

  // these might be more appropriate in the opt structure but are here
  // presumably because the snapshots are intrastep dependent and not
  // final output of the run.
  //
  std::string tiled_snapshot_fn;
  std::string sliced_tiled_snapshot_fn;
  std::string patch_snapshot_fn;
  std::string exploded_tiled_snapshot_fn;

  tiled_export_t explodedTiled;
  int32_t exploded_tile_size[2],
          exploded_cell_max_tile_size[2],
          exploded_cell_margin[2];

  // hack for now that lets other programs that call
  // poms_main have access to a callback in case they
  // need updates during the run
  //
  int (*global_callback)(void);

  std::vector< int32_t > m_conflict_grid;

  double m_alpha, m_beta;

  int64_t m_iter;

  std::vector< fnl_state >  noise;
  std::vector< double >     noise_min;
  std::vector< double >     noise_max;
  std::vector< double >     noise_threshold;
  std::vector< NOISE_TYPE > noise_type;

  //std::vector< int32_t >  soften_min,
  //                        soften_max;

  int64_t m_slideshow_id;

} g_ctx_t;


int poms_main(int , char **);

#endif
