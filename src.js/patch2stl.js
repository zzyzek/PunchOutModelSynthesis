// LICENSE: CC0
//
// Note this has only been really tested for 2x2 windows.
// The boundary conditions might be a little screwed
// up for other types of windows, so this needs to be revisited.
//


var PATCH2STL_VERSION = "0.1.0";

var fs = require("fs");

var jeom = require("./jeom.js");

var libpoms = require("./libpoms.js");

//----

var g_ctx = {
  "out_stl": "out.stl",
  //"poms_fn": "./marble_poms_32x12x32.json",
  //"patch_fn" : "./marble_patch_32x12x32.json",
  "poms_fn": "",
  "patch_fn" : "",

  "filter_regex": "",

  "verbose":0,

  "uniq_obj" : [],
  "uniq_obj_idx": {},

  "tile_geom": []
};


//----

var getopt = require("posix-getopt");
var parser, opt;
parser = new getopt.BasicParser("hvV:(verbose)C:(poms-file)P:(patch-file)o:(out-stl)F:(filter-regex)", process.argv);


function show_help() {
  console.log("\nusage:");
  console.log("");
  console.log("    patch2stl [-h] [-v] [-V] [-C poms-file] [-P patch-fn] [-F filter-regex] [-o out-stl]");
  console.log("");

  console.log("  -C         poms config file (required)");
  console.log("  -P         patch file (required)");
  console.log("  -o         output stl file (default stdout)");
  console.log("  -F         filter tile name by regex");

  console.log("  -h         help (this screen)");
  console.log("  -v         print version");
  console.log("  -V         verbosity level");

  console.log("\n");
}

function show_version(preamble) {
  preamble = ((typeof preamble === "undefined") ? false : preamble);
  let pfx = (preamble ? "version: " : "" );
  console.log(pfx + PATCH2STL_VERSION);
}

while ((opt = parser.getopt()) !== undefined) {
  switch (opt.option) {

    case 'P':
      g_ctx.patch_fn = opt.optarg;
      break;
    case 'C':
      g_ctx.poms_fn = opt.optarg;
      break;
    case 'o':
      g_ctx.out_stl = opt.optarg;
      break;

    case 'F':
      g_ctx.filter_regex = opt.optarg;
      break;

    case 'h':
      show_version();
      show_help();
      break;
    case 'v':
      show_version();
      break;
    case 'V':
      g_ctx.verbose++;
      break;

    default:
      show_version();
      show_help();
      break;

  }
}

if ((g_ctx.patch_fn.length == 0) ||
    (g_ctx.poms_fn.length == 0)) {
  console.log("\nprovide patch and POMS config file");
  show_help();
  process.exit();
}

//----


function load_obj(fn) {
  let sdat = fs.readFileSync(fn);
  let _obj = jeom.obj_split_loads(sdat.toString());
  let flat_obj = jeom.obj2flat(_obj);
  let tri = jeom.obj2tri(_obj);

  let dat = {
    "orig_obj": _obj,
    "obj": flat_obj,
    "tri": tri
  }

  return dat;
}


function main(ctx) {

  var poms = JSON.parse( fs.readFileSync(ctx.poms_fn) );
  var patch = JSON.parse( fs.readFileSync(ctx.patch_fn) );

  for (let ii=0; ii<poms.objMap.length; ii++) {

    let objfn = poms.objMap[ii];

    if (!(objfn in ctx.uniq_obj_idx)) {
      let _o = load_obj(objfn);

      ctx.uniq_obj_idx[objfn] = ctx.uniq_obj.length;
      //ctx.uniq_obj.push( load_obj(objfn) );
      ctx.uniq_obj.push( _o );


    }

  }


  let cell_count = patch.size[0]*patch.size[1]*patch.size[2];

  //console.log("# >>>",  libpoms.vec2cell([1,2,3], poms.size), libpoms.cell2vec(5, poms.size));
  //console.log( poms.objMap[0], ctx.uniq_obj[0] );

  let data = patch.patch;

  let out_tri = [];
  let flat_tri = [];

  for (let cell=0; cell<cell_count; cell++) {

    let v = libpoms.cell2vec(cell, patch.size);

    let tile = data[cell];

    if (tile < 0) { continue; }

    let tile_name = poms.name[tile];
    if (ctx.filter_regex.length > 0) {
      if (!(tile_name.match(ctx.filter_regex))) {
        continue;
      }
    }


    let objname = poms.objMap[tile];

    //if (objname.match( /empty\.obj$/ )) { continue; }

    let obj_idx = ctx.uniq_obj_idx[objname];

    if (ctx.uniq_obj[obj_idx].tri.length == 0) { continue; }

    let _tri = jeom.dup( ctx.uniq_obj[obj_idx].tri );

    jeom.mov( _tri, v );

    out_tri.push(_tri);

    for (let ii=0; ii<_tri.length; ii++) {
      flat_tri.push(_tri[ii]);
    }

  }

  console.log("# writing:", ctx.out_stl);
  fs.writeFileSync( ctx.out_stl, jeom.stl_stringify(flat_tri) );

}

main(g_ctx);
