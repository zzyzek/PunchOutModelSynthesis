/*
 * To the extent possible under law, the person who associated CC0 with
 * this file has waived all copyright and related or neighboring rights
 * to this file.
 *      
 * You should have received a copy of the CC0 legalcode along with this
 * work. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 *
 */

// local arc consistent entropy
//

#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>
#include <getopt.h>

#include <math.h>

#include <unistd.h>
#include <termios.h>
#include <poll.h>

#include "poms.hpp"

#include "TiledExport.hpp"

#define POMS_LACE_VERSION "0.1.0"

typedef struct g_ctx_type {
  double beta_0,
         beta_m,
         gamma;
  double gamma_start_threshold;
  int64_t step_count;
  int verbose;

  std::vector< POMS * > poms;
  std::vector< int64_t > start_pos;

  std::string tiled_fn;
  tiled_export_t T;

  int32_t reference_conflict_count;

  double rand_coefficient,
         rand_exponent;


  std::vector< double > rwin;
  int rwin_start=0,
      rwin_n=0,
      rwin_size = 1000;

  double rwin_snapshot[2];
  int rwin_snapshot_counter=-1,
      rwin_snapshot_size = 2000;
  double rwin_snapshot_threshold[2] = {100.0, 10.0};

  int rwin_add(double val) {
    if (rwin.size() != rwin_size) {
      rwin.resize(rwin_size);
    }
    rwin[ (rwin_start + rwin_n) % rwin_size ] = val;
    if (rwin_n < rwin_size) {
      rwin_n++;
    }
    else {
      rwin_start++;
    }
    return 0;
  }

  int rwin_reset() {
    rwin_n = 0;
    rwin_start = 0;
    return 0;
  }

  void rwin_debug() {
    int i;
    printf("rwin[%i:%i/%i]:",
        rwin_start, rwin_n, rwin_size);
    for (i=0; i<rwin_n; i++) {
      printf(" %f", rwin[i]);
    }
    printf("\n");
  }

  double rwin_mean() {
    int i;
    double m = 0.0;
    if (rwin_n < 1) { return 0.0; }
    for (i=0; i<rwin_n; i++) {
      m += rwin[ (rwin_start+i) % rwin_n ];
    }
    return m / (double)rwin_n;
  }

  double rwin_var() {
    int i;
    double a,
           m = 0.0,
           v = 0.0;
    if (rwin_n < 1) { return 0.0; }
    for (i=0; i<rwin_n; i++) {
      a = rwin[ (rwin_start+i) % rwin_n ];
      m += a;
      v += a*a;
    }
    m /= (double)(rwin_n);
    v /= (double)rwin_n;

    return (v - (m*m));
  }

  double rwin_sigma() {
    return sqrt(rwin_var());
  }


  int rwin_tick() {
    int r = 0;
    double _m = 0.0,
           _s = 0.0;

    _m = rwin_mean();
    _s = rwin_sigma();

    rwin_snapshot_counter++;

    if (rwin_snapshot_counter == 0) {
      rwin_snapshot[0] = _m;
      rwin_snapshot[1] = _s;
      return 0;
    }


    if (rwin_snapshot_counter >= rwin_snapshot_size) {

      printf("rwin_tick fire: snapshot[%f,%f] >< (%f,%f), thres[%f,%f]\n",
          rwin_snapshot[0], rwin_snapshot[1],
          _m, _s,
          rwin_snapshot_threshold[0], rwin_snapshot_threshold[1]);

      if ( ((rwin_snapshot[0] - _m) < rwin_snapshot_threshold[0]) &&
           (fabs(rwin_snapshot[1] - _s) < rwin_snapshot_threshold[1]) ) {
        r = 1;
      }
      rwin_snapshot[0] = _m;
      rwin_snapshot[1] = _s;
      rwin_snapshot_counter = 0;

    }

    return r;
  }



} g_ctx_t;

g_ctx_t g_ctx;


//----
//----
//----


typedef struct transfer_block_info_type {
  int32_t min_block[3],
          max_block[3],
          soften_edge[3];

  int32_t soften_block[3][2],
          copy_block[3][2];
  double  alpha;
} transfer_block_info_t;

//----
//----
//----


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

  int32_t ref_n, ref_tile,
          cpy_n, cpy_tile;

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
        g_ctx.T.layers[1].data[tpos] = g_ctx.poms[idx]->cellSize(g_ctx.poms[idx]->m_plane, cell);

        g_ctx.T.layers[2].data[tpos] = 0;

        // reference, skip
        //
        if ((x==0) && (y==0)) {
          g_ctx.T.layers[2].data[tpos] = 1;
          continue;
        }

        ref_n = g_ctx.poms[ 0 ]->cellSize( g_ctx.poms[ 0 ]->m_plane, cell );
        cpy_n = g_ctx.poms[idx]->cellSize( g_ctx.poms[idx]->m_plane, cell );

        if ((ref_n != 1) ||
            (cpy_n != ref_n)) { continue; }

        ref_tile = g_ctx.poms[ 0 ]->cellTile( g_ctx.poms[ 0 ]->m_plane, cell, 0 );
        cpy_tile = g_ctx.poms[idx]->cellTile( g_ctx.poms[idx]->m_plane, cell, 0 );

        if (ref_tile != cpy_tile) { continue; }

        g_ctx.T.layers[2].data[tpos] = 1;


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

