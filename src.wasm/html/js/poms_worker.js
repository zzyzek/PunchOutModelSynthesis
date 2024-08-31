/*
 * To the extent possible under law, the person who associated CC0 with
 * this file has waived all copyright and related or neighboring rights
 * to this file.
 *
 * You should have received a copy of the CC0 legalcode along with this
 * work. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 *
 */

var g_wrk = {

  "stdout_buf": "",
  "stderr_buf": "",

  "buf" : [],
  "buf_n" : 0,
  "buf_max" : 1024,
  "poms_fn": "",
  "tiled_fn" : "",
  "snapshot_fn": "",
  "ready": false
};


for (let ii=0; ii<g_wrk.buf_max; ii++) { g_wrk.buf.push(''); }

importScripts("poms.js");


// need this here because the emscripten runtime takes a while
// to load. When instanciating the web worker we need to
// wait till the whole environment is loaded before we kick
// off a run.
//
Module['onRuntimeInitialized'] = function() {
  g_wrk.ready = true;
}

function in_cb() {
  return 0;
};

function out_cb(x) {
  let ch = String.fromCharCode(x);
  g_wrk.stdout_buf += ch;
  if (g_wrk.stdout_buf.length > g_wrk.stdout_buf) {

    postMessage({"type":"stdout", "code":"maxbuf", "data":g_wrk.stdout_buf});

    g_wrk.stdout_buf = '';

  }

  if (ch == '\n') {
    postMessage({"type":"stdout", "code":"line", "data":g_wrk.stdout_buf});
    g_wrk.stdout_buf = '';
  }

}

function err_cb(x) {
  let ch = String.fromCharCode(x);
  g_wrk.stderr_buf += ch;
  if (g_wrk.stderr_buf.length > g_wrk.stderr_buf) {

    postMessage({"type":"stderr", "code":"maxbuf", "data":g_wrk.stderr_buf});

    g_wrk.stderr_buf = '';
  }

  if (ch == '\n') {
    postMessage({"type":"stderr", "code":"line", "data":g_wrk.stderr_buf});
    g_wrk.stderr_buf = '';
  }


}

Module.preRun.push( function() {
  FS.init(in_cb, out_cb, err_cb);
});

//Module.preRun = function() { console.log("prerun"); }

onmessage = function(e) {
  let msg = e.data;
  let op = msg.type;

  if (!g_wrk.ready) {
    setTimeout( (function(_e) { return function() { onmessage(_e); } })(e), 10 );
    return;
  }

  let argv = ["bin"];
  for (let ii=0; ii<msg.argv.length; ii++) {
    argv.push(msg.argv[ii]);
  }

  for (let ii=1; ii<argv.length; ii++) {
    if (argv[ii-1] == "-8") {
      g_wrk.snapshot_fn = argv[ii];
    }

    if (argv[ii-1] == "-1") {
      g_wrk.tiled_fn = argv[ii];
    }

    if (argv[ii-1] == "-C") {
      g_wrk.poms_fn = argv[ii];
    }

  }

  if (op == "run") {
    main_like( Module._main, argv );
  }
  //else if (op == "start") { example_run(); }
  else if (op == "ping") {
    postMessage({"type":"pong"});
  }
  else if (op == "result") {
    let json_txt = new TextDecoder().decode( FS.readFile(g_wrk.tiled_fn) );
    postMessage({ "type":"data", "data":json_txt});
  }
  else {
    postMessage({"type":"error", "message":"unknown op"});
  }

}

//
// https://stackoverflow.com/a/70267473/4002265
// convert a Javascript string to a C string
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

  let poms_json_txt = new TextDecoder().decode( FS.readFile( g_wrk.poms_fn ));
  let tiled_json_txt = new TextDecoder().decode( FS.readFile( g_wrk.tiled_fn ));


  postMessage({"type":"fin", "code":rc, "tiled": tiled_json_txt, "poms": poms_json_txt  });

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
  return out_json;
}

// special function called from emscripten compiled code
//
function web_worker_cb() {

  let json_txt = "{}";

  if (g_wrk.snapshot_fn.length > 0) {
    //json_txt = new TextDecoder().decode( FS.readFile("pillMortal_snapshot.json") );
    json_txt = new TextDecoder().decode( FS.readFile(g_wrk.snapshot_fn) );
  }
  postMessage({"type":"data", "data":json_txt});
}


