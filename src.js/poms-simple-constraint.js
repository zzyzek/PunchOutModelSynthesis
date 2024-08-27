// LICENSE: CC0
//
// To the extent possible under law, the person who associated CC0 with
// this project has waived all copyright and related or neighboring rights
// to this project.
//
//

// This is pretty much hard coded but I'm still going to check this in.
// The constraints are rudimentary in the POMS config file.
// To help with creating constraints that are more easily specified,
// one can maybe use something like this, where tiles can be referred
// to by name, simple constraints can be specified easily, etc.
//
// This is a layer of abstraction on top of a layer of abstraction,
// so it might be too many degrees of separation but I'm keeping
// this here for now.
//
// At the very least, this creates a simple road2d test that can
// be used for elsewhere.
//

// node poms-simple-constraint.js ../data/road2d_poms.json > ../data/road2d_test.json

"use strict";

var fs = require("fs");
var libpoms = require("./libpoms");

function load_poms_config(fn) {
  let raw = fs.readFileSync(fn);
  let data = JSON.parse(raw);

  return data;
}

if (process.argv.length < 3) {
  console.log("provide poms file");
  process.exit(-1);
}

let poms = load_poms_config(process.argv[2]);

let constraint = [

  { "p": [ 0, 0,0], "t": "only", "v": [ "r003" ] },
  { "p": [ 1, 0,0], "t": "only", "v": [ "p001", "T002" ] },
  { "p": [-1, 0,0], "t": "only", "v": [ "r002" ] },


  { "p": [ 0, 1,0], "t": "only", "v": [ "p000", "T003" ] },
  { "p": [ 1, 1,0], "t": "only", "v": [ "r001", "r002", "p000", "T001"  ] },
  { "p": [-1, 1,0], "t": "only", "v": [ "p000" ] },

  { "p": [ 0,-1,0], "t": "only", "v": [ "r000" ] },
  { "p": [ 1,-1,0], "t": "only", "v": [ "p001", "T000" ] },
  { "p": [-1,-1,0], "t": "only", "v": [ "r001" ] }

];

let name_idx = {};
for (let ii=0; ii<poms.name.length; ii++) {
  name_idx[ poms.name[ii] ] = ii;
}

let new_size = [3,3,1];
let new_constraint = [
 { "type": "remove", "range": { "tile": [], "x": [], "y": [], "z": [] } }
];

for (let ii=0; ii<constraint.length; ii++) {

  let c = constraint[ii];

  for (let tidx=0; tidx<c.v.length; tidx++) {
    let tile_name = c.v[tidx];
    let tile_id = name_idx[ tile_name ];


    let pc = {
      "type": "add",
      "range": {
        "tile": [tile_id, tile_id+1],
        "x": [c.p[0], c.p[0]+1],
        "y": [c.p[1], c.p[1]+1],
        "z": [c.p[2], c.p[2]+1]
      }
    };

    new_constraint.push(pc);
  }
}

poms.size = new_size;
poms.constraint = new_constraint;


console.log(libpoms.configStringify( poms ));




