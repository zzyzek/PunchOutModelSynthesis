// LICENSE: CC0
//

var fs = require("fs");

var jscad = require("@jscad/modeling");
var array_utils = require('@jscad/array-utils')

var objectDeserializer = require('@jscad/obj-deserializer')
var objectSerializer = require('@jscad/obj-serializer')

var stlSerializer = require('@jscad/stl-serializer')
var stlDeserializer = require('@jscad/stl-deserializer')

var m4 = require("./m4.js");
var jeom = require("./jeom.js");

var op = {

  "obj_loads": objectDeserializer.deserialize,
  "obj_dumps": objectSerializer.serialize,

  "stl_loads": stlDeserializer.deserialize,
  "stl_dumps": stlSerializer.serialize,

  "flatten": array_utils.flatten,

  "clone": jscad.geometries.geom3.clone,
  "points": jscad.geometries.geom3.toPoints,
  "polygons": jscad.geometries.geom3.toPolygons,
  "validate": jscad.geometries.geom3.validate,
  "create": jscad.geometries.geom3.create,

  "add" : jscad.booleans.union,
  "sub" : jscad.booleans.subtract,
  "or" : jscad.booleans.union,
  "and" : jscad.booleans.intersect,
  "scission": jscad.booleans.scission,

  "expand": jscad.expansions.expand,
  "offset": jscad.expansions.offset,

  "cube": jscad.primitives.cube,
  "cuboid": jscad.primitives.cuboid,
  "cylinder": jscad.primitives.cylinder,
  "sphere": jscad.primitives.sphere,

  "ruboid": jscad.primitives.roundedCuboid,

  "rot": jscad.transforms.rotate,
  "rotX": jscad.transforms.rotateX,
  "rotY": jscad.transforms.rotateY,
  "rotZ": jscad.transforms.rotateZ,

  "mul": jscad.transforms.transform,

  "mov": jscad.transforms.translate,
  "movX": jscad.transforms.translateX,
  "movY": jscad.transforms.translateY,
  "movZ": jscad.transforms.translateZ,

  "flip": jscad.transforms.mirror,
  "flipX": jscad.transforms.mirrorX,
  "flipY": jscad.transforms.mirrorY,
  "flipZ": jscad.transforms.mirrorZ,

  "scale": jscad.transforms.scale,
  "scaleX": jscad.transforms.scaleX,
  "scaleY": jscad.transforms.scaleY,
  "scaleZ": jscad.transforms.scaleZ,

  "com" : jscad.measurements.measureCenterOfMass,
  "bbox" : jscad.measurements.measureBoundingBox,
  "area" : jscad.measurements.measureArea,
  "vol" : jscad.measurements.measureVolume

};

function _simple_point_count(geom) {
  let pnts = op.points(geom);
  let n = 0;
  for (let ii=0; ii<pnts.length; ii++) {
    n += pnts[ii].length;
  }
  return n;
}

function _simple_print(geom) {

  let pnts = op.points(geom);
  for (let ii=0; ii<pnts.length; ii++) {
    for (let jj=0; jj<pnts[ii].length; jj++) {
      console.log(pnts[ii][jj].join(" "));
    }
    if (pnts[ii].length > 0) {
      console.log(pnts[ii][0].join(" "));
    }
    console.log("\n");
  }
  return;


  let pgon = geom.polygons;
  for (let ii=0; ii<pgon.length; ii++) {
    let v = pgon[ii].vertices;
    for (let jj=0; jj<v.length; jj++) {
      console.log(v[jj].join(" "));
    }
    if (v.length > 0) {
      console.log(v[0].join(" "));
    }
    console.log("\n");
  }

}

function obj2geom(fn_name) {
  let rawData = fs.readFileSync(fn_name);
  var geom = op.obj_loads({"output":"geometry"}, rawData.toString());
  return geom;
}

function _debug_print_dock_lib(dock_lib) {

  // debug...
  //
  for (i=0; i<dock_lib.length; i++)  {

    let dx = 4;
    let dy = 6;
    let dz = 6;

    console.log("##\n##", dock_lib[i].exemplar);

    let _pgn = op.mov( [i*dx,0,0], dock_lib[i].src_pos );
    _simple_print(_pgn);
    console.log("\n\n");

    _pgn = op.mov( [i*dx,0,0], dock_lib[i].dst_pos );
    _simple_print(_pgn);
    console.log("\n\n");

    _pgn = op.mov( [i*dx,0,dz], dock_lib[i].src_neg );
    _simple_print(_pgn);
    console.log("\n\n");

    _pgn = op.mov( [i*dx,0,dz], dock_lib[i].dst_neg);
    _simple_print(_pgn);
    console.log("\n\n");

    // slices...
    //
    //_pgn = op.mov( [i*dx,0,dz*2], dock_lib[i].src_slice);
    //_simple_print(_pgn);
    //console.log("\n\n");

    //_pgn = op.mov( [i*dx,0,dz*2], dock_lib[i].dst_slice);
    //_simple_print(_pgn);
    //console.log("\n\n");

  }

}

