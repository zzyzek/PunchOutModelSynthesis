// LICENSE: CC0
//

var g_app = {
  "app" : {},
  "supertile_id":-1,
  "cur_tile": {"x":-1, "y":-1},
  "mouse": {"x":-1, "y":-1},
  "tileset_sprite": []
};

function pathTranslate(v, p, x, y) {
  for (let ii=0; ii<p.length; ii+=2) {
    v[ii+0] = p[ii+0] + x;
    v[ii+1] = p[ii+1] + y;
  }
}

function _pathTranslate(p, x, y) {
  let v = [];
  for (let ii=0; ii<p.length; ii+=2) {
    v[ii+0] = p[ii+0] + x;
    v[ii+1] = p[ii+1] + y;
  }
  return v;
}


function draw_rects_exemplar() {
  draw_rects_et(g_poms, "exemplar");
}

function draw_rects_tileset() {
  draw_rects_et(g_poms, "tileset");
}

function draw_rects_supertile() {
  draw_rects_et(g_poms, "supertile");
}


// super function for exemplar, tileset or supertile
// (et == exemplar/tileset?)
//
function draw_rects_et(poms, mouse_source) {

  poms = g_poms;

  let sx = poms.stride[0];
  let sy = poms.stride[1];
  let s = sx;

  let f = 1/4;
  let fx = f;
  let fy = f;

  let hl_x = Math.floor(g_app.mouse.x/s)*s;
  let hl_y = Math.floor(g_app.mouse.y/s)*s;

  if ((hl_x == g_app.cur_tile.x) &&
      (hl_y == g_app.cur_tile.y)) { return; }

  let gfxe = g_app.graphics_exemplar;
  gfxe.clear();

  let gfxt = g_app.graphics_tileset;
  gfxt.clear();

  let color = [
    0xfe3f3f,
    0xffffff,
    0x000000
  ];

  color = [
    0x494e4c,
    0xb7806c,
    0xf3f1e4,
    0x99c7c1
  ];

  color = [ 0x99c7c1 ];

  let path = [
    [ 0,fy*sy, fx*sx,fy*sy, sx/2,sy/2, fx*sx,(1-fy)*sy, 0,(1-fy)*sy ],
    [ sx,fy*sy, (1-fx)*sx,fy*sy, sx/2,sy/2, (1-fx)*sx,(1-fy)*sy, sx,(1-fy)*sy],
    [ fx*sx,0, fx*sx,fy*sy, sx/2,sy/2, (1-fx)*sx,fy*sy, (1-fx)*sx,0 ],
    [ fx*sx,sy, (1-fx)*sx,sy, (1-fx)*sx,(1-fy)*sy, sx/2,sy/2, fx*sx,(1-fy)*sy ]
  ];

  let mp = [ g_app.mouse.x, g_app.mouse.y ];

  let tilepos_xy = [ Math.floor(mp[0]/sx), Math.floor(mp[1]/sy) ];
  let img_floor_pos_xy = [ Math.floor(mp[0]/sx)*sx, Math.floor(mp[1]/sy)*sy ];

  hl_x = tilepos_xy[0] ;
  hl_y = tilepos_xy[1] ;

  let rect_info = [];

  let nei_map = g_app.poms_nei_info.tile_nei_list;
  let map_bp = g_app.poms_nei_info.tile_map_idx;

  let map_w = poms.map_w;
  let map_h = poms.map_h;

  let tileset_w = poms.tileset_size[0] / poms.stride[0];
  let tileset_h = poms.tileset_size[1] / poms.stride[1];

  let src_tile = -1;
  let first_gid = 1;

  let map_hl_tile = [];

  if (mouse_source == "exemplar") {
    let idx = tilepos_xy[1]*poms.map_w + tilepos_xy[0];
    if ((idx<0) || (idx >= poms.map_array.length)) { return; }

    src_tile = poms.map_array[idx];
  }

  else if (mouse_source == "tileset") {
    let idx = tilepos_xy[1]*poms.tileset_width + tilepos_xy[0];
    if ((idx<0) || (idx >= (poms.tile_name.length-1))) { return; }

    src_tile = idx+1;
  }

  else if (mouse_source == "supertile") {
    src_tile = g_app.supertile_id;
    if ((src_tile<0) || (src_tile >= (poms.tile_name.length-1))) { return; }
  }

  if (src_tile < 0) { console.log("SANITY: src_tile:", src_tile); return; }

  for (let idir=0; idir<4; idir++) {

    let nei_list  = nei_map[idir][src_tile];

    for (let nei_idx=0; nei_idx<nei_list.length; nei_idx++) {
      let nei_tile = nei_list[nei_idx];

      // highlight neighboring tiles in exemplar image
      //
      let pos_idx_a = map_bp[nei_tile];
      for (let pos_idx=0; pos_idx<pos_idx_a.length; pos_idx++) {
        let map_idx = pos_idx_a[pos_idx];

        let map_x = map_idx % map_w;
        let map_y = Math.floor(map_idx / map_w);

        let map_pxl_x = (map_x*sx) ;
        let map_pxl_y = (map_y*sy) ;

        let p = _pathTranslate(path[idir], map_pxl_x, map_pxl_y);

        let cidx = 0;

        gfxe.lineStyle(0);
        gfxe.beginFill(color[cidx], 0.85);
        gfxe.drawPolygon(p);
        gfxe.endFill();

        gfxe.lineStyle(1, color[cidx], 0.5);
        gfxe.beginFill(color[cidx], 0.25);
        gfxe.drawRect(map_pxl_x,map_pxl_y,sx,sy);
        gfxe.endFill();

      }

      // hieghlight neighboring tiles in tileset image
      //
      let tileset_x = (nei_tile-first_gid) % tileset_w;
      let tileset_y = Math.floor((nei_tile-first_gid) / tileset_w);

      let tileset_pxl_x = (tileset_x*sx) ;
      let tileset_pxl_y = (tileset_y*sy) ;

      let p = _pathTranslate(path[idir], tileset_pxl_x, tileset_pxl_y);

      let cidx = 0;

      gfxt.lineStyle(0);
      gfxt.beginFill(color[cidx], 0.85);
      gfxt.drawPolygon(p);
      gfxt.endFill();

      gfxt.lineStyle(1, color[cidx], 0.5);
      gfxt.beginFill(color[cidx], 0.25);
      gfxt.drawRect(tileset_pxl_x,tileset_pxl_y,sx,sy);
      gfxt.endFill();

    }
  }

  let c = 0xf3f1e4; 
  c = 0xd46a6a;

  for (let ii=0; ii<map_bp[src_tile].length; ii++) {

    let map_idx = map_bp[src_tile][ii];

    let map_x = map_idx % map_w;
    let map_y = Math.floor(map_idx / map_w);

    let map_pxl_x = (map_x*sx) ;
    let map_pxl_y = (map_y*sy) ;

    gfxe.lineStyle(3, c, 0.95);
    gfxe.beginFill(0x000000, 0.0);
    gfxe.drawRect(map_pxl_x,map_pxl_y,sx,sy);
    gfxe.endFill();

  }

  let hl_tileset_x = (src_tile-first_gid) % tileset_w;
  let hl_tileset_y = Math.floor((src_tile-first_gid) / tileset_w);

  let hl_tileset_pxl_x = (hl_tileset_x*sx) ;
  let hl_tileset_pxl_y = (hl_tileset_y*sy) ;

  gfxt.lineStyle(3, c, 0.95);
  gfxt.beginFill(0x000000, 0.0);
  gfxt.drawRect(hl_tileset_pxl_x,hl_tileset_pxl_y,sx,sy);
  gfxt.endFill();

}