//----
//----
//----

static struct option long_options[] = {

  {"size",      required_argument,  0, 's' },
  {"iter",      required_argument,  0, 'J' },

  {"config",        required_argument,  0, 'C' },
  {"tiled",         required_argument,  0, '1' },
  {"tiled-snapshot",      required_argument,  0, '8' },

  //{"viz",         required_argument,  0, '@' },
  {"seed",        required_argument,  0, 'S' },
  {"replica",     required_argument,  0, 'M' },
  {"snapshot-freq",     required_argument,  0, '/' },

  {"start-threshold",     required_argument,  0, 't' },
  {"rand-w",     required_argument,  0, 'w' },
  {"rand-E",     required_argument,  0, 'E' },

  {"verbose",   required_argument,  0, 'V' },
  {"help",      no_argument,        0, 'h' },
  {"version",   no_argument,        0, 'v' },
  {0,           0,                  0,  0  },
};


static char long_options_descr[][128] = {
  "size of map (override default from poms JSON file)",
  "# iterations",
  "POMS file",
  "output Tiled JSON file",
  "Tiled snapshot file",
  "random seed",
  "replica count",
  "snapshot frequency",

  "start threshold",
  "random choice coefficient",
  "random choice exponent",

  "verbose level",
  "help (this screen)",
  "version"
};

void print_version(FILE *fp) {
  fprintf(fp, "%s\n", POMS_LACE_VERSION);
}

void print_help(FILE *fp) {
  int lo_idx=0,
      spacing=0,
      ii;
  struct option *lo;

  fprintf(fp, "\n");
  fprintf(fp, "poms.lace, version: ");
  print_version(fp);
  fprintf(fp, "\n");

  for (lo_idx=0; long_options[lo_idx].name; lo_idx++) {

    fprintf(fp, "  -%c,--%s ",
        long_options[lo_idx].val,
        long_options[lo_idx].name);

    spacing = 12 - strnlen(long_options[lo_idx].name, 127);
    for (ii=0; ii<spacing; ii++) { fprintf(fp, " "); }

    fprintf(fp, "%s\n", long_options_descr[lo_idx]);
  }

  fprintf(fp, "\n");
}



//----
//----
//----

int64_t resolvedCount( POMS &poms ) {
  int64_t cell,
          count=0;
  for (cell=0; cell<poms.m_cell_count; cell++) {
    if (poms.cellSize( poms.m_plane, cell )==1) { count++; }
  }
  return count;
}

double avg_energy(void) {
  int ii;
  double d;
  int64_t resolved_count=0,
          tot_resolved=0,
          energy_i64 = 0,
          m_cell_count;
  m_cell_count = g_ctx.poms[0]->m_cell_count;
  for (ii=0; ii<g_ctx.poms.size(); ii++) {
    resolved_count = resolvedCount( *(g_ctx.poms[ii]) );
    tot_resolved += resolved_count;

    energy_i64 = (m_cell_count - resolved_count);
  }
  d = (double)energy_i64 / (double)(g_ctx.poms.size());
  return d;
}

void log_line(int iter, int step, int n_iter) {
  int ii;
  int64_t cell, resolved_count, m_cell_count;
  int64_t tot_resolved=0;
  std::vector< double > dist;
  double d;

  m_cell_count = g_ctx.poms[0]->m_cell_count;

  d = (double)iter / (double)n_iter;

  printf("## it:%i/%i{%f},",
      (int)iter, (int)n_iter, d);

  printf(" E[");
  for (ii=0; ii<g_ctx.poms.size(); ii++) {
    resolved_count=0;
    resolved_count = resolvedCount( *(g_ctx.poms[ii]) );

    if (ii>0){ printf(","); }
    printf("%i", (int)(m_cell_count-resolved_count));

    tot_resolved += resolved_count;
  }
  d = (double)tot_resolved / (double)(g_ctx.poms.size()*m_cell_count);
  d = 1.0 - d;


  printf("]/%i{avg:%f} (win{%f,%f}/%i)",
      (int)m_cell_count,
      d,
      g_ctx.rwin_mean(),
      g_ctx.rwin_sigma(),
      g_ctx.rwin_n);

  printf(" dist[0");
  for (ii=1; ii<g_ctx.poms.size(); ii++) {
    //g_ctx.poms[0]->distJaccard( &d, *(g_ctx.poms[ii]) );
    g_ctx.poms[0]->distCosine( &d, *(g_ctx.poms[ii]) );
    printf(",%f", 1.0-d);
  }
  printf("]");
  printf(" {gamma:%f}", g_ctx.gamma);
  printf("\n");

  fflush(stdout);
}

//---
//---
//---

