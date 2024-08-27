// LICENSE: CC0
//
// Note this has only been really tested for 2x2 windows.
// The boundary conditions might be a little screwed
// up for other types of windows, so this needs to be revisited.
//

// reference:
//
// https://paulbourke.net/dataformats/obj/

var fs = require("fs");
var jeom = require("./jeom.js");

var _EPS = (1/64);

var g_ctx = {
  "in_dir": "../data/kenney_marble/obj_fmt/",
  "fns": [],

  "obj_dir": "./marble_obj_out/",

  "weight_block_flag" : true,

  "orig_obj":[],
  "obj": [],
  "tri": [],

  "uniq_pnt": [],

  "name_index" : {},

  "bbox": [],

  "conn_point_count": 10,

  "dir_code" : ["x+", "x-", "y+", "y-", "z+", "z-" ],
  "connect": [ [],[], [],[], [],[] ],

  "cell_info": {
    "x": -0.5, "X": 0.5, "dx": 1,
    "y":    0, "Y":   1, "dy": 1/8,
    "z": -0.5, "Z": 0.5, "dz": 1
  },

  "output_obj": true,
  "obj_rescale_v": [2,2,2],

  "cp": []
};

// debug
//g_ctx.in_dir = "./.tmp/marble/Models/obj/";
//g_ctx.in_dir = "./marble_obj/";
read_objs(g_ctx);

//-----
//-----
//-----
//-----

function pnt_diff(a_pnt, b_pnt) {

  let dup_a = jeom.dup(a_pnt);
  let dup_b = jeom.dup(b_pnt);

  dup_a.sort( vec_cmp );
  dup_b.sort( vec_cmp );

  let _count = Math.abs( dup_a.length - dup_b.length );

  for (let ii=0; (ii<dup_a.length) && (ii<dup_b.length); ii++) {
    if ((Math.abs(dup_a[ii][0] - dup_b[ii][0]) > _EPS) ||
        (Math.abs(dup_a[ii][1] - dup_b[ii][1]) > _EPS) ||
        (Math.abs(dup_a[ii][2] - dup_b[ii][2]) > _EPS)) {
      _count++;
    }
        
  }

  return _count;
}

function _min(a,b) { return (a<b) ? a : b ; }
function _max(a,b) { return (a>b) ? a : b ; }

function _in_bbox_face(p, bbox, _eps) {
  _eps = ((typeof _eps === "undefined") ? _EPS : _eps);

  let _count = 0;
  if (((p[0]-bbox.x) >  _eps) &&
      ((p[0]-bbox.X) < -_eps)) { _count++; }

  if (((p[1]-bbox.y) >  _eps) &&
      ((p[1]-bbox.Y) < -_eps)) { _count++; }

  if (((p[2]-bbox.z) >  _eps) &&
      ((p[2]-bbox.Z) < -_eps)) { _count++; }

  return _count;
}

function _in_bbox(p,bbox, _eps) {
  _eps = ((typeof _eps === "undefined") ? _EPS : _eps);

  /*
  if ( ((p[0]-bbox.x) <  _eps) ||
       ((p[0]-bbox.X) > -_eps) ||
       ((p[1]-bbox.y) <  _eps) ||
       ((p[1]-bbox.Y) > -_eps) ||
       ((p[2]-bbox.z) <  _eps) ||
       ((p[2]-bbox.Z) > -_eps)) { return false; }
  return true;
  */

  if ((p[0] < bbox.x) || (p[0] > bbox.X) ||
      (p[1] < bbox.y) || (p[1] > bbox.Y) ||
      (p[2] < bbox.z) || (p[2] > bbox.Z)) {
    return false;
  }
  return true;
}

function cp_ms(ctx) {
  ctx = ((typeof ctx === "undefined") ? g_ctx : ctx);
  let d = new Date();
  ctx.cp.push( d.getTime()/1000 );
}

function _timems() {
  let d = new Date();
  return d.getTime() / 1000;
}

function stlPrint(ctx) {
  let tri_a = ctx.tri, _tri = [];
  let dx = 1.5, dy = 1.5, dz = 1.5;
  for (let i=0; i<tri_a.length; i++) {
    let x = dx*(i%8);
    let y = dy*Math.floor(i/8);
    let z = 0.0;
    let tri_dup = jeom.dup(tri_a[i]);
    jeom.mov( tri_dup, [x,y,z] );
    for (let ii=0; ii<tri_dup.length; ii++) {
      _tri.push(tri_dup[ii]);
    }
  }
  jeom.stl_print( process.stdout, _tri );
}

function vec_cmp(a,b) {
  if (a[0] < b[0]) { return -1; }
  if (a[0] > b[0]) { return  1; }
  if (a[1] < b[1]) { return -1; }
  if (a[1] > b[1]) { return  1; }
  if (a[2] < b[2]) { return -1; }
  if (a[2] > b[2]) { return  1; }
  return 0;
}

function dedup_point_list(dst_pnt, src_pnt) {
  let _eps = (1/256);
  let C = Math.floor(1/_eps);
  let _pnt = [];
  for (let i=0; i<src_pnt.length; i+=3) {

    //let ix = Math.floor(C*src_pnt[i+0]);
    //let iy = Math.floor(C*src_pnt[i+1]);
    //let iz = Math.floor(C*src_pnt[i+2]);
    //_pnt.push( [ ix, iy, iz, i ] );

    _pnt.push( [ src_pnt[i+0], src_pnt[i+1], src_pnt[i+2] ] );
  }

  _pnt.sort( vec_cmp );

  dst_pnt.push( _pnt[0][0], _pnt[0][1], _pnt[0][2] );
  for (let i=1; i<_pnt.length; i++) {
    if (vec_eq( _pnt[i-1], _pnt[i] )) { continue; }
    dst_pnt.push( _pnt[i][0], _pnt[i][1], _pnt[i][2] );
  }

  return dst_pnt;
}

function clamp_sort_point_list(dst_pnt, src_pnt) {
  let _eps = (1/256);
  let C = Math.floor(1/_eps);
  let _pnt = [];
  for (let i=0; i<src_pnt.length; i+=3) {
    let ix = Math.floor(C*src_pnt[i+0]);
    let iy = Math.floor(C*src_pnt[i+1]);
    let iz = Math.floor(C*src_pnt[i+2]);
    _pnt.push( [ ix, iy, iz, i ] );
  }

  _pnt.sort( vec_cmp );

  dst_pnt.push( _pnt[0][0]/C, _pnt[0][1]/C, _pnt[0][2]/C );
  for (let i=1; i<_pnt.length; i++) {
    if (vec_eq( _pnt[i-1], _pnt[i] )) { continue; }
    dst_pnt.push( _pnt[i][0]/C, _pnt[i][1]/C, _pnt[i][2]/C );
  }

  return dst_pnt;
}

function read_objs(ctx) {
  let files = fs.readdirSync( ctx.in_dir );
  for (let i=0; i<files.length; i++) {
    let fn = files[i];
    if (!fn.match( /\.obj$/ )) { continue; }
    ctx.fns.push(fn);

    let sdat = fs.readFileSync( ctx.in_dir + "/" + fn);
    let _obj = jeom.obj_split_loads( sdat.toString() );
    let flat_obj = jeom.obj2flat(_obj);
    let tri = jeom.obj2tri( _obj );

    ctx.orig_obj.push(_obj);
    ctx.obj.push(flat_obj);
    ctx.tri.push(tri);

    let pnt_list = [];
    dedup_point_list(pnt_list, tri);
    ctx.uniq_pnt.push(pnt_list);

    let name = fn.split(".").slice(0,-1);
    ctx.name_index[ name ] = ctx.fns.length-1;
  }
  return ctx;
}

function _find_bbox_tri(_tri) {
  let bbox = { "x":0, "y":0, "z":0, "X":0, "Y":0, "Z": 0};
  let first = true;

  for (let ii=0; ii<_tri.length; ii+=3) {
    if (first) {
      bbox.x = _tri[ii+0]; bbox.X = _tri[ii+0];
      bbox.y = _tri[ii+1]; bbox.Y = _tri[ii+1];
      bbox.z = _tri[ii+2]; bbox.Z = _tri[ii+2];
    }
    first = false;

    if (_tri[ii+0] < bbox.x) { bbox.x = _tri[ii+0]; }
    if (_tri[ii+1] < bbox.y) { bbox.y = _tri[ii+1]; }
    if (_tri[ii+2] < bbox.z) { bbox.z = _tri[ii+2]; }

    if (_tri[ii+0] > bbox.X) { bbox.X = _tri[ii+0]; }
    if (_tri[ii+1] > bbox.Y) { bbox.Y = _tri[ii+1]; }
    if (_tri[ii+2] > bbox.Z) { bbox.Z = _tri[ii+2]; }
  }

  return bbox;
}

function _find_bbox(_obj_model) {
  let bbox = { "x":0, "y":0, "z":0, "X":0, "Y":0, "Z": 0};
  let first = true;

  let group_list = _obj_model.g;
  for (let g_idx=0; g_idx<group_list.length; g_idx++) {
    let face_list = group_list[g_idx].f;
    for (let f_idx=0; f_idx<face_list.length; f_idx++) {
      let n = face_list[f_idx].length;
      for (let pnt_idx=0; pnt_idx<n; pnt_idx++) {
        let x = face_list[f_idx][pnt_idx].x;
        let y = face_list[f_idx][pnt_idx].y;
        let z = face_list[f_idx][pnt_idx].z;
        if (first) {
          bbox.x = x; bbox.X = x;
          bbox.y = y; bbox.Y = y;
          bbox.z = z; bbox.Z = z;
        }
        first = false;

        if (x < bbox.x) { bbox.x = x; }
        if (y < bbox.y) { bbox.y = y; }
        if (z < bbox.z) { bbox.z = z; }

        if (x > bbox.X) { bbox.X = x; }
        if (y > bbox.Y) { bbox.Y = y; }
        if (z > bbox.Z) { bbox.Z = z; }

      }
    }
  }

  return bbox;
}

function vec_eq(a,b, _eps) {
  _eps = ((typeof _eps !== "undefined") ? _eps : _EPS );
  if ((Math.abs(a[0]-b[0]) < _eps) &&
      (Math.abs(a[1]-b[1]) < _eps) &&
      (Math.abs(a[2]-b[2]) < _eps)) { return true; }
  return false;
}

/*
function pvec_match( pnt_a, pnt_b ) {
  let m_count = 0;
  let u = [0,0,0], v=[0,0,0];
  for (let ii=0; ii<pnt_a.length; ii+=3) {
    for (let jj=0; jj<pnt_b.length; jj+=3) {
      u[0] = pnt_a[ii+0];
      u[1] = pnt_a[ii+1];
      u[2] = pnt_a[ii+2];

      v[0] = pnt_b[jj+0];
      v[1] = pnt_b[jj+1];
      v[2] = pnt_b[jj+2];

      if (vec_eq(u,v)) { m_count++; }
    }
  }
  return m_count;
}
*/

