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

#include <unistd.h>
#include <termios.h>
#include <poll.h>

#include "poms.hpp"

#include "TiledExport.hpp"

#define POMS_MCMC_VERSION "0.1.0"

typedef struct g_ctx_type {
  double beta_0,
         beta_m,
         gamma;
  int64_t step_count;

  std::vector< POMS * > poms;

  std::vector< int64_t > start_pos;

  tiled_export_t T;

  std::string tiled_fn;

  int verbose;
} g_ctx_t;

g_ctx_t g_ctx;

int exit_err(const char *msg=NULL) {
  if (msg) { fprintf(stderr, "%s\n", msg); }
  exit(-1);
}

void _init_rand(POMS &poms) {
  int64_t cell,
          m_cell_count;
  int32_t tile_val,
          tile_idx,
          x,y,z,
          m_tile_count,
          m_plane;

  m_plane = poms.m_plane;
  m_cell_count = poms.m_cell_count;
  m_tile_count = poms.m_tile_count;
  for (cell=0; cell<m_cell_count; cell++) {

    tile_val = rand() % m_tile_count;

    poms.setCellSize( m_plane, cell, 1 );
    poms.setCellTile( m_plane, cell, 0, tile_val );

  }

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

int64_t _count_contradictions(POMS &poms) {
  int64_t cell,
          nei_cell,
          m_cell_count;
  int64_t n_c=0;
  int32_t boundary_tile = 0;

  int32_t idir,
          m_plane,
          m_tile_count,
          tile_val,
          nei_tile_val,
          tile_idx,
          x,y,z;

  m_plane = poms.m_plane;
  m_tile_count = poms.m_tile_count;
  m_cell_count = poms.m_cell_count;
  for (cell=0; cell<m_cell_count; cell++) {

    tile_val = poms.cellTile( m_plane, cell, 0 );

    for (idir=0; idir<6; idir++) {
      nei_cell = poms.neiCell(cell, idir);
      if (nei_cell < 0) {
        if (poms.F(tile_val, boundary_tile, idir) < poms.m_eps) { n_c++; }
        continue;
      }

      nei_tile_val = poms.cellTile( m_plane, nei_cell, 0 );
      if (poms.F(tile_val, nei_tile_val, idir) < poms.m_eps) { n_c++; }
    }

  }

  return n_c;
}

/*
int64_t _count_contradictions_a(std::vector< POMS * > &poms) {
  int64_t n_c=0;
  int ii;
  for (ii=0; ii<poms.size(); ii++) {
    n_c += _count_contradictions(poms[ii]);
  }
  return n_c;
}
*/

int64_t _dist_from_ref(std::vector< POMS * > &poms) {
  int ii;
  int32_t m_plane,
          ref_tile,
          cmp_tile;
  int64_t n_c=0, cell, m_cell_count;
  if (poms.size() <= 1) { return 0; }

  m_plane = poms[0]->m_plane;
  m_cell_count = poms[0]->m_cell_count;
  for (cell=0; cell<m_cell_count; cell++) {
    ref_tile = poms[0]->cellTile( m_plane, cell, 0 );
    for (ii=1; ii<poms.size(); ii++) {
      cmp_tile = poms[ii]->cellTile( m_plane, cell, 0 );
      if (ref_tile != cmp_tile) { n_c++; }
    }
  }

  return n_c;
}


int getch_immediate_init() {   
  int c, ret;
  static struct termios oldt, newt;
  struct pollfd fds[1];

  fds[0].fd = 0;
  fds[0].events = POLLIN;

  tcgetattr( STDIN_FILENO, &oldt);
  newt = oldt;

  newt.c_lflag &= ~(ICANON);

  tcsetattr( STDIN_FILENO, TCSANOW, &newt);

  while (1) {
    ret = poll(fds, 1, 0);
    if (ret<0) { exit(-1); }
    if (ret==0) { continue; }

    c = getchar();
    printf("!!%c!!\n", c);
  }

  /*
  while((c=getchar())!= 'e') {
    printf("?\n");
    putchar(c);                 
  }
  */

  tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
  return 0;

}

static struct option long_options[] = {

  {"size",      required_argument,  0, 's' },
  {"iter",      required_argument,  0, 'J' },

  {"config",        required_argument,  0, 'C' },
  {"tiled",         required_argument,  0, '1' },
  {"tiled-snapshot",      required_argument,  0, '8' },

  //{"viz",         required_argument,  0, '@' },
  {"seed",        required_argument,  0, 'S' },
  {"replica",     required_argument,  0, 'M' },

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
  "verbose level",
  "help (this screen)",
  "version"
};

void print_version(FILE *fp) {
  fprintf(fp, "%s\n", POMS_MCMC_VERSION);
}

void print_help(FILE *fp) {
  int lo_idx=0,
      spacing=0,
      ii;
  struct option *lo;

  fprintf(fp, "\n");
  fprintf(fp, "poms.mcmc, version: ");
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



int main(int argc, char **argv) {
  int r, err, ret;


  int _ch, ret_inp;
  static struct termios oldt, newt;
  struct pollfd fds[1];



  int y;
  std::string poms_cfg_fn;
  std::vector< POMS * > poms;

  double prv_e_energy,
         cur_e_energy,
         p_rel_energy,
         e_diff;

  int N;

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
  int64_t cur_contradict,
          prv_contradict;

  int64_t cell,
          m_cell_count;
  int32_t prv_tile_val,
          cur_tile_val,
          m_tile_count,
          m_plane;

  double _X, _prob, _d, _f,
         _beta_0_s, _beta_0_e,
         _beta_m_s, _beta_m_e,
         _gamma_s, _gamma_e;

  int64_t log_n = 100;

  int replica_count,
      replica_idx = 0;

  int64_t cur_c_energy[3],
          prv_c_energy[3];

  double f_gamma,
         f_beta_0,
         f_beta_m;

  int opt_idx, seed, ch;

  // defaults
  //
  g_ctx.verbose = 0;
  g_ctx.tiled_fn = "mcmc_tiled.json";

  replica_count = 3;
  log_n = 1000;
  n_it = 100000;

  while (ch = getopt_long(argc, argv, "hvV:s:J:C:1:8:S:M:", long_options, &opt_idx)) {
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
        //s = optarg;
        //splitStr(tok, s, sep);
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


  //poms_cfg_fn = "../data/pillMortal_le.json";
  //poms_cfg_fn = "../data/rrti_mcmc.json";
  //poms_cfg_fn = "../data/road2d_mcmc.json";

  poms.push_back( new POMS() );
  for (y=1; y<=replica_count; y++) {
    poms.push_back( new POMS() );
  }

  r = poms[0]->loadJSONFile( poms_cfg_fn );
  if (r<0) {
    fprintf(stderr, "error loading file: %s\n", poms_cfg_fn.c_str());
    exit(-1);
  }

  for (replica_idx=1; replica_idx<N; replica_idx++) {
    poms[replica_idx]->loadPOMS(*poms[0]);
  }

  // init tiled snapshot structure
  //


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

  m_plane = poms[0]->m_plane;
  m_cell_count = poms[0]->m_cell_count;
  m_tile_count = poms[0]->m_tile_count;

  f_beta_0 = (1.0 + 1.0/128.0);
  f_beta_m = (1.0 + 1.0/128.0);
  f_gamma = (1.0 + 1.0/128.0);

  g_ctx.beta_0 = 1.0/1024.0;
  g_ctx.beta_m = 1.0/1024.0;
  g_ctx.gamma = 1.0/1024.0;

  g_ctx.beta_0 = 1;

  _beta_0_s = g_ctx.beta_0;
  _beta_0_e = 10;

  _beta_m_s = g_ctx.beta_m;
  _beta_m_e = 10;

  _gamma_s = g_ctx.gamma;
  _gamma_e = 10;

  printf("size:[%i,%i,%i], replicas:%i (+1 reference), beta_0:%f, beta_m:%f, gamma:%f\n",
      (int)poms[0]->m_size[0],
      (int)poms[0]->m_size[1],
      (int)poms[0]->m_size[2],
      (int)replica_count,
      g_ctx.beta_0, g_ctx.beta_m, g_ctx.gamma );
  
  for (replica_idx=0; replica_idx<N; replica_idx++) {
    _init_rand(*poms[replica_idx]);
  }

  prv_c_energy[0] = _count_contradictions(*poms[0]);
  prv_c_energy[1] = 0;
  for (replica_idx=1; replica_idx < poms.size(); replica_idx++) {
    prv_c_energy[1] += _count_contradictions(*poms[replica_idx]);
  }
  prv_c_energy[2] = _dist_from_ref(poms);

  prv_e_energy =
    (double)( ((double)prv_c_energy[0]) * g_ctx.beta_0 ) +
    (double)( ((double)prv_c_energy[1]) * g_ctx.beta_m ) +
    (double)( ((double)prv_c_energy[2]) * g_ctx.gamma );


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


  for (it=0; it<n_it; it++) {

    replica_idx = random()%N;

    //----
    //_d = (double)it;
    //_d /= (double)n_it;

    //g_ctx.beta_0 = (1.0 - _d)*_beta_0_s + _d*_beta_0_e;
    //g_ctx.beta_m = (1.0 - _d)*_beta_m_s + _d*_beta_m_e;
    //g_ctx.gamma = (1.0 - _d)*_gamma_s + _d*_gamma_e;

    ret_inp = poll(fds, 1, 0);
    if (ret_inp<0) { exit(-1); }
    if (ret_inp>0) {
      _ch = getchar();

      switch (_ch) {
        case 'a': g_ctx.beta_0 /= f_beta_0; break;
        case 'q': g_ctx.beta_0 *= f_beta_0; break;
        case 's': g_ctx.beta_m /= f_beta_m; break;
        case 'w': g_ctx.beta_m *= f_beta_m; break;
        case 'd': g_ctx.gamma /= f_gamma; break;
        case 'e': g_ctx.gamma *= f_gamma; break;
        default:
          break;
      }

      printf(">>>>>>>>>>>>>>>>> beta_0:%f, beta_m:%f, gamma:%f\n",
          g_ctx.beta_0, g_ctx.beta_m, g_ctx.gamma);
    }
    //----

    if ((it % log_n)==0) {
      tiled_replica_snapshot();
    }


    cell = (int64_t)( ((int64_t)random()) % m_cell_count );

    prv_tile_val = poms[replica_idx]->cellTile( m_plane, cell, 0 );
    cur_tile_val = (int32_t)( ((int32_t)random()) % m_tile_count );

    poms[replica_idx]->setCellTile( m_plane, cell, 0, cur_tile_val );

    cur_c_energy[0] = _count_contradictions(*poms[0]);
    cur_c_energy[1] = 0;
    for (y=1; y< poms.size(); y++) {
      cur_c_energy[1] += _count_contradictions(*poms[y]);
    }
    cur_c_energy[2] = _dist_from_ref(poms);

    if (cur_c_energy[0] == 0) { printf("FOUND\n"); break; }

    cur_e_energy =
      (double)( ((double)cur_c_energy[0]) * g_ctx.beta_0 ) +
      (double)( ((double)cur_c_energy[1]) * g_ctx.beta_m ) +
      (double)( ((double)cur_c_energy[2]) * g_ctx.gamma );


    // accept
    //
    if (cur_e_energy <= prv_e_energy) {

      if ((it%log_n)==0) {
        printf("it[%i/%i]: y:%i cell:%i, tile(prv:%i->cur:%i) (prv:[%i,%i,%i]->cur:[%i,%i,%i]) contradict(prv:%f->cur:%f) (%f;B0:%f,Bm:%f,G:%f) (accept.0)\n",
            (int)it, (int)n_it,
            (int)replica_idx,
            (int)cell, (int)prv_tile_val, (int)cur_tile_val,
            (int)prv_c_energy[0], (int)prv_c_energy[1], (int)prv_c_energy[2],
            (int)cur_c_energy[0], (int)cur_c_energy[1], (int)cur_c_energy[2],
            prv_e_energy, cur_e_energy,
            _d, g_ctx.beta_0, g_ctx.beta_m, g_ctx.gamma);
      }


      prv_e_energy = cur_e_energy;
      prv_c_energy[0] = cur_c_energy[0];
      prv_c_energy[1] = cur_c_energy[1];
      prv_c_energy[2] = cur_c_energy[2];
      continue;
    }

    // cur_contradict > prv_contradict
    //
    //_prob = exp( g_ctx.beta_0*((double)(prv_contradict - cur_contradict)) );
    _prob = exp( prv_e_energy - cur_e_energy );
    _X = drand48();
    if (_X < _prob) {

      if ((it%log_n)==0) {
        printf("it[%i/%i]: y:%i, cell:%i, tile(prv:%i->cur:%i) (prv:[%i,%i,%i]->cur:[%i,%i,%i]) ln(E)(prv:%f->cur:%f) (%f<exp(%f)=%f) (%f;B0:%f,Bm:%f,G:%f) (accept.1)\n",
            (int)it, (int)n_it,
            (int)replica_idx,
            (int)cell, (int)prv_tile_val, (int)cur_tile_val,
            (int)prv_c_energy[0], (int)prv_c_energy[1], (int)prv_c_energy[2],
            (int)cur_c_energy[0], (int)cur_c_energy[1], (int)cur_c_energy[2],
            prv_e_energy, cur_e_energy,
            _X, prv_e_energy - cur_e_energy, _prob,
            _d, g_ctx.beta_0, g_ctx.beta_m, g_ctx.gamma);
      }


      prv_e_energy = cur_e_energy;
      prv_c_energy[0] = cur_c_energy[0];
      prv_c_energy[1] = cur_c_energy[1];
      prv_c_energy[2] = cur_c_energy[2];
      continue;
    }


    // revert
    //
    if ((it%log_n)==0) {
      printf("it[%i/%i]: y:%i, cell:%i, tile(prv:%i..cur:%i) (prv:[%i,%i,%i]..cur:[%i,%i,%i]) ln(E)(prv:%f..cur:%f) (%f>=exp(%f)=%f) (%f;B0:%f,Bm:%f,G:%f) (reject.0)\n",
          (int)it, (int)n_it,
          (int)replica_idx,
          (int)cell, (int)prv_tile_val, (int)cur_tile_val,
          (int)prv_c_energy[0], (int)prv_c_energy[1], (int)prv_c_energy[2],
          (int)cur_c_energy[0], (int)cur_c_energy[1], (int)cur_c_energy[2],
          prv_e_energy, cur_e_energy,
          _X, prv_e_energy - cur_e_energy, _prob,
          _d, g_ctx.beta_0, g_ctx.beta_m, g_ctx.gamma );
    }

    poms[replica_idx]->setCellTile( m_plane, cell, 0, prv_tile_val );

  }

}
