// LICENSE: CC0
//

import * as THREE from "three";
import { OBJLoader } from 'three_OBJLoader';
import { MTLLoader } from 'three_MTLLoader';
import { OrbitControls } from 'three_OrbitControls';

// https://github.com/mrdoob/three.js/tree/dev/examples/jsm

// notes:
// https://discourse.threejs.org/t/how-to-set-different-textures-on-each-instancedmesh/29433/2

window.OBJLoader = OBJLoader;
window.MTLLoader = MTLLoader;

let tile_chunk = {
  "emptyMap": {},
  "objMap":[],
  "data" : [],
  "cellSize": [],
  "region": [[0,0], [0,0], [0,0]]
};

var ctx = {
  "t_prv": 0,

  "poll_ms" : 1000,

  "poms_url" : "data/poms.json",
  "patch_url" : "data/patch.json",

  "three": THREE,
  "camera": {},
  "controls": {},
  "scene": {},
  "renderer": {},
  "scratch3d": new THREE.Object3D(),

  // instanced mesh
  "mesh": [],

  "tile": [],
  //"tile_pos": [],

  "loader": {},
  "material": {},

  "size": [10,10,10],
  "cell_count":-1,
  "tile_count":-1,

  "poms": {},
  "patch": {},

  "time": 0,
  "camera_still_counter" : 0,
  "camera_still_threshold" : 240,
  "camera_full_threshold" : 300,
  "camera_speed" : 1/16,
  "camera_auto_rotate" : true,
  "camera_state" : "idle",

  "camera_up" : [0,0,1],

  "particle": {
    "init_pos": false,
    "point_size": 1/12,
    "density": 2,
    "geometry": null,
    "material": null,
    "points": null,
    "n": 0,
    "position": [],
    "color": []
  },

  "load": {
    "list": [ ],

    "tile_chunk_ready": false,
    "poms_ready": false,
    "mesh_ready": false,
    "ready": false,
    "buffer": 0,

    "mesh_progress":0
  },

  "camera_distance": [ 1.125, 2, 1.125 ],
  "aspect_ratio": 1,
  "W" : 100,
  "H": 100
};

function ease_in_alpha(x, alpha)      { return Math.pow(x, alpha); }
function ease_out_alpha(x, alpha)     { return 1-Math.pow(1-x, alpha); }
function ease_in_out_alpha(x, alpha)  {
  if (x<0.5) { return ease_in_alpha(2*x, alpha)/2.0; }
  return (ease_out_alpha(2*(x-0.5), alpha)/2.0) + 0.5;
}



/* 0 <= h, s, v <= 1  */
/* https://stackoverflow.com/a/17243070/4002265 CC-BY-SA3.0 */
function HSVtoRGB(out, h, s, v) {
    var r, g, b, i, f, p, q, t;
    //if (arguments.length === 1) { s = h.s, v = h.v, h = h.h; }
    i = Math.floor(h * 6);
    f = h * 6 - i;
    p = v * (1 - s);
    q = v * (1 - f * s);
    t = v * (1 - (1 - f) * s);
    switch (i % 6) {
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
    }

    //return {
        out.r= r;
        out.g= g;
        out.b= b;
    //};
}


// fetch JSON, call callback cb
// uses native json parsing
//
function fetchJSON(url, cb) {
  let req = new XMLHttpRequest();
  req.responseType = 'json';
  req.open("GET", url, true);
  req.onload = (function(_x) {
    return function (z, y) {
      let json_data = _x.response;
      if (typeof cb !== "undefined") { cb(json_data); }
    };
  })(req);
  req.onerror = function(a,b) {
    console.log("error.0", a,b);
  };
  req.addEventListener("error", function(a,b) {
    console.log("error.1", a,b);
  });
  req.send(null);
}

// fetch JSON, call callback cb
// "old stle" JSON parsing
//
function _fetchJSON(url, cb) {
  let req = new XMLHttpRequest();
  req.responseType = 'application/json';
  req.open("GET", url, true);
  req.onload = function () {
    let json_data = JSON.parse(req.responseText);
    if (typeof cb !== "undefined") { cb(json_data); }
  };
  req.send(null);
}

