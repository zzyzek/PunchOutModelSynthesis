// LICENSE: CC0
//
// Note this has only been really tested for 2x2 windows.
// The boundary conditions might be a little screwed
// up for other types of windows, so this needs to be revisited.
//

var IMG2TILE_VERSION = "0.7.0";

var DEBUG = false;

if (typeof module !== "undefined") {
  var fs = require("fs");
  var PNG = require("pngjs").PNG;
  var getopt = require("posix-getopt");
}

var VERBOSE_ERROR = -1;
var VERBOSE_NONE = 0;
var VERBOSE_INFO = 1;
var VERBOSE_DEBUG = 2;


//--------

//
//         (+y)
//          2
//  (-x) 1     0 (+x)
//          3
//         (-y)
//
var g_info = {
  "action": "none",

  // supertile offset in png (where supertile
  // is shifted to in PNG).
  //
  "supertile_offset": [ 0, 0 ],

  // dimension of bsae tile
  //
  "stride": [ 8, 8 ],

  // dimension of supertile
  //
  "supertile_dim" : [ 16, 16 ],

  // where base tile is in the supertile,
  // relative to (0,0) upper left in supertile.
  //
  "tile_offset": [0,0],

  // POMS default size
  //
  "size": [0,0],
  "quiltSize": [0,0],

  // rectangular bounds for neighbor overlap
  // pixel matching:
  //
  //   [xmin,max], [ymin,ymax]
  //
  "supertile_neighbor_bound" : [
    [ [8, 16], [0, 16] ],
    [ [0, 8],  [0, 16] ],
    [ [0, 16], [0, 8]  ],
    [ [0, 16], [8, 16] ]
  ],
  "supertile_band" : [-1,-1,-1],

  "opposite_dir": [ 1, 0, 3, 2, 5, 4 ],
  //"oob_pxl": [128, 128, 128, 255],
  "oob_pxl": [128, 0, 128, 255],

  "idir_map" : [ 0, 1, 3, 2, 4, 5 ],

  "verbose" : 0,
  "rule_wrap": false,
  "rule_only_wrap": false,

  "example_fn" : "",
  "supertile_dir" : "",

  "poms_fn": "",

  "tiled_fn" : "",
  "flat_tiled_fn" : "",

  "tileset_fn" : "",
  "flat_tileset_fn" : "",

  "consolidate_tile": false,

  "tile_weight_policy": "uniform"

  //"use_example_img_freq": false

};

//--------

function show_help(fp) {
  fp.write("\nimg2tile, version ");
  show_version(fp);
  fp.write("\n");
  fp.write("  -E,--example <img>            example image to build tileset from\n");
  fp.write("  -T,--tileset <img>            output tileset PNG file\n");
  fp.write("  -t,--flat-tileset <img>       output flat tileset PNG file\n");
  fp.write("  -M,--tiled-fn <json>          Tiled JSON file\n");
  fp.write("  -m,--flat-tiled-fn <json>     flat Tiled JSON file\n");
  fp.write("  -P,--poms-fn <json>           write POMS JSON file\n");
  fp.write("  -s,--stride  <#[,#]>          stride for example image\n");
  fp.write("  -w,--window  <#[,#]>          supertile window\n");
  fp.write("  -D,--size <#[,#]>             size to use in POMS config\n");
  fp.write("  -q,--quiltSize <#[,#]>        quilt size to use in POMS config (defaults to size)\n");
  fp.write("  -O,--supertile-offset <#[,#]> supertile window offset in PNG example image (default 0,0)\n");
  fp.write("  -B,--supertile-band   <#[,#]> supertile overlap band (default 1/2 supertile size in each dim)\n");
  fp.write("  -o,--tile-offset <#[,#]>      tile window offset from origin of supertile (default 0,0)\n");
  fp.write("  -S,--supertile-dir <dir>      supertile directory (debug output)\n");
  fp.write("  -W,--weight-tile <code>       use weight tiles (uniform|image|flat) (default uniform)\n");
  fp.write("  -u,--rule-wrap                wrap around rules (allow 0 tiles)\n");
  fp.write("  -U,--rule-only-wrap           wrap around rules only (remove 0 tiles)\n");
  fp.write("  -V,--verbose                  set verbosity level (default 0)\n");
  fp.write("  -v,--version                  show version\n");
  fp.write("  -h,--help                     help (this scren)\n");
  fp.write("\n");
}

function show_version(fp) {
  fp.write( IMG2TILE_VERSION + "\n")
}

