//LICENSE: CC0
//

// experiments with seeding corners and edges
// of OLoZ to try and get closer to the hard
// constraints for the boundary conditions
// implied by the exemplar image.
//

var fs = require("fs");
var libpoms = require("./libpoms.js");

function idx2xy(idx, sz) {
  let x = idx % sz[0];
  let y = Math.floor(idx / sz[0]);
  return {"x":x, "y":y};
}

function xy2idx(x,y,sz) {
  return (y*sz[0]) + x;
}

var g_ctx = {
  "size": [256,88,1],

  "seed_type": "frame",
  "frame_len": [0,0],
  "percent": 0.25,

  "poms": {},
  "tiled": {},
  "tiled_size": [-1,-1,-1]

};

var poms_fn = "../data/oloz_poms_quilt_m.json";
var tiled_fn = "../data/oloz_tilemap.json";
var out_fn = "../data/oloz_poms_m_frame.json";


g_ctx.poms  = JSON.parse( fs.readFileSync(poms_fn) );
g_ctx.tiled = JSON.parse( fs.readFileSync(tiled_fn) );


g_ctx.tiled_size[0] = g_ctx.tiled.width;
g_ctx.tiled_size[1] = g_ctx.tiled.height;
g_ctx.tiled_size[2] = 1;


let data = g_ctx.tiled.layers[0].data;

g_ctx.frame_len[0] = Math.floor(g_ctx.size[0]*g_ctx.percent);
g_ctx.frame_len[1] = Math.floor(g_ctx.size[1]*g_ctx.percent);

let x_n = g_ctx.frame_len[0],
    y_n = g_ctx.frame_len[1];



let x_s=0, x_e=0, y_s=0, y_e=0;

let frame_xy_range = [

  [ [ 0,    x_n,  1],   [ 0,  1,    0] ],
  [ [ 0,    x_n,  1],   [-1,  0,    0] ],
  [ [-x_n,  0,    1],   [ 0,  1,    0] ],
  [ [-x_n,  0,    1],   [-1,  0,    0] ],

  [ [ 0,    1,    0],   [ 1,    y_n,  1] ],
  [ [ 0,    1,    0],   [-y_n,  -1,   1] ],
  [ [-1,    0,    0],   [ 1,    y_n,  1] ],
  [ [-1,    0,    0],   [-y_n,  -1,   1] ]

];

let constraints = [];


for (let range_idx=0; range_idx<frame_xy_range.length; range_idx++) {
  let range = frame_xy_range[range_idx];

  console.log("##", JSON.stringify(range));

  let x = range[0][0];
  let y = range[1][0];
  while ((x < range[0][1]) &&
         (y < range[1][1])) {

    let x_idx = ((x<0) ? (g_ctx.tiled_size[0]+x) : x);
    let y_idx = ((y<0) ? (g_ctx.tiled_size[1]+y) : y);


    let tiled_idx = xy2idx(x_idx,y_idx, g_ctx.tiled_size);
    let tile_id = g_ctx.tiled.layers[0].data[tiled_idx];

    let constraint = { "type": "force", "range": {"tile":[tile_id, tile_id+1], "x":[x,x+1], "y":[y,y+1], "z":[0,1]} };

    constraints.push(constraint);
    console.log("  ", JSON.stringify(constraint));

    x += range[0][2];
    y += range[1][2];
  }


}


for (let ii=0; ii<constraints.length; ii++) {
  g_ctx.poms.constraint.push(constraints[ii]);
}

libpoms.configWrite(g_ctx.poms, out_fn);

//console.log( JSON.stringify(g_ctx.poms) );