function vec2cell(v, sz) {
  return (v[2]*sz[0]*sz[1]) + (v[1]*sz[0]) + v[0];
}

function cell2vec(cell,sz) {
  let v = [-1,-1,-1];

  v[2] = Math.floor( cell / (sz[0]*sz[1]) );
  cell -= v[2]*sz[0]*sz[1];
  v[1] = Math.floor( cell / (sz[0]) );
  cell -= v[1]*sz[0];
  v[0] = cell;

  return v;
}

//---
//---

function refresh_patch() {


  if ((!ctx.load.poms_ready) ||
      (!ctx.load.mesh_ready)) {
    setTimeout( refresh_patch, 100 );
    return;
  }

  let url = ctx.patch_url;
  url += "?t=" + Date.now().toString();

  fetchJSON( url, function(patch) {

    if (!("patch" in ctx.patch)) { ctx.patch = patch; }
    if (!("size" in ctx.patch)) { ctx.patch = patch; }
    if (!("offset" in ctx.patch)) { ctx.patch = patch; }

    if ((patch.size[0] != ctx.patch.size[0]) ||
        (patch.size[1] != ctx.patch.size[1]) ||
        (patch.size[2] != ctx.patch.size[2])) {
      ctx.patch = patch;
    }

    if ((patch.offset[0] != ctx.patch.offset[0]) ||
        (patch.offset[1] != ctx.patch.offset[1]) ||
        (patch.offset[2] != ctx.patch.offset[2])) {
      ctx.patch = patch;
    }

    if (patch.patch.length != ctx.patch.patch.length) {
      ctx.patch = patch;
    }

    let idx=0;
    for (idx=0; idx<patch.patch.length; idx++) {
      if (ctx.patch.patch[idx] != patch.patch[idx]) {
        ctx.patch = patch;
        break;
      }
    }

    for (let xyz=0; xyz<3; xyz++) {
      tile_chunk.region[xyz][0] = ctx.patch.offset[xyz];
      tile_chunk.region[xyz][1] = ctx.patch.size[xyz] + ctx.patch.offset[xyz];
    }

    let n = tile_chunk.data.length;
    let m = ctx.patch.patch.length;
    for (let i=n; i<m; i++) { tile_chunk.data.push(-1); }
    for (let i=m; i<n; i++) { tile_chunk.data.pop(); }

    for (let i=0; i<ctx.patch.patch.length; i++) {
      tile_chunk.data[i] = ctx.patch.patch[i];
    }

    if ("cellSize" in patch) {
      n = tile_chunk.cellSize.length;
      m = patch.cellSize.length;
      for (let i=n; i<m; i++) { tile_chunk.cellSize.push(-1); }
      for (let i=m; i<n; i++) { tile_chunk.cellSize.pop(); }

      for (let i=0; i<patch.cellSize.length; i++) {
        tile_chunk.cellSize[i] = patch.cellSize[i];
      }
    }
    else {
      n = tile_chunk.cellSize.length;
      m = patch.patch.length;
      for (let i=n; i<m; i++) { tile_chunk.cellSize.push(-1); }
      for (let i=m; i<n; i++) { tile_chunk.cellSize.pop(); }

      for (let i=0; i<tile_chunk.cellSize.length; i++) {
        tile_chunk.cellSize[i] = 1;
      }
    }

    if ((ctx.patch.size[0] != ctx.poms.quiltSize[0]) ||
        (ctx.patch.size[1] != ctx.poms.quiltSize[1]) ||
        (ctx.patch.size[2] != ctx.poms.quiltSize[2])) {
      ctx.poms.quiltSize[0] = ctx.patch.size[0];
      ctx.poms.quiltSize[1] = ctx.patch.size[1];
      ctx.poms.quiltSize[2] = ctx.patch.size[2];

      ctx.poms.size[0] = ctx.patch.size[0];
      ctx.poms.size[1] = ctx.patch.size[1];
      ctx.poms.size[2] = ctx.patch.size[2];

      free_three();
      init_three();
    }
    else {
      ctx.load.tile_chunk_ready = true;
      update_ready();
    }

  });

  setTimeout( refresh_patch, ctx.poll_ms );
}


//---
//---