function cli_parse(info) {
  info = ((typeof info === "undefined") ? g_info : info);

  let parser, opt;

  var g_cmd = "";

  let long_opt = [
    "E", ":(example)",
    "T", ":(tileset)",
    "t", ":(flat-tileset)",
    "M", ":(tiled-fn)",
    "m", ":(flat-tiled-fn)",
    "S", ":(supertile-dir)",
    "s", ":(stride)",
    "w", ":(window)",
    "O", ":(supertile-offset)",
    "B", ":(supertile-band)",
    "o", ":(tile-offset)",
    "D", ":(size)",
    "q", ":(quiltSize)",
    "P", ":(poms)",
    "W", ":(weight-tile)",
    "u", "(rule-wrap)",
    "U", "(rule-only-wrap)",
    "C", "(consolidate)",
    "V", ":(verbose)",
    "v", "(version)",
    "h", "(help)"
  ];

  let _s;

  parser = new getopt.BasicParser(long_opt.join(""), process.argv);
  while ((opt =  parser.getopt()) !== undefined) {

    switch(opt.option) {
      case 'h':
        show_help(process.stdout);
        process.exit(0);
        break;
      case 'v':
        show_version(process.stdout);
        process.exit(0);
        break;
      case 'V':
        info.verbose = parseInt(opt.optarg);
        break;

      case 'u':
        info.rule_wrap = true;
        break;
      case 'U':
        info.rule_only_wrap = true;
        info.rule_wrap = true;
        break;

      case 'E':
        info.example_fn = opt.optarg;
        break;
      case 'T':
        info.tileset_fn = opt.optarg;
        break;
      case 't':
        info.flat_tileset_fn = opt.optarg;
        break;
      case 'M':
        info.tiled_fn = opt.optarg;
        break;
      case 'm':
        info.flat_tiled_fn = opt.optarg;
        break;
      case 'P':
        info.poms_fn = opt.optarg;
        break;

      case 's':
        _s = opt.optarg.split(",").map( function(ele) { return parseInt(ele); } );
        if      (_s.length == 1) { info.stride = [ _s[0], _s[0], _s[0] ]; }
        else if (_s.length == 2) { info.stride = [ _s[0], _s[1], 1 ]; }
        else if (_s.length == 3) { info.stride = [ _s[0], _s[1], _s[2] ]; }
        break;

      case 'w':
        _s = opt.optarg.split(",").map( function(ele) { return parseInt(ele); } );
        if      (_s.length == 1) { info.supertile_dim = [ _s[0], _s[0], _s[0] ]; }
        else if (_s.length == 2) { info.supertile_dim = [ _s[0], _s[1], 1 ]; }
        else if (_s.length == 3) { info.supertile_dim = [ _s[0], _s[1], _s[2] ]; }
        break;
      case 'O':
        _s = opt.optarg.split(",").map( function(ele) { return parseInt(ele); } );
        if      (_s.length == 1) { info.supertile_offset = [ _s[0], _s[0], _s[0] ]; }
        else if (_s.length == 2) { info.supertile_offset = [ _s[0], _s[1], 1 ]; }
        else if (_s.length == 3) { info.supertile_offset = [ _s[0], _s[1], _s[2] ]; }
        break;
      case 'B':
        _s = opt.optarg.split(",").map( function(ele) { return parseInt(ele); } );
        if      (_s.length == 1) { info.supertile_band = [ _s[0], _s[0], _s[0] ]; }
        else if (_s.length == 2) { info.supertile_band = [ _s[0], _s[1], 1 ]; }
        else if (_s.length == 3) { info.supertile_band = [ _s[0], _s[1], _s[2] ]; }
        break;

      case 'o':
        _s = opt.optarg.split(",").map( function(ele) { return parseInt(ele); } );
        if      (_s.length == 1) { info.tile_offset = [ _s[0], _s[0], _s[0] ]; }
        else if (_s.length == 2) { info.tile_offset = [ _s[0], _s[1], 1 ]; }
        else if (_s.length == 3) { info.tile_offset = [ _s[0], _s[1], _s[2] ]; }
        break;

      case 'W':
        info.tile_weight_policy = opt.optarg;
        break;
      case 'C':
        info.consolidate_tile = true;
        break;

      case 'D':
        _s = opt.optarg.split(",").map( function(ele) { return parseInt(ele); } );
        if      (_s.length == 1) { info.size = [ _s[0], _s[0], _s[0] ]; }
        else if (_s.length == 2) { info.size = [ _s[0], _s[1], 1 ]; }
        else if (_s.length == 3) { info.size = [ _s[0], _s[1], _s[2] ]; }
        break;

      case 'q':
        _s = opt.optarg.split(",").map( function(ele) { return parseInt(ele); } );
        if      (_s.length == 1) { info.quiltSize = [ _s[0], _s[0], _s[0] ]; }
        else if (_s.length == 2) { info.quiltSize = [ _s[0], _s[1], 1 ]; }
        else if (_s.length == 3) { info.quiltSize = [ _s[0], _s[1], _s[2] ]; }
        break;

      case 'S':
        info.supertile_dir = opt.optarg;
        break;
      default:
        show_help(process.stderr);
        process.exit(-1);
        break;
    }
  }

  if (parser.optind() < process.argv.length) {
    g_cmd = process.argv[parser.optind()];
  }

  if (info.example_fn.length == 0) {
    process.stderr.write("Provide example file (-E)\n");
    show_help(process.stderr);
    process.exit(-2);
  }

  info.img = PNG.sync.read( fs.readFileSync(info.example_fn) );

  if (info.quiltSize.length < 3) { info.quiltSize.push(1); }

  info.quiltSize[0] = ((info.quiltSize[0] <= 0) ? info.size[0] : info.quiltSize[0]);
  info.quiltSize[1] = ((info.quiltSize[1] <= 0) ? info.size[1] : info.quiltSize[1]);
  info.quiltSize[2] = ((info.quiltSize[2] <= 0) ? info.size[2] : info.quiltSize[2]);

  return info;
}

//--------

function _log() {

  let lvl = arguments[0];
  let _rest = Array.prototype.slice.call(arguments,1);

  if (lvl >= g_info.verbose) {
    console.log.apply(null, _rest);
  }
}

function _hxs2(x) {
  let lookup = [ "0", "1", "2", "3",
                 "4", "5", "6", "7",
                 "8", "9", "a", "b",
                 "c", "d", "e", "f" ];

  let xx = Math.floor(Math.abs(x));

  if (xx < 16) {
    return "0" + lookup[xx];
  }

  let _dig = [];


  while (xx > 0) {
    let lsd = (xx % 16);
    _dig.push( lookup[lsd] );
    xx = Math.floor( xx / 16 );
  }

  return _dig.reverse().join("");
}

function build_null_tile(info) {
  let img = info.img;
  let w = img.width;
  let h = img.height;
  //let stride = [info.stride, g_info.stride];
  let stride = info.stride;

  let st_w = info.supertile_dim[0];
  let st_h = info.supertile_dim[1];
  let st_ox = info.supertile_offset[0];
  let st_oy = info.supertile_offset[1];

  let oob_pxl = info.oob_pxl;
  let nei_bnd = info.supertile_neighbor_bound;

  let supertile_buf = [];
  let supertile_buf_str = [];
  let key_buf = [];

  let supertile_nei_buf = [ [], [], [], [] ];
  let supertile_nei_str = [ [], [], [], [] ];

  let nei_key_map = {};

  let supertile_count = info["supertile_count"];
  let supertile_key = info["supertile_key"];
  let supertile_lib = info["supertile_lib"];

  for (let y=0; y<st_h; y++) {

    for (let x=0; x<st_w; x++) {

      if (x>0) { key_buf.push(","); }
      let idx = ((y*w) + x)*4;

      let _pxl;
      let nei_pfx_char = [ "", "", "", "" ];

      // add indexes for neighbor pixel ribbons
      //
      let nei_list = [];
      for (let ii=0; ii<nei_bnd.length; ii++) {

        if ((x <  nei_bnd[ii][0][0]) ||
            (x >= nei_bnd[ii][0][1]) ||
            (y <  nei_bnd[ii][1][0]) ||
            (y >= nei_bnd[ii][1][1])) { continue; }
        nei_list.push(ii);

        // we want ';' at row breaks and ',' between
        // string representation of pixels
        //
        if ((x == nei_bnd[ii][0][0]) &&
            (y >  nei_bnd[ii][1][0])) {
          nei_pfx_char[ii] = ";\n";
        }
        else if (x > nei_bnd[ii][0][0]) { nei_pfx_char[ii] = ","; }

      }

      _pxl = [ oob_pxl[0], oob_pxl[1], oob_pxl[2], oob_pxl[3] ];

      for (let ii=0; ii<4; ii++) {
        supertile_buf.push(_pxl[ii]);
        supertile_buf_str.push( ((ii>0) ? ":" : "" ) + _hxs2(_pxl[ii]) );
        for (let jj=0; jj<nei_list.length; jj++) {
          let nei_idx = nei_list[jj];
          //if (ii==0) { supertile_nei_str[nei_idx].push( nei_pfx_char[ii] ); }
          if (ii==0) { supertile_nei_str[nei_idx].push( nei_pfx_char[nei_idx] ); }
          supertile_nei_buf[nei_idx].push( _pxl[ii] );
          supertile_nei_str[nei_idx].push( ((ii>0) ? ":" : "" ) + _hxs2(_pxl[ii]) );
        }

      }

    }
  }

  let st_key = supertile_buf_str.join("");

  let nei_key_str = [];
  for (let ii=0; ii<supertile_nei_str.length; ii++) {
    nei_key_str.push( supertile_nei_str[ii].join("") );
  }

  supertile_key.push(st_key);
  supertile_lib[ st_key ] = {
    "nei_buf": supertile_nei_buf,
    "nei_key": nei_key_str,
    "data": supertile_buf,
    "boundaryInfo": {},
    "flat_id": 0,
    "id": 0,
    "freq": 1
  };

  for (let ii=0; ii<nei_bnd.length; ii++) {
    let nei_key = supertile_nei_str[ii].join("");
  }

  info.supertile_count++;

}

