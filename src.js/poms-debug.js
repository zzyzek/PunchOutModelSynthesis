var fs = require("fs");

//var poms_fn = "straight_end_poms.json";
var poms_fn = "";

if (process.argv.length > 2) {
  poms_fn = process.argv[2];
}

if (poms_fn.length == 0) {
  console.log("provide POMS config file");
  process.exit(-1);
}

var poms = JSON.parse( fs.readFileSync(poms_fn) );

let name = poms.name;
let rule = poms.rule;

let dir_name = [ "+x", "-x", "+y", "-y", "+z", "-z" ];

for (let ii=0; ii<rule.length; ii++) {

  let tile_src = name[ rule[ii][0] ];
  let tile_dst = name[ rule[ii][1] ];
  let dir_str = dir_name[ rule[ii][2] ];

  let ts = rule[ii][0];
  let td = rule[ii][1];

  console.log(tile_src.toString() +  "{" + ts.toString() + "}",
    "--(" + dir_str + ")-->",
    tile_dst.toString() +  "{" + td.toString() + "}");
}

