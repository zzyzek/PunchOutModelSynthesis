// LICENSE: cc0
//
// To the extent possible under law, the person who associated CC0 with
// this code has waived all copyright and related or neighboring rights
// to this code.
//
// You should have received a copy of the CC0 legalcode along with this
// work.  If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//

// This program creates a POMS ramps_roads.json config file along with
// OBJ files for each of the tiles (including the .000 blank/empty tile).
// This is effectively the output of `stair.js` restricted to just the xy
// plane for roads, bends, etc.
//
// Outputs:
//
//   ramps_roads_poms.json        - POMS config file
//   ramps_roads_obj/...          - OBJ files for each of the tiles
//

var fs = require("fs");
var jeom = require("./jeom.js");

const TILE_WIDTH = 1/3;
const TILE_HEIGHT = 1/4;

//----
// Parts of the following were taken from https://webglfundamentals.org/
// https://github.com/gfxfundamentals/webgl-fundamentals
// which are used with permission via a BSD-3 clause license.
//

var m4 = {

  projection: function(width, height, depth) {

    // Note: This matrix flips the Y axis so 0 is at the top.
    //
    return [
       2 / width, 0, 0, 0,
       0, -2 / height, 0, 0,
       0, 0, 2 / depth, 0,
      -1, 1, 0, 1,
    ];
  },

  multiply: function(a, b) {
    var a00 = a[0 * 4 + 0];
    var a01 = a[0 * 4 + 1];
    var a02 = a[0 * 4 + 2];
    var a03 = a[0 * 4 + 3];
    var a10 = a[1 * 4 + 0];
    var a11 = a[1 * 4 + 1];
    var a12 = a[1 * 4 + 2];
    var a13 = a[1 * 4 + 3];
    var a20 = a[2 * 4 + 0];
    var a21 = a[2 * 4 + 1];
    var a22 = a[2 * 4 + 2];
    var a23 = a[2 * 4 + 3];
    var a30 = a[3 * 4 + 0];
    var a31 = a[3 * 4 + 1];
    var a32 = a[3 * 4 + 2];
    var a33 = a[3 * 4 + 3];
    var b00 = b[0 * 4 + 0];
    var b01 = b[0 * 4 + 1];
    var b02 = b[0 * 4 + 2];
    var b03 = b[0 * 4 + 3];
    var b10 = b[1 * 4 + 0];
    var b11 = b[1 * 4 + 1];
    var b12 = b[1 * 4 + 2];
    var b13 = b[1 * 4 + 3];
    var b20 = b[2 * 4 + 0];
    var b21 = b[2 * 4 + 1];
    var b22 = b[2 * 4 + 2];
    var b23 = b[2 * 4 + 3];
    var b30 = b[3 * 4 + 0];
    var b31 = b[3 * 4 + 1];
    var b32 = b[3 * 4 + 2];
    var b33 = b[3 * 4 + 3];
    return [
      b00 * a00 + b01 * a10 + b02 * a20 + b03 * a30,
      b00 * a01 + b01 * a11 + b02 * a21 + b03 * a31,
      b00 * a02 + b01 * a12 + b02 * a22 + b03 * a32,
      b00 * a03 + b01 * a13 + b02 * a23 + b03 * a33,
      b10 * a00 + b11 * a10 + b12 * a20 + b13 * a30,
      b10 * a01 + b11 * a11 + b12 * a21 + b13 * a31,
      b10 * a02 + b11 * a12 + b12 * a22 + b13 * a32,
      b10 * a03 + b11 * a13 + b12 * a23 + b13 * a33,
      b20 * a00 + b21 * a10 + b22 * a20 + b23 * a30,
      b20 * a01 + b21 * a11 + b22 * a21 + b23 * a31,
      b20 * a02 + b21 * a12 + b22 * a22 + b23 * a32,
      b20 * a03 + b21 * a13 + b22 * a23 + b23 * a33,
      b30 * a00 + b31 * a10 + b32 * a20 + b33 * a30,
      b30 * a01 + b31 * a11 + b32 * a21 + b33 * a31,
      b30 * a02 + b31 * a12 + b32 * a22 + b33 * a32,
      b30 * a03 + b31 * a13 + b32 * a23 + b33 * a33,
    ];
  },

  t2: function(tx, ty, tz) {
    return [
       1,  0,  0, tx,
       0,  1,  0, ty,
       0,  0,  1, tz,
       0,  0,  0,  1,
    ];
  },

  translation: function(tx, ty, tz) {
    return [
       1,  0,  0,  0,
       0,  1,  0,  0,
       0,  0,  1,  0,
       tx, ty, tz, 1,
    ];
  },

  xRotation: function(angleInRadians) {
    var c = Math.cos(angleInRadians);
    var s = Math.sin(angleInRadians);

    return [
      1, 0, 0, 0,
      0, c, s, 0,
      0, -s, c, 0,
      0, 0, 0, 1,
    ];
  },
  yRotation: function(angleInRadians) {
    var c = Math.cos(angleInRadians);
    var s = Math.sin(angleInRadians);

    return [
      c, 0, -s, 0,
      0, 1, 0, 0,
      s, 0, c, 0,
      0, 0, 0, 1,
    ];
  },

  zRotation: function(angleInRadians) {
    var c = Math.cos(angleInRadians);
    var s = Math.sin(angleInRadians);

    return [
       c, s, 0, 0,
      -s, c, 0, 0,
       0, 0, 1, 0,
       0, 0, 0, 1,
    ];
  },

  scaling: function(sx, sy, sz) {
    return [
      sx, 0,  0,  0,
      0, sy,  0,  0,
      0,  0, sz,  0,
      0,  0,  0,  1,
    ];
  },

  translate: function(m, tx, ty, tz) {
    return m4.multiply(m, m4.translation(tx, ty, tz));
  },

  xRotate: function(m, angleInRadians) {
    return m4.multiply(m, m4.xRotation(angleInRadians));
  },

  yRotate: function(m, angleInRadians) {
    return m4.multiply(m, m4.yRotation(angleInRadians));
  },

  zRotate: function(m, angleInRadians) {
    return m4.multiply(m, m4.zRotation(angleInRadians));
  },

  scale: function(m, sx, sy, sz) {
    return m4.multiply(m, m4.scaling(sx, sy, sz));
  },

};

//
//----

function _template_rot_mov(tplate, rx, ry, rz, tx, ty, tz) {
  tx = ((typeof tx === "undefined") ? 0 : tx);
  ty = ((typeof ty === "undefined") ? 0 : ty);
  tz = ((typeof tz === "undefined") ? 0 : tz);

  let tri = [];

  let mx = m4.xRotation(rx);
  let my = m4.yRotation(ry);
  let mz = m4.zRotation(rz);

  let mr = m4.multiply( m4.multiply(mx, my), mz );

  for (let p=0; p<tplate.length; p+=3) {
    tri.push(tx); tri.push(ty); tri.push(tz);
    for (let _i=0; _i<3; _i++) {
      for (let _j=0; _j<3; _j++) {
        tri[p+_i] += tplate[p+_j]*mr[4*_i + _j];
      }
    }
  }

  return tri;
}

function _p_mul_mov(v, s, tx, ty ,tz) {
  for (let i=0; i<v.length; i++) {
    v[i] *= s;

    if ((i%3)==0) { v[i] += tx; }
    if ((i%3)==1) { v[i] += ty; }
    if ((i%3)==2) { v[i] += tz; }
  }

  return v;
}

function _m_v_mul(m, v) {
  let tx = 0;
  let ty = 0;
  let tz = 0;

  let tri = [];

  tri.push(tx);
  tri.push(ty);
  tri.push(tz);
  for (let _i=0; _i<3; _i++) {
    for (let _j=0; _j<3; _j++) {
      tri[_i] += m[4*_i + _j] * v[_j];
    }
  }

  return tri;
}

function _dist3(u,v) {
  let dsq = (u[0] - v[0])*(u[0] - v[0]) +
            (u[1] - v[1])*(u[1] - v[1]) +
            (u[2] - v[2])*(u[2] - v[2]);

  return Math.sqrt(dsq);
}


function _v_in(v, va, _eps) {
  _eps = ((typeof _eps === "undefined") ? (1/128.0) : _eps);

  for (let i=0; i<va.length; i++) {
    //if (_dist3(v, va[i]) <= _eps) { return true; }
    if (_dist3(v, va[i]) <= _eps) { return i; }
  }
  //return false;
  return -1;
}