// 'super tiles' are the tiles used to create the neighboring
// constraints.
// The base tile should reside somewhere inside the super tile,
// with the edges in each dimension used to find the overlap
// to other super tiles. The overlap determines which super
// tile is neighbor to every other super tile.
// The tileset created will have the same number as unique
// super tiles found.
//
function build_super_tile_lib(info) {
  let img = info.img;
  let w = img.width;
  let h = img.height;
  let n = img.data.length;
  let data = img.data;
  let stride = info.stride;

  let st_w = info.supertile_dim[0];
  let st_h = info.supertile_dim[1];
  let st_ox = info.supertile_offset[0];
  let st_oy = info.supertile_offset[1];

  let idir_map = info.idir_map;

  let oob_pxl = info.oob_pxl;

  // we force a 'null' tile with id of 0
  //
  let supertile_count = 0;
  let supertile_key = [];
  let supertile_lib = {};

  info["supertile_count"] = supertile_count;
  info["supertile_key"] = supertile_key;
  info["supertile_lib"] = supertile_lib;

  let simpletile_lib = info.simpletile_lib;

  let nei_bnd = info.supertile_neighbor_bound;

  let map_w = Math.floor(w / stride[0]);
  let map_h = Math.floor(h / stride[1]);
  let map_array = Array(map_w*map_h).fill(-1);
  let flat_map_array = Array(map_w*map_h).fill(-1);

  build_null_tile(info);
  supertile_count = info["supertile_count"];

  // We build super tiles from the underlying example
  // image pixels, taking a window, with offset, and
  // converting those pixels to a string that's used
  // as a key into the `supertile_lib` data structure.
  // Neighbor strips of pixels are used to later find
  // adjacency.

  let n_data = w*h*4;
  for (let y=0; y<h; y+=stride[1]) {

    for (let x=0; x<w; x+=stride[0]) {

      let boundaryInfo = {};
      if (y==0) { boundaryInfo[ "y-" ] = 1; }
      //if ((y+stride[1]) >= h) { boundaryInfo.push( "y+" ); }

      if (x==0) { boundaryInfo[ "x-" ] = 1; }
      //if ((x+stride[0])>=w) { boundaryInfo.push( "x+" ); }

      let idx = ((w*y) + x)*4;

      let _sx = (x + st_ox);
      let _sy = (y + st_oy);

      let _nx = st_w;
      let _ny = st_h;

      let supertile_buf = [];
      let supertile_buf_str = [];

      let supertile_buf_wrap = [];
      let supertile_buf_str_wrap = [];
      //let supertile_wrap = false;

      let key_buf = [];

      let supertile_nei_buf = [ [], [], [], [] ];
      let supertile_nei_str = [ [], [], [], [] ];

      let supertile_nei_buf_wrap = [ [], [], [], [] ];
      let supertile_nei_str_wrap = [ [], [], [], [] ];

      let nei_key_map = {};

      let simpletile_key_str = [];

      // (tx,ty) position in img, including negative pos.
      // (rx,ry) are relative within supertile, 0 start.
      //
      for (let ty=_sy; ty<(_sy+_ny); ty++) {

        if (ty>_sy) { key_buf.push(";"); }
        for (let tx=_sx; tx<(_sx+_nx); tx++) {

          if (tx>_sx) { key_buf.push(","); }
          let idx = ((ty*w) + tx)*4;

          let rx = tx - _sx;
          let ry = ty - _sy;

          let _pxl;
          let _pxl_wrap;

          let nei_pfx_char = [ "", "", "", "" ];

          // add indexes for neighbor pixel ribbons
          //
          let nei_list = [];
          for (let ii=0; ii<nei_bnd.length; ii++) {

            if ((rx <  nei_bnd[ii][0][0]) ||
                (rx >= nei_bnd[ii][0][1]) ||
                (ry <  nei_bnd[ii][1][0]) ||
                (ry >= nei_bnd[ii][1][1])) { continue; }
            nei_list.push(ii);

            // we want ';' at row breaks and ',' between
            // string representation of pixels
            //
            if ((rx == nei_bnd[ii][0][0]) &&
                (ry > nei_bnd[ii][1][0])) {
              nei_pfx_char[ii] = ";\n";
            }
            else if (rx > nei_bnd[ii][0][0]) { nei_pfx_char[ii] = ","; }
          }

          if ( (tx<0) || (tx>=w) ||
               (ty<0) || (ty>=h) ) {

            //if (info.rule_wrap) {
              let _idx = ((((ty+h)%h)*w) + ((tx+w)%w))*4;
              _pxl_wrap = [ data[_idx+0], data[_idx+1], data[_idx+2], data[_idx+3] ];
            //}

            //else {
              _pxl = [ oob_pxl[0], oob_pxl[1], oob_pxl[2], oob_pxl[3] ];
            //}

          }
          else {
            _pxl      = [ data[idx+0], data[idx+1], data[idx+2], data[idx+3] ];
            _pxl_wrap = [ data[idx+0], data[idx+1], data[idx+2], data[idx+3] ];
          }

          // add each pixel to supertile_buf and supertile_buf_wrap
          //
          for (let ii=0; ii<4; ii++) {

            let _pfx = "";

            if (ii==0) {
              if ((ty>_sy) && (tx==_sx)) { _pfx = ";"; }
              if (tx>_sx) { _pfx += ","; }
            }

            supertile_buf.push(_pxl[ii]);
            supertile_buf_str.push( _pfx + ((ii>0) ? ":" : "" ) + _hxs2(_pxl[ii]) );

            supertile_buf_wrap.push(_pxl_wrap[ii]);
            supertile_buf_str_wrap.push( _pfx + ((ii>0) ? ":" : "" ) + _hxs2(_pxl_wrap[ii]) );

            for (let jj=0; jj<nei_list.length; jj++) {
              let nei_idx = nei_list[jj];
              if (ii==0) {
                supertile_nei_str[nei_idx].push( nei_pfx_char[nei_idx] );
                supertile_nei_str_wrap[nei_idx].push( nei_pfx_char[nei_idx] );
              }
              supertile_nei_buf[nei_idx].push( _pxl[ii] );
              supertile_nei_str[nei_idx].push( ((ii>0) ? ":" : "" ) + _hxs2(_pxl[ii]) );

              supertile_nei_buf_wrap[nei_idx].push( _pxl_wrap[ii] );
              supertile_nei_str_wrap[nei_idx].push( ((ii>0) ? ":" : "" ) + _hxs2(_pxl_wrap[ii]) );
            }

          }

          // calculate simple tile key
          //
          if ((rx < stride[0]) && (ry < stride[1])) {
            let _pfx = "";
            if ((rx==0) && (ry > 0)) { _pfx += ";"; }
            if ((rx>0)) { _pfx += ","; }
            simpletile_key_str.push( _pfx + _hxs2(_pxl[0]) + ":" + _hxs2(_pxl[1]) + ":" + _hxs2(_pxl[2]) + ":" + _hxs2(_pxl[3]) );
          }

        }
      }

      let st_key = supertile_buf_str.join("");
      let st_key_wrap = supertile_buf_str_wrap.join("");

      let simple_key = simpletile_key_str.join("");

      if (!(st_key in supertile_lib)) {

        let nei_key_str = [];
        for (let ii=0; ii<supertile_nei_str.length; ii++) {
          nei_key_str.push( supertile_nei_str[ii].join("") );
        }

        let flat_tile_id = -1;
        if (simple_key in simpletile_lib) {
          flat_tile_id = simpletile_lib[simple_key].id;
        }

        supertile_key.push(st_key);
        supertile_lib[ st_key ] = {
          "nei_buf": supertile_nei_buf,
          "nei_key": nei_key_str,
          "data": supertile_buf,
          "id": supertile_count,
          "flat_id": flat_tile_id,
          "boundaryInfo": boundaryInfo,
          "freq": 1
        };
        supertile_count++;


        //for (let ii=0; ii<nei_bnd.length; ii++) { let nei_key = supertile_nei_str[ii].join(""); }
      }
      else {
        supertile_lib[ st_key ].freq++;

        // tile might have initially appeared in the middle of the example
        // image but later appeared at the edge of the map, so update
        // our record to add boundary information, if present.
        //
        for (let bi_key in boundaryInfo) {
          supertile_lib[ st_key ].boundaryInfo[ bi_key ] = boundaryInfo[ bi_key ];
        }
      }

      //---

      if (info.rule_wrap) {
        if (!(st_key_wrap in supertile_lib)) {

          let nei_key_str_wrap = [];
          for (let ii=0; ii<supertile_nei_str_wrap.length; ii++) {
            nei_key_str_wrap.push( supertile_nei_str_wrap[ii].join("") );
          }

          let flat_tile_id = simpletile_lib[simple_key].id;

          supertile_key.push(st_key_wrap);
          supertile_lib[ st_key_wrap ] = {
            "nei_buf": supertile_nei_buf_wrap,
            "nei_key": nei_key_str_wrap,
            "data": supertile_buf_wrap,
            "id": supertile_count,
            "flat_id": flat_tile_id,
            "boundaryInfo": boundaryInfo,
            "freq": 1
          };
          supertile_count++;

        }
        else {
          supertile_lib[ st_key_wrap ].freq++;

          // tile might have initially appeared in the middle of the example
          // image but later appeared at the edge of the map, so update
          // our record to add boundary information, if present.
          //
          for (let bi_key in boundaryInfo) {
            supertile_lib[ st_key_wrap ].boundaryInfo[ bi_key ] = boundaryInfo[ bi_key ];
          }
        }
      }

      // create map

      let _m_x = Math.floor(x / stride[0]);
      let _m_y = Math.floor(y / stride[1]);
      let _m_id = supertile_lib[st_key].id;
      map_array[ _m_y*map_w + _m_x ] = _m_id;

      let _flat_id = supertile_lib[st_key].flat_id;
      flat_map_array[ _m_y*map_w + _m_x ] = _flat_id;


      //---

      if (info.supertile_dir.length > 0) {

        let _odir = info.supertile_dir;

        let st_png = new PNG({ "width": st_w, "height": st_h });
        let _opt = {};
        for (let ty=0; ty<st_h; ty++) {
          for (let tx=0; tx<st_w; tx++) {
            let idx = ((ty*st_w) + tx)*4;
            st_png.data[idx+0] = supertile_buf[idx+0];
            st_png.data[idx+1] = supertile_buf[idx+1];
            st_png.data[idx+2] = supertile_buf[idx+2];
            st_png.data[idx+3] = supertile_buf[idx+3];
          }
        }
        let _buf = PNG.sync.write(st_png, _opt);
        fs.writeFileSync( _odir + "/" + supertile_lib[st_key].id.toString() + ".png", _buf);

        if (info.rule_wrap) {
          let st_png = new PNG({ "width": st_w, "height": st_h });
          let _opt = {};
          for (let ty=0; ty<st_h; ty++) {
            for (let tx=0; tx<st_w; tx++) {
              let idx = ((ty*st_w) + tx)*4;
              st_png.data[idx+0] = supertile_buf_wrap[idx+0];
              st_png.data[idx+1] = supertile_buf_wrap[idx+1];
              st_png.data[idx+2] = supertile_buf_wrap[idx+2];
              st_png.data[idx+3] = supertile_buf_wrap[idx+3];
            }
          }
          let _buf = PNG.sync.write(st_png, _opt);
          fs.writeFileSync( _odir + "/" + supertile_lib[st_key_wrap].id.toString() + ".png", _buf);
        }

      }

    }

  }

  let name_list = [];
  let adj_list = [];

  for (let st_idx=0; st_idx < supertile_count; st_idx++) {
    name_list.push( st_idx.toString() );
  }

  // construct adjacency matrix
  //
  for (let st_a_idx=0; st_a_idx < supertile_count; st_a_idx++) {

    let st_a_key  = supertile_key[st_a_idx];
    let st_a_info = supertile_lib[st_a_key];

    if ("x-" in st_a_info.boundaryInfo) {
      adj_list.push( [ st_a_idx, 0, idir_map[1], 1 ] );
      adj_list.push( [ 0, st_a_idx, idir_map[0], 1 ] );
    }
    if ("y-" in st_a_info.boundaryInfo) {
      adj_list.push( [ st_a_idx, 0, 3, 1 ] );
      adj_list.push( [ 0, st_a_idx, 2, 1 ] );
    }

    // add z boundary information
    //
    adj_list.push( [ st_a_idx, 0, idir_map[4], 1 ] );
    adj_list.push( [ st_a_idx, 0, idir_map[5], 1 ] );
    adj_list.push( [ 0, st_a_idx, idir_map[5], 1 ] );
    adj_list.push( [ 0, st_a_idx, idir_map[4], 1 ] );

    for (let st_b_idx=0; st_b_idx < supertile_count; st_b_idx++) {

      let st_b_key  = supertile_key[st_b_idx];
      let st_b_info = supertile_lib[st_b_key];

      for (let dir_idx=0; dir_idx<nei_bnd.length; dir_idx++) {

        let oppo_dir_idx = info.opposite_dir[dir_idx];

        let dir_code = [ ">", "<", "^", "v" ];
        let _d0 = dir_code[dir_idx];
        let _d1 = dir_code[oppo_dir_idx];

        if ( st_a_info.nei_key[dir_idx] == st_b_info.nei_key[oppo_dir_idx] ) {
          adj_list.push( [ st_a_idx, st_b_idx, idir_map[dir_idx], 1 ] );

          if (DEBUG) {
            console.log("a[" + _d0  +  "]:", st_a_idx, "-> b[" + _d1 + "]:", st_b_idx);
          }
        }

      }

    }
  }

  if (DEBUG) {
    console.log("\n\n");
    console.log("#NAME");
    console.log("#tile_id,tile_name");
    for (let ii=0; ii<name_list.length; ii++) {
      console.log( name_list[ii].join(",") );
    }


    console.log("\n\n");
    console.log("#RULE");
    console.log("#atile,btile,diridx,weight");
    for (let ii=0; ii<adj_list.length; ii++) {
      console.log( adj_list[ii].join(",") );
    }
  }

  // this option forces no 0 tile on 2d boundary,
  // so go through and take them out of the
  // adj_list (rule_list)
  //
  if (info.rule_only_wrap) {

    let _tmp_rule = [];
    for (let ii=0; ii<adj_list.length; ii++) {
      let idir = adj_list[ii][2];

      // we only want rules in 2d so skip over
      // 4,5/z+,z- directions
      //
      if (idir > 4) {
        _tmp_rule.push( adj_list[ii] );
        continue;
      }

      let atile = adj_list[ii][0];
      let btile = adj_list[ii][1];
      if ((atile == 0) || (btile == 0)) { continue; }

      _tmp_rule.push( adj_list[ii] );
    }

    adj_list = _tmp_rule;
  }


  info["supertile_key"] = supertile_key;
  info["supertile_lib"] = supertile_lib;
  info["supertile_count"] = supertile_count;
  info["tile_rule"] = adj_list;
  info["tile_name"] = name_list;

  info["map_w"] = map_w;
  info["map_h"] = map_h;
  info["map_array"] = map_array;
  info["flat_map_array"] = flat_map_array;

  //DEBUG
  if (info.verbose >= VERBOSE_DEBUG) {
    console.log("------", map_w, map_h);
    for (let _y=0; _y<map_h; _y++) {
      let row_a = [];
      for (let _x=0; _x<map_w; _x++) {
        row_a.push(map_array[ _y*map_w + _x ]);
      }
      console.log(row_a.join(","));
    }
    console.log("------");
  }

}

