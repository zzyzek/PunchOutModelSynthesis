// LICENSE: CC0
//
// To the extent possible under law, the person who associated CC0 with
// this project has waived all copyright and related or neighboring rights
// to this project.
// 
// You should have received a copy of the CC0 legalcode along with this
// work. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//


// Y is 'up'
// XZ plan is flat, Z comes into camera, X goes to the right
//
// Everything is centered aroudn 0 with unit cube cell (+-0.5, +-0.5, +-0.5)
//

let OUT_DIR = "brutal-plum_tile";
let OUT_DIR_STL = "brutal-plum_stl";
let OUT_DIR_OBJ = "brutal-plum_obj";

var fs = require("fs");

var jscad = require("@jscad/modeling");
var array_utils = require('@jscad/array-utils')


var objectDeserializer = require('@jscad/obj-deserializer')
var objectSerializer = require('@jscad/obj-serializer')
var stlDeserializer = require('@jscad/stl-deserializer')
var stlSerializer = require('@jscad/stl-serializer')

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
  "cub": jscad.primitives.cuboid,
  "cylinder": jscad.primitives.cylinder,
  "sphere": jscad.primitives.sphere,

  "ruboid": jscad.primitives.roundedCuboid,

  "circle": jscad.primitives.circle,
  "cir": jscad.primitives.circle,

  "lift": jscad.extrusions.extrudeLinear,
  "lif": jscad.extrusions.extrudeLinear,

  "lifr": jscad.extrusions.extrudeRotate,
  "lifc": jscad.extrusions.extrudeRectangular,
  "lifh": jscad.extrusions.extrudeHelical,

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

function debug_geom_f(size, center, opt) {

  opt = ((typeof opt === "undefined") ? [(1/32), (1/32), (1/32)] : opt);

  let _s = ((typeof size === "undefined") ? [1,1,1] : size);
  let _c = ((typeof center === "undefined") ? [0,0,0] : center);
  let _s2 = [ _s[0]/2, _s[1]/2, _s[2]/2 ];

  let _p = [
    [ _c[0]-_s2[0], _c[0]+_s2[0] ],
    [ _c[1]-_s2[1], _c[1]+_s2[1] ],
    [ _c[2]-_s2[2], _c[2]+_s2[2] ]
  ];

  let _debug_geom =
    op.add(
      op.mov([ _p[0][0], _p[1][0], _p[2][0] ], op.cub({"size":opt})),
      op.mov([ _p[0][1], _p[1][0], _p[2][0] ], op.cub({"size":opt})),
      op.mov([ _p[0][0], _p[1][1], _p[2][0] ], op.cub({"size":opt})),
      op.mov([ _p[0][0], _p[1][0], _p[2][1] ], op.cub({"size":opt})),

      op.mov([ _p[0][1], _p[1][1], _p[2][0] ], op.cub({"size":opt})),
      op.mov([ _p[0][0], _p[1][1], _p[2][1] ], op.cub({"size":opt})),
      op.mov([ _p[0][1], _p[1][0], _p[2][1] ], op.cub({"size":opt})),

      op.mov([ _p[0][1], _p[1][1], _p[2][1] ], op.cub({"size":opt}))
    );

  return _debug_geom;
}
//var DEBUG_GEOM = debug_geom_f([1,1,1], [0.5,0.5,0.5]);
var DEBUG_GEOM = debug_geom_f([1,1,1], [0,0,0]);

// untested, abandonded?
//
function make_arch(r) {
  r = ( ((typeof r) === "undefined") ? 1 : r);

  let all_pnts = [];

  for (let x=0; x<(r+1); x++) {
    let d = (r*r) - (x*x);
    if (d < 0) { continue; }
    let y = Math.sqrt(d);

    all_pnts.push([x,y]);

    console.log("x>", x,y);
  }

  for (let y=0; y<(r+1); y++) {
    let d = (r*r) - (y*y);
    if (d < 0) { continue; }
    let x = Math.sqrt(d);

    all_pnts.push([x,y]);

    console.log("y>", x,y);
  }

  all_pnts.sort(pnt_cmp);

  let pnts = [];
  pnts.push( all_pnts[0] );
  for (let ii=1; ii<all_pnts.length; ii++) {
    if (! pnt_eq(pnts[ pnts.length-1 ], all_pnts[ii])) {
      pnts.push( all_pnts[ii] );
    }
  }

  let info_a = [];

  for (let ii=1; ii<pnts.length; ii++) {

    let prv = pnts[ii-1];
    let cur = pnts[ii];

    let pxy = [ prv[0] + ((cur[0]-prv[0])/2), prv[1] + ((cur[1]-prv[1])/2) ];
    let ix = Math.floor(pxy[0]);
    let iy = Math.floor(pxy[1]);

    let theta_s = Math.atan2(prv[1], prv[0]);
    let theta_e = Math.atan2(cur[1], cur[0]);
    console.log("prv:", prv, "cur:", cur, "pxy:", pxy, "ixy:", ix, iy, "theta_se:", theta_s, theta_e);
    let info = { };
  }

  //for (let ii=0; ii<pnts.length; ii++) { console.log(pnts[ii][0], pnts[ii][1]); }

}

function _main() {
  make_arch(2);
}

function wedge_up() {
  let geom = op.sub(
    op.cub({"size":[1,1,1], "center":[0, 0, 0]}),
    op.rot([Math.PI/4,0,0], op.cub({"size":[2,2,2], "center":[0,0.0,-1.0]}))
  );

  return [
    {"ds":[0,0,0], "geom":geom, "id":"w", "dock":[ ": .", ": .",  ".", "_",  "b", "."], "anchor":geom}
  ];
}

function wedge_down() {
  let geom = op.sub(
    op.cub({"size":[1,1,1], "center":[0, 0, 0]}),
    op.rot([Math.PI/4,0,0], op.cub({"size":[2,2,2], "center":[0,0,1]}))
  );

  return [
    {"ds":[0,0,0], "geom":geom, "id":"m", "dock":[ ": .", ": .", "b",".", ".","b"], "anchor":geom}
  ];
}

// y up
// stairs facing -z direction, sides in +-x
// slabs in xz plane, going up y
//
// bottom step needs to start in a step and top step
// needs to be missing as the diagonal element will
// be a full block, taking the place of the steps.
//
function stair(n, _debug) {
  n = ((typeof n === "undefined") ? 3 : n);

  let geom = {};

  let N = n+1;
  for (let i=0; i<n; i++) {
    let zi = i - ((n-1)/2);

    let sx = 1,
        sy = 1/N,
        sz = (n-i)/N;

    let cx = 0,
        cy = (zi/N) - (1/(2*N)),
        cz = ((i+1)/(2*N));
    let s = op.cub({"size":[sx,sy,sz], "center":[cx,cy,cz]});


    if (i==0) { geom = s; }
    else      { geom = op.add(geom, s); }

  }

  if (_debug) {
    geom = op.add(geom, DEBUG_GEOM);
  }

  return [
    {"ds":[0,0,0], "geom":geom, "id":"b", "dock":[ ": .", ": .", ".","_ Py", "b","."], "anchor":geom}
  ];
}

function block() {

  let geom = op.mov([0,0,0], op.cub({"size":[1,1,1]}));
  return [
    {"ds":[0,0,0], "geom":geom, "id":"b", "dock":[ ": .", ": .", "b Py","_", ": .", ": ."], "anchor":geom}
  ];
}

function block_b() {

  let geom = op.mov([0,0,0], op.cub({"size":[1,1,1]}));
  return [
    { "ds":[0,0,0],
      "geom":geom, "id":"B",
      "dock":[
        //"Pxz b q0xz q1xz q2xz q3xz .",
        //"Pxz b q0xz q1xz q2xz q3xz .",
        "Pxz b q2xz q3xz .",
        "Pxz b q2xz q3xz .",
        "b Py",
        //"b q0y q1y q2y q3y #",
        "b q2y q3y #",
        //"Pxz b q0xz q1xz q2xz q3xz .",
        //"Pxz b q0xz q1xz q2xz q3xz ."],
        "Pxz b q2xz q3xz .",
        "Pxz b q2xz q3xz ."],
      "anchor":geom}
  ];
}

//---

function platform_bend() {
  let geom = op.mov([0,0,0], op.cub({"size":[1,1,1]}));
  return [
    {"ds":[0,0,0], "geom":geom, "id":"pb", "dock":[ "Pxz", ".", "b .","Py #", ".", "Pxz"], "anchor":geom}
    //{"ds":[0,0,0], "geom":geom, "id":"pb", "dock":[ "Pxz", ".", "b .","Py # .", ".", "Pxz"], "anchor":geom}
  ];
}

function platform_tee() {
  let geom = op.mov([0,0,0], op.cub({"size":[1,1,1]}));
  return [
    {"ds":[0,0,0], "geom":geom, "id":"pt", "dock":[ "Pxz", "Pxz", "b .","Py #", ".", "Pxz"], "anchor":geom}
    //{"ds":[0,0,0], "geom":geom, "id":"pt", "dock":[ "Pxz", "Pxz", "b .","Py #", ".", "Pxz"], "anchor":geom}
  ];
}

function platform_straight() {
  let geom = op.mov([0,0,0], op.cub({"size":[1,1,1]}));
  return [
    //{"ds":[0,0,0], "geom":geom, "id":"ps", "dock":[ "Pxz", "Pxz", "b .","Py #", ".", "."], "anchor":geom}
    {"ds":[0,0,0], "geom":geom, "id":"ps", "dock":[ "Pxz", "Pxz", "b .","Py # .", ".", "."], "anchor":geom}
  ];
}