function animate() {

  let t_now = Date.now();
  let dt = t_now - ctx.t_prv;
  ctx.t_prv = t_now;

  let dt_s = dt / 1000.0;

  requestAnimationFrame(animate);
  let r = ctx.controls.update();

  ctx.camera_still_counter++;
  if (ctx.camera_still_counter >= ctx.camera_still_threshold) {

    if ((ctx.camera_state == "idle") &&
        (ctx.camera_auto_rotate)) {

      let s = (ctx.camera_still_counter - ctx.camera_still_threshold) / ctx.camera_full_threshold;
      if (s >= 1.0) {
        ctx.controls.autoRotateSpeed = ctx.camera_speed;
      }
      else {
        ctx.controls.autoRotateSpeed = ctx.camera_speed * ease_in_out_alpha( s, 4.0 );
      }

      ctx.controls.autoRotate = true
      r = ctx.controls.update();
    }

  }


  /*
  if (!r) {
    ctx.camera_still_counter++;
    if (ctx.camera_still_counter == ctx.camera_still_threshold) {

      if (ctx.camera_auto_rotate) {
        ctx.controls.autoRotate = true
        r = ctx.controls.update();
      }
    }
  }
  else {
    ctx.camera_still_counter = 0;
    ctx.controls.autoRotate = false
    r = ctx.controls.update();
  }
  */

  render();
}

function onWindowResize() {
  ctx.camera.aspect = window.innerWidth / window.innerHeight;
  ctx.camera.updateProjectionMatrix();
  ctx.renderer.setSize( window.innerWidth, window.innerHeight );
}

function refresh_grid_pos() {
  let poms = ctx.poms;
  let P = ctx.particle;
  let geom = P.geometry;

  let _m = [ poms.quiltSize[0]/2, poms.quiltSize[1]/2, poms.quiltSize[2]/2 ];

  let C_min = 1/32;
  let C_max = 1/2;

  C_min=0;
  C_max=1;


  let pos = geom.attributes.position.array;
  for (let ii=0; ii<pos.length; ii+=3) {
    let cell = Math.floor( ii / (3*P.density) );
    let v = cell2vec(cell, poms.quiltSize);
    for (let xyz=0; xyz<3; xyz++) {
      let r = (C_max - C_min)*Math.random() + C_min;
      pos[ii+xyz] = r - _m[xyz] + v[xyz];
    }
  }
  geom.attributes.position.needsUpdate = true;
  geom.attributes.color.needsUpdate = true;
}

function update_fow() {
  let poms = ctx.poms;
  let P = ctx.particle;
  let geom = P.geometry;

  let pos = geom.attributes.position.array;
  let clr = geom.attributes.color.array;

  let _m = [ poms.quiltSize[0]/2, poms.quiltSize[1]/2, poms.quiltSize[2]/2 ];

  let C = 1.0;
  let rgb = [1,1,1];
  let cell_unit = [1,1,1];

  let C_min = 1/32;
  let C_max = 1;

  C_max = C_min ;//+ 1/64;

  let tile_count = poms.name.length;

  //let def_clr = [1/32, 1/32, 1/32];
  let def_clr = [1/8, 1/8, 1/8];

  let _trgb = { "r": 1.0, "g": 1.0, "b": 1.0 };
  let _t = 1.0;

  if (!ctx.particle.init_pos) {
    ctx.particle.init_pos=true;
    refresh_grid_pos();
  }

  for (let ii=0; ii<pos.length; ii+=3) {

    let cell = Math.floor( ii / (3*P.density) );
    let v = cell2vec(cell, poms.quiltSize);

    //v[0] += cell_unit[0]/2;
    //v[1] += cell_unit[1]/2;
    //v[2] += cell_unit[2]/2;

    //rgb[0]=1; rgb[1]=1; rgb[2]=1;
    rgb[0]=def_clr[0];
    rgb[1]=def_clr[1];
    rgb[2]=def_clr[2];

    C = 1.0;
    if (tile_chunk.data[cell] < 0) {
      C = C_min;
      rgb[0] = 0.125; rgb[1] = 0; rgb[2] = 0;
      rgb[0] = 0; rgb[1] = 0.75; rgb[2] = 0.75;
    }
    else {
      C = ((C_max - C_min)*tile_chunk.cellSize[cell] / tile_count) + C_min;

      //let p = tile_chunk.cellSize[cell] / tile_count;
      //let _v = HSVtoRGB(p, 0.9, 0.9);


      rgb[0] = 0;
      rgb[1] = 1;
      rgb[2] = 0;

      _t = (tile_chunk.cellSize[cell]-1)/ tile_count;
      if (_t<0) { _t = 0; }
      HSVtoRGB(_trgb, _t, 0.8, 0.8);

      rgb[0] = _trgb.r;
      rgb[1] = _trgb.g;
      rgb[2] = _trgb.b;


      //rgb[0] = _v.r;
      //rgb[1] = _v.g;
      //rgb[2] = _v.b;

      if (tile_chunk.cellSize[cell] == 1) {
        rgb[0] = 0; rgb[1] = 0; rgb[2] = 0;
      }

    }

    for (let xyz=0; xyz<3; xyz++) {

      //let r = Math.random();
      let r = 1;
      r = C*r;

      //pos[ii+xyz] = r - _m[xyz] + v[xyz];
      clr[ii+xyz] = rgb[xyz];
    }
  }
  geom.attributes.position.needsUpdate = true;
  geom.attributes.color.needsUpdate = true;
}

