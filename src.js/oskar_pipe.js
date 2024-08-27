// LICENSE: cc0
//
// To the extent possible under law, the person who associated CC0 with
// this code has waived all copyright and related or neighboring rights
// to this code.
//
// You should have received a copy of the CC0 legalcode along with this
// work.  If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//

var jeom = require("./jeom.js");
var fs = require("fs");
var m4 = require("./m4.js").m4;

const TILE_WIDTH = 1/4;
const TILE_HEIGHT = 1/4;

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

  "weight": {
    ".": 1,

    "s": 1,
    "p": 1,
    "b": 1
  },

  "pdf":  {
    ".": -1,

    "s": 1,
    "p": 1,
    "b": 1

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

  "." : [],
  "s" : [],
  "p" : [],
  "b" : []

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

    // start
    //
    "s": [
      [ -_g_w/2,  1/2, -_g_h/2 ], [ _g_w/2,  1/2, -_g_h/2 ],
      [ -_g_w/2,  1/2, +_g_h/2 ], [ _g_w/2,  1/2, +_g_h/2 ]
    ],

    // pipe
    //
    "p": [
      [ -_g_w/2, -1/2, -_g_h/2 ], [ _g_w/2, -1/2, -_g_h/2 ],
      [ -_g_w/2, -1/2, +_g_h/2 ], [ _g_w/2, -1/2, +_g_h/2 ],

      [ -_g_w/2,  1/2, -_g_h/2 ], [  _g_w/2,  1/2, -_g_h/2 ],
      [ -_g_w/2,  1/2, +_g_h/2 ], [  _g_w/2,  1/2, +_g_h/2 ]
    ],

    // bend
    //
    "b": [
      [ -_g_w/2, -1/2, -_g_h/2 ], [  _g_w/2, -1/2, -_g_h/2 ],
      [ -_g_w/2, -1/2, +_g_h/2 ], [  _g_w/2, -1/2, +_g_h/2 ],

      [  1/2,  _g_w/2, -_g_h/2 ], [  1/2, -_g_w/2, -_g_h/2 ],
      [  1/2,  _g_w/2, +_g_h/2 ], [  1/2, -_g_w/2, +_g_h/2 ]
    ]

  };

  template["endpoint_order"] = [ ".", "s", "p", "b" ];

  // 's' (start tile) base height
  //
  let _sbh = 1/8;
  let _sbp = -0.5 + _sbh;
  let _sb_ws = _g_w;
  let _sb_hs = _g_h;

  let _sb_wb = _g_w + (1-_g_w)/4;
  let _sb_hb = _g_h + (1-_g_h)/4;

  // geometry
  //
  template["s"] = [

    // skinny portion

    // front panel
    //
    -_g_w/2,  1/2, -_g_h/2,  _g_w/2,    1/2, -_g_h/2,   -_g_w/2, _sbp, -_g_h/2,
     _g_w/2,  1/2, -_g_h/2,  _g_w/2, _sbp, -_g_h/2,   -_g_w/2, _sbp, -_g_h/2,

    // back panel
    //
    -_g_w/2,  1/2, +_g_h/2, -_g_w/2, _sbp, +_g_h/2,   _g_w/2,    1/2, +_g_h/2,
     _g_w/2,  1/2, +_g_h/2, -_g_w/2, _sbp, +_g_h/2,   _g_w/2, _sbp, +_g_h/2,

    // left side stripe
    //
    -_g_w/2,  1/2, -_g_h/2,   -_g_w/2, _sbp, -_g_h/2,  -_g_w/2, _sbp, +_g_h/2,
    -_g_w/2,  1/2, -_g_h/2,   -_g_w/2, _sbp, +_g_h/2,  -_g_w/2,    1/2, +_g_h/2,


    // right side stripe
    //
     _g_w/2,  1/2, -_g_h/2,   _g_w/2, _sbp, +_g_h/2,  _g_w/2, _sbp, -_g_h/2,
     _g_w/2,  1/2, -_g_h/2,   _g_w/2,    1/2, +_g_h/2,  _g_w/2, _sbp, +_g_h/2,

    // back cap (optional)
    //
    -_g_w/2,  1/2, -_g_h/2,   -_g_w/2,  1/2, +_g_h/2,   _g_w/2,  1/2, -_g_h/2,
     _g_w/2,  1/2, -_g_h/2,   -_g_w/2,  1/2, +_g_h/2,   _g_w/2,  1/2, +_g_h/2,

    // front cap (optional)
    //
    -_g_w/2, _sbp, -_g_h/2,   _g_w/2, _sbp, +_g_h/2,    -_g_w/2, _sbp, +_g_h/2,
     _g_w/2, _sbp, -_g_h/2,   _g_w/2, _sbp, +_g_h/2,    -_g_w/2, _sbp, -_g_h/2,

    //---

    // fat base

    // front panel
    //
    -_sb_wb/2,  _sbp, -_sb_hb/2,  _sb_wb/2,  _sbp, -_sb_hb/2,   -_sb_wb/2, -1/2, -_sb_hb/2,
     _sb_wb/2,  _sbp, -_sb_hb/2,  _sb_wb/2,  -1/2, -_sb_hb/2,   -_sb_wb/2, -1/2, -_sb_hb/2,

    // back panel
    //
    -_sb_wb/2,  _sbp, +_sb_hb/2, -_sb_wb/2, -1/2, +_sb_hb/2,   _sb_wb/2,  _sbp, +_sb_hb/2,
     _sb_wb/2,  _sbp, +_sb_hb/2, -_sb_wb/2, -1/2, +_sb_hb/2,   _sb_wb/2,  -1/2, +_sb_hb/2,

    // left side stripe
    //
    -_sb_wb/2,  _sbp, -_sb_hb/2,   -_sb_wb/2, -1/2, -_sb_hb/2,  -_sb_wb/2,  -1/2, +_sb_hb/2,
    -_sb_wb/2,  _sbp, -_sb_hb/2,   -_sb_wb/2, -1/2, +_sb_hb/2,  -_sb_wb/2,  _sbp, +_sb_hb/2,


    // right side stripe
    //
     _sb_wb/2,  _sbp, -_sb_hb/2,   _sb_wb/2,  -1/2, +_sb_hb/2,  _sb_wb/2, -1/2, -_sb_hb/2,
     _sb_wb/2,  _sbp, -_sb_hb/2,   _sb_wb/2,  _sbp, +_sb_hb/2,  _sb_wb/2, -1/2, +_sb_hb/2,

    // back cap (optional)
    //
    -_sb_wb/2,  _sbp, -_sb_hb/2,   -_sb_wb/2,  _sbp, +_sb_hb/2,   _sb_wb/2,  _sbp, -_sb_hb/2,
     _sb_wb/2,  _sbp, -_sb_hb/2,   -_sb_wb/2,  _sbp, +_sb_hb/2,   _sb_wb/2,  _sbp, +_sb_hb/2,

    // front cap (optional)
    //
    -_sb_wb/2, -1/2, -_sb_hb/2,   _sb_wb/2, -1/2, +_sb_hb/2,    -_sb_wb/2, -1/2, +_sb_hb/2,
     _sb_wb/2, -1/2, -_sb_hb/2,   _sb_wb/2, -1/2, +_sb_hb/2,    -_sb_wb/2, -1/2, -_sb_hb/2

  ];

  // geometry
  //
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


  // 'b'
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

  template["b"] = flat_fr;

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

    for (let xidx=0; xidx<4; xidx++) {
      for (let yidx=0; yidx<4; yidx++) {
        for (let zidx=0; zidx<4; zidx++) {

          // take out if you want all orientations
          //
          if ((pkey == 's') &&
              ((xidx>0) ||
               (yidx>0) ||
               (zidx>0))) {
            continue;
          }

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

          rot_lib[ukey] = { "m": [mx, my, mz], "r": [xidx, yidx, zidx ], "M": mr  };

        }
      }

    }


    // once in the library, go through and compare each
    // to see if they're equivalent by seeing if the endpoints
    // line up
    //  
    for (let i=0; i<_type_a.length; i++) {
      for (let j=i+1; j<_type_a.length; j++) {

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

    // since only one s tile was created, without rotations,
    // we need to add it explicitly here.
    //
    if (!("s000" in _equiv_map)) {
      _equiv_map[ "s000" ] = [ "s000" ];
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
      uniq_repr[repr_id] = {"count":0, "attach_dv": {}, "pos_tile_attach":{}, "tri": [] };
    }
    uniq_repr[repr_id].count++;
  }

  let tile_attach = {};

  // now for each representative, test every other tile to see if can
  // be joined
  //
  for (let anchor_key in uniq_repr) {

    // store triangles for this representation
    //
    let _tp_tri = template[ anchor_key[0] ];
    let _rx = rot_lib[ anchor_key].r[0] * Math.PI * 0.5;
    let _ry = rot_lib[ anchor_key].r[1] * Math.PI * 0.5;
    let _rz = rot_lib[ anchor_key].r[2] * Math.PI * 0.5;
    uniq_repr[anchor_key].tri = _template_rot_mov( _tp_tri, _rx, _ry, _rz );

    // ignore 'empty' and debug tiles
    //
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

function write_objs(template, odir) {
  let uniq_repr = template.uniq_repr;

  for (let tile_name in uniq_repr) {
    let tri = uniq_repr[tile_name].tri;
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
    "weight": [],
    "constraint": [],
    "size" : [8,8,8],
    "quiltSize" : [8,8,8],
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

  for (let ii=0; ii<poms_cfg.name.length; ii++) {
    poms_cfg.weight.push(1);
  }

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

    //multi_field.push( "1" + sfx );
    multi_field.push( poms_json.weight[ii].toString() + sfx );
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

  lines.push("\"quiltSize\":" + JSON.stringify(poms_json.quiltSize) + "," );
  lines.push("\"size\":" + JSON.stringify(poms_json.size) );
  lines.push("}");

  fs.writeFileSync( out_fn, lines.join("\n") );
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

function poms_debugPrint(poms) {

  let name = poms.name;
  let rule = poms.rule;

  let dir_s = [ "+x", "-x", "+y", "-y", "+z", "-z" ];

  for (let ii=0; ii<rule.length; ii++) {
    let src_name = name[ rule[ii][0] ];
    let dst_name = name[ rule[ii][1] ];
    let d = dir_s[ rule[ii][2] ];
    console.log( src_name, "--(" + d + ")-->", dst_name );
  }




}


init_template(_template);
build_tile_library(_template);

var OUT_FN = "oskar_pipe_poms.json";
var OUT_DIR = "oskar_pipe";
var OBJ_DIR = "oskar_pipe_obj";

var _opt = { "objDir": OBJ_DIR };

fs.mkdirSync( OBJ_DIR, {"recursive":true} );


var poms_json = create_poms_config( _template, _opt );

poms_json["comment"] = "Oskar Pipes based off of https://twitter.com/OskSta/status/793806535898136576.";
poms_json["comment"] += "\nPipes shoot out perpendicular the XZ plane, with Y- (y=0) being the start and Y+ going up.";
poms_json["comment"] += " '.' is blank, 's' is start, 'p' is straight pipe, 'b' is bend.";
poms_json["comment"] += "\nThe numbers after the tile code represent 90 (pi/2) rotations around X, Y, Z respectively.";
poms_json["comment"] += "\nFor example, 'b012' reprsents no rotation about X, one pi/2 rotation around Y and a pi rotation about Z.";
poms_json["comment"] += "\nConstraints have been added to only allow blank ('.') and start tiles/pipes ('s') on the y=0 plane";
poms_json["comment"] += " with the start tile removed from the rest of the grid, forcing a start tile to be on the y=0 plane.";

poms_json.constraint.push({"type":"remove", "range": { "tile": [1,2], "x":[], "y":[1], "z":[] } });
poms_json.constraint.push({"type":"remove", "range": { "tile": [2], "x":[], "y":[0,1], "z":[] } });

//poms_debugPrint(poms_json);

write_poms_json( poms_json, OUT_FN );
write_objs( _template.uniq_repr, OBJ_DIR );