function platform_cross() {
  let geom = op.mov([0,0,0], op.cub({"size":[1,1,1]}));
  return [
    {"ds":[0,0,0], "geom":geom, "id":"ps", "dock":[ "Pxz", "Pxz", "b .","Py #", "Pxz", "Pxz"], "anchor":geom}
  ];
}

function platform_cross_overhang() {
  let geom = op.mov([0,0,0], op.cub({"size":[1,1,1]}));
  return [
    {"ds":[0,0,0], "geom":geom, "id":"ps", "dock":[ "Pxz", "Pxz", "b .","Py # .", "Pxz", "Pxz"], "anchor":geom}
  ];
}

function platform_tee_overhang() {
  let geom = op.mov([0,0,0], op.cub({"size":[1,1,1]}));
  return [
    {"ds":[0,0,0], "geom":geom, "id":"pt", "dock":[ "Pxz", "Pxz", "b .","Py # .", ".", "Pxz"], "anchor":geom}
  ];
}

//---

// path bend has 2 path connectors and other connectors for platform blocks, as needed.
// That is, behaves like a platform block but must connect up to other path blocks so
// we can force embed a path into the instance if we filter the grid appropriately.
//
// xn - x negative direction
// zp - z positive direction
//
// path_id is used to create the path docking id
// block_perm_id cycles through the different valid platform connectors in the xz plane (0-3).
//
function path_bend(path_id, block_perm_id, _debug) {
  path_id = ((typeof path_id === "undefined") ? 0 : path_id);
  block_perm_id = ((typeof block_perm_id === "undefined") ? 0 : block_perm_id);

  let pid = path_id.toString();

  let xn_zp = [
    [".", "."],
    ["Pxz", "."],
    [".", "Pxz"],
    ["Pxz", "Pxz"]
  ];

  let xn = xn_zp[block_perm_id][0];
  let zp = xn_zp[block_perm_id][1];

  let geom = op.mov([0,0,0], op.cub({"size":[1,1,1]}));

  if (_debug) {
    geom = op.add( geom, op.cub({"center":[0.25,0.5,0], "size":[0.5,0.8,1/16]}) );
    geom = op.add( geom, op.cub({"center":[0,0.5,-0.25], "size":[1/16,0.8,0.5]}) );
  }

  return [
    //{"ds":[0,0,0], "geom":geom, "id":"path_bend_" + pid, "dock":[ "path" + pid, xn, "b .","Py", zp, "path" + pid], "anchor":geom}
    {"ds":[0,0,0], "geom":geom, "id":"path_bend_" + pid, "dock":[ "path" + pid, xn, ".","Py #", zp, "path" + pid], "anchor":geom}
  ];
}


function path_straight(path_id, block_perm_id, _debug) {
  path_id = ((typeof path_id === "undefined") ? 0 : path_id);
  block_perm_id = ((typeof block_perm_id === "undefined") ? 0 : block_perm_id);

  // symmetry takes care of the other direction
  //
  let zp_zn = [
    [".", "."],
    ["Pxz", "."],
    ["Pxz", "Pxz"]
  ];

  let zp = zp_zn[block_perm_id][0];
  let zn = zp_zn[block_perm_id][1];

  let pid = path_id.toString();
  let geom = op.mov([0,0,0], op.cub({"size":[1,1,1]}));

  if (_debug) {
    geom = op.add(geom, op.cub({"center":[0,0.5,0], "size":[1,0.8,1/16]}));
  }

  return [
    //{"ds":[0,0,0], "geom":geom, "id":"path_straight_" + pid, "dock":[ "path" + pid, "path" + pid, "b .","Py", zp, zn], "anchor":geom}
    {"ds":[0,0,0], "geom":geom, "id":"path_straight_" + pid, "dock":[ "path" + pid, "path" + pid, ".","Py #", zp, zn], "anchor":geom}
  ];
}

// connects to stairs upwards
//
function path_bend_up(path_id, block_perm_id, _debug) {
  path_id = ((typeof path_id === "undefined") ? 0 : path_id);
  block_perm_id = ((typeof block_perm_id === "undefined") ? 0 : block_perm_id);

  // want at least two platform like blocks on the same level?
  //
  let xn_zp_zn = [
    //[".", ".", "."],
    ["Pxz", ".", "."],
    [".", "Pxz", "."],
    ["Pxz", "Pxz", "."],
    [".", ".", "Pxz"],
    ["Pxz", ".", "Pxz"],
    [".", "Pxz", "Pxz"],
    ["Pxz", "Pxz", "Pxz"]
  ];

  let xn = xn_zp_zn[block_perm_id][0];
  let zp = xn_zp_zn[block_perm_id][1];
  let zn = xn_zp_zn[block_perm_id][2];

  let pid = path_id.toString();
  let geom = op.mov([0,0,0], op.cub({"size":[1,1,1]}));

  if (_debug) {
    geom = op.add(geom, op.cub({"center":[0.35,0.35,0], "size":[1, 1, 1/16]}));
  }

  return [
    {"ds":[0,0,0], "geom":geom, "id":"path_bendup_" + pid, "dock":[ "path" + pid, xn, "path" + pid,"Py #", zp, zn], "anchor":geom}
  ];
}


function path_stair(path_id, _debug) {
  path_id = ((typeof path_id === "undefined") ? 0 : path_id);
  let pid = path_id.toString();

  let s = stair(5);

  let path_dock_tok = "path" + pid;

  // 3 - y-
  // 4 - z+

  s[0].id = "path_stair_" + pid;

  if (_debug) {
    s[0].anchor = op.add( s[0].anchor, op.cub({"center":[0,0,0], "size":[1/16, 1, 1 ]}) );
  }

  s[0].dock[3] = "$1";
  s[0].dock[4] = path_dock_tok;

  let ublock = op.cub({"center":[0,-1,0], "size":[1,1,1]});
  s[0].anchor = op.add( s[0].anchor, ublock );

  s.push({ "ds":[0,0,0], "geom": ublock, "id":s[0].id, "dock":[": .", ": .", "$0", "_", ":", path_dock_tok ] });

  return s;
}

function path_cap(path_id, block_perm_id, _debug) {
  path_id = ((typeof path_id === "undefined") ? 0 : path_id);
  block_perm_id = ((typeof block_perm_id === "undefined") ? 0 : block_perm_id);


  // want at least two platform like blocks on the same level?
  //
  let xn_zp_zn = [
    [".", ".", "."],
    ["Pxz", ".", "."],
    [".", "Pxz", "."],
    ["Pxz", "Pxz", "."],
    [".", ".", "Pxz"],
    ["Pxz", ".", "Pxz"],
    [".", "Pxz", "Pxz"],
    ["Pxz", "Pxz", "Pxz"]
  ];

  let xn = xn_zp_zn[block_perm_id][0];
  let zp = xn_zp_zn[block_perm_id][1];
  let zn = xn_zp_zn[block_perm_id][2];

  let pid = path_id.toString();
  let geom = op.mov([0,0,0], op.cub({"size":[1,1,1]}));

  if (_debug) {
    geom = op.add( geom, op.cub({"center":[0.25,0.5,0], "size":[0.5, 0.8, 1/16 ]}) );
  }

  return [
    //{"ds":[0,0,0], "geom":geom, "id":"path_cap_" + pid, "dock":[ "path" + pid, xn, "b .","Py", zp, zn], "anchor":geom}
    {"ds":[0,0,0], "geom":geom, "id":"path_cap_" + pid, "dock":[ "path" + pid, xn, ".","Py #", zp, zn], "anchor":geom}
  ];
}

//---

function column2() {

  let geom = op.mov([0,0,0], op.cub({"size":[1,2,1], "center": [0,0.5,0] }));
  return [
    {"ds":[0,0,0], "geom":geom, "id":"b", "dock":[ ": .", ": .",  "$1",  "_", ": .", ": ."], "anchor":geom},
    //{"ds":[0,0,0], "geom":geom, "id":"b", "dock":[ ": .", ": .", "b", "$0", ": .", ": ."]}
    {"ds":[0,0,0], "geom":geom, "id":"b", "dock":[ ": .", ": .", "b Py", "$0", ": .", ": ."]}
  ];
}

function column3() {

  let geom = op.mov([0,0,0], op.cub({"size":[1,3,1], "center": [0,1,0] }));
  return [
    {"ds":[0,0,0], "geom":geom, "id":"b", "dock":[ ": .", ": .",  "$1",  "_", ": .", ": ."], "anchor":geom},
    {"ds":[0,0,0], "geom":geom, "id":"b", "dock":[ ": .", ": .",  "$2", "$0", ": .", ": ."] },
    //{"ds":[0,0,0], "geom":geom, "id":"b", "dock":[ ": .", ": .", "b .", "$1", ": .", ": ."] }
    {"ds":[0,0,0], "geom":geom, "id":"b", "dock":[ ": .", ": .", "b Py", "$1", ": .", ": ."] }
  ];
}


