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
  "worker_state": "ready",
  "worker_counter": 0,
  "worker": null
};

//----
//----
//----

function worker_update(e) {
  let msg_data = e.data;

  if (!("type" in msg_data)) { console.log("ignoring", msg_data); return; }
  if (msg_data.type == "fin") {
    g_ui_info.worker_state = "ready";
    ui_run_ready(true);
    return;
  }

  else if (msg_data.type == "stdout") {
    ui_update_log_lines( msg_data.data );
    return;
  }

  else if (msg_data.type == "stderr") {
    console.log("got(stderr):", msg_data.data);
    return;
  }

  if (!("data" in msg_data)) { console.log("no data, ignoring", msg_data); return; }

  var txt = msg_data.data;
  var data = JSON.parse(txt);

  update_pixi_tilemap(data);


  let ele = document.getElementById("ui_run");
  let _txt = ["running"];
  for (let ii=0; ii<g_ui_info.worker_counter; ii++) {
    _txt.push(".");
  }
  ele.innerHTML = _txt.join("");
  g_ui_info.worker_counter ++;
  g_ui_info.worker_counter %= 4;

}


function start_run(dry_run) {
  dry_run = ((typeof dry_run === "undefined") ? false : dry_run);

  let tileset_name = ui_getSelect("ui_tileset");

  let poms_cfg = g_poms_default[tileset_name]["-C"];
  let tiled_fn = g_poms_default[tileset_name]["-1"];
  let snapshot_fn = g_poms_default[tileset_name]["-8"];

  let _argv_idx_map = {};

  let _argv = [];
  let _default_argmap = g_poms_default[tileset_name];
  for (let opt_key in _default_argmap) {

    if (opt_key.length == 2) {
      if ((opt_key == "-s") ||
          (opt_key == "-q") ||
          (opt_key == "-B") ||
          (opt_key == "-J") ||
          (opt_key == "-w") ||
          (opt_key == "-E") ||
          (opt_key == "-P") ||
          (opt_key == "-S") ) {
        _argv_idx_map[opt_key] = _argv.length;
      }

      _argv.push(opt_key);
      _argv.push(_default_argmap[opt_key]);


    }
    else if (opt_key.length > 2) {
      let key_parts = opt_key.split(" ");
      let param = key_parts[0];
      let val = key_parts[1] + "=" + _default_argmap[opt_key];

      if ((opt_key == "-O patch-policy") ||
          (opt_key == "-O viz_step")) {
        _argv_idx_map[opt_key] = _argv.length;
      }

      _argv.push(param);
      _argv.push(val);
    }

  }

  g_ui_info.snapshot_fn = g_poms_default[tileset_name]["-8"];

  // fill in _argv with ui specified elemnts, if appropriate
  //

  let _tx, _ty, _tx_e, _ty_e, _tv;

  // block size
  //
  let bs_str = g_poms_default[tileset_name]["-s"].split(",");
  _tx = parseInt(ui_getNum("ui_blocksize_x"));
  if ((_tx != "") && (!isNaN(_tx))) { bs_str[0] = _tx.toString(); }
  _ty = parseInt(ui_getNum("ui_blocksize_y"));
  if ((_ty != "") && (!isNaN(_ty))) { bs_str[1] = _ty.toString(); }
  _argv[ _argv_idx_map["-s"]+1 ] = bs_str.join(",");

  // grid(quilt) size
  //
  let qs_str = g_poms_default[tileset_name]["-q"].split(",");
  _tx = parseInt(ui_getNum("ui_gridsize_x"));
  if ((_tx != "") && (!isNaN(_tx))) { bs_str[0] = _tx.toString(); }
  _ty = parseInt(ui_getNum("ui_gridsize_y"));
  if ((_ty != "") && (!isNaN(_ty))) { bs_str[1] = _ty.toString(); }
  _argv[ _argv_idx_map["-q"]+1 ] = bs_str.join(",");

  // rand w
  //
  let randw_str = g_poms_default[tileset_name]["-w"];
  _tv = parseFloat(ui_getNum("ui_rand_w"));
  if ((_tv != "") && (!isNaN(_tv))) { randw_str = _tv.toString(); }
  _argv[ _argv_idx_map["-w"]+1 ] = randw_str;

  // rand E (exponent)
  //
  let randE_str = g_poms_default[tileset_name]["-E"];
  _tv = parseFloat(ui_getNum("ui_rand_E"));
  if ((_tv != "") && (!isNaN(_tv))) { randE_str = _tv.toString(); }
  _argv[ _argv_idx_map["-E"]+1 ] = randE_str;

  // max iter
  //
  let mi_str = g_poms_default[tileset_name]["-J"];
  _tv = parseInt(ui_getNum("ui_maxiter"));
  if ((_tv != "") && (!isNaN(_tv))) { mi_str = _tv.toString(); }
  _argv[ _argv_idx_map["-J"]+1 ] = mi_str;

  // seed
  //
  let seed_str = g_poms_default[tileset_name]["-S"];
  _tv = parseInt(ui_getNum("ui_seed"));
  if ((_tv != "") && (!isNaN(_tv))) { seed_str = _tv.toString(); }
  _argv[ _argv_idx_map["-S"]+1 ] = seed_str;

  let cell_choice_str = g_poms_default[tileset_name]["-P"];
  _tv = ui_getSelect("ui_cell_choice_policy");
  if (_tv.match( '^(min|max|rand|wf)$' )) { cell_choice_str = _tv; }
  _argv[ _argv_idx_map["-P"]+1 ] = cell_choice_str;

  // block(patch) choice policy
  //
  let patch_choice_str = g_poms_default[tileset_name]["-O patch-policy"];
  _tv = ui_getSelect("ui_block_choice_policy");
  if (_tv.match( '^(pending|xnyn|xpyp|xnyn|cone|cone1|cone-|cone1-|wf|wf-|wf2|wf3)$' )) {
    patch_choice_str = _tv;
  }
  _argv[ _argv_idx_map["-O patch-policy"]+1 ] = "patch-policy=" + patch_choice_str;

  let viz_step_str = g_poms_default[tileset_name]["-O viz_step"];
  _tv = parseInt(ui_getNum("ui_viz_step"));
  if ((_tv != "") && (!isNaN(_tv))) { viz_step_str = _tv.toString(); }
  _argv[ _argv_idx_map["-O viz_step"]+1 ] = "viz_step=" + viz_step_str;

  let msg = {
    "type":"run",
    "argv": _argv
  };

  if (dry_run) {
    console.log("msg:", msg);
    return;
  }

  ui_run_ready(false);

  g_ui_info.worker.postMessage(msg);
}

