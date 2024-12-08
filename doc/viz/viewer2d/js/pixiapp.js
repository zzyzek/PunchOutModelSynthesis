// LICENSE: CC0
//
// To the extent possible under law, the person who associated CC0 with
// this file has waived all copyright and related or neighboring rights
// to this file.
//
// You should have received a copy of the CC0 legalcode along with this
// work. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//

// a quick note on control flow:
//
// * `init()` is called to initialize the websocket connection
//   (`init_ws()`), create the pixi application
//   and setup the keyboard bindings
// * `poll_tiled()` polls for the snapshot file to load
// * `update_pixi_tilemap(...)` does the main draw:
//   - if the (implied) url for the tile set has changed,
//     reload it and reconstruct the atlas
//     (`construct_atlas()`, `load_tileset(...)`)
//   - if the tile map size has changed, resize the
//     tilemap (`resize_pixi_tilemap(...)`)
//   - finally, draw the tilemap

// This application is still pretty minimal.
// There are some lurking bugs in terms of rendering
// the tile map (sometimes there are vestigial sprites
// being displayed?) and other issues regarding the
// websocket connections (doesn't try to reconnect etc.).
//
// If problems are encountered, try restarting the web client
// and, of course, make sure the web server and websocket server
// are running (see README.md).
//
// Naviatgion:
//
// wasd         - up left down right
// hjkl         - left down up right
// Arrow keys   - up down left right
// shift+arrow  - zoom in and out
//
//
// run-time options:
//
// < > . /  - viz mode (simple, fog, ?) (default simple)
// f F      - turn on/off adaptive fog (default on)
// [ {      - decrease fog exponent (default 3, min 1, max 5)
// ] }      - incrase fog exponent (default 3, min 1, max 5)
// q - _    - decrease zoom amount
// e = +    - increase zoom amount


// Note that Tiled does not respect tile ids below 1,
// so tile ID of 1 represents the first tile in the tile
// set (`first_gid` below, hard coded to 1).
// 0 or negative tile IDs are invalid or transparent.
// When displaying, to get the proper index into the tile set,
// the first GID has to be subtracted from the tile ID.
//

// the reference point is (0,0) for world coordinate transforms,
// which means all scales happen relative to upper left hand corner.
// It'd be nice to put this in a better location or be able to move
// it in the future.
//

//var WEBSOCKET_URL = "ws://localhost";
//var WEBSOCKET_PORT = "3000";

var g_atlasTemplate = {
  "_frames": {
    "fame_name": {
      "frame": { "x": 452, "y": 232, "w": 169, "h": 226 },
      "rotated": false,
      "trimmed": true,
      "spriteSourceSize": { "x": 3, "y": 4, "w": 169, "h": 226 },
      "sourceSize": { "w": 175, "h": 240 }
    }
  },
  "meta": {
      "app": "none",
      "version": "0.0",
      "image": "na",
      "format": "RGBA8888",
      "size": { "w": -1, "h": -1 },
      "scale": "1"
  }
}

var g_tile_viewer_info = {
  "pixi": {
    "app": {},
    "spritesheet": {},
    "tileset": {},
    "sprite_grid": [],
    "atlas": {}
  },

  "poll_ms": 100,

  "tiled_url": "data/snapshot.json",

  "view_scale": 0.125,
  "view_scale_factor": (1.0+(1/32)),

  "highlight_mode" : 0,
  "highlight_mode_n" : 3,
  "highlight_mode_name": [ "none", "cellSize", "replica" ],
  "highlight_mode_color": [ "#ffffff", "#c8c3e3", "#ffffff" ],

  "view_x": 0,
  "view_y": 0,

  "view_dx": 8,
  "view_dy": 8,

  "tilemap": {
    "size": [16,16],
    "data": []
  },

  "first_gid":1,

  //"tilesetURL": "img/rrti_tileset.png",
  "tilesetURL": "",
  "stride": [16,16],
  "data": {},

  "dx": 1,
  "dy": 1,
  "tx": 0,
  "ty": 0,

  "M" : [ [1, 0, 1], [0, 1, 1], [0, 0, 1] ],

  "ready": false,
  "resize_to_fit": true,

  "adaptive_fog": true,
  "fog_exponent" : 3,

  "socket": {}
};