// smaller arch taken out of block
//
function doorway(opt, _debug) {

  opt = ((typeof opt === "undefined") ? {"r":0.25, "h":0.75}:opt);

  let r = (("r" in opt) ? opt.r : 0.25);
  let h = (("h" in opt) ? opt.h : 0.75);

  let _h = (h-r)/2;
  let _c = h-r-0.5;

  let geom =
    op.sub(
      op.mov([0,0,0], op.cub({"size":[1,1,1]})),
      op.mov([0,_c,-0.6], op.lif( {height:1.2}, op.cir({"radius":r})) ),
      op.mov([0,-_h,0], op.cub({"size":[2*r,2*_h,1.2]}))
    );

  if (_debug) {
    geom = op.add(geom, DEBUG_GEOM);
  }

  return [
    {"ds":[0,0,0], "geom":geom, "id":"a1", "dock":["b .", "b .",  "b", "_", ".", "."], "anchor":geom}
  ];
}

// smaller arch taken out of block
//
function double_doorway(opt, _debug) {

  opt = ((typeof opt === "undefined") ? {"r":0.25, "h":0.75}:opt);

  let r = (("r" in opt) ? opt.r : 0.25);
  let h = (("h" in opt) ? opt.h : 0.75);

  let _h = (h-r)/2;
  let _c = h-r-0.5;

  let geom =
    op.sub(
      op.mov([0,0,0], op.cub({"size":[1,1,1]})),
      op.mov([0,_c,-0.6], op.lif( {height:1.2}, op.cir({"radius":r})) ),
      op.rot([0,Math.PI/2,0], op.mov([0,_c,-0.6], op.lif( {height:1.2}, op.cir({"radius":r})) ) ),
      op.mov([0,-_h,0], op.cub({"size":[2*r,2*_h,1.2]})),
      op.mov([0,-_h,0], op.cub({"size":[1.2,2*_h,2*r]})),
    );

  if (_debug) {
    geom = op.add(geom, DEBUG_GEOM);
  }

  return [
    {"ds":[0,0,0], "geom":geom, "id":"a1", "dock":[ ".", ".", "b","_ 4",  ".", "."], "anchor":geom}
  ];
}

function block_2x2(opt, _debug) {

  opt = ((typeof opt === "undefined") ? {"r":0.25, "h":0.75}:opt);

  let r = (("r" in opt) ? opt.r : 0.25);

  let geom =
    op.sub(
      op.mov([0,0,0], op.cub({"size":[1,1,1]})),
      op.cub({"size":[2*r,1.2,1.2]}),
      op.cub({"size":[1.2,1.2,2*r]})
    );

  if (_debug) {
    geom = op.add(geom, DEBUG_GEOM);
  }

  return [
    {"ds":[0,0,0], "geom":geom, "id":"a1", "dock":[ ": .", ": .",  "4","_ 4", ": .", ": ."], "anchor":geom}
  ];

}

function arch0_s(opt, _debug) {
  let geom =
    op.sub( 
        op.mov([0,0.0,0], op.cub({"size":[1,1,1]})),
        op.mov([0,0,-0.5], op.lif( {height:1}, op.cir({"radius":0.5})) ),
        op.mov([0,-0.5,0], op.cub({"size":[1,1,1]}))
      );

  if (_debug) {
    geom = op.add(geom, DEBUG_GEOM);
  }

  return [
    //{"dock":[ "p0xz b","p0xz b", "p0y Py b", ". _",  ": .", ": ."], "anchor":geom},
    //{"dock":[ "p0xz b","p0xz b", "p0y Py b", ".",  ": .", ": ."], "anchor":geom},
    {"dock":[ "b","b", "Py b", ".",  ": .", ": ."], "anchor":geom},
  ];
}

function arch1_s(opt, _debug) {
  let geom =
    op.mov([0.5,-0.5,0],
      op.sub( 
        op.mov([0,0.0,0], op.cub({"size":[2,2,1]})),
        op.mov([0,0,-0.5], op.lif( {height:1}, op.cir({"radius":1.0})) ),
        op.mov([0,-1,0], op.cub({"size":[2,2,1]}))
      )
    );

  if (_debug) {
    geom = op.add(geom, DEBUG_GEOM);
  }

  return [
    //{"dock":[ "$1","p1xz b", "p1y Py b", ". _",  ": .", ": ."], "anchor":geom},
    //{"dock":[ "p1xz b","$0", "p1y Py b", ". _",  ": .", ": ."]}
    //{"dock":[ "$1","p1xz b", "p1y Py b", ".",  ": .", ": ."], "anchor":geom},
    //{"dock":[ "p1xz b","$0", "p1y Py b", ".",  ": .", ": ."]}
    {"dock":[ "$1","b", "Py b", ".",  ": .", ": ."], "anchor":geom},
    {"dock":[ "b","$0", "Py b", ".",  ": .", ": ."]}
  ];
}

function arch2_s(opt, _debug) {
  let geom =
    op.mov([1,0,0],
      op.sub( 
        op.mov([0,0.0,0], op.cub({"size":[3,3,1]})),
        op.mov([0,0,-0.5], op.lif( {height:1}, op.cir({"radius":1.5})) ),
        op.mov([0,-1.5,0], op.cub({"size":[3,3,1]}))
      )
    );

  if (_debug) {
    geom = op.add(geom, DEBUG_GEOM);
  }

  return [
    {"dock":[ ".","b",     "$2", ". _",  ": .", ": ."], "anchor":geom},
    {"dock":[ "b",".",     "$4", ". _",  ": .", ": ."]},

    //{"dock":[ ".","p2xz b",     "$2", ".",  ": .", ": ."], "anchor":geom},
    //{"dock":[ "p2xz b",".",     "$4", ".",  ": .", ": ."]},

    {"dock":[ "$3", "b",   "Py b", "$0",  ": .", ": ."]},
    {"dock":[ "$4","$2",   "Py b",  ".",  ": .", ": ."]},
    {"dock":[ "b", "$3",   "Py b", "$1",  ": .", ": ."]}

    //{"dock":[ "$3", "p2xz b",   "p2y Py b", "$0",  ": .", ": ."]},
    //{"dock":[ "$4","$2",        "p2y Py b",  ".",  ": .", ": ."]},
    //{"dock":[ "p2xz b", "$3",   "p2y Py b", "$1",  ": .", ": ."]}
  ];
}

function arch3_s(opt, _debug) {
  let geom =
    op.mov([1.5,-0.5,0],
      op.sub( 
        op.mov([0,0.0,0], op.cub({"size":[4,4,1]})),
        op.mov([0,0,-0.5], op.lif( {height:1}, op.cir({"radius":2})) ),
        op.mov([0,-2,0], op.cub({"size":[4,4,1]}))
      )
    );

  if (_debug) {
    geom = op.add(geom, DEBUG_GEOM);
  }

  return [
    {"dock":[ ".","b",  "$2", ". _",  ": .", ": ."], "anchor":geom},
    {"dock":[ "b",".",  "$5", ". _",  ": .", ": ."]},
    //{"dock":[ ".","p3xz b",  "$2", ".",  ": .", ": ."], "anchor":geom},
    //{"dock":[ "p3xz b",".",  "$5", ".",  ": .", ": ."]},

    {"dock":[ "$3", "b", "Py b", "$0",  ": .", ": ."]},
    {"dock":[ "$4","$2", "Py b",  ".",  ": .", ": ."]},
    {"dock":[ "$5","$3", "Py b",  ".",  ": .", ": ."]},
    {"dock":[ "b", "$4", "Py b", "$1",  ": .", ": ."]}

    //{"dock":[ "$3", "p3xz b", "p3y Py b", "$0",  ": .", ": ."]},
    //{"dock":[ "$4","$2",      "p3y Py b",  ".",  ": .", ": ."]},
    //{"dock":[ "$5","$3",      "p3y Py b",  ".",  ": .", ": ."]},
    //{"dock":[ "p3xz b", "$4", "p3y Py b", "$1",  ": .", ": ."]}
  ];
}



// 3x2 cell occupancy to try and get ride of staircases on ends.
//
function arch0(opt, _debug) {
  let geom =
    op.mov([1,0,0],
      op.sub(
        op.mov([0,0.5,0], op.cub({"size":[3,2,1]})),
        op.mov([0,0,-0.5], op.lif( {height:1}, op.cir({"radius":0.5})) ),
        op.mov([0,-0.5,0], op.cub({"size":[1,1,1]}))
      )
    );

  if (_debug) {
    geom = op.add(geom, DEBUG_GEOM);
  }

  return [
    {"ds":[0,0,0], "geom":geom, "dock":[ "$1","b .", "$3", "b #",  ": .", ": ."], "anchor":geom},
    {"ds":[0,0,0], "geom":geom, "dock":[ "$2","$0",  "$4", ". #",  ": .", ": ."] },
    {"ds":[0,0,0], "geom":geom, "dock":["b .","$1",  "$5", "b #",  ": .", ": ."] },

    //{"ds":[0,0,0], "geom":geom, "dock":[ "$4","b .", "b .","$0", ": .", ": ."] },
    //{"ds":[0,0,0], "geom":geom, "dock":[ "$5","$3",  "b .","$1", ": .", ": ."] },
    //{"ds":[0,0,0], "geom":geom, "dock":["b .","$4",  "b .","$2", ": .", ": ."] }

    {"ds":[0,0,0], "geom":geom, "dock":[ "$4","Pxz b .",  "b Py .","$0", "Pxz : .", "Pxz : ."] },
    {"ds":[0,0,0], "geom":geom, "dock":[ "$5","$3",       "b Py .","$1", "Pxz : .", "Pxz : ."] },
    {"ds":[0,0,0], "geom":geom, "dock":[ "Pxz b .","$4",  "b Py .","$2", "Pxz : .", "Pxz : ."] }
  ]

}

