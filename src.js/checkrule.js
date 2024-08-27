// LICENSE: CC0
//
// Note this has only been really tested for 2x2 windows.
// The boundary conditions might be a little screwed
// up for other types of windows, so this needs to be revisited.
//

var fs = require("fs");
var getopt = require("posix-getopt");

var _eps = (1.0/(1024.0*1024.0));


var g_opt = {
  "tiled_fn" : "out.json",
  "poms_fn" : "poms.json"
};

tiled_json = JSON.parse( fs.readFileSync(g_opt.tiled_fn) );
poms_json = JSON.parse( fs.readFileSync(g_opt.poms_fn) );

let tiled_width = tiled_json.width;
let tiled_height = tiled_json.height;
let tiled_data = tiled_json.layers[0].data;

let rule_data = poms_json.rule;
let rule_map = {};

for (let ii=0; ii<rule_data.length; ii++) {
  let src_tile = rule_data[ii][0];
  let dst_tile = rule_data[ii][1];
  let idir = rule_data[ii][2];
  let val = rule_data[ii][3];

  if (val < _eps)  { continue; }

  let key = src_tile.toString() + ":" + dst_tile.toString() + ":" + idir.toString();
  rule_map[key] = rule_data[ii];
}

//console.log("## setup done");

let idir_xy = [ [1,0], [-1,0], [0,1], [0,-1] ];
let nei_tile = [0, 0, 0, 0];
let boundary_tile = 0;

for (let h=0; h<tiled_height; h++) {

  let _row = [];
  for (let w=0; w<tiled_width; w++) {
    let p = h*tiled_width + w;
    let src_tile = tiled_data[p];

    let nei_p = [0,0,0,0];

    for (let idir=0; idir<4; idir++) {
      let _w = w+idir_xy[idir][0];
      let _h = h+idir_xy[idir][1];
      if ((_w<0) || (_w>=tiled_width) ||
          (_h<0) || (_h>=tiled_height)) {
        nei_p[idir] = -1;
        continue;
      }
      nei_p[idir] = _h*tiled_width + _w;
    }

    nei_tile = [-1,-1,-1,-1];
    for (let idir=0; idir<4; idir++) {
      if (nei_p[idir] < 0) {
        nei_tile[idir] = boundary_tile;
        continue;
      }
      nei_tile[idir] = tiled_data[nei_p[idir]];
    }

    //console.log(p, src_tile, nei_p, nei_tile);

    for (let idir=0; idir<4; idir++) {
      let key = src_tile.toString() + ":" + nei_tile[idir].toString() + ":" + idir.toString();

      if (!(key in rule_map)) {
        console.log("key:", key, "not found @", w,h);
      }
    }

  }

}

console.log("## done");