int splitStr( std::vector< std::string > &tok, std::string s, int sep ) {
  int i;
  std::string cur;

  tok.clear();

  for (i=0; i<s.size(); i++) {
    if (s[i]==sep) {
      tok.push_back(cur);
      cur.clear();
      continue;
    }
    cur += s[i];
  }

  tok.push_back(cur);
  return 0;
}


//---
//---
//---


static int32_t _clamp32(int32_t x, int32_t m, int32_t M) {
  if (x<m) { x = m; }
  if (x>M) { x = M; }
  return x;
}

int frustratedCOM( int32_t com[3], POMS &poms ) {
  int64_t cell,
          count=0;
  int32_t x,y,z,
          xyz[3];
  int32_t s;

  double _dcom[3];
  double R = 0.0;

  _dcom[0] = 0.0;
  _dcom[1] = 0.0;
  _dcom[2] = 0.0;

  com[0] = 0;
  com[1] = 0;
  com[2] = 0;

  for (cell=0; cell<poms.m_cell_count; cell++) {
    if (poms.cellSize( poms.m_plane, cell ) <= 1) { continue; }

    poms.cell2vec(xyz, cell);

    s = poms.cellSize( poms.m_plane, cell );

    _dcom[0] += ((double)xyz[0])*((double)s);
    _dcom[1] += ((double)xyz[1])*((double)s);
    _dcom[2] += ((double)xyz[2])*((double)s);

    com[0] += xyz[0]*s;
    com[1] += xyz[1]*s;
    com[2] += xyz[2]*s;

    count+=s;

    R += (double)s;
  }

  if (count==0) { return -1; }
  com[0] /= count;
  com[1] /= count;
  com[2] /= count;

  com[0] = (int32_t)(_dcom[0] / R);
  com[1] = (int32_t)(_dcom[1] / R);
  com[2] = (int32_t)(_dcom[2] / R);

  return 0;
}

// hacky, experimenting with an idea
//
int gash( POMS &poms, int32_t p[3], int32_t soften_size[3] ) {

  int32_t s2[3];
  int32_t x,y,z=0,
          tx,ty,tz=0,
          dx,dy,dz;

  int32_t wblock[3][2];

  int64_t cell;
  int32_t tile_n, tile_val, tile_idx;

  int32_t cur_p[3];
  int32_t xyz;

  s2[0] = soften_size[0]/2;
  s2[1] = soften_size[0]/2;
  s2[2] = soften_size[0]/2;

  wblock[2][0] = 0;
  wblock[2][1] = 1;

  wblock[0][0] = p[0];
  wblock[0][1] = p[0];

  wblock[1][0] = p[1];
  wblock[1][1] = p[1];

  cur_p[0] = p[0];
  cur_p[1] = p[1];
  cur_p[2] = p[2];

  while ((cur_p[0] < poms.m_size[0]) &&
         (cur_p[0] >= 0) &&
         (cur_p[1] < poms.m_size[1]) &&
         (cur_p[1] >= 0)) {
    for (dx=0; dx<soften_size[0]; dx++) {
      tx = cur_p[0] - s2[0] + dx;
      ty = cur_p[1];

      if ((tx < 0) || (ty < 0)) { continue; }
      if ((tx >= poms.m_size[0]) || (ty >= poms.m_size[1])) { continue; }

      if (tx < wblock[0][0]) { wblock[0][0] = tx; }
      if (tx > wblock[0][1]) { wblock[0][1] = tx; }

      if (ty < wblock[1][0]) { wblock[1][0] = ty; }
      if (ty > wblock[1][1]) { wblock[1][1] = ty; }

      cell = poms.xyz2cell(tx,ty,tz);

      tile_n = poms.cellBufSize( &(poms.m_prefatory_size[0]), cell );
      poms.setCellSize( poms.m_plane, cell, tile_n );

      for (tile_idx=0; tile_idx<tile_n; tile_idx++) {
        tile_val = poms.cellBufTile( &(poms.m_prefatory[0]), cell, tile_idx );
        poms.setCellTile( poms.m_plane, cell, tile_idx, tile_val );
      }

    }
    cur_p[0]++;
    cur_p[1]--;
  }

  cur_p[0] = p[0];
  cur_p[1] = p[1];
  cur_p[2] = p[2];

  while ((cur_p[0] < poms.m_size[0]) &&
         (cur_p[0] >= 0) &&
         (cur_p[1] < poms.m_size[1]) &&
         (cur_p[1] >= 0)) {
    for (dx=0; dx<soften_size[0]; dx++) {
      tx = cur_p[0] - s2[0] + dx;
      ty = cur_p[1];

      if ((tx < 0) || (ty < 0)) { continue; }
      if ((tx >= poms.m_size[0]) || (ty >= poms.m_size[1])) { continue; }

      if (tx < wblock[0][0]) { wblock[0][0] = tx; }
      if (tx > wblock[0][1]) { wblock[0][1] = tx; }

      if (ty < wblock[1][0]) { wblock[1][0] = ty; }
      if (ty > wblock[1][1]) { wblock[1][1] = ty; }

      cell = poms.xyz2cell(tx,ty,tz);

      tile_n = poms.cellBufSize( &(poms.m_prefatory_size[0]), cell );
      poms.setCellSize( poms.m_plane, cell, tile_n );

      for (tile_idx=0; tile_idx<tile_n; tile_idx++) {
        tile_val = poms.cellBufTile( &(poms.m_prefatory[0]), cell, tile_idx );
        poms.setCellTile( poms.m_plane, cell, tile_idx, tile_val );
      }

    }
    cur_p[0]--;
    cur_p[1]++;
  }

  wblock[0][0] = _clamp32(wblock[0][0]-1, 0, poms.m_size[0]);
  wblock[0][1] = _clamp32(wblock[0][1]+1, 0, poms.m_size[0]);

  wblock[1][0] = _clamp32(wblock[1][0]-1, 0, poms.m_size[1]);
  wblock[1][1] = _clamp32(wblock[1][1]+1, 0, poms.m_size[1]);

  wblock[2][0] = _clamp32(wblock[2][0]-1, 0, poms.m_size[2]);
  wblock[2][1] = _clamp32(wblock[2][1]+1, 0, poms.m_size[2]);

  //debug
  for (xyz=0; xyz<3; xyz++) {
    wblock[xyz][0] = 0;
    wblock[xyz][1] = poms.m_size[xyz];
  }

  printf("gash: ac4initblock [%i:%i,%i:%i,%i:%i]\n",
      (int)wblock[0][0], (int)wblock[0][1],
      (int)wblock[1][0], (int)wblock[1][1],
      (int)wblock[2][0], (int)wblock[2][1]);

  return poms.AC4InitBlock( wblock );
}

