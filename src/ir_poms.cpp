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

#define POMS_IR_VERSION "0.2.0"


int exit_err(const char *msg=NULL) {
  if (msg) { fprintf(stderr, "%s\n", msg); }
  exit(-1);
}

//----
//----
//----

static struct option long_options[] = {

  {"size",      required_argument,  0, 's' },
  {"point",     required_argument,  0, 'P' },

  {"config",    required_argument,  0, 'C' },
  {"print-opt", required_argument,  0, 'o' },
  {"freq-opt",  required_argument,  0, 'F' },
  //{"tiled",         required_argument,  0, '1' },
  //{"tiled-snapshot",      required_argument,  0, '8' },

  //{"seed",        required_argument,  0, 'S' },

  {"verbose",   required_argument,  0, 'V' },
  {"help",      no_argument,        0, 'h' },
  {"version",   no_argument,        0, 'v' },
  {0,           0,                  0,  0  },
};


static char long_options_descr[][128] = {
  "size of map (override default from poms JSON file)",
  "point to inspect",

  "POMS file",
  "print option (0 grid, 1 voxel)",
  "frequency option (0 simple, 1 tile difference)",
  //"output Tiled JSON file",
  //"Tiled snapshot file",

  //"random seed",

  "verbose level",
  "help (this screen)",
  "version"
};

void print_version(FILE *fp) {
  fprintf(fp, "%s\n", POMS_IR_VERSION);
}