function construct_atlas(template, texture) {

  let atlas = Object.assign({}, template);

  let W = texture.baseTexture.width;
  let H = texture.baseTexture.height;

  // make pixelated
  // gets rid of most of the stitching artifacts
  //
  texture.baseTexture.scaleMode = PIXI.SCALE_MODES.NEAREST;

  atlas.meta.size.w = W;
  atlas.meta.size.h = H;

  if (texture.textureCacheIds.length > 0) {
    atlas.meta.image = texture.textureCacheIds[0];
  }

  let stride = g_tile_viewer_info.stride;
  let frames = {};

  let base_id = 1;
  let cur_id = base_id;
  for (let h=0; h<H; h+=stride[1]) {
    for (let w=0; w<W; w+=stride[0]) {
      frames[ cur_id.toString() ] = {
        "frame": { "x": w, "y": h, "w": stride[0], "h": stride[1] },
        "rotated": false,
        "sourceSize": { "w": stride[0], "h": stride[1] }
      };
      cur_id++;
    }
  }

  // should be blank but just to put someting here for now
  //
  frames["0"] = {
    "frame": { "x": 0, "y": 0, "w": stride[0], "h": stride[1] },
    "rotated": false,
    "sourceSize": { "w": stride[0], "h": stride[1] }
  };

  atlas["frames"] = frames;

  return atlas;
}

//----
//----
//----

function irnd(a,b) {
  a = ((typeof a === "undefined") ? 0 : a);
  b = ((typeof b === "undefined") ? 2 : b);

  return Math.floor( Math.random()*(b-a) + a );
}

function init_pixi_tilemap() {
  let tm_sz = g_tile_viewer_info.tilemap.size;
  let stride = g_tile_viewer_info.stride;
  let data = g_tile_viewer_info.tilemap.data;

  let app = g_tile_viewer_info.pixi.app;

  let scale = g_tile_viewer_info.view_scale;

  let view_x = g_tile_viewer_info.view_x;
  let view_y = g_tile_viewer_info.view_y;

  g_tile_viewer_info.pixi.sprite_grid = [];
  let gr = g_tile_viewer_info.pixi.sprite_grid;
  for (let h=0; h<tm_sz[1]; h++) {
    for (let w=0; w<tm_sz[0]; w++) {
      let tile_id = data[ h*tm_sz[0] + w ];
      let sprite = new PIXI.Sprite(g_tile_viewer_info.pixi.spritesheet.textures[tile_id]);

      let world_x = stride[0]*w;
      let world_y = stride[1]*h;

      sprite.x = (world_x - view_x)*scale;
      sprite.y = (world_y - view_y)*scale;

      sprite.scale.x = scale;
      sprite.scale.y = scale;

      if (tile_id == 0) { sprite.visible = false; }
      else { sprite.visible = true; }

      gr.push(sprite);
      app.stage.addChild(sprite);
    }
  }

  g_tile_viewer_info.ready = true;
}

function resize_pixi_tilemap(sz) {
  sz = ((typeof sz === "undefined") ? g_tile_viewer_info.tilemap.size : sz);

  if (!g_tile_viewer_info.ready) { return; }

  let m = sz[0]*sz[1];
  let cur_size    = g_tile_viewer_info.tilemap.size;
  let stride      = g_tile_viewer_info.stride;
  let first_gid   = g_tile_viewer_info.first_gid;
  let spritesheet = g_tile_viewer_info.pixi.spritesheet;
  let app         = g_tile_viewer_info.pixi.app;

  let scale       = g_tile_viewer_info.view_scale;
  let view_x     = g_tile_viewer_info.view_x;
  let view_y     = g_tile_viewer_info.view_y;

  if (typeof spritesheet === "undefined") { return; }
  if (!("textures" in spritesheet)) { return; }

  let gr = g_tile_viewer_info.pixi.sprite_grid;
  let n = gr.length;


  for (let ii=0; ii<n; ii++) {
    let sprite = gr[ii];
    let h = Math.floor(ii/sz[0]);
    let w = Math.floor(ii%sz[0]);

    let world_x = stride[0]*w;
    let world_y = stride[1]*h;

    sprite.x = (world_x - view_x)*scale;
    sprite.y = (world_y - view_y)*scale;

    sprite.scale.x = scale;
    sprite.scale.y = scale;

  }

  for (let ii=n; ii<m; ii++) {
    let sprite = new PIXI.Sprite(spritesheet.textures[first_gid]);

    let h = Math.floor(ii/sz[0]);
    let w = Math.floor(ii%sz[0]);

    sprite.x = w*stride[0]*scale;
    sprite.y = h*stride[1]*scale;

    sprite.scale.x = scale;
    sprite.scale.y = scale;

    gr.push(sprite);
    app.stage.addChild(sprite);
  }

  cur_size[0] = sz[0];
  cur_size[1] = sz[1];
}

