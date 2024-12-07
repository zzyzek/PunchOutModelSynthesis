/*
 * To the extent possible under law, the person who associated CC0 with
 * this file has waived all copyright and related or neighboring rights
 * to this file.
 *
 * You should have received a copy of the CC0 legalcode along with this
 * work. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 *
 */

#define FNL_IMPL
#include "FastNoiseLite.h"

#include "main_poms.hpp"

g_ctx_t g_ctx = {0};

static double _rand() {
  static int init=0;
  double p;

  if (init==0) {
    init=1;
    srand(0xc0ffee);
  }

  p = (double)rand() / (1.0 + RAND_MAX);
  return p;
}


//----

typedef struct soften_window_opt_type {

  // inclusive max
  //
  int32_t soften_min[3],
          soften_max[3],
          soften_size[3];

  std::vector< double > cdf, pdf;

  int init_cdf(void) {
    int i, xyz;
    int32_t _max_dim = 1, _min_val=-1, _max_val=-1;
    double R = 0.0,
           _eps = (1.0 / (1024.0*1024.0));

    for (xyz=0; xyz<3; xyz++) {

      if ((xyz==0) ||
          ((soften_max[xyz] - soften_min[xyz]+1) > _max_dim)) {
        _max_dim = soften_max[xyz] - soften_min[xyz] + 1;
        _min_val = soften_min[xyz];
        _max_val = soften_max[xyz];
      }
    }

    if (_max_dim <= 0) { return -1; }

    cdf.clear();
    cdf.resize(_max_dim);

    pdf.clear();
    pdf.resize(_max_dim);

    for (i=0; i<pdf.size(); i++) {
      pdf[i] = 1.0 / (double)((double)_min_val + (double)i);
      R += pdf[i];
    }

    if (R < _eps) { return -1; }

    for (i=0; i<pdf.size(); i++) {
      pdf[i] /= R;
      cdf[i] = pdf[i];
      if (i>0) { cdf[i] += cdf[i-1]; }
    }

    return 0;
  }

  int choose_soften_size(int32_t _soften_size[3]) {
    int i;
    double p, q;

    p = _rand();
    for (i=0; i<(cdf.size()-1); i++) {
      if (p < cdf[i]) { break; }
    }
    q = (double)i / (double)cdf.size();

    _soften_size[0] = soften_min[0] + (int32_t)( ((double)(soften_max[0] - soften_min[0] + 1.0))*q );
    _soften_size[1] = soften_min[1] + (int32_t)( ((double)(soften_max[1] - soften_min[1] + 1.0))*q );
    _soften_size[2] = soften_min[2] + (int32_t)( ((double)(soften_max[2] - soften_min[2] + 1.0))*q );

    return 0;
  }

} soften_window_opt_t;


typedef struct _opt_type {
  int valid;
  int verbose;

  int size[3];
  int quilt_size[3];

  int block_size[3];
  //int soften_size[3];

  soften_window_opt_t soften_window;

  int seed;

  double rand_coefficient,
         rand_exponent;

  int viz_step;
  std::string gnuplot_fn;
  std::string cfg_fn;
  std::string tiled_fn;
  std::string sliced_tiled_fn;
  std::string stl_fn;
  std::string tiled_slideshow_dir;
  std::string tiled_snapshot_fn;

  std::string poms_block_snapshot_fn;

  std::string stl_snapshot_fn;
  //std::string patch_snapshot_fn;

  //std::string algorithm;

  std::vector< std::vector< float > > highlight_point;

  int retry_max;

  int tiled_fmt_type;

  int block_choice_policy;
  std::string block_choice_policy_str;

  int erode_count;
  std::string patch_choice_policy_str;

  int     noise_preset;

  int     noise_option;
  double  noise_freq;
  int     noise_seed;

  int     noise_test;

} _opt_t;


//----
//----


static struct option long_options[] = {

  {"size",      required_argument,  0, 's' },
  {"quilt-size",required_argument,  0, 'q' },
  {"block",     required_argument,  0, 'b' },
  {"soften",    required_argument,  0, 'B' },
  {"iter",      required_argument,  0, 'J' },

  {"rand-w",    required_argument,  0, 'w' },
  {"rand-E",    required_argument,  0, 'E' },

  {"block-policy",      required_argument,  0, 'P' },
  {"config",            required_argument,  0, 'C' },
  {"tiled",             required_argument,  0, '1' },
  {"stl",               required_argument,  0, '2' },
  {"tiled-poms",        required_argument,  0, '3' },
  {"sliced-tiled-poms", required_argument,  0, '4' },

  {"patch-snapshot",      required_argument,  0, '6' },
  {"stl-snapshot",        required_argument,  0, '7' },
  {"tiled-snapshot",      required_argument,  0, '8' },
  {"tiled-slideshow-dir", required_argument,  0, '9' },

  {"noise",       required_argument,  0, 'N' },
  {"viz",         required_argument,  0, '@' },

  {"seed",        required_argument,  0, 'S' },
  {"option",      required_argument,  0, 'O' },

  {"verbose",   required_argument,  0, 'V' },
  {"help",      no_argument,        0, 'h' },
  {"version",   no_argument,        0, 'v' },
  {0,           0,                  0,  0  },
};

static char long_options_descr[][128] = {
  "size of map",
  "quilt size of map",

  "block size",
  "soften block size",
  "iteration count",

  "random coefficient",
  "random exponent",

  "block-policy (min|max|seq)",
  "input config file",
  "output Tiled JSON file",
  "output STL file (requires objMap)",
  "output Tiled POMS JSON file",
  "output sliced Tiled POMS JSON file",

  "JSON patch snapshot file",
  "STL snapshot file",
  "tiled snapshot JSON file",
  "directory for tiled slideshow JSON files",

  "custom noise G function (<freq>:<seed>:<type>) (type {0:none,1:linear,2:threshold,3:tierd})",
  "gnuplot visualization file",

  "seed",
  "option",

  "verbose level",
  "help (this screen)",
  "show version",
  0
};

void print_version(FILE *fp) {
  fprintf(fp, "poms bin version: %s\n", POMS_BIN_VERSION);
  fprintf(fp, "poms lib version: %s\n", POMS_VERSION);
}