/*
function conn_match(conn_a, conn_b, idir_a, idir_b) {
  let a_idx=0, b_idx=0;
  let m_count=0;

  let u = [0,0,0], v=[0,0,0];

  for (let ii=0; ii<conn_a[idir_a].length; ii+=3) {
    for (let jj=0; jj<conn_b[idir_b].length; jj+=3) {
      u[0] = conn_a[idir_a][ii+0];
      u[1] = conn_a[idir_a][ii+1];
      u[2] = conn_a[idir_a][ii+2];

      v[0] = conn_b[idir_b][jj+0];
      v[1] = conn_b[idir_b][jj+1];
      v[2] = conn_b[idir_b][jj+2];

      if (vec_eq(u,v)) { m_count++; }
    }
  }

  return m_count;

  while ((a_idx < conn_a[idir].length) &&
         (b_idx < conn_b[idir].length)) {

    if (vec_eq(conn_a[idir][a_idx], conn_b[idir][b_idx])) {
      m_count++;
    }
    a_idx++;
    b_idx++;
  }

  return m_count;
}
*/

/*
function _fill_connect(_tri) {
  let _eps = (1/4096);
  let bbox = _find_bbox_tri(_tri);

  let conn = [ [],[], [],[], [],[] ];

  let check_plane = {
    "x+": [  0.5 ],
    "x-": [ -0.5 ],

    "y+": [ 1/8 ],
    "y-": [   0 ],

    "z+": [ 0.5 ],
    "z-": [-0.5 ]
  };

  let cp = check_plane;

  for (let ii=0; ii<_tri.length; ii+=3) {
    let x = _tri[ii+0];
    let y = _tri[ii+1];
    let z = _tri[ii+2];

    if (Math.abs(cp["x+"]-x) < _eps) { conn[0].push( [x,y,z] ); }
    if (Math.abs(cp["x-"]-x) < _eps) { conn[1].push( [x,y,z] ); }
    if (Math.abs(cp["y+"]-y) < _eps) { conn[2].push( [x,y,z] ); }
    if (Math.abs(cp["y-"]-y) < _eps) { conn[3].push( [x,y,z] ); }
    if (Math.abs(cp["z+"]-z) < _eps) { conn[4].push( [x,y,z] ); }
    if (Math.abs(cp["z-"]-z) < _eps) { conn[5].push( [x,y,z] ); }
  }

  let ddc = [ [],[], [],[], [],[] ];


  for (let idir=0; idir<6; idir++) {
    if (conn[idir].length == 0) { continue; }

    conn[idir].sort( vec_cmp );

    ddc[idir].push( conn[idir][0][0], conn[idir][0][1], conn[idir][0][2] );
    for (let ii=1; ii<conn[idir].length; ii++) {
      if (vec_eq( conn[idir][ii-1], conn[idir][ii] )) {

        //console.log("#", idir, conn[idir][ii-1], conn[idir][ii], "equal, skipping");

        continue;
      }

      //console.log("#", idir, conn[idir][ii-1], conn[idir][ii], "adding");

      ddc[idir].push( conn[idir][ii][0], conn[idir][ii][1], conn[idir][ii][2] );
    }

  }

  return ddc;


  for (let idir=0; idir<6; idir++) {
    if (ddc[idir].length == 0) { continue; }
    for (let ii=0; ii<ddc[idir].length; ii++) {
      console.log( ddc[idir][ii][0],ddc[idir][ii][1],ddc[idir][ii][2] );
    }
    console.log( ddc[idir][0][0],ddc[idir][0][1],ddc[idir][0][2] );
    console.log("\n\n");
  }

  return cp;

  for (let idir=0; idir<6; idir++) {
    if (conn[idir].length == 0) { continue; }
    if (conn[idir].length == 0) { continue; }
    for (let ii=0; ii<conn[idir].length; ii++) {
      console.log( conn[idir][ii][0],conn[idir][ii][1],conn[idir][ii][2] );
    }
    console.log( conn[idir][0][0],conn[idir][0][1],conn[idir][0][2] );
    console.log("\n\n");
  }

  return cp;
}
*/

function debug_print_obj_model(_obj_model) {

  let group_list = _obj_model.g;
  for (let g_idx=0; g_idx<group_list.length; g_idx++) {
    let face_list = group_list[g_idx].f;
    for (let f_idx=0; f_idx<face_list.length; f_idx++) {
      let n = face_list[f_idx].length;
      for (let pnt_idx=0; pnt_idx<=n; pnt_idx++) {
        console.log( face_list[f_idx][pnt_idx%n].x, face_list[f_idx][pnt_idx%n].y, face_list[f_idx][pnt_idx%n].z );
      }
      console.log("\n");
    }
  }

}

function debug_print_tri(tri) {
  for (let ii=0; ii<tri.length; ii+=9) {
    console.log(tri[ii+0], tri[ii+1], tri[ii+2]);
    console.log(tri[ii+3], tri[ii+4], tri[ii+5]);
    console.log(tri[ii+6], tri[ii+7], tri[ii+8]);
    console.log(tri[ii+0], tri[ii+1], tri[ii+2]);
    console.log("\n");
  } 
}

function debug_print_pnt(pnt) {
  for (let ii=0; ii<pnt.length; ii+=3) {
    console.log(pnt[ii+0], pnt[ii+1], pnt[ii+2]);
  }
  console.log("\n");
}

//---


// Create a dictionary with key entries for
// codes of which 1x1x1 blocks points occupy.
//
// This is pretty hacky as it only considers
// points and only points that are _eps away
// from 1x1x1 lattice walls.
//
function occupancy_block(pnt_a) {
  let _eps = _EPS;

  let dxyz = [ 0.5, 0.5, 0.5 ];

  let dblock = {};
  for (let ii=0; ii<pnt_a.length; ii+=3) {

    let _x = pnt_a[ii+0] + dxyz[0];
    let _y = pnt_a[ii+1] + dxyz[1];
    let _z = pnt_a[ii+2] + dxyz[2];

    //let qx = Math.round(pnt_a[ii+0]);
    //let qy = Math.round(pnt_a[ii+1]);
    //let qz = Math.round(pnt_a[ii+2]);

    let qx = Math.round(_x);
    let qy = Math.round(_y);
    let qz = Math.round(_z);

    if (Math.abs( qx - _x ) < _eps) { continue; }
    if (Math.abs( qy - _y ) < _eps) { continue; }
    if (Math.abs( qz - _z ) < _eps) { continue; }

    //let fx = Math.floor(pnt_a[ii+0]);
    //let fy = Math.floor(pnt_a[ii+1]);
    //let fz = Math.floor(pnt_a[ii+2]);

    let fx = Math.floor(_x + _eps);
    let fy = Math.floor(_y + _eps);
    let fz = Math.floor(_z + _eps);

    let key = fx.toString() + ":" + fy.toString() + ":" + fz.toString();
    if (!(key in dblock)) { dblock[key] = 0; }
    dblock[key]++;
  }

  return dblock;
}

//
//
function occupancy_block_bound(pnt_a) {

  //let _eps = (1/64);
  let _eps = _EPS;

  let first = true;
  let block_bbox = {
    "x": 0, "X":0,
    "y": 0, "Y":0,
    "z": 0, "Z":0,
  };

  for (let ii=0; ii<pnt_a.length; ii+=3) {

    let _x = pnt_a[ii+0];
    let _y = pnt_a[ii+1];
    let _z = pnt_a[ii+2];

    let qx = Math.round(pnt_a[ii+0]);
    let qy = Math.round(pnt_a[ii+1]);
    let qz = Math.round(pnt_a[ii+2]);

    if (Math.abs( qx - _x ) < _eps) { continue; }
    if (Math.abs( qy - _y ) < _eps) { continue; }
    if (Math.abs( qz - _z ) < _eps) { continue; }

    let fx = Math.floor(pnt_a[ii+0]);
    let fy = Math.floor(pnt_a[ii+1]);
    let fz = Math.floor(pnt_a[ii+2]);

    if (first) {
      block_bbox.x = fx; block_bbox.X = fx;
      block_bbox.y = fy; block_bbox.Y = fy;
      block_bbox.z = fz; block_bbox.Z = fz;
      first = false;
    }
    if (fx < block_bbox.x) { block_bbox.x = fx; }
    if (fx > block_bbox.X) { block_bbox.X = fx; }

    if (fy < block_bbox.y) { block_bbox.y = fy; }
    if (fy > block_bbox.Y) { block_bbox.Y = fy; }

    if (fz < block_bbox.z) { block_bbox.z = fz; }
    if (fz > block_bbox.Z) { block_bbox.Z = fz; }

    //let key = fx.toString() + ":" + fy.toString() + ":" + fz.toString();
    //if (!(key in dblock)) { dblock[key] = 0; }
    //dblock[key]++;
  }

  return block_bbox;
}

// return number of cells in common (1x1x1 cell).
// Hacky, see above occupancy_block function.
//
function occupancy_intersect(pnt_a, pnt_b) {
  //let block_a = occupancy_block(pnt_a);
  //let block_b = occupancy_block(pnt_b);

  //let block_a = jeom.occupancy_block_map(pnt_a, [0.5,0,0], _EPS);
  //let block_b = jeom.occupancy_block_map(pnt_b, [0.5,0,0], _EPS);

  let block_a = jeom.occupancy_block_map(pnt_a, [0.5,0.5,0.5], [0,0,0], _EPS);
  let block_b = jeom.occupancy_block_map(pnt_b, [0.5,0.5,0.5], [0,0,0], _EPS);

  let _b = {};

  let icount = 0;

  for (let key_a in block_a) {
    if (key_a in block_b) {
      _b[key_a] = 1;
      icount++;
    }
  }

  for (let key_b in block_b) {
    if (key_b in block_a) {
      _b[key_b] = 1;
      icount++;
    }
  }
  return { "count": icount, "block": _b };
}


function count_point_On2(pnt_a, pnt_b) {
  //let _eps = (1/4096);
  //_eps = 1/256;
  let _eps = _EPS;
  let count = 0;
  for (let a=0; a<pnt_a.length; a+=3) {
    for (let b=0; b<pnt_b.length; b+=3) {
      let dx = pnt_a[a+0]-pnt_b[b+0];
      let dy = pnt_a[a+1]-pnt_b[b+1];
      let dz = pnt_a[a+2]-pnt_b[b+2];
      let d = Math.sqrt( dx*dx + dy*dy + dz*dz );
      if (d<_eps) { count++; }
    }
  }
  return count;
}

function mov_ipoint(ipnt, tv) {
  for (let i=0; i<ipnt.length; i++) {
    ipnt[i][0] += tv[0];
    ipnt[i][1] += tv[1];
    ipnt[i][2] += tv[2];
  }
}

function create_ipoint(pnt) {
  let _eps = _EPS;
  let C = Math.floor(1/_eps);

  let ipnt = [];
  for (let ii=0; ii<pnt.length; ii+=3) {
    let n = ipnt.length;
    ipnt.push( [ Math.round(C*pnt[ii+0]), Math.round(C*pnt[ii+1]), Math.round(C*pnt[ii+2]) ] );
  }

  ipnt.sort( vec_cmp );

  return ipnt;
}

