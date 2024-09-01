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

  "worker_result": {},

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

    g_ui_info.worker_result = e.data;

    ui_run_ready(true);

    // let download
    //
    let dl_btn = document.getElementById("ui_download");
    dl_btn.classList.remove("button-disabled");

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

function ui_run_ready(ready_to_run) {
  ready_to_run = ((typeof ready_to_run === "undefined") ? false : ready_to_run);
  let ele = document.getElementById("ui_run");
  if (ready_to_run) {
    ele.innerHTML = "run";
    ele.disabled = false;

    ele.classList.remove("button-disabled");
    ele.classList.add("button-primary");
  }
  else {
    ele.innerHTML = "running...";
    ele.disabled = true;

    ele.classList.remove("button-primary");
    ele.classList.add("button-disabled");

    let dl_btn = document.getElementById("ui_download");
    dl_btn.classList.add("button-disabled");
  }
}

function ui_update_log_lines(logline) {
  let ele = document.getElementById("ui_log");

  if (typeof logline === "undefined") {
    ele.innerHTML = '';
    return;
  }

  let div = document.createElement("div");
  div.classList.add("twelve");
  div.classList.add("columns");
  div.innerHTML = html_encode( logline );

  ele.appendChild(div);

}

//---
// https://stackoverflow.com/a/22172860/4002265
// CC-BY-SA 3.0 https://stackoverflow.com/users/1865613/%cb%88v%c9%94l%c9%99
//
function getBase64Image(img) {
  var canvas = document.createElement("canvas");
  canvas.width = img.width;
  canvas.height = img.height;
  var ctx = canvas.getContext("2d");
  ctx.drawImage(img, 0, 0);
  var dataURL = canvas.toDataURL("image/png");
  return dataURL.replace(/^data:image\/?[A-z]*;base64,/, '');
}
//---

function create_flat_tiled(base_tiled_txt, poms_txt, flat_tileset_img) {
  let flat_tiled_json = JSON.parse(base_tiled_txt);
  let poms_json = JSON.parse(poms_txt);

  let id_count = -1;

  let layers = flat_tiled_json.layers;
  for (let layer_idx=0; layer_idx < layers.length; layer_idx++) {
    let data = layers[ layer_idx ].data;
    for (let cell=0; cell < data.length; cell++) {
      let src_id = data[cell];
      let dst_id = 0;
      if ((src_id >= 0) && (src_id < poms_json.flatMap.length)) {
        dst_id = poms_json.flatMap[src_id];
      }
      data[cell] = dst_id;

      if (dst_id > id_count) { id_count = dst_id; }
    }
  }
  id_count++;

  for (let tileset_id=0; tileset_id < flat_tiled_json.tilesets.length; tileset_id++) {
    flat_tiled_json.tilesets[tileset_id].name =
      flat_tiled_json.tilesets[tileset_id].name.replace('_tileset', '_flat_tileset');
    flat_tiled_json.tilesets[tileset_id].image =
      flat_tiled_json.tilesets[tileset_id].image.replace('_tileset', '_flat_tileset');

    flat_tiled_json.tilesets[tileset_id].tilecount = id_count;
    flat_tiled_json.tilesets[tileset_id].imageheight = flat_tileset_img.height;
    flat_tiled_json.tilesets[tileset_id].imagewidth = flat_tileset_img.width;
  }

  return JSON.stringify(flat_tiled_json);
}

