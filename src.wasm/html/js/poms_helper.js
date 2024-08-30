/*
 * To the extent possible under law, the person who associated CC0 with
 * this file has waived all copyright and related or neighboring rights
 * to this file.
 *
 * You should have received a copy of the CC0 legalcode along with this
 * work. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 *
 */

//
// https://stackoverflow.com/a/70267473/4002265
// convert a Javascript string to a C string
//
function string2charp(s) {
  var size = lengthBytesUTF8(s) + 1;
  var ptr = _malloc(size);
  stringToUTF8Array(s, HEAP8, ptr, size);
  return ptr;
}

function string2charp_1(s) {
  var size = lengthBytesUTF8(s) + 1;
  var ptr = _malloc(size);
  stringToUTF8(s, ptr, size);
  return ptr;
}

function strings2charpp(a) {
  let char_pp = [];
  for (let idx=0; idx<a.length; idx++) {
    char_pp.push( string2charp(a[idx]) )
  }
  return char_pp;
}

function free_charpp(a) {
  for (let idx=0; idx<a.length; idx++) {
    _free(a[idx]);
  }
}


// f_name - function name
// param  - array of arrays, (command line) parameter list
//
// e.g.
//
//   f_ptr(param.length, FORMAT(param))
//
function main_like(f_cb, param) {

  let _w = 4;
  let _ws = "i32";

  let c_charpp = strings2charpp(param);

  let ptr_a = _malloc( param.length*_w );
  for (let ii=0; ii<param.length; ii++) {
    Module.setValue( ptr_a + (ii*_w), c_charpp[ii], _ws );
  }

  let rc = f_cb(param.length, ptr_a);

  _free(ptr_a);
  free_charpp(c_charpp);

  return rc;
}

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

var g_worker_info = {
  "init":false,
  "worker": null
};

function worker_update(e) {
  var txt = e.data;
  var data = JSON.parse(txt);

  update_pixi_tilemap(data);
}

function example_run_worker() {
  //let app = new PIXI.Application({ background: '#7f7f7f', resizeTo: window });
  let app = new PIXI.Application({ background: '#ffffff', resizeTo: window });
  document.body.appendChild(app.view);

  app.stage.eventMode = 'static';
  app.stage.hitArea = app.screen;
  app.stage.addEventListener('pointermove', mouse_move);

  g_tile_viewer_info.pixi.app = app;

  setup_keyboard();


  g_worker_info.worker = new Worker("js/poms_worker.js");
  g_worker_info.worker.onmessage = worker_update;
  g_worker_info.worker.postMessage("start");
}

// special function called from emscripten compiled code
//
function web_worker_cb() {
  return;

  console.log("web_worker_cb");
  let json_txt = new TextDecoder().decode( FS.readFile("pillMortal_snapshot.json") );
  let json_data = JSON.parse(json_txt);
  update_pixi_tilemap(json_data);
}

function poms_web_init() {
  console.log("poms_web_init");
}