// copmare two integer 3d points, sorted
// in lexagraphic x,y,z order.
//
function count_ipoint(ipnt_a, ipnt_b) {
  let _eps = _EPS;
  let count = 0;
  let a = 0, b = 0;

  let pnt = [];

  while ((a<ipnt_a.length) && (b<ipnt_b.length)) {

    let c = vec_cmp(ipnt_a[a], ipnt_b[b]);
    if (c < -_eps) { a++; continue; }
    if (c >  _eps) { b++; continue; }

    if ( (ipnt_a[a][0] == ipnt_b[b][0]) &&
         (ipnt_a[a][1] == ipnt_b[b][1]) &&
         (ipnt_a[a][2] == ipnt_b[b][2]) ) {

      pnt.push( [ ipnt_a[a][0], ipnt_a[a][1], ipnt_a[a][2] ] );
      count++;
    }

    a++;
    b++;
  }

  return { "count": count, "p": pnt } ;
}

function count_point(pnt_a, pnt_b) {
  let _eps = _EPS;
  let count = 0;
  let a = 0, b = 0;

  let C = Math.floor(1/_eps);

  let ipnt_a = create_ipoint(pnt_a);
  let ipnt_b = create_ipoint(pnt_b);

  return count_ipoint(ipnt_a, ipnt_b);
}

function _count_point(pnt_a, pnt_b) {
  let _eps = _EPS;
  let count = 0;
  let a = 0, b = 0;

  let C = Math.floor(1/_eps);

  let a_map = {},
      b_map = {};


  let ipnt_a = [],
      ipnt_b = [];
  let use_map = false;

  for (let ii=0; ii<pnt_a.length; ii+=3) {
    let n = ipnt_a.length;
    ipnt_a.push( [ Math.round(C*pnt_a[ii+0]), Math.round(C*pnt_a[ii+1]), Math.round(C*pnt_a[ii+2]) ] );

    if (use_map) {
      let key = ipnt_a[n][0].toString() + ":" + ipnt_a[n][1].toString() + ":" + ipnt_a[n][2].toString();
      a_map[key]=1;
    }
  }

  for (let ii=0; ii<pnt_b.length; ii+=3) {
    let n = ipnt_b.length;
    ipnt_b.push( [ Math.round(C*pnt_b[ii+0]), Math.round(C*pnt_b[ii+1]), Math.round(C*pnt_b[ii+2]) ] );

    if (use_map) {
      let key = ipnt_b[n][0].toString() + ":" + ipnt_b[n][1].toString() + ":" + ipnt_b[n][2].toString();
      b_map[key]=1;
    }
  }

  if (use_map) {
    for (let key in a_map) {
      if (key in b_map) { count++; }
    }
    for (let key in b_map) {
      if (key in a_map) { count++; }
    }
    return count;
  }

  ipnt_a.sort( vec_cmp );
  ipnt_b.sort( vec_cmp );

  while ((a<ipnt_a.length) && (b<ipnt_b.length)) {

    let c = vec_cmp(ipnt_a[a], ipnt_b[b]);
    if (c < -_eps) { a++; continue; }
    if (c >  _eps) { b++; continue; }

    if ( (ipnt_a[a][0] == ipnt_b[b][0]) &&
         (ipnt_a[a][1] == ipnt_b[b][1]) &&
         (ipnt_a[a][2] == ipnt_b[b][2]) ) {
      count++;
    }

    a++;
    b++;
  }

  return count;
}

//                   _       
//   _ __ ___   __ _(_)_ __  
//  | '_ ` _ \ / _` | | '_ \ 
//  | | | | | | (_| | | | | |
//  |_| |_| |_|\__,_|_|_| |_|
//                           

function com_point(pnt) {
  let com = [0,0,0];
  if (pnt.length == 0) { return com; }
  for (let ii=0; ii<pnt.length; i++) {
    com[0] += pnt[ii][0];
    com[1] += pnt[ii][1];
    com[2] += pnt[ii][2];
  }
  com[0] /= pnt.length;
  com[1] /= pnt.length;
  com[2] /= pnt.length;
  return com;
}

function find_face_dock(ctx, name) {
  ctx = ((typeof ctx === "undefined") ? g_ctx : ctx);
  let _eps = _EPS;

  let idx = ctx.name_index[name];

  let block_grid_info = {
    "x": -6, "X": 6,
    "y": -6, "Y": 6,
    "z": -6, "Z": 6,
    "nx": -1,
    "ny": -1,
    "nz": -1,
    "sx": 1/8, "sy": 1/8, "sz": 1/8,
    "dxyz" : [ 1/8, 1/8, 1/8 ],
    "dx": 1/8, "dy": 1/8, "dz": 1/8
  };

  let info = block_grid_info;


  let bbox = jeom.bounding_box( ctx.uniq_pnt[idx] );

  console.log("find_face_dock", name);

  let dsxyz = [
    bbox.X - bbox.x,
    bbox.Y - bbox.y,
    bbox.Z - bbox.z
  ];

  let bxyz = [
    Math.floor((dsxyz[0] + _eps) / info.dx),
    Math.floor((dsxyz[1] + _eps) / info.dy),
    Math.floor((dsxyz[2] + _eps) / info.dz)
  ];


  console.log(bbox);
  console.log(dsxyz, bxyz);

}

function cmp_ipnt(x,y) {
  if (x[0] < y[0]) { return -1; }
  if (x[0] > y[0]) { return  1; }

  if (x[1] < y[1]) { return -1; }
  if (x[1] > y[1]) { return  1; }

  if (x[2] < y[2]) { return -1; }
  if (x[2] > y[2]) { return  1; }

  return 0;
}

function construct_rot_geom(ctx, name) {
  ctx = ((typeof ctx === "undefined") ? g_ctx : ctx);

  let _eps = _EPS;
  let _idx = ctx.name_index[name];

  let geom_lib = [];
  let _dup_tri = jeom.dup(ctx.tri[_idx]);
  let _dup_uniq = jeom.dup(ctx.uniq_pnt[_idx]);
  let _orig_ipnt = create_ipoint(_dup_uniq);
  geom_lib.push({
    "name": name + "000",
    "tri": _dup_tri,
    "uniq": _dup_uniq,
    "ipnt": _orig_ipnt,
    "theta": 0,
    "theta_idx": 0
  });

  for (let theta_idx=1; theta_idx<4; theta_idx++) {

    let theta = theta_idx*Math.PI/2;

    let dup_uniq = jeom.dup(ctx.uniq_pnt[_idx]);
    jeom.roty(dup_uniq, theta);
    let rot_ipnt = create_ipoint(dup_uniq);

    rot_ipnt.sort( cmp_ipnt );
    

    let dup_tri = jeom.dup(ctx.tri[_idx]);
    jeom.roty(dup_tri, theta);

    // check for identical point in current geom_lib
    // if a point differs, we can break early out
    // of the ipnt comparison.
    // If a point doesn't differ, we must have an identical
    // rotated object, and we can break out of looking at
    // our current geom_lib.
    //
    // Only if current object has at least one point differing
    // from each entry in geom_lib do we then add it.
    //
    let ident = false;
    for (let geom_idx=0; geom_idx<geom_lib.length; geom_idx++) {

      let orig_ipnt = geom_lib[geom_idx].ipnt;
      orig_ipnt.sort( cmp_ipnt );

      ident = true;
      for (let ii=0; ii<orig_ipnt.length; ii++) {
        if ((Math.abs(orig_ipnt[ii][0] - rot_ipnt[ii][0]) > _eps) ||
            (Math.abs(orig_ipnt[ii][1] - rot_ipnt[ii][1]) > _eps) ||
            (Math.abs(orig_ipnt[ii][2] - rot_ipnt[ii][2]) > _eps)) {
          ident = false;
          break;
        }
      }

      if (ident) { break; }
    }

    if (!ident) {
      geom_lib.push({
        "name": name + "0" + theta_idx + "0",
        "tri": dup_tri,
        "uniq": dup_uniq,
        "ipnt": rot_ipnt,
        "theta": theta,
        "theta_idx": theta_idx
      });
    }

  }

  return geom_lib;
}

// returns dock_info object:
//
// {
//   "info": [
//     { "src": <srcname>, "dst": <dstname>, "axis": <x|y|z>, "rot": <radians>, "dxyz": [ <x>, <y>, <z> ], "p": [ ... ] },
//     ...
//     
//   ]
// }
//
// Where `p` is the points where they intersect.
// axis, rot and dxyz are all done to the `src` 3d object to align it to the `dst` 3d object.
//

/*
function find_dock_T(geom_a, geom_b) {
  let _eps = _EPS;

  let block_grid_info = {
    "x": -6, "X": 6,
    "y": -6, "Y": 6,
    "z": -6, "Z": 6,
    "nx": -1,
    "ny": -1,
    "nz": -1,

    "dxyz" : [ 1/2, 1/2, 1/2 ],
    "dx":1/2, "dz":1/2, "dy":1/2
  };

  let bbox_a = jeom.bounding_box( geom_a.tri );
  let bbox_b = jeom.bounding_box( geom_b.tri );

  let dx = block_grid_info.dx;
  let dy = block_grid_info.dy;
  let dz = block_grid_info.dz;

  let _mx = Math.floor( (bbox_b.x - (bbox_a.X - bbox_a.x)) / dx ) * dx;
  let _my = Math.floor( (bbox_b.y - (bbox_a.Y - bbox_a.y)) / dy ) * dy;
  let _mz = Math.floor( (bbox_b.z - (bbox_a.Z - bbox_a.z)) / dz ) * dz;

  let _Mx = Math.ceil( (bbox_b.X + (bbox_a.X - bbox_a.x)) / dx ) * dx;
  let _My = Math.ceil( (bbox_b.Y + (bbox_a.Y - bbox_a.y)) / dy ) * dy;
  let _Mz = Math.ceil( (bbox_b.Z + (bbox_a.Z - bbox_a.z)) / dz ) * dz;

  let _C = Math.floor(1/_EPS);
  let dock_info = {
    "src": geom_a.name,
    "dst": geom_b.name,
    "info" : [
      //{ "axis": "y", "rot": 0, "dxyz": [0,0,0], "src_point": [0,0,0], "dst_point":[0,0,0] }
    ],
  };

  let dup_a = jeom.dup( geom_a.tri );
  let dup_b = jeom.dup( geom_b.tri );

  let ipnt_a = create_ipoint(dup_a);
  let ipnt_b = create_ipoint(dup_b);

  block_grid_info.x = _mx;
  block_grid_info.y = _my;
  block_grid_info.z = _mz;

  block_grid_info.X = _Mx;
  block_grid_info.Y = _My;
  block_grid_info.Z = _Mz;

  block_grid_info.nx = Math.floor( (block_grid_info.X - block_grid_info.x + _EPS)/block_grid_info.dx );
  block_grid_info.ny = Math.floor( (block_grid_info.Y - block_grid_info.y + _EPS)/block_grid_info.dy );
  block_grid_info.nz = Math.floor( (block_grid_info.Z - block_grid_info.z + _EPS)/block_grid_info.dz );

  // experimental...

  for (let ix=0; ix<block_grid_info.nx; ix++) {
    for (let iy=0; iy<block_grid_info.ny; iy++) {
      for (let iz=0; iz<block_grid_info.nz; iz++) {

        let tv = [
          block_grid_info.x + ix*block_grid_info.dx,
          block_grid_info.y + iy*block_grid_info.dy,
          block_grid_info.z + iz*block_grid_info.dz,
        ];

        let itv = [ Math.floor(_C*tv[0]), Math.floor(_C*tv[1]), Math.floor(_C*tv[2]) ]

        let tv_n = [ -tv[0], -tv[1], -tv[2] ];
        let itv_n = [ -itv[0], -itv[1], -itv[2] ];

        jeom.mov(dup_a, tv);
        mov_ipoint(ipnt_a, itv);

        let _n = count_ipoint(ipnt_a, ipnt_b);

        if (_n.count==0) {
          jeom.mov(dup_a, tv_n);
          mov_ipoint(ipnt_a, itv_n);
          continue;
        }

        // slower than count_point
        //
        let m = occupancy_intersect(dup_a, dup_b);

        jeom.mov(dup_a, tv_n);
        mov_ipoint(ipnt_a, itv_n);

        if (m.count!=0) { continue; }

        if ((_n.count>=6) && (m.count==0) && (_n.count<=10)) {
          dock_info.info.push( {"dxyz":[tv[0], tv[1], tv[2]], "p": _n.p } );
        }
        else { }

      }
    }
  }

  return dock_info;
}
*/