function _incr_rot_idx(rot_v, symmetry) {

  let tok = symmetry.split(",");

  for (let tok_idx=0; tok_idx<tok.length; tok_idx++) {

    let sub_tok = tok[tok_idx].split("");

    let _carry = 0;
    for (let sub_tok_idx=0; sub_tok_idx<sub_tok.length; sub_tok_idx++) {

      let _axis = sub_tok[sub_tok_idx];

      if (_axis == "x") { rot_v[0] = (rot_v[0] + 1) % 4; _carry = ((rot_v[0]==0) ? 1 : _carry); }
      if (_axis == "y") { rot_v[1] = (rot_v[1] + 1) % 4; _carry = ((rot_v[1]==0) ? 1 : _carry); }
      if (_axis == "z") { rot_v[2] = (rot_v[2] + 1) % 4; _carry = ((rot_v[2]==0) ? 1 : _carry); }

    }

    if (_carry==0) { break; }
  }

}

// takes in idir and irot (3vec) and
// return idir of resulting direction.
//
function idir_irot(idir, irot) {
  let v = [ 0, 0, 0, 1 ];

  if      (idir == 0) { v[0] =  1; }
  else if (idir == 1) { v[0] = -1; }
  else if (idir == 2) { v[1] =  1; }
  else if (idir == 3) { v[1] = -1; }
  else if (idir == 4) { v[2] =  1; }
  else if (idir == 5) { v[2] = -1; }
  else { return -1; }

  let Mx = m4.xRotation( irot[0]*Math.PI/2 );
  let My = m4.yRotation( irot[1]*Math.PI/2 );
  let Mz = m4.zRotation( irot[2]*Math.PI/2 );

  let M = m4.mul( Mz, m4.mul( My, Mx ) );

  let v_r = m4.mulp(M, v);

  if      (v_r[0] >  0.5) { return 0; }
  else if (v_r[0] < -0.5) { return 1; }
  else if (v_r[1] >  0.5) { return 2; }
  else if (v_r[1] < -0.5) { return 3; }
  else if (v_r[2] >  0.5) { return 4; }
  else if (v_r[2] < -0.5) { return 5; }

  return -1;
}

// permuatation of cube faces as if it were
// a type of transform (rotation, mirror, etc.)
//
//            2 (y+)
//            ^
//            |   5 (z-)
//            |  / 
// 1 (x-) ____. /___> 0 (x+)
//           /|
//          / |
//         /  |
//        L   |
//    4 (z+)  3 (y-)
//
function dock_permutation(sym, dock) {
  let perm_dock = [ dock[0], dock[1], dock[2], dock[3], dock[4], dock[5] ];

  // rotations
  //
  if ((sym == 'y') || (sym == 'ry')) {
    perm_dock[4] = dock[0];
    perm_dock[1] = dock[4];
    perm_dock[5] = dock[1];
    perm_dock[0] = dock[5];
  }

  if ((sym == 'x') || (sym == 'rx')) {
    perm_dock[3] = dock[5];
    perm_dock[4] = dock[3];
    perm_dock[2] = dock[4];
    perm_dock[5] = dock[2];
  }

  if ((sym == 'z') || (sym == 'rz')) {
    perm_dock[0] = dock[2];
    perm_dock[3] = dock[0];
    perm_dock[1] = dock[3];
    perm_dock[2] = dock[1];
  }

  // flip
  //
  if (sym == 'fx') {
    perm_dock[0] = dock[1];
    perm_dock[1] = dock[0];
  }

  if (sym == 'fy') {
    perm_dock[2] = dock[3];
    perm_dock[3] = dock[2];
  }

  if (sym == 'fz') {
    perm_dock[4] = dock[5];
    perm_dock[5] = dock[4];
  }

  return perm_dock;
}

function _mprint(m) {
  let count = 0;
  for (let ii=0; ii<4; ii++) {

    let t = [];
    for (let jj=0; jj<4; jj++) {
      t.push(m[count]);
      count++;
    }
    console.log("  " + t.join(" "));
  }
}

function blockRotate(cell, rot) {

  let rcell = [0,0,0];

  Mz = m4.zRotation(rot[2]);
  My = m4.yRotation(rot[1]);
  Mx = m4.xRotation(rot[0]);

  var _v = Array.from(m4.mulp( My, cell ));

  rcell = m4.mulp( Mz, m4.mulp( My, m4.mulp( Mx, cell ) ) );

  for (let ii=0; ii<rcell.length; ii++) {
    rcell[ii] = Math.round(rcell[ii]);
  }

  return Array.from(rcell);
}

