#include "poms.hpp"
#include <yajl/yajl_tree.h>
//#include "yajl/yajl_tree.h"

static int _load_boundary_condition_json(POMS &poms, yajl_val root_bc) {
  int i, j, k;
  std::string _s, _t;

  yajl_val vs,
           node_b,
           v_t, v_v;

  int ival=-1;

  int32_t tile_s, tile_e_ninc,
          x_s, x_e_ninc,
          y_s, y_e_ninc,
          z_s, z_e_ninc;

  const char *type_path[]   = {"type", NULL};
  const char *value_path[]  = {"value", NULL};

  const char *xp_path[]      = {"x+", NULL};
  const char *xn_path[]      = {"x-", NULL};
  const char *yp_path[]      = {"y+", NULL};
  const char *yn_path[]      = {"y-", NULL};
  const char *zp_path[]      = {"z+", NULL};
  const char *zn_path[]      = {"z-", NULL};

  node_b = yajl_tree_get( root_bc , xp_path, yajl_t_object );
  v_t = yajl_tree_get( node_b, type_path, yajl_t_string );
  _t = YAJL_GET_STRING( v_t );
  if ( _t == "tile") {
    v_v = yajl_tree_get( node_b, value_path, yajl_t_number );
    ival = YAJL_GET_INTEGER( v_v );
  }

  node_b = yajl_tree_get( root_bc , xn_path, yajl_t_object );
  v_t = yajl_tree_get( node_b, type_path, yajl_t_string );
  _t = YAJL_GET_STRING( v_t );
  if ( _t == "tile") {
    v_v = yajl_tree_get( node_b, value_path, yajl_t_number );
    ival = YAJL_GET_INTEGER( v_v );
  }

  node_b = yajl_tree_get( root_bc , yp_path, yajl_t_object );
  v_t = yajl_tree_get( node_b, type_path, yajl_t_string );
  _t = YAJL_GET_STRING( v_t );
  if ( _t == "tile") {
    v_v = yajl_tree_get( node_b, value_path, yajl_t_number );
    ival = YAJL_GET_INTEGER( v_v );
  }

  node_b = yajl_tree_get( root_bc , yn_path, yajl_t_object );
  v_t = yajl_tree_get( node_b, type_path, yajl_t_string );
  _t = YAJL_GET_STRING( v_t );
  if ( _t == "tile") {
    v_v = yajl_tree_get( node_b, value_path, yajl_t_number );
    ival = YAJL_GET_INTEGER( v_v );
  }

  node_b = yajl_tree_get( root_bc , zp_path, yajl_t_object );
  v_t = yajl_tree_get( node_b, type_path, yajl_t_string );
  _t = YAJL_GET_STRING( v_t );
  if ( _t == "tile") {
    v_v = yajl_tree_get( node_b, value_path, yajl_t_number );
    ival = YAJL_GET_INTEGER( v_v );
  }

  node_b = yajl_tree_get( root_bc , zn_path, yajl_t_object );
  v_t = yajl_tree_get( node_b, type_path, yajl_t_string );
  _t = YAJL_GET_STRING( v_t );
  if ( _t == "tile") {
    v_v = yajl_tree_get( node_b, value_path, yajl_t_number );
    ival = YAJL_GET_INTEGER( v_v );
  }

  return 0;
}