// template tiles.
// These will be rotated to build the whole tile library.
// The endpoints are there so that we can weed out duplicates
// (from our brute force rotate) and so we can see how each
// tile can join with the others.
//
let _template = {

  "admissible_pos" : [
    { "dv_key" : "1:0:0"  , "dv": [ 1,  0,  0] },
    { "dv_key" : "-1:0:0" , "dv": [-1,  0,  0] },

    { "dv_key" : "0:1:0"  , "dv": [ 0,  1,  0] },
    { "dv_key" : "0:-1:0" , "dv": [ 0, -1,  0] },

    { "dv_key" : "0:0:1"  , "dv": [ 0,  0,  1] },
    { "dv_key" : "0:0:-1" , "dv": [ 0,  0, -1] }
  ],

  "oppo" :  {
    "-1:0:0" :  "1:0:0",
    "1:0:0"  : "-1:0:0",

    "0:-1:0" :  "0:1:0",
    "0:1:0"  : "0:-1:0",

    "0:0:-1" : "0:0:1",
    "0:0:1"  : "0:0:-1"
  },

  // . - empty
  // s - start cap (?) (unused)
  // p - road (pipe)
  // r - bend
  // c - cross
  // v - stair
  // d - debug (unused)

  "weight": {
    ".": 1,
    //"s": 1,
    "p": 1,
    "r": 1,
    "c": 1,
    "v": 1
  },

  "pdf":  {
    ".": -1,
    //"s": 1,
    "p": 1,
    "r": 1,
    "c": 1,
    "v": 1
  },

  "cdf": [],

  // enpoints tell how we can connect to the other tiles
  //
  // these are flush with the interface plane but in a rectangular pattern, so
  // four points to an interface.
  //
  // The null ('.') and debug ('d') tiles don't have any interfaces
  //
  "endpoint": {},

  ".": [],
  "p" : [],
  "r" : [],
  "v" : [],
  "c" : [],
  "T" : []

};


function weight2pd(w) {
  let S = 0.0;
  let pdf = {};
  let cdf = [];

  for (key in w) { S += w[key]; }

  let cdf_s = 0.0;
  for (key in w) {
    pdf[key] = w[key] / S;

    cdf_s += pdf[key];
    cdf.push( {"key": key, "s": cdf_s} );
  }

  return { "pdf": pdf, "cdf": cdf, "w": w };
}


function _3rect_xz(w,h,x,y,z,o) {
  let tri_a = [];

  if (o) {
    let tri = [];
    tri.push( [ x-w/2, y, z+h/2 ] );
    tri.push( [ x+w/2, y, z+h/2 ] );
    tri.push( [ x-w/2, y, z-h/2 ] );
    tri_a.push(tri);

    tri = [];
    tri.push( [ x+w/2, y, z+h/2 ] );
    tri.push( [ x+w/2, y, z-h/2 ] );
    tri.push( [ x-w/2, y, z-h/2 ] );
    tri_a.push(tri);
  }
  else {
    let tri = [];
    tri.push( [ x-w/2, y, z+h/2 ] );
    tri.push( [ x-w/2, y, z-h/2 ] );
    tri.push( [ x+w/2, y, z+h/2 ] );
    tri_a.push(tri);

    tri = [];
    tri.push( [ x+w/2, y, z+h/2 ] );
    tri.push( [ x-w/2, y, z-h/2 ] );
    tri.push( [ x+w/2, y, z-h/2 ] );
    tri_a.push(tri);
  }

  return tri_a;
}

function _3rect_xy(w,h,x,y,z,o) {

  let tri_a = [];

  if (o) {
    let tri = [];
    tri.push( [ x-w/2, y+h/2, z ] );
    tri.push( [ x+w/2, y+h/2, z ] );
    tri.push( [ x-w/2, y-h/2, z ] );
    tri_a.push(tri);

    tri = [];
    tri.push( [ x+w/2, y+h/2, z ] );
    tri.push( [ x+w/2, y-h/2, z ] );
    tri.push( [ x-w/2, y-h/2, z ] );
    tri_a.push(tri);
  }
  else {
    let tri = [];
    tri.push( [ x-w/2, y+h/2, z ] );
    tri.push( [ x-w/2, y-h/2, z ] );
    tri.push( [ x+w/2, y+h/2, z ] );
    tri_a.push(tri);

    tri = [];
    tri.push( [ x+w/2, y+h/2, z ] );
    tri.push( [ x-w/2, y-h/2, z ] );
    tri.push( [ x+w/2, y-h/2, z ] );
    tri_a.push(tri);
  }

  return tri_a;
}


function _3rect_zy(w,h,x,y,z,o) {
  let tri_a = [];

  if (o) {
    let tri = [];
    tri.push( [ x, y+h/2, z-w/2 ] );
    tri.push( [ x, y+h/2, z+w/2 ] );
    tri.push( [ x, y-h/2, z-w/2 ] );
    tri_a.push(tri);

    tri = [];
    tri.push( [ x, y+h/2, z+w/2 ] );
    tri.push( [ x, y-h/2, z+w/2 ] );
    tri.push( [ x, y-h/2, z-w/2 ] );
    tri_a.push(tri);
  }
  else {
    let tri = [];
    tri.push( [ x, y+h/2, z-w/2 ] );
    tri.push( [ x, y-h/2, z-w/2 ] );
    tri.push( [ x, y+h/2, z+w/2 ] );
    tri_a.push(tri);

    tri = [];
    tri.push( [ x, y+h/2, z+w/2 ] );
    tri.push( [ x, y-h/2, z-w/2 ] );
    tri.push( [ x, y-h/2, z+w/2 ] );
    tri_a.push(tri);
  }

  return tri_a;
}


