// LICENSE: CC0
//
// To the extent possible under law, the person who associated CC0 with
// this project has waived all copyright and related or neighboring rights
// to this project.
// 
// You should have received a copy of the CC0 legalcode along with this
// work.  If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//

let stride = [34,34];

let nxy = [8,8];

let n_tile = nxy[0]*nxy[1];

let rule = [];
let tile_dock = [];


//  1 3 2
//   \|/
//   /|\
//  5 0 4
//

let name = [ "." ];
let weight = [ 1 ];
let tileGroup = [ 0 ];

let boundaryCondition = {
  "x+":{"type":"tile","value":0},
  "x-":{"type":"tile","value":0},
  "y+":{"type":"tile","value":0},
  "y-":{"type":"tile","value":0},
  "z+":{"type":"tile","value":0},
  "z-":{"type":"tile","value":0}
};

let constraint = [ { "type":"remove", "range": { "tile":[0,1], "x":[], "y":[], "z":[] } } ];

let tileset = {
  "image" : "atlas_flat.png",
  "tilecount": 64,
  "imagewidth": 34*8,
  "imageheight": 34*8,
  "tilewidth": 34,
  "tileheight": 34
};

let size = [128,128,1];

for (let y=0; y<nxy[1]; y++) {
  for (let x=0; x<nxy[0]; x++) {

    let idx = y*nxy[0] + x;

    let bv_sum=0;
    let bv = [0,0,0,0,0,0];
    for (let i=0; i<6; i++) {
      if (idx & (1<<i)) { bv[i] = 1; bv_sum++; }
    }

    let _w = 1;
    let _s = 't' + bv_sum.toString() + ".x:" + x.toString() + ".y:" + y.toString();
    let _g = 1;


    name.push( _s );
    weight.push( _w );
    tileGroup.push( _g );

    tile_dock.push(bv);
  }
}

weight[1] = 30;

for (let src_idx=0; src_idx<tile_dock.length; src_idx++) {
  for (let dst_idx=0; dst_idx<tile_dock.length; dst_idx++) {

    let src_id = src_idx+1;
    let dst_id = dst_idx+1;

    if (tile_dock[src_idx][3] == tile_dock[dst_idx][0]) {
      rule.push([src_id, dst_id, 3,1]);
    }

    if (tile_dock[src_idx][0] == tile_dock[dst_idx][3]) {
      rule.push([src_id, dst_id, 2,1]);
    }

    if ((tile_dock[src_idx][2] == tile_dock[dst_idx][1]) &&
             (tile_dock[src_idx][4] == tile_dock[dst_idx][5])) {
      rule.push([src_id, dst_id, 0,1]);
    }

    if ((tile_dock[src_idx][1] == tile_dock[dst_idx][2]) &&
             (tile_dock[src_idx][5] == tile_dock[dst_idx][4])) {
      rule.push([src_id, dst_id, 1,1]);
    }

  }
}

for (let i=0; i<tile_dock.length; i++) {
  for (let idir=0; idir<6; idir++) {
    rule.push( [0, i+1, idir, 1] );
    rule.push( [i+1, 0, idir, 1] );
  }
}

let poms = {
  "rule": rule,
  "name": name,
  "weight": weight,
  "tileGroup": tileGroup,
  "boundaryCondition": boundaryCondition,
  "constraint": constraint,
  "tileset": tileset,
  "size": size
};

console.log( JSON.stringify( poms, undefined, 2 ) );