static int _load_quilt_patch_json(POMS &poms, yajl_val root_quiltPatch) {
  int i, j, k;
  int ival;
  yajl_val vp,
           v_tile_a,
           node_range,
           v_x, v_y, v_z;
  int32_t x_s, x_e_ninc,
          y_s, y_e_ninc,
          z_s, z_e_ninc;
  int64_t n;

  const char *tile_path[]   = {"tile", NULL};
  const char *range_path[]  = {"range", NULL};
  const char *x_path[]      = {"x", NULL};
  const char *y_path[]      = {"y", NULL};
  const char *z_path[]      = {"z", NULL};

  std::vector< int32_t > tile_a;

  poms_quilt_patch_t patch;

  for (i=0; i<root_quiltPatch->u.array.len; i++) {
    vp = root_quiltPatch->u.array.values[i];

    patch.tile.clear();
    x_s = -1; x_e_ninc = -1;
    y_s = -1; y_e_ninc = -1;
    z_s = -1; z_e_ninc = -1;

    v_tile_a = yajl_tree_get( vp, tile_path, yajl_t_array );
    if (!v_tile_a) { continue; }

    node_range = yajl_tree_get( vp, range_path, yajl_t_object );
    if (!node_range) { continue; }

    v_x = yajl_tree_get( node_range, x_path, yajl_t_array );
    if (!v_x) { continue; }

    v_y = yajl_tree_get( node_range, y_path, yajl_t_array );
    if (!v_y) { continue; }

    v_z = yajl_tree_get( node_range, z_path, yajl_t_array );
    if (!v_z) { continue; }

    for (j=0; j<v_x->u.array.len; j++) {
      ival = YAJL_GET_INTEGER( v_x->u.array.values[j] );
      if (j==0) { x_s = ival; }
      if (j==1) { x_e_ninc = ival; }
    }

    for (j=0; j<v_y->u.array.len; j++) {
      ival = YAJL_GET_INTEGER( v_y->u.array.values[j] );
      if (j==0) { y_s = ival; }
      if (j==1) { y_e_ninc = ival; }
    }

    for (j=0; j<v_z->u.array.len; j++) {
      ival = YAJL_GET_INTEGER( v_z->u.array.values[j] );
      if (j==0) { z_s = ival; }
      if (j==1) { z_e_ninc = ival; }
    }

    n = (x_e_ninc - x_s)*(y_e_ninc - y_s)*(z_e_ninc - z_s);

    if (n != v_tile_a->u.array.len) { continue; }

    patch.m_region[0][0] = x_s;
    patch.m_region[0][1] = x_e_ninc;

    patch.m_region[1][0] = y_s;
    patch.m_region[1][1] = y_e_ninc;

    patch.m_region[2][0] = z_s;
    patch.m_region[2][1] = z_e_ninc;

    for (j=0; j<v_tile_a->u.array.len; j++) {
      ival = YAJL_GET_INTEGER(v_tile_a->u.array.values[j]);
      patch.tile.push_back(ival);
    }

    poms.m_quilt_patch.push_back(patch);

  }

  return 0;
}