function render() {
  if (!ctx.load.ready) { return; }

  let poms = ctx.poms;

  //let amount = 10;
  let time = Date.now() * 0.001;
  //let amount = (ctx.size[0] + ctx.size[1] + ctx.size[2])/3;
  //amount*=1.5;
  //let offset = -( amount - 1 ) / 2;
  //offset = -amount;
  //offset = 0;

  //time = ctx.time;

  let offset = [ -ctx.size[0]/2, -ctx.size[1]/2, -ctx.size[2]/2 ];
  //offset = [ ctx.size[0]/2, ctx.size[1]/2, ctx.size[2]/2 ];


  for (let tile=0; tile<ctx.tile_count; tile++) {
    ctx.tile[tile].count = 0;
  }

  for (let mesh_idx=0; mesh_idx<ctx.mesh.length; mesh_idx++) {
    ctx.mesh[mesh_idx].count = 0;
  }

  for (let cell=0; cell<ctx.cell_count; cell++) {

    let tile = tile_chunk.data[cell];
    if (tile < 0) { continue; }
    if (tile >= ctx.tile_count) { continue; }

    //console.log("???", JSON.stringify(ctx.tile));

    if (typeof ctx.tile[tile] === "undefined") {
      console.log(tile, ctx.tile[tile], ctx.tile.length, ctx.tile_count);
    }

    ctx.tile[tile].count++;
  }


  for (let cell=0; cell<ctx.cell_count; cell++) {

    let tile = tile_chunk.data[cell];
    if (tile < 0) { continue; }
    if (tile >= ctx.tile_count) {
      console.log("ERROR", tile, ctx.tile_count);
      continue; }

    // skip 0 tile (should be empty)
    //
    //if (tile==0) { continue; }

    if (ctx.tile[tile].pos.length < ctx.tile[tile].count) {
      let _s = ctx.tile[tile].pos.length;
      let _e = ctx.tile[tile].count;

      for (let t=_s; t<_e; t++) {
        ctx.tile[tile].pos.push([0,0,0]);
      }
    }
  }

  for (let cell=0; cell<ctx.cell_count; cell++) {

    let tile = tile_chunk.data[cell];
    if (tile < 0) { continue; }
    if (tile >= ctx.tile_count) { continue; }

    let mesh_idx = ctx.tile2mesh_idx[tile];
    let instance_idx = ctx.mesh[mesh_idx].count;

    // skip 0 tile (should be empty)
    //
    if (tile==0) { continue; }
    if (mesh_idx == 0) { continue; }

    ctx.mesh[mesh_idx].count++;
    //ctx.tile[tile].count++;

    let v = cell2vec(cell, ctx.size);
    //let v = cell2vec(cell, ctx.quiltSize);

    // world rotation
    //
    //ctx.mesh[mesh_idx].rotation.x = Math.sin( time / 4 );
    //ctx.mesh[mesh_idx].rotation.y = Math.sin( time / 2 );

    ctx.tile[tile].pos[instance_idx][0] = v[0];
    ctx.tile[tile].pos[instance_idx][1] = v[1];
    ctx.tile[tile].pos[instance_idx][2] = v[2];

    ctx.scratch3d.position.set( offset[0]+v[0], offset[1]+v[1], offset[2]+v[2] );
    ctx.scratch3d.updateMatrix();

    ctx.mesh[mesh_idx].setMatrixAt( instance_idx, ctx.scratch3d.matrix );
    ctx.mesh[mesh_idx].instanceMatrix.needsUpdate = true;
  }

  for (let mesh_idx=0; mesh_idx<ctx.mesh.length; mesh_idx++) {
    ctx.mesh[mesh_idx].computeBoundingSphere();
  }

  //---

  update_fow();

  //---

  let custom_cam = false;

  if (custom_cam) {
    let v_ab = [ Math.cos(time/4), Math.sin(time/4) ];
    let v_abc = [
      Math.cos(time/4),
      Math.cos(time/Math.sqrt(2)),
      Math.sin(time/4)
    ];
    let _S = ctx.camera_distance;
    let cpos = [ v_abc[0]*_S[0]*ctx.size[0], v_abc[1]*_S[1]*ctx.size[1], v_abc[2]*_S[2]*ctx.size[2] ];
    ctx.camera.position.set( cpos[0], cpos[1], cpos[2] );
    ctx.camera.lookAt(0,0,0);
    ctx.camera.updateProjectionMatrix();
  }

  ctx.renderer.render( ctx.scene, ctx.camera );
}