async function ui_download() {

  let tileset_name = ui_getSelect("ui_tileset");

  let poms_cfg_fn = g_poms_default[tileset_name]["-C"];
  let tiled_fn = g_poms_default[tileset_name]["-1"];
  let tileset_img_ele = document.getElementById("img_" + tileset_name);
  let flat_tileset_img_ele = document.getElementById("img_flat_" + tileset_name);

  //let canvas_b64 = document.getElementById("ui_canvas").toDataURL("image/png");
  let canvas_img = await g_ui_info.app.renderer.extract.image( g_ui_info.app.stage, "image/png" )
  let canvas_b64 = canvas_img.src.replace(/^data:image\/?[A-z]*;base64,/, '');


  let poms_txt = g_ui_info.worker_result.poms;
  let tileset_imgdata = getBase64Image( tileset_img_ele );
  let flat_tileset_imgdata = getBase64Image( flat_tileset_img_ele );
  let tiled_txt = g_ui_info.worker_result.tiled;


  let flat_tiled_txt = create_flat_tiled(tiled_txt, poms_txt, flat_tileset_img_ele);

  let zip = new JSZip();
  zip.file(tileset_name + "/" + tileset_name + "_img.png", canvas_b64, {"base64":true});
  zip.file(tileset_name + "/" + tileset_name + "_tiled.json", tiled_txt);
  zip.file(tileset_name + "/" + tileset_name + "_tileset.png", tileset_imgdata, {"base64":true});
  zip.file(tileset_name + "/" + tileset_name + "_flat_tiled.json", flat_tiled_txt);
  zip.file(tileset_name + "/" + tileset_name + "_flat_tileset.png", flat_tileset_imgdata, {"base64":true});
  zip.file(tileset_name + "/" + tileset_name + "_poms.json", poms_txt);

  zip.generateAsync({"type":"blob"})
    .then( function(c) {
      saveAs(c, tileset_name + ".zip");
    });
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

  document.getElementById("ui_dialog_close").addEventListener("click",
    function(e) {
      let _d = document.getElementById("ui_dialog");
      _d.open = false;

  });
  /*
  document.getElementById("ui_option_cancel").addEventListener("click",
    function(e) {
      let _d = document.getElementById("ui_dialog");
      _d.open = false;
    });
  */

  //-----
  //-----

  document.getElementById("ui_download").addEventListener("click",
    function(e) {
      let ele = document.getElementById("ui_download");
      if (ele.classList.contains("button-disabled")) { return false; }

      ui_download();
    });


  //-----
  //-----
  // default buttons in dialog
  //

  document.getElementById("ui_blocksize_default").addEventListener("click",
    function(e) {
      let ele_x = document.getElementById("ui_blocksize_x");
      let ele_y = document.getElementById("ui_blocksize_y");
      let tileset_name = ui_getSelect("ui_tileset");
      ele_x.value = g_poms_default[tileset_name]["-s"].split(",")[0];
      ele_y.value = g_poms_default[tileset_name]["-s"].split(",")[1];
    });

  document.getElementById("ui_gridsize_default").addEventListener("click",
    function(e) {
      let ele_x = document.getElementById("ui_gridsize_x");
      let ele_y = document.getElementById("ui_gridsize_y");
      let tileset_name = ui_getSelect("ui_tileset");
      ele_x.value = g_poms_default[tileset_name]["-q"].split(",")[0];
      ele_y.value = g_poms_default[tileset_name]["-q"].split(",")[1];
    });

  document.getElementById("ui_softensize_default").addEventListener("click",
    function(e) {
      let ele_x_s = document.getElementById("ui_softensize_x_start");
      let ele_y_s = document.getElementById("ui_softensize_y_start");
      let ele_x_e = document.getElementById("ui_softensize_x_end");
      let ele_y_e = document.getElementById("ui_softensize_y_end");

      let tileset_name = ui_getSelect("ui_tileset");

      let soften_se = [['',''],['','']];

      let tok = g_poms_default[tileset_name]["-B"].split(":");
      let tok_v = tok[0].split(",");

      soften_se[0][0] = tok_v[0];
      if (tok_v.length > 1) { soften_se[0][1] = tok_v[1]; }

      soften_se[1][0] = soften_se[0][0];
      soften_se[1][1] = soften_se[0][1];

      if (tok.length > 0) {
        tok_v = tok_v[1].split(",");
        soften_se[1][0] = tok_v[0];
        if (tok_v.length > 1) { soften_se[1][1] = tok_v[1]; }
      }

      ele_x_s.value = soften_se[0][0];
      ele_y_s.value = soften_se[0][1];

      ele_x_e.value = soften_se[1][0];
      ele_y_e.value = soften_se[1][1];
    });

  document.getElementById("ui_maxiter_default").addEventListener("click",
    function(e) {
      let ele = document.getElementById("ui_maxiter");
      let tileset_name = ui_getSelect("ui_tileset");
      ele.value = g_poms_default[tileset_name]["-J"];
    }
  );

  document.getElementById("ui_rand_default").addEventListener("click",
    function(e) {
      let ele_w = document.getElementById("ui_rand_w");
      let ele_E = document.getElementById("ui_rand_E");
      let tileset_name = ui_getSelect("ui_tileset");
      ele_w.value = g_poms_default[tileset_name]["-w"];
      ele_E.value = g_poms_default[tileset_name]["-E"];
    }
  );

  document.getElementById("ui_cell_choice_policy_default").addEventListener("click",
    function(e) {
      let tileset_name = ui_getSelect("ui_tileset");
      ui_setSelect("ui_cell_choice_policy", g_poms_default[tileset_name]["-P"]);
    }
  );

  document.getElementById("ui_block_choice_policy_default").addEventListener("click",
    function(e) {
      let tileset_name = ui_getSelect("ui_tileset");
      ui_setSelect("ui_block_choice_policy", g_poms_default[tileset_name]["-O patch-policy"]);
    }
  );

  document.getElementById("ui_viz_step_default").addEventListener("click",
    function(e) {
      let ele = document.getElementById("ui_viz_step");
      let tileset_name = ui_getSelect("ui_tileset");
      ele.value = g_poms_default[tileset_name]["-O viz_step"];
    }
  );

  document.getElementById("ui_seed_default").addEventListener("click",
    function(e) {
      let ele = document.getElementById("ui_seed");
      let tileset_name = ui_getSelect("ui_tileset");
      ele.value = g_poms_default[tileset_name]["-S"];
    }
  );

  document.getElementById("ui_seed_random").addEventListener("click",
    function(e) {
      let ele = document.getElementById("ui_seed");
      ele.value = Math.floor(Math.random()*100000);
    }
  );



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


  ui_update_log_lines();
}