static int _load_constraint_json(POMS &poms, yajl_val root_constraint) {
  int i, j, k;
  std::string _s, _t;
  yajl_val vp, va;

  yajl_val vs,
           node_range,
           v_ta, v_x, v_y, v_z;

  int ival;

  int32_t tile_s, tile_e_ninc,
          x_s, x_e_ninc,
          y_s, y_e_ninc,
          z_s, z_e_ninc;

  // keep 'implicit' bounds
  //
  int32_t im_tile_s, im_tile_e_ninc,
          im_x_s, im_x_e_ninc,
          im_y_s, im_y_e_ninc,
          im_z_s, im_z_e_ninc;

  const char *type_path[]   = {"type", NULL};
  const char *value_path[]  = {"value", NULL};
  const char *range_path[]  = {"range", NULL};
  const char *tile_path[]   = {"tile", NULL};
  const char *x_path[]      = {"x", NULL};
  const char *y_path[]      = {"y", NULL};
  const char *z_path[]      = {"z", NULL};

  char _type=0;
  poms_constraint_t constraint = {0};

  for (i=0; i<root_constraint->u.array.len; i++) {
    vp = root_constraint->u.array.values[i];

    vs = yajl_tree_get( vp , type_path, yajl_t_string );
    if (!vs) { continue; }

    _t = YAJL_GET_STRING( vs );

    _type = '.';
    if      (_t == "add")         { _type = 'a'; }
    else if (_t == "rem")         { _type = 'd'; }
    else if (_t == "remove")      { _type = 'd'; }
    else if (_t == "del")         { _type = 'd'; }
    else if (_t == "delete")      { _type = 'd'; }
    else if (_t == "force")       { _type = 'f'; }
    else if (_t == "fix")         { _type = 'f'; }

    else if (_t == "startAdd")    { _type = 'A'; }
    else if (_t == "startRem")    { _type = 'D'; }
    else if (_t == "startRemove") { _type = 'D'; }
    else if (_t == "startDel")    { _type = 'D'; }
    else if (_t == "startDelete") { _type = 'D'; }
    else if (_t == "startFix")    { _type = 'F'; }
    else if (_t == "startForce")  { _type = 'F'; }

    else if (_t == "a")           { _type = 'a'; }
    else if (_t == "d")           { _type = 'd'; }
    else if (_t == "f")           { _type = 'f'; }

    else if (_t == "A")           { _type = 'A'; }
    else if (_t == "D")           { _type = 'D'; }
    else if (_t == "F")           { _type = 'F'; }

    else if (_t == "p")           { _type = 'p'; }
    else if (_t == "pin")         { _type = 'p'; }

    else if (_t == "quiltAdd")    { _type = '+'; }
    else if (_t == "quiltRem")    { _type = '-'; }
    else if (_t == "quiltRemove") { _type = '-'; }
    else if (_t == "quiltDel")    { _type = '-'; }
    else if (_t == "quiltDelete") { _type = '-'; }
    else if (_t == "quiltFix")    { _type = '!'; }
    else if (_t == "quiltForce")  { _type = '!'; }
    else if (_t == "quiltPin")    { _type = '='; }

    else if (_t == "+")           { _type = '+'; }
    else if (_t == "-")           { _type = '-'; }
    else if (_t == "!")           { _type = '!'; }
    else if (_t == "=")           { _type = '='; }

    node_range = yajl_tree_get( vp, range_path, yajl_t_object );
    if (!node_range) { continue; }

    v_ta = yajl_tree_get( node_range, tile_path, yajl_t_array );
    if (!v_ta) { continue; }

    v_x = yajl_tree_get( node_range, x_path, yajl_t_array );
    if (!v_x) { continue; }

    v_y = yajl_tree_get( node_range, y_path, yajl_t_array );
    if (!v_y) { continue; }

    v_z = yajl_tree_get( node_range, z_path, yajl_t_array );
    if (!v_z) { continue; }

    tile_s = 0;
    tile_e_ninc = poms.m_tile_count;

    im_tile_s = 0;
    im_tile_e_ninc = 0;

    x_s = 0; x_e_ninc = poms.m_size[0];
    y_s = 0; y_e_ninc = poms.m_size[1];
    z_s = 0; z_e_ninc = poms.m_size[2];

    im_x_s = 0; im_x_e_ninc = poms.m_size[0];
    im_y_s = 0; im_y_e_ninc = poms.m_size[1];
    im_z_s = 0; im_z_e_ninc = poms.m_size[2];

    // fill with default quilt endpoints if these
    // are quilt constraints
    //
    if ((_type == '+') ||
        (_type == '-') ||
        (_type == '=') ||
        (_type == '!')) {
      x_s = 0; x_e_ninc = poms.m_quilt_size[0];
      y_s = 0; y_e_ninc = poms.m_quilt_size[1];
      z_s = 0; z_e_ninc = poms.m_quilt_size[2];

      im_x_s = 0; im_x_e_ninc = poms.m_quilt_size[0];
      im_y_s = 0; im_y_e_ninc = poms.m_quilt_size[1];
      im_z_s = 0; im_z_e_ninc = poms.m_quilt_size[2];
    }

    for (j=0; j<v_ta->u.array.len; j++) {
      ival = YAJL_GET_INTEGER( v_ta->u.array.values[j] );

      if (j==0) { im_tile_s = ival; }
      if (j==1) { im_tile_e_ninc = ival; }

      if (ival<0) { ival = poms.m_tile_count+ival; }

      if (j==0) { tile_s = ival; }
      if (j==1) { tile_e_ninc = ival; }
    }

    for (j=0; j<v_x->u.array.len; j++) {
      ival = YAJL_GET_INTEGER( v_x->u.array.values[j] );

      if (j==0) { im_x_s = ival; }
      if (j==1) { im_x_e_ninc = ival; }

      if (ival<0) {
        ival = poms.m_size[0]+ival;
        if ((_type == '+') ||
            (_type == '-') ||
            (_type == '=') ||
            (_type == '!')) {
          ival = poms.m_quilt_size[0]+ival;
        }
      }

      if (j==0) { x_s = ival; }
      if (j==1) { x_e_ninc = ival; }
    }

    for (j=0; j<v_y->u.array.len; j++) {
      ival = YAJL_GET_INTEGER( v_y->u.array.values[j] );

      if (j==0) { im_y_s = ival; }
      if (j==1) { im_y_e_ninc = ival; }

      if (ival<0) {
        ival = poms.m_size[1]+ival;
        if ((_type == '+') ||
            (_type == '-') ||
            (_type == '=') ||
            (_type == '!')) {
          ival = poms.m_quilt_size[1]+ival;
        }
      }

      if (j==0) { y_s = ival; }
      if (j==1) { y_e_ninc = ival; }
    }

    for (j=0; j<v_z->u.array.len; j++) {
      ival = YAJL_GET_INTEGER( v_z->u.array.values[j] );

      if (j==0) { im_z_s = ival; }
      if (j==1) { im_z_e_ninc = ival; }

      if (ival<0) {
        ival = poms.m_size[2]+ival;
        if ((_type == '+') ||
            (_type == '-') ||
            (_type == '=') ||
            (_type == '!')) {
          ival = poms.m_quilt_size[2]+ival;
        }
      }

      if (j==0) { z_s = ival; }
      if (j==1) { z_e_ninc = ival; }
    }


    constraint.type = _type;
    constraint.tile_range[0] = tile_s;
    constraint.tile_range[1] = tile_e_ninc;

    constraint.size_range[0][0] = x_s;
    constraint.size_range[0][1] = x_e_ninc;

    constraint.size_range[1][0] = y_s;
    constraint.size_range[1][1] = y_e_ninc;

    constraint.size_range[2][0] = z_s;
    constraint.size_range[2][1] = z_e_ninc;


    constraint.implicit_tile[0] = im_tile_s;
    constraint.implicit_tile[1] = im_tile_e_ninc;

    constraint.implicit_size[0][0] = im_x_s;
    constraint.implicit_size[0][1] = im_x_e_ninc;

    constraint.implicit_size[1][0] = im_y_s;
    constraint.implicit_size[1][1] = im_y_e_ninc;

    constraint.implicit_size[2][0] = im_z_s;
    constraint.implicit_size[2][1] = im_z_e_ninc;

    poms.m_constraint.push_back( constraint );

    if ((_type == 'A') ||
        (_type == 'F') ||
        (_type == 'D')) {
      poms.m_constraint_start_count++;
    }
  }

  return 0;
}