// returns dock_info object:
//
// {
//   "info": [
//     { "src": <srcname>, "dst": <dstname>, "axis": <x|y|z>, "rot": <radians>, "dxyz": [ <x>, <y>, <z> ], "p": [ ... ] },
//     ...
//     
//   ]
// }
//
// Where `p` is the points where they intersect.
// axis, rot and dxyz are all done to the `src` 3d object to align it to the `dst` 3d object.
//
function find_dock(ctx, name_a, name_b) {
  ctx = ((typeof ctx === "undefined") ? g_ctx : ctx);

  let _eps = _EPS;

  //let a_idx = ctx.name_index[name_list[0]];
  //let b_idx = ctx.name_index[name_list[1]];

  let a_idx = ctx.name_index[name_a];
  let b_idx = ctx.name_index[name_b];

  let block_grid_info = {
    //"x": -4, "X": 3,
    //"y": -1, "Y": 3,
    //"z": -4, "Z": 3,
    "x": -6, "X": 6,
    "y": -6, "Y": 6,
    "z": -6, "Z": 6,
    "nx": -1,
    "ny": -1,
    "nz": -1,
    //"dxyz" : [ 1/2, 1/8, 1/2 ],
    //"dxyz" : [ 1/8, 1/8, 1/8 ],
    //"dx": 1/2, "dy": 1/8, "dz": 1/2
    //"dx": 1/8, "dy": 1/8, "dz": 1/8

    //"dxyz" : [ 1, 1/2, 1 ],
    //"dx":1, "dz":1, "dy":1/2
    "dxyz" : [ 1/2, 1/2, 1/2 ],
    "dx":1/2, "dz":1/2, "dy":1/2
  };

  let bbox_a = jeom.bounding_box( ctx.uniq_pnt[a_idx] );
  let bbox_b = jeom.bounding_box( ctx.uniq_pnt[b_idx] );

  let dx = block_grid_info.dx;
  let dy = block_grid_info.dy;
  let dz = block_grid_info.dz;

  let _mx = Math.floor( (bbox_b.x - (bbox_a.X - bbox_a.x)) / dx ) * dx;
  let _my = Math.floor( (bbox_b.y - (bbox_a.Y - bbox_a.y)) / dy ) * dy;
  let _mz = Math.floor( (bbox_b.z - (bbox_a.Z - bbox_a.z)) / dz ) * dz;

  let _Mx = Math.ceil( (bbox_b.X + (bbox_a.X - bbox_a.x)) / dx ) * dx;
  let _My = Math.ceil( (bbox_b.Y + (bbox_a.Y - bbox_a.y)) / dy ) * dy;
  let _Mz = Math.ceil( (bbox_b.Z + (bbox_a.Z - bbox_a.z)) / dz ) * dz;

  let _m = _mx;
  if (_m > _my) { _m = _my; }
  if (_m > _mz) { _m = _mz; }

  let _M = _Mx;
  if (_M < _My) { _M = _My; }
  if (_M < _Mz) { _M = _Mz; }

  /*
  console.log("## bbox: xyz{", _m, _M, "} ... ", JSON.stringify(bbox_a), JSON.stringify(bbox_b),
    "x{", bbox_b.x - (bbox_a.X - bbox_a.x), bbox_b.X + (bbox_a.X - bbox_a.x), "}",
    "y{", bbox_b.y - (bbox_a.Y - bbox_a.y), bbox_b.Y + (bbox_a.Y - bbox_a.y), "}",
    "z{", bbox_b.z - (bbox_a.Z - bbox_a.z), bbox_b.Z + (bbox_a.Z - bbox_a.z), "}" );
    */


  /*
  block_grid_info.x = bbox_b.x - (bbox_a.X - bbox_a.x);
  block_grid_info.X = bbox_b.X + (bbox_a.X - bbox_a.x);

  block_grid_info.y = bbox_b.y - (bbox_a.Y - bbox_a.y);
  block_grid_info.Y = bbox_b.Y + (bbox_a.Y - bbox_a.y);

  block_grid_info.z = bbox_b.z - (bbox_a.Z - bbox_a.z);
  block_grid_info.Z = bbox_b.Z + (bbox_a.Z - bbox_a.z);
  */

  block_grid_info.x = _m;
  block_grid_info.y = _m;
  block_grid_info.z = _m;

  block_grid_info.X = _M;
  block_grid_info.Y = _M;
  block_grid_info.Z = _M;

  block_grid_info.nx = Math.floor( (block_grid_info.X - block_grid_info.x + _EPS)/block_grid_info.dx );
  block_grid_info.ny = Math.floor( (block_grid_info.Y - block_grid_info.y + _EPS)/block_grid_info.dy );
  block_grid_info.nz = Math.floor( (block_grid_info.Z - block_grid_info.z + _EPS)/block_grid_info.dz );

  //console.log(block_grid_info);

  let _C = Math.floor(1/_EPS);

  let dock_info = {
    "src": name_a,
    "dst": name_b,
    "info" : [
      //{ "axis": "y", "rot": 0, "dxyz": [0,0,0], "src_point": [0,0,0], "dst_point":[0,0,0] }
    ],
  };

  for (let theta_idx=0; theta_idx<4; theta_idx++) {

    let theta = theta_idx*Math.PI/2;

    let dup_a = jeom.dup(ctx.uniq_pnt[a_idx]);
    let dup_b = jeom.dup(ctx.uniq_pnt[b_idx]);

    jeom.roty(dup_a, theta);

    let ipnt_a = create_ipoint(dup_a);
    let ipnt_b = create_ipoint(dup_b);


    // experimental...
    let bbox_a = jeom.bounding_box( dup_a );
    let bbox_b = jeom.bounding_box( dup_b );

    let dx = block_grid_info.dx;
    let dy = block_grid_info.dy;
    let dz = block_grid_info.dz;

    let _mx = Math.floor( (bbox_b.x - (bbox_a.X - bbox_a.x)) / dx ) * dx;
    let _my = Math.floor( (bbox_b.y - (bbox_a.Y - bbox_a.y)) / dy ) * dy;
    let _mz = Math.floor( (bbox_b.z - (bbox_a.Z - bbox_a.z)) / dz ) * dz;

    let _Mx = Math.ceil( (bbox_b.X + (bbox_a.X - bbox_a.x)) / dx ) * dx;
    let _My = Math.ceil( (bbox_b.Y + (bbox_a.Y - bbox_a.y)) / dy ) * dy;
    let _Mz = Math.ceil( (bbox_b.Z + (bbox_a.Z - bbox_a.z)) / dz ) * dz;

    /*
    let _m = _mx;
    if (_m > _my) { _m = _my; }
    if (_m > _mz) { _m = _mz; }

    let _M = _Mx;
    if (_M < _My) { _M = _My; }
    if (_M < _Mz) { _M = _Mz; }
    */

    /*
    console.log("## bbox: theta:", theta, "...", JSON.stringify(bbox_a), JSON.stringify(bbox_b),
      "x{", _mx, _Mx, "}",
      "y{", _my, _My, "}",
      "z{", _mz, _Mz, "}" );
      */

    block_grid_info.x = _mx;
    block_grid_info.y = _my;
    block_grid_info.z = _mz;

    block_grid_info.X = _Mx;
    block_grid_info.Y = _My;
    block_grid_info.Z = _Mz;

    block_grid_info.nx = Math.floor( (block_grid_info.X - block_grid_info.x + _EPS)/block_grid_info.dx );
    block_grid_info.ny = Math.floor( (block_grid_info.Y - block_grid_info.y + _EPS)/block_grid_info.dy );
    block_grid_info.nz = Math.floor( (block_grid_info.Z - block_grid_info.z + _EPS)/block_grid_info.dz );
    // experimental...

    //block_grid_info.nx += 4;
    //block_grid_info.ny += 4;
    //block_grid_info.nz += 4;

    //let _sx = block_grid_info.x - 2*block_grid_info.dx;
    //let _sy = block_grid_info.y - 2*block_grid_info.dy;
    //let _sz = block_grid_info.z - 2*block_grid_info.dz;

    for (let ix=0; ix<block_grid_info.nx; ix++) {
      for (let iy=0; iy<block_grid_info.ny; iy++) {
        for (let iz=0; iz<block_grid_info.nz; iz++) {

          let tv = [
            block_grid_info.x + ix*block_grid_info.dx,
            block_grid_info.y + iy*block_grid_info.dy,
            block_grid_info.z + iz*block_grid_info.dz,
            //_sx + ix*block_grid_info.dx,
            //_sy + iy*block_grid_info.dy,
            //_sz + iz*block_grid_info.dz,
          ];

          let itv = [ Math.floor(_C*tv[0]), Math.floor(_C*tv[1]), Math.floor(_C*tv[2]) ]

          let tv_n = [ -tv[0], -tv[1], -tv[2] ];
          let itv_n = [ -itv[0], -itv[1], -itv[2] ];

          jeom.mov(dup_a, tv);
          mov_ipoint(ipnt_a, itv);

          let _n = count_ipoint(ipnt_a, ipnt_b);

          if (_n.count==0) {
            jeom.mov(dup_a, tv_n);
            mov_ipoint(ipnt_a, itv_n);
            continue;
          }

          // slower than count_point
          //
          let m = occupancy_intersect(dup_a, dup_b);

          jeom.mov(dup_a, tv_n);
          mov_ipoint(ipnt_a, itv_n);

          //console.log(theta, tv, "_n:", _n, "m:", m);

          if (m.count!=0) { continue; }


          //if ((n==10) && (m==0)) {
          if ((_n.count>=6) && (m.count==0) && (_n.count<=10)) {

            dock_info.info.push( {"axis":"y", "rot": theta, "dxyz":[tv[0], tv[1], tv[2]], "p": _n.p } );

            // DEBUG
            //console.log(">>>:", name_a, name_b, ix,iy,iz, tv, theta, _n.count, m.count);
            //console.log(_n);
            //for (let ii=0; ii<_n.p.length; ii++) {
            //  console.log( _n.p[ii][0] * _EPS,  _n.p[ii][1] * _EPS,  _n.p[ii][2] * _EPS ); 
            //}

          }
          else {
            //console.log("___:", name_a, name_b, ix,iy,iz, tv, theta, _n.count, m.count);
          }

        }
      }
    }
  }

  return dock_info;
}