function load_tileset(tex) {
  g_tile_viewer_info.pixi.tileset = tex;
  g_tile_viewer_info.pixi.atlas = construct_atlas(g_atlasTemplate, tex);

  g_tile_viewer_info.pixi.spritesheet = new PIXI.Spritesheet(
    tex,
    g_tile_viewer_info.pixi.atlas
  );

  g_tile_viewer_info.pixi.spritesheet.parse().then( (xx) => {
    init_pixi_tilemap();
  });
}

function update_pixi_tilemap(tiled_data) {
  if (!("tilesets" in tiled_data)) { return; }
  if (tiled_data.tilesets.length == 0) { return; }

  let tileset_url = "img/" + tiled_data.tilesets[0].image;
  let tileset_stride = [ tiled_data.tilesets[0].tilewidth, tiled_data.tilesets[0].tileheight ];

  g_tile_viewer_info.stride[0] = tileset_stride[0];
  g_tile_viewer_info.stride[1] = tileset_stride[1];

  if (tileset_url != g_tile_viewer_info.tilesetURL) {

    PIXI.utils.clearTextureCache();

    g_tile_viewer_info.tilesetURL = tileset_url;
    PIXI.Assets.load( tileset_url ).then( (tex) => {
      load_tileset(tex);

      g_tile_viewer_info.resize_to_fit=true;
      update_pixi_tilemap(tiled_data);
    });

    // redo until tileset image has been loaded
    //
    setTimeout(
      (function(_x) {
        return function() { update_pixi_tilemap(_x); };
      })(tiled_data),
      1000 );

    return;
  }

  if (!(g_tile_viewer_info.ready)) { return; }

  let cur_size = g_tile_viewer_info.tilemap.size;
  let stride = g_tile_viewer_info.stride;
  let data = g_tile_viewer_info.tilemap.data;

  let app = g_tile_viewer_info.pixi.app;
  let spritesheet = g_tile_viewer_info.pixi.spritesheet.textures;

  if (typeof spritesheet === "undefined") { return; }


  let sprite = {};
  let gr = g_tile_viewer_info.pixi.sprite_grid;

  if ( (!("layers" in tiled_data)) ||
       (tiled_data.layers.length == 0) ) { return; }

  let tm_gr = tiled_data.layers[0].data;
  let fog_gr = undefined;
  let match_gr = undefined;

  let max_tile = tiled_data.tilesets[0].tilecount;
  if (max_tile < 2) { max_tile = 2; }

  if (g_tile_viewer_info.highlight_mode == 1) {

    if (tiled_data.layers.length > 1) {
      for (let ii=1; ii<tiled_data.layers.length; ii++) {
        if (tiled_data.layers[ii].name == g_tile_viewer_info.highlight_mode_name[1]) {
          fog_gr = tiled_data.layers[ii].data;
          break;
        }
      }
    }

  }
  else if (g_tile_viewer_info.highlight_mode == 2) {

    if (tiled_data.layers.length > 1) {
      for (let ii=1; ii<tiled_data.layers.length; ii++) {
        if (tiled_data.layers[ii].name == g_tile_viewer_info.highlight_mode_name[2]) {
          match_gr = tiled_data.layers[ii].data;
          break;
        }
      }
    }

  }

  let tiled_size = [ -1, -1 ];
  if ("width" in tiled_data) { tiled_size[0] = tiled_data.width; }
  if ("height" in tiled_data) { tiled_size[1] = tiled_data.height; }
  if ((tiled_size[0]<=0) || (tiled_size[1]<=0)) { return; }

  if ((tiled_size[0] != cur_size[0]) ||
      (tiled_size[1] != cur_size[1])) {

    resize_pixi_tilemap(tiled_size);

    //paranoia
    //
    cur_size = g_tile_viewer_info.tilemap.size;
  }

  if (g_tile_viewer_info.resize_to_fit) {
    let _wx = stride[0]*(tiled_size[0]+1);
    let _wy = stride[1]*(tiled_size[1]+1);


    let _winy = window.innerHeight;
    let _vs = _winy / (1.125 * _wy);

    let _ds = _vs / g_tile_viewer_info.view_scale;
    g_tile_viewer_info.view_scale *= _ds;
    g_tile_viewer_info.view_x *= _ds;
    g_tile_viewer_info.view_y *= _ds;

    g_tile_viewer_info.resize_to_fit = false;

    resize_pixi_tilemap(tiled_size);
  }

  let fog_max = max_tile-1;
  let adaptive_fog_opt = g_tile_viewer_info.adaptive_fog;

  if (fog_max < 1) { fog_max = 1; }

  if (adaptive_fog_opt) {
    fog_max = 1;
    for (let h=0; h<cur_size[1]; h++) {
      for (let w=0; w<cur_size[0]; w++) {
        let idx = w + (h*cur_size[0]);
        if (idx >= tm_gr.length) { continue; }
        if ( (typeof fog_gr !== "undefined") &&
             (idx < fog_gr.length) &&
             (fog_gr[idx] > 0) ) {

          if (fog_max < fog_gr[idx]) {
            fog_max = fog_gr[idx];
          }
        }
      }
    }
  }

  for (let h=0; h<cur_size[1]; h++) {
    for (let w=0; w<cur_size[0]; w++) {

      let idx = w + (h*cur_size[0]);
      if (idx >= tm_gr.length) { continue; }

      let tile_id = tm_gr[idx];
      if (!(tile_id in spritesheet)) { continue; }

      sprite = gr[ h*cur_size[0] + w ];
      sprite.texture = spritesheet[tile_id];

      sprite.alpha = 1.0;

      if (tile_id == 0) {
        sprite.visible = false;
        continue;
      }


      if ( (typeof fog_gr !== "undefined") &&
           (idx < fog_gr.length) &&
           (fog_gr[idx] > 0) ) {

        let a = 1.0 - ((fog_gr[idx] - 1) / (fog_max) );
        a = Math.pow( a, g_tile_viewer_info.fog_exponent );
        sprite.alpha = a;
      }

      else if ( (typeof match_gr !== "undefined") &&
                (idx < match_gr.length) ) {
        sprite.alpha = ((match_gr[idx] > 0.5) ? 1.0 : 0.5);
      }

      sprite.visible = true;
    }
  }

  for (let ii=(cur_size[0]*cur_size[1]); ii<gr.length; ii++) {
    gr[ii].visible = false;
  }

}