int POMS::loadJSONFile(std::string &fn) {
  std::string buf;
  FILE *fp=NULL;
  int ch=0;

  buf.clear();

  fp = fopen(fn.c_str(), "r");
  if (!fp) { return -1; }
  while (!feof(fp)) {
    ch = fgetc(fp);
    if (ch==EOF) { continue; }
    buf.push_back((char)ch);
  }
  fclose(fp);

  return loadJSONString(buf);
}

static int _parse_tileset_info( tileset_ctx_t &ctx, yajl_val root_node ) {
  const char *ts[]              = { "tileset", NULL};
  const char *ts_image[]        = { "tileset", "image", NULL};
  const char *ts_tilecount[]    = { "tileset", "tilecount", NULL};
  const char *ts_imageheight[]  = { "tileset", "imageheight", NULL};
  const char *ts_imagewidth[]   = { "tileset", "imagewidth", NULL};
  const char *ts_tileheight[]   = { "tileset", "tileheight", NULL};
  const char *ts_tilewidth[]    = { "tileset", "tilewidth", NULL};

  std::string _s;
  yajl_val node;
  yajl_val v0, v1, v2;
  yajl_val vp, va, vs, vi, vn, vo;
  char errbuf[1024];

  std::string image_fn;
  int32_t tilecount=-1,
          imageheight=-1, imagewidth=-1,
          tileheight=-1, tilewidth=-1;

  vo = yajl_tree_get( root_node, ts, yajl_t_object );
  if (!vo) { return -1; }

  vs = yajl_tree_get( root_node, ts_image, yajl_t_string );
  if (vs) { image_fn = YAJL_GET_STRING(vs); }

  vi = yajl_tree_get( root_node, ts_tilecount, yajl_t_number );
  if (vi) { tilecount = YAJL_GET_INTEGER( vi ); }

  vi = yajl_tree_get( root_node, ts_imageheight, yajl_t_number );
  if (vi) { imageheight = YAJL_GET_INTEGER( vi ); }

  vi = yajl_tree_get( root_node, ts_imagewidth, yajl_t_number );
  if (vi) { imagewidth = YAJL_GET_INTEGER( vi ); }

  vi = yajl_tree_get( root_node, ts_tileheight, yajl_t_number );
  if (vi) { tileheight = YAJL_GET_INTEGER( vi ); }

  vi = yajl_tree_get( root_node, ts_tilewidth, yajl_t_number );
  if (vi) { tilewidth = YAJL_GET_INTEGER( vi ); }

  ctx.name          = image_fn;
  ctx.image         = image_fn;
  ctx.tilecount     = tilecount;
  ctx.imagewidth    = imagewidth;
  ctx.imageheight   = imageheight;
  ctx.tilewidth     = tilewidth;
  ctx.tileheight    = tileheight;

  return 0;
}