// returns dock object:
//
// {
//   "src": <name>,
//   "dst": <name>
//   "info" : [
//     { "dxyz": [ <dx>, <dy>, <dz> ], "p": [ ... ] },
//     ...
//   ]
// }
//
// Where info, if populated, holds transform information for `geom_b`
// to be transformed to line up with `geom_a`.
//
// The methodoligy is a little janky but thebasic idea is to translate
// one of the two geometries around and make sure that:
//
// * they don't intersect
// * they have some number of points in common
//
// The intersection is done via a coarse grained voxelization of the
// underlying geometry to make sure not voxels intersect.
//
// The common point testing is comparing the geometries as raw points and
// just counting how many are within and epsilon of each other.
//
// If the coarse grained voxels don't intersect and there are 6 to 10
// intersecting points, then the geometries are considered to be able to dock.
//
// The compromises here are:
//
// * Triangles are subdivided to make sure the coarse grained voxelization doesn't
//   skip any blocks. This is done through two levels of subdivision
// * The docking is done by consider just points and hard coding the number (6 to 10)
//
// This has been developed with Kenney's Marble tile set in mind, which is why the subdivision
// and docking hard coding have been chosen. It's not clear how to generalize this for other
// geometry.
//
function find_dock_t(geom_a, geom_b) {
  let _eps = _EPS;

  let block_grid_info = {
    "x": -6, "X": 6,
    "y": -6, "Y": 6,
    "z": -6, "Z": 6,
    "nx": -1,
    "ny": -1,
    "nz": -1,

    "dxyz" : [ 1/2, 1/2, 1/2 ],
    "dx":1/2, "dz":1/2, "dy":1/2
  };

  let _C = Math.floor(1/_EPS);
  let dock_info = {
    "comment": "if .info is populated, .dxyz will be translation of dst to be able to dock with src",
    "src": geom_a.name,
    "dst": geom_b.name,
    "C": _C,
    "info" : [
      //{ "axis": "y", "rot": 0, "dxyz": [0,0,0], "src_point": [0,0,0], "dst_point":[0,0,0] }
    ],
  };

  //let dup_a = jeom.dup( geom_a.tri );
  //let dup_b = jeom.dup( geom_b.tri );

  let dup_a = jeom.dup( geom_a.uniq );
  let dup_b = jeom.dup( geom_b.uniq );

  let occupancy_pnt_a = jeom.tri_subdivide( jeom.tri_subdivide( geom_a.tri ) );
  let occupancy_pnt_b = jeom.tri_subdivide( jeom.tri_subdivide( geom_b.tri ) );

  let ipnt_a = create_ipoint(dup_a);
  let ipnt_b = create_ipoint(dup_b);

  let bbox_a = jeom.bounding_box( dup_a );
  let bbox_b = jeom.bounding_box( dup_b );

  let dx = block_grid_info.dx;
  let dy = block_grid_info.dy;
  let dz = block_grid_info.dz;

  let _mx = Math.floor( (bbox_b.x - (bbox_a.X - bbox_a.x)) / dx ) * dx;
  let _my = Math.floor( (bbox_b.y - (bbox_a.Y - bbox_a.y)) / dy ) * dy;
  let _mz = Math.floor( (bbox_b.z - (bbox_a.Z - bbox_a.z)) / dz ) * dz;

  let _Mx = Math.ceil( (bbox_b.X + (bbox_a.X - bbox_a.x)) / dx ) * dx;
  let _My = Math.ceil( (bbox_b.Y + (bbox_a.Y - bbox_a.y)) / dy ) * dy;
  let _Mz = Math.ceil( (bbox_b.Z + (bbox_a.Z - bbox_a.z)) / dz ) * dz;

  block_grid_info.x = _mx;
  block_grid_info.y = _my;
  block_grid_info.z = _mz;

  block_grid_info.X = _Mx;
  block_grid_info.Y = _My;
  block_grid_info.Z = _Mz;

  block_grid_info.nx = Math.floor( (block_grid_info.X - block_grid_info.x + _EPS)/block_grid_info.dx );
  block_grid_info.ny = Math.floor( (block_grid_info.Y - block_grid_info.y + _EPS)/block_grid_info.dy );
  block_grid_info.nz = Math.floor( (block_grid_info.Z - block_grid_info.z + _EPS)/block_grid_info.dz );
  // experimental...

  for (let ix=0; ix<block_grid_info.nx; ix++) {
    for (let iy=0; iy<block_grid_info.ny; iy++) {
      for (let iz=0; iz<block_grid_info.nz; iz++) {

        let tv = [
          block_grid_info.x + ix*block_grid_info.dx,
          block_grid_info.y + iy*block_grid_info.dy,
          block_grid_info.z + iz*block_grid_info.dz,
        ];

        // points can be large so it's faster to transform then transform back, in place,
        // then to allocate a new point list.
        //
        let itv = [ Math.floor(_C*tv[0]), Math.floor(_C*tv[1]), Math.floor(_C*tv[2]) ]

        let tv_n = [ -tv[0], -tv[1], -tv[2] ];
        let itv_n = [ -itv[0], -itv[1], -itv[2] ];

        //jeom.mov(dup_a, tv);
        //mov_ipoint(ipnt_a, itv);

        jeom.mov(dup_b, tv);
        mov_ipoint(ipnt_b, itv);

        jeom.mov(occupancy_pnt_b, tv);

        let _n = count_ipoint(ipnt_a, ipnt_b);

        if (_n.count==0) {
          //jeom.mov(dup_a, tv_n);
          //mov_ipoint(ipnt_a, itv_n);
          jeom.mov(dup_b, tv_n);
          mov_ipoint(ipnt_b, itv_n);

          jeom.mov(occupancy_pnt_b, tv_n);

          continue;
        }

        // slower than count_point
        //
        //let m = occupancy_intersect(dup_a, dup_b);
        let m = occupancy_intersect(occupancy_pnt_a, occupancy_pnt_b);

        //jeom.mov(dup_a, tv_n);
        //mov_ipoint(ipnt_a, itv_n);

        jeom.mov(dup_b, tv_n);
        mov_ipoint(ipnt_b, itv_n);

        jeom.mov(occupancy_pnt_b, tv_n);

        if (m.count!=0) { continue; }

        if ((_n.count>=6) && (m.count==0) && (_n.count<=10)) {
          let ip_sort = [];
          let _rp = [];
          for (let p_idx=0; p_idx<_n.p.length; p_idx++) {
            _rp.push( [ _n.p[p_idx][0]/_C, _n.p[p_idx][1]/_C, _n.p[p_idx][2]/_C ] );
            ip_sort.push( [ _n.p[p_idx][0], _n.p[p_idx][1], _n.p[p_idx][2] ] );
          }
          ip_sort.sort( vec_cmp );
          dock_info.info.push( {"dxyz":[tv[0], tv[1], tv[2]], "ip": _n.p, "p": _rp, "ip_sort": ip_sort } );
        }
        else { }

      }
    }
  }

  return dock_info;
}


// ---------------------
// ---------------------
// ---------------------
// construct POMS file
//

function pnt_eq(a,b,_eps) {
  _eps = ((typeof _eps === "undefined") ? _EPS : _eps);
  if ((Math.abs( a[0] - b[0] ) < _eps) &&
      (Math.abs( a[1] - b[1] ) < _eps) &&
      (Math.abs( a[2] - b[2] ) < _eps)) {
    return true;
  }
  return false;
}