// info holds the tile group with `info.dock`
// holding the array of docking port information,
// one for each cell location that each of the tiles
// in the tile group occupy.
// The `info.dock` has 6 positions, one for each direction,
// that hold docking information.
// This function rotates the docking information for each
// tile and throws away tiles with duplicate docking information.
//
// Only unique representatives are kept.
//
function createRepresentative(cfg, info) {

  let name = info.name;
  let dock_block_list = info.dock;
  //let d_cell = (("d_cell" in info) ? info.d_cell : [[0,0,0]]);

  let rot_lib = {};
  let rep_list = [];

  let sym = [];
  let _syms_ = cfg.symmetry.split(",");
  for (let ii=0; ii<_syms_.length; ii++) {
    sym.push( _syms_[ii].split("") );
  }

  for (let dock_idx=0; dock_idx<info.dock.length; dock_idx++) {

    let cur_dock = info.dock[dock_idx];

    let irot=[0,0,0];
    let rot_sfx = '';

    do {

      let dock_key = dock_idx.toString() + "," + cur_dock.join(",");
      if (!(dock_key in rot_lib)) {

        rot_sfx = "_" + irot.join("") + "_" + dock_idx.toString();

        let tile_name = name + rot_sfx;
        let tile_irot = [ irot[0], irot[1], irot[2] ];
        let tile_rad_rot = [ Math.PI*irot[0]/2, Math.PI*irot[1]/2, Math.PI*irot[2]/2 ];

        //let tile_geom = op.rotZ( tile_rad_rot[2],
        //                op.rotY( tile_rad_rot[1],
        //                op.rotX( tile_rad_rot[0], geom ) ) );

        //let tile_block = [[0,0,0]];
        //for (let cidx=1; cidx<d_cell.length; cidx++) {
        //  tile_block.push( blockRotate( d_cell[cidx], tile_rad_rot ) );
        //}

        rot_lib[dock_key] = rep_list.length;

        rep_list.push({
          "source_name": name,
          "name": tile_name,
          "irot": tile_irot,
          "rot": tile_rad_rot,
          //"block" : tile_block,
          //"geom": tile_geom,
          "dock": cur_dock
        });

      }

      cur_dock = dock_permutation(cfg.symmetry, cur_dock);
      _incr_rot_idx(irot, cfg.symmetry);
    } while ((irot[0] != 0) ||
             (irot[1] != 0) ||
             (irot[2] != 0));

  }

  /*
  console.log(rep_list);
  console.log("rot_lib:");
  for (let k in rot_lib) {
    console.log(k, "   ", rot_lib[k]);
  }
  */

  /*
  let rot_match = {};

  // dock_idx is really subtile index that indexes the tile
  // in the pattern group
  //
  for (let dock_idx=0; dock_idx<info.dock.length; dock_idx++) {

    rot_match[dock_idx] = {};
    rot_match[dock_idx]["rot"] = {};

    let src_dock = info.dock[dock_idx];
    let dst_dock = info.dock[dock_idx];

    let src_irot = [0,0,0];
    do {

      let src_dock_key = dock_idx.toString() + "," + src_dock.join(",");
      let src_irot_key = [ src_irot[0].toString(), src_irot[1].toString(), src_irot[2].toString() ].join(",");

      rot_match[dock_idx].rot[src_irot_key] = {};

      let dst_irot = [0,0,0];
      do {

        let dst_dock_key = dock_idx.toString() + "," + dst_dock.join(",");
        let dst_irot_key = [ dst_irot[0].toString(), dst_irot[1].toString(), dst_irot[2].toString() ].join(",");

        rot_match[dock_idx].rot[src_irot_key][dst_irot_key] = {
          "src_dock": src_dock,
          "dst_dock": dst_dock,
          "src_dock_key": src_dock_key,
          "dst_dock_key": dst_dock_key,
          "src_irot": [ src_irot[0], src_irot[1], src_irot[2] ],
          "dst_irot": [ dst_irot[0], dst_irot[1], dst_irot[2] ],
          "src_repr_idx": rot_lib[ src_dock_key ],
          "dst_repr_idx": rot_lib[ dst_dock_key ]
        };

        dst_dock = dock_permutation(cfg.symmetry, dst_dock);
        _incr_rot_idx(dst_irot, cfg.symmetry);
      } while ((dst_irot[0] != 0) ||
               (dst_irot[1] != 0) ||
               (dst_irot[2] != 0));

      src_dock = dock_permutation(cfg.symmetry, src_dock);
      _incr_rot_idx(src_irot, cfg.symmetry);
    } while ((src_irot[0] != 0) ||
             (src_irot[1] != 0) ||
             (src_irot[2] != 0));
    */

    //----
    //----
    /*
    cur_dock = info.dock[dock_idx];
    irot = [0,0,0];
    do {
      let flip_dock = dock_permutation('fx', cur_dock);

      let cur_dock_key  = cur_dock.join(",");
      let flip_dock_key = flip_dock.join(",");

      if ((cur_dock_key != flip_dock_key) &&
          (cur_dock_key in rot_fam) &&
          (flip_dock_key in rot_fam)) {
        console.log(name, ">>", cur_dock, "flip:", flip_dock);
      }

      cur_dock = dock_permutation(cfg.symmetry, cur_dock);
      _incr_rot_idx(irot, cfg.symmetry);
    } while ((irot[0] != 0) ||
             (irot[1] != 0) ||
             (irot[2] != 0));
    */
    //----
    //----


    //DEBUG
    /*
    console.log(name);
    for (let dock_key in rot_fam) {
      console.log("rot_fam[", dock_key, "]:");
      for (let ii=0; ii<rot_fam[dock_key].fam_irot.length; ii++) {
        console.log("  ", JSON.stringify(rot_fam[dock_key].fam_irot[ii]) );
      }
    }
    */

  //}

/*
  console.log(">>>>");
  console.log(name);
  for (let dock_key in rot_match) {
    for (let src_rot_key in rot_match[dock_key].rot) {
      for (let dst_rot_key in rot_match[dock_key].rot[src_rot_key]) {
        console.log("[", dock_key, "][", src_rot_key, "][", dst_rot_key, "]:", rot_match[dock_key].rot[src_rot_key][dst_rot_key]);
      }
    }
  }
  //console.log(rot_match);
  console.log("<<<<");
*/


  return {"repr_list": rep_list, "rot_idx_map": rot_lib, "source": info, "name": name};
}