function arch0_c(opt, _debug) {
  let geom =
    op.mov([1,0,0],
      op.sub(
        op.mov([0,0.5,0], op.cub({"size":[3,2,1]})),
        op.mov([0,0,-0.5], op.lif( {height:1}, op.cir({"radius":0.5})) ),
        op.mov([0,-0.5,0], op.cub({"size":[1,1,1]}))
      )
    );

  if (_debug) {
    geom = op.add(geom, DEBUG_GEOM);
  }

  return [
    {"ds":[0,0,0], "geom":geom, "dock":[ "$1","A b .", "$3", "b #",  "A : .", "A : ."], "anchor":geom},
    {"ds":[0,0,0], "geom":geom, "dock":[ "$2","$0",  "$4", ". #",  "A : .", "A : ."] },
    {"ds":[0,0,0], "geom":geom, "dock":["A b .","$1",  "$5", "b #",  "A : .", "A : ."] },

    //{"ds":[0,0,0], "geom":geom, "dock":[ "$4","b .", "b .","$0", ": .", ": ."] },
    //{"ds":[0,0,0], "geom":geom, "dock":[ "$5","$3",  "b .","$1", ": .", ": ."] },
    //{"ds":[0,0,0], "geom":geom, "dock":["b .","$4",  "b .","$2", ": .", ": ."] }

    {"ds":[0,0,0], "geom":geom, "dock":[ "$4","A Pxz b .",  "b Py .","$0", "A Pxz : .", "A Pxz : ."] },
    {"ds":[0,0,0], "geom":geom, "dock":[ "$5","$3",       "b Py .","$1", "A Pxz : .", "A Pxz : ."] },
    {"ds":[0,0,0], "geom":geom, "dock":[ "A Pxz b .","$4",  "b Py .","$2", "A Pxz : .", "A Pxz : ."] }
  ]

}

//var x = arch0(null, true)
//console.log( op.obj_dumps({}, x[0].anchor).join("") );
//process.exit();

// one cell occupancy.
// Half height up arch
//
function _arch0(opt, _debug) {
  let geom =
    op.sub(
      op.mov([0,0,0], op.cub({"size":[1,1,1]})),
      op.mov([0,0,-0.5], op.lif( {height:1}, op.cir({"radius":0.5})) ),
      op.mov([0,-0.5,0], op.cub({"size":[1,1,1]}))
    );

  if (_debug) {
    geom = op.add(geom, DEBUG_GEOM);
  }

  return [
    {"ds":[0,0,0], "geom":geom, "id":"a1", "dock":[ "b", "b",  "b", ".",  ": .", ": ."], "anchor":geom}
  ];
}

// 4x3 occupancy to try to get rid of staircase on ends
//
function arch1(opt, _debug) {

  // left geom centered at (0,0,0)
  //
  let geom =
    op.mov([1.5,0,0],
      op.sub(
        op.mov([0,  0.5,    0], op.cub({"size":[4,2,1]})),
        op.mov([0, -0.5, -0.5], op.lif( {height:1}, op.cir({"radius":1.0}))),
        op.mov([0, -0.5,    0], op.cub({"size":[2,2,1.2], "center":[0,-1,0]}))
      )
    );

  if (_debug) {
    geom = op.add(geom, DEBUG_GEOM);
  }

  return [
    {"ds":[ 0,0,0], "geom":geom, "dock":[  "$1","b .",  "$4", "_",    ": .", ": ."], "anchor": geom},
    {"ds":[ 0,0,0], "geom":geom, "dock":[  "$2", "$0",  "$5", ". #",  ": .", ": ."] },
    {"ds":[ 0,0,0], "geom":geom, "dock":[  "$3", "$1",  "$6", ". #",  ": .", ": ."] },
    {"ds":[ 0,0,0], "geom":geom, "dock":[ "b .", "$2",  "$7", "_",    ": .", ": ."] },

    //{"ds":[ 0,0,0], "geom":geom, "dock":[  "$5","b .","b .", "$0",  ": .", ": ."] },
    //{"ds":[ 0,0,0], "geom":geom, "dock":[  "$6", "$4","b .", "$1",  ": .", ": ."] },
    //{"ds":[ 0,0,0], "geom":geom, "dock":[  "$7", "$5","b .", "$2",  ": .", ": ."] },
    //{"ds":[ 0,0,0], "geom":geom, "dock":[ "b .", "$6","b .", "$3",  ": .", ": ."] }

    {"ds":[ 0,0,0], "geom":geom, "dock":[ "$5","Pxz b .",   "b Py .", "$0",  "Pxz : .", "Pxz : ."] },
    {"ds":[ 0,0,0], "geom":geom, "dock":[ "$6", "$4",       "b Py .", "$1",  "Pxz : .", "Pxz : ."] },
    {"ds":[ 0,0,0], "geom":geom, "dock":[ "$7", "$5",       "b Py .", "$2",  "Pxz : .", "Pxz : ."] },
    {"ds":[ 0,0,0], "geom":geom, "dock":[ "Pxz b .", "$6",  "b Py .", "$3",  "Pxz : .", "Pxz : ."] }
  ];
}

// 4x3 occupancy to try to get rid of staircase on ends
//
function arch1_c(opt, _debug) {

  //    y
  //    |
  //    . --x
  //   /
  //  z
  //     |  b   |  b   |  b  |   b  |
  //  b  |  $4  |  $5  | $6  |  $7  |  b
  //  b  |  $0  |  $1  | $2  |  $3  |  b
  //     |   b  |   .  |  .  |   b  |


  // left geom centered at (0,0,0)
  //
  let geom =
    op.mov([1.5,0,0],
      op.sub(
        op.mov([0,  0.5,    0], op.cub({"size":[4,2,1]})),
        op.mov([0, -0.5, -0.5], op.lif( {height:1}, op.cir({"radius":1.0}))),
        op.mov([0, -0.5,    0], op.cub({"size":[2,2,1.2], "center":[0,-1,0]}))
      )
    );

  if (_debug) {
    geom = op.add(geom, DEBUG_GEOM);
  }

  return [
    {"ds":[ 0,0,0], "geom":geom, "dock":[ "$1", "b .",  "$4", "_",    ": .", ": ."], "anchor": geom},
    {"ds":[ 0,0,0], "geom":geom, "dock":[ "$2",  "$0",  "$5", ". #",  ": .", ": ."] },
    {"ds":[ 0,0,0], "geom":geom, "dock":[ "$3",  "$1",  "$6", ". #",  ": .", ": ."] },
    {"ds":[ 0,0,0], "geom":geom, "dock":[ "b .", "$2",  "$7", "_",    ": .", ": ."] },

    //{"ds":[ 0,0,0], "geom":geom, "dock":[  "$5","b .","b .", "$0",  ": .", ": ."] },
    //{"ds":[ 0,0,0], "geom":geom, "dock":[  "$6", "$4","b .", "$1",  ": .", ": ."] },
    //{"ds":[ 0,0,0], "geom":geom, "dock":[  "$7", "$5","b .", "$2",  ": .", ": ."] },
    //{"ds":[ 0,0,0], "geom":geom, "dock":[ "b .", "$6","b .", "$3",  ": .", ": ."] }

    {"ds":[ 0,0,0], "geom":geom, "dock":[ "$5","Pxz b .",   "b Py .", "$0",  "Pxz : .", "Pxz : ."] },
    {"ds":[ 0,0,0], "geom":geom, "dock":[ "$6", "$4",       "b Py .", "$1",  "Pxz : .", "Pxz : ."] },
    {"ds":[ 0,0,0], "geom":geom, "dock":[ "$7", "$5",       "b Py .", "$2",  "Pxz : .", "Pxz : ."] },
    {"ds":[ 0,0,0], "geom":geom, "dock":[ "Pxz b .", "$6",  "b Py .", "$3",  "Pxz : .", "Pxz : ."] }
  ];
}

//var x = arch1(null, true);
//console.log( op.obj_dumps({}, x[0].anchor).join("") );
//process.exit();

// 2x1 top portion of arch
//
function _arch1(opt, _debug) {

  // left geom centered at (0,0,0)
  //
  let geom =
    op.mov([0.5,-0.5,0],
      op.sub(
        op.cub({"size":[2,2,1], "center":[0,0,0]}),
        op.mov([0,0,-0.5], op.lif( {height:1}, op.cir({"radius":1.0}))),
        op.cub({"size":[2,2,1.2], "center":[0,-1,0]})
      )
    );

  let lgeom = op.and(geom, op.cub({"size":[1,1,1], "center":[0,0,0]}));
  let rgeom = op.and(op.mov([-1,0,0],geom), op.cub({"size":[1,1,1], "center":[0,0,0]}));

  if (_debug) {
    lgeom = op.add(lgeom, DEBUG_GEOM);
    rgeom = op.add(rgeom, DEBUG_GEOM);
  }

  return [
    {"ds":[ 0,0,0], "geom":lgeom, "id":"a1_0", "dock":[  "$1",  "b", "b", ".",  ": .", ": ."], "anchor": geom},
    {"ds":[ 1,0,0], "geom":rgeom, "id":"a1_1", "dock":[   "b", "$0", "b", ".",  ": .", ": ."]}
  ];
}

