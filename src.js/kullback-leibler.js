// LICENSE: CC0
//
// Note this has only been really tested for 2x2 windows.
// The boundary conditions might be a little screwed
// up for other types of windows, so this needs to be revisited.
//


if (typeof module !== "undefined") {
  var fs = require("fs");
  var getopt = require("posix-getopt");
}

var g_info = {

  "theory_tiled_fn": "",
  "observed_tiled_fn": "",

  "poms_fn" : "",
  "tiled_fn" : "",
  "verbose" : 0
};

function show_version() {
}

function show_help() {
}

function cli_parse(info) {
  info = ((typeof info === "undefined") ? g_info : info);

  let parser, opt;

  var g_cmd = "";

  let long_opt = [
    "Q", ":(theory-tiled)",
    "P", ":(observed-tiled)",
    "V", ":(verbose)",
    "v", "(version)",
    "h", "(help)"
  ];

  let _s;

  parser = new getopt.BasicParser(long_opt.join(""), process.argv);
  while ((opt =  parser.getopt()) !== undefined) {

    switch(opt.option) {
      case 'h':
        show_help(process.stdout);
        process.exit(0);
        break;
      case 'v':
        show_version(process.stdout);
        process.exit(0);
        break;
      case 'V':
        info.verbose = parseInt(opt.optarg);
        break;

      case 'P':
        info.observed_tiled_fn = opt.optarg;
        break;
      case 'Q':
        info.theory_tiled_fn = opt.optarg;
        break;

      default:
        show_help(process.stderr);
        process.exit(-1);
        break;
    }
  }


}

cli_parse(g_info);

var theory_tiled = JSON.parse( fs.readFileSync(g_info.theory_tiled_fn) );
var observed_tiled = JSON.parse( fs.readFileSync(g_info.observed_tiled_fn) );

let m = 0;

for (let ii=0; ii<theory_tiled.layers[0].data.length; ii++) {
  if (m < theory_tiled.layers[0].data[ii]) {
    m = theory_tiled.layers[0].data[ii];
  }
}
m++;


theory_q = [];
observed_p = [];
for (let ii=0; ii<m; ii++) {
  theory_q.push(0);
  observed_p.push(0);
}

let theory_N = 0,
    observed_N = 0;

for (let ii=0; ii<theory_tiled.layers[0].data.length; ii++) {
  let tile = theory_tiled.layers[0].data[ii];
  theory_q[tile] ++;
  theory_N++;
}

for (let ii=0; ii<theory_q.length; ii++) {
  theory_q[ii] /= theory_N;
}

for (let ii=0; ii<observed_tiled.layers[0].data.length; ii++) {
  let tile = observed_tiled.layers[0].data[ii];
  observed_p[tile] ++;
  observed_N++;
}

for (let ii=0; ii<observed_p.length; ii++) {
  observed_p[ii] /= observed_N;
}

let D_KL = 0.0;
for (let ii=1; ii<observed_p.length; ii++) {
  //console.log(theory_q[ii], observed_p[ii]);
  D_KL += observed_p[ii] * Math.log( observed_p[ii] / theory_q[ii] );
}

console.log(D_KL);