// Build single tile library.
// Each tile is strid[0] x stride[1] of pixels.
// `info.simpletile_lib` has as the key the base tile's
// string representation along with it's pixel
// data, base tile id and frequency.
//
function build_simple_tile_lib(info) {
  let img = info.img;

  let w = img.width;
  let h = img.height;
  let n = img.data.length;

  let data = img.data;

  let simpletile_lib = {};
  let simpletile_key = [ [] ];
  let tile_count = 0;

  let stride = info.stride;

  for (let y=0; y<h; y+=stride[1]) {
    for (let x=0; x<w; x+=stride[0]) {

      let key_buf = [];
      let simpletile_buf = [];
      for (let jj=0; jj<stride[1]; jj++) {
        if (jj>0) { key_buf.push(";"); }
        for (let ii=0; ii<stride[0]; ii++) {

          if (ii>0) { key_buf.push(","); }

          let idx = ((w*(y+jj) + (x+ii)))*4;

          key_buf.push( _hxs2( data[idx+0] ) + ":" );
          key_buf.push( _hxs2( data[idx+1] ) + ":" );
          key_buf.push( _hxs2( data[idx+2] ) + ":" );
          key_buf.push( _hxs2( data[idx+3] ) );

          simpletile_buf.push( data[idx+0] );
          simpletile_buf.push( data[idx+1] );
          simpletile_buf.push( data[idx+2] );
          simpletile_buf.push( data[idx+3] );
        }
      }

      let key_str = key_buf.join("");
      if (!(key_str in simpletile_lib)) {
        tile_count++;
        simpletile_lib[key_str] = {
          "key": key_str,
          "data": simpletile_buf,
          "id": tile_count,
          "freq": 1
        };
        simpletile_key.push( key_str );
      }
      else {
        simpletile_lib[key_str].freq++;
      }

    }
  }

  info["simpletile_key"] = simpletile_key;
  info["simpletile_lib"] = simpletile_lib;
  info["simpletile_count"] = tile_count;

  if (info.size[0] == 0) { info.size[0] = Math.floor(w/stride[0]); }
  if (info.size[1] == 0) { info.size[1] = Math.floor(h/stride[1]); }
}