//
//  _   _ ___ 
// | | | |_ _|
// | |_| || | 
//  \___/|___|
//            

function ui_update_log_lines(logline) {
  let ele = document.getElementById("ui_log");

  if (typeof logline === "undefined") {
    ele.innerHTML = '';
    return;
  }

  let div = document.createElement("div");
  div.classList.add("twelve");
  div.classList.add("columns");
  //div.innerHTML = encodeURIComponent( logline );
  div.innerHTML = html_encode( logline );

  ele.appendChild(div);

}

function ui_run_ready(ready_to_run) {
  ready_to_run = ((typeof ready_to_run === "undefined") ? false : ready_to_run);
  let ele = document.getElementById("ui_run");
  if (ready_to_run) {
    ele.innerHTML = "run";
    ele.disabled = false;
    ele.style.color = "rgba(80,80,80,1.0)";
    ele.style.background='rgba(176,237,136,0.5)';
  }
  else {
    ele.innerHTML = "running...";
    ele.disabled = true;
    ele.style.color = "rgba(80,80,80,0.5)";
    ele.style.background = "rgba(0,0,0,0.2)";
  }
}

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

//----
// https://stackoverflow.com/a/14130005/4002265
// CC-BY-SA 3.0 https://stackoverflow.com/users/616443/j08691
//
function html_encode(str) {
    return String(str).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');
}
//----

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

  // these are command line options that have variables and more complex
  // choices, so we have to make special considerations.
  //
  // The options here are of the form `-O param=val`, so
  // the convention here is to construct the key in the
  // default param list as the simple command line option (`-O`)
  // followed by a space, followed by the param and have the
  // value be `val`.
  //
  // So, for example:
  //
  //   g_poms_default["pillMortal"]["-O viz_step"] = "50"
  //
  // would correspond to a command line argument of:
  //
  //   ... -O viz_step=50 ...
  //
  // we need to reconstruct the options in `start_run`
  //
  let viz_step = _default["-O viz_step"];
  ui_setNum("ui_viz_step", viz_step);

  let block_choice_policy = _default["-O patch-policy"];
  ui_setSelect("ui_block_choice_policy", block_choice_policy);

}

function setup_ui_callbacks() {
  document.getElementById("ui_dialog").addEventListener("click",
    function(e) {
      e.preventDefault();
      e.stopPropagation();
      e.stopImmediatePropagation();
      return false;
    }
  );

  document.getElementById("ui_container").addEventListener("click",
    function(e) {
      let ele = document.getElementById("ui_dialog");
      if (ele.open) { ele.open = false; }
      return true;
    }
  );


  document.getElementById("ui_run").addEventListener("click",
    function(e) {
      start_run();
      ui_update_log_lines();
      //example_run_worker();
    });

  document.getElementById("ui_tileset").addEventListener("change",
    function(e) {
      let v = ui_getSelect("ui_tileset");
      ui_populate_defaults(v);
    });

  document.getElementById("ui_option").addEventListener("click",
    function(e) {
      let _d = document.getElementById("ui_dialog");
      _d.open = true;

      e.preventDefault();
      e.stopPropagation();
      e.stopImmediatePropagation();
      return false;
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

  ui_populate_defaults(ui_getSelect("ui_tileset"));

  let app = new PIXI.Application({ background: '#ffffff', resizeTo: window });
  app.view.id = 'ui_canvas';

  let ele = document.getElementById("ui_gfx");
  ele.innerHTML = '';
  ele.appendChild(app.view);

  app.stage.eventMode = 'static';
  app.stage.hitArea = app.screen;
  app.stage.addEventListener('pointermove', mouse_move);

  g_tile_viewer_info.pixi.app = app;
  g_ui_info.app = app;
  g_ui_info.gx = ele;

  //setup_keyboard( document.getElementById("ui_canvas") );

  g_ui_info.worker = new Worker("js/poms_worker.js");
  g_ui_info.worker.onmessage = worker_update;
}