function matchSymmetry(cfg, src_rep_info, dstRepList) {

  let rot_match = {};

  let template_list = src_rep_info.source;
  let name = src_rep_info.name;

  src_rep_list = src_rep_info.repr_list;
  let rot_idx_map = src_rep_info.rot_idx_map;

  // dock_idx is really subtile index that indexes the tile
  // in the pattern group
  //
  for (let dock_idx=0; dock_idx<template_list.dock.length; dock_idx++) {

    rot_match[dock_idx] = {};
    rot_match[dock_idx]["rot"] = {};

    let src_dock = template_list.dock[dock_idx];
    let dst_dock = template_list.dock[dock_idx];

    let src_irot = [0,0,0];
    do {

      let src_dock_key = dock_idx.toString() + "," + src_dock.join(",");
      let src_irot_key = [ src_irot[0].toString(), src_irot[1].toString(), src_irot[2].toString() ].join(",");

      rot_match[dock_idx].rot[src_irot_key] = {};

      let dst_irot = [0,0,0];
      do {

        let dst_dock_key = dock_idx.toString() + "," + dst_dock.join(",");
        let dst_irot_key = [ dst_irot[0].toString(), dst_irot[1].toString(), dst_irot[2].toString() ].join(",");

        rot_match[dock_idx].rot[src_irot_key][dst_irot_key] = {
          "src_dock": src_dock,
          "dst_dock": dst_dock,
          "src_dock_key": src_dock_key,
          "dst_dock_key": dst_dock_key,
          "src_irot": [ src_irot[0], src_irot[1], src_irot[2] ],
          "dst_irot": [ dst_irot[0], dst_irot[1], dst_irot[2] ],
          "src_repr_idx": rot_idx_map[ src_dock_key ],
          "dst_repr_idx": rot_idx_map[ dst_dock_key ]
        };

        dst_dock = dock_permutation(cfg.symmetry, dst_dock);
        _incr_rot_idx(dst_irot, cfg.symmetry);
      } while ((dst_irot[0] != 0) ||
               (dst_irot[1] != 0) ||
               (dst_irot[2] != 0));

      src_dock = dock_permutation(cfg.symmetry, src_dock);
      _incr_rot_idx(src_irot, cfg.symmetry);
    } while ((src_irot[0] != 0) ||
             (src_irot[1] != 0) ||
             (src_irot[2] != 0));

  }

  /*
  console.log(">>>>");
  console.log(name);
  for (let dock_key in rot_match) {
    for (let src_rot_key in rot_match[dock_key].rot) {
      for (let dst_rot_key in rot_match[dock_key].rot[src_rot_key]) {
        console.log("[", dock_key, "][", src_rot_key, "][", dst_rot_key, "]:", rot_match[dock_key].rot[src_rot_key][dst_rot_key]);
      }
    }
  }
  //console.log(rot_match);
  console.log("<<<<");
  */


}

