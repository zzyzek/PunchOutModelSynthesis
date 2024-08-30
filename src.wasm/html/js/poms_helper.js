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
  "worker": null
};

var g_tileset_default = {
  "pillMortal" : {
    "-C" : "data/pillMortal_poms.json",
    "-s": "48,48,1",
    "-q": "64,64,1",
    "-b": "1",
    "-B": "8,8,1",
    "-J":"10000",
    "-w": "1.0",
    "-E": "-1.95",
    "-1": "pillMortal_tiled.json",
    "-8": "pillMortal_snapshot.json",
    "-P": "min",
    "-O": "viz_step=50",
    "-O": "patch-policy=pending",
    "-S": "1337",
    "-V": "1"
  },
  "oarpgo" : {
    "-C" : "data/oarpgo_poms.json",
    "-s": "50,70,1",
    "-q": "256,256,1",
    "-b": "1",
    "-B": "12:24",
    "-J":"10000",
    "-w": "1.5",
    "-E": "-1.75",
    "-1": "oarpgo_tiled.json",
    "-8": "oarpgo_snapshot.json",
    "-P": "min",
    "-O": "viz_step=50",
    "-O": "patch-policy=cone-",
    "-S": "1337",
    "-V": "1"

  },
  "rrti": {}
};


function example_run() {

  // pillMortal_poms.json was preloaded/baked into the
  // emscripten port, so we have a copy in data/ but
  // that's not the copy referenced below
  //
  let argv = [
    "foo",
    "-C", "data/pillMortal_poms.json",
    "-s", "48,48,1",
    "-q", "64,64,1",
    "-b", "1",
    "-B", "8,8,1",
    "-J", "10000",
    "-w", "1.0",
    "-E", "-1.95",
    "-1", "pillMortal_64x64.json",
    "-8", "pillMortal_snapshot.json",
    "-P", "min",
    "-O", "viz_step=50",
    "-O", "patch-policy=pending",
    "-S", "1337",
    "-V", "1"
  ];
  main_like( Module._main, argv );

  let json_txt = new TextDecoder().decode( FS.readFile("pillMortal_64x64.json") );

  let out_json = JSON.parse(json_txt);


  //EXPERIMENTAL
  //EXPERIMENTAL

  update_pixi_tilemap( out_json );

  //let app = new PIXI.Application({ background: '#7f7f7f', resizeTo: window });
  let app = new PIXI.Application({ background: '#ffffff', resizeTo: window });
  document.body.appendChild(app.view);

  app.stage.eventMode = 'static';
  app.stage.hitArea = app.screen;
  app.stage.addEventListener('pointermove', mouse_move);

  g_tile_viewer_info.pixi.app = app;

  setup_keyboard();

  //EXPERIMENTAL
  //EXPERIMENTAL

  return out_json;
}

//----
//----
//----

function worker_update(e) {
  var txt = e.data;
  var data = JSON.parse(txt);

  update_pixi_tilemap(data);
}

//----
//----
//----

function example_run_worker() {
  //let app = new PIXI.Application({ background: '#7f7f7f', resizeTo: window });
  let app = new PIXI.Application({ background: '#ffffff', resizeTo: window });
  //document.body.appendChild(app.view);

  let ele = document.getElementById("ui_gfx");
  ele.innerHTML = '';
  ele.appendChild(app.view);

  app.stage.eventMode = 'static';
  app.stage.hitArea = app.screen;
  app.stage.addEventListener('pointermove', mouse_move);

  g_tile_viewer_info.pixi.app = app;

  setup_keyboard();


  g_ui_info.worker = new Worker("js/poms_worker.js");
  g_ui_info.worker.onmessage = worker_update;
  g_ui_info.worker.postMessage("start");
}

function start_poms_worker() {

  let msg = {
    "type":"run",
    "argv": []
  };

  let pm_argv = [
    "-C", "data/pillMortal_poms.json",
    "-s", "48,48,1",
    "-q", "64,64,1",
    "-b", "1",
    "-B", "8,8,1",
    "-J", "10000",
    "-w", "1.0",
    "-E", "-1.95",
    "-1", "pillMortal_tiled.json",
    "-8", "pillMortal_snapshot.json",
    "-P", "min",
    "-O", "viz_step=50",
    "-O", "patch-policy=pending",
    "-S", "1337",
    "-V", "1"
  ];

  msg.argv = pm_argv;

  g_ui_info.worker.postMessage(msg);
}

function ui_getSelect(_id) {
  let ele = document.getElementById(_id);
  let idx = ele.selectedIndex;
  return ele.options[idx].value;
}

function ui_getNum(_id) {
  let ele = document.getElementById(_id);
  return ele.value;
}

function ui_getStr(_id) {
  let ele = document.getElementById(_id);
  return ele.value;
}


function start_run() {
  let tileset_name = ui_getSelect("ui_tileset");

  console.log(tileset_name);

  let poms_cfg = g_tileset_default[tileset_name]["-C"];
  let tiled_fn = g_tileset_default[tileset_name]["-1"];
  let snapshot_fn = g_tileset_default[tileset_name]["-8"];

  console.log(poms_cfg, tiled_fn, snapshot_fn);


}

//
//  _   _ ___ 
// | | | |_ _|
// | |_| || | 
//  \___/|___|
//            

function setup_ui_callbacks() {

  document.getElementById("ui_run").addEventListener("click",
    function(e) {
      start_run();
      //example_run_worker();
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