// create simple (flat) tile set from previously
// processed simpletile_lib
//
function build_simple_tile_set(info) {
  let simpletile_key  = info.simpletile_key;
  let simpletile_lib  = info.simpletile_lib;
  let tile_count      = info.simpletile_count;

  let oob_pxl = info.oob_pxl;

  let stride = info.stride;
  let N = Math.ceil( Math.sqrt( tile_count ) );

  let _png = newPNG(stride[0]*N, stride[1]*N);

  let data = _png.data;

  let pxl_w = stride[0]*N;
  let pxl_h = stride[1]*N;

  let tile_id = 1;
  for (let y=0; y<N; y++) {
    for (let x=0; x<N; x++) {

      if (tile_id >= simpletile_key.length) {

        for (let dy=0; dy<stride[1]; dy++) {
          for (let dx=0; dx<stride[0]; dx++) {

            let png_idx = (((y*stride[1]) + dy)*pxl_w + ((x*stride[0])+dx))*4;

            data[png_idx+0] = oob_pxl[0];
            data[png_idx+1] = oob_pxl[1];
            data[png_idx+2] = oob_pxl[2];
            data[png_idx+3] = oob_pxl[3];
          }
        }

        continue;

      }

      let st_key    = simpletile_key[tile_id];

      let tile_dat  = simpletile_lib[ st_key ].data;

      let st_sx = 0;
      let st_sy = 0;

      let st_w = info.stride[0];
      let st_h = info.stride[1];

      for (let dy=0; dy<stride[1]; dy++) {
        for (let dx=0; dx<stride[0]; dx++) {

          let png_idx = (((y*stride[1]) + dy)*pxl_w + ((x*stride[0])+dx))*4;
          let st_idx = ( ((st_sy + dy)*st_w) + (st_sx + dx) )*4;

          data[png_idx+0] = tile_dat[st_idx+0];
          data[png_idx+1] = tile_dat[st_idx+1];
          data[png_idx+2] = tile_dat[st_idx+2];
          data[png_idx+3] = tile_dat[st_idx+3];
        }
      }

      tile_id++;

    }
  }

  info["simple_tileset_png"] = _png;
  info["simple_tileset_size"] = [ pxl_w, pxl_h ];

}

function write_png_data(data, out_fn) {
  let _opt = {};
  let _buf = PNG.sync.write(data, _opt);
  fs.writeFileSync( out_fn, _buf );
}