function debug() {
  ctx.mesh.traverse( function(child) {
    if (child instanceof THREE.Mesh) {
      child.material = ctx.material;
    }
  });
}

function update_ready() {
  if ((ctx.load.mesh_ready) &&
      (ctx.load.tile_chunk_ready) &&
      (ctx.load.poms_ready)) {
    ctx.load.ready = true;
  }
}

function init() {

  let url = window.location.href;
  let params = url.split("?");
  if (params.length > 1) {

    let tok = params[1].split("&");
    for (let ii=0; ii<tok.length; ii++) {
      let kv = tok[ii].split("=");

      if (kv.length < 2) { continue; }

      if (kv[0] == "poms") {
        ctx.poms_url = kv[1];
      }

      else if (kv[0] == "patch") {
        ctx.patch_url = kv[1];
      }

      else if (kv[0] == "poll") {
        ctx.poll_ms = float(kv[1]);
      }

      else if (kv[0] == "up") {
        if      (kv[1] == "x") { ctx.camera_up = [1,0,0]; }
        else if (kv[1] == "y") { ctx.camera_up = [0,1,0]; }
        else if (kv[1] == "z") { ctx.camera_up = [0,0,1]; }
      }

    }

  }


  console.log("using:");
  console.log("POMS:", ctx.poms_url);
  console.log("PATCH:", ctx.patch_url);
  console.log("poll:", ctx.poll_ms, "ms");

  let poms_url = ctx.poms_url;

  poms_url += "?t=" + Date.now().toString();
  fetchJSON( poms_url, function (poms) {
    ctx.poms = poms;

    ctx.load.poms_ready = true;

    update_ready();

    init_three();
  });


  setTimeout( refresh_patch, 100 );

}

// doing my best, probably needs to less ham fisted
// here (might introduce memory leakes if not disposed
// properly, also not checking for initialization in the
// first place)
//
function free_three() {
  document.body.innerHTML = '';

  ctx.controls.dispose();
  ctx.renderer.dispose();
  //ctx.scene.dispose();
  //ctx.camera.dispose();
}