function construct_poms(ctx) {
  ctx = ((typeof ctx === "undefined") ? g_ctx : ctx);

  let _name_list = ctx.name_list;

  let oppo_dir = [ 1,0, 3,2, 5,4 ];
  let dxyz = [0.5, 0.5, 0.5];

  let tile_count = 1;
  let tile_name = [ "empty" ];
  let tile_weight = [ 1 ];

  let tile_obj_map = [ "empty.obj" ];
  let tile_obj_dxyz = [ [0,0,0] ];

  let obj_offset = [ 0.5, 0, 0.5 ];

  geom_lib = [];
  for (let ii=0; ii<_name_list.length; ii++) {

    //console.log("??", _name_list[ii]);

    let uniq_r = construct_rot_geom(g_ctx, _name_list[ii]);
    for (let jj=0; jj<uniq_r.length; jj++) {

      //console.log(">>>", uniq_r[jj]);

      // construct "voxelized" bounding voxel blocks for the geometry.
      // These reprsent the cell occupancy the object will eventually reside
      // in and will be needed for constructing the tiles associated with the
      // object as well as figuring out adjacency rules to other tiles/objects.
      //
      //let coarse_block_map = jeom.occupancy_block_map( uniq_r[jj].tri, [0.5, 0.5, 0.5], [0,0,0], _EPS );

      let _sstri = jeom.tri_subdivide( jeom.tri_subdivide( uniq_r[jj].tri ) );

      //let coarse_block_map = jeom.occupancy_block_map( uniq_r[jj].uniq, [0.5, 0.5, 0.5], [0,0,0], _EPS );
      let coarse_block_map = jeom.occupancy_block_map( _sstri, [0.5, 0.5, 0.5], [0,0,0], _EPS );

      //----
      //----
      /*
      console.log(uniq_r[jj].name, coarse_block_map);

      let block2idx = [];

      let _n = 8;
      let _n2 = Math.floor(_n/2);
      let gr = [];
      for (let __z=0; __z<_n; __z++) {
        gr.push([]);
        for (let __x=0; __x<_n; __x++) { gr[__z].push(0); }
      }

      for (let key in coarse_block_map) {
        let tok = key.split(":");
        let __x = parseInt(tok[0]) + _n2;
        let __z = parseInt(tok[2]) + _n2;
        gr[__z][__x] = 1;
      }

      for (let __z=0; __z<_n; __z++) {
        let field = [];
        for (let __x=0; __x<_n; __x++) {
          //field.push( (gr[__z][__x]>0) ? '*' : ' ' );
          field.push( (gr[__z][__x]>0) ? '*' : ' ' );
        }
        console.log( field.join("") );
      }
      */
      //----
      //----


      let min_v_idx = -1,
          min_v = [0,0,0];

      let coarse_block = [];
      for (let _key in coarse_block_map) {
        let v = [0,0,0];
        let tok = _key.split(":");
        v[0] = parseFloat(tok[0]) * dxyz[0];
        v[1] = parseFloat(tok[1]) * dxyz[1];
        v[2] = parseFloat(tok[2]) * dxyz[2];

        if (min_v_idx<0) {
          min_v_idx = 0;
          min_v = v;
          uniq_r[jj]["min_v"] = [ -v[0], -v[1], -v[2] ];
        }

        if (vec_cmp(v,min_v) < 0) {
          min_v_idx = coarse_block.length;
          min_v = v;
          uniq_r[jj]["min_v"] = [ -v[0], -v[1], -v[2] ];
        }

        //DEBUG
        //console.log(uniq_r[jj].name, ">>", coarse_block.length, v);

        coarse_block.push(v);


        //DEBUG
        //block2idx.push( [ parseInt(tok[0]), parseInt(tok[1]), parseInt(tok[2]) ] );
      }

      //console.log( uniq_r[jj].name, coarse_block_map);

      uniq_r[jj]["block_map"] = coarse_block_map;
      uniq_r[jj]["block"] = coarse_block;

      uniq_r[jj]["tile_rep"] = -1;

      uniq_r[jj]["block_tile_id"] = [];
      for (let block_idx=0; block_idx<coarse_block.length; block_idx++) {

        let _tn = uniq_r[jj].name + "_" + block_idx.toString();

        uniq_r[jj].block_tile_id.push( tile_count );
        tile_name.push( _tn );

        let tile_w = 1;
        if (ctx.weight_block_flag) { tile_w = 1.0 / coarse_block.length; }
        tile_weight.push(tile_w);

        // DEBUG
        // DEBUG
        /*
        let _n = 8;
        let _n2 = Math.floor(_n/2);
        let __x = block2idx[block_idx][0] + _n2;
        let __z = block2idx[block_idx][2] + _n2;
        gr[ __z ][ __x ] = tile_count;
        console.log(coarse_block[block_idx]);
        */
        // DEBUG
        // DEBUG


        // min_v_idx is the geometry representative.
        // If we're at the representative, add the representative
        // tile as the anchor point for hte geometry.
        // All other tiles are emtpy
        //
        if (min_v_idx == block_idx) {
          //console.log("!!", _tn, "(", min_v_idx, ")", uniq_r[jj].name);

          //tile_obj_map.push( uniq_r[jj].name + ".stl" );
          tile_obj_map.push( uniq_r[jj].name + ".obj" );

          let _dxyz = [ min_v[0] + obj_offset[0], min_v[1] + obj_offset[1], min_v[2] + obj_offset[2] ];
          tile_obj_dxyz.push( _dxyz );

          uniq_r[jj].tile_rep = min_v_idx;

          //console.log(">>>", uniq_r[jj].name, _dxyz, min_v, min_v_idx);
        }
        else {
          //console.log("  ", _tn, "...", "empty");
          //tile_obj_map.push( "empty.stl" );
          tile_obj_map.push( "empty.obj" );

          //tile_obj_dxyz.push( [ 0,0,0 ] );
        }

        tile_count++;
      }

      geom_lib.push(uniq_r[jj]);

      //DEBUG
      //DEBUG
      /*
      for (let __z=_n-1; __z>=0; __z--) {
        let field = [];
        for (let __x=0; __x<_n; __x++) {
          if (gr[__z][__x] > 0) {
            if (gr[__z][__x] < 10) {
              field.push( " " + gr[__z][__x].toString() );
            }
            else {
              field.push( gr[__z][__x].toString() );
            }
          } else {
            field.push( "  ");
          }
        }
        console.log( field.join(" ") );
      }
      */
      //DEBUG
      //DEBUG


    }


  }
  geom_lib.tile_count = tile_count;

  //let output_obj = true;
  if (ctx.output_obj) {
    //let rescale_v = [2,2,2];
    for (let ii=0; ii<geom_lib.length; ii++) {

      let dup_tri = jeom.dup( geom_lib[ii].tri );
      //jeom.mov(dup_tri, tile_obj_dxyz[ii]);
      jeom.mov(dup_tri, geom_lib[ii].min_v);
      jeom.scale(dup_tri, ctx.obj_rescale_v);

      //console.log("tile_rep", geom_lib[ii].tile_rep, "shifting", geom_lib[ii].name, "by", tile_obj_dxyz[ii]);
      //console.log("tile_rep", geom_lib[ii].tile_rep, "shifting", geom_lib[ii].name, "by", geom_lib[ii].min_v);


      //let obj_name = geom_lib[ii].name + ".stl";
      //fs.writeFileSync( obj_name, jeom.stl_stringify(dup_tri) );


      let pfx = "./";
      if (("obj_dir" in ctx) && (ctx.obj_dir.length>0)) { pfx = ctx.obj_dir + "/"; }
      obj_name = geom_lib[ii].name + ".obj";
      fs.writeFileSync( pfx + obj_name, jeom.obj_stringify(dup_tri) );

      //console.log(">>", obj_name, tile_obj_dxyz[ii]);

    }
  }

  // keep track of which tiles can have the 0 tile as its neighbor
  //
  let tile_zero_nei = Array(tile_count);
  for (let tidx=0; tidx<tile_count; tidx++) {
    tile_zero_nei[tidx] = [1,1, 1,1, 1,1];
  }

  let nei_dxyz = [
    [  1,  0,  0 ], [ -1,  0,  0 ],
    [  0,  1,  0 ], [  0, -1,  0 ],
    [  0,  0 , 1 ], [  0,  0, -1 ]
  ];
  for (let ii=0; ii<nei_dxyz.length; ii++) {
    nei_dxyz[ii][0] *= dxyz[0];
    nei_dxyz[ii][1] *= dxyz[1];
    nei_dxyz[ii][2] *= dxyz[2];
  }

  // add the 'internal' tile adjacency to the rules
  // list
  //
  let rule_list = [];
  for (let geom_idx=0; geom_idx<geom_lib.length; geom_idx++) {
    let geom = geom_lib[geom_idx];
    let tile_a = geom.block_tile_id;
    let block = geom.block;

    for (let b_idx=0; b_idx<block.length; b_idx++) {

      for (let idir=0; idir<6; idir++) {
        let tpnt = [
          block[b_idx][0] + nei_dxyz[idir][0],
          block[b_idx][1] + nei_dxyz[idir][1],
          block[b_idx][2] + nei_dxyz[idir][2]
        ];

        for (let _idx=0; _idx<block.length; _idx++) {
          if (_idx == b_idx) { continue; }
          if (pnt_eq(tpnt, block[_idx])) {
            rule_list.push( [ tile_a[b_idx], tile_a[_idx], idir, 1 ] );

            let rdir = oppo_dir[idir];
            tile_zero_nei[ tile_a[b_idx] ][idir] = 0;
            tile_zero_nei[ tile_a[_idx] ][rdir]  = 0;

            break;
          }
        }

      }

    }

  }

  // To find out which geometry matches with the others,
  // we consider all pairs of geometry and translate them
  // in dscrete grid steps.
  // `find_dock_t` does all the low level geometry moving
  // to find matching points, so check that function
  // for more details.
  // If `find_dock_t` finds docking information,
  // the subsequent code matches up the finer grained tiles
  // and adds them to the rule set.
  //
  let n = geom_lib.length;

  let cur_count=0;
  for (let src_geom_idx=0; src_geom_idx < n; src_geom_idx++) {
    for (let dst_geom_idx=src_geom_idx; dst_geom_idx < n; dst_geom_idx++) {

      //DEBUG
      console.log("## (", tile_name[src_geom_idx], ":", src_geom_idx, ",",
                          tile_name[dst_geom_idx], ":", dst_geom_idx, ",",
                          cur_count, "/", n*n, " (n:", n,"))");
      cur_count++;

      let src_geom = geom_lib[src_geom_idx];
      let dst_geom = geom_lib[dst_geom_idx];

      let src_tile = src_geom.block_tile_id;
      let dst_tile = dst_geom.block_tile_id;

      let _di = find_dock_t(geom_lib[src_geom_idx], geom_lib[dst_geom_idx]);

      // DEBUG
      /*
      console.log(_di.src, _di.dst);
      if (_di.info.length > 0) {
        for (let _ii=0; _ii<_di.info.length; _ii++) {
          console.log(_di.info[_ii]);
        }
        console.log("src(anch):", src_geom.name, src_geom.block);
        console.log("dst(test):", dst_geom.name, dst_geom.block);
      }
      */

      for (let di_idx=0; di_idx<_di.info.length; di_idx++) {
        let die = _di.info[di_idx];
        let dst_dxyz  = die.dxyz;

        let anch_pnt = jeom.dup_tri(src_geom.block);
        let test_pnt  = jeom.dup_tri(dst_geom.block);

        let orig_test_pnt = dst_geom.block;

        for (let ii=0; ii<test_pnt.length; ii++) {
          test_pnt[ii][0] += dst_dxyz[0];
          test_pnt[ii][1] += dst_dxyz[1];
          test_pnt[ii][2] += dst_dxyz[2];
        }

        for (let t_idx=0; t_idx < test_pnt.length; t_idx++) {
          for (let a_idx=0; a_idx < anch_pnt.length; a_idx++) {

            for (let idir=0; idir<6; idir++) {
              let nei_pnt = [
                test_pnt[t_idx][0] + nei_dxyz[idir][0],
                test_pnt[t_idx][1] + nei_dxyz[idir][1],
                test_pnt[t_idx][2] + nei_dxyz[idir][2]
              ];

              let rdir = oppo_dir[idir];

              let _eps = _EPS;

              if (pnt_eq(nei_pnt, anch_pnt[a_idx], _eps)) {

                let _cs = 0.25;
                let _ds = 0.5;

                //console.log("nei_pnt:", nei_pnt, "anch_pnt[", a_idx, "]", anch_pnt[a_idx]);

                let _tbbox = {
                  "x": test_pnt[t_idx][0],
                  "X": test_pnt[t_idx][0]+_ds,
                  "y": test_pnt[t_idx][1],
                  "Y": test_pnt[t_idx][1]+_ds,
                  "z": test_pnt[t_idx][2],
                  "Z": test_pnt[t_idx][2]+_ds
                };

                let _abbox = {
                  "x": anch_pnt[a_idx][0],
                  "X": anch_pnt[a_idx][0]+_ds,
                  "y": anch_pnt[a_idx][1],
                  "Y": anch_pnt[a_idx][1]+_ds,
                  "z": anch_pnt[a_idx][2],
                  "Z": anch_pnt[a_idx][2]+_ds
                };

                //console.log("tbbox:", _tbbox)
                //console.log("abbox:", _abbox)

                let in_count = 0;
                for (let dock_p_idx=0; dock_p_idx<die.p.length; dock_p_idx++) {

                  //console.log("  ", die.p[dock_p_idx],
                  //  _in_bbox(die.p[dock_p_idx], _tbbox),
                  //  _in_bbox(die.p[dock_p_idx], _abbox),
                  //  _in_bbox_face(die.p[dock_p_idx], _tbbox),
                  //  _in_bbox_face(die.p[dock_p_idx], _abbox) );


                  //if (_in_bbox(die.p[dock_p_idx], _tbbox) &&
                  //    _in_bbox(die.p[dock_p_idx], _abbox)) {
                  if ((_in_bbox_face(die.p[dock_p_idx], _tbbox) > 1) &&
                      (_in_bbox_face(die.p[dock_p_idx], _abbox) > 1)) {
                    in_count++;
                  }
                }

                //console.log("incount:", in_count);

                if (in_count > 0) {

                  rule_list.push( [ dst_tile[t_idx], src_tile[a_idx], idir, 1 ] );
                  rule_list.push( [ src_tile[a_idx], dst_tile[t_idx], rdir, 1 ] );

                  tile_zero_nei[ dst_tile[t_idx] ][idir] = 0;
                  tile_zero_nei[ src_tile[a_idx] ][rdir] = 0;

                  //DEBUG
                  let __t = dst_tile[t_idx];
                  let __a = src_tile[a_idx];

                  console.log("## match:", tile_name[__t], "-(", idir, ")->", tile_name[__a]);

                }

                //console.log("MATCH",
                //  "idir:", idir,
                //  "t_idx:", t_idx, "test_pnt:", test_pnt[t_idx], "(test_orig:", orig_test_pnt[t_idx], ")",
                //  "a_idx:", a_idx, "anch_pnt:", anch_pnt[a_idx]);
              }
            }

          }
        }

      }

    }
  }

  for (let tile=0; tile<tile_zero_nei.length; tile++) {
    for (let idir=0; idir<6; idir++) {
      if (tile_zero_nei[tile][idir] > 0.5) {
        let rdir = oppo_dir[idir];
        rule_list.push( [ tile, 0, idir, 1 ] );
        rule_list.push( [ 0, tile, rdir, 1 ] );
      }
    }
  }

  //console.log("!!!", tile_name);

  let poms = {
  };

  poms.rule = rule_list;
  poms.name = tile_name;
  poms.weight = tile_weight;
  poms.constraint= [];
  poms.boundaryCondition = {
    "x+":{"type":"tile","value":0}, "x-":{"type":"tile","value":0},
    "y+":{"type":"tile","value":0}, "y-":{"type":"tile","value":0},
    "z+":{"type":"tile","value":0}, "z-":{"type":"tile","value":0}
  };
  poms.size = [32,32,32];
  poms.quiltSize = [128,128,128];
  poms.objMap = tile_obj_map;

  return poms;
}