static int _parse_flat_tileset_info( tileset_ctx_t &ctx, yajl_val root_node ) {
  const char *ts[]              = { "flatTileset", NULL};
  const char *ts_image[]        = { "flatTileset", "image", NULL};
  const char *ts_tilecount[]    = { "flatTileset", "tilecount", NULL};
  const char *ts_imageheight[]  = { "flatTileset", "imageheight", NULL};
  const char *ts_imagewidth[]   = { "flatTileset", "imagewidth", NULL};
  const char *ts_tileheight[]   = { "flatTileset", "tileheight", NULL};
  const char *ts_tilewidth[]    = { "flatTileset", "tilewidth", NULL};

  std::string _s;
  yajl_val node;
  yajl_val v0, v1, v2;
  yajl_val vp, va, vs, vi, vn, vo;
  char errbuf[1024];

  std::string image_fn;
  int32_t tilecount=-1,
          imageheight=-1, imagewidth=-1,
          tileheight=-1, tilewidth=-1;

  vo = yajl_tree_get( root_node, ts, yajl_t_object );
  if (!vo) { return -1; }

  vs = yajl_tree_get( root_node, ts_image, yajl_t_string );
  if (vs) { image_fn = YAJL_GET_STRING(vs); }

  vi = yajl_tree_get( root_node, ts_tilecount, yajl_t_number );
  if (vi) { tilecount = YAJL_GET_INTEGER( vi ); }

  vi = yajl_tree_get( root_node, ts_imageheight, yajl_t_number );
  if (vi) { imageheight = YAJL_GET_INTEGER( vi ); }

  vi = yajl_tree_get( root_node, ts_imagewidth, yajl_t_number );
  if (vi) { imagewidth = YAJL_GET_INTEGER( vi ); }

  vi = yajl_tree_get( root_node, ts_tileheight, yajl_t_number );
  if (vi) { tileheight = YAJL_GET_INTEGER( vi ); }

  vi = yajl_tree_get( root_node, ts_tilewidth, yajl_t_number );
  if (vi) { tilewidth = YAJL_GET_INTEGER( vi ); }

  ctx.name          = image_fn;
  ctx.image         = image_fn;
  ctx.tilecount     = tilecount;
  ctx.imagewidth    = imagewidth;
  ctx.imageheight   = imageheight;
  ctx.tilewidth     = tilewidth;
  ctx.tileheight    = tileheight;

  return 0;
}