// the geometry for some of the more copmlex shapes is a little
// too involved to list out statically so do it here.
//
function init_template(template) {

  let _g_w = TILE_WIDTH;
  let _g_h = TILE_HEIGHT;

  let _g_epd = 0;
  let _plat_del = 0;
  let _p_w = _g_w + _plat_del;


  let S = 0;
  for (let _tsn in template.weight) {
    S += template.weight[_tsn];
  }

  let cdf_s = 0;
  for (let _tsn in template.weight) {
    template.pdf[_tsn] = template.weight[_tsn]/S;
    cdf_s += template.pdf[_tsn];
    template.cdf.push( { "tile_code": _tsn, "s": cdf_s } );
  }
  template.cdf[ template.cdf.length-1 ].s = 1.01;

  // enpoints tell how we can connect to the other tiles
  //
  // these are flush with the interface plane but in a rectangular pattern, so
  // four points to an interface.
  //
  // The null ('.') and debug ('d') tiles don't have any interfaces
  //
  template["endpoint"] = {

    ".": [],

    //"d": [],

    // pipe:
    // 
    //   road sits in xy plane
    //
    "p": [
      [ -_g_w/2, -1/2, -_g_h/2 ], [ _g_w/2, -1/2, -_g_h/2 ],
      [ -_g_w/2, -1/2, +_g_h/2 ], [ _g_w/2, -1/2, +_g_h/2 ],

      [ -_g_w/2,  1/2, -_g_h/2 ], [  _g_w/2,  1/2, -_g_h/2 ],
      [ -_g_w/2,  1/2, +_g_h/2 ], [  _g_w/2,  1/2, +_g_h/2 ]
    ],

    /*
    "s": [
      [ -_g_w/2, -1/2, -_g_h/2 ], [  _g_w/2, -1/2, -_g_h/2 ],
      [ -_g_w/2, -1/2, +_g_h/2 ], [  _g_w/2, -1/2, +_g_h/2 ]
    ],
    */

    // bend:
    //
    //   bend sits in xy plane
    //
    "r": [
      [ -_g_w/2, -1/2, -_g_h/2 ], [  _g_w/2, -1/2, -_g_h/2 ],
      [ -_g_w/2, -1/2, +_g_h/2 ], [  _g_w/2, -1/2, +_g_h/2 ],

      [  1/2,  _g_w/2, -_g_h/2 ], [  1/2, -_g_w/2, -_g_h/2 ],
      [  1/2,  _g_w/2, +_g_h/2 ], [  1/2, -_g_w/2, +_g_h/2 ]
    ],


    // cross:
    //
    //   cross sits in xy plane
    //
    "c": [
      [ -_g_w/2,  1/2, -_g_h/2 ], [  _g_w/2,  1/2, -_g_h/2 ],
      [ -_g_w/2,  1/2, +_g_h/2 ], [  _g_w/2,  1/2, +_g_h/2 ],

      [ -_g_w/2, -1/2, -_g_h/2 ], [  _g_w/2, -1/2, -_g_h/2 ],
      [ -_g_w/2, -1/2, +_g_h/2 ], [  _g_w/2, -1/2, +_g_h/2 ],

      [  1/2,  _g_w/2, -_g_h/2 ], [  1/2, -_g_w/2, -_g_h/2 ],
      [  1/2,  _g_w/2, +_g_h/2 ], [  1/2, -_g_w/2, +_g_h/2 ],

      [ -1/2,  _g_w/2, -_g_h/2 ], [ -1/2, -_g_w/2, -_g_h/2 ],
      [ -1/2,  _g_w/2, +_g_h/2 ], [ -1/2, -_g_w/2, +_g_h/2 ]
    ],

    // "tee":
    //
    //   xy plane
    //
    "T": [
      [  _g_w/2, -1/2, -_g_h/2 ], [ -_g_w/2, -1/2, -_g_h/2 ],
      [  _g_w/2, -1/2, +_g_h/2 ], [ -_g_w/2, -1/2, +_g_h/2 ],

      [ -1/2,  _g_w/2, -_g_h/2 ], [ -1/2, -_g_w/2, -_g_h/2 ],
      [ -1/2,  _g_w/2, +_g_h/2 ], [ -1/2, -_g_w/2, +_g_h/2 ],

      [  1/2,  _g_w/2, -_g_h/2 ], [  1/2, -_g_w/2, -_g_h/2 ],
      [  1/2,  _g_w/2, +_g_h/2 ], [  1/2, -_g_w/2, +_g_h/2 ]
    ],

    // stair:
    //
    //   connects bottom(y-) xz plane to top(z+) xy plane
    //   the xz plane will connect to other objects (above)
    //   that sit in the xy plane.
    //
    //   stairs serve double duty (as do most of the above, if
    //   desired) in that the 'flipped' version is symmetric
    //   and connected.
    //   This means we don't need another stair element to
    //   connect this stair to the rest of the in-plane,
    //   but on another leve/tier, ramps.
    //
    //   This is the only tile that needs to be flipped by
    //   180 around the x vector.
    //
    "v": [
      [  _g_w/2, -1/2, -_g_h/2 ], [ -_g_w/2, -1/2, -_g_h/2 ],
      [  _g_w/2, -1/2, +_g_h/2 ], [ -_g_w/2, -1/2, +_g_h/2 ],

      [  _g_w/2,  +_g_h/2,  1/2 ], [ -_g_w/2, +_g_h/2,  1/2 ],
      [  _g_w/2,  -_g_h/2,  1/2 ], [ -_g_w/2, -_g_h/2,  1/2 ]

    ]

  };

  // simple plane for debuging
  //
  let debug_plane = false;
  if (debug_plane) {
    template["d"] = [
      -1/2,  1/2, 0,  -1/2, -1/2, 0,    1/2,  1/2, 0,
      -1/2, -1/2, 0,   1/2, -1/2, 0,    1/2,  1/2, 0,
      -1/2,  1/2, 0,   1/2,  1/2, 0,   -1/2, -1/2, 0,
      -1/2, -1/2, 0,   1/2,  1/2, 0,    1/2, -1/2, 0
    ];
  }

  template["endpoint_order"] = [ ".", "p", "r", "c", "T", "v" ];

  template["p"] = [

    // front panel
    //
    -_g_w/2,  1/2, -_g_h/2,  _g_w/2,  1/2, -_g_h/2,   -_g_w/2, -1/2, -_g_h/2,
     _g_w/2,  1/2, -_g_h/2,  _g_w/2, -1/2, -_g_h/2,   -_g_w/2, -1/2, -_g_h/2,

    // back panel
    //
    -_g_w/2,  1/2, +_g_h/2, -_g_w/2, -1/2, +_g_h/2,   _g_w/2,  1/2, +_g_h/2,
     _g_w/2,  1/2, +_g_h/2, -_g_w/2, -1/2, +_g_h/2,   _g_w/2, -1/2, +_g_h/2,

    // left side stripe
    //
    -_g_w/2,  1/2, -_g_h/2,   -_g_w/2, -1/2, -_g_h/2,  -_g_w/2, -1/2, +_g_h/2,
    -_g_w/2,  1/2, -_g_h/2,   -_g_w/2, -1/2, +_g_h/2,  -_g_w/2,  1/2, +_g_h/2,


    // right side stripe
    //
     _g_w/2,  1/2, -_g_h/2,   _g_w/2, -1/2, +_g_h/2,  _g_w/2, -1/2, -_g_h/2,
     _g_w/2,  1/2, -_g_h/2,   _g_w/2,  1/2, +_g_h/2,  _g_w/2, -1/2, +_g_h/2,

    // back cap (optional)
    //
    -_g_w/2,  1/2, -_g_h/2,   -_g_w/2,  1/2, +_g_h/2,   _g_w/2,  1/2, -_g_h/2,
     _g_w/2,  1/2, -_g_h/2,   -_g_w/2,  1/2, +_g_h/2,   _g_w/2,  1/2, +_g_h/2,

    // front cap (optional)
    //
    -_g_w/2, -1/2, -_g_h/2,   _g_w/2, -1/2, +_g_h/2,    -_g_w/2, -1/2, +_g_h/2,
     _g_w/2, -1/2, -_g_h/2,   _g_w/2, -1/2, +_g_h/2,    -_g_w/2, -1/2, -_g_h/2

  ];


  /*
  // cap (?)
  //
  template["s"] =  [

    // front panel
    //
    -_p_w/2,    0, -_g_h/2,  _p_w/2,    0, -_g_h/2,   -_g_w/2, -1/2, -_g_h/2,
     _p_w/2,    0, -_g_h/2,  _g_w/2, -1/2, -_g_h/2,   -_g_w/2, -1/2, -_g_h/2,

    // back panel
    //
    -_p_w/2,    0, +_g_h/2, -_g_w/2, -1/2, +_g_h/2,   _p_w/2,    0, +_g_h/2,
     _p_w/2,    0, +_g_h/2, -_g_w/2, -1/2, +_g_h/2,   _g_w/2, -1/2, +_g_h/2,

    // left side stripe
    //
    -_p_w/2,    0, -_g_h/2,   -_g_w/2, -1/2, -_g_h/2,  -_g_w/2, -1/2, +_g_h/2,
    -_p_w/2,    0, -_g_h/2,   -_g_w/2, -1/2, +_g_h/2,  -_p_w/2,    0, +_g_h/2,

    // right side stripe
    //
     _p_w/2,    0, -_g_h/2,   _g_w/2, -1/2, +_g_h/2,  _g_w/2, -1/2, -_g_h/2,
     _p_w/2,    0, -_g_h/2,   _p_w/2,    0, +_g_h/2,  _g_w/2, -1/2, +_g_h/2,

    // back cap (not optional anymore);
    //
    -_p_w/2,    0, -_g_h/2,   -_p_w/2,    0, +_g_h/2,   _p_w/2,    0, -_g_h/2,
     _p_w/2,    0, -_g_h/2,   -_p_w/2,    0, +_g_h/2,   _p_w/2,    0, +_g_h/2,

    // front cap (optional)
    //
    -_g_w/2, -1/2, -_g_h/2,   _g_w/2, -1/2, +_g_h/2,    -_g_w/2, -1/2, +_g_h/2,
     _g_w/2, -1/2, -_g_h/2,   _g_w/2, -1/2, +_g_h/2,    -_g_w/2, -1/2, -_g_h/2

  ];
  */


  // 'r'
  // top
  //

  // bottom square
  //
  let fr = [];
  let tri = [];
  tri.push( [ -_g_w/2, -1/2,    -_g_h/2 ] );
  tri.push( [  _g_w/2, -_g_w/2, -_g_h/2 ] );
  tri.push( [  _g_w/2, -1/2,    -_g_h/2 ] );
  fr.push(tri);


  tri = [];
  tri.push( [ -_g_w/2, -1/2,    -_g_h/2 ] );
  tri.push( [ -_g_w/2, -_g_w/2, -_g_h/2 ] );
  tri.push( [  _g_w/2, -_g_w/2, -_g_h/2 ] );
  fr.push(tri);

  // connecting triangle
  //
  tri = [];
  tri.push( [ -_g_w/2, -_g_w/2, -_g_h/2 ] );
  tri.push( [  _g_w/2,  _g_w/2, -_g_h/2 ] );
  tri.push( [  _g_w/2, -_g_w/2, -_g_h/2 ] );
  fr.push(tri);

  // right square
  //
  tri = [];
  tri.push( [  1/2,    -_g_w/2, -_g_h/2 ] )
  tri.push( [  _g_w/2,  _g_w/2, -_g_h/2 ] )
  tri.push( [  1/2,     _g_w/2, -_g_h/2 ] )
  fr.push(tri);

  tri = [];
  tri.push( [  1/2,    -_g_w/2, -_g_h/2 ] )
  tri.push( [  _g_w/2, -_g_w/2, -_g_h/2 ] )
  tri.push( [  _g_w/2,  _g_w/2, -_g_h/2 ] )
  fr.push(tri);

  //--

  // bottom
  //

  // bottom square
  //
  tri = [];
  tri.push( [ -_g_w/2, -1/2,    +_g_h/2 ] );
  tri.push( [  _g_w/2, -1/2,    +_g_h/2 ] );
  tri.push( [  _g_w/2, -_g_w/2, +_g_h/2 ] );
  fr.push(tri);

  tri = [];
  tri.push( [ -_g_w/2, -1/2,    +_g_h/2 ] );
  tri.push( [  _g_w/2, -_g_w/2, +_g_h/2 ] );
  tri.push( [ -_g_w/2, -_g_w/2, +_g_h/2 ] );
  fr.push(tri);

  // connecting triangle
  //
  tri = [];
  tri.push( [ -_g_w/2, -_g_w/2, +_g_h/2 ] );
  tri.push( [  _g_w/2, -_g_w/2, +_g_h/2 ] );
  tri.push( [  _g_w/2,  _g_w/2, +_g_h/2 ] );
  fr.push(tri);

  // right square
  //
  tri = [];
  tri.push( [  1/2,    -_g_w/2, +_g_h/2 ] )
  tri.push( [  1/2,     _g_w/2, +_g_h/2 ] )
  tri.push( [  _g_w/2,  _g_w/2, +_g_h/2 ] )
  fr.push(tri);

  tri = [];
  tri.push( [  1/2,    -_g_w/2, +_g_h/2 ] )
  tri.push( [  _g_w/2,  _g_w/2, +_g_h/2 ] )
  tri.push( [  _g_w/2, -_g_w/2, +_g_h/2 ] )
  fr.push(tri);

  //--

  // front edge
  //
  tri = [];
  tri.push( [ -_g_w/2, -1/2, -_g_h/2 ] )
  tri.push( [  _g_w/2, -1/2, -_g_h/2 ] )
  tri.push( [  _g_w/2, -1/2, +_g_h/2 ] )
  fr.push(tri);

  tri = [];
  tri.push( [ -_g_w/2, -1/2, +_g_h/2 ] )
  tri.push( [ -_g_w/2, -1/2, -_g_h/2 ] )
  tri.push( [  _g_w/2, -1/2, +_g_h/2 ] )
  fr.push(tri);

  // front right edge
  //
  tri = [];
  tri.push( [  _g_w/2, -1/2,    -_g_h/2 ] )
  tri.push( [  _g_w/2, -_g_w/2, -_g_h/2 ] )
  tri.push( [  _g_w/2, -1/2,    +_g_h/2 ] )
  fr.push(tri);

  tri = [];
  tri.push( [  _g_w/2, -1/2,    +_g_h/2 ] )
  tri.push( [  _g_w/2, -_g_w/2, -_g_h/2 ] )
  tri.push( [  _g_w/2, -_g_w/2, +_g_h/2 ] )
  fr.push(tri);

  // front left edge
  //
  tri = [];
  tri.push( [ -_g_w/2, -1/2,    -_g_h/2 ] )
  tri.push( [ -_g_w/2, -1/2,    +_g_h/2 ] )
  tri.push( [ -_g_w/2, -_g_w/2, -_g_h/2 ] )
  fr.push(tri);

  tri = [];
  tri.push( [ -_g_w/2, -1/2,    +_g_h/2 ] )
  tri.push( [ -_g_w/2, -_g_w/2, +_g_h/2 ] )
  tri.push( [ -_g_w/2, -_g_w/2, -_g_h/2 ] )
  fr.push(tri);

  //---

  // right edge
  //
  tri = [];
  tri.push( [ 1/2, -_g_w/2, -_g_h/2 ] )
  tri.push( [ 1/2,  _g_w/2, -_g_h/2 ] )
  tri.push( [ 1/2,  _g_w/2, +_g_h/2 ] )
  fr.push(tri);

  tri = [];
  tri.push( [ 1/2, -_g_w/2, +_g_h/2 ] )
  tri.push( [ 1/2, -_g_w/2, -_g_h/2 ] )
  tri.push( [ 1/2,  _g_w/2, +_g_h/2 ] )
  fr.push(tri);

  // right up edge
  //
  tri = [];
  tri.push( [ 1/2,     _g_w/2, -_g_h/2 ] )
  tri.push( [ _g_w/2,  _g_w/2, -_g_h/2 ] )
  tri.push( [ 1/2,     _g_w/2, +_g_h/2 ] )
  fr.push(tri);

  tri = [];
  tri.push( [ _g_w/2,  _g_w/2, -_g_h/2 ] )
  tri.push( [ _g_w/2,  _g_w/2, +_g_h/2 ] )
  tri.push( [ 1/2,     _g_w/2, +_g_h/2 ] )
  fr.push(tri);

  // right down edge
  //
  tri = [];
  tri.push( [ 1/2,    -_g_w/2, -_g_h/2 ] )
  tri.push( [ 1/2,    -_g_w/2, +_g_h/2 ] )
  tri.push( [ _g_w/2, -_g_w/2, -_g_h/2 ] )
  fr.push(tri);

  tri = [];
  tri.push( [ _g_w/2, -_g_w/2, -_g_h/2 ] )
  tri.push( [ 1/2,    -_g_w/2, +_g_h/2 ] )
  tri.push( [ _g_w/2, -_g_w/2, +_g_h/2 ] )
  fr.push(tri);

  // diagnoal connecting edge
  //
  tri = [];
  tri.push( [ -_g_w/2,  -_g_w/2, -_g_h/2 ] )
  tri.push( [  _g_w/2,   _g_w/2, +_g_h/2 ] )
  tri.push( [  _g_w/2,   _g_w/2, -_g_h/2 ] )
  fr.push(tri);

  tri = [];
  tri.push( [  _g_w/2,   _g_w/2, +_g_h/2 ] )
  tri.push( [ -_g_w/2,  -_g_w/2, -_g_h/2 ] )
  tri.push( [ -_g_w/2,  -_g_w/2, +_g_h/2 ] )
  fr.push(tri);

  let flat_fr = [];
  for (let i=0; i<fr.length; i++) {
    for (let j=0; j<fr[i].length; j++) {
      for (let k=0; k<fr[i][j].length; k++) {
        flat_fr.push( fr[i][j][k] );
      }
    }
  }

  template["r"] = flat_fr;



  // stairs (v)
  //
  //     ^
  //     |           x-
  //     z         .xx-
  //     |         xxx
  //     ._y_>    -xx.
  //    /         -x
  //   x
  //  /
  // L

  let parity = 1;

  let st = [];
  let _st_n = Math.floor( (2/_g_h) + _g_h );
  let _st_m = Math.floor( (1/_g_h) + _g_h );

  let _st_ds = 1/_st_n;
  let _st_dy = 1/_st_n;
  let _st_dz = 1/_st_n;

  let _r = {};
  let dx=0, dy=0, dz=0;

  // front facing step
  //
  for (let i=0; i<(_st_m-1); i++) {
    dx = 0;
    dy = -0.5 + (i*_st_ds);
    dz = ((3/2)*_st_ds) + (i*_st_ds);
    _r = _3rect_xz( _g_w, _st_ds,
      dx, dy, dz, 1-parity);
    for (let j=0; j<_r.length; j++) { st.push(_r[j]); }

    // up facing top step
    //
    dx = 0;
    dy = -0.5 + ((1/2)*_st_ds) + (i*_st_ds);
    dz = (2*_st_ds) + (i*_st_ds);
    _r = _3rect_xy( _g_w, _st_ds,
      dx, dy, dz, 1-parity);
    for (let j=0; j<_r.length; j++) { st.push(_r[j]); }

    // right side stair
    //
    dx = _g_w/2;
    dy = -0.5 + _st_ds/2 + i*_st_ds;
    dz = +_st_ds/2 + i*_st_ds;
    _r = _3rect_zy(
      3*_st_ds, _st_ds,
      dx, dy, dz,
      parity);
    for (let j=0; j<_r.length; j++) { st.push(_r[j]); }


    // left side stair
    //
    dx = -_g_w/2;
    dy = -0.5 + _st_ds/2 + i*_st_ds;
    dz = +_st_ds/2 + i*_st_ds;
    _r = _3rect_zy(
      3*_st_ds, _st_ds,
      dx, dy, dz,
      1-parity);
    for (let j=0; j<_r.length; j++) { st.push(_r[j]); }

  }

  for (let i=0; i<(_st_m+1); i++) {

    // back facing step
    //
    dx = 0;
    dy = -0.5 + _st_ds + (i*_st_ds);
    dz =  - ((1/2)*_st_ds) + (i*_st_ds);
    _r = _3rect_xz( _g_w, _st_ds,
      dx, dy, dz, parity);
    for (let j=0; j<_r.length; j++) { st.push(_r[j]); }

    // bottom facing bottom step
    //
    dx = 0;
    dy = -0.5 + ((1/2)*_st_ds) + (i*_st_ds);
    dz =  - (_st_ds) + (i*_st_ds);
    _r = _3rect_xy( _g_w, _st_ds,
      dx, dy, dz, parity);
    for (let j=0; j<_r.length; j++) { st.push(_r[j]); }

  }

  // right side stair
  //
  dx = _g_w/2;
  dy =  - (_st_ds/2);
  dz = 0.5 - _st_ds;
  _r = _3rect_zy(
    2*_st_ds, _st_ds,
    dx, dy, dz,
    parity);
  for (let j=0; j<_r.length; j++) { st.push(_r[j]); }

  dx = _g_w/2;
  dy =  + (_st_ds/2);
  dz = 0.5 - (_st_ds/2);
  _r = _3rect_zy(
    1*_st_ds, _st_ds,
    dx, dy, dz,
    parity);
  for (let j=0; j<_r.length; j++) { st.push(_r[j]); }

  // left side stair
  //
  dx = -_g_w/2;
  dy = - (_st_ds/2);
  dz = 0.5 - _st_ds;
  _r = _3rect_zy(
    2*_st_ds, _st_ds,
    dx, dy, dz,
    1-parity);
  for (let j=0; j<_r.length; j++) { st.push(_r[j]); }


  dx = -_g_w/2;
  dy = + (_st_ds/2);
  dz = 0.5 - (_st_ds/2);
  _r = _3rect_zy(
    1*_st_ds, _st_ds,
    dx, dy, dz,
    1-parity);
  for (let j=0; j<_r.length; j++) { st.push(_r[j]); }


  // optional end caps
  //
  let _st_endcap = true;
  if (_st_endcap) {
    let _r = {};
    let dx=0, dy=0, dz=0;

    // front endcap
    //
    _r = _3rect_xz( _g_w, _g_h,
      0, -0.5, 0, 0);
    for (let j=0; j<_r.length; j++) { st.push(_r[j]); }

    // top endcap
    //
    _r = _3rect_xy( _g_w, _g_h,
      0, 0, 0.5, 0);
    for (let j=0; j<_r.length; j++) { st.push(_r[j]); }
  }


  let flat_st = [];
  for (let i=0; i<st.length; i++) {
    for (let j=0; j<st[i].length; j++) {
      for (let k=0; k<st[i][j].length; k++) {
        flat_st.push( st[i][j][k] );
      }
    }
  }

  template["v"] = flat_st;


  //---
  // T
  //

  // t faces (top and bottom)
  //
  let T = [];

  _r = _3rect_xy( 1, _g_w, 0, 0, -_g_h/2, 1);
  for (let j=0; j<_r.length; j++) { T.push(_r[j]); }

  _r = _3rect_xy(_g_w, (1-_g_w)/2, 0, -1/2 + (1-_g_w)/4, -_g_h/2, 1);
  for (let j=0; j<_r.length; j++) { T.push(_r[j]); }

  _r = _3rect_xy( 1, _g_w, 0, 0, +_g_h/2, 0);
  for (let j=0; j<_r.length; j++) { T.push(_r[j]); }
  _r = _3rect_xy(_g_w, (1-_g_w)/2, 0, -1/2 + (1-_g_w)/4, +_g_h/2, 0);
  for (let j=0; j<_r.length; j++) { T.push(_r[j]); }

  // optional...
  // bottom and top
  //
  _r = _3rect_xz( _g_w, _g_h, 0, -1/2, 0, 0);
  for (let j=0; j<_r.length; j++) { T.push(_r[j]); }
  _r = _3rect_xz( 1, _g_h, 0, _g_w/2, 0, 1);
  for (let j=0; j<_r.length; j++) { T.push(_r[j]); }

  // optional...
  // left and right
  //
  _r = _3rect_zy( _g_h, _g_w, 1/2, 0, 0, 1);
  for (let j=0; j<_r.length; j++) { T.push(_r[j]); }
  _r = _3rect_zy( _g_h, _g_w,-1/2, 0, 0, 0);
  for (let j=0; j<_r.length; j++) { T.push(_r[j]); }

  // inner caps
  //
  _r = _3rect_zy( _g_h, (1-_g_w)/2, -_g_w/2, -1/2+(1-_g_w)/4, 0, 0);
  for (let j=0; j<_r.length; j++) { T.push(_r[j]); }
  _r = _3rect_zy( _g_h, (1-_g_w)/2,  _g_w/2, -1/2+(1-_g_w)/4, 0, 1);
  for (let j=0; j<_r.length; j++) { T.push(_r[j]); }

  _r = _3rect_xz( (1-_g_w)/2, _g_h, -1/2+(1-_g_w)/4, -_g_w/2, 0, 0);
  for (let j=0; j<_r.length; j++) { T.push(_r[j]); }

  _r = _3rect_xz( (1-_g_w)/2, _g_h,  1/2-(1-_g_w)/4, -_g_w/2, 0, 0);
  for (let j=0; j<_r.length; j++) { T.push(_r[j]); }

  let flat_T = [];
  for (let i=0; i<T.length; i++) {
    for (let j=0; j<T[i].length; j++) {
      for (let k=0; k<T[i][j].length; k++) {
        flat_T.push( T[i][j][k] );
      }
    }
  }

  template["T"] = flat_T;

  //---
  //+
  //

  // c (cross) faces (top and bottom)
  //
  let pl = [];

  _r = _3rect_xy( 1, _g_w, 0, 0, -_g_h/2, 1);
  for (let j=0; j<_r.length; j++) { pl.push(_r[j]); }

  _r = _3rect_xy(_g_w, (1-_g_w)/2, 0, -1/2 + (1-_g_w)/4, -_g_h/2, 1);
  for (let j=0; j<_r.length; j++) { pl.push(_r[j]); }

  _r = _3rect_xy(_g_w, (1-_g_w)/2, 0,  1/2 - (1-_g_w)/4, -_g_h/2, 1);
  for (let j=0; j<_r.length; j++) { pl.push(_r[j]); }

  _r = _3rect_xy( 1, _g_w, 0, 0, +_g_h/2, 0);
  for (let j=0; j<_r.length; j++) { pl.push(_r[j]); }

  _r = _3rect_xy(_g_w, (1-_g_w)/2, 0, -1/2 + (1-_g_w)/4, +_g_h/2, 0);
  for (let j=0; j<_r.length; j++) { pl.push(_r[j]); }
  _r = _3rect_xy(_g_w, (1-_g_w)/2, 0,  1/2 - (1-_g_w)/4, +_g_h/2, 0);
  for (let j=0; j<_r.length; j++) { pl.push(_r[j]); }

  // optional...
  // bottom and top
  //
  _r = _3rect_xz( _g_w, _g_h, 0, -1/2, 0, 0);
  for (let j=0; j<_r.length; j++) { pl.push(_r[j]); }
  _r = _3rect_xz( _g_w, _g_h, 0,  1/2, 0, 1);
  for (let j=0; j<_r.length; j++) { pl.push(_r[j]); }

  // optional...
  // left and right
  //
  _r = _3rect_zy( _g_h, _g_w,  1/2, 0, 0, 1);
  for (let j=0; j<_r.length; j++) { pl.push(_r[j]); }
  _r = _3rect_zy( _g_h, _g_w, -1/2, 0, 0, 0);
  for (let j=0; j<_r.length; j++) { pl.push(_r[j]); }

  // middle caps
  //
  let _mc = (1/2) - ((1-_g_w)/4);
  _r = _3rect_zy( _g_h, (1-_g_w)/2, -_g_w/2, -_mc, 0, 0);
  for (let j=0; j<_r.length; j++) { pl.push(_r[j]); }
  _r = _3rect_zy( _g_h, (1-_g_w)/2,  _g_w/2, -_mc, 0, 1);
  for (let j=0; j<_r.length; j++) { pl.push(_r[j]); }

  _r = _3rect_zy( _g_h, (1-_g_w)/2, -_g_w/2,  _mc, 0, 0);
  for (let j=0; j<_r.length; j++) { pl.push(_r[j]); }
  _r = _3rect_zy( _g_h, (1-_g_w)/2,  _g_w/2,  _mc, 0, 1);
  for (let j=0; j<_r.length; j++) { pl.push(_r[j]); }

  //--

  _r = _3rect_xz( (1-_g_w)/2, _g_h, -_mc, -_g_w/2, 0, 0);
  for (let j=0; j<_r.length; j++) { pl.push(_r[j]); }
  _r = _3rect_xz( (1-_g_w)/2, _g_h,  _mc, -_g_w/2, 0, 0);
  for (let j=0; j<_r.length; j++) { pl.push(_r[j]); }

  _r = _3rect_xz( (1-_g_w)/2, _g_h, -_mc,  _g_w/2, 0, 1);
  for (let j=0; j<_r.length; j++) { pl.push(_r[j]); }
  _r = _3rect_xz( (1-_g_w)/2, _g_h,  _mc,  _g_w/2, 0, 1);
  for (let j=0; j<_r.length; j++) { pl.push(_r[j]); }

  let flat_pl = [];
  for (let i=0; i<pl.length; i++) {
    for (let j=0; j<pl[i].length; j++) {
      for (let k=0; k<pl[i][j].length; k++) {
        flat_pl.push( pl[i][j][k] );
      }
    }
  }

  template["c"] = flat_pl;


  //----

}