function poms_print(poms) {
  let boundaryCondition_str = "{\n\
    \"x+\":{\"type\":\"tile\",\"value\":0}, \"x-\":{\"type\":\"tile\",\"value\":0},\n\
    \"y+\":{\"type\":\"tile\",\"value\":0}, \"y-\":{\"type\":\"tile\",\"value\":0},\n\
    \"z+\":{\"type\":\"tile\",\"value\":0}, \"z-\":{\"type\":\"tile\",\"value\":0}\n\
  }";

  console.log("{");
  console.log("  \"rule\":[");
  let field = [];
  for (let ii=0; ii<poms.rule.length; ii++) {
    sfx = "";

    field.push( JSON.stringify(poms.rule[ii]) );

    if (ii<(poms.rule.length-2)) { sfx = ","; }
    if ((ii>0) && ((ii%8)==0)) { 
      console.log( "    " + field.join(",") + sfx)
      field = [];
    }

  }
  if (field.length>0) { console.log( "    " + field.join(",") ); }
  console.log("  ],");

  field = [];
  console.log("  \"name\":[");
  for (let ii=0; ii<poms.name.length; ii++) {
    sfx = "";


    if (ii<(poms.name.length)) { sfx = ","; }
    if ((ii>0) && ((ii%8)==0)) { 
      console.log( "    " + field.join(",") + sfx)
      field = [];
    }

    field.push( JSON.stringify(poms.name[ii]) );

  }
  if (field.length>0) { console.log( "    " + field.join(",") ); }
  console.log("  ],");

  field = [];
  console.log("  \"weight\":[");
  for (let ii=0; ii<poms.weight.length; ii++) {
    sfx = "";

    if (ii<(poms.weight.length)) { sfx = ","; }
    if ((ii>0) && ((ii%8)==0)) { 
      console.log( "    " + field.join(",") + sfx)
      field = [];
    }

    field.push( JSON.stringify(poms.weight[ii]) );
  }
  if (field.length>0) { console.log( "    " + field.join(",") ); }
  console.log("  ],");

  field = [];
  console.log("  \"objMap\":[");
  for (let ii=0; ii<poms.objMap.length; ii++) {
    sfx = "";

    if (ii<(poms.objMap.length)) { sfx = ","; }
    if ((ii>0) && ((ii%8)==0)) { 
      console.log( "    " + field.join(",") + sfx)
      field = [];
    }

    field.push( JSON.stringify(poms.objMap[ii]) );
  }
  if (field.length>0) { console.log( "    " + field.join(",") ); }
  console.log("  ],");

  //console.log("  \"boundaryConditions\":", JSON.stringify(poms.boundaryCondition) + ",");
  console.log("  \"boundaryConditions\":", boundaryCondition_str  + ",");
  console.log("  \"constraint\":", JSON.stringify(poms.constraint) + ",");

  console.log("  \"size\":", JSON.stringify(poms.size) + ",");
  console.log("  \"quiltSize\":", JSON.stringify(poms.quiltSize)  );

  console.log("}");

  //console.log( JSON.stringify(poms, undefined, 2) );

  //console.log("rule_list");
  //for (let ii=0; ii<rule_list.length; ii++) { console.log(ii, rule_list[ii]); }
  //return;



}

//
// ---------------------
// ---------------------
// ---------------------

function kenney_marble_run() {
  let _name_list = [
    "end_hole_rounded", "end_hole_square", // !!!
    "end_rounded", "end_square", // !!!

    "bend", "bend_large", "bend_medium",
    "bump_A", "bump_B", "bump_C", "bump_D",
    "corner",
    "cross", // !!!
    "curve", "curve_large",
    //"curve_wide", "curve_wide_large", "curve_wide_medium",

    "funnel", "funnel_long",

    "helix_half_left", "helix_half_right",
    "helix_large_half_left", "helix_large_half_right",
    "helix_large_left", "helix_large_right",
    "helix_large_quarter_left", "helix_large_quarter_right",
    "helix_left", "helix_right",
    "helix_quarter_left", "helix_quarter_right",

    //"ramp_long_A",

    "ramp_long_B", "ramp_long_C", "ramp_long_D",

    "s-curve_left", "s-curve_right",
    "s-curve_left_large", "s-curve_right_large",
    "s-curve_short_left_C", //"s-curve_short_right_C",

    // slants need to be paired with ramp start and ends
    // to look nice
    //
    //---
    //"slan_long_D",
    //"slant_B", "slant_C", "slant_D",
    //"slant_long_B", "slant_long_C",
    //---
    // "rampStart_A.obj", "rampStart_B.obj",
    // "rampStart_C.obj", "rampStart_D.obj",
    // "rampEnd_A.obj", "rampEnd_B.obj",
    // "rampEnd_C.obj", "rampEnd_D.obj",
    //---

    "split", "split_double", "split_double_sides", // !!!
    "split_large_left", "split_large_right", // !!!
    "split_left", "split_right", // !!!

    "straight", "straight_hole"
    //"straight_wide", "straight_wide_hole",
    //"wave_A", "wave_B", "wave_C",
    //"tunnel"
  ];

  // quick debugging
  /*
  _name_list = [
    "end_hole_rounded", "end_hole_square", // !!!
    "corner",
    "end_rounded", "end_square" // !!!
  ];
  */


  g_ctx.name_list = _name_list;
  let poms = construct_poms(g_ctx);

  // By default, remove some of the tiles from the map.
  // We want to keep them in as options in case we want
  // to force some configurations.
  //
  let remove_list = [
    "end_hole_rounded", "end_hole_square",
    "end_rounded", "end_square",
    "corner",
    "cross",
    "split", "split_double", "split_double_sides",
    "split_large_left", "split_large_right",
    "split_left", "split_right"
  ];


  let remove_tile_id = [];
  for (let ridx=0; ridx<remove_list.length; ridx++) {
    let remove_name = remove_list[ridx];

    for (let tile_id=0; tile_id<poms.name.length; tile_id++) {
      let name = poms.name[tile_id];

      if ( name.match( remove_name + '\\d\\d\\d_\\d+$' ) ) {
        remove_tile_id.push( tile_id );
      }
    }
  }
  remove_tile_id.sort( function(a,b) { return (a<b) ? (-1) : ((b>a) ? 1 : 0); } );

  let remove_range = [];

  let s = remove_tile_id[0];
  for (let ii=1; ii<remove_tile_id.length; ii++) {
    if (remove_tile_id[ii] != (remove_tile_id[ii-1]+1)) {
      remove_range.push([s,remove_tile_id[ii-1]+1]);
      s = remove_tile_id[ii];
    }
  }
  remove_range.push([s, remove_tile_id[ remove_tile_id.length-1 ]+1]);

  for (let ii=0; ii<remove_range.length; ii++) {
    poms.constraint.push({ "type":"remove", "range":{"tile":remove_range[ii], "x":[], "y":[], "z":[]}});
  }

  //----

  let reweight_factor = {
    "empty": 250,
    "helix_" : 5,
    "ramp_" : 4
  };

  for (let reweight_pfx in reweight_factor) {
    for (let tile_id=0; tile_id < poms.name.length; tile_id++) {
      let name = poms.name[tile_id];
      if (name.match( '^' + reweight_pfx )) {
        poms.weight[ tile_id ] *= reweight_factor[ reweight_pfx ];
      }
    }
  }

  //---

  poms_print(poms);

}

kenney_marble_run();

//let name_list  = [ "bump_A", "helix_half_right" ];
//let name_list  = [ "helix_large_right", "helix_large_right" ];
//let name_list  = [ "bend", "bend_large" ];
//let name_list = [];

let _debug = -1;
if (_debug == 1) {



  //DEBUG
  //DEBUG
  let name_list  = [ "bend", "bend_large" ];
  find_dock(g_ctx, name_list[0], name_list[1]);
  process.exit();
  //DEBUG
  //DEBUG
}

if (_debug == 2) {

  console.log("## debug 2");

  let name_list  = [ "rampStart_solid_A", "s-curve_short_left_B", "bend", "bend_large" ];
  find_face_dock(g_ctx, name_list[0]);
  process.exit();
}