void print_help(FILE *fp) {
  int lo_idx=0,
      spacing=0,
      ii;
  struct option *lo;

  fprintf(fp, "\n");
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

//-----
//-----
//-----

int _tiled_slideshow(POMS &poms, _opt_t &opt, int id) {
  std::string fn;
  char fn_tmp[64];
  int r;

  snprintf(fn_tmp, 63, "%i.json", id);
  fn_tmp[63] = '\0';

  fn = opt.tiled_slideshow_dir;
  fn += "/";
  fn += fn_tmp;

  r = poms.exportTiledJSON( fn, 0 );
  return r;
}

int tiled_slideshow(g_ctx_t &ctx, _opt_t &opt, int64_t id) {
  char fn_tmp[128];
  FILE *fp;
  int r;

  snprintf(fn_tmp, 127, "%s/%06lli.json", opt.tiled_slideshow_dir.c_str(), (long long int)id);
  fn_tmp[127] = '\0';

  fp = fopen(fn_tmp, "w");
  if (!fp) { return -1; }

  r = exportTiledJSON( fp, ctx.T );

  fclose(fp);

  return r;
}

//---

int tiled_snapshot(POMS &poms, _opt_t &opt, int id) {
  int fd, r;
  FILE *fp;
  char fn_tmp[64];
  std::string fn;

  fn = opt.tiled_snapshot_fn;
  if (fn.size() == 0) { return -1; }

  strncpy( fn_tmp, "snapshot.json.XXXXXX", 32 );

  fd = mkstemp(fn_tmp);
  if (fd<0) { return fd; }

  fp = fdopen(fd, "w");
  if (!fp) { return -1; }

  r = poms.exportTiledJSON( fp, 0 );
  if (r<0) { return r; }

  fclose(fp);
  rename( fn_tmp, fn.c_str() );

  return r;
}

//---

// real time Tiled snapshot (for visualization).
//
// layer 0 - grid tile, value 0 for indeterminite
// layer 1 - cellsize
//
// creates/overwrites ctx->tiled_snapshot_fn file
//
int rt_tiled_snapshot(g_ctx_t *ctx) {
  int fd, r;
  FILE *fp;
  char fn_tmp[64];
  std::string fn;

  int64_t cell, rc;
  int32_t p[3], rp[3];
  POMS *poms;

  poms = ctx->poms;

  for (cell=0; cell<poms->m_quilt_cell_count; cell++) {
    poms->cell2vec(p, cell, poms->m_quilt_size);
    if ((p[0]  < poms->m_patch_region[0][0]) ||
        (p[0] >= poms->m_patch_region[0][1]) ||
        (p[1]  < poms->m_patch_region[1][0]) ||
        (p[1] >= poms->m_patch_region[1][1]) ||
        (p[2]  < poms->m_patch_region[2][0]) ||
        (p[2] >= poms->m_patch_region[2][1])) {

      if (poms->m_quilt_tile[cell] < 0) {
        ctx->T.layers[0].data[cell] = 0;
        ctx->T.layers[1].data[cell] = poms->m_tile_count;
        continue;
      }

      ctx->T.layers[0].data[cell] = poms->m_quilt_tile[cell];
      ctx->T.layers[1].data[cell] = 1;
      continue;
    }

    rp[0] = p[0] - poms->m_patch_region[0][0];
    rp[1] = p[1] - poms->m_patch_region[1][0];
    rp[2] = p[2] - poms->m_patch_region[2][0];
    rc = poms->vec2cell(rp);
    if (rc<0) { continue; }

    ctx->T.layers[0].data[cell] = poms->cellTile( poms->m_plane, rc, 0 );
    ctx->T.layers[1].data[cell] = poms->cellSize( poms->m_plane, rc );

  }

  fn = ctx->tiled_snapshot_fn;
  if (fn.size() == 0) { return -1; }

  strncpy( fn_tmp, "snapshot.json.XXXXXX", 32 );

  fd = mkstemp(fn_tmp);
  if (fd<0) { return fd; }

  fp = fdopen(fd, "w");
  if (!fp) { return -1; }

  r = exportTiledJSON( fp, ctx->T );
  if (r<0) { return r; }

  fclose(fp);
  rename( fn_tmp, fn.c_str() );

  return r;
}

//---

// real time sliced Tiled snapshot (for visualization).
//
// layer 0 - grid tile, value 0 for indeterminite
// layer 1 - cellsize
//
// creates/overwrites ctx->tiled_snapshot_fn file
//
int rt_sliced_tiled_snapshot(g_ctx_t *ctx, int32_t cols) {
  int fd, r;
  FILE *fp;
  char fn_tmp[64];
  std::string fn;

  int64_t cell, rc;
  int32_t p[3], rp[3];
  POMS *poms;

  poms = ctx->poms;

  for (cell=0; cell<poms->m_quilt_cell_count; cell++) {
    poms->cell2vec(p, cell, poms->m_quilt_size);
    if ((p[0]  < poms->m_patch_region[0][0]) ||
        (p[0] >= poms->m_patch_region[0][1]) ||
        (p[1]  < poms->m_patch_region[1][0]) ||
        (p[1] >= poms->m_patch_region[1][1]) ||
        (p[2]  < poms->m_patch_region[2][0]) ||
        (p[2] >= poms->m_patch_region[2][1])) {

      if (poms->m_quilt_tile[cell] < 0) {
        ctx->T.layers[0].data[cell] = 0;
        ctx->T.layers[1].data[cell] = poms->m_tile_count;
        continue;
      }

      ctx->T.layers[0].data[cell] = poms->m_quilt_tile[cell];
      ctx->T.layers[1].data[cell] = 1;
      continue;
    }

    rp[0] = p[0] - poms->m_patch_region[0][0];
    rp[1] = p[1] - poms->m_patch_region[1][0];
    rp[2] = p[2] - poms->m_patch_region[2][0];
    rc = poms->vec2cell(rp);
    if (rc<0) { continue; }

    ctx->T.layers[0].data[cell] = poms->cellTile( poms->m_plane, rc, 0 );
    ctx->T.layers[1].data[cell] = poms->cellSize( poms->m_plane, rc );

  }

  fn = ctx->tiled_snapshot_fn;
  if (fn.size() == 0) { return -1; }

  strncpy( fn_tmp, "snapshot.json.XXXXXX", 32 );

  fd = mkstemp(fn_tmp);
  if (fd<0) { return fd; }

  fp = fdopen(fd, "w");
  if (!fp) { return -1; }

  r = exportTiledJSON( fp, ctx->T );
  if (r<0) { return r; }

  fclose(fp);
  rename( fn_tmp, fn.c_str() );

  return r;
}


//---

_opt_t *g_opt=NULL;
POMS *g_poms=NULL;

int viz_gnuplot_cellfreq_4d(POMS &poms, _opt_t &opt);

int viz_cb(void) {
  static int64_t seq=0;
  int64_t viz_step=1;
  int ret;

  if ((g_poms==NULL) || (g_opt==NULL)) { return -1; }

  if (g_opt->viz_step > 0) { viz_step = g_opt->viz_step; }

  seq++;
  if ((seq%viz_step)!=0) { return 1; }

  if (g_opt->tiled_slideshow_dir.size() > 0) {
    //ret = tiled_slideshow(*g_poms, *g_opt, seq);
    ret = tiled_slideshow(g_ctx, *g_opt, seq);
    if (ret<0) { return ret; }
  }

  if (g_opt->gnuplot_fn.size() > 0) {
    ret = viz_gnuplot_cellfreq_4d(*g_poms, *g_opt);
    if (ret<0) { return ret; }
  }

  if (g_opt->tiled_snapshot_fn.size() > 0) {
    ret = tiled_snapshot(*g_poms, *g_opt, seq);
    if (ret<0){ return ret; }
  }

  return ret;
}

int viz_gnuplot_cellfreq_4d(POMS &poms, _opt_t &opt) {
  int fd;
  FILE *fp;
  std::string fn;
  char fn_tmp[64];

  int64_t cell;
  int32_t X,Y,Z,
          ix,iy,iz,
          tile_n;
  double f;

  double dxyz[3], df;
  int32_t max_tile_n=-1;

  int i, j;

  if ((opt.viz_step <= 0) ||
      (opt.gnuplot_fn.size() == 0)) { return -1; }

  fn = opt.gnuplot_fn;

  strncpy( fn_tmp, "viz.gp.XXXXXX", 32 );
  fd = mkstemp(fn_tmp);
  if (fd<0) { return fd; }
  fp = fdopen(fd, "w");
  if (!fp) { return -1; }

  X = poms.m_size[0];
  Y = poms.m_size[1];
  Z = poms.m_size[2];

  fprintf(fp, "%f %f %f 0\n",
      -1.0/(double)X,
      -1.0/(double)Y,
      -1.0/(double)Z);


  for (ix=0; ix<X; ix++) {
    for (iy=0; iy<Y; iy++) {
      for (iz=0; iz<Z; iz++) {

        cell    = poms.xyz2cell(ix,iy,iz);
        tile_n  = poms.cellSize( poms.m_plane, cell );

        if (tile_n<=1) { continue; }

        f = (double)tile_n / (double)poms.m_tile_count;

        dxyz[0] = (double)ix / (double)X;
        dxyz[1] = (double)iy / (double)Y;
        dxyz[2] = (double)iz / (double)Z;

        //fprintf(fp, "%i %i %i %f\n", (int)ix, (int)iy, (int)iz, (double)f);
        fprintf(fp, "%f %f %f %f\n",
            dxyz[0], dxyz[1], dxyz[2], (double)f);
      }
      fprintf(fp, "\n");
    }
    fprintf(fp, "\n");
  }
  fprintf(fp, "\n");

  fprintf(fp, "%f %f %f 1\n",
      (double)(X+1.0)/(double)X,
      (double)(Y+1.0)/(double)Y,
      (double)(Z+1.0)/(double)Z);

  for (i=0; i<opt.highlight_point.size(); i++) {
    if (i==0) { fprintf(fp, "\n\n"); }
    if (opt.highlight_point[i].size() < 4) { continue; }
    fprintf(fp, "%f %f %f %f\n",
        opt.highlight_point[i][0],
        opt.highlight_point[i][1],
        opt.highlight_point[i][2],
        opt.highlight_point[i][3]);
  }

  fclose(fp);
  rename( fn_tmp, fn.c_str() );

  return 0;
}


//-----
//-----
//-----

int exportQuiltJSON(FILE *fp, POMS *poms, int active_region=0) {
  int64_t cell, patch_cell;
  int32_t x, y, z,
          v[3],
          tile;
  int i, r;

  int fold = 8;

  int64_t fold_x, fold_xy;

  fold_x = poms->m_quilt_size[0];
  fold_xy = poms->m_quilt_size[1] * fold_x;

  fprintf(fp, "{\n");

  fprintf(fp, "  \"size\":[%i,%i,%i],\n",
      (int)poms->m_quilt_size[0],
      (int)poms->m_quilt_size[1],
      (int)poms->m_quilt_size[2]);
  fprintf(fp, "  \"active_region\":%i,\n", (int)active_region);
  fprintf(fp, "  \"patch_region\":[[%i,%i],[%i,%i],[%i,%i]],\n",
      (int)poms->m_patch_region[0][0], (int)poms->m_patch_region[0][1],
      (int)poms->m_patch_region[1][0], (int)poms->m_patch_region[1][1],
      (int)poms->m_patch_region[2][0], (int)poms->m_patch_region[2][1]);
  fprintf(fp, "  \"offset\":[0,0,0],\n");

  if (active_region) {
    fprintf(fp, "  \"cellSize\":[\n");

    for (cell=0; cell<poms->m_quilt_cell_count; cell++) {

      if ((cell % fold_x) == 0) { fprintf(fp, "\n"); }
      if ((cell % fold_xy) == 0) { fprintf(fp, "\n"); }

      poms->cell2vec(v, cell, poms->m_quilt_size);
      if ((v[0] <  poms->m_patch_region[0][0]) ||
          (v[0] >= poms->m_patch_region[0][1]) ||
          (v[1] <  poms->m_patch_region[1][0]) ||
          (v[1] >= poms->m_patch_region[1][1]) ||
          (v[2] <  poms->m_patch_region[2][0]) ||
          (v[2] >= poms->m_patch_region[2][1])) {
        fprintf(fp, "%i", (poms->m_quilt_tile[cell] < 0) ? (int)poms->m_tile_count : 1 );
      }
      else {
        v[0] -= poms->m_patch_region[0][0];
        v[1] -= poms->m_patch_region[1][0];
        v[2] -= poms->m_patch_region[2][0];
        patch_cell = poms->vec2cell(v);
        fprintf(fp, "%i", (int)poms->cellSize(poms->m_plane, patch_cell));
      }

      if (cell < (poms->m_quilt_cell_count-1)) { fprintf(fp, ","); }

    }

    fprintf(fp, "  ],\n");
  }

  fprintf(fp, "  \"patch\":[");
  for (cell=0; cell<poms->m_quilt_cell_count; cell++) {

    if ((cell % fold_x) == 0) { fprintf(fp, "\n"); }
    if ((cell % fold_xy) == 0) { fprintf(fp, "\n"); }

    poms->cell2vec(v, cell, poms->m_quilt_size);
    if (active_region) {
      if ((v[0] <  poms->m_patch_region[0][0]) ||
          (v[0] >= poms->m_patch_region[0][1]) ||
          (v[1] <  poms->m_patch_region[1][0]) ||
          (v[1] >= poms->m_patch_region[1][1]) ||
          (v[2] <  poms->m_patch_region[2][0]) ||
          (v[2] >= poms->m_patch_region[2][1])) {
        fprintf(fp, "%i", poms->m_quilt_tile[cell]);
      }
      else {
        v[0] -= poms->m_patch_region[0][0];
        v[1] -= poms->m_patch_region[1][0];
        v[2] -= poms->m_patch_region[2][0];
        patch_cell = poms->vec2cell(v);
        fprintf(fp, "%i", poms->cellTile(poms->m_plane, patch_cell, 0));
      }
    }
    else {
      fprintf(fp, "%i", poms->m_quilt_tile[cell]);
    }

    if (cell < (poms->m_quilt_cell_count-1)) { fprintf(fp, ","); }

  }
  fprintf(fp, "  ]\n");
  fprintf(fp, "}\n");

  return 0;
}

int rt_patch_snapshot(g_ctx_t *ctx, int active_region=0) {
  int fd, r;
  FILE *fp;
  char fn_tmp[64];
  std::string fn;

  int64_t cell, rc;
  int32_t p[3], rp[3];
  POMS *poms;

  poms = ctx->poms;

  fn = ctx->patch_snapshot_fn;
  if (fn.size() == 0) { return -1; }

  strncpy( fn_tmp, "patch_snapshot.json.XXXXXX", 32 );

  fd = mkstemp(fn_tmp);
  if (fd<0) { return fd; }

  fp = fdopen(fd, "w");
  if (!fp) { return -1; }

  r = exportQuiltJSON(fp, poms, active_region);
  if (r<0) { return r; }

  fclose(fp);
  rename( fn_tmp, fn.c_str() );

  return r;
}



int exportQuiltSTL(POMS &poms, std::string &out_fn) {
  int64_t cell;
  int32_t x, y, z,
          tile;
  double dx, dy, dz;
  int i, r;
  std::vector< std::vector< double > > stlTile;
  std::vector< double > tri;

  std::string solid_name = "";

  FILE *fp = fopen(out_fn.c_str(), "w");
  if (!fp) { return -1; }

  if (out_fn.size() == 0) { return -1; }

  stlTile.clear();

  for (i=0; i<poms.m_objMap.size(); i++) {
    tri.clear();
    r = auxExport_obj2tri( tri, poms.m_objMap[i] );
    if (r<0) {
      if (poms.m_verbose >= POMS_VERBOSE_ERROR) {
        fprintf(stderr, "ERROR: trying to open OBJ file '%s', got: %i\n",
            poms.m_objMap[i].c_str(), r);
      }
    }
    stlTile.push_back(tri);
  }


  fprintf(fp, "solid %s\n", solid_name.c_str());

  for (z=0; z<poms.m_quilt_size[2]; z++) {
    for (y=0; y<poms.m_quilt_size[1]; y++) {
      for (x=0; x<poms.m_quilt_size[0]; x++) {

        dx = (double)x;
        dy = (double)y;
        dz = (double)z;

        cell = poms.xyz2cell(x,y,z, poms.m_quilt_size);
        tile = poms.m_quilt_tile[cell];

        tri.clear();

        if (tile < 0) { continue; }

        for (i=0; i<stlTile[tile].size(); i+=3) {
          tri.push_back( stlTile[tile][i+0] + dx );
          tri.push_back( stlTile[tile][i+1] + dy );
          tri.push_back( stlTile[tile][i+2] + dz );
        }

        r = auxExport_tri2stl_facets( fp, tri );
        if (r<0) {
          fclose(fp);
          return r;
        }


      }
    }
  }

  fprintf(fp, "endsolid %s\n", solid_name.c_str());

  fclose(fp);
  return 0;
}

int _exportSTL(POMS &poms, std::string &out_fn) {
  int64_t cell;
  int32_t x, y, z,
          tile;
  double dx, dy, dz;
  int i, r;
  std::vector< std::vector< double > > stlTile;
  std::vector< double > tri;

  if (out_fn.size() == 0) { return -1; }

  stlTile.clear();

  for (i=0; i<poms.m_objMap.size(); i++) {
    tri.clear();
    r = auxExport_obj2tri( tri, poms.m_objMap[i] );
    if (r<0) {
      if (poms.m_verbose >= POMS_VERBOSE_ERROR) {
        fprintf(stderr, "ERROR: trying to open OBJ file '%s', got: %i\n",
            poms.m_objMap[i].c_str(), r);
      }
    }
    stlTile.push_back(tri);
  }

  tri.clear();

  for (z=0; z<poms.m_size[2]; z++) {
    for (y=0; y<poms.m_size[1]; y++) {
      for (x=0; x<poms.m_size[0]; x++) {

        dx = (double)x;
        dy = (double)y;
        dz = (double)z;

        cell = poms.xyz2cell(x,y,z);
        tile = poms.cellTile( poms.m_plane, cell, 0);

        for (i=0; i<stlTile[tile].size(); i+=3) {
          tri.push_back( stlTile[tile][i+0] + dx );
          tri.push_back( stlTile[tile][i+1] + dy );
          tri.push_back( stlTile[tile][i+2] + dz );
        }

      }
    }
  }

  r = auxExport_tri2stl( out_fn, tri );

  return r;
}

void err_and_exit(const char *s) {
  fprintf(stderr, "%s\n", s);
  exit(-1);
}

int err_and_return(const char *s) {
  fprintf(stderr, "%s\n", s);
  return -1;
}

//-----
//-----
//-----

int _debug_export_quilt(POMS &poms) {

  int i;
  char buf[8];
  std::string fn;

  int64_t qcell, _count, _fold;
  int32_t x,y,z;

  FILE *fp;

  _fold = 64;

  if ((poms.m_patch_region[0][1] - poms.m_patch_region[0][0]) < _fold) {
    _fold = poms.m_patch_region[0][1] - poms.m_patch_region[0][0];
  }

  for (i=0; i<7; i++) {
    buf[i] = (rand()%26)  + 'a';
  }
  buf[7] = '\0';

  fn = "debug_quilt_export_";
  fn += buf;
  fn += ".json";

  //DEBUG
  printf("### export debug patch: %s\n", fn.c_str());

  fp = fopen(fn.c_str(), "w");
  if (!fp ) { return -1; }

  fprintf(fp, "{\n");
  fprintf(fp, "  \"tile\":[\n");

  _count=0;
  for (z=0; z<poms.m_quilt_size[2]; z++) {
    for (y=0; y<poms.m_quilt_size[1]; y++) {
      for (x=0; x<poms.m_quilt_size[0]; x++) {
        qcell = poms.xyz2cell(x,y,z, poms.m_quilt_size);

        if (_count>0) { fprintf(fp, ","); }
        if ((_count%_fold)==0) {
          if (_count>0) { fprintf(fp, "\n"); }
          fprintf(fp, "    ");
        }

        if (qcell < 0) { fprintf(fp, "-1"); }
        else {
          fprintf(fp, "%i", (int)poms.m_quilt_tile[qcell]);
        }
        _count++;
      }
    }
  }
  fprintf(fp, "  ],\n");
  fprintf(fp, "  \"size\":[%i,%i,%i]\n",
      (int)poms.m_quilt_size[0], (int)poms.m_quilt_size[1], (int)poms.m_quilt_size[2]);
  fprintf(fp, "}\n");


  fclose(fp);
  return 0;
}

int _debug_export_patch(POMS &poms) {

  int i;
  char buf[8];
  std::string fn;

  int64_t qcell, _count, _fold;
  int32_t x,y,z;

  FILE *fp;

  _fold = 64;

  if ((poms.m_patch_region[0][1] - poms.m_patch_region[0][0]) < _fold) {
    _fold = poms.m_patch_region[0][1] - poms.m_patch_region[0][0];
  }

  for (i=0; i<7; i++) {
    buf[i] = (rand()%26)  + 'a';
  }
  buf[7] = '\0';

  fn = "debug_patch_export_";
  fn += buf;
  fn += ".json";

  //DEBUG
  printf("### export debug patch: %s\n", fn.c_str());

  fp = fopen(fn.c_str(), "w");
  if (!fp ) { return -1; }

  fprintf(fp, "{\n");
  fprintf(fp, "  \"tile\":[\n");

  _count=0;
  for (z=poms.m_patch_region[2][0]; z<poms.m_patch_region[2][1]; z++) {
    for (y=poms.m_patch_region[1][0]; y<poms.m_patch_region[1][1]; y++) {
      for (x=poms.m_patch_region[0][0]; x<poms.m_patch_region[0][1]; x++) {
        qcell = poms.xyz2cell(x,y,z, poms.m_quilt_size);

        if (_count>0) { fprintf(fp, ","); }
        if ((_count%_fold)==0) {
          if (_count>0) { fprintf(fp, "\n"); }
          fprintf(fp, "    ");
        }

        if (qcell < 0) { fprintf(fp, "-1"); }
        else {
          fprintf(fp, "%i", (int)poms.m_quilt_tile[qcell]);
        }
        _count++;
      }
    }
  }
  fprintf(fp, "  ],\n");
  fprintf(fp, "  \"range\":{\n");
  fprintf(fp, "    \"x\":[%i,%i],\n", (int)poms.m_patch_region[0][0], (int)poms.m_patch_region[0][1]);
  fprintf(fp, "    \"y\":[%i,%i],\n", (int)poms.m_patch_region[1][0], (int)poms.m_patch_region[1][1]);
  fprintf(fp, "    \"z\":[%i,%i]\n", (int)poms.m_patch_region[2][0], (int)poms.m_patch_region[2][1]);
  fprintf(fp, "  }\n");
  fprintf(fp, "}\n");


  fclose(fp);
  return 0;
}

//-----
//-----
//-----


int _patch_choice_rand(int32_t *block_start, int32_t *block_ds, int32_t *block_size, int32_t *grid_size) {

  int32_t rx=0, ry=0, rz=0;

  if ((grid_size[0] - block_size[0]) > 0) { rx = rand() % (grid_size[0] - block_size[0]+1); }
  if ((grid_size[1] - block_size[1]) > 0) { ry = rand() % (grid_size[1] - block_size[1]+1); }
  if ((grid_size[2] - block_size[2]) > 0) { rz = rand() % (grid_size[2] - block_size[2]+1); }

  block_start[0] = rx;
  block_start[1] = ry;
  block_start[2] = rz;

  return 0;
}

int _patch_choice_weight_conflict(g_ctx_t &ctx,
                                  int32_t *block_start,
                                  int32_t *block_ds,
                                  int32_t *block_size,
                                  int32_t *grid_size ) {
  int32_t x,y,z;
  int64_t cell;
  POMS *poms;

  double count=0.0, val, p;
  int32_t _ds[3], _dss[3];
  int xyz=0;

  poms = ctx.poms;

  _ds[0] = block_size[0]/2;
  _ds[1] = block_size[1]/2;
  _ds[2] = block_size[2]/2;

  _dss[0] = block_size[0]/4;
  _dss[1] = block_size[1]/4;
  _dss[2] = block_size[2]/4;

  _patch_choice_rand(block_start, block_ds, block_size, grid_size);

  for (cell=0; cell<poms->m_quilt_cell_count; cell++) {

    val = (double)ctx.m_conflict_grid[cell];
    if (val < 0.5) { continue; }

    count += val;

    p = poms->rnd();
    if (p < (val / count)) {
      poms->cell2vec(block_start, cell, poms->m_quilt_size);
    }
  }

  // recenter and jitter
  //
  if (count > 0) {
    for (xyz=0; xyz<3; xyz++) {
      block_start[xyz] -= _ds[xyz];
      block_start[xyz] -= (int32_t)((poms->rnd()-0.5)*_dss[xyz]);
    }
  }

  // clamp
  //
  if (block_start[0] < 0) { block_start[0] = 0; }
  if (block_start[1] < 0) { block_start[1] = 0; }
  if (block_start[2] < 0) { block_start[2] = 0; }

  if ((block_start[0] + block_size[0]) > grid_size[0]) { block_start[0] = grid_size[0] - block_size[0]; }
  if ((block_start[1] + block_size[1]) > grid_size[1]) { block_start[1] = grid_size[1] - block_size[1]; }
  if ((block_start[2] + block_size[2]) > grid_size[2]) { block_start[2] = grid_size[2] - block_size[2]; }

  return 0;
}

double _wf_f(double x) {

  //return x*x*x*x;

  return exp(-x*x);
  //return exp(-x);

  //return (1.0-x)*(1.0-x)*(1.0-x)*(1.0-x);

}

int _patch_choice_wf_cone0p( g_ctx_t &ctx,
                      int32_t *block_start,
                      int32_t *block_ds,
                      int32_t *block_size,
                      int32_t *grid_size) {
  int64_t cell;
  double count=0.0, p;
  int32_t _ds[3], xyz, v[3];
  double d, _sum=0.0;

  double maxP = -1.0,
         val=0.0,
         u = -1.0,
         max_v=0.0,
         max_u=0.0,
         min_v=-1.0,
         min_u=-1.0;

  double  max_xyz[3],
          dx,dy,dz,
          cx,cy,cz;

  int32_t min_pos[3];

  POMS *poms;

  poms = ctx.poms;

  _ds[0] = block_size[0]/2;
  _ds[1] = block_size[1]/2;
  _ds[2] = block_size[2]/2;

  cx = (double)grid_size[0] / 2.0;
  cy = (double)grid_size[1] / 2.0;
  cz = (double)grid_size[2] / 2.0;

  block_start[0] = 0;
  block_start[1] = 0;
  block_start[2] = 0;

  max_u = (double)( poms->m_quilt_size[0] + poms->m_quilt_size[1] + poms->m_quilt_size[2] - 3.0 + 1.0 );

  max_xyz[0] = (double)(poms->m_quilt_size[0]-1);
  max_xyz[1] = (double)(poms->m_quilt_size[1]-1);
  max_xyz[2] = (double)(poms->m_quilt_size[2]-1);

  min_u = -1.0;
  for (cell=0; cell<poms->m_quilt_cell_count; cell++) {
    if (poms->m_quilt_tile[cell] >= 0) { continue; }
    if (poms->m_quilt_pin[cell] != 0) { continue; }

    poms->cell2vec(v, cell, poms->m_quilt_size);

    dx = ((double)v[0]) - cx;
    dy = ((double)v[1]) - cy;
    dz = ((double)v[2]) - cz;

    u = sqrt( (dx*dx) + (dy*dy) + (dz*dz) );
    if ((min_u < 0) ||
        (u < min_u)) {
      min_u = u;
      min_pos[0] = v[0];
      min_pos[1] = v[1];
      min_pos[2] = v[2];
    }

  }

  printf("## min_u: %f, min_pos[%i,%i,%i], max_u:%f\n",
      min_u, min_pos[0], min_pos[1], min_pos[2], max_u);
  printf("## quilt_cell_count:%i\n", (int)poms->m_quilt_cell_count);


  _sum = 0.0;
  for (cell=0; cell<poms->m_quilt_cell_count; cell++) {

    if (poms->m_quilt_tile[cell] >= 0) { continue; }
    if (poms->m_quilt_pin[cell] != 0) { continue; }
    poms->cell2vec(v, cell, poms->m_quilt_size);

    dx = ((double)v[0]) - cx;
    dy = ((double)v[1]) - cy;
    dz = ((double)v[2]) - cz;

    u = sqrt( (dx*dx) + (dy*dy) + (dz*dz) );
    u -= min_u;

    //val = _wf_f(u);
    val = pow(u, 6.0);
    _sum += val;

    p = poms->rnd();
    if (p < (val / _sum)) {
      block_start[0] = v[0];
      block_start[1] = v[1];
      block_start[2] = v[2];
    }

  }


  //if (_sum > poms->m_zero) {
  if (min_u > -poms->m_zero) {
    for (xyz=0; xyz<3; xyz++) {
      d = poms->noisePowerLaw(0.125, -2.0);
      if (poms->rnd() < 0.5) { d = -d; }

      block_start[xyz] -= _ds[xyz];
    }
  }

  if (block_start[0] < 0) { block_start[0] = 0; }
  if (block_start[1] < 0) { block_start[1] = 0; }
  if (block_start[2] < 0) { block_start[2] = 0; }

  if ((block_start[0] + block_size[0]) > grid_size[0]) { block_start[0] = grid_size[0] - block_size[0]; }
  if ((block_start[1] + block_size[1]) > grid_size[1]) { block_start[1] = grid_size[1] - block_size[1]; }
  if ((block_start[2] + block_size[2]) > grid_size[2]) { block_start[2] = grid_size[2] - block_size[2]; }

  return 0;
}

int _patch_choice_wf_cone0n( g_ctx_t &ctx,
                      int32_t *block_start,
                      int32_t *block_ds,
                      int32_t *block_size,
                      int32_t *grid_size) {
  int64_t cell;
  int32_t _ds[3], xyz, v[3];
  double d, _sum=0.0,
         p, val=0.0,
         cur_max = 0.0,
         u = -1.0,
         max_v=0.0,
         max_u=0.0;

  double dx,dy,dz,
         cx,cy,cz;

  int32_t max_pos[3];

  POMS *poms;

  poms = ctx.poms;

  _ds[0] = block_size[0]/2;
  _ds[1] = block_size[1]/2;
  _ds[2] = block_size[2]/2;

  cx = (double)grid_size[0] / 2.0;
  cy = (double)grid_size[1] / 2.0;
  cz = (double)grid_size[2] / 2.0;

  block_start[0] = 0;
  block_start[1] = 0;
  block_start[2] = 0;

  max_u = -1.0;
  for (cell=0; cell<poms->m_quilt_cell_count; cell++) {
    if (poms->m_quilt_tile[cell] >= 0) { continue; }
    if (poms->m_quilt_pin[cell] != 0) { continue; }

    poms->cell2vec(v, cell, poms->m_quilt_size);

    dx = ((double)v[0]) - cx;
    dy = ((double)v[1]) - cy;
    dz = ((double)v[2]) - cz;

    u = sqrt( (dx*dx) + (dy*dy) + (dz*dz) );
    if ((max_u < 0) ||
        (u > max_u)) {
      max_u = u;
      max_pos[0] = v[0];
      max_pos[1] = v[1];
      max_pos[2] = v[2];
    }

  }

  cur_max = -1.0;
  _sum = 0.0;
  for (cell=0; cell<poms->m_quilt_cell_count; cell++) {

    if (poms->m_quilt_tile[cell] >= 0) { continue; }
    if (poms->m_quilt_pin[cell] != 0) { continue; }
    poms->cell2vec(v, cell, poms->m_quilt_size);

    dx = ((double)v[0]) - cx;
    dy = ((double)v[1]) - cy;
    dz = ((double)v[2]) - cz;

    u = max_u - sqrt( (dx*dx) + (dy*dy) + (dz*dz) );

    val = pow(u, 6.0);
    _sum += val;

    if ((cur_max < 0.0) ||
        (u > cur_max)) {
      cur_max = u;
      block_start[0] = v[0];
      block_start[1] = v[1];
      block_start[2] = v[2];
    }

  }


  //if (_sum > poms->m_zero) {
  if (cur_max > -poms->m_zero) {
    for (xyz=0; xyz<3; xyz++) {
      block_start[xyz] -= _ds[xyz];

      block_start[xyz] += poms->irnd(_ds[xyz]/2) - (int32_t)(_ds[xyz]/4);
    }
  }

  if (block_start[0] < 0) { block_start[0] = 0; }
  if (block_start[1] < 0) { block_start[1] = 0; }
  if (block_start[2] < 0) { block_start[2] = 0; }

  if ((block_start[0] + block_size[0]) > grid_size[0]) { block_start[0] = grid_size[0] - block_size[0]; }
  if ((block_start[1] + block_size[1]) > grid_size[1]) { block_start[1] = grid_size[1] - block_size[1]; }
  if ((block_start[2] + block_size[2]) > grid_size[2]) { block_start[2] = grid_size[2] - block_size[2]; }

  return 0;
}

int _patch_choice_wf_cone1n( g_ctx_t &ctx,
                      int32_t *block_start,
                      int32_t *block_ds,
                      int32_t *block_size,
                      int32_t *grid_size) {
  int64_t cell;
  int32_t _ds[3], xyz, v[3];
  double d, _sum=0.0,
         p, val=0.0,
         cur_max = 0.0,
         u = -1.0,
         max_v=0.0,
         max_u=0.0;

  double dx,dy,dz,
         cx,cy,cz;

  int32_t max_pos[3];

  POMS *poms;

  poms = ctx.poms;

  _ds[0] = block_size[0]/2;
  _ds[1] = block_size[1]/2;
  _ds[2] = block_size[2]/2;

  cx = (double)grid_size[0] / 2.0;
  cy = (double)grid_size[1] / 2.0;
  cy = 0.0;
  cz = (double)grid_size[2] / 2.0;

  block_start[0] = 0;
  block_start[1] = 0;
  block_start[2] = 0;

  max_u = -1.0;
  for (cell=0; cell<poms->m_quilt_cell_count; cell++) {
    if (poms->m_quilt_tile[cell] >= 0) { continue; }
    if (poms->m_quilt_pin[cell] != 0) { continue; }

    poms->cell2vec(v, cell, poms->m_quilt_size);

    dx = ((double)v[0]) - cx;
    dy = ((double)v[1]) - cy;
    dz = ((double)v[2]) - cz;

    u = sqrt( (dx*dx) + (dy*dy) + (dz*dz) );
    if ((max_u < 0) ||
        (u > max_u)) {
      max_u = u;
      max_pos[0] = v[0];
      max_pos[1] = v[1];
      max_pos[2] = v[2];
    }

  }

  cur_max = -1.0;
  _sum = 0.0;
  for (cell=0; cell<poms->m_quilt_cell_count; cell++) {

    if (poms->m_quilt_tile[cell] >= 0) { continue; }
    if (poms->m_quilt_pin[cell] != 0) { continue; }
    poms->cell2vec(v, cell, poms->m_quilt_size);

    dx = ((double)v[0]) - cx;
    dy = ((double)v[1]) - cy;
    dz = ((double)v[2]) - cz;

    u = max_u - sqrt( (dx*dx) + (dy*dy) + (dz*dz) );

    val = pow(u, 6.0);
    _sum += val;

    if ((cur_max < 0.0) ||
        (u > cur_max)) {
      cur_max = u;
      block_start[0] = v[0];
      block_start[1] = v[1];
      block_start[2] = v[2];
    }

  }


  //if (_sum > poms->m_zero) {
  if (cur_max > -poms->m_zero) {
    for (xyz=0; xyz<3; xyz++) {
      block_start[xyz] -= _ds[xyz];
    }
  }

  if (block_start[0] < 0) { block_start[0] = 0; }
  if (block_start[1] < 0) { block_start[1] = 0; }
  if (block_start[2] < 0) { block_start[2] = 0; }

  if ((block_start[0] + block_size[0]) > grid_size[0]) { block_start[0] = grid_size[0] - block_size[0]; }
  if ((block_start[1] + block_size[1]) > grid_size[1]) { block_start[1] = grid_size[1] - block_size[1]; }
  if ((block_start[2] + block_size[2]) > grid_size[2]) { block_start[2] = grid_size[2] - block_size[2]; }

  return 0;
}

// simple max cone
//
int _patch_choice_wf_cone1p( g_ctx_t &ctx,
                      int32_t *block_start,
                      int32_t *block_ds,
                      int32_t *block_size,
                      int32_t *grid_size) {
  int64_t cell;
  int32_t _ds[3],
          xyz,
          v[3];
  double d, _sum=0.0,
         u = -1.0;

  double  cur_max,
          dx,dy,dz,
          cx,cy,cz;

  POMS *poms;

  poms = ctx.poms;

  _ds[0] = block_size[0]/2;
  _ds[1] = block_size[1]/2;
  _ds[2] = block_size[2]/2;

  cx = (double)grid_size[0] / 2.0;
  cy = (double)grid_size[1] / 2.0;
  cz = (double)grid_size[2] / 2.0;

  cx -= (double)block_size[0];
  cy = (double)grid_size[1];

  block_start[0] = 0;
  block_start[1] = 0;
  block_start[2] = 0;

  cur_max = -1.0;
  for (cell=0; cell<poms->m_quilt_cell_count; cell++) {

    if (poms->m_quilt_tile[cell] >= 0) { continue; }
    if (poms->m_quilt_pin[cell] != 0) { continue; }
    poms->cell2vec(v, cell, poms->m_quilt_size);

    dx = ((double)v[0]) - cx;
    dy = ((double)v[1]) - cy;
    dz = ((double)v[2]) - cz;

    u = sqrt( (dx*dx) + (dy*dy) + (dz*dz) );

    if ((cur_max < 0.0) ||
        (cur_max < u)) {
      cur_max = u;
      block_start[0] = v[0];
      block_start[1] = v[1];
      block_start[2] = v[2];
    }

  }

  for (xyz=0; xyz<3; xyz++) {
    block_start[xyz] -= _ds[xyz];
    if (block_start[xyz] < 0) { block_start[xyz] = 0; }
    if ((block_start[xyz] + block_size[xyz]) > grid_size[xyz]) { block_start[xyz] = grid_size[xyz] - block_size[xyz]; }
  }

  return 0;
}

// simple max cone
//
int _patch_choice_wf_xyz2(g_ctx_t &ctx,
                          int32_t *block_start,
                          int32_t *block_ds,
                          int32_t *block_size,
                          int32_t *grid_size) {
  int64_t cell;
  int32_t _ds[3],
          xyz,
          v[3];
  double d, _sum=0.0,
         u = -1.0;

  double  cur_min,
          dx,dy,dz,
          cx,cy,cz;
  int32_t x,y,z;

  double alpha = 0.0, beta=1.0;
  char code=' ';

  POMS *poms;

  poms = ctx.poms;

  alpha = ctx.m_alpha;
  beta  = ctx.m_beta;

  _ds[0] = block_size[0]/2;
  _ds[1] = block_size[1]/2;
  _ds[2] = block_size[2]/2;

  cx = (double)grid_size[0] / 2.0;
  cy = (double)grid_size[1] / 2.0;
  cz = (double)grid_size[2] / 2.0;

  cx -= (double)block_size[0];
  cy = (double)grid_size[1];

  cx = 0;
  cy = 0;
  cz = 0;

  block_start[0] = 0;
  block_start[1] = 0;
  block_start[2] = 0;

  cur_min = -1.0;
  for (cell=0; cell<poms->m_quilt_cell_count; cell++) {

    if (poms->m_quilt_tile[cell] >= 0) { continue; }
    if (poms->m_quilt_pin[cell] != 0) { continue; }
    poms->cell2vec(v, cell, poms->m_quilt_size);

    dx = ((double)v[0]) - cx;
    dy = ((double)v[1]) - cy;
    dz = ((double)v[2]) - cz;

    dx /= (double)grid_size[0];
    dy /= (double)grid_size[1];
    dz /= (double)grid_size[2];

    dx *= beta;
    dy *= beta;
    dz *= beta;

    //u = sqrt( (dx*dx) + (dy*dy) + (dz*dz) );

    u = fabs(pow(pow(dx,alpha) + pow(dy,alpha) + pow(dz,alpha), 1.0/alpha));

    if ((cur_min < 0.0) ||
        (cur_min > u)) {
      cur_min = u;
      block_start[0] = v[0];
      block_start[1] = v[1];
      block_start[2] = v[2];
    }

  }

  for (xyz=0; xyz<3; xyz++) {
    block_start[xyz] -= _ds[xyz];

    block_start[xyz] += poms->irnd(_ds[xyz]/2) - (int32_t)(_ds[xyz]/4);

    if (block_start[xyz] < 0) { block_start[xyz] = 0; }
    if ((block_start[xyz] + block_size[xyz]) > grid_size[xyz]) { block_start[xyz] = grid_size[xyz] - block_size[xyz]; }
  }

  return 0;
}

// simple max cone
//

static void iswp(int32_t *a, int32_t *b) {
  int32_t t;
  t = *a;
  *a = *b;
  *b = t;
}

static void isrt2(int32_t *u3, int32_t *v3) {

  if (u3[0] < u3[1]) {
    iswp(&(u3[0]), &(u3[1]));
    iswp(&(v3[0]), &(v3[1]));
  }

  if (u3[1] < u3[2]) {
    iswp(&(u3[2]), &(u3[1]));
    iswp(&(v3[2]), &(v3[1]));
  }

  if (u3[0] < u3[1]) {
    iswp(&(u3[0]), &(u3[1]));
    iswp(&(v3[0]), &(v3[1]));
  }

}

static double _square_skin_energy(int32_t  x, int32_t  y, int32_t z,
                                  int32_t cx, int32_t cy, int32_t cz,
                                  int32_t n) {
  double d, val, tx, ty, tz, val_a, val_b;

  int32_t ival[3],
          ipval[3],
          t;
  int32_t xp, yp, zp,
          axp, ayp, azp,
          nc;

  nc = n - cx;
  if ( nc < (n - cy) ) { nc = n - cy; }
  if ( nc < (n - cz) ) { nc = n - cz; }

  if ( nc < cx ) { nc = cx; }
  if ( nc < cy ) { nc = cy; }
  if ( nc < cz ) { nc = cz; }

  ival[0] = x-cx;
  ival[1] = y-cy;
  ival[2] = z-cz;

  ipval[0] = ((ival[0]<0) ? (-ival[0]) : ival[0]);
  ipval[1] = ((ival[1]<0) ? (-ival[1]) : ival[1]);
  ipval[2] = ((ival[2]<0) ? (-ival[2]) : ival[2]);

  isrt2(ipval, ival);

  d = ipval[0];

  val_a = ( ((double)ipval[1]) / (d+1.0) );
  val_b = ( ((double)ipval[2]) / ((d+1.0)*(d+1.0)) );

  val = val_a + val_b + d;
  val = ((double)nc) + 1.0 - (val_a + val_b + d);

  return val;
}

int _patch_choice_wf_cube(g_ctx_t &ctx,
                          int32_t *block_start,
                          int32_t *block_ds,
                          int32_t *block_size,
                          int32_t *grid_size) {
  int64_t cell;
  int32_t _ds[3],
          xyz,
          v[3];
  double d, _sum=0.0,
         u = -1.0;

  double  cur_max,
          dx,dy,dz,
          cx,cy,cz;
  int32_t x,y,z, max_nxyz;

  double alpha = 0.0, beta=1.0;
  char code=' ';

  POMS *poms;

  poms = ctx.poms;

  alpha = ctx.m_alpha;
  beta  = ctx.m_beta;

  _ds[0] = block_size[0]/2;
  _ds[1] = block_size[1]/2;
  _ds[2] = block_size[2]/2;

  cx = (double)grid_size[0] / 2.0;
  cy = (double)grid_size[1] / 2.0;
  cz = (double)grid_size[2] / 2.0;

  cx -= (double)block_size[0];
  cy = (double)grid_size[1];

  cx = 0;
  cy = 0;
  cz = 0;

  block_start[0] = 0;
  block_start[1] = 0;
  block_start[2] = 0;

  max_nxyz = grid_size[0];
  if ( max_nxyz < grid_size[1]) { max_nxyz = grid_size[1]; }
  if ( max_nxyz < grid_size[2]) { max_nxyz = grid_size[2]; }

  cur_max = -1.0;
  for (cell=0; cell<poms->m_quilt_cell_count; cell++) {

    if (poms->m_quilt_tile[cell] >= 0) { continue; }
    if (poms->m_quilt_pin[cell] != 0) { continue; }
    poms->cell2vec(v, cell, poms->m_quilt_size);

    dx = ((double)v[0]) - cx;
    dy = ((double)v[1]) - cy;
    dz = ((double)v[2]) - cz;

    dx /= (double)grid_size[0];
    dy /= (double)grid_size[1];
    dz /= (double)grid_size[2];

    dx *= beta;
    dy *= beta;
    dz *= beta;

    u = _square_skin_energy(v[0], v[1], v[2], cx, cy, cz, max_nxyz);

    if ((cur_max < 0.0) ||
        (cur_max < u)) {
      cur_max = u;
      block_start[0] = v[0];
      block_start[1] = v[1];
      block_start[2] = v[2];
    }

  }

  for (xyz=0; xyz<3; xyz++) {
    block_start[xyz] -= _ds[xyz];

    block_start[xyz] += poms->irnd(_ds[xyz]/2) - (int32_t)(_ds[xyz]/4);

    if (block_start[xyz] < 0) { block_start[xyz] = 0; }
    if ((block_start[xyz] + block_size[xyz]) > grid_size[xyz]) { block_start[xyz] = grid_size[xyz] - block_size[xyz]; }
  }

  return 0;
}



// simple max cone
//
int _patch_choice_wf_xy(g_ctx_t &ctx,
                        int32_t *block_start,
                        int32_t *block_ds,
                        int32_t *block_size,
                        int32_t *grid_size) {
  int64_t cell;
  int32_t _ds[3],
          xyz,
          v[3];
  double d, _sum=0.0,
         u = -1.0;

  double  cur_min,
          dx,dy,dz,
          cx,cy,cz;
  int32_t x,y,z;

  char code=' ';

  POMS *poms;

  poms = ctx.poms;

  _ds[0] = block_size[0]/2;
  _ds[1] = block_size[1]/2;
  _ds[2] = block_size[2]/2;

  cx = (double)grid_size[0] / 2.0;
  cy = (double)grid_size[1] / 2.0;
  cz = (double)grid_size[2] / 2.0;

  cx -= (double)block_size[0];
  cy = (double)grid_size[1];

  cx = 0;
  cy = 0;
  cz = 0;

  block_start[0] = 0;
  block_start[1] = 0;
  block_start[2] = 0;

  cur_min = -1.0;
  for (cell=0; cell<poms->m_quilt_cell_count; cell++) {

    if (poms->m_quilt_tile[cell] >= 0) { continue; }
    if (poms->m_quilt_pin[cell] != 0) { continue; }
    poms->cell2vec(v, cell, poms->m_quilt_size);

    dx = ((double)v[0]) - cx;
    dy = ((double)v[1]) - cy;
    dz = ((double)v[2]) - cz;

    //u = sqrt( (dx*dx) + (dy*dy) + (dz*dz) );

    u = fabs(dx + dy + dz);

    if ((cur_min < 0.0) ||
        (cur_min > u)) {
      cur_min = u;
      block_start[0] = v[0];
      block_start[1] = v[1];
      block_start[2] = v[2];
    }

  }

  for (xyz=0; xyz<3; xyz++) {
    block_start[xyz] -= _ds[xyz];

    block_start[xyz] += poms->irnd(_ds[xyz]/2) - (int32_t)(_ds[xyz]/4);

    if (block_start[xyz] < 0) { block_start[xyz] = 0; }
    if ((block_start[xyz] + block_size[xyz]) > grid_size[xyz]) { block_start[xyz] = grid_size[xyz] - block_size[xyz]; }
  }

  return 0;
}

// simple max cone
//
int _patch_choice_wf_xy_n(g_ctx_t &ctx,
                        int32_t *block_start,
                        int32_t *block_ds,
                        int32_t *block_size,
                        int32_t *grid_size) {
  int64_t cell;
  int32_t _ds[3],
          xyz,
          v[3];
  double d, _sum=0.0,
         u = -1.0;

  double  cur_min,
          dx,dy,dz,
          cx,cy,cz;
  int32_t x,y,z;

  char code=' ';

  POMS *poms;

  poms = ctx.poms;

  _ds[0] = block_size[0]/2;
  _ds[1] = block_size[1]/2;
  _ds[2] = block_size[2]/2;

  cx = (double)grid_size[0] - 1;
  cy = (double)grid_size[1] - 1;
  cz = (double)grid_size[2] - 1;

  block_start[0] = 0;
  block_start[1] = 0;
  block_start[2] = 0;

  cur_min = -1.0;
  for (cell=0; cell<poms->m_quilt_cell_count; cell++) {

    if (poms->m_quilt_tile[cell] >= 0) { continue; }
    if (poms->m_quilt_pin[cell] != 0) { continue; }
    poms->cell2vec(v, cell, poms->m_quilt_size);

    dx = ((double)v[0]) - cx;
    dy = ((double)v[1]) - cy;
    dz = ((double)v[2]) - cz;

    //u = sqrt( (dx*dx) + (dy*dy) + (dz*dz) );

    u = fabs(dx + dy + dz);

    if ((cur_min < 0.0) ||
        (cur_min > u)) {
      cur_min = u;
      block_start[0] = v[0];
      block_start[1] = v[1];
      block_start[2] = v[2];
    }

  }

  for (xyz=0; xyz<3; xyz++) {
    block_start[xyz] -= _ds[xyz];

    block_start[xyz] += poms->irnd(_ds[xyz]/2) - (int32_t)(_ds[xyz]/4);

    if (block_start[xyz] < 0) { block_start[xyz] = 0; }
    if ((block_start[xyz] + block_size[xyz]) > grid_size[xyz]) { block_start[xyz] = grid_size[xyz] - block_size[xyz]; }
  }

  return 0;
}

int _patch_choice_weight_pending( POMS &poms,
                                  int32_t *block_start,
                                  int32_t *block_ds,
                                  int32_t *block_size,
                                  int32_t *grid_size) {
  int64_t cell;
  double count=0.0, p;
  int32_t _ds[3], xyz;
  double d;

  _ds[0] = block_size[0]/2;
  _ds[1] = block_size[1]/2;
  _ds[2] = block_size[2]/2;

  _patch_choice_rand(block_start, block_ds, block_size, grid_size);

  for (cell=0; cell<poms.m_quilt_cell_count; cell++) {
    if (poms.m_quilt_tile[cell] >= 0) { continue; }
    if (poms.m_quilt_pin[cell]  != 0) { continue; }

    count+=1.0;

    p = poms.rnd();
    if (p < (1.0 / count)) {
      poms.cell2vec(block_start, cell, poms.m_quilt_size);
    }
  }

  if (count > 0) {
    for (xyz=0; xyz<3; xyz++) {
      d = poms.noisePowerLaw(1.0, -2.0);
      if (poms.rnd() < 0.5) { d = -d; }
      block_start[xyz] -= _ds[xyz];
      block_start[xyz] += (int32_t)d;
    }

    //block_start[0] -= (int32_t)(poms.rnd()*(double)_ds[0]);
    //block_start[1] -= (int32_t)(poms.rnd()*(double)_ds[1]);
    //block_start[2] -= (int32_t)(poms.rnd()*(double)_ds[2]);
  }


  if (block_start[0] < 0) { block_start[0] = 0; }
  if (block_start[1] < 0) { block_start[1] = 0; }
  if (block_start[2] < 0) { block_start[2] = 0; }

  if ((block_start[0] + block_size[0]) > grid_size[0]) { block_start[0] = grid_size[0] - block_size[0]; }
  if ((block_start[1] + block_size[1]) > grid_size[1]) { block_start[1] = grid_size[1] - block_size[1]; }
  if ((block_start[2] + block_size[2]) > grid_size[2]) { block_start[2] = grid_size[2] - block_size[2]; }

  return 0;
}

int _patch_choice_xpyp(int32_t *block_start, int32_t *block_ds, int32_t *block_size, int32_t *grid_size) {

  if ((block_start[0] + block_size[0]) >= grid_size[0]) {

    block_start[0] = 0;
    if ((block_start[1] + block_size[1]) >= grid_size[1]) {

      block_start[1] = 0;
      if ((block_start[2] + block_size[2]) >= grid_size[2]) {
        block_start[2] = 0;
      }
      else {
        block_start[2] += block_ds[2];
        if ((block_start[2] + block_size[2]) >= grid_size[2]) {
          block_start[2] = grid_size[2] - block_size[2];
        }
      }

    }
    else {

      block_start[1] += block_ds[1];
      if ((block_start[1] + block_size[1]) >= grid_size[1]) {
        block_start[1] = grid_size[1] - block_size[1];
      }
    }

  }
  else {

    block_start[0] += block_ds[0];
    if ((block_start[0] + block_size[0]) >= grid_size[0]) {
      block_start[0] = grid_size[0] - block_size[0];
    }

  }

  return 0;
}

int _patch_choice_xnyp(int32_t *block_start, int32_t *block_ds, int32_t *block_size, int32_t *grid_size) {

  //if ((block_start[0] + block_size[0]) >= grid_size[0]) {
  if (block_start[0] == 0) {

    block_start[0] = grid_size[0] - block_size[0];

    if ((block_start[1] + block_size[1]) >= grid_size[1]) {

      block_start[1] = 0;
      if ((block_start[2] + block_size[2]) >= grid_size[2]) {
        block_start[2] = 0;
      }
      else {
        block_start[2] += block_ds[2];
        if ((block_start[2] + block_size[2]) >= grid_size[2]) {
          block_start[2] = grid_size[2] - block_size[2];
        }
      }

    }
    else {

      block_start[1] += block_ds[1];
      if ((block_start[1] + block_size[1]) >= grid_size[1]) {
        block_start[1] = grid_size[1] - block_size[1];
      }
    }

  }
  else {

    block_start[0] -= block_ds[0];
    //if ((block_start[0] + block_size[0]) >= grid_size[0]) {
    if (block_start[0] < 0) {
      block_start[0] = 0;
    }

  }

  return 0;
}

int _patch_choice_xnyn(int32_t *block_start, int32_t *block_ds, int32_t *block_size, int32_t *grid_size) {

  if (block_start[0] == 0) {
    block_start[0] = grid_size[0] - block_size[0];

    if (block_start[1] == 0) {
      block_start[1] = grid_size[1] - block_size[1];

      if ((block_start[2] + block_size[2]) >= grid_size[2]) {
        block_start[2] = 0;
      }
      else {
        block_start[2] += block_ds[2];
        if ((block_start[2] + block_size[2]) >= grid_size[2]) {
          block_start[2] = grid_size[2] - block_size[2];
        }
      }

    }
    else {

      block_start[1] -= block_ds[1];
      if (block_start[1] < 0) { block_start[1] = 0; }
    }

  }
  else {

    block_start[0] -= block_ds[0];
    if (block_start[0] < 0) { block_start[0] = 0; }

  }

  return 0;
}

int _export_failed_run(POMS &poms, const char *fn) {
  FILE *fp;
  int32_t idir, n;
  int32_t i, j, first=1;

  int64_t cell, qcell;
  int32_t p[3],  q[3];

  fp = fopen(fn, "w");
  if (!fp ) { return -1; }



  fprintf(fp, "{\n");
  fprintf(fp, "  \"rule\":[\n");

  for (idir=0; idir<6; idir++) {
    for (i=0; i<poms.m_tile_count; i++) {
      n = poms.tileAdjIdxSize(i, idir);
      for (j=0; j<n; j++) {
        if (!first) { fprintf(fp, ",\n"); }
        else { fprintf(fp, "\n"); }
        first = 0;
        fprintf(fp, "[%i,%i,%i,1]", i, poms.tileAdjIdx(i, idir, j), idir);
      }
    }
  }
  fprintf(fp, "  ],\n");

  fprintf(fp, "  \"weight\":[\n");
  first = 1;
  for (i=0; i<poms.m_tile_count; i++) {
    if (!first) { fprintf(fp, ",\n"); }
    else { fprintf(fp, "\n"); }
    first = 0;
    fprintf(fp, "1");
  }
  fprintf(fp, "  ],\n");

  fprintf(fp, "  \"name\":[\n");
  first = 1;
  for (i=0; i<poms.m_tile_count; i++) {
    if (!first) { fprintf(fp, ",\n"); }
    else { fprintf(fp, "\n"); }
    first = 0;
    fprintf(fp, "\"%i\"", i);
  }
  fprintf(fp, "  ],\n");

  fprintf(fp, "  \"tileset\":{\n");
  fprintf(fp, "    \"image\": \"%s\",\n", poms.m_tileset_ctx.image.c_str());
  fprintf(fp, "    \"tilecount\": \"%i\",\n", poms.m_tileset_ctx.tilecount);
  fprintf(fp, "    \"imageheight\": \"%i\",\n", poms.m_tileset_ctx.imageheight);
  fprintf(fp, "    \"imagewidth\": \"%i\",\n", poms.m_tileset_ctx.imagewidth);
  fprintf(fp, "    \"tileheight\": \"%i\",\n", poms.m_tileset_ctx.tileheight);
  fprintf(fp, "    \"tilewidth\": \"%i\"", poms.m_tileset_ctx.tilewidth);
  fprintf(fp, "  },\n");

  fprintf(fp, "  \"boundaryConditions\":{\n");
  fprintf(fp, "    \"x+\":{\"type\":\"tile\",\"value\":0},\n");
  fprintf(fp, "    \"x-\":{\"type\":\"tile\",\"value\":0},\n");
  fprintf(fp, "    \"y+\":{\"type\":\"tile\",\"value\":0},\n");
  fprintf(fp, "    \"y-\":{\"type\":\"tile\",\"value\":0},\n");
  fprintf(fp, "    \"z+\":{\"type\":\"tile\",\"value\":0},\n");
  fprintf(fp, "    \"z-\":{\"type\":\"tile\",\"value\":0}\n");
  fprintf(fp, "  },\n");

  fprintf(fp, "  \"constraint\":[\n");
  for (cell=0; cell<poms.m_cell_count; cell++) {
    poms.cell2vec(p, cell);
    q[0] = p[0] + poms.m_patch_region[0][0];
    q[1] = p[1] + poms.m_patch_region[1][0];
    q[2] = p[2] + poms.m_patch_region[2][0];
    qcell = poms.vec2cell(q, poms.m_quilt_size);

    if (poms.m_quilt_tile[qcell] >= 0) {
      fprintf(fp, "    {\"type\":\"force\", \"range\":{\"tile\":[%i,%i], \"x\":[%i,%i], \"y\":[%i,%i], \"z\":[%i,%i] } },\n",
          poms.m_quilt_tile[qcell], poms.m_quilt_tile[qcell]+1,
          p[0], p[0]+1,
          p[1], p[1]+1,
          p[2], p[2]+1);
    }

    if (poms.m_cell_pin[cell] != 0) {
      fprintf(fp, "    {\"type\":\"pin\", \"range\":{\"tile\":[%i,%i], \"x\":[%i,%i], \"y\":[%i,%i], \"z\":[%i,%i] } },\n",
          -1, -1,
          p[0], p[0]+1,
          p[1], p[1]+1,
          p[2], p[2]+1);
    }


  }
  fprintf(fp, "    {\"type\":\"remove\", \"range\":{\"tile\":[0,1],\"x\":[], \"y\":[], \"z\":[]} }\n");
  fprintf(fp, "  ],\n");

  fprintf(fp, "  \"size\":[%i,%i,%i]\n", poms.m_size[0], poms.m_size[1], poms.m_size[2]);
  fprintf(fp, "}\n");

  fclose(fp);

  return 0;
}

//---


int _update_conflict_grid(g_ctx_t &ctx, int32_t *block_s, int32_t *block_ds, int code) {
  int32_t x,y,z;
  int64_t cell;
  POMS *poms;

  poms = ctx.poms;

  for (z=block_s[2]; z < (block_s[2]+block_ds[2]); z++) {
    for (y=block_s[1]; y < (block_s[1]+block_ds[1]); y++) {
      for (x=block_s[0]; x < (block_s[0]+block_ds[0]); x++) {

        cell = poms->xyz2cell(x,y,z,poms->m_quilt_size);
        if (cell < 0) { continue; }

        if (code==0) {
          ctx.m_conflict_grid[cell] = 0;
        }
        else {
          ctx.m_conflict_grid[cell]++;
        }

      }
    }
  }


  return 0;
}

int _erode_quilt(g_ctx_t &ctx, double p=1.0) {
  int64_t cell, nei_cell, idx;
  int32_t idir;
  POMS *poms;

  std::vector< int64_t > _queue;
  std::vector< int8_t > _visited;

  poms = ctx.poms;

  _visited.resize( poms->m_quilt_cell_count, 0 );

  for (cell=0; cell<poms->m_quilt_cell_count; cell++) {
    if (poms->m_quilt_tile[cell] >= 0) { continue; }
    if (poms->m_quilt_pin[cell] != 0) { continue; }

    for (idir=0; idir<6; idir++) {
      nei_cell = poms->neiCell(cell, idir, poms->m_quilt_size);
      if (nei_cell<0) { continue; }
      if (_visited[nei_cell]) { continue; }

      _visited[nei_cell] = 1;

      if (poms->rnd() < p) {
        _queue.push_back(nei_cell);
      }
    }

  }

  for (idx=0; idx<_queue.size(); idx++) {
    poms->m_quilt_tile[ _queue[idx] ] = -1;
  }

  return 0;
}

int _erode_quilt_region(g_ctx_t &ctx, int32_t *region, double p=1.0) {
  int64_t cell, nei_cell, idx;
  int32_t x,y,z, v[3];
  int32_t idir;
  POMS *poms;

  std::vector< int64_t > _queue;
  std::vector< int8_t > _visited;

  poms = ctx.poms;

  _visited.resize( poms->m_quilt_cell_count, 0 );

  for (cell=0; cell<poms->m_quilt_cell_count; cell++) {
    if (poms->m_quilt_tile[cell] >= 0) { continue; }
    if (poms->m_quilt_pin[cell] != 0) { continue; }

    for (idir=0; idir<6; idir++) {
      nei_cell = poms->neiCell(cell, idir, poms->m_quilt_size);
      if (nei_cell<0) { continue; }
      if (_visited[nei_cell]) { continue; }

      poms->cell2vec(v, nei_cell, poms->m_quilt_size);
      if ( (v[0] < region[0]) || (v[0] >= region[1]) ||
           (v[1] < region[2]) || (v[1] >= region[3]) ||
           (v[2] < region[4]) || (v[2] >= region[5])) {
        continue;
      }

      _visited[nei_cell] = 1;

      if (poms->rnd() < p) {
        _queue.push_back(nei_cell);
      }
    }

  }

  for (idx=0; idx<_queue.size(); idx++) {
    poms->m_quilt_tile[ _queue[idx] ] = -1;
  }

  return 0;
}

int _remove_quilt_region(g_ctx_t &ctx, int32_t *region, double p=1.0) {
  int64_t cell, idx;
  int32_t x,y,z, v[3];
  int32_t idir;
  POMS *poms;

  std::vector< int64_t > _queue;
  std::vector< int8_t > _visited;

  poms = ctx.poms;

  _visited.resize( poms->m_quilt_cell_count, 0 );

  for (cell=0; cell<poms->m_quilt_cell_count; cell++) {
    if (poms->m_quilt_tile[cell] < 0) { continue; }
    if (poms->m_quilt_pin[cell] != 0) { continue; }

    poms->cell2vec(v, cell, poms->m_quilt_size);
    if ( (v[0] < region[0]) || (v[0] >= region[1]) ||
         (v[1] < region[2]) || (v[1] >= region[3]) ||
         (v[2] < region[4]) || (v[2] >= region[5])) {
      continue;
    }


    if (poms->rnd() < p) {
      _queue.push_back(cell);
    }

  }

  for (idx=0; idx<_queue.size(); idx++) {
    poms->m_quilt_tile[ _queue[idx] ] = -1;
  }

  return 0;
}

//---

void boundary_spot_check(POMS &poms) {
  int32_t x, y, z;
  int64_t cell;
      //DEBUG
      //DEBUG
      printf("spot check patch bounds\n");
      printf("cellsize[:,0]:");
      for (x=0; x<poms.m_size[0]; x++) {
        cell = poms.xyz2cell(x,0,0);
        printf(" %i", (int)poms.cellSize(poms.m_plane, cell));
      }
      printf("\n");

      printf("cellsize[:,%i]:", poms.m_size[1]-1);
      for (x=0; x<poms.m_size[0]; x++) {
        cell = poms.xyz2cell(x,poms.m_size[1]-1,0);
        printf(" %i", (int)poms.cellSize(poms.m_plane, cell));
      }
      printf("\n");

      printf("cellsize[0,:]:");
      for (y=0; y<poms.m_size[1]; y++) {
        cell = poms.xyz2cell(0,y,0);
        printf(" %i", (int)poms.cellSize(poms.m_plane, cell));
      }
      printf("\n");

      printf("cellsize[%i,:]:", poms.m_size[0]-1);
      for (y=0; y<poms.m_size[1]; y++) {
        cell = poms.xyz2cell(poms.m_size[0]-1,y,0);
        printf(" %i", (int)poms.cellSize(poms.m_plane, cell));
      }
      printf("\n");

      //DEBUG
      //DEBUG

}

//---
//---


int init_noise(POMS &poms, _opt_t &opt) {
  int32_t tile, group;
  int32_t max_tile_group=0, n_tile_group=1;

  for (tile=0; tile < poms.m_tile_count; tile++) {
    if (poms.m_tile_group[tile] > max_tile_group) {
      max_tile_group = poms.m_tile_group[tile];
    }
  }
  n_tile_group = max_tile_group+1;

  g_ctx.noise.clear();
  g_ctx.noise_min.clear();
  g_ctx.noise_max.clear();
  g_ctx.noise_threshold.clear();
  g_ctx.noise_type.clear();

  g_ctx.noise.resize(n_tile_group);
  g_ctx.noise_min.resize(n_tile_group);
  g_ctx.noise_max.resize(n_tile_group);
  g_ctx.noise_threshold.resize(n_tile_group);
  g_ctx.noise_type.resize(n_tile_group);
  for (group=0; group < n_tile_group; group++) {
    g_ctx.noise[group]            = fnlCreateState();
    g_ctx.noise[group].noise_type = FNL_NOISE_OPENSIMPLEX2;
    g_ctx.noise[group].frequency  = opt.noise_freq;
    g_ctx.noise[group].seed       = opt.noise_seed+group;
  }

  if (opt.noise_preset == NOISE_PRESET_TYPE_SIMPLE) {
    if (n_tile_group != 2) { return -1; }

    g_ctx.noise_min[0] = 1.0;
    g_ctx.noise_max[0] = 200.0;
    g_ctx.noise_threshold[0] = 1000.0;
    g_ctx.noise_type[0] = NOISE_TYPE_TIERED;

    g_ctx.noise_min[1] = 0;
    g_ctx.noise_max[1] = 0;
    g_ctx.noise_threshold[1] = 0;
    g_ctx.noise_type[1] = NOISE_TYPE_NONE;

    return 0;
  }

  else if (opt.noise_preset == NOISE_PRESET_TYPE_VECTOR) {
    for (group=0; group < n_tile_group; group++) {
      //g_ctx.noise_min[group] = 1.0;
      //g_ctx.noise_max[group] = 9.0;
      g_ctx.noise_min[group] = 1.0/16.0;
      g_ctx.noise_max[group] = 1.0;
      //g_ctx.noise_threshold[group] = 20.0;
      g_ctx.noise_threshold[group] = -1;
      g_ctx.noise_type[group] = NOISE_TYPE_LINEAR;
    }

    return 0;
  }

  return -1;
}

void print_noise(POMS &poms, g_ctx_t &ctx) {
  int32_t i, j, k,
          x,y,z;
  int32_t tile;
  int32_t group=0, n_group=-1;
  int64_t cell;
  std::vector< int32_t > tile_rep;
  std::vector< double > tile_rep_weight;

  for (tile=0; tile<poms.m_tile_count; tile++) {
    if (n_group < poms.m_tile_group[tile]) {
      n_group = poms.m_tile_group[tile];
    }
  }
  n_group++;

  tile_rep.resize(n_group, -1);
  tile_rep_weight.resize(n_group, 0.0);

  // find representative tile and make sure it has
  // maximum weight within the group.
  //
  for (tile=0; tile<poms.m_tile_count; tile++) {
    group = poms.m_tile_group[tile];
    if (tile_rep[group] < 0) {
      tile_rep[group] = tile;
    }
    if (poms.G(tile) > tile_rep_weight[group]) {
      tile_rep_weight[group] = poms.G(tile);
      tile_rep[group] = tile;
    }
  }

  for (group=0; group < n_group; group++) {

    printf("# tile_rep[group:%i]:%i (G_orig(%i):%f, quilt_size:[%i,%i,%i])\n",
        (int)group, (int)tile_rep[group],
        (int)tile,
        poms.G(tile_rep[group]),
        (int)poms.m_quilt_size[0],
        (int)poms.m_quilt_size[1],
        (int)poms.m_quilt_size[2]);

    if (poms.m_quilt_size[2] == 1) {
      for (x=0; x<poms.m_quilt_size[0]; x++) {
        for (y=0; y<poms.m_quilt_size[1]; y++) {

          cell = poms.xyz2cell(x,y,0, poms.m_quilt_size);

          printf("%i %i %f\n",
              (int)x, (int)y, poms.m_g_cb( &poms, cell, tile_rep[group] ) );

        }
        printf("\n");
      }
      printf("\n");
    }

    else {
      for (x=0; x<poms.m_quilt_size[0]; x++) {
        for (y=0; y<poms.m_quilt_size[1]; y++) {
          for (z=0; z<poms.m_quilt_size[2]; z++) {

            cell = poms.xyz2cell(x,y,z, poms.m_quilt_size);
            printf("%i %i %i %f\n",
                (int)x, (int)y, (int)z, poms.m_g_cb( &poms, cell, tile_rep[group] ) );
          }
          printf("\n");
        }
      }

    }

  }

}

double custom_G(POMS *poms, int64_t cell, int32_t tile) {
  int32_t xyz[3], tile_group;
  double v, R, M;
  double _min = 1/64.0,
         _max = 1.0-(1/64.0);


  //_min = 1;
  //_max = 200.0;

  //M = 1000.0;

  tile_group = poms->m_tile_group[tile];

  if (g_ctx.noise_type[tile_group] == NOISE_TYPE_NONE) {
    return poms->G(tile);
  }

  _min = g_ctx.noise_min[tile_group];
  _max = g_ctx.noise_max[tile_group];

  //M = g_ctx.noise_max[tile_group];


  //if (tile != 0) { return poms->G(tile); }
  //_max = poms->G(0);

  poms->cell2vec(xyz, cell, poms->m_quilt_size);
  v = fnlGetNoise3D(&(g_ctx.noise[tile_group]), xyz[0], xyz[1], xyz[2]);
  R = ((_max - _min)*(v+1.0)/2.0) + _min;

  //printf("#### tile:%i, xyz[%i,%i,%i]{%i} v:%f\n",
  //    (int)tile, (int)xyz[0], (int)xyz[1], (int)xyz[2],
  //    (int)cell, v);


  //if (tile == 0) { return poms->G(0) * R; }

  //if (tile==0) {

  switch (g_ctx.noise_type[tile_group]) {
    case NOISE_TYPE_NONE:
      return poms->G(tile);
      break;
    case NOISE_TYPE_LINEAR:
      return poms->G(tile)*R;
      break;
    case NOISE_TYPE_THRESHOLD:
      return ((v < _min) ? (poms->G(tile)*_max) : (poms->G(tile)*_min));
      break;
    case NOISE_TYPE_TIERED:
      return ((v > 0) ? (poms->G(tile)*R) : (poms->G(tile)*g_ctx.noise_threshold[tile_group]));
      break;
    default:
      //return poms->G(tile);
      break;
  }

  /*
  if (g_ctx.noise_type[tile_group] == NOISE_TYPE_LINEAR) {
    return poms->G(tile)*R;
  }

  else if (g_ctx.noise_type[tile_group] == NOISE_TYPE_THRESHOLD) {
    if (v < _min) { return poms->G(tile)*_max; }
    return poms->G(tile)*_min;
  }

  else if (g_ctx.noise_type[tile_group] == NOISE_TYPE_TIERED) {
    if (v < 0) { return poms->G(tile)*R; }
    //return poms->G(tile)*M;
    return poms->G(tile)*_max;
  }
  */

  return poms->G(tile);
  //return -1;
}

//---
//---

/*
double window_alpha(_opt_t &opt) {
  int32_t i, idx, fail_count;
  double R=1.0;

  fail_count = 0;
  for (i=0; i<opt.window_fail_n; i++) {
    idx = (opt.window_fail_s + i) % opt.window_fail.size();

    if (opt.window_fail[idx] > 0) { fail_count++; }
  }

  if (fail_count < opt.window_fail_threshold[0]) {
    return 0.0;
  }

  if (fail_count >= opt.window_fail_threshold[1]) {
    return 1.0;
  }

  fail_count -= opt.window_fail_threshold[0];

  R = (double)(opt.window_fail_threshold[1] - opt.window_fail_threshold[0]);

  return ((double)fail_count) / R;
}
*/

///----
// VIZUALIZATION begin
///----
//
//  _   __________
// | | / /  _/_  /
// | |/ // /  / /_
// |___/___/ /___/
//

static int _update_viz_step( int64_t bms_step, _opt_t &opt, POMS &poms, g_ctx_t &ctx ) {
  int _r;

  if ((opt.viz_step>0) &&
      ((bms_step%opt.viz_step)==0)) {
    if (opt.gnuplot_fn.size() > 0) { viz_gnuplot_cellfreq_4d(poms, opt); }
    if (opt.tiled_slideshow_dir.size() > 0) {
      tiled_slideshow(ctx, opt, ctx.m_slideshow_id);
      ctx.m_slideshow_id++;
    }
    if (ctx.tiled_snapshot_fn.size() > 0) {
      _r = rt_tiled_snapshot(&ctx);
      if (_r<0) { printf("# failed to save snapshot, got (%i)\n", _r); }
      if (ctx.global_callback) { ctx.global_callback(); }
    }
    if (ctx.patch_snapshot_fn.size() > 0) {
      _r = rt_patch_snapshot(&ctx,1);
      if (_r<0) { printf("# failed to save patch snapshot, got (%i)\n", _r); }
      if (ctx.global_callback) { ctx.global_callback(); }
    }
  }

  return 0;
}


static int _update_viz_init( _opt_t &opt, g_ctx_t &ctx ) {
  int _r;

  if (opt.viz_step>0) {
    if (ctx.patch_snapshot_fn.size() > 0) {
      _r = rt_patch_snapshot(&ctx,1);
      if (_r<0) { printf("# failed to save patch snapshot, got (%i)\n", _r); }

      if (ctx.global_callback) { ctx.global_callback(); }
    }
  }

  return 0;
}


static int _update_viz_begin( int64_t it, _opt_t &opt, POMS &poms, g_ctx_t &ctx ) {
  int _r;

  if ((opt.viz_step>0) &&
      ((it%opt.viz_step)==0)) {
    if (ctx.patch_snapshot_fn.size() > 0) {
      _r = rt_patch_snapshot(&ctx,1);
      if (_r<0) { printf("# failed to save patch snapshot, got (%i)\n", _r); }

      if (ctx.global_callback) { ctx.global_callback(); }
    }
  }

  return 0;
}


static int _update_viz_snapshots( _opt_t &opt, POMS &poms, g_ctx_t &ctx ) {
  int _r;

  if (ctx.tiled_snapshot_fn.size() > 0) {
    _r = rt_tiled_snapshot(&ctx);
    if (_r<0) { printf("# failed to save snapshot, got (%i)\n", _r); }
    if (ctx.global_callback) { ctx.global_callback(); }
  }
  if (ctx.patch_snapshot_fn.size() > 0) {
    _r = rt_patch_snapshot(&ctx,1);
    if (_r<0) { printf("# failed to save patch snapshot, got (%i)\n", _r); }
    if (ctx.global_callback) { ctx.global_callback(); }
  }

  return 0;
}


static int _update_viz_stl_snapshot( _opt_t &opt, POMS &poms ) {
  // need to load obj
  //
  if (opt.stl_snapshot_fn.size() > 0) {

    if (poms.m_objMap.size() > 0) {
      if (poms.m_verbose >= POMS_VERBOSE_RUN) {
        printf("# exporting to STL snapshot file '%s'\n", opt.stl_snapshot_fn.c_str());
        fflush(stdout);
      }

      exportQuiltSTL(poms, opt.stl_snapshot_fn);
    }
    else {
      if (poms.m_verbose >= POMS_VERBOSE_WARNING) {
        printf("WARNING: snapshot STL '%s' file specified but no objMap detected, not writing\n", opt.stl_snapshot_fn.c_str());
      }
    }

  }

  return 0;
}


static int _update_viz_stl( _opt_t &opt, POMS &poms ) {
  // need to load obj
  //
  if (opt.stl_fn.size() > 0) {
    if (poms.m_objMap.size() > 0) {
      if (poms.m_verbose >= POMS_VERBOSE_DEBUG) {
        printf("# exporting to STL file '%s'\n", opt.stl_fn.c_str());
      }
      exportQuiltSTL(poms, opt.stl_fn);
    }
    else {
      if (poms.m_verbose >= POMS_VERBOSE_WARNING) {
        printf("WARNING: STL '%s' file specified but no objMap detected, not writing\n", opt.stl_fn.c_str());
      }
    }

  }

  return 0;
}


static int _update_viz_patch_snapshot( g_ctx_t &ctx ) {
  int _r;
  if (ctx.patch_snapshot_fn.size() > 0) {
    _r = rt_patch_snapshot(&ctx,0);
    if (_r<0) { printf("# failed to save final patch, got (%i)\n", _r); }

    if (ctx.global_callback) { ctx.global_callback(); }
  }
  return 0;
}


static int _update_viz_tiled_fin( _opt_t &opt, POMS &poms ) {
  if (opt.tiled_fn.size() > 0) {
    if (poms.m_verbose >= POMS_VERBOSE_DEBUG) {
      printf("# exporting final Tiled JSON file '%s'\n", opt.tiled_fn.c_str());
    }
    poms.exportTiledJSON( opt.tiled_fn, opt.tiled_fmt_type, 1 );
  }
  return 0;
}


static int _update_viz_tiled_intermediate( _opt_t &opt, POMS &poms ) {
  if (opt.tiled_fn.size() > 0) {
    if (poms.m_verbose >= POMS_VERBOSE_DEBUG) {
      printf("# exporting intermediate Tiled JSON file '%s'\n", opt.tiled_fn.c_str());
    }
    poms.exportTiledJSON( opt.tiled_fn, opt.tiled_fmt_type, 1 );
  }
  return 0;
}


static int _update_viz_block_snapshot( _opt_t &opt, POMS &poms, int64_t quilt_step ) {
  char tmp_fn[128];
  if (opt.poms_block_snapshot_fn.size() > 0) {
    snprintf(tmp_fn, 127, "%s.it%i", opt.poms_block_snapshot_fn.c_str(), (int)quilt_step);
    if (poms.m_verbose >= POMS_VERBOSE_RUN) {
      printf("## exporting block poms '%s'\n", tmp_fn);
    }
    poms.exportPOMSBlock(tmp_fn);
  }
  return 0;
}


//  _   __________
// | | / /  _/_  /
// | |/ // /  / /_
// |___/___/ /___/
//
///----
// VIZUALIZATION begin
///----

//-----
// verbose print begin
//-----
//  _   _________  ___  ____  ________
// | | / / __/ _ \/ _ )/ __ \/ __/ __/
// | |/ / _// , _/ _  / /_/ /\ \/ _/
// |___/___/_/|_/____/\____/___/___/
//

static int _verbose_block_solver_start( POMS &poms, int64_t n_it, int64_t max_bms_step ) {
  if (poms.m_verbose >= POMS_VERBOSE_RUN) {
    printf("# bms, n_it: %i, max_bms_step:%i\n", (int)n_it, (int)max_bms_step);
  }
  return 0;
}


static int _verbose_block_solver_iter_beg( POMS &poms, int64_t it, int64_t n_it ) {
  if (poms.m_verbose >= POMS_VERBOSE_STEP) {
    printf("# bms, %i/%i, soften[%i,%i,%i]\n", (int)it, (int)n_it,
        (int)poms.m_soften_size[0], (int)poms.m_soften_size[1], (int)poms.m_soften_size[2]);
  }
  return 0;
}


static int _verbose_block_solver_iter_end( POMS &poms, int64_t it, int64_t n_it ) {
  if (poms.m_verbose >= POMS_VERBOSE_STEP) {
    printf("# it:%i/%i, m_state:%s(%i) (E:%i)\n",
        (int)it, (int)n_it,
        poms.stateDescr(poms.m_state), (int)poms.m_state,
        (int)(poms.m_cell_count - poms.resolvedCount()) );
  }
  return 0;
}


static int _verbose_setup_quilt_patch_fail( POMS &poms, int fail_counter, int fail_counter_reset ) {
  if (poms.m_verbose >= POMS_VERBOSE_RUN) {
    printf("# main: setupQuiltPatch failed, discarding region [%i:%i][%i:%i][%i:%i] (fail_counter:%i/%i)\n",
        poms.m_patch_region[0][0], poms.m_patch_region[0][1],
        poms.m_patch_region[1][0], poms.m_patch_region[1][1],
        poms.m_patch_region[2][0], poms.m_patch_region[2][1],
        (int)fail_counter, (int)fail_counter_reset);
  }
  return 0;
}


static int _verbose_fin( POMS &poms, int quilting, int64_t quilt_step ) {
  if (poms.m_verbose >= POMS_VERBOSE_ITER) {
    printf("## FIN:%i quilt_step:%i quilt_cells_resolved:%i/%i\n",
        (int)quilting,
        (int)quilt_step,
        (int)poms.quiltResolvedCount(), (int)poms.m_quilt_cell_count);
  }
  return 0;
}


static int _verbose_end_step( POMS &poms, int ret ) {
  int64_t _count=0;
  if (poms.m_verbose >= POMS_VERBOSE_RUN) {
    if (ret==-3) {
      printf("## SANITY? cell:%i, tile:%i, idir:%i, type: %i\n",
          (int)poms.m_conflict_cell, (int)poms.m_conflict_tile, (int)poms.m_conflict_idir,
          (int)poms.m_conflict_type);
      printf("# EXPORTING DEBUG PATCH...\n");
      _debug_export_patch(poms);
      _debug_export_quilt(poms);
    }
    _count = poms.quiltResolvedCount();
    printf("# got :%i, quilt cells resolved %i/%i(%f)\n", ret,
        (int)_count, (int)poms.m_quilt_cell_count,
        (double)_count / (double)poms.m_quilt_cell_count);
  }

  if (poms.m_verbose >= POMS_VERBOSE_DEBUG) {
    poms.printDebug();
    printf("------\n");
  }
  return ret;
}


static int _verbose_quilt_save_begin( POMS &poms ) {
  if (poms.m_verbose >= POMS_VERBOSE_DEBUG) {
    poms.printDebugGrid();
  }

  if (poms.m_verbose >= POMS_VERBOSE_STEP) {
    printf("## before save quilt: arc sanity: %i, ac4 consistency: %i, sanity quilt: %i\n",
        poms.sanityArcConsistency(),
        poms.AC4Consistency(),
        poms.sanityQuilt());
  }
  return 0;
}


static int _verbose_quilt_save_end( POMS &poms, int ret, int32_t *print_order ) {
  int64_t _count=0;
  if (poms.m_verbose >= POMS_VERBOSE_STEP) {
    printf("## saving quilt patch [%i;%i][%i:%i][%i:%i]\n",
          poms.m_patch_region[0][0], poms.m_patch_region[0][1],
          poms.m_patch_region[1][0], poms.m_patch_region[1][1],
          poms.m_patch_region[2][0], poms.m_patch_region[2][1]);
  }

  if (poms.m_verbose >= POMS_VERBOSE_RUN) {
    poms.printDebugQuiltGrid(print_order);
    _count = poms.quiltResolvedCount();
    printf("# got :%i, quilt cells resolved %i/%i(%f)\n", ret,
        (int)_count, (int)poms.m_quilt_cell_count,
        (double)_count / (double)poms.m_quilt_cell_count);
  }
  return 0;
}


static int _verbose_erode_begin( POMS &poms ) {
  if (poms.m_verbose >= POMS_VERBOSE_RUN) {
    printf("## REJECT quilt patch [%i;%i][%i:%i][%i:%i]\n",
          poms.m_patch_region[0][0], poms.m_patch_region[0][1],
          poms.m_patch_region[1][0], poms.m_patch_region[1][1],
          poms.m_patch_region[2][0], poms.m_patch_region[2][1]);
  }
  return 0;
}


static int _verbose_eroding( POMS &poms, double erode_p, double erode_p_s, double erode_p_e ) {
  if (poms.m_verbose >= POMS_VERBOSE_RUN) {
    printf("# ERODING (erode_p:%f {%f,%f})\n", erode_p, erode_p_s, erode_p_e);
  }
  return 0;
}


static int _verbose_quilt_step_start( POMS &poms, _opt_t &opt,
                                      int64_t quilt_step,
                                      double erode_p, double erode_p_s, double erode_p_e,
                                      int fail_counter, int fail_counter_reset ) {
  if (poms.m_verbose >= POMS_VERBOSE_RUN) {
    printf("#######################\n");
    printf("# quilt step %i (%i/%i), patch [%i:%i,%i:%i,%i:%i] (block-choice:%s, patch-choice:%s) (erode_p:%f{%f:%f},fail_counter:%i/%i) "
        "(tile-support-option:%i) (sanity:%i)\n",
        (int)quilt_step, (int)poms.quiltResolvedCount(), (int)poms.m_quilt_cell_count,
        poms.m_patch_region[0][0], poms.m_patch_region[0][1],
        poms.m_patch_region[1][0], poms.m_patch_region[1][1],
        poms.m_patch_region[2][0], poms.m_patch_region[2][1],
        opt.block_choice_policy_str.c_str(),
        opt.patch_choice_policy_str.c_str(),
        erode_p, erode_p_s, erode_p_e,
        (int)fail_counter, (int)fail_counter_reset,
        poms.m_tile_support_option,
        poms.sanityQuilt() );
        //_block_choice_descr[ quilt_step%4 ].c_str());
    printf("# bms: tile_choice_policy: %i, block_choice_policy: %i, block:[%i,%i,%i], soften:[%i,%i,%i]\n",
        (int)poms.m_tile_choice_policy,
        (int)poms.m_block_choice_policy,
        (int)poms.m_block_size[0], (int)poms.m_block_size[1], (int)poms.m_block_size[2],
        (int)poms.m_soften_size[0], (int)poms.m_soften_size[1], (int)poms.m_soften_size[2]);
    printf("# block_choice_policy: %i\n", poms.m_block_choice_policy);
    printf("# distance_modifier_opt: %i\n", poms.m_distance_modifier_opt);

    printf("\n\n");
    //fflush(stdout);
  }
  return 0;
}


static int _verbose_quilt_step( POMS &poms, int ret, int64_t quilt_step, int fail_counter, int ac4init_fail_indicator, int erode_indicator ) {
  int64_t _count=0;
  if (poms.m_verbose >= POMS_VERBOSE_ITER) {
    _count = poms.quiltResolvedCount();
    printf("# quilt_step:%i got:%i fail_counter:%i ac4init_fail:%i erode:%i quilt_cells_resolved:%i/%i(%f)\n",
        (int)quilt_step,
        ret,
        (int)fail_counter,
        (int)ac4init_fail_indicator,
        (int)erode_indicator,
        (int)_count, (int)poms.m_quilt_cell_count,
        (double)_count / (double)poms.m_quilt_cell_count);
  }
  return 0;
}


//  _   _________  ___  ____  ________
// | | / / __/ _ \/ _ )/ __ \/ __/ __/
// | |/ / _// , _/ _  / /_/ /\ \/ _/
// |___/___/_/|_/____/\____/___/___/
//
//-----
// verbose print end
//-----


//---
//---


int poms_main(int argc, char **argv) {
  int i, ii,
      r, _ret, ret, _r,
      sep = ',',
      ch,
      opt_idx=0;

  int32_t print_order[3] = {1,2,0};

  int64_t step,
          it,
          n_it=0,
          mms_step=0,
          max_mms_step=0,
          bms_step=0,
          max_bms_step=0,
          mcmc_step=0,
          max_mcmc_step=0,
          poms_step=0,
          max_poms_step=0 ;

  int64_t _count;
  int64_t quilt_step, max_quilt_step = 100000;

  int __r;
  int quilting=1;

  int32_t quilt_s[3],
          quilt_ds[3],
          stitch_s[3];

  int32_t quilt_seq_s[3];
  int xyz=0;

  int32_t _tp[3];
  int fail_counter=0, fail_counter_reset = 4;
  double erode_p = 0.1,
         erode_p_s = 0.1,
         erode_p_e = 0.025;

  int32_t x,y,z, cell;
  int code='.';

  char tmp_fn[128];


  int implicit_distance_point = 1,
      ac4init_fail_indicator=0,
      erode_indicator=0;


  std::string opt_str;

  std::vector< double > dvec;

  std::vector< std::string > tok, tok_t, tok_vals;
  std::string s;

  _opt_t opt = {0};
  opt.block_choice_policy = -1;

  opt.retry_max = 10;
  opt.tiled_fmt_type = 0;

  POMS poms;

  //----

  quilt_s[0]  = 0;  quilt_s[1]  = 0;  quilt_s[2]  = 0;
  stitch_s[0] = 1;  stitch_s[1] = 1;  stitch_s[2] = 1;

  //----

  g_ctx.m_slideshow_id = 0;

  //----


  //opt.patch_choice_policy_str = "rand";
  opt.patch_choice_policy_str = "pending";

  while (ch = getopt_long(argc, argv, "hvV:s:q:@:C:b:B:1:2:3:6:7:8:9:S:J:w:E:P:O:N:", long_options, &opt_idx)) {
    if (ch<0) { break; }

    switch (ch) {
      case 'h':
        print_help(stdout);
        return 0;
      case 'v':
        print_version(stdout);
        return 0;
      case 'V':
        opt.verbose = atoi(optarg);
        break;
      case 'S':
        opt.seed = atoi(optarg);
        break;

      case 'N':

        // noise options are
        // <freq>:<seed>:<type>
        //

        opt.noise_preset  = NOISE_PRESET_TYPE_SIMPLE;
        opt.noise_option  = NOISE_TYPE_TIERED;
        opt.noise_freq    = 0.0125;
        opt.noise_seed    = 1337;

        opt_str = optarg;
        splitStr(tok, opt_str, ':');
        if (tok.size() > 0) {
          opt.noise_freq = atof(tok[0].c_str());

          if (tok.size() > 1) {
            opt.noise_seed = atoi(tok[1].c_str());

            if (tok.size() > 2) {
              opt.noise_option = atoi(tok[2].c_str());
            }
          }
        }

        break;

      case 'O':
        opt_str = optarg;

        if (opt_str.find("ac4opt=",0) == 0) {
          splitStr(tok, opt_str, '=');
          if (tok.size() > 1) {

            if (tok[1] == "none") {
              poms.m_tile_support_option = POMS_OPTIMIZATION_AC4_NONE;
            }
            else if (tok[1] == "flat") {
              poms.m_tile_support_option = POMS_OPTIMIZATION_AC4_FLAT;
            }
            else if (tok[1] == "tier4") {
              poms.m_tile_support_option = POMS_OPTIMIZATION_AC4_TIER4;
            }
            else if (tok[1] == "tier4m1") {
              poms.m_tile_support_option = POMS_OPTIMIZATION_AC4_TIER4_M;
            }
            else if (tok[1] == "tier4m2") {
              poms.m_tile_support_option = POMS_OPTIMIZATION_AC4_TIER4_M2;
            }
            else if (tok[1] == "tier6") {
              poms.m_tile_support_option = POMS_OPTIMIZATION_AC4_TIER6;
            }
            else if (tok[1] == "tier6m1") {
              poms.m_tile_support_option = POMS_OPTIMIZATION_AC4_TIER6_M;
            }

          }
        }

        else if (opt_str.find("debug-poms-block-file=", 0)==0) {
          splitStr(tok, opt_str, '=');
          if (tok.size() > 1) {
            opt.poms_block_snapshot_fn = tok[1].c_str();
          }
        }

        else if (opt_str.find("viz_step=", 0)==0) {
          splitStr(tok, opt_str, '=');
          if (tok.size()>1) {
            opt.viz_step = atoi(tok[1].c_str());
          }
        }
        else if (opt_str.find("retry=", 0)==0) {
          splitStr(tok, opt_str, '=');
          if (tok.size()>1) {
            opt.retry_max = atoi(tok[1].c_str());
            poms.m_retry_max = opt.retry_max;
          }
        }

        else if (opt_str.find("flat_tileset=", 0)==0) {
          opt.tiled_fmt_type = 2;
        }

        else if (opt_str.find("noise-test", 0)==0) {
          opt.noise_test = 1;
        }

        else if (opt_str.find("noise-preset=", 0)==0) {
          splitStr(tok, opt_str, '=');
          if (tok.size()>1) {
            if      (tok[1] == "none")    { opt.noise_preset = NOISE_PRESET_TYPE_NONE; }
            else if (tok[1] == "simple")  { opt.noise_preset = NOISE_PRESET_TYPE_SIMPLE; }
            else if (tok[1] == "vector")  { opt.noise_preset = NOISE_PRESET_TYPE_VECTOR; }
            else                          { opt.noise_preset = NOISE_PRESET_TYPE_NONE; }
          }
        }

        else if (opt_str.find("qstep=", 0)==0) {
          splitStr(tok, opt_str, '=');
          if (tok.size()>1) {
            max_quilt_step = atoi(tok[1].c_str());
          }
        }

        // deprecate as soon as possible...
        //
        else if (opt_str.find("erode=", 0)==0) {
          splitStr(tok, opt_str, '=');
          if (tok.size() > 1) {
            opt.erode_count = atoi(tok[1].c_str());
          }
        }

        else if (opt_str.find("erode_count=", 0)==0) {
          splitStr(tok, opt_str, '=');
          if (tok.size() > 1) {
            opt.erode_count = atoi(tok[1].c_str());
          }
        }

        else if (opt_str.find("erode_p=", 0)==0) {
          splitStr(tok, opt_str, '=');
          if (tok.size() > 1) {
            splitStr(tok_t, tok[1], ':');
            erode_p_s = atof(tok_t[0].c_str());
            erode_p_e = erode_p_s;
            if (tok_t.size() > 1) {
              erode_p_e = atof(tok_t[1].c_str());
            }
          }
        }

        else if (opt_str.find("patch-policy=", 0)==0) {
          splitStr(tok, opt_str, '=');
          if (tok.size() > 1) {

            if ((tok[1] == "x+y+") ||
                (tok[1] == "xpyp")) {
              opt.patch_choice_policy_str = "xpyp";
            }
            else if ((tok[1] == "x-y+") ||
                     (tok[1] == "xnyp")) {
              opt.patch_choice_policy_str = "xnyp";
            }
            else if ((tok[1] == "x-y-") ||
                     (tok[1] == "xnyn")) {
              opt.patch_choice_policy_str = "xnyn";
            }
            else if (tok[1] == "rand") {
              opt.patch_choice_policy_str = "rand";
            }
            else if (tok[1] == "conflict") {
              opt.patch_choice_policy_str = "conflict";
            }
            else if (tok[1] == "pending") {
              opt.patch_choice_policy_str = "pending";
            }

            else if ((tok[1] == "cone") ||
                     (tok[1] == "cone+") ||
                     (tok[1] == "cone0+") ||
                     (tok[1] == "cone0")) {
              opt.patch_choice_policy_str = "cone0+";
            }
            else if ((tok[1] == "cone1") ||
                     (tok[1] == "cone1+")) {
              opt.patch_choice_policy_str = "cone1+";
            }

            else if ((tok[1] == "cone-") ||
                     (tok[1] == "cone0-")) {
              opt.patch_choice_policy_str = "cone0-";
            }
            else if (tok[1] == "cone1-") {
              opt.patch_choice_policy_str = "cone1-";
            }

            else if (tok[1] == "wf") {
              opt.patch_choice_policy_str = "wf";
            }
            else if (tok[1] == "wf-") {
              opt.patch_choice_policy_str = "wf-";
            }
            else if (tok[1] == "wf2") {
              opt.patch_choice_policy_str = "wf2";
            }
            else if (tok[1] == "wf3") {
              opt.patch_choice_policy_str = "wf3";
            }

          }
        }
        break;

      case 'P':
        opt.block_choice_policy_str = optarg;
        if (opt.block_choice_policy_str == "min") {
          opt.block_choice_policy = POMS_BLOCK_CHOICE_MIN_ENTROPY;
        }
        else if (opt.block_choice_policy_str == "max") {
          opt.block_choice_policy = POMS_BLOCK_CHOICE_MAX_ENTROPY;
        }
        else if (opt.block_choice_policy_str == "seq") {
          opt.block_choice_policy = POMS_BLOCK_CHOICE_SEQUENTIAL;
        }
        else if (opt.block_choice_policy_str == "wf") {
          opt.block_choice_policy = POMS_BLOCK_CHOICE_WAVEFRONT;
          poms.m_distance_modifier_opt = POMS_OPT_DISTANCE_XYZ;
        }
        else if (opt.block_choice_policy_str.find("wf=")==0) {
          opt.block_choice_policy = POMS_BLOCK_CHOICE_WAVEFRONT;
          poms.m_distance_modifier_opt = POMS_OPT_DISTANCE_XYZ;

          splitStr(tok, opt.block_choice_policy_str, '=');
          if (tok.size()>1) {
            if      (tok[1] == "xyz")   { poms.m_distance_modifier_opt = POMS_OPT_DISTANCE_XYZ; }
            else if (tok[1] == "xyz-")  { poms.m_distance_modifier_opt = POMS_OPT_DISTANCE_MINUS_XYZ; }
            else if (tok[1] == "cone")  { poms.m_distance_modifier_opt = POMS_OPT_DISTANCE_CONE; }
            else if (tok[1] == "cone-") { poms.m_distance_modifier_opt = POMS_OPT_DISTANCE_MINUS_CONE; }
            else if (tok[1] == "x")     { poms.m_distance_modifier_opt = POMS_OPT_DISTANCE_X; }
            else if (tok[1] == "y")     { poms.m_distance_modifier_opt = POMS_OPT_DISTANCE_Y; }
            else if (tok[1] == "z")     { poms.m_distance_modifier_opt = POMS_OPT_DISTANCE_Z; }
            else {

              splitStr(tok_t , tok[1], ':');

              if      (tok_t[0] == "plane")  {
                poms.m_distance_modifier_opt = POMS_OPT_DISTANCE_PLANE;
                poms.m_distance_coef =  1.0;

                printf("## plane+\n");
              }
              else if (tok_t[0] == "plane-") {
                poms.m_distance_modifier_opt = POMS_OPT_DISTANCE_MINUS_PLANE;
                poms.m_distance_coef = -1.0;

                printf("## plane-\n");
              }
              else {
                fprintf(stderr, "bad wavefront option (must be one of plane,plane-)\n");
                return -1;
              }

              if (tok_t.size() > 1) {
                splitStr(tok_vals, tok_t[1], ',');

                poms.m_distance_v[0] = atof(tok_vals[0].c_str());
                poms.m_distance_v[1] = ((tok_vals.size() > 1) ? atof(tok_vals[1].c_str()) : poms.m_distance_v[0]);
                poms.m_distance_v[2] = ((tok_vals.size() > 2) ? atof(tok_vals[2].c_str()) : poms.m_distance_v[1]);

                if (tok_t.size() > 2) {

                  splitStr(tok_vals, tok_t[2], ',');

                  poms.m_distance_p[0] = atof(tok_vals[0].c_str());
                  poms.m_distance_p[1] = ((tok_vals.size() > 1) ? atof(tok_vals[1].c_str()) : poms.m_distance_p[0]);
                  poms.m_distance_p[2] = ((tok_vals.size() > 2) ? atof(tok_vals[2].c_str()) : poms.m_distance_p[1]);

                  implicit_distance_point=0;
                }

              }

            }
          }

        }
        break;

      case 'w':
        opt.rand_coefficient = atof(optarg);
        break;
      case 'E':
        opt.rand_exponent = atof(optarg);
        break;

      case 's':
        s = optarg;
        splitStr(tok, s, sep);
        opt.size[0] = ((tok.size() > 0) ? atoi(tok[0].c_str()) : 1);
        opt.size[1] = ((tok.size() > 1) ? atoi(tok[1].c_str()) : opt.size[0]);
        opt.size[2] = ((tok.size() > 2) ? atoi(tok[2].c_str()) : opt.size[1]);
        break;

      case 'q':
        s = optarg;
        splitStr(tok, s, sep);
        opt.quilt_size[0] = ((tok.size() > 0) ? atoi(tok[0].c_str()) : 1);
        opt.quilt_size[1] = ((tok.size() > 1) ? atoi(tok[1].c_str()) : opt.quilt_size[0]);
        opt.quilt_size[2] = ((tok.size() > 2) ? atoi(tok[2].c_str()) : opt.quilt_size[1]);
        break;

      case 'b':
        s = optarg;
        splitStr(tok, s, sep);
        opt.block_size[0] = ((tok.size() > 0) ? atoi(tok[0].c_str()) : 1);
        opt.block_size[1] = ((tok.size() > 1) ? atoi(tok[1].c_str()) : opt.block_size[0]);
        opt.block_size[2] = ((tok.size() > 2) ? atoi(tok[2].c_str()) : opt.block_size[1]);
        break;
      case 'B':
        s = optarg;

        splitStr(tok_t, s, ':');
        if (tok_t.size() > 0) {
          splitStr(tok, tok_t[0], sep);
          opt.soften_window.soften_min[0] = ((tok.size() > 0) ? atoi(tok[0].c_str()) : 1);
          opt.soften_window.soften_min[1] = ((tok.size() > 1) ? atoi(tok[1].c_str()) : opt.soften_window.soften_min[0]);
          opt.soften_window.soften_min[2] = ((tok.size() > 2) ? atoi(tok[2].c_str()) : opt.soften_window.soften_min[1]);

          if (tok_t.size() > 1) {
            splitStr(tok, tok_t[1], sep);
            opt.soften_window.soften_max[0] = ((tok.size() > 0) ? atoi(tok[0].c_str()) : 1);
            opt.soften_window.soften_max[1] = ((tok.size() > 1) ? atoi(tok[1].c_str()) : opt.soften_window.soften_max[0]);
            opt.soften_window.soften_max[2] = ((tok.size() > 2) ? atoi(tok[2].c_str()) : opt.soften_window.soften_max[1]);
          }

        }

        break;
      case 'J':
        n_it = atoi(optarg);
        break;

      case '@':
        opt.gnuplot_fn = optarg;
        if (opt.viz_step <= 0) { opt.viz_step = 10; }
        //opt.viz_step = 1;
        break;
      case 'C':
        opt.cfg_fn = optarg;
        opt.valid = 1;
        break;
      case '1':
        opt.tiled_fn = optarg;
        break;
      case '2':
        opt.stl_fn = optarg;
        break;
      case '3':
        opt.tiled_fn = optarg;
        opt.tiled_fmt_type = 1;
        break;

      case '4':
        opt.sliced_tiled_fn = optarg;
        opt.tiled_fmt_type = 1;
        break;

      case '6':
        g_ctx.patch_snapshot_fn = optarg;
        if (opt.viz_step <= 0) { opt.viz_step = 10; }
        //opt.viz_step = 1;
        break;

      case '7':
        opt.stl_snapshot_fn = optarg;
        break;

      case '8':
        //opt.tiled_snapshot_fn = optarg;
        g_ctx.tiled_snapshot_fn = optarg;
        if (opt.viz_step <= 0) { opt.viz_step = 100; }
        //opt.viz_step = 1;
        break;
      case '9':
        opt.tiled_slideshow_dir = optarg;
        break;

      default:
        fprintf(stderr, "invalid option (%c)", (char)ch);
        print_help(stderr);
        return -1;
        break;
    }

  }

  if (!opt.valid) {
    fprintf(stderr, "must provide POMS config JSON file\n");
    print_help(stderr);
    return(-1);
  }

  poms.m_verbose = opt.verbose;

  poms.m_size[0] = ((opt.size[0] > 0) ? opt.size[0] : -1);
  poms.m_size[1] = ((opt.size[1] > 0) ? opt.size[1] : -1);
  poms.m_size[2] = ((opt.size[2] > 0) ? opt.size[2] : -1);

  poms.m_quilt_size[0] = ((opt.quilt_size[0] > 0) ? opt.quilt_size[0] : -1);
  poms.m_quilt_size[1] = ((opt.quilt_size[1] > 0) ? opt.quilt_size[1] : -1);
  poms.m_quilt_size[2] = ((opt.quilt_size[2] > 0) ? opt.quilt_size[2] : -1);

  if (poms.m_verbose >= POMS_VERBOSE_DEBUG) {
    printf("# cfg: %s\n", opt.cfg_fn.c_str());
    printf("# gp: %s\n", opt.gnuplot_fn.c_str());
    printf("# size: %i,%i,%i\n", opt.size[0], opt.size[1], opt.size[2]);
    //printf("# ac4_opt: %i\n", poms.m_ac4_opt);
    printf("# tile_support_option: %i\n", poms.m_tile_support_option);
  }

  if (opt.seed > 0) {
    poms.m_seed = opt.seed;
  }

  if (opt.rand_coefficient > 0) {
    poms.m_entropy_rand_coefficient = opt.rand_coefficient;
  }

  if (opt.rand_exponent < -1.0) {
    poms.m_entropy_rand_exponent = opt.rand_exponent;
  }

  /*
  poms.m_block_size[0] = ((opt.block_size[0] > 0) ? opt.block_size[0] : -1);
  poms.m_block_size[1] = ((opt.block_size[1] > 0) ? opt.block_size[1] : -1);
  poms.m_block_size[2] = ((opt.block_size[2] > 0) ? opt.block_size[2] : -1);

  poms.m_soften_size[0] = ((opt.soften_size[0] > 0) ? opt.soften_size[0] : -1);
  poms.m_soften_size[1] = ((opt.soften_size[1] > 0) ? opt.soften_size[1] : -1);
  poms.m_soften_size[2] = ((opt.soften_size[2] > 0) ? opt.soften_size[2] : -1);
  */

  //----
  // better default?
  //

  poms.m_block_size[0] = ((opt.block_size[0] > 0) ? opt.block_size[0] : 1);
  poms.m_block_size[1] = ((opt.block_size[1] > 0) ? opt.block_size[1] : 1);
  poms.m_block_size[2] = ((opt.block_size[2] > 0) ? opt.block_size[2] : 1);

  //poms.m_soften_size[0] = ((opt.soften_size[0] > 0) ? opt.soften_size[0] : 1);
  //poms.m_soften_size[1] = ((opt.soften_size[1] > 0) ? opt.soften_size[1] : 1);
  //poms.m_soften_size[2] = ((opt.soften_size[2] > 0) ? opt.soften_size[2] : 1);

  opt.soften_window.soften_min[0] = ((opt.soften_window.soften_min[0] > 0) ? opt.soften_window.soften_min[0] : 1);
  opt.soften_window.soften_min[1] = ((opt.soften_window.soften_min[1] > 0) ? opt.soften_window.soften_min[1] : 1);
  opt.soften_window.soften_min[2] = ((opt.soften_window.soften_min[2] > 0) ? opt.soften_window.soften_min[2] : 1);

  opt.soften_window.soften_max[0] = ((opt.soften_window.soften_max[0] > 0) ? opt.soften_window.soften_max[0] : (opt.soften_window.soften_min[0]) );
  opt.soften_window.soften_max[1] = ((opt.soften_window.soften_max[1] > 0) ? opt.soften_window.soften_max[1] : (opt.soften_window.soften_min[1]) );
  opt.soften_window.soften_max[2] = ((opt.soften_window.soften_max[2] > 0) ? opt.soften_window.soften_max[2] : (opt.soften_window.soften_min[2]) );

  r = opt.soften_window.init_cdf();
  if (r<0) {
    fprintf(stderr, "soften_window.init_cdf failed, got: %i, exiting\n", r);
    return(-1);
  }

  poms.m_soften_size[0] = opt.soften_window.soften_min[0];
  poms.m_soften_size[1] = opt.soften_window.soften_min[1];
  poms.m_soften_size[2] = opt.soften_window.soften_min[2];

  /*
  opt.window_fail.resize(128);
  opt.window_fail_s = 0;
  opt.window_fail_n = 0;
  opt.window_fail_threshold[0] = 10;
  opt.window_fail_threshold[1] = 128-10;
  */

  //
  //----

  r = poms.loadJSONFile(opt.cfg_fn);
  if (r<0) {
    fprintf(stderr, "error loading '%s' JSON config file, got %i, exiting\n",
        opt.cfg_fn.c_str(),
        r);
    return(-1);
  }

  poms.clampParameters();

  if (poms.m_verbose >= POMS_VERBOSE_RUN) {
    printf("## soften_window:{min:[%i,%i,%i], max:[%i,%i,%i]}\n",
        opt.soften_window.soften_min[0], opt.soften_window.soften_min[1], opt.soften_window.soften_min[2],
        opt.soften_window.soften_max[0], opt.soften_window.soften_max[1], opt.soften_window.soften_max[2]);
    printf("## m_quilt_size[%i,%i,%i]\n",
        (int)poms.m_quilt_size[0],
        (int)poms.m_quilt_size[1],
        (int)poms.m_quilt_size[2]);
    printf("## m_block_size[%i,%i,%i]\n",
        (int)poms.m_block_size[0],
        (int)poms.m_block_size[1],
        (int)poms.m_block_size[2]);
    printf("## m_size[%i,%i,%i]\n",
        (int)poms.m_size[0],
        (int)poms.m_size[1],
        (int)poms.m_size[2]);
  }


  //----

  quilt_ds[0] = poms.m_size[0]/2;
  quilt_ds[1] = poms.m_size[1]/2;
  quilt_ds[2] = poms.m_size[2]/2;

  if (quilt_ds[0] == 0) { quilt_ds[0] = 1; }
  if (quilt_ds[1] == 0) { quilt_ds[1] = 1; }
  if (quilt_ds[2] == 0) { quilt_ds[2] = 1; }

  // check to make sure parameters make sense
  //

  if ((poms.m_block_size[0] <= 0) ||
      (poms.m_block_size[1] <= 0) ||
      (poms.m_block_size[2] <= 0)) {
    fprintf(stderr, "invalid block size [%i,%i,%i], exiting\n",
        (int)poms.m_block_size[0],
        (int)poms.m_block_size[1],
        (int)poms.m_block_size[2]);
    return(-1);
  }

  if ((poms.m_soften_size[0] <= 0) ||
      (poms.m_soften_size[1] <= 0) ||
      (poms.m_soften_size[2] <= 0)) {
    fprintf(stderr, "invalid soften size [%i,%i,%i], exiting\n",
        (int)poms.m_soften_size[0],
        (int)poms.m_soften_size[1],
        (int)poms.m_soften_size[2]);
    return(-1);
  }

  //----

  //CUSTOM NOISE OPTION
  //
  if (opt.noise_option > 0) {
    init_noise(poms, opt);
    poms.m_g_cb = custom_G;

    if (poms.m_verbose >= POMS_VERBOSE_RUN) {
      printf("## noise info:\n");
      printf("## opt.noise{ preset:%i, option:%i, freq:%f, seed:%i  }\n",
          opt.noise_preset, opt.noise_option, opt.noise_freq, opt.noise_seed);
      printf("## g_ctx.noise[%i]:\n", (int)g_ctx.noise.size());
      for (i=0; i<g_ctx.noise.size(); i++) {
        printf("##   [%i]: { type:%i, {m:%f,M:%f,T:%f}, freq:%f, seed:%i }\n",
            i, g_ctx.noise_type[i],
            g_ctx.noise_min[i], g_ctx.noise_max[i], g_ctx.noise_threshold[i],
            g_ctx.noise[i].frequency, g_ctx.noise[i].seed );
      }
    }

    if (opt.noise_test) {
      print_noise(poms, g_ctx);
      return(0);
    }

  }


  if (poms.m_verbose >= POMS_VERBOSE_DEBUG3) {
    printf("## after JSON load, before constraints:\n");
    poms.printDebug();
  }

  if (poms.m_verbose >= POMS_VERBOSE_RUN) {
    printf("# fstat:\n");
    poms.printDebugFStat();
    poms.printDebugMemStat();
    printf("\n\n");
    fflush(stdout);
  }

  //----
  //----
  //----

  if (poms.m_verbose >= POMS_VERBOSE_DEBUG) {
    printf("# PHASE_START\n");
    poms.printDebug();
  }

  poms.m_tile_choice_policy = POMS_TILE_CHOICE_PROB;

  poms.m_block_choice_policy = POMS_BLOCK_CHOICE_MIN_ENTROPY;
  if (opt.block_choice_policy >= 0) {
    poms.m_block_choice_policy = opt.block_choice_policy;
  }

  //------------------------------------
  //              _ _ _   _
  //   __ _ _   _(_) | |_(_)_ __   __ _
  //  / _` | | | | | | __| | '_ \ / _` |
  // | (_| | |_| | | | |_| | | | | (_| |
  //  \__, |\__,_|_|_|\__|_|_| |_|\__, |
  //     |_|                      |___/
  //------------------------------------


  //---
  //---
  //---

  g_ctx.poms = &poms;
  g_ctx.m_iter=-1;
  g_ctx.m_alpha = 2;
  g_ctx.m_beta = 1.0;


  if (g_ctx.tiled_snapshot_fn.size() > 0) {
    g_ctx.T.tilesets.resize(1);
    g_ctx.T.layers.resize(2);
    g_ctx.T.width = poms.m_quilt_size[0];
    g_ctx.T.height = poms.m_quilt_size[1];
    g_ctx.T.tileheight = poms.m_tileset_ctx.tileheight;
    g_ctx.T.tilewidth = poms.m_tileset_ctx.tilewidth;

    g_ctx.T.tilesets[0].tileheight = poms.m_tileset_ctx.tileheight;
    g_ctx.T.tilesets[0].tilewidth = poms.m_tileset_ctx.tilewidth;
    g_ctx.T.tilesets[0].tilecount = poms.m_tileset_ctx.tilecount;
    g_ctx.T.tilesets[0].image = poms.m_tileset_ctx.image;

    g_ctx.T.layers[0].height = g_ctx.T.height;
    g_ctx.T.layers[0].width = g_ctx.T.width;
    g_ctx.T.layers[0].data.resize( g_ctx.T.width * g_ctx.T.height );

    g_ctx.T.layers[1].height = g_ctx.T.height;
    g_ctx.T.layers[1].width = g_ctx.T.width;
    g_ctx.T.layers[1].data.resize( g_ctx.T.width * g_ctx.T.height );
    g_ctx.T.layers[1].name = "cellSize";
    g_ctx.T.layers[1].visible = false;

    g_ctx.m_conflict_grid.resize( poms.m_quilt_cell_count, 0 );
  }


  //---
  //---
  //---


  quilt_s[0] = 0;
  quilt_s[1] = 0;
  quilt_s[2] = 0;

  quilt_seq_s[0] = 0;
  quilt_seq_s[1] = 0;
  quilt_seq_s[2] = 0;

  if (implicit_distance_point) {
    poms.m_distance_p[0] = (double)poms.m_size[0]/2.0;
    poms.m_distance_p[1] = (double)poms.m_size[1]/2.0;
    poms.m_distance_p[2] = (double)poms.m_size[2]/2.0;
  }

  for (quilt_step=0; quilt_step < max_quilt_step; quilt_step++) {

    g_ctx.m_iter++;

    opt.soften_window.choose_soften_size( poms.m_soften_size );

    // TODO:
    // many(most?) of these patch strategies are broken, need to go through
    // and fix up
    //
    if (opt.patch_choice_policy_str == "rand") {
      _patch_choice_rand(quilt_s, quilt_ds, poms.m_size, poms.m_quilt_size);
    }
    else if (opt.patch_choice_policy_str == "xpyp") {
      _patch_choice_xpyp(quilt_seq_s, quilt_ds, poms.m_size, poms.m_quilt_size);
    }
    else if (opt.patch_choice_policy_str == "xnyn") {
      _patch_choice_xnyn(quilt_seq_s, quilt_ds, poms.m_size, poms.m_quilt_size);
    }

    else if (opt.patch_choice_policy_str == "conflict") {
      _patch_choice_weight_conflict(g_ctx, quilt_s, quilt_ds, poms.m_size, poms.m_quilt_size);
    }
    else if (opt.patch_choice_policy_str == "pending") {
      _patch_choice_weight_pending(poms, quilt_s, quilt_ds, poms.m_size, poms.m_quilt_size);
    }

    else if (opt.patch_choice_policy_str == "cone0+") {
      _patch_choice_wf_cone0p(g_ctx, quilt_s, quilt_ds, poms.m_size, poms.m_quilt_size);
    }
    else if (opt.patch_choice_policy_str == "cone1+") {
      _patch_choice_wf_cone1p(g_ctx, quilt_s, quilt_ds, poms.m_size, poms.m_quilt_size);
    }
    else if (opt.patch_choice_policy_str == "cone0-") {
      _patch_choice_wf_cone0n(g_ctx, quilt_s, quilt_ds, poms.m_size, poms.m_quilt_size);
    }
    else if (opt.patch_choice_policy_str == "cone1-") {
      _patch_choice_wf_cone1n(g_ctx, quilt_s, quilt_ds, poms.m_size, poms.m_quilt_size);
    }


    else if (opt.patch_choice_policy_str == "wf") {
      _patch_choice_wf_xy(g_ctx, quilt_s, quilt_ds, poms.m_size, poms.m_quilt_size);
    }
    else if (opt.patch_choice_policy_str == "wf-") {
      _patch_choice_wf_xy_n(g_ctx, quilt_s, quilt_ds, poms.m_size, poms.m_quilt_size);
    }
    else if (opt.patch_choice_policy_str == "wf2") {
      _patch_choice_wf_xyz2(g_ctx, quilt_s, quilt_ds, poms.m_size, poms.m_quilt_size);
    }
    else if (opt.patch_choice_policy_str == "wf3") {
      _patch_choice_wf_cube(g_ctx, quilt_s, quilt_ds, poms.m_size, poms.m_quilt_size);
    }
    else {
      printf("ERROR: bad patch choice policy option, exiting\n"); return(-1);
    }

    erode_p = (double)poms.quiltResolvedCount() / (double)poms.m_quilt_cell_count;
    erode_p = (erode_p_e - erode_p_s)*erode_p + erode_p_s;

    for (xyz=0; xyz<3; xyz++) {
      poms.m_patch_region[xyz][0] = quilt_s[xyz];
      poms.m_patch_region[xyz][1] = quilt_s[xyz] + poms.m_size[xyz];
    }

    ac4init_fail_indicator = 0;
    erode_indicator = 0;

    // Take POMS snapshot so we can debug this block
    //
    _verbose_quilt_step_start( poms, opt, quilt_step, erode_p, erode_p_s, erode_p_e, fail_counter, fail_counter_reset );
    _update_viz_block_snapshot( opt, poms, quilt_step );

    // Setup patch (block) for block level solver
    //
    r = poms.setupQuiltPatch();
    if (r<0) {

      ac4init_fail_indicator = 1;
      fail_counter++;

      _verbose_setup_quilt_patch_fail( poms, fail_counter, fail_counter_reset );

      // REVERSION
      //
      _remove_quilt_region( g_ctx, (int32_t *)(&(poms.m_patch_region[0][0])), 1 );

      // EROSION
      //
      if (fail_counter >= fail_counter_reset) {

        erode_indicator = 1;

        fail_counter = 0;

        _verbose_eroding( poms, erode_p, erode_p_s, erode_p_e );

        for (ii=0; ii<1; ii++) {
          _erode_quilt_region( g_ctx, (int32_t *)(&(poms.m_patch_region[0][0])) );
          _erode_quilt(g_ctx, erode_p);
        }

      }

      _update_viz_snapshots( opt, poms, g_ctx );
      _verbose_quilt_step( poms, ret, quilt_step, fail_counter, ac4init_fail_indicator, erode_indicator );

      continue;
    }


    //--------------------
    //    ___  __  _______
    //   / _ )/  |/  / __/
    //  / _  / /|_/ /\ \
    // /____/_/  /_/___/
    //
    //--------------------


    r = poms.BMSInit();
    if (r < 0) { return err_and_return("BMSInit error"); }
    _update_viz_init( opt, g_ctx );

    if (r == 0) {

      n_it = ( (n_it <= 0) ? poms.blockSequenceCount() : n_it );
      max_bms_step = poms.m_block_size[0]*poms.m_block_size[1]*poms.m_block_size[2];

      ret = 1;

      _verbose_block_solver_start( poms, n_it, max_bms_step );

      for (it=0; (it<n_it) && (ret>=0); it++) {

        opt.soften_window.choose_soften_size( poms.m_soften_size );
        _verbose_block_solver_iter_beg( poms, it, n_it );

        r = poms.BMSBegin();
        if (r<0) { ret=r; break; }

        _update_viz_begin( it, opt, poms, g_ctx );

        // run bms on block until max step, resolved or contradiction
        //
        for (bms_step=0; bms_step<max_bms_step; bms_step++) {
          r = poms.BMSStep();
          if (r<=0) { break; }
          _update_viz_step( bms_step, opt, poms, g_ctx );
        }

        // keep block or revert
        //
        r = poms.BMSEnd();
        if (r<=0) { ret=r; break; }

        // viz here as well in case it gets skipped in the above
        // iteration
        //
        _update_viz_step( bms_step, opt, poms, g_ctx );
        _verbose_block_solver_iter_end( poms, it, n_it );
      }

    }

    // BMSInit call fully resovled grid, fall through below and cleanup
    //
    else { ret = 0; }

    _update_viz_snapshots( opt, poms, g_ctx );

    //--------------------
    //    ___  __  _______
    //   / _ )/  |/  / __/
    //  / _  / /|_/ /\ \
    // /____/_/  /_/___/
    //
    //--------------------


    _verbose_end_step( poms, ret );

    erode_indicator=0;

    if (ret == 0) {

      _verbose_quilt_save_begin( poms );

      // SUCCESS
      //
      r = poms.saveQuiltPatchRegion();
      if (r<0) { return err_and_return("saveQuiltPatchRegion error"); }

      _verbose_quilt_save_end( poms, ret, print_order );

      _update_viz_stl_snapshot(opt, poms);
    }
    else {

      _verbose_erode_begin( poms );

      // EROSION
      //
      fail_counter++;
      if (fail_counter >= fail_counter_reset) {
        fail_counter=0;

        erode_indicator=1;

        _verbose_eroding( poms, erode_p, erode_p_s, erode_p_e );
  
        for (ii=0; ii<1; ii++) {
          _erode_quilt_region( g_ctx, (int32_t *)(&(poms.m_patch_region[0][0])) );
          _erode_quilt(g_ctx,erode_p);
        }

      }

    }

    _verbose_quilt_step( poms, ret, quilt_step, fail_counter, ac4init_fail_indicator, erode_indicator );

    // see if we've found a full resolution
    //
    if (poms.quiltResolvedCount() == poms.m_quilt_cell_count) {
      quilting = 0;
      break;
    }

    //----
    //----
    //----

    _update_viz_tiled_intermediate( opt, poms );
    _update_viz_stl( opt, poms );
  }

  _verbose_fin(poms, quilting, quilt_step );

  if (poms.m_verbose >= POMS_VERBOSE_RUN) {
    printf("## PROF\n");
    poms._prof_print();
  }

  _update_viz_stl( opt, poms );
  _update_viz_patch_snapshot( g_ctx );
  _update_viz_tiled_fin( opt, poms );

  return 0;
}

#ifndef CUSTOM_MAIN
int main(int argc, char **argv) {
  return poms_main(argc, argv);
}
#endif