function newPNG(w,h) {
  if (typeof module !== "undefined") {
    return  new PNG({
      "width": w,
      "height": h
    });
  }
  else {
    let _canv = document.createElement("canvas");
    _canv.width = w;
    _canv.height = h;
    let _ctx = _canv.getContext("2d", {"alpha":false});

		_ctx.webkitImageSmoothingEnabled = false;
		_ctx.imageSmoothingEnabled = false;
		_ctx.mozImageSmoothingEnabled = false;
		_ctx.oImageSmoothingEnabled = false;

    return _ctx.getImageData(0, 0, _canv.width, _canv.height);
  }
  return undefined;
}

function build_tile_set(info) {
  let tile_count = info.supertile_count;
  let oob_pxl = info.oob_pxl;

  let stride = info.stride;
  let N = Math.ceil( Math.sqrt( tile_count ) );

  let _png = newPNG(stride[0]*N, stride[1]*N);

  info["tileset_width"] = N;
  info["tileset_height"] = N;

  info["tileset_pixel_width"] = N*stride[0];
  info["tileset_pixel_height"] = N*stride[1];

  let data = _png.data;

  let pxl_w = stride[0]*N;
  let pxl_h = stride[1]*N;

  let tile_id = 1;
  for (let y=0; y<N; y++) {
    for (let x=0; x<N; x++, tile_id++) {

      // fill with out of bound (OOB) pixel if
      // we've gone past the last tile ID
      //
      if (tile_id >= tile_count) {
        for (let dy=0; dy<stride[1]; dy++) {
          for (let dx=0; dx<stride[0]; dx++) {
            let png_idx = (((y*stride[1])+dy)*pxl_w + ((x*stride[0])+dx))*4;
            data[png_idx+0] = oob_pxl[0];
            data[png_idx+1] = oob_pxl[1];
            data[png_idx+2] = oob_pxl[2];
            data[png_idx+3] = oob_pxl[3];
          }
        }

        continue;
      }

      let st_key = info.supertile_key[tile_id];
      let tile_dat = info.supertile_lib[ st_key ].data;

      st_sx = info.tile_offset[0];
      st_sy = info.tile_offset[1];

      let st_w = info.supertile_dim[0];
      let st_h = info.supertile_dim[1];

      for (let dy=0; dy<stride[1]; dy++) {
        for (let dx=0; dx<stride[0]; dx++) {

          let png_idx = (((y*stride[1])+dy)*pxl_w + ((x*stride[0])+dx))*4;
          let st_idx = ( ((st_sy + dy)*st_w) + (st_sx + dx) )*4;

          data[png_idx+0] = tile_dat[st_idx+0];
          data[png_idx+1] = tile_dat[st_idx+1];
          data[png_idx+2] = tile_dat[st_idx+2];
          data[png_idx+3] = tile_dat[st_idx+3];

        }
      }

    }
  }

  info["tileset_png"] = _png;
  info["tileset_size"] = [ pxl_w, pxl_h ];
}

function write_tileset_png(info, out_fn) {
  let _opt = {};
  let _buf = PNG.sync.write(info.tileset_png, _opt);
  fs.writeFileSync( out_fn, _buf);
}

function write_flat_tiled_json(info, out_fn) {

  //let stride = [ info.stride, info.stride ];
  let stride = info.stride;

  let template = {
    "backgroundcolor":"#ffffff",
    "height": -1,
    "width": -1,
    "layers": [
      {
        "data": [],
        "width":-1,
        "height":-1,
        "x":0,
        "y":0,
        "name": "main",
        "opacity": 1,
        "type":"tilelayer",
        "visible":true
      }
    ],
    "nextobjectid":1,
    "orientation": "orthogonal",
    "properties":[],
    "renderorder": "right-down",
    "tileheight": -1,
    "tilewidth": -1,
    "tilesets": [{
      "columns": -1,
      "image": "",
      "imageheight": -1, // ****
      "imagewidth": -1, // ****
      "tilecount": -1, // ****
      "tileheight": -1, // ****
      "tilewidth": -1, // ****
      "margin": 0,
      "spacing": 0,
      "name": "tileset",
      "firstgid": 1
    }],
    "version": 1
  };

  template.width = info.map_w;
  template.height = info.map_h;
  template.tileheight = stride[1];
  template.tilewidth = stride[0];

  template.layers[0].data = info.flat_map_array;
  template.layers[0].width = info.map_w;
  template.layers[0].height = info.map_h;

  //template.tilesets[0].image = ".out/tilemap.png";
  template.tilesets[0].image = info.flat_tileset_fn;
  template.tilesets[0].tileheight = stride[1];
  template.tilesets[0].tilewidth = stride[0];
  template.tilesets[0].tilecount = info.simpletile_count-1;
  template.tilesets[0].columns = info.map_w;
  template.tilesets[0].rows = info.map_w;

  let data = JSON.stringify(template, null, 2);
  fs.writeFileSync( out_fn, data );
}



function write_tiled_json(info, out_fn) {

  //let stride = [ info.stride, info.stride ];
  let stride = info.stride;

  let template = {
    "backgroundcolor":"#ffffff",
    "height": -1,
    "width": -1,
    "layers": [
      {
        "data": [],
        "width":-1,
        "height":-1,
        "x":0,
        "y":0,
        "name": "main",
        "opacity": 1,
        "type":"tilelayer",
        "visible":true
      }
    ],
    "nextobjectid":1,
    "orientation": "orthogonal",
    "properties":[],
    "renderorder": "right-down",
    "tileheight": -1,
    "tilewidth": -1,
    "tilesets": [{
      "columns": -1,
      "image": "",
      "imageheight": -1, // ****
      "imagewidth": -1, // ****
      "tilecount": -1, // ****
      "tileheight": -1, // ****
      "tilewidth": -1, // ****
      "margin": 0,
      "spacing": 0,
      "name": "tileset",
      "firstgid": 1
    }],
    "version": 1
  };

  template.width = info.map_w;
  template.height = info.map_h;
  template.tileheight = stride[1];
  template.tilewidth = stride[0];

  template.layers[0].data = info.map_array;
  template.layers[0].width = info.map_w;
  template.layers[0].height = info.map_h;

  //template.tilesets[0].image = ".out/tilemap.png";
  template.tilesets[0].image = info.tileset_fn;
  template.tilesets[0].tileheight = stride[1];
  template.tilesets[0].tilewidth = stride[0];
  template.tilesets[0].tilecount = info.supertile_count-1;
  template.tilesets[0].columns = info.map_w;
  template.tilesets[0].rows = info.map_w;

  let data = JSON.stringify(template, null, 2);
  fs.writeFileSync( out_fn, data );
}