// 5x3 top portion of arch
// middle empty region not returned, so only 5 blocks
//
function arch2(opt, _debug) {

  // want center is at 0, which means a cube is +-0.5 in each direction.
  // op.cub will create a curboid whose center of mass ias at (0,0,0).
  //

  let sx = 5,
      sy = 4,
      sz = 1;
  let cx = (sx/2 - 0.5),
      cy = (sy/2 - 0.5),
      cz = (sz/2 - 0.5);

  let geom =
    op.sub(
      op.cub({"size":[sx,sy,sz], "center":[cx,cy,cz]}),
      op.mov([cx,0.5,-0.5], op.lif( {height:1}, op.cir({"radius":1.5}))),
      op.cub({"size":[3,1,1], "center":[cx,0.0,cz]})
    );

  //    y
  //    |
  //    . --x
  //   /
  //  z
  //     |  b   |  b   |   b   |   b  |   b  |
  //  b  | $11  | $12  | $13   | $14  | $15  |  b
  //  b  |  $6  |  $7  |  $8   |  $9  | $10  |  b
  //  b  |  $2  |  $3  |   .   |  $4  |  $5  |  b
  //  b  |  $0  |   .  |   .   |   .  |  $1  |  b
  //     |   b  |   .  |   .   |   .  |   b  |

  let info = [];

  for (let ii=0; ii<16; ii++) { info.push({"dock":['.', '.', '.', '.', '.', '.']}); }
  info[0]["anchor"] = geom;


  if (_debug) {
    info[0].anchor =  op.add( info[0].anchor, DEBUG_GEOM );
    for (let ii=0; ii<info.length; ii++) {
      info[ii].geom = op.add( info[ii].geom, DEBUG_GEOM );
    }
  }

  info[0]["dock"] = [ ".",  ". b",  "$2",  "_",   ": .",    ": ." ];
  info[1]["dock"] = [ ". b",  ".",  "$5",  "_",   ": .",    ": ." ];

  info[2]["dock"] = [ "$3", ". b",  "$6", "$0",   ": .",    ": ." ];
  info[3]["dock"] = [  ".",  "$2",  "$7",  ".",   ": .",    ": ." ];
  info[4]["dock"] = [ "$5",   ".",  "$9",  ".",   ": .",    ": ." ];
  info[5]["dock"] = [ ". b", "$4", "$10", "$1",   ": .",    ": ." ];

  info[6]["dock"]   = [ "$7", ". b", "$11", "$2",   ": .",    ": ." ];
  info[7]["dock"]   = [ "$8",  "$6", "$12", "$3",   ": .",    ": ." ];
  info[8]["dock"]   = [ "$9",  "$7", "$13",  ".",   ": .",    ": ." ];
  info[9]["dock"]   = ["$10",  "$8", "$14", "$4",   ": .",    ": ." ];
  info[10]["dock"]  = [". b",  "$9", "$15", "$5",   ": .",    ": ." ];

  info[11]["dock"]  = ["$12", ". b",  "b . Py", "$6",   "Pxz : .",    "Pxz : ." ];
  info[12]["dock"]  = ["$13", "$11",  "b . Py", "$7",   "Pxz : .",    "Pxz : ." ];
  info[13]["dock"]  = ["$14", "$12",  "b . Py", "$8",   "Pxz : .",    "Pxz : ." ];
  info[14]["dock"]  = ["$15", "$13",  "b . Py", "$9",   "Pxz : .",    "Pxz : ." ];
  info[15]["dock"]  = [". b", "$14",  "b . Py","$10",   "Pxz : .",    "Pxz : ." ];

  return info;
}

// 3x2 top portion of arch
// middle empty region not returned, so only 5 blocks
//
function _arch2(opt, _debug) {

  let geom =
    op.mov([1,-0.0,0],
        op.sub(
          op.cub({"size":[3,3,1], "center":[0,0,0]}),
          op.mov([0,0,-0.5], op.lif( {height:1}, op.cir({"radius":1.5}))),
          op.cub({"size":[3,3,1], "center":[0,-1.5,0]})
        )
      );

  //    y
  //    |
  //    . --x
  //   /
  //  z
  //     |  b   |  b   |   b  |
  //  b  | a2_1 | a2_2 | a2_4 | b
  //  b  | a2_0 |  .   | a2_3 | b
  //     |  .   |  .   |   .  |

  let info = [];

  info.push({"ds":[ 0,0,0], "geom": op.and(op.cub({"size":[1,1,1],"center":[0,0,0]}), op.mov([0,0,0], geom)), "anchor": geom  });
  info.push({"ds":[ 0,0,0], "geom": op.and(op.cub({"size":[1,1,1],"center":[0,0,0]}), op.mov([0,-1,0], geom)) });
  //info.push({"ds":[ 0,0,0], "geom": op.and(op.cub({"size":[1,1,1],"center":[0,0,0]}), op.mov([-1,0,0], geom)) });
  info.push({"ds":[ 0,0,0], "geom": op.and(op.cub({"size":[1,1,1],"center":[0,0,0]}), op.mov([-1,-1,0], geom)) });
  info.push({"ds":[ 0,0,0], "geom": op.and(op.cub({"size":[1,1,1],"center":[0,0,0]}), op.mov([-2,0,0], geom)) });
  info.push({"ds":[ 0,0,0], "geom": op.and(op.cub({"size":[1,1,1],"center":[0,0,0]}), op.mov([-2,-1,0], geom)) });

  if (_debug) {
    for (let ii=0; ii<info.length; ii++) {
      info[ii].geom = op.add( info[ii].geom, DEBUG_GEOM );
    }
  }

  info[0]["id"] = "a3_0";
  info[1]["id"] = "a3_1";
  info[2]["id"] = "a3_2";
  info[3]["id"] = "a3_3";
  info[4]["id"] = "a3_4";

  info[0]["dock"] = [    ".",    "b", "$1",  ".",    ": .",    ": ." ];
  info[1]["dock"] = [   "$2",    "b",  "b", "$0",    ": .",    ": ." ];
  info[2]["dock"] = [   "$4",   "$1",  "b",  ".",    ": .",    ": ." ];
  info[3]["dock"] = [    "b",    ".", "$4",  ".",    ": .",    ": ." ];
  info[4]["dock"] = [    "b",   "$2",  "b", "$3",    ": .",    ": ." ];

  return info;
}

function _print_stickem_conf(info) {

  console.log("{");

  if ("comment" in info) {
    console.log("  \"comment\": ", JSON.stringify(info.comment) + ",");
  }

  console.log("  \"unit\": ", JSON.stringify(info.unit) + ",");
  console.log("  \"unit_center\": ", JSON.stringify(info.unit_center) + ",");
  console.log("  \"up\": ", JSON.stringify(info.up) + ",");
  console.log("  \"symmetry\": ", JSON.stringify(info.symmetry) + ",");
  console.log("  \"stl_dir\": ", JSON.stringify(info.stl_dir) + ",");
  console.log("  \"obj_dir\": ", JSON.stringify(info.obj_dir) + ",");

  console.log("  \"dock\": {");
  let dock_key_a = [];
  for (let dock_key in info.dock) {
    dock_key_a.push(dock_key);
  }

  for (let dock_key_idx=0; dock_key_idx<dock_key_a.length; dock_key_idx++) {
    let dock_key = dock_key_a[dock_key_idx];
    let sfx = ",";
    if (dock_key_idx == (dock_key_a.length-1)) { sfx = ""; }
    console.log("    \"" + dock_key.toString() + "\" :", JSON.stringify(info.dock[dock_key]) + sfx);
  }
  console.log("  },");

  console.log("  \"tile\": {");
  let tile_key_a = [];
  for (let tile_key in info.tile) {
    tile_key_a.push(tile_key);
  }

  for (let tile_key_idx=0; tile_key_idx<tile_key_a.length; tile_key_idx++) {
    let tile_key = tile_key_a[tile_key_idx];
    let sfx = ",";
    if (tile_key_idx == (tile_key_a.length-1)) { sfx = ""; }
    console.log("    \"" + tile_key.toString() + "\" :", JSON.stringify(info.tile[tile_key]) + sfx);
  }
  console.log("  },");

  console.log("  \"source\": [");
  for (let idx=0; idx<info.source.length; idx++) {
    let sfx = ",";
    if (idx == (info.source.length-1)) { sfx = ""; }

    if (info.source[idx].dock.length==1) {
      console.log("    {\"name\":" + JSON.stringify(info.source[idx].name) + ", \"dock\":" + JSON.stringify(info.source[idx].dock) + "}" + sfx)
    }
    else {
      console.log("    {\"name\":" + JSON.stringify(info.source[idx].name) + ", \"dock\":[");
      for (let dock_idx=0; dock_idx < info.source[idx].dock.length; dock_idx++) {
        dock_sfx = ",";
        if (dock_idx == (info.source[idx].dock.length-1)) { dock_sfx = ""; }
        console.log("      " + JSON.stringify(info.source[idx].dock[dock_idx]) + dock_sfx )
      }
      console.log("    ]}" + sfx);
    }
    //console.log("    " + JSON.stringify(info.source[idx]) + sfx);

    //console.log("  {");
    //console.log("  { \"name\":", JSON.stringify(info.source[idx].name) + ",")
    //console.log("    \"dock\":", JSON.stringify(info.source[idx].dock) + "}" + sfx)

    //console.log("  }" + sfx);
  }
  console.log("  ],");

  if ("constraint" in info) {
    console.log("  \"constraint\": [");
    for (let ii=0; ii < info.constraint.length; ii++) {
      sfx = ((ii == (info.constraint.length-1)) ? "" : ",");
      console.log("    " + JSON.stringify(info.constraint[ii]) + sfx);
    }
    console.log("  ],");
  }

  console.log("  \"weight\": {");
  if ("weight" in info) {
    let weight_key_a = [];
    for (let weight_key in info.weight) { weight_key_a.push(weight_key); }
    for (let weight_key_idx=0; weight_key_idx<weight_key_a.length; weight_key_idx++) {
      let weight_key = weight_key_a[weight_key_idx];
      let sfx = ",";
      if (weight_key_idx == (weight_key_a.length-1)) { sfx = ""; }
      console.log("    \"" + weight_key + "\" : " + info.weight[weight_key].toString() + sfx);
    }
  }
  console.log("  }");

  console.log("}");
}