int POMS::loadJSONString(std::string &json_buf) {
  const char *size_path[] = {"size", NULL};
  const char *rule_path[] = {"rule", NULL};
  const char *name_path[] = {"name", NULL};
  const char *weight_path[] = {"weight", NULL};
  const char *tile_group_path[] = {"tileGroup", NULL};
  const char *tile_flat_map_path[] = {"flatMap", NULL};
  const char *seed_path[] = {"seed", NULL};
  const char *constraint_path[] = {"constraint", NULL};
  const char *boundaryCondition_path[] = {"boundaryCondition", NULL};
  const char *blockSize_path[] = {"blockSize", NULL};
  const char *softenSize_path[] = {"softenSize", NULL};
  const char *quiltSize_path[] = {"quiltSize", NULL};
  const char *objMap_path[] = {"objMap", NULL};

  const char *quiltPatch_path[] = {"quiltPatch", NULL};

  const char *type_path[]   = {"type", NULL};
  const char *value_path[]  = {"value", NULL};
  const char *range_path[]  = {"range", NULL};

  std::string _s;
  yajl_val root_node, node;
  yajl_val v0, v1, v2;
  yajl_val vp, va;
  char errbuf[1024];

  std::vector< std::vector< double > > rule_list;
  std::vector< double > _d_list;

  int32_t src_tile,
          dst_tile,
          idir, rdir;
  int ival;
  double d;

  double _ecount=0.0, _tcount=0.0;

  int i, j, k;

  m_constraint.clear();
  m_constraint_start_count=0;

  root_node = yajl_tree_parse(json_buf.c_str(), errbuf, sizeof(errbuf));
  if (!root_node) { return -1; }

  m_tile_name.clear();
  m_tile_weight.clear();
  m_tile_group.clear();
  m_tile_flat_map.clear();

  //--
  // size
  //
  // size is kind of privileged as constraints and some other
  // structures later on depend on it.
  // If size has been provided, use that, otherwise read size
  // as the first thing so it can be used later.
  // If neither, we can't really proceed, so bail out.
  //

  v0 = yajl_tree_get( root_node, size_path, yajl_t_array );
  if (v0) {
    for (i=0; i<v0->u.array.len; i++) {
      ival = YAJL_GET_INTEGER( v0->u.array.values[i] );

      if ((i<3) &&
          (m_size[i] <= 0) &&
          (ival > 0)) {
        m_size[i] = (int32_t)ival;
      }
    }
  }

  if ((m_size[0] <= 0) ||
      (m_size[1] <= 0) ||
      (m_size[2] <= 0)) { return -2; }

  if (m_verbose >= POMS_VERBOSE_DEBUG) {
    printf("loadJSONString: read size[%i,%i,%i]\n",
        (int)m_size[0], (int)m_size[1], (int)m_size[2]);
    fflush(stdout);
  }

  //--
  // name
  //
  // required
  //

  vp = yajl_tree_get( root_node, name_path, yajl_t_array );
  if (!vp) { return -1; }
  for (i=0; i<vp->u.array.len; i++) {
    _s.clear();
    _s = YAJL_GET_STRING(vp->u.array.values[i]);
    m_tile_name.push_back( _s );
    m_tile_weight.push_back( 1.0 );
    m_tile_group.push_back( 0 );
    m_tile_flat_map.push_back( 0 );
  }
  computeTileCDF();
  m_tile_count = m_tile_name.size();

  if (m_verbose >= POMS_VERBOSE_DEBUG) {
    printf("loadJSONString: m_tile_count:%i\n", (int)m_tile_count);
    fflush(stdout);
  }

  //--
  // rule
  //
  // required
  //

  v0 = yajl_tree_get( root_node, rule_path, yajl_t_array);
  if (!v0) { return -1; }
  for (i=0; i<v0->u.array.len; i++) {
    vp = v0->u.array.values[i];

    _d_list.clear();

    for (j=0; j<vp->u.array.len; j++) {
      _d_list.push_back(YAJL_GET_DOUBLE(vp->u.array.values[j]));
    }

    rule_list.push_back(_d_list);

  }

  if (m_verbose >= POMS_VERBOSE_DEBUG) {
    printf("loadJSONString: read rules (%i)\n", (int)rule_list.size());
    fflush(stdout);
  }

  //--
  // weight
  //

  vp = yajl_tree_get( root_node, weight_path, yajl_t_array );
  if (vp) {
    for (i=0; i<vp->u.array.len; i++) {
      d = YAJL_GET_DOUBLE(vp->u.array.values[i]);
      if (i < m_tile_weight.size()) {
        m_tile_weight[i] = d;
      }
    }
    computeTileCDF();
  }

  if (m_verbose >= POMS_VERBOSE_DEBUG) {
    printf("loadJSONString: read m_tile_weight:(%i)\n", (int)m_tile_weight.size());
    fflush(stdout);
  }

  //--
  // tile group
  //

  vp = yajl_tree_get( root_node, tile_group_path, yajl_t_array );
  if (vp) {
    for (i=0; i<vp->u.array.len; i++) {
      d = YAJL_GET_INTEGER(vp->u.array.values[i]);
      if (i < m_tile_group.size()) {
        m_tile_group[i] = d;
      }
    }
    computeTileCDF();
  }

  //--
  // tile flat map
  //

  vp = yajl_tree_get( root_node, tile_flat_map_path, yajl_t_array );
  if (vp) {
    for (i=0; i<vp->u.array.len; i++) {
      d = YAJL_GET_INTEGER(vp->u.array.values[i]);
      if (i < m_tile_flat_map.size()) {
        m_tile_flat_map[i] = d;
      }
    }
  }

  //--
  // seed
  //

  vp = yajl_tree_get( root_node, seed_path, yajl_t_number );
  if (vp) {
    if (m_seed <= 0) {
      m_seed = YAJL_GET_INTEGER( vp );
    }
  }

  //--
  // blockSize
  //

  vp = yajl_tree_get( root_node, blockSize_path, yajl_t_array );
  if (vp) {
    for (i=0; i<vp->u.array.len; i++) {
      ival = YAJL_GET_INTEGER(vp->u.array.values[i]);
      if ((i < 3) &&
          (ival > 0) &&
          (m_block_size[i] <= 0)) {
        m_block_size[i] = ival;
      }
    }
  }


  //--
  // softenSize
  //

  vp = yajl_tree_get( root_node, softenSize_path, yajl_t_array );
  if (vp) {
    for (i=0; i<vp->u.array.len; i++) {
      ival = YAJL_GET_INTEGER(vp->u.array.values[i]);
      if ((i < 3) &&
          (ival > 0) &&
          (m_soften_size[i] <= 0)) {
        m_soften_size[i] = ival;
      }
    }
  }


  //--
  // quiltSize
  //
  vp = yajl_tree_get( root_node, quiltSize_path, yajl_t_array );
  if (vp) {
    for (i=0; i<vp->u.array.len; i++) {
      ival = YAJL_GET_INTEGER(vp->u.array.values[i]);
      if ((i < 3) &&
          (ival > 0) &&
          (m_quilt_size[i] <= 0)) {
        m_quilt_size[i] = ival;
      }
    }
  }

  v0 = yajl_tree_get( root_node, quiltPatch_path, yajl_t_array );
  if (v0) {
    _load_quilt_patch_json(*this, v0);
  }




  //--
  // constraint
  //

  v0 = yajl_tree_get( root_node, constraint_path, yajl_t_array );
  if (v0) {
    _load_constraint_json(*this, v0);
  }

  if (m_verbose >= POMS_VERBOSE_DEBUG) {
    printf("loadJSONString: read constraint\n");
    fflush(stdout);
  }


  //--
  // boundaryCondition
  //

  v0 = yajl_tree_get( root_node, boundaryCondition_path, yajl_t_object );
  if (v0) {
    _load_boundary_condition_json(*this, v0);
  }


  //--
  // objMap
  //

  v0 = yajl_tree_get( root_node, objMap_path, yajl_t_array );
  if (v0) {
    for (i=0; i<v0->u.array.len; i++) {
      //vp = v0->u.array.values[i];

      _s.clear();
      _s = YAJL_GET_STRING(v0->u.array.values[i]);
      m_objMap.push_back( _s );
    }
  }

  _parse_tileset_info( m_tileset_ctx, root_node );
  _parse_tileset_info( m_flat_tileset_ctx, root_node );

  //--

  yajl_tree_free(root_node);

  //----
  //----
  //----

  if (m_verbose >= POMS_VERBOSE_DEBUG) {
    printf("loadJSONString: adding rules (m_tile_count:%i, rule size:%i)\n",
        (int)m_tile_count, 6*m_tile_count*m_tile_count);
    fflush(stdout);
  }

  // tile rule only needs one bit, so we can save space on
  // the storage.
  // Should we not care about space or want weighted tile rules,
  // we'll have to reallocate and change some of the access/write
  // functions
  //
  m_tile_rule.clear();
  //m_tile_rule.resize( 6*m_tile_count*m_tile_count, 0.0 );
  //m_tile_rule.resize( 6*m_tile_count*m_tile_count, 0 );
  m_tile_rule.resize( 6*(m_tile_count*m_tile_count + 8)/8, 0 );

  m_tileAdj.resize( 6*m_tile_count );

  // rule format is:
  // src, dst, idir, val
  //
  for (i=0; i<rule_list.size(); i++) {
    if (rule_list[i].size() < 4) { continue; }

    src_tile = (int32_t)rule_list[i][0];
    dst_tile = (int32_t)rule_list[i][1];
    idir = (int32_t)rule_list[i][2];
    d = (int32_t)rule_list[i][3];

    setF( src_tile, dst_tile, idir, d );
  }

  // m_tileAdj holds list of tile adjacency per
  // tile.
  //
  for (src_tile=0; src_tile<m_tile_count; src_tile++) {
    m_tileAdj[src_tile].clear();
    for (idir=0; idir<6; idir++) {
      for (dst_tile=0; dst_tile<m_tile_count; dst_tile++) {
        if (F(src_tile, dst_tile, idir) > m_zero) {
          m_tileAdj[(idir*m_tile_count) + src_tile].push_back(dst_tile);
        }
      }
    }
  }

  _ecount = (double)rule_list.size();
  _tcount = (double)(m_tile_count*m_tile_count*6);

  renew();

  return 0;
}

