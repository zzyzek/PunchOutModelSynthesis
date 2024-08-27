/*
 * To the extent possible under law, the person who associated CC0 with
 * this file has waived all copyright and related or neighboring rights
 * to this file.
 *      
 * You should have received a copy of the CC0 legalcode along with this
 * work. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>
#include <getopt.h>

#include <math.h>

#include "poms.hpp"

#include "TiledExport.hpp"

typedef struct g_ctx_type {
  double beta_0,
         beta_m,
         gamma;
  int64_t step_count;

  std::vector< POMS * > poms;

  std::vector< int64_t > start_pos;

  tiled_export_t T;

  std::string tiled_fn;
} g_ctx_t;

g_ctx_t g_ctx;

int exit_err(const char *msg=NULL) {
  if (msg) { fprintf(stderr, "%s\n", msg); }
  exit(-1);
}


int tiled_replica_snapshot(void) {
  int fd, r;
  FILE *fp;
  char fn_tmp[64];
  std::string fn;

  int64_t p, n;

  int64_t idx,
          x,y,
          dx,dy,
          tpos, cell;

  n = g_ctx.T.width * g_ctx.T.height;

  //---
  //

  // transfer poms data to Tiled structure
  //
  for (p=0; p<n; p++) { g_ctx.T.layers[0].data[p] = 0; }

  for (idx=0; idx<g_ctx.poms.size(); idx++) {
    x = g_ctx.start_pos[2*idx];
    y = g_ctx.start_pos[2*idx+1];
    for (dy=0; dy<g_ctx.poms[idx]->m_size[1]; dy++) {
      for (dx=0; dx<g_ctx.poms[idx]->m_size[0]; dx++) {
        tpos = (y+dy)*g_ctx.T.width + (x+dx);
        cell = g_ctx.poms[idx]->xyz2cell(dx,dy,0);
        g_ctx.T.layers[0].data[tpos] = g_ctx.poms[idx]->cellTile(g_ctx.poms[idx]->m_plane, cell, 0);
      }
    }

  }

  //---

  fn = g_ctx.tiled_fn;
  if (fn.size() == 0) { return -1; }

  strncpy( fn_tmp, "snapshot.json.XXXXXX", 32 );

  fd = mkstemp(fn_tmp);
  if (fd<0) { return fd; }

  fp = fdopen(fd, "w");
  if (!fp) { return -1; }

  r = exportTiledJSON( fp, g_ctx.T );
  if (r<0) { return r; }

  fclose(fp);
  rename( fn_tmp, fn.c_str() );

  return r;
}


int main(int argc, char **argv) {
  int r, err, ret;

  int y;
  std::string poms_cfg_fn;
  std::vector< POMS * > poms;

  //double beta_0, beta_m, gamma;

  double prv_e_energy,
         cur_e_energy,
         p_rel_energy,
         e_diff;

  int M=3;
  int N=M+1;

  double _S, _dist,
         S_0, S_m,
         E_dist,
         t, d,
         p;

  int64_t ref_x, ref_y;

  int ceil_root_n;

  int i, j, k,
      sx, sy;

  int64_t it, n_it, step;

  //poms_cfg_fn = "../data/pillMortal_le.json";
  poms_cfg_fn = "../data/rrti_le.json";

  poms.push_back( new POMS() );
  for (y=1; y<=M; y++) {
    poms.push_back( new POMS() );
  }

  r = poms[0]->loadJSONFile( poms_cfg_fn );
  if (r<0) {
    fprintf(stderr, "error loading file: %s\n", poms_cfg_fn.c_str());
    exit(-1);
  }

  // init tiled snapshot structure
  //

  g_ctx.tiled_fn = "replica_tiled.json";

  g_ctx.poms = poms;

  ceil_root_n = (int)(sqrt(N-1.0) + 1.0);
  ref_x = ceil_root_n / 2;
  ref_y = ceil_root_n / 2;

  ref_x *= poms[0]->m_size[0];
  ref_y *= poms[0]->m_size[1];

  g_ctx.T.tilesets.resize(1);
  g_ctx.T.layers.resize(1);
  g_ctx.T.width  = ceil_root_n * poms[0]->m_size[0];
  g_ctx.T.height = ceil_root_n * poms[0]->m_size[1];

  g_ctx.T.tileheight = 8;
  g_ctx.T.tilewidth = 8;

  //g_ctx.T.tilesets[0].tileheight = 8;
  //g_ctx.T.tilesets[0].tilewidth = 8;
  //g_ctx.T.tilesets[0].tilecount = 189;
  //g_ctx.T.tilesets[0].image = "./pillMortal_tileset.png";

  g_ctx.T.tilesets[0].tileheight = poms[0]->m_tileset_ctx.tileheight;
  g_ctx.T.tilesets[0].tilewidth = poms[0]->m_tileset_ctx.tilewidth;
  g_ctx.T.tilesets[0].tilecount = poms[0]->m_tileset_ctx.tilecount;
  g_ctx.T.tilesets[0].image = poms[0]->m_tileset_ctx.image;

  g_ctx.T.layers[0].height  = g_ctx.T.height;
  g_ctx.T.layers[0].width   = g_ctx.T.width;
  g_ctx.T.layers[0].data.resize( g_ctx.T.width * g_ctx.T.height );

  sx = 0;
  sy = 0;
  for (i=0; i<N; i++) {
    g_ctx.start_pos.push_back(sx);
    g_ctx.start_pos.push_back(sy);

    sx += poms[0]->m_size[0];
    if (sx >= g_ctx.T.layers[0].width) {
      sx = 0;
      sy += poms[0]->m_size[1];
    }
  }

  for (i=0; i<N; i++) {
    printf("start[%i]: %i %i\n", (int)i, (int)g_ctx.start_pos[2*i], (int)g_ctx.start_pos[2*i+1]);
  }

  //---

  r = poms[0]->applyConstraints();
  if (r<0) { exit_err("applyConstraints"); }

  r = poms[0]->cullSweep();
  if (r<0) { exit_err("cullSweep"); }

  r = poms[0]->savePrefatory();
  if (r<0) { exit_err("savePrefatory"); }

  //---

  r = poms[0]->applyStartConstraints();
  if (r<0) { exit_err("applyStartConstraints"); }

  r = poms[0]->cullSweep();
  if (r<0) { exit_err("cullSweep"); }

  poms[0]->m_tile_choice_policy = POMS_TILE_CHOICE_PROB;
  poms[0]->m_block_choice_policy = POMS_BLOCK_CHOICE_MAX_ENTROPY;

  //---

  g_ctx.beta_0 = 2.0/40240.0;
  g_ctx.beta_m = 2.0/102400.0;
  g_ctx.gamma = 100;

  printf("size:[%i,%i,%i], replicas:%i (+1 reference), beta_0:%f, beta_m:%f, gamma:%f\n",
      (int)poms[0]->m_size[0],
      (int)poms[0]->m_size[1],
      (int)poms[0]->m_size[2],
      (int)M,
      g_ctx.beta_0, g_ctx.beta_m, g_ctx.gamma );

  for (y=1; y<=M; y++) {
    poms[y]->loadPOMS(*poms[0]);
    poms[y]->copyState(*poms[0]);
  }

  for (y=0; y<N; y++) {
    poms[y]->entropy(&_S);
    poms[y]->dist(&_dist, *poms[y]);
    printf("y[%i]: entropy:%f, dist(0,%i):%f (size:[%i,%i,%i], block:[%i,%i,%i], soften:[%i,%i,%i]\n",
        (int)y, _S, (int)y, _dist,
        (int)poms[y]->m_size[0],
        (int)poms[y]->m_size[1],
        (int)poms[y]->m_size[2],

        (int)poms[y]->m_block_size[0],
        (int)poms[y]->m_block_size[1],
        (int)poms[y]->m_block_size[2],

        (int)poms[y]->m_soften_size[0],
        (int)poms[y]->m_soften_size[1],
        (int)poms[y]->m_soften_size[2]

        );

  }

  //----
  //----
  //----

  g_ctx.step_count=1;

  for (y=0; y<N; y++) { g_ctx.poms[y]->BMSInit(); }

  prv_e_energy = 0.0;
  S_0 = 0.0;
  S_m = 0.0;
  E_dist = 0.0;
  for (i=0; i<N; i++) {
    poms[i]->entropy(&t);
    if (i==0) { S_0 = t; }
    else { S_m += t; }

    if (i>0) {
      poms[0]->dist(&_dist, *poms[i]);
      E_dist += _dist;
    }
  }
  prv_e_energy = (g_ctx.beta_0*S_0 + g_ctx.beta_m*S_m + g_ctx.gamma*E_dist);


  n_it = 100000;
  for (it=0; it<n_it; it++) {

    // round-robin scheduling
    //
    y = (it%poms.size());

    printf("#[%5i/%5i] y:%2i, e(%f*%f + %f*%f + %f*%f=%f)\n",
        (int)it, (int)n_it, (int)y,
        g_ctx.beta_0, S_0,
        g_ctx.beta_m, S_m,
        g_ctx.gamma, E_dist,
        prv_e_energy);

    //printf("#it:%i (/%i), y:%i, step_count:%i\n",
    //    (int)it, (int)n_it, (int)y,
    //    (int)g_ctx.step_count);

    r = g_ctx.poms[y]->BMSBegin();
    if (r<0) { err=1; break; }

    for (step=0; step<g_ctx.step_count; step++) {
      r = g_ctx.poms[y]->BMSStep();

      //printf("  bmsstep: got %i, m_state:%i\n", (int)r, (int)g_ctx.poms[y]->m_state);

      if (r<=0) { break; }
    }

    r = g_ctx.poms[y]->BMSEnd();
    if (r<0) { err=2; break; }

    if ((y==0) && (r==0)) {

      //printf("solution found, breaking\n");

      err=0;
      break;
    }


    S_0 = 0.0;
    S_m = 0.0;
    E_dist = 0.0;
    for (i=0; i<N; i++) {
      poms[i]->entropy(&t);
      if (i==0) { S_0 = t; }
      else { S_m += t; }

      if (i>0) {
        poms[0]->dist(&_dist, *poms[i]);
        E_dist += _dist;
      }
    }
    cur_e_energy = (g_ctx.beta_0*S_0 + g_ctx.beta_m*S_m + g_ctx.gamma*E_dist);

    // higher energy
    //
    if (cur_e_energy > prv_e_energy) {
      e_diff = cur_e_energy - prv_e_energy;
      p_rel_energy = exp(-e_diff);
      p = drand48();
      if (p < p_rel_energy) {

        //DEBUG
        //printf("accept (ecur:%f,eprv:%f) (%f < %f)\n", cur_e_energy, prv_e_energy, p, p_rel_energy);

        prv_e_energy = cur_e_energy;
      }
      else {

        //DEBUG
        //printf("reject (ecur:%f,eprv:%f) (%f >= %f)\n", cur_e_energy, prv_e_energy, p, p_rel_energy);

        g_ctx.poms[y]->restoreGrid();
      }
    }

    // accept
    //
    else {

      //DEBUG
      //printf("accept (ecur:%f,eprv:%f)\n", cur_e_energy, prv_e_energy);

      prv_e_energy = cur_e_energy;
    }


    tiled_replica_snapshot();
  }

  printf("err!:%i\n", err);

}