// by using the endpoint library,
// build a list of all rotations of the endpoints (raw_lib),
// a map of equivalent representations (equiv_lib),
// a map that has the representative of each rotational class (repr_map),
// a 'tile_attach' list of tiles that can attach to each other (based off
// of the endpoint)
//
function build_tile_library( template ) {
  let raw_lib = {};
  let rot_lib = {};

  // equiv_map holds array of equivalent rotations
  //
  let _equiv_map = {};

  // repr map takes a single representation for all
  // equivalent rotations
  //
  let _repr_map = {};

  // *all* admissible nieghbors
  //
  let admissible_tile_pos = {};
  let admissible_pos_tile = {};
  let admissible_nei = {};

  let _endp_lib = template.endpoint;
  let _endp_order = template.endpoint_order;

  for (let pkey_idx=0; pkey_idx<_endp_order.length; pkey_idx++) {
    let pkey = _endp_order[pkey_idx];

    let _endp = _endp_lib[pkey];

    let _type_a = [];
    let _type_a_key = [];

    let xidx_dn = 4;
    let yidx_dn = 4;

    // special case of the ramp, make sure to
    // rotate 180 around x axis.
    //
    if (pkey == "v") { xidx_dn = 2; }

    for (let xidx=0; xidx<4; xidx+=xidx_dn) {
      for (let yidx=0; yidx<4; yidx+=yidx_dn) {
        for (let zidx=0; zidx<4; zidx++) {

          mx = m4.xRotation(xidx*Math.PI/2);
          my = m4.yRotation(yidx*Math.PI/2);
          mz = m4.zRotation(zidx*Math.PI/2);
          let mr = m4.multiply( m4.multiply(mx, my), mz );

          let ukey = pkey + xidx.toString() + yidx.toString() + zidx.toString();
          raw_lib[ukey] = [];

          // rotate endpoints around the various pi/2 rotations (x,y,z)
          // and put them in the 'raw' library
          //
          for (let ep_idx=0; ep_idx<_endp.length; ep_idx++) {
            let v = _m_v_mul(mr, _endp[ep_idx]);
            raw_lib[ukey].push(v);
          }

          _type_a.push( raw_lib[ukey] );
          _type_a_key.push(ukey);

          rot_lib[ukey] = { "m": [mx, my, mz], "r": [xidx, yidx, zidx ] };

        }
      }
    }

    // once in the library, go through and compare each
    // to see if they're equivalent by seeing if the endpoints
    // line up
    //
    for (let i=0; i<_type_a.length; i++) {
      for (let j=i; j<_type_a.length; j++) {

        let k=0;
        for (k=0; k<_type_a[i].length; k++) {
          if (_v_in(_type_a[i][k], _type_a[j])<0) { break; }
        }

        if (k==_type_a[i].length) {
          if (!(_type_a_key[i] in _equiv_map)) { _equiv_map[_type_a_key[i]] = []; }
          if (!(_type_a_key[j] in _equiv_map)) { _equiv_map[_type_a_key[j]] = []; }
          _equiv_map[_type_a_key[i]].push( _type_a_key[j] );
          _equiv_map[_type_a_key[j]].push( _type_a_key[i] );
        }

      }
    }

    // once we've constructed the equiv_map, go through and pick out the
    // representative
    //
    for (let key in _equiv_map) {
      if (key in _repr_map) { continue; }
      _repr_map[key] = key;
      for (let ii=0; ii<_equiv_map[key].length; ii++) {
        _repr_map[_equiv_map[key][ii]] = key;
      }
    }

  }

  let uniq_repr = {};
  for (let key in _repr_map) {
    let repr_id = _repr_map[key];
    if (!(repr_id in uniq_repr)) {
      uniq_repr[repr_id] = {"count":0, "attach_dv": {}, "pos_tile_attach":{} };
    }
    uniq_repr[repr_id].count++;
  }

  let tile_attach = {};

  // now for each representative, test every other tile to see if can
  // be joined
  //
  for (let anchor_key in uniq_repr) {

    let _tp_tri = template[ anchor_key[0] ];
    let _rx = rot_lib[ anchor_key].r[0] * Math.PI * 0.5;
    let _ry = rot_lib[ anchor_key].r[1] * Math.PI * 0.5;
    let _rz = rot_lib[ anchor_key].r[2] * Math.PI * 0.5;

    uniq_repr[anchor_key].tri = _template_rot_mov( _tp_tri, _rx, _ry, _rz );

    if ((anchor_key.charAt(0) == '.') ||
        (anchor_key.charAt(0) == 'd')) { continue; }
    for (let test_key in uniq_repr) {
      if ((test_key.charAt(0) == '.') ||
          (test_key.charAt(0) == 'd')) { continue; }

      // anchor_key is at the 'center' where test_key
      // is shifted around the 3x3x3 cube, excluding the
      // center point.
      //
      for (let dx=-1; dx<2; dx++) {
        for (let dy=-1; dy<2; dy++) {
          for (let dz=-1; dz<2; dz++) {

            // test_key can't be at center because that's where the
            // anchor_key is
            //
            if ((dx==0) && (dy==0) && (dz==0)) { continue; }

            //---

            let anch_endp = raw_lib[anchor_key];
            let test_endp = raw_lib[test_key];

            let endp_group = -1;
            let endp_count = 0;
            for (let idx=0; idx<test_endp.length; idx++) {
              let tv = [ test_endp[idx][0] + dx, test_endp[idx][1] + dy, test_endp[idx][2] + dz ];
              let _anch_endp_pos = _v_in(tv, anch_endp);
              if (_anch_endp_pos >= 0) {
                endp_count++;
                endp_group = Math.floor(_anch_endp_pos/4);
              }

            }

            if (endp_count==4) {
              if (!(anchor_key in tile_attach)) { tile_attach[anchor_key] = {}; }
              if (!(test_key in tile_attach[anchor_key])) {
                tile_attach[anchor_key][test_key] = { "anchor": anchor_key, "attach": test_key, "dv": [], "endpoint_group": [] };
              }
              tile_attach[anchor_key][test_key].dv.push( [dx, dy, dz ] );
              tile_attach[anchor_key][test_key].endpoint_group.push( endp_group );

              dv_key = dx.toString() + ":" + dy.toString() + ":" + dz.toString();
              uniq_repr[anchor_key].attach_dv[ dv_key ] = [ dx, dy, dz ];

              if (!(dv_key in uniq_repr[anchor_key].pos_tile_attach)) {
                uniq_repr[anchor_key].pos_tile_attach[dv_key] = {};
              }
              uniq_repr[anchor_key].pos_tile_attach[dv_key][test_key] = true;
            }

          }
        }
      }
    }

  }

  // easy access to endpoint group count
  //
  for (let key in uniq_repr) {
    let n_endpoint_group = 0;
    for (let _dvkey in uniq_repr[key].attach_dv) {
      n_endpoint_group++;
    }
    uniq_repr[key].n_endpoint_group = n_endpoint_group;
  }

  let admissible_pos = template.admissible_pos;
  let oppo  = template.oppo;

  // tile_attach only has actual connections
  // use it to fill out the admissible_attach
  // hash with *all* valid neighboring tile pairs
  //
  for (let key_anchor in uniq_repr) {

    if (!(key_anchor in admissible_nei)) {
      admissible_nei[key_anchor] = {};
    }

    for (let key_nei in uniq_repr) {

      if (!(key_nei in admissible_nei)) {
        admissible_nei[key_nei] = {};
      }

      for (let ii=0; ii<admissible_pos.length; ii++) {
        let dv_anch = admissible_pos[ii].dv_key;
        let dv_nei = oppo[dv_anch];

        if ((dv_anch in uniq_repr[key_anchor].pos_tile_attach) &&
            (dv_nei  in uniq_repr[key_nei].pos_tile_attach)) {
          if (key_nei in uniq_repr[key_anchor].pos_tile_attach[dv_anch]) {

            if (!(key_nei in admissible_nei[key_anchor])) {
              admissible_nei[key_anchor][key_nei] = {};
            }
            if (!(dv_anch in admissible_nei[key_anchor][key_nei])) {
              admissible_nei[key_anchor][key_nei][dv_anch] = {};
            }
            if (!(dv_anch in admissible_nei[key_anchor])) {
              admissible_nei[key_anchor][dv_anch] = {};
            }


            if (!(key_anchor in admissible_nei[key_nei])) {
              admissible_nei[key_nei][key_anchor] = {};
            }
            if (!(key_anchor in admissible_nei[key_nei][key_anchor])) {
              admissible_nei[key_nei][key_anchor][dv_nei] = {};
            }
            if (!(dv_nei in admissible_nei[key_nei])) {
              admissible_nei[key_nei][dv_nei] = {};
            }


            admissible_nei[key_anchor][key_nei][dv_anch] = {"conn": true};
            admissible_nei[key_nei][key_anchor][dv_nei] = {"conn": true};

            admissible_nei[key_anchor][dv_anch][key_nei] = {"conn": true};
            admissible_nei[key_nei][dv_nei][key_anchor] = {"conn": true};
          }
        }
        if ((!(dv_anch in uniq_repr[key_anchor].pos_tile_attach)) &&
            (!(dv_nei  in uniq_repr[key_nei].pos_tile_attach))) {

          if (!(key_nei in admissible_nei[key_anchor])) {
            admissible_nei[key_anchor][key_nei] = {};
          }
          if (!(dv_anch in admissible_nei[key_anchor][key_nei])) {
            admissible_nei[key_anchor][key_nei][dv_anch] = {};
          }
          if (!(dv_anch in admissible_nei[key_anchor])) {
            admissible_nei[key_anchor][dv_anch] = {};
          }

          if (!(key_anchor in admissible_nei[key_nei])) {
            admissible_nei[key_nei][key_anchor] = {};
          }
          if (!(key_anchor in admissible_nei[key_nei][key_anchor])) {
            admissible_nei[key_nei][key_anchor][dv_nei] = {};
          }
          if (!(dv_nei in admissible_nei[key_nei])) {
            admissible_nei[key_nei][dv_nei] = {};
          }

          admissible_nei[key_anchor][key_nei][dv_anch] = {"conn": false};
          admissible_nei[key_nei][key_anchor][dv_nei] = {"conn": false};

          admissible_nei[key_anchor][dv_anch][key_nei] = {"conn": false};
          admissible_nei[key_nei][dv_nei][key_anchor] = {"conn": false};
        }

      }

    }
  }

  template["raw_lib"] = raw_lib;
  template["equiv_map"] = _equiv_map;
  template["repr_map"] = _repr_map
  template["uniq_repr"] = uniq_repr;
  template["tile_attach"] = tile_attach;
  template["rot_lib"] = rot_lib;
  template["admissible_nei"] = admissible_nei;


  // restrict undesirable combinations
  //
  filter_steeple(template);


  //---
  //
  // build matrix
  //
  let _eps = (1/(1024*1024*1024));

  let tile_name_a = [];
  let tile_name_a_map_idx = {};
  for (let tn in uniq_repr) {
    tile_name_a_map_idx[tn] = tile_name_a.length;
    tile_name_a.push(tn);
  }

  let F_matrix = {};
  let _adj_info = {};

  template["svd"] = [];

  for (let pos_idx=0; pos_idx<admissible_pos.length; pos_idx++) {

    let dv_key = admissible_pos[pos_idx].dv_key;

    let _F = [];
    let _F_map = {};
    for (let ii=0; ii<tile_name_a.length; ii++) {
      let anchor_key = tile_name_a[ii];

      _F.push([]);
      _F_map[ tile_name_a[ii] ] = {};
      for (let jj=0; jj<tile_name_a.length; jj++) {
        let nei_key = tile_name_a[jj];

        let _val = ((nei_key in admissible_nei[ anchor_key ][ dv_key ]) ? 1 : 0);

        _F[ii].push( _val );
        _F_map[ anchor_key ][ nei_key ] = _val;
      }
    }

    F_matrix[ dv_key ] = _F;

    let _SVt = [];
    let _U = [];
    let S = [];


    if (typeof numeric !== "undefined") {

      let svd = numeric.svd( _F );

      let Sm = [];
      let Vt = numeric.transpose(svd.V);
      let nz_count = 0;

      template.svd.push(svd);

      for (let ii=0; ii<svd.S.length; ii++) {
        S.push([]);
        for (let jj=0; jj<svd.S.length; jj++) {
          S[ii].push( (ii==jj) ? svd.S[ii] : 0 );
        }
        if (Math.abs(svd.S[ii]) > _eps) { nz_count++; }
      }

      let SVt_all = numeric.dot(S, numeric.transpose(svd.V));

      //console.log("SVt_all", SVt_all);
      //console.log(">>>nz", nz_count);

      for (let ii=0; ii<nz_count; ii++) { _SVt.push(SVt_all[ii]); }

      //console.log( svd.U.length, svd.U[0].length, "::", svd.S.length, "::", svd.V.length, svd.V[0].length);
      //let SVt_all = numeric.dot(Sm, Vt);

      for (let ii=0; ii<svd.U.length; ii++) {
        _U.push([]);
        for (let jj=0; jj<nz_count; jj++) {
          _U[ii].push( svd.U[ii][jj] );
        }
      }

    }

    _adj_info[ dv_key ] = {
      "F": _F,
      "Fmap": _F_map,
      "S": S,
      "SV": _SVt,
      "U": _U
    };

  }

  //
  //---

  template["tile_name"] = tile_name_a;
  template["tile_name_idx"] = tile_name_a_map_idx;

  template["F"] = F_matrix;
  template["adj"] = _adj_info;

}