// update constraints (actual) from implicit values
// must be called after size, cell count and tile count
// hve been updated.
//

static int32_t _i32clamp(int32_t v, int32_t l, int32_t u) {
  if (v<l) { return l; }
  if (u<v) { return u; }
  return v;
}

int POMS::refreshConstraints(void) {
  size_t idx,
         lu, xyz;
  int32_t val;

  for (idx=0; idx<m_constraint.size(); idx++) {

    for (lu=0; lu<2; lu++) {

      val = m_constraint[idx].implicit_tile[lu];
      if (lu==0) {
        if (val<0) { val += m_tile_count; }
      } else {
        if (val<=0) { val += m_tile_count; }
      }
      m_constraint[idx].tile_range[lu] = _i32clamp(val, 0, m_tile_count);

      for (xyz=0; xyz<3; xyz++) {
        val = m_constraint[idx].implicit_size[xyz][lu];
        if (lu==0) {
          if (val<0) {
            if ((m_constraint[idx].type == '+') ||
                (m_constraint[idx].type == '-') ||
                (m_constraint[idx].type == '=') ||
                (m_constraint[idx].type == '!')) {
              val += m_quilt_size[xyz];
            }
            else {
              val += m_size[xyz];
            }
          }
        } else {
          if (val<=0) {
            if ((m_constraint[idx].type == '+') ||
                (m_constraint[idx].type == '-') ||
                (m_constraint[idx].type == '=') ||
                (m_constraint[idx].type == '!')) {
              val += m_quilt_size[xyz];
            }
            else {
              val += m_size[xyz];
            }
          }
        }

        if ((m_constraint[idx].type == '+') ||
            (m_constraint[idx].type == '-') ||
            (m_constraint[idx].type == '=') ||
            (m_constraint[idx].type == '!')) {
          m_constraint[idx].size_range[xyz][lu] = _i32clamp(val, 0, m_quilt_size[xyz]);
        }
        else {
          m_constraint[idx].size_range[xyz][lu] = _i32clamp(val, 0, m_size[xyz]);
        }

      }
    }

  }

  return 0;
}

