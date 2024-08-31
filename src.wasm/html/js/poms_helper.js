/*
 * To the extent possible under law, the person who associated CC0 with
 * this file has waived all copyright and related or neighboring rights
 * to this file.
 *
 * You should have received a copy of the CC0 legalcode along with this
 * work. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 *
 */

var g_ui_info = {
  "app": null,
  "gfx": null,
  "init":false,
  "snapshot_fn" : "",
  "worker": null
};

//----
//----
//----

function worker_update(e) {
  var txt = e.data;
  var data = JSON.parse(txt);

  update_pixi_tilemap(data);
}


function start_run() {
  let tileset_name = ui_getSelect("ui_tileset");

  console.log(tileset_name);

  let poms_cfg = g_poms_default[tileset_name]["-C"];
  let tiled_fn = g_poms_default[tileset_name]["-1"];
  let snapshot_fn = g_poms_default[tileset_name]["-8"];

  console.log(poms_cfg, tiled_fn, snapshot_fn);

  let _argv = [];
  let _default_argmap = g_poms_default[tileset_name];
  for (let opt_key in _default_argmap) {

    if (opt_key.length == 2) {
      _argv.push(opt_key);
      _argv.push(_default_argmap[opt_key]);
    }
    else if (opt_key.length > 2) {
      let key_parts = opt_key.split(" ");
      let param = key_parts[0];
      let val = key_parts[1] + "=" + _default_argmap[opt_key];

      _argv.push(param);
      _argv.push(val);
    }
  }

  g_ui_info.snapshot_fn = g_poms_default[tileset_name]["-8"];

  let msg = {
    "type":"run",
    "argv": _argv
  };

  g_ui_info.worker.postMessage(msg);
}

//
//  _   _ ___ 
// | | | |_ _|
// | |_| || | 
//  \___/|___|
//            
function ui_getSelect(_id) {
  let ele = document.getElementById(_id);
  let idx = ele.selectedIndex;
  return ele.options[idx].value;
}

function ui_setSelect(_id, v) {
  let ele = document.getElementById(_id);

  for (let ii=0; ii<ele.options.length; ii++) {
    if (ele.options[ii].value == v) {
      ele.selectedIndex = ii;
    }
  }

}

function ui_getNum(_id) {
  let ele = document.getElementById(_id);
  return ele.value;
}

function ui_setNum(_id, v) {
  let ele = document.getElementById(_id);
  ele.value = v;
}

function ui_getStr(_id) {
  let ele = document.getElementById(_id);
  return ele.value;
}

function ui_setStr(_id, v) {
  let ele = document.getElementById(_id);
  ele.value = v;
}

function ui_populate_defaults(tileset_name) {
  let _default = g_poms_default[tileset_name];

  console.log("??", tileset_name, _default);

  let blocksize = _default["-s"].split(",");
  ui_setNum("ui_blocksize_x", blocksize[0]);
  ui_setNum("ui_blocksize_y", blocksize[1]);

  let gridsize = _default["-q"].split(",");
  ui_setNum("ui_gridsize_x", gridsize[0]);
  ui_setNum("ui_gridsize_y", gridsize[1]);

  let softensize = _default["-B"].split(":");
  let soften_start_a = softensize[0].split(",");
  let soften_end_a = softensize[0].split(",");
  if (softensize.length > 1) { soften_end_a = softensize[1].split(","); }
  ui_setNum("ui_softensize_x_start", soften_start_a[0]);
  ui_setNum("ui_softensize_y_start", (soften_start_a.length == 1) ? soften_start_a[0] : soften_start_a[1] );
  ui_setNum("ui_softensize_x_end", soften_end_a[0]);
  ui_setNum("ui_softensize_y_end", (soften_end_a.length == 1) ? soften_end_a[0] : soften_end_a[1] );

  let maxiter = _default["-J"];
  ui_setNum("ui_maxiter", maxiter);

  let rand_w = _default["-w"];
  ui_setNum("ui_rand_w", rand_w);

  let rand_E = _default["-E"];
  ui_setNum("ui_rand_E", rand_E);

  let seed = _default["-S"];
  ui_setNum("ui_seed", seed);

  let cell_policy = _default["-P"];
  ui_setSelect("ui_cell_choice_policy", cell_policy);

  // whoops
  //
  let viz_step = _default["-O viz_step"];
  ui_setNum("ui_vizstep", viz_step);

  let block_choice_policy = _default["-O patch-policy"];
  ui_setSelect("ui_block_choice_policy", block_choice_policy);

}

function setup_ui_callbacks() {

  document.getElementById("ui_run").addEventListener("click",
    function(e) {
      start_run();
      //example_run_worker();
    });

  document.getElementById("ui_tileset").addEventListener("change",
    function(e) {
      let v = ui_getSelect("ui_tileset");

      console.log("??", v);

      ui_populate_defaults(v);
    });

  document.getElementById("ui_option").addEventListener("click",
    function(e) {
      let _d = document.getElementById("ui_dialog");
      _d.open = true;
    });

  document.getElementById("ui_option_accept").addEventListener("click",
    function(e) {
      let _d = document.getElementById("ui_dialog");
      _d.open = false;
    });

  document.getElementById("ui_option_cancel").addEventListener("click",
    function(e) {
      let _d = document.getElementById("ui_dialog");
      _d.open = false;
    });

}

function poms_web_init() {
  setup_ui_callbacks();

  let app = new PIXI.Application({ background: '#ffffff', resizeTo: window });
  let ele = document.getElementById("ui_gfx");
  ele.innerHTML = '';
  ele.appendChild(app.view);

  app.stage.eventMode = 'static';
  app.stage.hitArea = app.screen;
  app.stage.addEventListener('pointermove', mouse_move);

  g_tile_viewer_info.pixi.app = app;
  g_ui_info.app = app;
  g_ui_info.gx = ele;

  setup_keyboard();

  g_ui_info.worker = new Worker("js/poms_worker.js");
  g_ui_info.worker.onmessage = worker_update;
}