function write_poms_json(info, out_fn) {

  let poms_json = {
    "rule": [],
    "name": [],
    "weight": [],
    "size": [],
    "quiltSize": [],
    "boundaryCondition":{},
    "constraint":{},
    "tileset": {}
  };

  poms_json.rule = g_info.tile_rule;
  poms_json.name = g_info.tile_name;
  poms_json.size = g_info.size;
  poms_json.quiltSize = g_info.quiltSize;
  poms_json.boundaryCondition = {
    "x+": { "type": "tile" , "value": 0 },
    "x-": { "type": "tile" , "value": 0 },
    "y+": { "type": "tile" , "value": 0 },
    "y-": { "type": "tile" , "value": 0 },
    "z+": { "type": "tile" , "value": 0 },
    "z-": { "type": "tile" , "value": 0 }
  };

  if (poms_json.size.length == 2) { poms_json.size.push(1); }
  if (poms_json.quiltSize.length == 2) { poms_json.quiltSize.push(1); }

  for (let ii=0; ii<g_info.tile_name.length; ii++) {
    poms_json.weight.push(1.0);
  }

  //if (info.use_example_img_freq) {
  if (info.tile_weight_policy == "image") {
    for (let ii=0; ii<g_info.tile_name.length; ii++) {
      let st_key = g_info.supertile_key[ii];
      poms_json.weight[ii] = g_info.supertile_lib[st_key].freq;
    }
  }
  else if (info.tile_weight_policy == "flat") {

    let maxflat = 0;
    let flatmap = [];
    for (let ii=0; ii<g_info.tile_name.length; ii++) {
      let st_key = g_info.supertile_key[ii];
      let flat_id = g_info.supertile_lib[st_key].flat_id;

      flatmap.push(flat_id);
      if (flat_id > maxflat) { maxflat = flat_id; }
    }

    let flatfreq = new Array(maxflat+1);
    for (let ii=0; ii<flatfreq.length; ii++) { flatfreq[ii]=0; }
    for (let ii=0; ii<flatmap.length; ii++) { flatfreq[ flatmap[ii] ]++; }
    for (let ii=0; ii<flatfreq.length; ii++) { flatfreq[ii] = 1/flatfreq[ii]; }

    for (let ii=0; ii<g_info.tile_name.length; ii++) {
      //let st_key = g_info.supertile_key[ii];
      poms_json.weight[ii] = flatfreq[flatmap[ii]];
    }
  }

  poms_json.constraint =  [
    { "type":"remove", "range": { "tile":[0,1], "x":[], "y":[], "z":[] } }
  ];

  if (info.rule_only_wrap || info.rule_wrap) {
    poms_json.constraint.push( { "type": "quiltPin", "range": { "tile":[], "x":[0,1], "y":[], "z":[0,1] } });
    poms_json.constraint.push( { "type": "quiltPin", "range": { "tile":[], "x":[-1], "y":[], "z":[0,1] } });
    poms_json.constraint.push( { "type": "quiltPin", "range": { "tile":[], "x":[], "y":[0,1], "z":[0,1] } });
    poms_json.constraint.push( { "type": "quiltPin", "range": { "tile":[], "x":[], "y":[-1], "z":[0,1] } });
  }

  let lines = [];
  let _multi_field = [];
  let _multi_field_n = 8;

  lines.push("{");

  lines.push("\"rule\":[");


  for (let ii=0; ii<poms_json.rule.length; ii++) {
    let sfx = ( (ii==(poms_json.rule.length-1)) ? "" : "," );

    if (((ii%_multi_field_n)==0) &&
        (_multi_field.length > 0)) {
      lines.push( _multi_field.join(" ") );
      _multi_field = [];
    }

    _multi_field.push( JSON.stringify(poms_json.rule[ii]) + sfx );
  }
  if (_multi_field.length > 0) {
    lines.push( _multi_field.join(" ") );
  }
  lines.push("],");

  _multi_field = [];

  //---

  lines.push("\"name\":[");
  for (let ii=0; ii<poms_json.name.length; ii++) {
    let sfx = ( (ii==(poms_json.name.length-1)) ? "" : "," );

    if (((ii%_multi_field_n)==0) &&
        (_multi_field.length > 0)) {
      lines.push( _multi_field.join(" ") );
      _multi_field = [];
    }

    _multi_field.push( JSON.stringify(poms_json.name[ii]) + sfx );
  }
  if (_multi_field.length > 0) {
    lines.push( _multi_field.join(" ") );
  }
  lines.push("],");

  //---

  _multi_field = [];
  lines.push("\"weight\":[");
  for (let ii=0; ii<poms_json.weight.length; ii++) {
    let sfx = ( (ii==(poms_json.weight.length-1)) ? "" : "," );

    if (((ii%_multi_field_n)==0) &&
        (_multi_field.length > 0)) {
      lines.push( _multi_field.join(" ") );
      _multi_field = [];
    }

    _multi_field.push( JSON.stringify(poms_json.weight[ii]) + sfx );
  }
  if (_multi_field.length > 0) {
    lines.push( _multi_field.join(" ") );
  }
  lines.push("],");

  //---

  _multi_field = [];
  lines.push("\"tileGroup\":[");
  for (let ii=0; ii<poms_json.name.length; ii++) {
    let sfx = ( (ii==(poms_json.name.length-1)) ? "" : "," );
    let grp = ((ii==0) ? 0 : 1);

    if (((ii%_multi_field_n)==0) &&
        (_multi_field.length > 0)) {
      lines.push( _multi_field.join(" ") );
      _multi_field = [];
    }

    _multi_field.push( grp + sfx );
  }
  if (_multi_field.length > 0) {
    lines.push( _multi_field.join(" ") );
  }
  lines.push("],");

  //---

  lines.push("\"tileset\": {");
  lines.push("  \"image\": \"" + info.tileset_fn + "\",");
  lines.push("  \"tilecount\": " + (poms_json.name.length -1).toString() + ",");
  lines.push("  \"imageheight\": " + info.tileset_size[1].toString() + ",");
  lines.push("  \"imagewidth\": " + info.tileset_size[0].toString() + ",");
  lines.push("  \"tileheight\": " + info.stride[1].toString() + ",");
  lines.push("  \"tilewidth\": " + info.stride[0].toString() + "");
  lines.push("},");

  //---

  _multi_field = [];

  lines.push("\"flatMap\":[");
  for (let ii=0; ii<poms_json.weight.length; ii++) {
    let sfx = ( (ii==(poms_json.weight.length-1)) ? "" : "," );

    if (((ii%_multi_field_n)==0) &&
        (_multi_field.length > 0)) {
      lines.push( _multi_field.join(" ") );
      _multi_field = [];
    }


    let st_key = g_info.supertile_key[ii];
    let _flat_id = g_info.supertile_lib[st_key].flat_id;
    _multi_field.push( JSON.stringify(_flat_id) + sfx );
  }
  if (_multi_field.length > 0) {
    lines.push( _multi_field.join(" ") );
  }
  lines.push("],");

  if (info.flat_tileset_fn.length > 0) {
    lines.push("\"flatTileset\": {");
    lines.push("  \"image\": \"" + info.flat_tileset_fn + "\",");
    lines.push("  \"tilecount\": " + (info.simpletile_count-1).toString() + ",");
    lines.push("  \"imageheight\": " + info.simple_tileset_size[1].toString() + ",");
    lines.push("  \"imagewidth\": " + info.simple_tileset_size[0].toString() + ",");
    lines.push("  \"tileheight\": " + info.stride[1].toString() + ",");
    lines.push("  \"tilewidth\": " + info.stride[0].toString() + "");
    lines.push("},");
  }

  //---

  _multi_field = [];

  //lines.push("\"boundaryCondition\":" + JSON.stringify(poms_json.boundaryCondition) + ",");
  lines.push("\"boundaryCondition\": {");
  let bc_count = 0;
  for (let key in poms_json.boundaryCondition) {
    if (bc_count>0) { lines[ lines.length-1 ] += ","; }
    lines.push("  \"" + key + "\":" + JSON.stringify(poms_json.boundaryCondition[key]) );
    bc_count++;
  }
  lines.push("},");

  //lines.push("\"constraint\":" + JSON.stringify(poms_json.constraint) + ",");
  lines.push("\"constraint\":[");
  for (let ii=0; ii<poms_json.constraint.length; ii++) {
    let sfx = ( (ii < (poms_json.constraint.length-1)) ? "," : "" );
    lines.push("  " + JSON.stringify(poms_json.constraint[ii]) + sfx);
  }
  lines.push("],\n");

  lines.push("\"size\":" + JSON.stringify(poms_json.size) + ",");
  lines.push("\"quiltSize\":" + JSON.stringify(poms_json.quiltSize) );
  lines.push("}");

  //---

  fs.writeFileSync( out_fn, lines.join("\n") );
}

