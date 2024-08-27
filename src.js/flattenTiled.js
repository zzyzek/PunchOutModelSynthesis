// LICENSE: CC0
//
// Note this has only been really tested for 2x2 windows.
// The boundary conditions might be a little screwed
// up for other types of windows, so this needs to be revisited.
//

// This program takes in a:
//
// * POMS Config file
// * Tiled file
// * flattened tileset
//
// and outputs a Tiled JSON file that has the tilemap
// 'flattened' to use the flattened tileset.
// This requires the POMS config file to have a flatMap
// to do the mapping.
//

var FLATTENTILED_VERSION = "0.1.0";

if (typeof module !== "undefined") {
  var fs = require("fs");
  var getopt = require("posix-getopt");
}

var g_info = {
  "verbose" : 0,
  "input_poms_fn": "",
  "input_tiled_fn": "",
  "output_tiled_fn": "",
  "flat_tileset_fn": ""
};

function show_version(fp) {
  fp.write(FLATTENTILED_VERSION);
}

function show_help(fp) {
  fp.write("\nflattenTiled, version ");
  show_version(fp);
  fp.write("\n");
  fp.write("  -C,--poms-fn          Input POMS JSON config file\n");
  fp.write("  -i,--input-tiled-fn   Input Tiled JSON file\n");
  fp.write("  -o,--flat-tiled-fn    Output flat Tiled JSON file\n");
  fp.write("  -t,--flat-tileset     output flat tileset PNG file\n");
  fp.write("  -V,--verbose          set verbosity level (default 0)\n");
  fp.write("  -h,--help             help (this scren)\n");
  fp.write("  -v,--version          show version\n");
  fp.write("\n");
}


function cli_parse(info) {
  info = ((typeof info === "undefined") ? g_info : info);
  let _cmd = "";

  let parser, opt;

  let long_opt = [
    "C", ":(poms-fn)",
    "i", ":(input-tiled-fn)",
    "o", ":(flat-tiled-fn)",
    "t", ":(flat-tileset)",
    "V", "(verbose)",
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

      case 't':
        info.flat_tileset_fn = opt.optarg;
        break;
      case 'C':
        info.input_poms_fn = opt.optarg;
        break;
      case 'i':
        info.input_tiled_fn = opt.optarg;
        break;
      case 'o':
        info.output_tiled_fn = opt.optarg;
        break;

      default:
        show_help(process.stderr);
        process.exit(-1);
        break;
    }
  }

  if (parser.optind() < process.argv.length) {
    _cmd = process.argv[parser.optind()];
  }

  return info;
}




function write_tiled_json(info, out_fn) {

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


function flatten_tiled( poms, tiled ) {
  let flat_tiled_info = JSON.parse(JSON.stringify(tiled));

  let ft = poms.flatTileset;

  let flatmap = poms.flatMap;

  let row = ft.imageheight / ft.tileheight;
  let col = ft.imagewidth / ft.tilewidth;
  flat_tiled_info.tilesets[0].columns = col;
  flat_tiled_info.tilesets[0].rows = row;
  flat_tiled_info.tilesets[0].image = ft.image;
  flat_tiled_info.tilesets[0].tilecount = ft.tilecount;
  flat_tiled_info.tilesets[0].tileheight = ft.tileheight;
  flat_tiled_info.tilesets[0].tilewidth = ft.tilewidth;
  flat_tiled_info.tilesets[0].margin = 0;
  flat_tiled_info.tilesets[0].spacing = 0;
  flat_tiled_info.tilesets[0].name = "flat_tileset";
  flat_tiled_info.tilesets[0].firstgid = 1;

  let idat = flat_tiled_info.layers[0].data;

  for (let idx=0; idx<idat.length; idx++) {
    idat[idx] = flatmap[ idat[idx] ];
  }

  return flat_tiled_info;
}

function node_main() {
  g_info = cli_parse();

  if ((g_info.input_tiled_fn.length == 0) ||
      (g_info.input_poms_fn.length == 0)) {
    show_help(process.stderr);
    process.exit(-1);
  }

  let poms = JSON.parse( fs.readFileSync(g_info.input_poms_fn) );
  let inp_tiled = JSON.parse( fs.readFileSync(g_info.input_tiled_fn) );

  if (g_info.flat_tileset_fn.length == 0) {
    if ("flatTileset" in poms) {
      g_info.flat_tileset_fn = poms.flatTileset.image;
    }
    else {
      process.stderr.write("could not find flat tileset PNG in POMS config file");
      process.exit(-1);
    }
  }

  let flat_tiled = flatten_tiled( poms, inp_tiled );

  console.log(JSON.stringify(flat_tiled));


}



node_main();