void print_help(FILE *fp) {
  int lo_idx=0,
      spacing=0,
      ii;
  struct option *lo;

  fprintf(fp, "\n");
  fprintf(fp, "poms.ir, version: ");
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


void _print_freq(std::vector< int64_t > &freq, POMS &poms, int32_t tile_val=-1) {
  int32_t x,y,z;
  int64_t cell;

  for (z=0; z<poms.m_size[2]; z++) {
    for (y=0; y<poms.m_size[1]; y++) {

      if (tile_val >= 0) {
        printf("#freq[:%i]; ", tile_val);
      }

      for (x=0; x<poms.m_size[0]; x++) {
        if (x>0) { printf(" "); }
        cell = poms.xyz2cell(x,y,z);
        printf("%2i", (int)freq[cell]);
      }
      printf("\n");
    }
    printf("\n");
  }
  printf("\n");

}

void _print_voxel(std::vector< int64_t > &freq, POMS &poms) {
  int32_t x,y,z;
  int64_t cell;

  for (z=0; z<poms.m_size[2]; z++) {
    for (y=0; y<poms.m_size[1]; y++) {
      for (x=0; x<poms.m_size[0]; x++) {
        cell = poms.xyz2cell(x,y,z);
        if (freq[cell]==0) { continue; }
        printf("%i %i %i %i\n", x, y, z, (int)freq[cell]);
      }
      //printf("\n");
    }
    //printf("\n");
  }
  //printf("\n");

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

int main(int argc, char **argv) {
  int r, err, ret;
  POMS poms;

  int64_t cell;
  int32_t tile_idx,
          tile_val,
          tile_n,
          x,y,z,
          xyz,
          p[3],
          P[3],
          wblock[3][2];

  int opt_idx, seed, ch;


  std::string poms_cfg_fn;

  std::vector< std::string > tok;
  std::string s;
  char sep = ',';
  int32_t _opt_size[3],
          _opt_pnt[3];
  int _opt_print = 0,
      _opt_freq = 0;

  int32_t dim_order[3];

  std::vector< int64_t >  freq,
                          orig_size;

  dim_order[0] = 1;
  dim_order[1] = 0;
  dim_order[2] = 2;

  _opt_size[0] = -1;
  _opt_size[1] = -1;
  _opt_size[2] = -1;

  _opt_pnt[0] = -1;
  _opt_pnt[1] = -1;
  _opt_pnt[2] = -1;

  //---

  while (ch = getopt_long(argc, argv, "hvV:s:P:C:1:8:S:M:/:w:E:t:o:F:", long_options, &opt_idx)) {
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
        poms.m_verbose = atoi(optarg);
        break;

      case 'S':
        seed = atoi(optarg);
        break;

      case 'C':
        poms_cfg_fn = optarg;
        break;

      case 'o':
        _opt_print = atoi(optarg);
        break;

      case 'F':
        _opt_freq = atoi(optarg);
        break;

      case 's':
        s = optarg;
        splitStr(tok, s, sep);
        _opt_size[0] = ((tok.size() > 0) ? atoi(tok[0].c_str()) : 1);
        _opt_size[1] = ((tok.size() > 1) ? atoi(tok[1].c_str()) : _opt_size[0]);
        _opt_size[2] = ((tok.size() > 2) ? atoi(tok[2].c_str()) : _opt_size[1]);
        break;
      case 'P':
        s = optarg;
        splitStr(tok, s, sep);
        _opt_pnt[0] = ((tok.size() > 0) ? atoi(tok[0].c_str()) : -1);
        _opt_pnt[1] = ((tok.size() > 1) ? atoi(tok[1].c_str()) : _opt_pnt[0]);
        _opt_pnt[2] = ((tok.size() > 2) ? atoi(tok[2].c_str()) : _opt_pnt[1]);
        break;

      case '1':
        break;
      case '8':
        //g_ctx.tiled_fn = optarg;
        break;

      default:
        fprintf(stderr, "invalid option (%c)", (char)ch);
        print_help(stderr);
        exit(-1);
        break;
    }
  }

  if (poms_cfg_fn.size() == 0) {
    fprintf(stderr, "provide input POMS ocnfig file\n");
    print_help(stderr);
    exit(-1);
  }

  poms.rnd_seed(seed);

  if ((_opt_size[0] > 0) &&
      (_opt_size[1] > 0) &&
      (_opt_size[2] > 0)) {
    poms.m_size[0] = _opt_size[0];
    poms.m_size[1] = _opt_size[1];
    poms.m_size[2] = _opt_size[2];

    poms.m_quilt_size[0] = _opt_size[0];
    poms.m_quilt_size[1] = _opt_size[1];
    poms.m_quilt_size[2] = _opt_size[2];
  }

  poms.m_retry_max = 32;

  r = poms.loadJSONFile( poms_cfg_fn );
  if (r<0) {
    fprintf(stderr, "error loading file: %s\n", poms_cfg_fn.c_str());
    exit(-1);
  }

  r = poms.renew();
  if (r<0) { exit_err("ERROR: renew"); }
  memset( &(poms.m_cell_pin[0]), 0, sizeof(int8_t)*poms.m_cell_count);
  poms.resetAC4Dirty(poms.m_plane);

  //---

  r = poms.applyConstraints();
  if (r<0) { exit_err("ERROR: applyConstraints failed"); }

  r = poms.applyConstraints(0,1);
  if (r<0) { exit_err("ERROR: applyConstraints(0,1) failed"); }

  r = poms.cullSweep();
  if (r<0) {
    printf("#cullSeep failed with %i\n", r);
    poms.printDebugConflict();
    exit_err("ERROR: cullSweep failed");
  }

  r = poms.savePrefatory();
  if (r<0) { exit_err("ERROR: savePrefatory failed"); }

  //---

  r = poms.applyStartConstraints();
  if (r<0) { exit_err("ERROR: applyStartConstraints failed"); }

  r = poms.cullSweep();
  if (r<0) { exit_err("ERROR: cullSweep after applyStartConstraints failed"); }

  poms.m_tile_choice_policy = POMS_TILE_CHOICE_PROB;
  poms.m_block_choice_policy = POMS_BLOCK_CHOICE_MIN_ENTROPY;

  //----
  //----
  //----

  for (xyz=0; xyz<3; xyz++) {
    if (_opt_pnt[xyz] < 0) { _opt_pnt[xyz] = poms.m_size[xyz]/2; }
    P[xyz] = _opt_pnt[xyz];
  }

  if (poms.m_verbose >= POMS_VERBOSE_DEBUG) {
    printf("## P[%i,%i,%i] (_opt_pnt[%i,%i,%i]) m_size[%i,%i,%i]\n",
        (int)P[0], (int)P[1], (int)P[2],
        (int)_opt_pnt[0], (int)_opt_pnt[1], (int)_opt_pnt[2],
        (int)poms.m_size[0], (int)poms.m_size[1], (int)poms.m_size[2] );
  }

  for (xyz=0; xyz<3; xyz++) {
    wblock[xyz][0] = _clamp32( P[xyz]-1, 0, poms.m_size[xyz] );
    wblock[xyz][1] = _clamp32( P[xyz]+2, 0, poms.m_size[xyz] );
  }

  cell = poms.xyz2cell(P[0], P[1], P[2]);

  tile_n = poms.cellSize( poms.m_plane, cell );

  freq.resize( poms.m_size[0]*poms.m_size[1]*poms.m_size[2] );
  orig_size.resize( poms.m_size[0]*poms.m_size[1]*poms.m_size[2] );

  for (cell=0; cell<poms.m_cell_count; cell++) {
    orig_size[cell] = poms.cellSize( poms.m_plane, cell );

    //poms.markAC4Dirty(poms.m_plane, cell);
  }

  r = poms.saveGrid();

  if (poms.m_verbose >= POMS_VERBOSE_DEBUG) {
    poms.printDebug();
  }


  for (tile_val=0; tile_val < tile_n; tile_val++) {
    r = poms.restoreGrid();
    r = poms.saveGrid();

    cell = poms.xyz2cell( P[0], P[1], P[2] );
    poms.setCellTile( poms.m_plane, cell, 0, tile_val );
    poms.setCellSize( poms.m_plane, cell, 1 );

    if (poms.m_verbose >= POMS_VERBOSE_DEBUG2) {
      printf("## cell:%i[%i,%i,%i] tile_val:%i wblock[%i:%i,%i:%i,%i:%i]\n", (int)cell,
          (int)P[0], (int)P[1], (int)P[2], (int)tile_val,
          (int)wblock[0][0], (int)wblock[0][1],
          (int)wblock[1][0], (int)wblock[1][1],
          (int)wblock[2][0], (int)wblock[2][1] );
      fflush(stdout);
    }

    r = poms.AC4InitBlock( wblock, 1 );
    if (r<0) {
      if (poms.m_verbose >= POMS_VERBOSE_RUN) {
        printf("# invalid tile %i@(%i,%i,%i), skipping\n", tile_val, P[0], P[1], P[2]);
      }
      continue;
    }

    for (cell=0; cell<poms.m_cell_count; cell++) {

      if (_opt_freq == 1) {
        if (poms.cellSize( poms.m_plane, cell ) != orig_size[cell]) {
          freq[cell] += (orig_size[cell] - poms.cellSize( poms.m_plane, cell ));
        }
      }
      else {
        if (poms.cellSize( poms.m_plane, cell ) != orig_size[cell]) {
          freq[cell] ++;
        }
      }
    }

    if (poms.m_verbose >= POMS_VERBOSE_RUN) {
      if ((tile_val>0) &&
          ((tile_val % 100) == 0)) {

        if      (_opt_print==0) { _print_freq(freq, poms); }
        else if (_opt_print==1) { _print_voxel(freq, poms); }

      }
    }

  }
  r = poms.restoreGrid();

  if      (_opt_print==0) { _print_freq(freq, poms); }
  else if (_opt_print==1) { _print_voxel(freq, poms); }

  exit(0);
}
