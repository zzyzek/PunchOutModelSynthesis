// LICENSE: CC0
//
// To the extent possible under law, the person who associated CC0 with
// this file has waived all copyright and related or neighboring rights
// to this file.
// 
// You should have received a copy of the CC0 legalcode along with this
// work.  If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//

// helper function to write poms config files from JS
//

var libpoms;

(function() {
  "use strict";

  var fs = require("fs");

  libpoms = (typeof libpoms === "undefined")?(function libpoms() {}):(exports);
  if (typeof global !== "undefined") { global.libpoms = libpoms; }

  libpoms.version = "0.1.0";

  libpoms.configTemplate = function libpoms_template() {

    let poms_cfg = {
      "rule": [],
      "weight": [],
      "name": [],
      "size" : [1,1,1],
      "constraint": [],
      "boundaryCondition" : {
        "x+": { "type": "tile" , "value": 0 },
        "x-": { "type": "tile" , "value": 0 },
        "y+": { "type": "tile" , "value": 0 },
        "y-": { "type": "tile" , "value": 0 },
        "z+": { "type": "tile" , "value": 0 },
        "z-": { "type": "tile" , "value": 0 }
      },

      "tileset" : {
        "image": "",
        "tilecount": -1,
        "imagewidth": -1,
        "imageheight": -1,
        "tileheight": -1,
        "tilewidth": -1
      },
      "objMap" : []

    };

    return poms_cfg;
  }

  libpoms.configWrite = function libpoms_write_json_file(poms_json, out_fn) {
    return fs.writeFileSync( out_fn, this.configStringify(poms_json) );
  }

  libpoms.configStringify = function libpoms_stringify_config_json(poms_json) {
    let lines = [];
    let multi_field = [];
    let multi_field_n = 8;

    lines.push("{");

    lines.push("\"rule\":[");
    for (let ii=0; ii<poms_json.rule.length; ii++) {
      let sfx = ( (ii==(poms_json.rule.length-1)) ? "" : "," );

      if (((ii%multi_field_n)==0) &&
          (multi_field.length > 0)) {
        lines.push( multi_field.join(" ") );
        multi_field = [];
      }

      //lines.push( JSON.stringify(poms_json.rule[ii]) + sfx );
      multi_field.push( JSON.stringify(poms_json.rule[ii]) + sfx );
    }
    if (multi_field.length > 0) {
      lines.push( multi_field.join(" ") );
    }
    lines.push("],");

    //---

    multi_field = [];
    lines.push("\"name\":[");
    for (let ii=0; ii<poms_json.name.length; ii++) {
      let sfx = ( (ii==(poms_json.name.length-1)) ? "" : "," );

      if (((ii%multi_field_n)==0) &&
          (multi_field.length > 0)) {
        lines.push( "  " + multi_field.join(" ") );
        multi_field = [];
      }

      multi_field.push( JSON.stringify(poms_json.name[ii]) + sfx );
    }
    if (multi_field.length > 0) {
      lines.push( "  " + multi_field.join(" ") );
    }
    lines.push("],");

    //---

    multi_field = [];
    lines.push("\"weight\":[");
    for (let ii=0; ii<poms_json.name.length; ii++) {
      let sfx = ( (ii==(poms_json.name.length-1)) ? "" : "," );

      if (((ii%multi_field_n)==0) &&
          (multi_field.length > 0)) {
        lines.push( "  " + multi_field.join(" ") );
        multi_field = [];
      }

      if (("weight" in poms_json) &&
          (ii < poms_json.weight.length)) {
        multi_field.push( poms_json.weight[ii].toString() + sfx );
      }
      else {
        multi_field.push( "1" + sfx );
      }
    }
    if (multi_field.length > 0) {
      lines.push( "  " + multi_field.join(" ") );
    }
    lines.push("],");

    //---

    multi_field = [];

    //lines.push("\"boundaryCondition\":" + JSON.stringify(poms_json.boundaryCondition) + ",");
    let bc_key = [];
    for (let _k in poms_json.boundaryCondition) { bc_key.push(_k); }
    lines.push("\"boundaryCondition\": {");
    for (let ii=0; ii<bc_key.length; ii++) {
      let _key = bc_key[ii];
      let bc = poms_json.boundaryCondition[_key];

      let _s = "  \"" + _key + "\" : " + JSON.stringify(bc);
      if (ii < (bc_key.length-1)) { _s += ","; }
      lines.push(_s);
    }
    lines.push("},");

    //lines.push("\"constraint\":" + JSON.stringify(poms_json.constraint) + ",");

    lines.push("\"constraint\": [");
    for (let ii=0; ii < poms_json.constraint.length; ii++) {
      let c = poms_json.constraint[ii];

      let _s = "  " + JSON.stringify(c);
      if (ii < (poms_json.constraint.length-1)) { _s += ","; }
      lines.push(_s);

    }
    lines.push("],");

    multi_field_n = 4;

    if (typeof poms_json.objMap !== "undefined") {
      lines.push("\"objMap\":[");
      for (let ii=0; ii<poms_json.objMap.length; ii++) {
        let sfx = ( (ii==(poms_json.objMap.length-1)) ? "" : "," );

        if (((ii%multi_field_n)==0) &&
            (multi_field.length > 0)) {
          lines.push( multi_field.join(" ") );
          multi_field = [];
        }

        //lines.push( JSON.stringify(poms_json.objMap[ii]) + sfx );
        multi_field.push( JSON.stringify(poms_json.objMap[ii]) + sfx );
      }
      if (multi_field.length > 0) {
        lines.push( multi_field.join(" ") );
      }
      lines.push("],");

      multi_field = [];
    }

    if ("flatMap" in poms_json){
      lines.push("\"flatMap\":[");

      for (let ii=0; ii<poms_json.flatMap.length; ii++) {
        let sfx = ( (ii==(poms_json.flatMap.length-1)) ? "" : "," );

        if (((ii%multi_field_n)==0) &&
            (multi_field.length > 0)) {
          lines.push( multi_field.join(" ") );
          multi_field = [];
        }

        multi_field.push( JSON.stringify(poms_json.flatMap[ii]) + sfx );
      }
      if (multi_field.length > 0) {
        lines.push( multi_field.join(" ") );
      }
      lines.push("],");

      multi_field = [];
    }

    if (typeof poms_json.tileset !== "undefined") {
      lines.push("\"tileset\":");

      lines.push( JSON.stringify(poms_json.tileset, undefined, 2) );
      lines.push(",");
    }

    if ("quiltSize" in poms_json) {
      lines.push("\"quiltSize\":" + JSON.stringify(poms_json.quiltSize)  + ",");
    }
    lines.push("\"size\":" + JSON.stringify(poms_json.size) );
    lines.push("}");

    return lines.join("\n");
  }

  libpoms.vec2cell = function(v,sz) {
    return (v[2]*sz[0]*sz[1]) + (v[1]*sz[0]) + v[0];
  }

  libpoms.cell2vec = function(cell,sz) {
    let v = [-1,-1,-1];

    v[2] = Math.floor( cell / (sz[0]*sz[1]) );
    cell -= v[2]*sz[0]*sz[1];
    v[1] = Math.floor( cell / (sz[0]) );
    cell -= v[1]*sz[0];
    v[0] = cell;

    return v;
  }


  if (typeof module !== "undefined") {
    module.exports = libpoms;
  }
})();