// I find stairs going up then immediately going down ("steeples") unsightly,
// so I filter them out of the rules to prevent them.
//
function filter_steeple(template) {
  let _eps = 1/(1024*1024);

  let admissible_nei  = template.admissible_nei;
  let admissible_pos  = template.admissible_pos;
  let oppo            = template.oppo;
  let raw_lib         = template.raw_lib;

  let delete_list = [];

  for (let key_anchor in admissible_nei) {
    if (key_anchor.charAt(0) != 'v') { continue; }

    let anc_p_repr = [0,0,0];
    let endp = raw_lib[key_anchor];
    for (let ii=0; ii<endp.length; ii++) {
      anc_p_repr[0] += endp[ii][0];
      anc_p_repr[1] += endp[ii][1];
      anc_p_repr[2] += endp[ii][2];
    }

    for (let posidx=0; posidx<admissible_pos.length; posidx++) {
      let dv_anc_key = admissible_pos[posidx].dv_key;
      let dv_nei_key = oppo[dv_anc_key];

      let dv_anc = admissible_pos[posidx].dv;
      let dv_nei = [ -dv_anc[0], -dv_anc[1], -dv_anc[2] ];

      for (let key_nei in admissible_nei[key_anchor][dv_anc_key]) {
        //if (key_nei.charAt(0) != '^') { continue; }
        if (key_nei.charAt(0) != 'v') { continue; }
        if (!admissible_nei[key_anchor][dv_anc_key][key_nei].conn) { continue; }

        let nei_p_repr = [0,0,0];
        let endp = raw_lib[key_nei];
        for (let ii=0; ii<endp.length; ii++) {
          nei_p_repr[0] += endp[ii][0];
          nei_p_repr[1] += endp[ii][1];
          nei_p_repr[2] += endp[ii][2];
        }

        let anc_v = [
          anc_p_repr[0] - 2*dv_anc[0],
          anc_p_repr[1] - 2*dv_anc[1],
          anc_p_repr[2] - 2*dv_anc[2]
        ];

        let nei_v = [
          nei_p_repr[0] - 2*dv_nei[0],
          nei_p_repr[1] - 2*dv_nei[1],
          nei_p_repr[2] - 2*dv_nei[2]
        ];

        let de = Math.abs(nei_v[0] - anc_v[0] + nei_v[1] - anc_v[1] + nei_v[2] - anc_v[2]);

        if (de < _eps) {
          delete_list.push( [ key_anchor, key_nei, dv_anc_key ] );
          delete_list.push( [ key_anchor, dv_anc_key, key_nei ] );
        }
      }

    }
  }
  for (let ii=0; ii<delete_list.length; ii++) {
    let a = delete_list[ii][0];
    let b = delete_list[ii][1];
    let c = delete_list[ii][2];

    delete admissible_nei[a][b][c];
  }

}