function _build_poms_nei_info(poms) {
  let _nei_info = {};

  _nei_info["tile_nei_list"] = [ [], [], [], [], [], [] ];
  _nei_info["tile_map_idx"] = [];

  let _seen = [ {}, {}, {}, {}, {}, {} ];

  for (let tile=0; tile<poms.tile_name.length; tile++) {
    for (let idir=0; idir<6; idir++) {
      _nei_info["tile_nei_list"][idir].push([]);
      _seen[idir][tile] = {};
    }
    _nei_info.tile_map_idx.push([]);
  }

  for (let ii=0; ii<poms.map_array.length; ii++) {
    let tile = poms.map_array[ii];
    _nei_info.tile_map_idx[tile].push(ii);
  }


  for (let rule_idx=0; rule_idx<poms.tile_rule.length; rule_idx++) {
    let src_tile = poms.tile_rule[rule_idx][0];
    let dst_tile = poms.tile_rule[rule_idx][1];
    let idir = poms.tile_rule[rule_idx][2];
    let val = poms.tile_rule[rule_idx][3];

    let rdir = poms.opposite_dir[idir];

    if (!(dst_tile in _seen[idir][src_tile])) {
      _seen[idir][src_tile][dst_tile] = 1;
      _nei_info.tile_nei_list[idir][src_tile].push(dst_tile);
    }

    if (!(src_tile in _seen[rdir][dst_tile])) {
      _seen[rdir][dst_tile][src_tile] = 1;
      _nei_info.tile_nei_list[rdir][dst_tile].push(src_tile);
    }

  }

  return _nei_info;
}