int main(int argc, char **argv) {
  int r, err, ret;

  int _ch, ret_inp;
  static struct termios oldt, newt;
  struct pollfd fds[1];

  int opt_idx, seed, ch;


  std::string poms_cfg_fn;
  //std::vector< POMS * > poms;

  //double beta_0, beta_m, gamma;

  double prv_e_energy,
         cur_e_energy,
         p_rel_energy,
         e_diff;

  int y;
  int replica_count=0, N=1,
      replica_idx;

  double _S, _dist,
         S_0, S_m,
         E_dist,
         t, d,
         d_v[3],
         p;

  int64_t ref_x, ref_y;

  int ceil_root_n;

  int i, j, k,
      sx, sy;

  int64_t it, n_it, step;

  int log_n=50, log_counter=0;

  int32_t b_s[3][2], b_b[3][2];

  int32_t transfer_block_range[3][2],
          transfer_block_size[3][2];


  int32_t _gash_size[3];

  double f_gamma = (0.0 + 1.0/128.0);

  std::vector< std::string > tok;
  std::string s;
  char sep = ',';
  int32_t _opt_size[3];

  int32_t com[3],
          com_dxyz[3];

  _opt_size[0] = -1;
  _opt_size[1] = -1;
  _opt_size[2] = -1;

  n_it = 100000;

  transfer_block_info_t transfer_block_info;

  transfer_block_info.min_block[0] = 8;
  transfer_block_info.min_block[1] = 8;
  transfer_block_info.min_block[2] = 1;

  transfer_block_info.max_block[0] = 32;
  transfer_block_info.max_block[1] = 32;
  transfer_block_info.max_block[2] = 1;

  //experiment
  transfer_block_info.max_block[0] = 64;
  transfer_block_info.max_block[1] = 64;
  transfer_block_info.max_block[2] = 1;

  // experimenting... getting a lot of failures
  //
  transfer_block_info.soften_edge[0] = 8;
  transfer_block_info.soften_edge[1] = 8;
  transfer_block_info.soften_edge[2] = 1;

  transfer_block_info.soften_edge[0] = 12;
  transfer_block_info.soften_edge[1] = 12;
  transfer_block_info.soften_edge[2] = 1;

  transfer_block_info.alpha = 0.0;

  _gash_size[0] = 32;
  _gash_size[1] = 32;
  _gash_size[2] = 1;

  com_dxyz[0] = 0;
  com_dxyz[1] = 0;
  com_dxyz[2] = 0;

  //---

  g_ctx.reference_conflict_count=0;
  g_ctx.tiled_fn = "lace_tiled.json";
  g_ctx.gamma = 1.0/1024.0;

  g_ctx.gamma_start_threshold = 0.65;
  g_ctx.rand_coefficient = 0.0;
  g_ctx.rand_exponent = -2.0;

  //---

  while (ch = getopt_long(argc, argv, "hvV:s:J:C:1:8:S:M:/:w:E:t:", long_options, &opt_idx)) {
    if (ch<0) { break; }

    switch (ch) {
      case 'h':
        print_help(stdout);
        exit(0);
        break;
      case 'v':
        print_version(stdout);
        exit(0);
        break;
      case 'V':
        g_ctx.verbose = atoi(optarg);
        break;

      case '/':
        log_n = atoi(optarg);
        if (log_n < 2) { log_n=1; }
        break;
      case 'w':
        g_ctx.rand_coefficient = atof(optarg);
        break;
      case 'E':
        g_ctx.rand_exponent = atof(optarg);
        break;

      case 't':
        g_ctx.gamma_start_threshold = atof(optarg);
        break;

      case 'M':
        replica_count = atoi(optarg);
        break;
      case 'S':
        seed = atoi(optarg);
        break;
      case 'C':
        poms_cfg_fn = optarg;
        break;
      case 'J':
        n_it = atoi(optarg);
        break;
      case 's':
        s = optarg;
        splitStr(tok, s, sep);
        _opt_size[0] = ((tok.size() > 0) ? atoi(tok[0].c_str()) : 1);
        _opt_size[1] = ((tok.size() > 1) ? atoi(tok[1].c_str()) : _opt_size[0]);
        _opt_size[2] = ((tok.size() > 2) ? atoi(tok[2].c_str()) : _opt_size[1]);
        break;

      case '1':
        break;
      case '8':
        g_ctx.tiled_fn = optarg;
        break;

      default:
        fprintf(stderr, "invalid option (%c)", (char)ch);
        print_help(stderr);
        exit(-1);
        break;
    }
  }

  N = replica_count+1;

  if (poms_cfg_fn.size() == 0) {
    fprintf(stderr, "provide input POMS ocnfig file\n");
    print_help(stderr);
    exit(-1);
  }

  if (N<1) {
    fprintf(stderr, "must have at least reference POMS\n");
    print_help(stderr);
    exit(-1);
  }

  //---

  g_ctx.poms.push_back( new POMS() );
  g_ctx.poms[0]->m_entropy_rand_coefficient = g_ctx.rand_coefficient;
  g_ctx.poms[0]->m_entropy_rand_exponent    = g_ctx.rand_exponent;
  g_ctx.poms[0]->rnd_seed(seed);
  for (y=1; y<N; y++) {
    g_ctx.poms.push_back( new POMS() );
    g_ctx.poms[y]->m_entropy_rand_coefficient = g_ctx.rand_coefficient;
    g_ctx.poms[y]->m_entropy_rand_exponent    = g_ctx.rand_exponent;
    g_ctx.poms[y]->rnd_seed(seed+y);
  }

  if ((_opt_size[0] > 0) &&
      (_opt_size[1] > 0) &&
      (_opt_size[2] > 0)) {
    g_ctx.poms[0]->m_size[0] = _opt_size[0];
    g_ctx.poms[0]->m_size[1] = _opt_size[1];
    g_ctx.poms[0]->m_size[2] = _opt_size[2];
  }

  g_ctx.poms[0]->m_retry_max = 32;

  r = g_ctx.poms[0]->loadJSONFile( poms_cfg_fn );
  if (r<0) {
    fprintf(stderr, "error loading file: %s\n", poms_cfg_fn.c_str());
    exit(-1);
  }

  // to make a larger Tiled map that encompases all
  // the replicas (for visualization)
  //
  ceil_root_n = (int)(sqrt(N-1.0) + 1.0);
  ref_x = ceil_root_n / 2;
  ref_y = ceil_root_n / 2;

  ref_x *= g_ctx.poms[0]->m_size[0];
  ref_y *= g_ctx.poms[0]->m_size[1];

  g_ctx.T.tilesets.resize(1);
  g_ctx.T.layers.resize(3);
  g_ctx.T.width  = ceil_root_n * g_ctx.poms[0]->m_size[0];
  g_ctx.T.height = ceil_root_n * g_ctx.poms[0]->m_size[1];

  g_ctx.T.tileheight  = g_ctx.poms[0]->m_tileset_ctx.tileheight;
  g_ctx.T.tilewidth   = g_ctx.poms[0]->m_tileset_ctx.tilewidth;

  g_ctx.T.tilesets[0].tileheight  = g_ctx.poms[0]->m_tileset_ctx.tileheight;
  g_ctx.T.tilesets[0].tilewidth   = g_ctx.poms[0]->m_tileset_ctx.tilewidth;
  g_ctx.T.tilesets[0].tilecount   = g_ctx.poms[0]->m_tileset_ctx.tilecount;
  g_ctx.T.tilesets[0].image       = g_ctx.poms[0]->m_tileset_ctx.image;

  g_ctx.T.layers[0].height  = g_ctx.T.height;
  g_ctx.T.layers[0].width   = g_ctx.T.width;
  g_ctx.T.layers[0].data.resize( g_ctx.T.width * g_ctx.T.height );

  g_ctx.T.layers[1].height  = g_ctx.T.height;
  g_ctx.T.layers[1].width   = g_ctx.T.width;
  g_ctx.T.layers[1].data.resize( g_ctx.T.width * g_ctx.T.height );
  g_ctx.T.layers[1].name = "cellSize";

  g_ctx.T.layers[2].height  = g_ctx.T.height;
  g_ctx.T.layers[2].width   = g_ctx.T.width;
  g_ctx.T.layers[2].data.resize( g_ctx.T.width * g_ctx.T.height );
  g_ctx.T.layers[2].name = "replica";

  sx = 0;
  sy = 0;
  for (i=0; i<N; i++) {
    g_ctx.start_pos.push_back(sx);
    g_ctx.start_pos.push_back(sy);

    sx += g_ctx.poms[0]->m_size[0];
    if (sx >= g_ctx.T.layers[0].width) {
      sx = 0;
      sy += g_ctx.poms[0]->m_size[1];
    }
  }

  //---

  r = g_ctx.poms[0]->applyConstraints();
  if (r<0) { exit_err("applyConstraints"); }

  r = g_ctx.poms[0]->cullSweep();
  if (r<0) { exit_err("cullSweep"); }

  r = g_ctx.poms[0]->savePrefatory();
  if (r<0) { exit_err("savePrefatory"); }

  //---

  r = g_ctx.poms[0]->applyStartConstraints();
  if (r<0) { exit_err("applyStartConstraints"); }

  r = g_ctx.poms[0]->cullSweep();
  if (r<0) { exit_err("cullSweep"); }

  g_ctx.poms[0]->m_tile_choice_policy = POMS_TILE_CHOICE_PROB;
  g_ctx.poms[0]->m_block_choice_policy = POMS_BLOCK_CHOICE_MIN_ENTROPY;
  g_ctx.poms[0]->m_block_choice_policy = POMS_BLOCK_CHOICE_WAVEFRONT;

  //---

  for (y=1; y<=replica_count; y++) {
    g_ctx.poms[y]->loadPOMS(*(g_ctx.poms[0]));
    g_ctx.poms[y]->copyState(*(g_ctx.poms[0]));
  }

  for (y=0; y<N; y++) {
    g_ctx.poms[y]->entropy(&_S);
    g_ctx.poms[y]->dist(&_dist, *(g_ctx.poms[y]));
    printf("y[%i]: entropy:%f, dist(0,%i):%f (size:[%i,%i,%i], block:[%i,%i,%i], soften:[%i,%i,%i], retry:%i) (threshold:%f,coef:%f,exponent:%f)\n",
        (int)y, _S, (int)y, _dist,
        (int)g_ctx.poms[y]->m_size[0],
        (int)g_ctx.poms[y]->m_size[1],
        (int)g_ctx.poms[y]->m_size[2],

        (int)g_ctx.poms[y]->m_block_size[0],
        (int)g_ctx.poms[y]->m_block_size[1],
        (int)g_ctx.poms[y]->m_block_size[2],

        (int)g_ctx.poms[y]->m_soften_size[0],
        (int)g_ctx.poms[y]->m_soften_size[1],
        (int)g_ctx.poms[y]->m_soften_size[2],

        (int)g_ctx.poms[y]->m_retry_max,

        g_ctx.gamma_start_threshold,
        g_ctx.poms[y]->m_entropy_rand_coefficient,
        g_ctx.poms[y]->m_entropy_rand_exponent

        );

  }

  //----
  //----
  //----


  //-----
  // init rt stdin
  //-----

  fds[0].fd = 0;
  fds[0].events = POLLIN;
  tcgetattr( STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON);
  tcsetattr( STDIN_FILENO, TCSANOW, &newt);

  //-----
  // init rt stdin
  //-----


  //----
  //----
  //----

  g_ctx.step_count=10;

  for (y=0; y<N; y++) { g_ctx.poms[y]->BMSInit(); }

  g_ctx.poms[0]->m_verbose = g_ctx.verbose;

  for (it=0; it<n_it; it++) {

    g_ctx.rwin_add( avg_energy() );
    if (g_ctx.rwin_tick()) {
      printf("BANG!\n");
    }

    //------------
    // INPUT
    //------------
    ret_inp = poll(fds, 1, 0);
    if (ret_inp<0) { exit(-1); }
    if (ret_inp>0) {
      _ch = getchar();

      switch (_ch) {
        /*
        case 'q': g_ctx.gamma *= (1.0+f_gamma); break;
        case 'a': g_ctx.gamma /= (1.0+f_gamma); break;

        case 'w': g_ctx.gamma *= (1.0+2.0*f_gamma); break;
        case 's': g_ctx.gamma /= (1.0+2.0*f_gamma); break;

        case 'e': g_ctx.gamma *= (1.0+4.0*f_gamma); break;
        case 'd': g_ctx.gamma /= (1.0+4.0*f_gamma); break;

        case 'r': g_ctx.gamma *= (1.0+8.0*f_gamma); break;
        case 'f': g_ctx.gamma /= (1.0+8.0*f_gamma); break;
        */

        case 'p':
          for (y=0; y<g_ctx.poms.size(); y++) {
            if (g_ctx.poms[y]->m_block_choice_policy == POMS_BLOCK_CHOICE_MIN_ENTROPY) {
              if (y==0) { printf("POMS_BLOCK_CHOICE_WAVEFRONT"); }
              g_ctx.poms[y]->m_block_choice_policy = POMS_BLOCK_CHOICE_WAVEFRONT;
            }
            else {
              if (y==0) { printf("POMS_BLOCK_CHOICE_MIN_ENTROPY"); }
              g_ctx.poms[y]->m_block_choice_policy = POMS_BLOCK_CHOICE_MIN_ENTROPY;
            }
          }
          break;

        case 'w': com_dxyz[1]--; break;
        case 's': com_dxyz[1]++; break;
        case 'a': com_dxyz[1]--; break;
        case 'd': com_dxyz[1]++; break;


        case 'g':


          frustratedCOM( com, *(g_ctx.poms[0]) );

          com[0] = _clamp32( com[0] + com_dxyz[0], 0, g_ctx.poms[0]->m_size[0] );
          com[1] = _clamp32( com[1] + com_dxyz[1], 0, g_ctx.poms[0]->m_size[1] );


          r = gash( *(g_ctx.poms[0]), com, _gash_size );
          printf("GASH: com[%i,%i,%i] (dxyz[%i,%i,%i]) (got %i)\n",
              com[0], com[1], com[2],
              com_dxyz[0], com_dxyz[1], com_dxyz[2],
              r);
          break;

        default:
          break;
      }

      printf(">>> dxyz[%i,%i,%i]\n",
        com_dxyz[0], com_dxyz[1], com_dxyz[2]);
    }
    //------------
    // INPUT
    //------------

    int64_t tot_resolved = 0;
    double _g=0.0;
    for (y=0; y<g_ctx.poms.size(); y++) {
      tot_resolved += resolvedCount( *(g_ctx.poms[y]) );
    }
    tot_resolved /= g_ctx.poms.size();
    g_ctx.gamma = ((double)tot_resolved) / ((double)(g_ctx.poms[0]->m_cell_count));

    if (g_ctx.gamma < g_ctx.gamma_start_threshold) {
      g_ctx.gamma = 0.0;
    }
    else {
      _g = (g_ctx.gamma - g_ctx.gamma_start_threshold) / (1.0 - g_ctx.gamma_start_threshold);
      g_ctx.gamma = _g*_g;
    }

    // round-robin scheduling
    //
    replica_idx = (it%g_ctx.poms.size());

    // ======================
    // transfer block experimenting
    //
    p = drand48();
    t = ((double)it / (double)n_it);

    if ((replica_idx>0) &&
        (p < g_ctx.gamma)) {

      int32_t _b[3], _B[3], _max, _min, _xyz;

      r = g_ctx.poms[0]->distCosine( &_dist, *(g_ctx.poms[replica_idx]) );
      _dist = 1.0 - _dist;
      if (r<0) {
        printf("error in cosine distance? %i %i err:%i\n", 0, (int)replica_idx, (int)r);
        continue;
      }

      transfer_block_info.alpha = ((double)it / (double)n_it);

      //experiment
      transfer_block_info.alpha = g_ctx.gamma;

      for (_xyz=0; _xyz<3; _xyz++) {
        t = transfer_block_info.alpha;
        d_v[_xyz] = t*((double)transfer_block_info.max_block[_xyz] - (double)transfer_block_info.min_block[_xyz]);
        d_v[_xyz] += (double)transfer_block_info.min_block[_xyz];
        d_v[_xyz] += 0.5;
        _b[_xyz] = (int32_t)d_v[_xyz];
        _B[_xyz] = _b[_xyz] + 2*transfer_block_info.soften_edge[_xyz];
        if (_b[_xyz] < 1) { _b[_xyz] = 1; }
        if (_b[_xyz] > g_ctx.poms[0]->m_size[_xyz]) { _b[_xyz] = g_ctx.poms[0]->m_size[_xyz]; }
        if (_B[_xyz] < 1) { _B[_xyz] = 1; }
        if (_B[_xyz] > g_ctx.poms[0]->m_size[_xyz]) { _B[_xyz] = g_ctx.poms[0]->m_size[_xyz]; }

        // choose random position of inner copy block to be within grid
        // boundaries, having soften edge potentially spill over
        //
        p = drand48();
        t = (double)(g_ctx.poms[0]->m_size[_xyz] - _b[_xyz]) * p;

        transfer_block_info.copy_block[_xyz][0] = (int32_t)(t);
        transfer_block_info.copy_block[_xyz][1] = transfer_block_info.copy_block[_xyz][0] + _b[_xyz];

        transfer_block_info.soften_block[_xyz][0] = transfer_block_info.copy_block[_xyz][0] - transfer_block_info.soften_edge[_xyz];
        transfer_block_info.soften_block[_xyz][1] = transfer_block_info.copy_block[_xyz][1] + transfer_block_info.soften_edge[_xyz];

        _min = 0;
        _max = g_ctx.poms[0]->m_size[_xyz];


        if (transfer_block_info.soften_block[_xyz][0] <  _min) { transfer_block_info.soften_block[_xyz][0] = _min; }
        if (transfer_block_info.soften_block[_xyz][0] >= _max) { transfer_block_info.soften_block[_xyz][0] = _max-1; }
        if (transfer_block_info.soften_block[_xyz][1] <= _min) { transfer_block_info.soften_block[_xyz][1] = _min+1; }
        if (transfer_block_info.soften_block[_xyz][1] >  _max) { transfer_block_info.soften_block[_xyz][1] = _max; }

        if (transfer_block_info.copy_block[_xyz][0] <  _min) { transfer_block_info.copy_block[_xyz][0] = _min; }
        if (transfer_block_info.copy_block[_xyz][0] >= _max) { transfer_block_info.copy_block[_xyz][0] = _max-1; }
        if (transfer_block_info.copy_block[_xyz][1] <= _min) { transfer_block_info.copy_block[_xyz][1] = _min+1; }
        if (transfer_block_info.copy_block[_xyz][1] >  _max) { transfer_block_info.copy_block[_xyz][1] = _max; }
      }

      printf("  ## dist:%f, choosing transfer block B[%i:%i,%i:%i,%i:%i] b(%i:%i,%i:%i,%i:%i)\n",
          _dist,
          (int)transfer_block_info.soften_block[0][0], (int)transfer_block_info.soften_block[0][1],
          (int)transfer_block_info.soften_block[1][0], (int)transfer_block_info.soften_block[1][1],
          (int)transfer_block_info.soften_block[2][0], (int)transfer_block_info.soften_block[2][1],

          (int)transfer_block_info.copy_block[0][0], (int)transfer_block_info.copy_block[0][1],
          (int)transfer_block_info.copy_block[1][0], (int)transfer_block_info.copy_block[1][1],
          (int)transfer_block_info.copy_block[2][0], (int)transfer_block_info.copy_block[2][1] );


      p = drand48();
      if (p<0.5) {

        printf("  transplant from %i -> 0....\n", (int)replica_idx);

        g_ctx.poms[0]->saveGrid();

        r =
          g_ctx.poms[0]->transplantBlock( 
            transfer_block_info.soften_block,
            transfer_block_info.copy_block,
            *(g_ctx.poms[replica_idx]) );

        if (r < 0) {
          printf("  transfer block FAILED, restoring grid\n");
          g_ctx.poms[0]->restoreGrid();
          printf("    --> %i\n", g_ctx.poms[0]->sanityArcConsistency());
        }
        else {

          //r = g_ctx.poms[0]->distJaccard( &_dist, *(g_ctx.poms[replica_idx]) );
          r = g_ctx.poms[0]->distCosine( &_dist, *(g_ctx.poms[replica_idx]) );
          _dist = 1.0 - _dist;
          printf("  new dist:%f (%i)\n", _dist, r);

        }


      }
      else {

        printf("  transplant from 0 -> %i ....\n", (int)replica_idx);

        g_ctx.poms[replica_idx]->saveGrid();

        r =
          g_ctx.poms[replica_idx]->transplantBlock( 
            transfer_block_info.soften_block,
            transfer_block_info.copy_block,
            *(g_ctx.poms[0]) );

        if (r < 0) {
          printf("  transfer block FAILED, restoring grid\n");
          g_ctx.poms[replica_idx]->restoreGrid();
          printf("    --> %i\n", g_ctx.poms[replica_idx]->sanityArcConsistency());
        }
        else {

          //r = g_ctx.poms[0]->distJaccard( &_dist, *(g_ctx.poms[replica_idx]) );
          r = g_ctx.poms[0]->distCosine( &_dist, *(g_ctx.poms[replica_idx]) );
          _dist = 1.0 - _dist;
          printf("  new dist:%f (%i)\n", _dist, r);

        }

      }




    }
    //
    // ======================

    //printf("### it:%i, replica_idx:%i\n", (int)it, (int)replica_idx);

    r = g_ctx.poms[replica_idx]->BMSBegin();
    if (r<0) { err=1; break; }

    for (step=0; step<g_ctx.step_count; step++) {
      r = g_ctx.poms[replica_idx]->BMSStep();
      if (r<=0) { break; }

      if ((log_counter%log_n)==0) {

        log_line(it, step, n_it);

        tiled_replica_snapshot();
      }
      log_counter++;

    }

    if ((replica_idx==0) &&
        (g_ctx.poms[replica_idx]->m_state == POMS_STATE_CONFLICT)) {
      g_ctx.reference_conflict_count++;
    }
    else {
      g_ctx.reference_conflict_count=0;
    }

    r = g_ctx.poms[replica_idx]->BMSEnd();
    if (r<0) { err=2; break; }

    if ((replica_idx==0) && (r==0)) {
      err=0;
      break;
    }

    if ((log_counter%log_n)==0) {

      log_line(it, -1, n_it);
 
      tiled_replica_snapshot();
    }
    log_counter++;

    //printf("waiting for input:");
    //fgetc(stdin);

    err = 1;
  }

  if (err!=0) {
    printf("answer not found? err:%i\n", (int)err);
    exit(-1);
  }

  tiled_replica_snapshot();
  printf("found!:%i\n", err);
}