function main_nopath() {

  let stickem_info = {
    "comment": "stl out dir: '" + OUT_DIR_STL + "/', OBJ out dir: '" + OUT_DIR_OBJ + "/'",
    "unit": [1,1,1],
    "unit_center":[0,0,0],
    "up":[0,1,0],
    "symmetry":"y",
    "stl_dir": OUT_DIR_STL,
    "obj_dir": OUT_DIR_OBJ,
    "dock": {
      "." : { "type":"!", "dock":[0], "description":"empty space (.)" },
      "#" : { "type":"!", "dock":[1], "description":"ground (#)" },

      "A" : { "type":"@", "description": "arch dock (xz)" },
      "B" : { "type":"@", "description": "arch dock (xz)" },

      //"p0xz" : { "type":"&", "dock":"q0xz", "description":"arch0_s(simple) pair to block_b (xz)" },
      //"q0xz" : { "type":"&", "dock":"p0xz", "description":"arch0_s(simple) pair to block_b (xz)" },
      //"p0y" : { "type":"&", "dock":"q0y", "description":"arch0_s(simple) pair to block_b (y)" },
      //"q0y" : { "type":"&", "dock":"p0y", "description":"arch0_s(simple) pair to block_b (y)" },

      //"p1xz" : { "type":"&", "dock":"q1xz", "description":"arch1_s(simple) pair to block_b (xz)" },
      //"q1xz" : { "type":"&", "dock":"p1xz", "description":"arch1_s(simple) pair to block_b (xz)" },
      //"p1y" : { "type":"&", "dock":"q1y", "description":"arch1_s(simple) pair to block_b (y)" },
      //"q1y" : { "type":"&", "dock":"p1y", "description":"arch1_s(simple) pair to block_b (y)" },

      //"p2xz" : { "type":"&", "dock":"q2xz", "description":"arch2_s(simple) pair to block_b (xz)" },
      //"q2xz" : { "type":"&", "dock":"p2xz", "description":"arch2_s(simple) pair to block_b (xz)" },
      //"p2y" : { "type":"&", "dock":"q2y", "description":"arch2_s(simple) pair to block_b (y)" },
      //"q2y" : { "type":"&", "dock":"p2y", "description":"arch2_s(simple) pair to block_b (y)" },

      //"p3xz" : { "type":"&", "dock":"q3xz", "description":"arch3_s(simple) pair to block_b (xz)" },
      //"q3xz" : { "type":"&", "dock":"p3xz", "description":"arch3_s(simple) pair to block_b (xz)" },
      //"p3y" : { "type":"&", "dock":"q3y", "description":"arch3_s(simple) pair to block_b (y)" },
      //"q3y" : { "type":"&", "dock":"p3y", "description":"arch3_s(simple) pair to block_b (y)" },

      "Pxz" : { "type":"@", "description": "platform dock (xz)" },
      "Py+" : { "type":"@", "description": "platform dock (y+)" },
      "Py-" : { "type":"@", "description": "platform dock (y-)" },

      "Py":  { "type":"@", "description": "platform dock (y+-)" },

      "_" : { "type":"%", "dock":"b #", "description":"general wildcard like dock" },
      ":" : { "type":"@", "description":"general wildcard like dock" },

      "b" : { "type":"!", "dock":["block"], "description":"block" }

    },
    "tile": {
      "0": {"name":"0", "description":"empty" },
      "1": {"name":"1", "description":"ground" }
    },
    "source": [
    ],

    "constraint": [
      {"type": "quiltRemove",  "range": { "x": [], "y":[1], "z":[], "tile":"#"} },
      {"type": "quiltForce",   "range" : { "x":[],"y":[0,1],"z":[], "tile":"#"}},
      {"type": "quiltPin",     "range" : { "x":[],"y":[0,1],"z":[], "tile":"#"}}
    ],


    "weight": {
      ".": 400,
      "#": 2,
      "block": 1,
      "block_b": 1,
      "column2": 1,
      "column3": 1,
      "wedge_up": 1,
      "wedge_down": 1,
      "doorway": 1,
      "double-doorway": 1,
      "block-2x2": 1,

      "platform_bend": 1,
      "platform_straight": 1,
      "platform_tee": 1,
      "platform_cross": 1,

      "arch0": 1,
      "arch1": 31,
      "arch2": 97,
      "arch3": 79,

      "stair": 43
    }
  };

  let lib_info = [
    { "name": "block",        "f": function() { return block(); } },
    { "name": "block_b",        "f": function() { return block_b(); } },

    { "name": "platform_bend",      "f": function() { return platform_bend(); } },
    { "name": "platform_straight",  "f": function() { return platform_straight(); } },
    //{ "name": "platform_tee",       "f": function() { return platform_tee(); } },
    //{ "name": "platform_cross",     "f": function() { return platform_cross(); } },
    { "name": "platform_tee",       "f": function() { return platform_tee_overhang(); } },
    { "name": "platform_cross",     "f": function() { return platform_cross_overhang(); } },

    { "name": "wedge_up",     "f": function() { return wedge_up(); } },
    { "name": "wedge_down",   "f": function() { return wedge_down(); } },
    { "name": "doorway",      "f": function() { return doorway(); } },
    { "name": "double-doorway",      "f": function() { return double_doorway(); } },
    { "name": "block-2x2",      "f": function() { return block_2x2(); } },

    { "name": "column2",        "f": function() { return column2(); } },
    { "name": "column3",        "f": function() { return column3(); } },

    { "name": "arch0",        "f": function() { return arch0_s(); } },
    { "name": "arch1",        "f": function() { return arch1_s(); } },
    { "name": "arch2",        "f": function() { return arch2_s(); } },
    { "name": "arch3",        "f": function() { return arch3_s(); } },

    { "name": "stair",        "f": (function(_n){ return function() { return stair(_n); } })(5)  }
  ];

  for (let li_idx=0; li_idx<lib_info.length; li_idx++) {
    let li = lib_info[li_idx];

    let shape_info = li.f();

    let stl_data = op.stl_dumps({"binary":false}, shape_info[0].anchor).join("");
    fs.writeFileSync(OUT_DIR_STL + "/" + li.name + ".stl", stl_data);

    let obj_data = op.obj_dumps({}, shape_info[0].anchor).join("");
    fs.writeFileSync(OUT_DIR_OBJ + "/" + li.name + ".obj", obj_data);

    let src_idx = stickem_info.source.length;
    stickem_info.source.push({"name":li.name, "dock":[ shape_info[0].dock ]});
    for (let ii=1; ii<shape_info.length; ii++) {
      stickem_info.source[src_idx].dock.push( shape_info[ii].dock );
    }

  }

  _print_stickem_conf(stickem_info);

  return;
}