function init_three() {

  ctx.t_prv = Date.now();

  let poms = ctx.poms;

  for (let xyz=0; xyz<3; xyz++) {
    ctx.size[xyz] = poms.size[xyz];
    if ("quiltSize" in poms) {
      ctx.size[xyz] = poms.quiltSize[xyz];
    }
  }

  if (!("objMap" in poms)) {
    console.log("ERROR: no 'objMap' field detected in POMS config file");
    return;
  }

  ctx.tile2mesh_idx = [];

  let obj_list = [];
  let obj_uniq = {};
  for (let tile=0; tile<poms.objMap.length; tile++) {
    let obj_fn = poms.objMap[tile];
    if (!(obj_fn in obj_uniq)) {

      let mesh_idx = obj_list.length;

      ctx.tile2mesh_idx.push( mesh_idx );

      obj_uniq[obj_fn] = mesh_idx;
      obj_list.push( obj_fn );
    }
    else {
      ctx.tile2mesh_idx.push( obj_uniq[obj_fn] );
    }
  }
  ctx.load.list = obj_list;
  ctx.load["uniq"] = obj_uniq;


  let _scale = 0.5;
  _scale = 1/1;

  ctx.cell_count = ctx.size[0]*ctx.size[1]*ctx.size[2];
  //ctx.tile_count = ctx.load.list.length;
  ctx.tile_count = poms.name.length;

  ctx.tile = [];
  for (let tile_id=0; tile_id<ctx.tile_count; tile_id++) {
    ctx.tile.push( {"pos":[], "count":0} );

    let n = 1;
    for (let instance_idx=0; instance_idx<n; instance_idx++) {
      ctx.tile[tile_id].pos.push([0,0,0]);

      let rx = Math.floor(Math.random()*10);
      let ry = Math.floor(Math.random()*10);
      let rz = Math.floor(Math.random()*10);

      ctx.tile[tile_id].pos[instance_idx][0] = rx;
      ctx.tile[tile_id].pos[instance_idx][1] = ry;
      ctx.tile[tile_id].pos[instance_idx][2] = rz;
    }
  }

  //---
  //---
  //---

  if (typeof window !== "undefined") {
    ctx.W = window.innerWidth;
    ctx.H = window.innerHeight;
  }
  ctx.aspect_ratio = ctx.W / ctx.H;

  ctx.camera = new THREE.PerspectiveCamera( 60, ctx.aspect_ratio, 0.1, 10000 );

  //experiment
  ctx.camera.up.set( ctx.camera_up[0], ctx.camera_up[1], ctx.camera_up[2] );

  let _S = ctx.camera_distance;
  let cpos = [ _S[0]*ctx.size[0], _S[1]*ctx.size[1], _S[2]*ctx.size[2] ];
  ctx.camera.position.set( cpos[0], cpos[1], cpos[2] );
  ctx.camera.lookAt(0,0,0);

  //---
  //---
  //---

  ctx.scene = new THREE.Scene();

  ctx.obj_loader = new OBJLoader();
  ctx.mtl_loader = new MTLLoader();

  ctx.mesh = Array( ctx.load.list.length );

  for (let ii=0; ii<ctx.load.list.length; ii++) {
    let ele = ctx.load.list[ii];

    if (ele.match(/\.obj$/)) {

      ctx.obj_loader.load( ctx.load.list[ii], (function(_fn) {
        return function(obj_data) {
          let count = ctx.cell_count;

          const material = new THREE.MeshNormalMaterial();
          ctx["mnm_material"] = material;
          ctx["obj_data"] = obj_data;

          if (obj_data.children.length == 0) { return ; }

          obj_data.children[0].geometry.scale( _scale, _scale, _scale );


          console.log("attempting to load", _fn);

          let mesh = new THREE.InstancedMesh( obj_data.children[0].geometry, ctx.mnm_material, count );
          mesh.instanceMatrix.setUsage( THREE.DynamicDrawUsage );

          mesh.count = 0;

          ctx.scene.add( mesh );

          //ctx.mesh.push(mesh);
          let mesh_idx = ctx.load.uniq[ _fn ];
          ctx.mesh[ mesh_idx ] = mesh;

          ctx.load.mesh_progress++;

          if (ctx.load.mesh_progress == ctx.load.list.length) {
            ctx.load.mesh_ready=true;
            update_ready();
          }

        }
      })(ele) );

    }

  }

  //---
  // particles

  let P = ctx.particle;

  //P.n = poms.size[0]*poms.size[1]*poms.size[2]*P.density;
  P.n = ctx.size[0]*ctx.size[1]*ctx.size[2]*P.density;

  P.position = Array(3*P.n);
  P.color = Array(3*P.n);
  P.geometry = new THREE.BufferGeometry();

  let _pnt_size = P.point_size;

  let color = new THREE.Color();
  let _m = [ poms.size[0]/2, poms.size[1]/2, poms.size[2]/2 ];
  let _range = {
    "x": -(poms.size[0] - _m[0]), "X": (poms.size[0] - _m[0]),
    "y": -(poms.size[1] - _m[1]), "Y": (poms.size[1] - _m[1]),
    "z": -(poms.size[2] - _m[2]), "Z": (poms.size[2] - _m[2])
  };

  let C = 1;
  let x=0, y=0, z=0;
  let r=0, g=0, b=0;
  for (let ii=0; ii<P.n; ii++) {

    let cell = Math.floor( ii / P.density );
    let v = cell2vec(cell, poms.size);

    //x = (Math.random()*(_range.X - _range.x)) + _range.x;
    //y = (Math.random()*(_range.Y - _range.y)) + _range.y;
    //z = (Math.random()*(_range.Z - _range.z)) + _range.z;

    x = C*Math.random() + v[0] - _m[0];
    y = C*Math.random() + v[1] - _m[1];
    z = C*Math.random() + v[2] - _m[2];

    r = Math.random();
    g = Math.random();
    b = Math.random();


    r=1; g=1; b=1;


    color.set(r,g,b);

    P.position[3*ii+0] = x;
    P.position[3*ii+1] = y;
    P.position[3*ii+2] = z;
    P.color[3*ii+0] = color.r;
    P.color[3*ii+1] = color.g;
    P.color[3*ii+2] = color.b;
  }

  P.geometry.setAttribute( 'position', new THREE.Float32BufferAttribute( P.position, 3 ) );
  P.geometry.setAttribute( 'color', new THREE.Float32BufferAttribute( P.color, 3 ) );

  P.geometry.computeBoundingSphere();
  P.material = new THREE.PointsMaterial( { size: _pnt_size, vertexColors: true } );
  P.points = new THREE.Points( P.geometry, P.material );
  ctx.scene.add( P.points );


  //---

  ctx.renderer = new THREE.WebGLRenderer( { antialias: true } );
  ctx.renderer.setPixelRatio( window.devicePixelRatio );
  ctx.renderer.setSize( window.innerWidth, window.innerHeight );
  document.body.appendChild( ctx.renderer.domElement );


  //----
  //----

  ctx.controls = new OrbitControls( ctx.camera, ctx.renderer.domElement );
  ctx.controls.listenToKeyEvents( window );

  ctx.controls.enableDamping = true;
  ctx.controls.dampingFactor = 0.5;

  ctx.controls.screenSpacePanning = false;

  ctx.controls.minDistance = 0.125;
  ctx.controls.maxDistance = 1000;

  ctx.controls.maxPolarAngle = Math.PI ;

  ctx.controls.target = new THREE.Vector3(0,0,0);

  ctx.controls.autoRotateSpeed = ctx.camera_speed;
  ctx.controls.autoRotate = false;

  ctx.controls.addEventListener( 'start', function(ev) {
    //console.log("controls.start:", ev);
    ctx.controls.autoRotate = false;

    ctx.camera_state = "active";
  });

  ctx.controls.addEventListener( 'change', function(ev) {
    //console.log("controls.change:", ev);
  });

  ctx.controls.addEventListener( 'end', function(ev) {
    ctx.controls.autoRotate = false;
    ctx.camera_state = "idle";
    ctx.camera_still_counter = 0;

    //console.log("controls.end:", ev);
  });

  ctx.controls.update();

  //ctx.autoRotate = true;
  //ctx.autoRotateSpeed = 20;

  //ctx.camera.up.set(0,1,0);
  //ctx.camera.up.set( ctx.camera_up[0], ctx.camera_up[1], ctx.camera_up[2] );


  //----
  //----


  window.addEventListener( 'resize', onWindowResize );

  setTimeout( animate, 1 );
}

export { ctx, init, debug, tile_chunk, fetchJSON, _fetchJSON, HSVtoRGB };