function _write_objs(template, odir) {
  let uniq_repr = template.uniq_repr;

  for (let tile_name in uniq_repr) {
    let tri = uniq_repr[tile_name].tri;
    let fn = odir + "/" + tile_name + ".obj";
    let fp = fs.createWriteStream(odir + "/" + tile_name + ".obj");
    jeom.obj_print(fp, tri);
    fp.end();
  }
}

function write_objs(obj_map, odir) {

  for (let tile_name in obj_map) {
    let tri = obj_map[tile_name].tri;
    let fn = odir + "/" + tile_name + ".obj";
    let fp = fs.createWriteStream(odir + "/" + tile_name + ".obj");
    jeom.obj_print(fp, tri);
    fp.end();
  }
}


function create_poms_config(template, opt) {
  opt = ((typeof opt === "undefined") ? {} : opt);

  let idir_a = [
    "1:0:0", "-1:0:0",
    "0:1:0", "0:-1:0",
    "0:0:1", "0:0:-1"
  ];

  let idir_map = {
    "1:0:0": 0, "-1:0:0": 1,
    "0:1:0": 2, "0:-1:0": 3,
    "0:0:1": 4, "0:0:-1": 5,
  };


  let poms_cfg = {
    "rule": [],
    "name": [],
    "constraint": [],
    "size" : [8,8,8],
    "boundaryCondition" : {
      "x+": { "type": "tile" , "value": 0 },
      "x-": { "type": "tile" , "value": 0 },
      "y+": { "type": "tile" , "value": 0 },
      "y-": { "type": "tile" , "value": 0 },
      "z+": { "type": "tile" , "value": 0 },
      "z-": { "type": "tile" , "value": 0 }
    },
    "objMap" : []

  };


  poms_cfg.name = template.tile_name;
  let n_tile = poms_cfg.name.length;

  let F = template.F;
  for (let idir=0; idir<6; idir++) {
    dkey = idir_a[idir];
    for (let ii=0; ii<n_tile; ii++) {
      for (let jj=0; jj<n_tile; jj++) {
        if (F[dkey][ii][jj] > 0) {
          poms_cfg.rule.push( [ ii, jj, idir, F[dkey][ii][jj] ] );
        }
      }
    }
  }

  if (typeof opt.objDir !== "undefined") {
    for (let ii=0; ii<poms_cfg.name.length; ii++) {
      poms_cfg.objMap.push( opt.objDir + "/" + poms_cfg.name[ii] + ".obj" );
    }
  }

  return poms_cfg;
}

