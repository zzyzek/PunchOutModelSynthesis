// LICENSE: cc0
//
// To the extent possible under law, the person who associated CC0 with
// this code has waived all copyright and related or neighboring rights
// to this code.
//
// You should have received a copy of the CC0 legalcode along with this
// work.  If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//

// create simple 2d road tileset
// re-use the obj files for
//  the stair tileset

var OUT_FN = "road2d_poms.json";

var WITHOUT_T = false;
var WITHOUT_DEADEND = false;

WITHOUT_T = true
WITHOUT_DEADEND = false;

var fs = require("fs");
var libpoms = require("./libpoms.js");

let tiled_compatible = true;


// rotations are clockwise
//

let dir_descr = [
  "+1:0:0", "-1:0:0",
  "0:+1:0", "0:-1:0",
  "0:0:+1", "0:0:-1"
];

let oppo_dir = [
  1, 0,
  3, 2,
  5, 4
];

let name_list = [
  ".000",
//  "e000",

  "p000",
  "p001",

  "r000",
  "r001",
  "r002",
  "r003",

  "c000",

  "T000",
  "T001",
  "T002",
  "T003",

  "d000",
  "d001",
  "d002",
  "d003"

];

if (tiled_compatible) {
  name_list.splice(1,0,"e000");
}


let name_link = [

  // .
  [ 0, 0, 0, 0 ],

  // e - empty
  //[ 0, 0, 0, 0 ],

  // p - path
  [ 0, 0, 1, 1 ],
  [ 1, 1, 0, 0 ],

  // r - bend
  [ 1, 0, 0, 1 ],
  [ 0, 1, 0, 1 ],
  [ 0, 1, 1, 0 ],
  [ 1, 0, 1, 0 ],

  // c - cross
  [ 1, 1, 1, 1 ],

  // T
  [ 1, 1, 0, 1 ],
  [ 0, 1, 1, 1 ],
  [ 1, 1, 1, 0 ],
  [ 1, 0, 1, 1 ],

  // d - dead end
  [ 0, 0, 0, 1 ],
  [ 0, 1, 0, 0 ],
  [ 0, 0, 1, 0 ],
  [ 1, 0, 0, 0 ]

];

if (tiled_compatible) {
  name_link.splice(1,0,[0,0,0,0]);
}

// a rule is valid when the value in the name link (above)
// has the same value as its neighbor.
// So, for each rule, check the source in idir direction
// to dst in opposite dirction (odir).
// The exception is for the third dimension, in which case
// the only valid neighbor is the 'null' tile (0 tile).
//
// The null tile is reserved by Tiled to be unused/transparent.
// The 'e' tile (e000) is an explicit tile for grass/"empty"
//
// Note that the null tile doesn't allow roads/bends/crosses/Ts
// to be neighbors, so roads can't spill outside of the grid
// if boundary conditions are 0.
//
let rule_a = [];
for (let idir=0; idir<6; idir++) {
  for (let src_tile=0; src_tile < name_list.length; src_tile++) {
    for (let dst_tile=0; dst_tile < name_list.length; dst_tile++) {

      let v = 0;
      if (idir >= 4) {
        if ((src_tile == 0) || (dst_tile == 0)) {
          v = 1;
        }
      }
      else {
        odir = oppo_dir[idir];
        if ( name_link[src_tile][idir] == name_link[dst_tile][odir] ) {
          v=1;
        }
      }

      rule_a.push( [ src_tile, dst_tile, idir, v ] );
    }
  }
}

let poms_json = {
  "rule": [],
  "weight": [],
  "tileset" : {
    "image": "road2d_tileset.png",
    "tilecount": name_list.length-1,
    "imagewidth": 32,
    "imageheight": 32,
    "tileheight": 8,
    "tilewidth": 8
  },
  "name" : [],
  "boundaryCondition": {
    "x+": { "type": "tile" , "value": 0 },
    "x-": { "type": "tile" , "value": 0 },
    "y+": { "type": "tile" , "value": 0 },
    "y-": { "type": "tile" , "value": 0 },
    "z+": { "type": "tile" , "value": 0 },
    "z-": { "type": "tile" , "value": 0 }
  },
  "size": [32,32,1],
  "constraint": [
    { "type":"remove", "range": { "tile":[0,1],"x":[], "y":[], "z":[] } }
  ]
};

if (WITHOUT_T) {
  poms_json.constraint.push(
    { "type":"remove", "range": {"tile":[ name_list.length-8, name_list.length-4 ], "x":[],"y":[],"z":[] } }
  );
}

if (WITHOUT_DEADEND) {
  poms_json.constraint.push(
    { "type":"remove", "range": {"tile":[ name_list.length-4, name_list.length ], "x":[],"y":[],"z":[] } }
  );
}

poms_json.name = name_list;
poms_json.rule = rule_a;

libpoms.configWrite(poms_json, OUT_FN);