function main_1path() {

  let stickem_info = {
    "comment": "stl out dir: '" + OUT_DIR_STL + "/', OBJ out dir: '" + OUT_DIR_OBJ + "/'",
    "unit": [1,1,1],
    "unit_center":[0,0,0],
    "up":[0,1,0],
    "symmetry":"y",
    "stl_dir": OUT_DIR_STL,
    "obj_dir": OUT_DIR_OBJ,
    "dock": {
      "." : { "type":"!", "dock":[0], "description":"empty space (.)" },
      "#" : { "type":"!", "dock":[1], "description":"ground (#)" },

      "Pxz" : { "type":"@", "description": "platform dock (xz)" },
      "Py+" : { "type":"@", "description": "platform dock (y+)" },
      "Py-" : { "type":"@", "description": "platform dock (y-)" },

      "Py":  { "type":"@", "description": "platform dock (y+-)" },

      "_" : { "type":"%", "dock":"b #", "description":"general wildcard like dock" },
      ":" : { "type":"@", "description":"general wildcard like dock" },

      "b" : { "type":"!", "dock":["block"], "description":"block" }

    },
    "tile": {
      "0": {"name":"0", "description":"empty" },
      "1": {"name":"1", "description":"ground" }
    },
    "source": [
    ],

    "constraint": [
      {"type": "quiltRemove",  "range": { "x": [], "y":[1], "z":[], "tile":"#"} },
      {"type": "quiltForce",   "range" : { "x":[],"y":[0,1],"z":[], "tile":"#"}},
      {"type": "quiltPin",     "range" : { "x":[],"y":[0,1],"z":[], "tile":"#"}},

      {"type": "quiltRemove", "range" : { "x":[],"y":[],"z":[], "tile":"patha_cap_.*$"}},
      {"type": "quiltRemove", "range" : { "x":[],"y":[],"z":[], "tile":"pathb_cap_.*$"}},

      {"type": "quiltAdd",   "range" : { "x":[2,3],"y":[1,2],"z":[1,2], "tile":"patha_cap_.*$"}},
      {"type": "quiltForce", "range" : { "x":[2,3],"y":[1,2],"z":[1,2], "tile":"patha_cap_.*$"}},

      {"type": "quiltAdd",   "range" : { "x":[1,2],"y":[1,2],"z":[2,3], "tile":"pathb_cap_.*$"}},
      {"type": "quiltForce", "range" : { "x":[1,2],"y":[1,2],"z":[2,3], "tile":"pathb_cap_.*$"}},

      {"type": "quiltAdd",   "range" : { "x":[-2,-1],"y":[-1],"z":[-3,-2], "tile":"patha_cap_.*$"}},
      {"type": "quiltForce", "range" : { "x":[-2,-1],"y":[-1],"z":[-3,-2], "tile":"patha_cap_.*$"}},

      {"type": "quiltAdd",   "range" : { "x":[-3,-2],"y":[-1],"z":[-2,-1], "tile":"pathb_cap_.*$"}},
      {"type": "quiltForce", "range" : { "x":[-3,-2],"y":[-1],"z":[-2,-1], "tile":"pathb_cap_.*$"}}

    ],


    "weight": {
      ".": 1001,
      "#": 2,
      "block": 4,
      "column2": 5,
      "column3": 5,
      "wedge_up": 1,
      "wedge_down": 1,
      "doorway": 1,
      "double-doorway": 1,
      "block-2x2": 1,

      "patha_stair_0": 5,

      "platform_bend": 1,
      "platform_straight": 1,
      "platform_tee": 1,
      "platform_cross": 1,
      "arch0": 1,
      "arch1": 1,
      "arch2": 1,
      "stair": 40
    }
  };

  let _dbg = false;

  let lib_info = [
    { "name": "block",        "f": function() { return block(); } },
    { "name": "block_b",        "f": function() { return block_b(); } },

    { "name": "platform_bend",      "f": function() { return platform_bend(); } },
    { "name": "platform_straight",  "f": function() { return platform_straight(); } },

    //{ "name": "platform_tee",       "f": function() { return platform_tee(); } },
    //{ "name": "platform_cross",     "f": function() { return platform_cross(); } },
    { "name": "platform_tee",       "f": function() { return platform_tee_overhang(); } },
    { "name": "platform_cross",     "f": function() { return platform_cross_overhang(); } },

    //---

    { "name": "patha_straight_0",        "f": function() { return path_straight(0,0,_dbg); } },
    { "name": "patha_straight_1",        "f": function() { return path_straight(0,1,_dbg); } },
    { "name": "patha_straight_2",        "f": function() { return path_straight(0,2,_dbg); } },

    { "name": "patha_bend_0",        "f": function() { return path_bend(0,0,_dbg); } },
    { "name": "patha_bend_1",        "f": function() { return path_bend(0,1,_dbg); } },
    { "name": "patha_bend_2",        "f": function() { return path_bend(0,2,_dbg); } },
    { "name": "patha_bend_3",        "f": function() { return path_bend(0,3,_dbg); } },

    /*
    { "name": "patha_bendup_0",        "f": function() { return path_bend_up(0,0,_dbg); } },
    { "name": "patha_bendup_1",        "f": function() { return path_bend_up(0,1,_dbg); } },
    { "name": "patha_bendup_2",        "f": function() { return path_bend_up(0,2,_dbg); } },
    { "name": "patha_bendup_3",        "f": function() { return path_bend_up(0,3,_dbg); } },
    { "name": "patha_bendup_4",        "f": function() { return path_bend_up(0,4,_dbg); } },
    { "name": "patha_bendup_5",        "f": function() { return path_bend_up(0,5,_dbg); } },
    { "name": "patha_bendup_6",        "f": function() { return path_bend_up(0,6,_dbg); } },
    */

    { "name": "patha_stair_0",        "f": function() { return path_stair(0,_dbg); } },

    { "name": "patha_cap_0",        "f": function() { return path_cap(0,0,_dbg); } },
    { "name": "patha_cap_1",        "f": function() { return path_cap(0,1,_dbg); } },
    { "name": "patha_cap_2",        "f": function() { return path_cap(0,2,_dbg); } },
    { "name": "patha_cap_3",        "f": function() { return path_cap(0,3,_dbg); } },
    { "name": "patha_cap_4",        "f": function() { return path_cap(0,4,_dbg); } },
    { "name": "patha_cap_5",        "f": function() { return path_cap(0,5,_dbg); } },
    { "name": "patha_cap_6",        "f": function() { return path_cap(0,6,_dbg); } },
    { "name": "patha_cap_7",        "f": function() { return path_cap(0,7,_dbg); } },

    //---

    //{ "name": "column2",        "f": function() { return column2(); } },
    //{ "name": "column3",        "f": function() { return column3(); } },

    { "name": "wedge_up",     "f": function() { return wedge_up(); } },
    { "name": "wedge_down",   "f": function() { return wedge_down(); } },
    { "name": "doorway",      "f": function() { return doorway(); } },
    { "name": "double-doorway",      "f": function() { return double_doorway(); } },
    { "name": "block-2x2",      "f": function() { return block_2x2(); } },

    //{ "name": "arch0",        "f": function() { return arch0(); } },
    //{ "name": "arch1",        "f": function() { return arch1(); } },
    //{ "name": "arch2",        "f": function() { return arch2(); } },

    { "name": "arch0",        "f": function() { return arch0_s(); } },
    { "name": "arch1",        "f": function() { return arch1_s(); } },
    { "name": "arch2",        "f": function() { return arch2_s(); } },

    { "name": "stair",        "f": (function(_n){ return function() { return stair(_n); } })(5)  }

  ];

  for (let li_idx=0; li_idx<lib_info.length; li_idx++) {
    let li = lib_info[li_idx];

    let shape_info = li.f();

    let stl_data = op.stl_dumps({"binary":false}, shape_info[0].anchor).join("");
    fs.writeFileSync(OUT_DIR_STL + "/" + li.name + ".stl", stl_data);

    let obj_data = op.obj_dumps({}, shape_info[0].anchor).join("");
    fs.writeFileSync(OUT_DIR_OBJ + "/" + li.name + ".obj", obj_data);

    let src_idx = stickem_info.source.length;
    stickem_info.source.push({"name":li.name, "dock":[ shape_info[0].dock ]});
    for (let ii=1; ii<shape_info.length; ii++) {
      stickem_info.source[src_idx].dock.push( shape_info[ii].dock );
    }

  }

  _print_stickem_conf(stickem_info);

  return;
}