// still some hardcoded values but getting closer to being finished.
//
// stickem...conf file has a character (string) with space seperator
// to denote which other valid tiles it can dock with.
// There are special codes beginning with a dollar sign that represent
// internal neighbors, for geometries that would span multiple unit tiles.
//
// Here y+ is considered 'up'
//
// * Create representatives
//   - rotate each base `.obj` around the axies of symmetry and deduplicate
//   - keep mappings of deduplicated names to representative
//   - special consideration for empty tile '.' (tile id 0)
//     and ground tile '#' (tile id 1)
//   - naming conviention is `<basename>_<rotcode>_<ele>` with
//     ele increasing for geometries that span multiple unit blocks
// * assign ids to represntatives
//   - again, special consideration for empty tile '.' (tile id 0) and
//     ground tile '#' (tile id 1)
// * create rules
//   - hard code empty and ground tile rules, with empty-empty in all directions,
//     ground-ground in all directions, and ground-empty in all directions except
//     ground-empty in the downward direction (empty can't be below ground)
//   - create map from dock token and idir to array of representatives for
//     easy lookup
//   - match up all representatives to each other if they share a docking token
//     in the appropriate direction (src idir to dst, dst rdir to src)
//   - special considering for empty ('.'), 'underneath' ('_'), ground ('#') and
//     pylon (':') docking tokens is taken
//     + empty docking token forces empty tile neighboring (so don't connect
//       representatives that both have a '.' docking token and only allow them
//       to connect to the empty tile)
//     + same with ground docking token ('#')
//     + underneath docking token ('_') forces either an empty or ground
//     + pylon docking token (':') allows empty or ground but still allows other
//       tiles to connect to it
//
// That last one with the pylon can be taken care of by adding a '_' to the pylon docking
// token.
//
//
//function _main(conf_fn, base_dir, out_base_dir, _out_type) {
function _main(conf_fn, out_base_dir, _out_type) {

  //var cfg = JSON.parse( fs.readFileSync("./data/stickem_minigolf.conf") );
  //let base_dir = "./data/minigolf.obj";

  var cfg = JSON.parse( fs.readFileSync(conf_fn) );

  let base_dir = cfg.obj_dir;

  if (!("n_dim" in cfg)) { cfg.n_dim = 3; }

  let stickem_info = {
    "basename": [],
    "repr" : [],

    "repr_idx_map": {},
    "name_repr_map": {},

    "basename_rep_idx" : {}
  };

  let = dock_info = {
    "force_token" : {},
    "pair_token": {},
    "expand_token" : {},
    "simple_token": {},

    "base_name_repr_idx": {}
  };

  for (let dock_tok in cfg.dock) {
    let dock_ele = cfg.dock[dock_tok];

    if (dock_ele.type == '!') {
      dock_info.force_token[dock_tok] = dock_ele.dock;
    }
    else if (dock_ele.type == '%') {
      dock_info.expand_token[dock_tok] = dock_ele.dock;
    }
    else if (dock_ele.type == '@') {
      dock_info.simple_token[dock_tok] = dock_tok;
    }
    else if (dock_ele.type == '&') {
      dock_info.pair_token[dock_tok] = dock_ele.dock;
    }
  }

  // 'macro' expand ('%' token)
  //
  for (let idx=0; idx<cfg.source.length; idx++) {
    let src_ele = cfg.source[idx];

    for (let dock_idx=0; dock_idx<src_ele.dock.length; dock_idx++) {
      for (let idir=0; idir<src_ele.dock[dock_idx].length; idir++) {

        for (let expand_tok in dock_info.expand_token) {
          src_ele.dock[dock_idx][idir] = src_ele.dock[dock_idx][idir].replace( expand_tok, dock_info.expand_token[expand_tok]);
        }

      }
    }

  }


  // create representative list, deduplicating identical docking patterns
  // depending on rotation from symmetry
  //
  for (let idx=0; idx<cfg.source.length; idx++) {
    let name = cfg.source[idx].name;

    let rep_info = createRepresentative(cfg, cfg.source[idx]);
    let rl = rep_info.repr_list;

    matchSymmetry(cfg, rep_info);

    for (let ii=0; ii<rl.length; ii++) {
      stickem_info.repr.push(rl[ii]);
    }
  }

  let tile_name = [];
  let tile_name_to_id = {};

  if (cfg.n_dim == 2) {
    tile_name.push("._000_0");
    tile_name_to_id['._000_0'] = 0;
    dock_info.base_name_repr_idx["."] = [0];
  }
  else {
    tile_name.push("._000_0");
    tile_name.push("#_000_0");

    tile_name_to_id['._000_0'] = 0;
    tile_name_to_id['#_000_0'] = 1;

    dock_info.base_name_repr_idx["."] = [0];
    dock_info.base_name_repr_idx["#"] = [1];
  }

  // assign ids to representatives
  //
  let cur_id = tile_name.length;

  let _rep_list = stickem_info.repr;
  for (let ii=0; ii<_rep_list.length; ii++) {

    tile_name.push( _rep_list[ii].name );
    tile_name_to_id[ _rep_list[ii].name ] = cur_id;
    _rep_list[ii]["id"] = cur_id;

    if (!(_rep_list[ii].source_name in dock_info.base_name_repr_idx)) {
      dock_info.base_name_repr_idx[ _rep_list[ii].source_name ] = [];
    }
    dock_info.base_name_repr_idx[ _rep_list[ii].source_name ].push(cur_id);

    cur_id++;
  }

  // create rules
  //
  let rule_list = [];
  let rule_map = {};
  let dock_tok_to_id = {};

  // rules for empty-empty, empty-ground and ground-ground connections
  let _empty_tile = 0,
      _ground_tile = 1;
  for (let idir=0; idir<6; idir++) { rule_list.push( [_empty_tile,_empty_tile, idir, 1] ); }
  rule_list.push([_empty_tile, _ground_tile, 0, 1]);
  rule_list.push([_empty_tile, _ground_tile, 1, 1]);
  rule_list.push([_empty_tile, _ground_tile, 4, 1]);
  rule_list.push([_empty_tile, _ground_tile, 5, 1]);
  rule_list.push([_empty_tile, _ground_tile, 3, 1]);

  rule_list.push([_ground_tile, _empty_tile, 0, 1]);
  rule_list.push([_ground_tile, _empty_tile, 1, 1]);
  rule_list.push([_ground_tile, _empty_tile, 4, 1]);
  rule_list.push([_ground_tile, _empty_tile, 5, 1]);
  rule_list.push([_ground_tile, _empty_tile, 2, 1]);

  rule_list.push([_ground_tile, _ground_tile, 0, 1]);
  rule_list.push([_ground_tile, _ground_tile, 1, 1]);
  rule_list.push([_ground_tile, _ground_tile, 2, 1]);
  rule_list.push([_ground_tile, _ground_tile, 3, 1]);
  rule_list.push([_ground_tile, _ground_tile, 4, 1]);
  rule_list.push([_ground_tile, _ground_tile, 5, 1]);


  let opp_idir = [ 1,0, 3,2, 5,4 ];

  let docktok_to_id = [ {}, {}, {}, {}, {}, {} ];

  // build structure for easy lookup
  //
  for (let repr_idx=0; repr_idx<_rep_list.length; repr_idx++) {
    let _repr = _rep_list[repr_idx];
    let _dock = _repr.dock;

    for (let idir=0; idir<_dock.length; idir++) {
      let toks = _dock[idir].split(" ");
      for (let tok_idx=0; tok_idx<toks.length; tok_idx++) {
        let tok = toks[tok_idx];

        if (tok == '.') { continue; }

        if (!(tok in dock_tok_to_id)) { dock_tok_to_id[tok] = [ [], [], [], [], [], [] ]; }

        dock_tok_to_id[tok][idir].push( {"id": _repr.id, "idir": idir } );
      }
    }
  }

  // now use it to build rule list
  //
  for (let repr_idx=0; repr_idx<_rep_list.length; repr_idx++) {

    let _repr = _rep_list[repr_idx];
    let _dock = _repr.dock;

    let src_tile_id = _repr.id;

    for (let idir=0; idir<_dock.length; idir++) {
      let toks = _dock[idir].split(" ");

      let rdir = opp_idir[idir];

      for (let tok_idx=0; tok_idx<toks.length; tok_idx++) {
        let tok = toks[tok_idx];

        // '!' docking token
        //
        if (tok in dock_info.force_token) {
          for (let dock_idx=0; dock_idx<dock_info.force_token[tok].length; dock_idx++) {

            let ftok = dock_info.force_token[tok][dock_idx];

            if ((ftok == 0) || (ftok == 1)) {
              rule_list.push( [_repr.id, dock_info.force_token[tok][dock_idx], idir, 1 ] );
              rule_list.push( [dock_info.force_token[tok][dock_idx], _repr.id, rdir, 1 ] );
            }
            else {

              let rep_idx_a = dock_info.base_name_repr_idx[ftok];

              for (let _ii=0; _ii<rep_idx_a.length; _ii++) {
                let _gobble_tile_id = rep_idx_a[_ii];

                rule_list.push( [_repr.id, _gobble_tile_id, idir, 1] );
                rule_list.push( [_gobble_tile_id, _repr.id, rdir, 1] );
              }

            }
          }
          continue;
        }

        // If we've matched an internal link, add the rule and
        // move on.
        //
        let tok_r = tok.match( '^\\$(\\d+)' );
        if (tok_r) {

          let nei_tile_name = _repr.name.split("_").slice(0,-1).join("_") + "_" + tok_r[1];

          let nei_tile_id = tile_name_to_id[ nei_tile_name ];
          rule_list.push( [src_tile_id, nei_tile_id, idir, 1] );
          rule_list.push( [nei_tile_id, src_tile_id, rdir, 1] );

          continue;
        }

        // change destination docking token if it's a pair token
        //
        if (tok in dock_info.pair_token) {
          tok = dock_info.pair_token[tok];
        }


        // Otherwise pair up all other tiles with the same docking token
        // in the relevant direction.
        //
        let nei_list = dock_tok_to_id[tok][rdir];
        for (let nei_idx=0; nei_idx < nei_list.length; nei_idx++) {
          rule_list.push( [src_tile_id, nei_list[nei_idx].id, idir, 1] );
        }

      }
    }

  }

  let idir_name = [ "x+", "x-", "y+", "y-", "z+", "z-" ];

  let poms_data = {
    "rule": [],
    "name": [],
    "weight": [],
    "objMap": [],
    "constraint": [
      //{"type":"quiltForce", "range": {"tile":[1,2], "x":[], "y":[0,1], "z":[], "note": "force ground tile on bottom xz plane, y+ up"} },
      //{"type":  "quiltPin", "range": {"tile":[1,2], "x":[], "y":[0,1], "z":[], "note": "pin previously forced operation"} },
      //{"type":"quiltRemove","range": {"tile":[1,2], "x":[], "y":[1],   "z":[], "note": "remove ground from rest of grid"} }
    ],
    "boundaryCondition":{
      "x+":{"type":"tile","value":0},
      "x-":{"type":"tile","value":0},
      "y+":{"type":"tile","value":0},
      "y-":{"type":"tile","value":0},
      "z+":{"type":"tile","value":0},
      "z-":{"type":"tile","value":0}
    },
    "size": [8,8,8],
    "quiltSize": [8,8,8]
  };

  //let out_base_dir = ".minigolf_tile";

  poms_data.rule = rule_list;
  poms_data.name = tile_name;
  for (let ii=0; ii<poms_data.name.length; ii++) {
    poms_data.weight.push(1);
  }

  //----
  // use weights in config
  //
  if ("weight" in cfg) {
    for (let src_name in cfg.weight) {
      let weight = cfg.weight[src_name];

      if (!(src_name in dock_info.base_name_repr_idx)) { continue; }

      let rep_idx_a = dock_info.base_name_repr_idx[src_name];

      for (let _idx=0; _idx<rep_idx_a.length; _idx++) {
        let tile_idx = rep_idx_a[_idx];

        poms_data.weight[tile_idx] = weight;
      }


    }
    
  }

  if ("constraint" in cfg) {
    for (let cidx=0; cidx<cfg.constraint.length; cidx++) {

      let n = poms_data.name.length;

      let tile_pattern = cfg.constraint[cidx].range.tile;

      let tile_list = [];

      for (let tile_idx=0; tile_idx < n; tile_idx++) {
        if (poms_data.name[tile_idx].match( tile_pattern )) {
          tile_list.push(tile_idx);
        }
      }

      if (tile_list.length == 0) { continue; }


      tile_list.sort( function(a,b) { if (a<b) { return -1; } if (a>b) { return 1; } return 0; } );

      let tile_range = [];
      let cur_range = [ tile_list[0], tile_list[0] + 1 ];
      for (let tile_list_idx=1; tile_list_idx < tile_list.length; tile_list_idx++) {
        if (tile_list[tile_list_idx] != cur_range[1]) {
          tile_range.push( [ cur_range[0], cur_range[1] ] );
          cur_range[0] = tile_list[tile_list_idx];
        }
        cur_range[1] = tile_list[tile_list_idx]+1;
      }
      tile_range.push( [ cur_range[0], cur_range[1] ] );

      let _tile_range = 0;

      for (let tile_range_idx=0; tile_range_idx < tile_range.length; tile_range_idx++) {
        let _constraint = {
          "type": cfg.constraint[cidx].type,
          "range": {
            "x": cfg.constraint[cidx].range.x,
            "y": cfg.constraint[cidx].range.y,
            "z": cfg.constraint[cidx].range.z,
            "tile": tile_range[tile_range_idx]
          }
        };

        poms_data.constraint.push( _constraint );
      }

    }
  }


  //
  //----

  if (_out_type == "obj") {

    let empty_obj = out_base_dir + "/" + "._000_0.obj";
    let ground_obj = out_base_dir + "/" + "#_000_0.obj";

    fs.writeFileSync( empty_obj, "v 0 0 0\nv 0 0 0\nv 0 0 0\n\nf 1 1 1\n");
    fs.writeFileSync( ground_obj, "v 0 0 0\nv 0 0 0\nv 0 0 0\n\nf 1 1 1\n");

    empty_obj = out_base_dir + "/" + "e_000_0.obj";
    ground_obj = out_base_dir + "/" + "g_000_0.obj";

    fs.writeFileSync( empty_obj, "v 0 0 0\nv 0 0 0\nv 0 0 0\n\nf 1 1 1\n");
    fs.writeFileSync( ground_obj, "v 0 0 0\nv 0 0 0\nv 0 0 0\n\nf 1 1 1\n");

    for (let ii=0; ii<poms_data.name.length; ii++) {

      if (poms_data.name[ii] == "._000_0") {
        poms_data.objMap.push( out_base_dir + "/" + "e_000_0"  + ".obj" );
      }
      else if (poms_data.name[ii] == "#_000_0") {
        poms_data.objMap.push( out_base_dir + "/" + "g_000_0"  + ".obj" );
      }
      else {
        poms_data.objMap.push( out_base_dir + "/" + poms_data.name[ii] + ".obj" );
      }

      let tok = poms_data.name[ii].split("_");
      let subtile_id = tok.slice(-1);
      let rotcode = tok.slice(-2).slice(0,1).join("");

      let source_name = tok.slice(0, tok.length-2).join("_") ;

      if ((source_name == '.') ||
          (source_name == '#')) { continue; }

      let json_obj = jeom.obj2json( fs.readFileSync(base_dir + "/" + source_name + ".obj") );

      let axis = '';
      let theta = 0;
      let _rc = rotcode.split("");
      if (_rc[2] != '0') {
        theta = -parseFloat(_rc[2])*Math.PI/2.0;
        json_obj = jeom.json_obj_transform(json_obj, [0,0,0], theta, 'z');
      }

      if (_rc[1] != '0') {
        theta = -parseFloat(_rc[1])*Math.PI/2.0;

        json_obj = jeom.json_obj_transform(json_obj, [0,0,0], theta, 'y');
      }

      if (_rc[0] != '0') {
        theta = -parseFloat(_rc[0])*Math.PI/2.0;
        json_obj = jeom.json_obj_transform(json_obj, [0,0,0], theta, 'x');
      }

      let sfx = poms_data.name[ii].split("_").slice(-1).join("");
      if (sfx == "0") {

        fs.writeFileSync( out_base_dir + "/" + poms_data.name[ii] + ".obj", jeom.json2obj(json_obj) );

        if (fs.existsSync( base_dir + "/" + source_name + ".mtl" )) {
          fs.writeFileSync( out_base_dir + "/" + poms_data.name[ii] + ".mtl", fs.readFileSync( base_dir + "/" + source_name + ".mtl" ));
        }
        else {
          fs.writeFileSync( out_base_dir + "/" + poms_data.name[ii] + ".mtl", "" );
        }

      }
      else {

        let placeholder_obj_str = "#placeholder empty\nv 0 0 0\nv 0 0 0\nv 0 0 0\n\nf 1 1 1\n";


        //fs.writeFileSync( out_base_dir + "/" + poms_data.name[ii] + ".obj", "" );
        fs.writeFileSync( out_base_dir + "/" + poms_data.name[ii] + ".obj", placeholder_obj_str );
        fs.writeFileSync( out_base_dir + "/" + poms_data.name[ii] + ".mtl", "" );
      }

    }

  }
  else if (_out_type == "stl") {

    let empty_stl = out_base_dir + "/" + "._000_0.stl";
    let ground_stl = out_base_dir + "/" + "#_000_0.stl";

    fs.writeFileSync( empty_stl, "solid EMPTY\nendsolid EMPTY\n" );
    fs.writeFileSync( ground_stl, "solid EMPTY\nendsolid EMPTY\n" );

    for (let ii=0; ii<poms_data.name.length; ii++) {
      poms_data.objMap.push( out_base_dir + "/" + poms_data.name[ii] + ".stl" );

      let tok = poms_data.name[ii].split("_");
      let subtile_id = tok.slice(-1);
      let rotcode = tok.slice(-2).slice(0,1).join("");

      let source_name = tok.slice(0, tok.length-2).join("_") ;

      if ((source_name == '.') ||
          (source_name == '#')) { continue; }

      let geom = op.stl_loads({"output":"geometry"}, fs.readFileSync(base_dir + "/" + source_name + ".stl"));

      let axis = '';
      let theta = 0;
      let _rc = rotcode.split("");
      if (_rc[2] != '0') {
        theta = -parseFloat(_rc[2])*Math.PI/2.0;
        geom = op.rotZ(theta, geom);
      }

      if (_rc[1] != '0') {
        theta = -parseFloat(_rc[1])*Math.PI/2.0;
        geom = op.rotY(theta, geom);
      }

      if (_rc[0] != '0') {
        theta = -parseFloat(_rc[0])*Math.PI/2.0;
        geom = op.rotX(theta, geom);
      }

      let sfx = poms_data.name[ii].split("_").slice(-1).join("");
      if (sfx == "0") {
        fs.writeFileSync( out_base_dir + "/" + poms_data.name[ii] + ".stl", op.stl_dumps({"binary":false},geom).join("").toString() );
      }
      else {
        fs.writeFileSync( out_base_dir + "/" + poms_data.name[ii] + ".stl", "solid EMPTY\nendsolid EMPTY\n" );
      }

    }


  }

  console.log(JSON.stringify(poms_data, undefined, 2));

}