// main entry point for new tilemap data.
// Try and catch the parse as a minimal attempt
// at mitigating bad data parsing or other garbage coming in.
// This way at least the application has a fighting chance
// of recovering on bad data.
//
/*
function ws_message(ev) {
  let _data = {};

  try { _data = JSON.parse(ev.data); }
  catch (_e) { console.log("parse error:", _e); }

  if (("type" in _data) && (_data.type == "tiled.json")) {
    if ("data" in _data) {
      update_pixi_tilemap(_data.data);
    }
  }
}

function init_ws() {
  var ws = new WebSocket(WEBSOCKET_URL + ":" + WEBSOCKET_PORT);
  ws.onmessage = ws_message;
}
*/

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


function poll_tiled() {
  let url = g_tile_viewer_info.tiled_url;
  url += "?t=" + Date.now().toString();

  fetchJSON( url, function(data) {
    update_pixi_tilemap(data);
  });

  setTimeout( poll_tiled, g_tile_viewer_info.poll_ms );
}

function _keyup(ev) { return false; }
function _keydown(ev) {
  let _sf = g_tile_viewer_info.view_scale_factor;
  let _dx = g_tile_viewer_info.view_dx;
  let _dy = g_tile_viewer_info.view_dy;

  let _redraw = true;

  let _reverse_control = false;

  if      (ev.key == 'ArrowDown') {

    if (ev.shiftKey) {
      g_tile_viewer_info.view_scale /= _sf;
      g_tile_viewer_info.view_dx /= _sf;
      g_tile_viewer_info.view_dy /= _sf;
    }
    else {

      if (_reverse_control) {
        g_tile_viewer_info.view_y += g_tile_viewer_info.view_dy;
      }
      else {
        g_tile_viewer_info.view_y -= g_tile_viewer_info.view_dy;
      }

    }
    ev.preventDefault();

  }
  else if (ev.key == 'ArrowUp') {

    if (ev.shiftKey) {
      g_tile_viewer_info.view_scale *= _sf;
      g_tile_viewer_info.view_dx *= _sf;
      g_tile_viewer_info.view_dy *= _sf;
    }
    else {

      if (_reverse_control) {
        g_tile_viewer_info.view_y += g_tile_viewer_info.view_dy;
      }
      else {
        g_tile_viewer_info.view_y += g_tile_viewer_info.view_dy;
      }

    }
    ev.preventDefault();

  }
  else if (ev.key == 'ArrowLeft') {
    if (ev.shiftKey) {
    }
    else {
      if (_reverse_control) {
        g_tile_viewer_info.view_x += g_tile_viewer_info.view_dx;
      }
      else {
        g_tile_viewer_info.view_x -= g_tile_viewer_info.view_dx;
      }
    }
    ev.preventDefault();
  }
  else if (ev.key == 'ArrowRight') {
    if (ev.shiftKey) {
    }
    else {
      if (_reverse_control) {
        g_tile_viewer_info.view_x -= g_tile_viewer_info.view_dx;
      }
      else {
        g_tile_viewer_info.view_x += g_tile_viewer_info.view_dx;
      }
    }
    ev.preventDefault();
  }

  else if ((ev.key == 'e') ||
           (ev.key == '=')) {
    g_tile_viewer_info.view_scale *= _sf;
    g_tile_viewer_info.view_dx *= _sf;
    g_tile_viewer_info.view_dy *= _sf;
    ev.preventDefault();
  }
  else if ((ev.key == 'q') ||
           (ev.key == '-')) {
    g_tile_viewer_info.view_scale /= _sf;
    g_tile_viewer_info.view_dx /= _sf;
    g_tile_viewer_info.view_dy /= _sf;
    ev.preventDefault();
  }
  else if ((ev.key == 'a') ||
           (ev.key == 'h')) {
    g_tile_viewer_info.view_x += g_tile_viewer_info.view_dx;
  }
  else if ((ev.key == 's') ||
           (ev.key == 'j'))  {
    g_tile_viewer_info.view_y -= g_tile_viewer_info.view_dy;
  }
  else if ((ev.key == 'd') ||
           (ev.key == 'l')) {
    g_tile_viewer_info.view_x -= g_tile_viewer_info.view_dx;
  }
  else if ((ev.key == 'w') ||
           (ev.key == 'k')) {
    g_tile_viewer_info.view_y += g_tile_viewer_info.view_dy;
  }

  else if ((ev.key == ',') ||
           (ev.key == '<')) {
    g_tile_viewer_info.highlight_mode += g_tile_viewer_info.highlight_mode_n-1;
    g_tile_viewer_info.highlight_mode %= g_tile_viewer_info.highlight_mode_n;

    let _mode = g_tile_viewer_info.highlight_mode;
    let _colora = g_tile_viewer_info.highlight_mode_color;
    let _renderer = g_tile_viewer_info.pixi.app.renderer;
    _renderer.background.color = _colora[ _mode ];
  }
  else if ((ev.key == '.') ||
           (ev.key == '>')) {
    g_tile_viewer_info.highlight_mode += 1;
    g_tile_viewer_info.highlight_mode %= g_tile_viewer_info.highlight_mode_n;

    let _mode = g_tile_viewer_info.highlight_mode;
    let _colora = g_tile_viewer_info.highlight_mode_color;
    let _renderer = g_tile_viewer_info.pixi.app.renderer;
    _renderer.background.color = _colora[ _mode ];
  }

  else if ((ev.key == '[') ||
           (ev.key == '{')) {
    g_tile_viewer_info.fog_exponent -= 1;
    if (g_tile_viewer_info.fog_exponent < 1) {
      g_tile_viewer_info.fog_exponent = 1;
    }
  }
  else if ((ev.key == ']') ||
           (ev.key == '}')) {
    g_tile_viewer_info.fog_exponent += 1;
    if (g_tile_viewer_info.fog_exponent > 5) {
      g_tile_viewer_info.fog_exponent = 5;
    }
  }

  else if ((ev.key == 'f') ||
           (ev.key == 'F')) {
    g_tile_viewer_info.adaptive_fog = !g_tile_viewer_info.adaptive_fog;
  }

  else {
    _redraw = false;
  }

  if (_redraw) {
    resize_pixi_tilemap();
  }

  return false;
}

function setup_keyboard(ui_ele) {
  ui_ele = ((typeof ui_ele === "undefined") ? window : ui_ele);
  ui_ele.addEventListener("keydown", _keydown, false);
  ui_ele.addEventListener("keyup", _keyup, false);
}

function mouse_move(ev) {
  let x = ev.screen.x;
  let y = ev.screen.y;


  if (!(g_tile_viewer_info.ready)) { return; }


  //console.log("mm:", x, y);
}

function init() {

  //init_ws();
  poll_tiled();

  let tm_sz = g_tile_viewer_info.tilemap.size;
  let data = [];

  for (let h=0; h<tm_sz[1]; h++) {
    for (let w=0; w<tm_sz[0]; w++) {
      data.push(irnd(1,10));
    }
  }
  g_tile_viewer_info.tilemap.data = data;

  //---

  //let app = new PIXI.Application({ background: '#7f7f7f', resizeTo: window });
  let app = new PIXI.Application({ background: '#ffffff', resizeTo: window });
  document.body.appendChild(app.view);

  app.stage.eventMode = 'static';
  app.stage.hitArea = app.screen;
  app.stage.addEventListener('pointermove', mouse_move);

  g_tile_viewer_info.pixi.app = app;

  setup_keyboard();
}