function main_2path() {

  let stickem_info = {
    "comment": "stl out dir: '" + OUT_DIR_STL + "/', OBJ out dir: '" + OUT_DIR_OBJ + "/'",
    "unit": [1,1,1],
    "unit_center":[0,0,0],
    "up":[0,1,0],
    "symmetry":"y",
    "stl_dir": OUT_DIR_STL,
    "obj_dir": OUT_DIR_OBJ,
    "dock": {
      "." : { "type":"!", "dock":[0], "description":"empty space (.)" },
      "#" : { "type":"!", "dock":[1], "description":"ground (#)" },

      "Pxz" : { "type":"@", "description": "platform dock (xz)" },
      "Py+" : { "type":"@", "description": "platform dock (y+)" },
      "Py-" : { "type":"@", "description": "platform dock (y-)" },

      "Py":  { "type":"@", "description": "platform dock (y+-)" },

      "_" : { "type":"%", "dock":"b #", "description":"general wildcard like dock" },
      ":" : { "type":"@", "description":"general wildcard like dock" },

      "b" : { "type":"!", "dock":["block"], "description":"block" }

    },
    "tile": {
      "0": {"name":"0", "description":"empty" },
      "1": {"name":"1", "description":"ground" }
    },

    "constraint": [
      {"type": "quiltRemove",  "range": { "x": [], "y":[1], "z":[], "tile":"#"} },
      {"type": "quiltForce",   "range" : { "x":[],"y":[0,1],"z":[], "tile":"#"}},
      {"type": "quiltPin",     "range" : { "x":[],"y":[0,1],"z":[], "tile":"#"}},

      {"type": "quiltRemove", "range" : { "x":[],"y":[],"z":[], "tile":"patha_cap_.*$"}},
      {"type": "quiltRemove", "range" : { "x":[],"y":[],"z":[], "tile":"pathb_cap_.*$"}},

      {"type": "quiltAdd",   "range" : { "x":[2,3],"y":[1,2],"z":[1,2], "tile":"patha_cap_.*$"}},
      {"type": "quiltForce", "range" : { "x":[2,3],"y":[1,2],"z":[1,2], "tile":"patha_cap_.*$"}},

      {"type": "quiltAdd",   "range" : { "x":[1,2],"y":[1,2],"z":[2,3], "tile":"pathb_cap_.*$"}},
      {"type": "quiltForce", "range" : { "x":[1,2],"y":[1,2],"z":[2,3], "tile":"pathb_cap_.*$"}},

      //---
      // trying to figure out why 32^3 doesn't work well for these constraints...
      //

      {"type": "quiltAdd",   "range" : { "x":[-2,-1],"y":[-2,-1],"z":[-3,-2], "tile":"patha_cap_.*$"}},
      {"type": "quiltForce", "range" : { "x":[-2,-1],"y":[-2,-1],"z":[-3,-2], "tile":"patha_cap_.*$"}},

      {"type": "quiltAdd",   "range" : { "x":[-3,-2],"y":[-2,-1],"z":[-2,-1], "tile":"pathb_cap_.*$"}},
      {"type": "quiltForce", "range" : { "x":[-3,-2],"y":[-2,-1],"z":[-2,-1], "tile":"pathb_cap_.*$"}}

      //{"type": "quiltAdd",   "range" : { "x":[-2,-1],"y":[1,2],"z":[-3,-2], "tile":"patha_cap_.*$"}},
      //{"type": "quiltForce", "range" : { "x":[-2,-1],"y":[1,2],"z":[-3,-2], "tile":"patha_cap_.*$"}},

      //{"type": "quiltAdd",   "range" : { "x":[-3,-2],"y":[1,2],"z":[-2,-1], "tile":"pathb_cap_.*$"}},
      //{"type": "quiltForce", "range" : { "x":[-3,-2],"y":[1,2],"z":[-2,-1], "tile":"pathb_cap_.*$"}}

    ],

    "source": [
    ],
    "weight": {
      ".": 1003,
      "#": 2,
      "block": 1,

      "column2": 5,
      "column3": 5,

      "wedge_up": 1,
      "wedge_down": 1,
      "doorway": 1,
      "double-doorway": 1,
      "block-2x2": 1,

      "patha_stair_0": 5,
      "pathb_stair_0": 5,

      "platform_bend": 1,
      "platform_straight": 1,
      "platform_tee": 1,
      "platform_cross": 1,
      "arch0": 1,
      "arch1": 1,
      "arch2": 1,
      "stair": 40
    }
  };

  let _dbg = false;

  let lib_info = [
    { "name": "block",        "f": function() { return block(); } },

    { "name": "block_b",        "f": function() { return block(); } },

    { "name": "platform_bend",      "f": function() { return platform_bend(); } },
    { "name": "platform_straight",  "f": function() { return platform_straight(); } },
    { "name": "platform_tee",       "f": function() { return platform_tee(); } },
    { "name": "platform_cross",     "f": function() { return platform_cross(); } },

    //---

    { "name": "patha_straight_0",        "f": function() { return path_straight(0,0,_dbg); } },
    { "name": "patha_straight_1",        "f": function() { return path_straight(0,1,_dbg); } },
    { "name": "patha_straight_2",        "f": function() { return path_straight(0,2,_dbg); } },

    { "name": "patha_bend_0",        "f": function() { return path_bend(0,0,_dbg); } },
    { "name": "patha_bend_1",        "f": function() { return path_bend(0,1,_dbg); } },
    { "name": "patha_bend_2",        "f": function() { return path_bend(0,2,_dbg); } },
    { "name": "patha_bend_3",        "f": function() { return path_bend(0,3,_dbg); } },

    { "name": "patha_bendup_0",        "f": function() { return path_bend_up(0,0,_dbg); } },
    { "name": "patha_bendup_1",        "f": function() { return path_bend_up(0,1,_dbg); } },
    { "name": "patha_bendup_2",        "f": function() { return path_bend_up(0,2,_dbg); } },
    { "name": "patha_bendup_3",        "f": function() { return path_bend_up(0,3,_dbg); } },
    { "name": "patha_bendup_4",        "f": function() { return path_bend_up(0,4,_dbg); } },
    { "name": "patha_bendup_5",        "f": function() { return path_bend_up(0,5,_dbg); } },
    { "name": "patha_bendup_6",        "f": function() { return path_bend_up(0,6,_dbg); } },

    { "name": "patha_stair_0",        "f": function() { return path_stair(0,_dbg); } },

    { "name": "patha_cap_0",        "f": function() { return path_cap(0,0,_dbg); } },
    { "name": "patha_cap_1",        "f": function() { return path_cap(0,1,_dbg); } },
    { "name": "patha_cap_2",        "f": function() { return path_cap(0,2,_dbg); } },
    { "name": "patha_cap_3",        "f": function() { return path_cap(0,3,_dbg); } },
    { "name": "patha_cap_4",        "f": function() { return path_cap(0,4,_dbg); } },
    { "name": "patha_cap_5",        "f": function() { return path_cap(0,5,_dbg); } },
    { "name": "patha_cap_6",        "f": function() { return path_cap(0,6,_dbg); } },
    { "name": "patha_cap_7",        "f": function() { return path_cap(0,7,_dbg); } },

    //---

    { "name": "pathb_straight_0",        "f": function() { return path_straight(1,0,_dbg); } },
    { "name": "pathb_straight_1",        "f": function() { return path_straight(1,1,_dbg); } },
    { "name": "pathb_straight_2",        "f": function() { return path_straight(1,2,_dbg); } },

    { "name": "pathb_bend_0",        "f": function() { return path_bend(1,0,_dbg); } },
    { "name": "pathb_bend_1",        "f": function() { return path_bend(1,1,_dbg); } },
    { "name": "pathb_bend_2",        "f": function() { return path_bend(1,2,_dbg); } },
    { "name": "pathb_bend_3",        "f": function() { return path_bend(1,3,_dbg); } },

    { "name": "pathb_bendup_0",        "f": function() { return path_bend_up(1,0,_dbg); } },
    { "name": "pathb_bendup_1",        "f": function() { return path_bend_up(1,1,_dbg); } },
    { "name": "pathb_bendup_2",        "f": function() { return path_bend_up(1,2,_dbg); } },
    { "name": "pathb_bendup_3",        "f": function() { return path_bend_up(1,3,_dbg); } },
    { "name": "pathb_bendup_4",        "f": function() { return path_bend_up(1,4,_dbg); } },
    { "name": "pathb_bendup_5",        "f": function() { return path_bend_up(1,5,_dbg); } },
    { "name": "pathb_bendup_6",        "f": function() { return path_bend_up(1,6,_dbg); } },

    { "name": "pathb_stair_0",        "f": function() { return path_stair(1,_dbg); } },

    { "name": "pathb_cap_0",        "f": function() { return path_cap(1,0,_dbg); } },
    { "name": "pathb_cap_1",        "f": function() { return path_cap(1,1,_dbg); } },
    { "name": "pathb_cap_2",        "f": function() { return path_cap(1,2,_dbg); } },
    { "name": "pathb_cap_3",        "f": function() { return path_cap(1,3,_dbg); } },
    { "name": "pathb_cap_4",        "f": function() { return path_cap(1,4,_dbg); } },
    { "name": "pathb_cap_5",        "f": function() { return path_cap(1,5,_dbg); } },
    { "name": "pathb_cap_6",        "f": function() { return path_cap(1,6,_dbg); } },
    { "name": "pathb_cap_7",        "f": function() { return path_cap(1,7,_dbg); } },

    //---

    //{ "name": "column2",        "f": function() { return column2(); } },
    //{ "name": "column3",        "f": function() { return column3(); } },

    { "name": "wedge_up",     "f": function() { return wedge_up(); } },
    { "name": "wedge_down",   "f": function() { return wedge_down(); } },
    { "name": "doorway",      "f": function() { return doorway(); } },
    { "name": "double-doorway",      "f": function() { return double_doorway(); } },
    { "name": "block-2x2",      "f": function() { return block_2x2(); } },

    //{ "name": "arch0",        "f": function() { return arch0(); } },
    //{ "name": "arch1",        "f": function() { return arch1(); } },
    //{ "name": "arch2",        "f": function() { return arch2(); } },

    { "name": "arch0",        "f": function() { return arch0_s(); } },
    { "name": "arch1",        "f": function() { return arch1_s(); } },
    { "name": "arch2",        "f": function() { return arch2_s(); } },

    { "name": "stair",        "f": (function(_n){ return function() { return stair(_n); } })(5)  }
  ];


  for (let li_idx=0; li_idx<lib_info.length; li_idx++) {
    let li = lib_info[li_idx];

    let shape_info = li.f();

    let stl_data = op.stl_dumps({"binary":false}, shape_info[0].anchor).join("");
    fs.writeFileSync(OUT_DIR_STL + "/" + li.name + ".stl", stl_data);

    let obj_data = op.obj_dumps({}, shape_info[0].anchor).join("");
    fs.writeFileSync(OUT_DIR_OBJ + "/" + li.name + ".obj", obj_data);

    let src_idx = stickem_info.source.length;
    stickem_info.source.push({"name":li.name, "dock":[ shape_info[0].dock ]});
    for (let ii=1; ii<shape_info.length; ii++) {
      stickem_info.source[src_idx].dock.push( shape_info[ii].dock );
    }

  }

  _print_stickem_conf(stickem_info);
  //console.log(JSON.stringify(stickem_info, undefined, 2));

  return;
}

function show_help() {
  console.log("\nusage:");
  console.log("\n  node brutal-plum_parth.js [op]");
  console.log("");
  console.log("provide operation, one of:");
  console.log("");
  console.log("  nopath     - create tileset with no embedded paths (default)");
  console.log("  1path      - create tileset with 1 path");
  console.log("  2path      - create tileset with 2 paths");
  console.log("  help       - this screen");
  console.log("");
  console.log("will print out the stickem config file and create OBJ or STL files in:");
  console.log("  ", OUT_DIR_OBJ);
  console.log("  ", OUT_DIR_STL);
  console.log("");

}

function main(argv) {

  let op = "nopath";
  if (argv.length > 2) { op = argv[2]; }

  if (op == "help") {
    show_help();
    return;
  }

  if      (op == "nopath")  { return main_nopath(); }
  else if (op == "1path")   { return main_1path(); }
  else if (op == "2path")   { return main_2path(); }
  else {
    show_help();
  }

  return -1;
}

main(process.argv);