function show_help() {
  console.log("");
  console.log("usage:");
  console.log("");
  console.log("  node stickem.js <stickem_conf> [out_dir] [out_type]");
  console.log("");
  console.log("    stickem_conf       - stickem config file (required)");
  console.log("    out_dir            - out directory for 3d object files");
  console.log("    out_type           - one of 'obj' or 'stl'");
  console.log("");
  console.log("will print out POMS config file to stdout");
  console.log("");
}

function cli_main(argv) {

  if (argv.length < 3) {
    show_help();
    return -1;
  }

  if (argv[2] == "help") { show_help(); return 0; }

  let conf_fn = argv[2];
  let out_dir = "out";
  let out_type = "obj";

  if (argv.length > 3) { out_dir = argv[3]; }
  if (argv.length > 4) { out_type = argv[4]; }

  _main( conf_fn, out_dir, out_type );


}

cli_main(process.argv);

//_main("./data/stickem_minigolf.conf", "./data/minigolf.obj", ".minigolf_tile", "obj");
//_main("./data/stickem_brutal-plum.conf", ".brutal-plum_stl", ".brutal-plum_tile", "stl");
//_main("./data/stickem_brutal-plum.conf", ".brutal-plum_obj", ".brutal-plum_tile", "obj");
//_main("./data/stickem_twoloop.conf", "", "", "none");
//_main("./data/stickem_brutal-plum_1.conf", ".brutal-plum_obj", "brutal-plum_tile", "obj");
//_main("./data/stickem_brutal-plum_2path.conf", ".brutal-plum_obj", "brutal-plum_tile", "obj");