int POMS::loadPOMS(POMS &src_poms) {
  int i;
  int32_t i32;

  if (m_size[0] < 0) { m_size[0] = src_poms.m_size[0]; }
  if (m_size[1] < 0) { m_size[1] = src_poms.m_size[1]; }
  if (m_size[2] < 0) { m_size[2] = src_poms.m_size[2]; }

  if ((m_size[0] <= 0) ||
      (m_size[1] <= 0) ||
      (m_size[2] <= 0)) {
    return -1;
  }

  m_verbose = src_poms.m_verbose;

  m_entropy_rand_exponent = src_poms.m_entropy_rand_exponent;
  m_entropy_rand_coefficient = src_poms.m_entropy_rand_coefficient;

  m_retry_max = src_poms.m_retry_max;
  m_retry_count = src_poms.m_retry_count;

  m_tile_choice_policy = src_poms.m_tile_choice_policy;
  m_block_choice_policy = src_poms.m_block_choice_policy;

  m_phase = src_poms.m_phase;
  m_hemp_policy = src_poms.m_hemp_policy;

  m_state = src_poms.m_state;
  m_seq = src_poms.m_seq;

  //---

  memcpy( m_block_size, src_poms.m_block_size, sizeof(int32_t)*3);
  memcpy( m_block, src_poms.m_block, sizeof(int32_t)*3*2);

  memcpy( m_soften_size, src_poms.m_soften_size, sizeof(int32_t)*3);
  memcpy( m_soften_block, src_poms.m_soften_block, sizeof(int32_t)*3*2);

  memcpy( m_soften_pos, src_poms.m_soften_pos, sizeof(int32_t)*3);

  //---

  m_tile_rule   = src_poms.m_tile_rule;

  m_tile_weight = src_poms.m_tile_weight;
  m_tile_group  = src_poms.m_tile_group;
  m_tile_flat_map = src_poms.m_tile_flat_map;
  m_tile_cdf    = src_poms.m_tile_cdf;

  m_tileAdj     = src_poms.m_tileAdj;

  m_tile_name   = src_poms.m_tile_name;

  m_tile_count  = src_poms.m_tile_count;
  m_cell_count  = m_size[0]*m_size[1]*m_size[2];

  m_tileset_ctx = src_poms.m_tileset_ctx;

  m_constraint = src_poms.m_constraint;
  refreshConstraints();

  renew();

  if (m_verbose >= POMS_VERBOSE_DEBUG1) { printDebug(); }

  return 0;
}