// called once on load, creates the HTML elements
// and initial pixi application/stage/whatever
// for the exemplar image and tileset
//
function pixi_init() {

  let app_exemplar = new PIXI.Application({
    //"antialias": true,
    "antialias": false,
    "background": '#dfdfdf',
    "width": 256,
    "height": 256
  });

  let app_tileset = new PIXI.Application({
    //"antialias": true,
    "antialias": false,
    "background": '#dfdfdf',
    "width": 256,
    "height": 256
  });

  let poms = g_poms;

  g_app.app_exemplar = app_exemplar;
  g_app.app_tileset = app_tileset;

  //----
  let html_location = document.getElementById("ui_pixi_location");

  let _row = document.createElement("div");
  _row.classList.add("row");

  let _col0 = document.createElement("div");
  _col0.classList.add("six","columns");
  _col0.id = 'ui_exemplar_container';
  _col0.style["max-height"] = "486px";
  _col0.style["max-width"] = "486px";
  _col0.style["overflow"] = "scroll";

  let _col1 = document.createElement("div");
  _col1.classList.add("six","columns");
  _col1.id = 'ui_tileset_container';
  _col1.style["max-height"] = "486px";
  _col1.style["max-width"] = "486px";
  _col1.style["overflow"] = "scroll";


  _col0.appendChild(app_exemplar.view);
  _col1.appendChild(app_tileset.view);
  _row.appendChild(_col0);
  _row.appendChild(_col1);
  html_location.appendChild(_row);



  //html_location.appendChild(app_exemplar.view);
  //html_location.appendChild(app_tileset.view);
  //----

  app_exemplar.stage.eventMode = 'static';
  app_tileset.stage.eventMode = 'static';

  app_exemplar.stage.hitArea = app_exemplar.screen;
  app_tileset.stage.hitArea = app_tileset.screen;

  app_exemplar.stage.addEventListener('pointermove', (e) => {
    g_app.pixi_win = "exemplar";
    g_app.mouse.x = e.screen.x;
    g_app.mouse.y = e.screen.y;
    draw_rects_exemplar(poms);
  });

  app_tileset.stage.addEventListener('pointermove', (e) => {
    g_app.pixi_win = "tileset";
    g_app.mouse.x = e.screen.x;
    g_app.mouse.y = e.screen.y;
    draw_rects_tileset(poms);
  });

  g_app.graphics_exemplar = new PIXI.Graphics();
  app_exemplar.stage.addChild(g_app.graphics_exemplar);

  g_app.graphics_tileset = new PIXI.Graphics();
  app_tileset.stage.addChild(g_app.graphics_tileset);

}

// loads the exemplar canvas and tileset canvas pixi app
// by removing the previously allocated pxi app/stage/whatever
// and replacing it with the new one.
//
function pixi_load(exemplar_img, tileset_img, poms) {

  if (g_app.tileset_sprite.length > 0) {
    g_app.app_exemplar.stage.removeChild( g_app.tileset_sprite[0] );
    g_app.app_tileset.stage.removeChild( g_app.tileset_sprite[1] );

    g_app.tileset_sprite[0].destroy();
    g_app.tileset_sprite[1].destroy();
  }

  let tileset_sprite = [];

  tileset_sprite.push( PIXI.Sprite.from(exemplar_img) );
  tileset_sprite.push( PIXI.Sprite.from(tileset_img) );

  tileset_sprite[0].x = 0;
  tileset_sprite[0].y = 0;

  tileset_sprite[1].x = 0;
  tileset_sprite[1].y = 0;

  g_app.app_exemplar.stage.addChild(tileset_sprite[0]);
  g_app.app_tileset.stage.addChild(tileset_sprite[1]);


  g_app.tileset_sprite = tileset_sprite;

  // append to g_tileset
  //
  //g_tileset.push( tileset_sprite[0] );
  //g_tileset.push( tileset_sprite[1] );

  g_app.poms_nei_info = _build_poms_nei_info(poms);

  g_app.app_exemplar.stage.removeChild( g_app.graphics_exemplar );
  g_app.graphics_exemplar = new PIXI.Graphics();
  g_app.app_exemplar.stage.addChild(g_app.graphics_exemplar);

  g_app.app_tileset.stage.removeChild( g_app.graphics_tileset );
  g_app.graphics_tileset = new PIXI.Graphics();
  g_app.app_tileset.stage.addChild(g_app.graphics_tileset);


  g_app.app_exemplar.renderer.resize( exemplar_img.width, exemplar_img.height );
  g_app.app_tileset.renderer.resize( tileset_img.width, tileset_img.height );


}