function debug_print_info(info) {
  console.log("#info:");

  console.log("  .stride: ", JSON.stringify(info.stride));
  console.log("  .supertile_dim: ", JSON.stringify(info.supertile_dim));
  console.log("  .supertile_offset: ", JSON.stringify(info.supertile_offset));
  console.log("  .tile_offset: ", JSON.stringify(info.tile_offset));
  console.log("  .size: ", JSON.stringify(info.size));

  console.log("  .supertile_neighbor_bound: ", JSON.stringify(info.supertile_neighbor_bound));
  console.log("  .oob_pxl: ", JSON.stringify(info.oob_pxl));
}

// this is kind of hacky right now.
// The neighbor ribbons need to match, so left ribbon needs
// to be the same size as the right ribbon or things will
// almost surely screw up (no neighbors will be found).
// Presumably this should be user specified?
// For right now, do this ceil/floor business to try and
// get them to match up for odd supertile_dim sizes.
//
function init_super_tile_neighbor_bound(info) {

  let tile_w = info.stride;
  let tile_ds = info.tile_offset;
  let stile_ds = info.supertile_offset;
  let stile_w = info.supertile_dim;

  let d = [ info.supertile_dim[0], info.supertile_dim[1] ];

  if ((info.supertile_band[0] <= 0) ||
      (info.supertile_band[1] <= 0)) {

    let x2r_s = d[0] - tile_w[0];
    let x2l_e = tile_w[0];

    let y2u_e = tile_w[1];
    let y2d_s = d[1] - tile_w[1];

    info.supertile_neighbor_bound[0][0] = [ x2r_s, d[0] ];
    info.supertile_neighbor_bound[0][1] = [ 0, d[1] ];

    info.supertile_neighbor_bound[1][0] = [ 0, x2l_e ];
    info.supertile_neighbor_bound[1][1] = [ 0, d[1] ];

    info.supertile_neighbor_bound[2][0] = [ 0, d[0] ];
    info.supertile_neighbor_bound[2][1] = [ 0, y2u_e ];

    info.supertile_neighbor_bound[3][0] = [ 0, d[0] ];
    info.supertile_neighbor_bound[3][1] = [ y2d_s, d[1] ];

  }

  else {

    let _bsx = info.supertile_band[0],
        _bsy = info.supertile_band[1];

    let x2r_s = d[0] - _bsx;
    let x2l_e = _bsx;

    let y2u_e = _bsy;
    let y2d_s = d[1] - _bsy;

    console.log("###>>>", "band:", _bsx, _bsy, "x2(r_s,l_e):", x2r_s, x2l_e, "y2(u_e,d_s):", y2u_e, y2d_s);


    info.supertile_neighbor_bound[0][0] = [ x2r_s, d[0] ];
    info.supertile_neighbor_bound[0][1] = [ 0, d[1] ];

    info.supertile_neighbor_bound[1][0] = [ 0, x2l_e ];
    info.supertile_neighbor_bound[1][1] = [ 0, d[1] ];

    info.supertile_neighbor_bound[2][0] = [ 0, d[0] ];
    info.supertile_neighbor_bound[2][1] = [ 0, y2u_e ];

    info.supertile_neighbor_bound[3][0] = [ 0, d[0] ];
    info.supertile_neighbor_bound[3][1] = [ y2d_s, d[1] ];


    console.log("# supertile_neighbor_bound:", JSON.stringify(info.supertile_neighbor_bound));

  }

}

function img2tile_run(opt) {
  let info = Object.assign({}, g_info);

  for (let key in opt) {
    if (key in info) {
    }
  }

  init_super_tile_neighbor_bound(info);
  build_simple_tile_lib(info);
  build_super_tile_lib(info);
  build_tile_set(info);

  return info;
}

if (typeof module !== "undefined") {

  cli_parse();

  init_super_tile_neighbor_bound(g_info);

  if (g_info.verbose >= VERBOSE_DEBUG) { debug_print_info(g_info); }

  _log(VERBOSE_INFO, "# building simple tile lib...");
  build_simple_tile_lib(g_info);

  _log(VERBOSE_INFO, "# building super tile lib...");
  build_super_tile_lib(g_info);

  _log(VERBOSE_INFO, "# building tile set...");
  build_tile_set(g_info);

  if (g_info.flat_tileset_fn.length > 0) {
    _log(VERBOSE_INFO,"# writing flat tileset '" + g_info.flat_tileset_fn + "'");
    build_simple_tile_set(g_info);
    write_png_data(g_info.simple_tileset_png, g_info.flat_tileset_fn);
  }

  if (g_info.tileset_fn.length > 0) {
    _log(VERBOSE_INFO,"# writing tileset '" + g_info.tileset_fn + "'");
    write_tileset_png(g_info, g_info.tileset_fn);
  }

  if (g_info.verbose >= VERBOSE_DEBUG) {
    let st_lib = g_info.supertile_lib;
    for (let key in st_lib) {
      console.log(">>>", st_lib[key].id);
      for (let ii=0; ii<st_lib[key].nei_key.length; ii++) {
        console.log("[", ii, "]:\n", st_lib[key].nei_key[ii]);
      }
    }
  }

  if (g_info.tiled_fn.length > 0) {
    _log(VERBOSE_INFO, "# writing tiled JSON file '" + g_info.tiled_fn + "'");
    write_tiled_json(g_info, g_info.tiled_fn);
  }

  if (g_info.flat_tiled_fn.length > 0) {
    _log(VERBOSE_INFO, "# writing flat tiled JSON file '" + g_info.flat_tiled_fn + "'");
    write_flat_tiled_json(g_info, g_info.flat_tiled_fn);
  }

  if (g_info.poms_fn.length > 0) {
    _log( VERBOSE_INFO, "# writing POMS JSON '" + g_info.poms_fn + "'");
    write_poms_json(g_info, g_info.poms_fn);
  }

}