if (_debug == 3) {

  let _name_list = [
    "bend", "bend_large", "bend_medium",
    "bump_A", "bump_B", "bump_C", "bump_D",
    "corner", "cross",
    "curve", "curve_large",
    //"curve_wide", "curve_wide_large", "curve_wide_medium",

    "end_hole_rounded", "end_hole_square",
    "end_rounded", "end_square",

    "funnel", "funnel_long",

    "helix_half_left", "helix_half_right",
    "helix_large_half_left", "helix_large_half_right",
    "helix_large_left", "helix_large_right",
    "helix_large_quarter_left", "helix_large_quarter_right",
    "helix_left", "helix_right",
    "helix_quarter_left", "helix_quarter_right",

    //"ramp_long_A",

    "ramp_long_B", "ramp_long_C", "ramp_long_D",

    "s-curve_left", "s-curve_right",
    "s-curve_left_large", "s-curve_right_large",
    "s-curve_short_left_C", //"s-curve_short_right_C",

    // slants need to be paired with ramp start and ends
    // to look nice
    //
    //---
    //"slan_long_D",
    //"slant_B", "slant_C", "slant_D",
    //"slant_long_B", "slant_long_C",
    //---
    // "rampStart_A.obj", "rampStart_B.obj",
    // "rampStart_C.obj", "rampStart_D.obj",
    // "rampEnd_A.obj", "rampEnd_B.obj",
    // "rampEnd_C.obj", "rampEnd_D.obj",
    //---


    "split", "split_double", "split_double_sides",
    "split_large_left", "split_large_right",
    "split_left", "split_right",
    "straight", "straight_hole"

    //"straight_wide", "straight_wide_hole",
    //"wave_A", "wave_B", "wave_C",
    //"tunnel"
  ];

  var _name_list_0 = [
    "bend", "bend_large", "bend_medium",
    "bump_A", "bump_B", "bump_C", "bump_D",
    "corner", "cross",
    "curve", "curve_large",

    "end_hole_rounded", "end_hole_square",
    "end_rounded", "end_square",

    "funnel", "funnel_long",

    "ramp_long_B", "ramp_long_C", "ramp_long_D",

    "s-curve_left", "s-curve_right",
    "s-curve_left_large", "s-curve_right_large",
    "s-curve_short_left_C", //"s-curve_short_right_C",

    "slan_long_D",
    "slant_B", "slant_C", "slant_D",
    "slant_long_B", "slant_long_C",
    "split", "split_double", "split_double_sides",
    "split_large_left", "split_large_right",
    "split_left", "split_right",
    "straight", "straight_hole"
  ]

  let xxxx = [
    "corner", "cross",
    "curve", "curve_large",
    //"curve_wide", "curve_wide_large", "curve_wide_medium",

    "end_hole_rounded", "end_hole_square",
    "end_rounded", "end_square",


    "helix_half_left", "helix_half_right",
    "helix_large_half_left", "helix_large_half_right",
    "helix_large_left", "helix_large_right",
    "helix_large_quarter_left", "helix_large_quarter_right",
    "helix_left", "helix_right",
    "helix_quarter_left", "helix_quarter_right",

    //"ramp_long_A",

    "ramp_long_B", "ramp_long_C", "ramp_long_D",

    "s-curve_left", "s-curve_right",
    "s-curve_left_large", "s-curve_right_large",
    "s-curve_short_left_C", //"s-curve_short_right_C",

    "slan_long_D",
    "slant_B", "slant_C", "slant_D",
    "slant_long_B", "slant_long_C",
    "split", "split_double", "split_double_sides",
    "split_large_left", "split_large_right",
    "split_left", "split_right",
    "straight", "straight_hole"
    //"straight_wide", "straight_wide_hole",
    //"wave_A", "wave_B", "wave_C",
    //"tunnel"
  ];

  // cp, works
  //_name_list = [ "straight", "cross", "corner", "end_square", "end_hole_square" ];

  var _name_list__x = [

    "helix_half_left",
    "helix_half_right",

    "helix_large_half_left", "helix_large_half_right",
    "helix_large_left", "helix_large_right",
    "helix_large_quarter_left", "helix_large_quarter_right",

    "helix_left",
    "helix_right",

    "bend", "bend_large", "bend_medium",
    //"bump_A", "bump_B", "bump_C", "bump_D",

    //"ramp_long_B", "ramp_long_C", "ramp_long_D",

    //"slan_long_D",
    //"slant_B", "slant_C", "slant_D",
    //"slant_long_B", "slant_long_C",

    "cross",
    "corner",
    "curve",
    "straight"

    //"curve",
    //"straight"
    //"straight", "cross", "corner", "end_square", "end_hole_square"
  ];

  var _name_list_1 = [
    "bend",
    "corner", "cross",
    "curve", "straight"
  ];

  //DEBUG
  //DEBUG
  /*
  let straight_index = g_ctx.name_index["straight"];
  let straight_tri = g_ctx.tri[straight_index];

  //console.log(straight_tri);
  let _a = jeom.tri_subdivide(straight_tri);
  let __a = jeom.tri_subdivide(_a);
  console.log( jeom.stl_stringify(__a) );
  process.exit();
  */
  //DEBUG
  //DEBUG


  g_ctx.name_list = _name_list;


  construct_poms(g_ctx);

  let x_x=false;
  if (x_x) {
  process.exit();

  let geom_lib = [];


  let _idx = g_ctx.name_index["bend"];

  //DEBUG
  for (let ii=0; ii<_name_list.length; ii++) {
    let _r = construct_rot_geom(g_ctx, _name_list[ii]);
    for (let jj=0; jj<_r.length; jj++) {
      geom_lib.push(_r[jj]);
    }
  }

  g_ctx.geom_lib = geom_lib;

  for (let ii=0; ii<4; ii++) {
    console.log(">>", geom_lib[ii].name);
  }

  console.log("point diff...:", pnt_diff( g_ctx.uniq_pnt[_idx], geom_lib[0].tri));

  let _di = find_dock_t(geom_lib[0], geom_lib[2]);
  for (let di_idx=0; di_idx < _di.info.length; di_idx++) {
    console.log(_di.info[di_idx].dxyz);
  }

  process.exit();

  _name_list = [ "bend" ];

  let out_info = {
    "name_index": {},
    "b": [],
    "dock": []
  };

  let dxyz = [0.5, 0.5, 0.5];

  

  //_name_list = ["curve_wide"];
  for (let ii=0; ii<_name_list.length; ii++) {

    console.log(_name_list[ii], ii);
    out_info.name_index[ _name_list[ii] ] = ii;

    let _idx = g_ctx.name_index[ _name_list[ii] ];

    let coarse_block_map = jeom.occupancy_block_map(g_ctx.uniq_pnt[_idx], [0.5,0.5,0.5], [0,0,0], _EPS);

    let coarse_block = [];
    for (let _key in coarse_block_map) {
      let v = [0,0,0];
      let tok = _key.split(":");
      v[0] = parseFloat(tok[0]) * dxyz[0];
      v[1] = parseFloat(tok[1]) * dxyz[1];
      v[2] = parseFloat(tok[2]) * dxyz[2];
      coarse_block.push(v);
    }

    out_info.b.push({ "name": _name_list[ii], "block_map": coarse_block_map, "block":  coarse_block });

    for (let jj=ii; jj<_name_list.length; jj++) {

      //console.log("## trying:", _name_list[ii], _name_list[jj]);

      let dock_info = find_dock(g_ctx, _name_list[ii], _name_list[jj] );
      //console.log("##", _name_list[ii], _name_list[jj], "dock_count:", dock_info.info.length);

      out_info.dock.push(dock_info);

      let _di = dock_info.info;
      for (let idx=0; idx<_di.length; idx++) {
        let _d = _di[idx];
        //console.log(dock_info.src, dock_info.dst, _d.axis, _d.rot, _d.dxyz, _d.p.length);
      }

    }
  }

  //WIP!!!!
  // need to create new rotated object
  // create tile per occupied block
  // get adjacency from the below simple block translates
  //

  for (let dock_idx=0; dock_idx<out_info.dock.length; dock_idx++) {
    let di = out_info.dock[dock_idx].info;

    let src_name  = out_info.dock[dock_idx].src;
    let src_idx   = out_info.name_index[ src_name ];

    for (let di_idx=0; di_idx<di.length; di_idx++) {
      let die = di[di_idx];
      let src_axis  = die.axis;
      let src_rot   = die.rot;
      let src_dxyz  = die.dxyz;

      let block_pnt = jeom.dup(out_info.b[ src_idx ].block);

      let _fbp = [];
      for (let ii=0; ii<block_pnt.length; ii++) {
        for (let jj=0; jj<block_pnt[ii].length; jj++) {
          _fbp.push( block_pnt[ii][jj] );
        }
      }

      jeom.roty( _fbp, src_rot );
      jeom.mov( _fbp, src_dxyz );

      console.log("src:", src_name, "...", src_axis, src_rot, src_dxyz, "-->");

      let tpnt = [];

      for (let ii=0; ii<Math.floor(_fbp.length/3); ii++) {
        console.log("  ", _fbp[ii], _fbp[ii+1], _fbp[ii+2]);
        tpnt.push( [ _fbp[ii], _fbp[ii+1], _fbp[ii+2] ] );
      }

      let nei_dxyz = [
        [ 1, 0, 0 ],
        [ -1, 0, 0 ],
        [ 0, 1, 0 ],
        [ 0, -1, 0],
        [ 0, 0 , 1],
        [ 0, 0, -1]
      ];

      for (let t_idx=0; t_idx < tpnt.length; t_idx++) {
        for (let b_idx=0; b_idx < block_pnt.length; b_idx++) {

          for (let idir=0; idir<6; idir++) {
            let _tpnt = [
              tpnt[t_idx][0] + nei_dxyz[idir][0],
              tpnt[t_idx][1] + nei_dxyz[idir][1],
              tpnt[t_idx][2] + nei_dxyz[idir][2]
            ];

            let _eps = _EPS;

            if ((Math.abs( _tpnt[0] - block_pnt[b_idx][0] ) < _eps) &&
                (Math.abs( _tpnt[1] - block_pnt[b_idx][1] ) < _eps) &&
                (Math.abs( _tpnt[2] - block_pnt[b_idx][2] ) < _eps)) {
              console.log("MATCH", "t_idx:", t_idx, "t_pnt:", tpnt[t_idx], "b_idx:", b_idx, "b_pnt:", block_pnt[b_idx]);
            }
          }

        }
      }

    }

  }

  console.log( JSON.stringify(out_info, undefined, 2) );

  process.exit(0);
  } // x_x
}

function ___xxx() {

  name_list = [];
  for (let name in g_ctx.name_index) {
    name_list.push(name);
  }


  //name_list  = [ "bend", "bend_large" ];
  //name_list  = [ "bend", "bend_medium" ];
  //name_list  = [ "bend_medium", "untitled" ];
  name_list = [ "bend_large", "helix_large_right" ];
  for (let ii=0; ii<name_list.length; ii++) {
    if (name_list[ii].match( /banner/ )) { continue; }
    if (name_list[ii].match( /marble/ )) { continue; }
    for (let jj=ii; jj<name_list.length; jj++) {
    //for (let jj=ii+1; jj<name_list.length; jj++) {
      if (name_list[jj].match( /banner/ )) { continue; }
      if (name_list[jj].match( /marble/ )) { continue; }

      console.log("##", ii, jj, name_list[ii], name_list[jj]);

      let dock_info = find_dock(g_ctx, name_list[ii], name_list[jj]);

      console.log("##", name_list[ii], name_list[jj], "dock_count:", dock_info.info.length);

      let _di = dock_info.info;
      for (let idx=0; idx<_di.length; idx++) {
        let _d = _di[idx];
        console.log(dock_info.src, dock_info.dst, _d.axis, _d.rot, _d.dxyz, _d.p.length);
      }
    }
  }


}