function write_poms_json(poms_json, out_fn) {

  let lines = [];
  let multi_field = [];
  let multi_field_n = 8;

  lines.push("{");

  if ("comment" in poms_json) {
    lines.push("\"comment\":" + JSON.stringify(poms_json.comment) + ",");
  }

  lines.push("\"rule\":[");
  for (let ii=0; ii<poms_json.rule.length; ii++) {
    let sfx = ( (ii==(poms_json.rule.length-1)) ? "" : "," );

    if (((ii%multi_field_n)==0) &&
        (multi_field.length > 0)) {
      lines.push( multi_field.join(" ") );
      multi_field = [];
    }

    //lines.push( JSON.stringify(poms_json.rule[ii]) + sfx );
    multi_field.push( JSON.stringify(poms_json.rule[ii]) + sfx );
  }
  if (multi_field.length > 0) {
    lines.push( multi_field.join(" ") );
  }
  lines.push("],");

  //---

  multi_field = [];
  lines.push("\"name\":[");
  for (let ii=0; ii<poms_json.name.length; ii++) {
    let sfx = ( (ii==(poms_json.name.length-1)) ? "" : "," );

    if (((ii%multi_field_n)==0) &&
        (multi_field.length > 0)) {
      lines.push( multi_field.join(" ") );
      multi_field = [];
    }

    multi_field.push( JSON.stringify(poms_json.name[ii]) + sfx );
  }
  if (multi_field.length > 0) {
    lines.push( multi_field.join(" ") );
  }
  lines.push("],");

  //---

  multi_field = [];
  lines.push("\"weight\":[");
  for (let ii=0; ii<poms_json.name.length; ii++) {
    let sfx = ( (ii==(poms_json.name.length-1)) ? "" : "," );

    if (((ii%multi_field_n)==0) &&
        (multi_field.length > 0)) {
      lines.push( multi_field.join(" ") );
      multi_field = [];
    }

    multi_field.push( "1" + sfx );
  }
  if (multi_field.length > 0) {
    lines.push( multi_field.join(" ") );
  }
  lines.push("],");

  //---

  multi_field = [];

  lines.push("\"boundaryCondition\":" + JSON.stringify(poms_json.boundaryCondition) + ",");
  lines.push("\"constraint\":" + JSON.stringify(poms_json.constraint) + ",");

  multi_field_n = 4;

  if (typeof poms_json.objMap !== "undefined") {
    lines.push("\"objMap\":[");
    for (let ii=0; ii<poms_json.objMap.length; ii++) {
      let sfx = ( (ii==(poms_json.objMap.length-1)) ? "" : "," );

      if (((ii%multi_field_n)==0) &&
          (multi_field.length > 0)) {
        lines.push( multi_field.join(" ") );
        multi_field = [];
      }

      //lines.push( JSON.stringify(poms_json.objMap[ii]) + sfx );
      multi_field.push( JSON.stringify(poms_json.objMap[ii]) + sfx );
    }
    if (multi_field.length > 0) {
      lines.push( multi_field.join(" ") );
    }
    lines.push("],");

    multi_field = [];
  }

  lines.push("\"size\":" + JSON.stringify(poms_json.size) );
  lines.push("}");

  fs.writeFileSync( out_fn, lines.join("\n") );
}



init_template(_template);
build_tile_library(_template);

var OUT_DIR = "ramps_roads";
var OBJ_DIR = "ramps_roads_obj";

var _opt = { "objDir": OBJ_DIR };


var poms_json = create_poms_config(_template, _opt);

poms_json["comment"] = "Tiles sit in the XY plane with the Z direction in the 'upwards' direction.";
poms_json["comment"] += "\nStairs ('v') connect the front face of the XZ plane to the top of the XY plane";
poms_json["comment"] += " so that it requires two stairs to connect one level to another.";

write_poms_json( poms_json, "ramps_roads_poms.json" );

fs.mkdirSync( OBJ_DIR, {"recursive":true} );
write_objs(_template.uniq_repr, OBJ_DIR);

