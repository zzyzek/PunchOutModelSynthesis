// LICENSE: CC0
//
// Note this has only been really tested for 2x2 windows.
// The boundary conditions might be a little screwed
// up for other types of windows, so this needs to be revisited.
//

// reweight a POMS config file based on the flatMap frequency.
//

var fs = require("fs");

if (process.argv.length < 3) {
  console.log("provide poms file to reweight\n");
  process.exit(-1);
}

var poms_fn = process.argv[2];

var poms = JSON.parse( fs.readFileSync(poms_fn) );

if ((!("weight" in poms)) ||
    (!("flatMap" in poms))) {
  console.log("'weight' and/or 'flatMap' fields not found, can't continue");
  process.exit(-2);
}


let weight = poms.weight;
let flatmap = poms.flatMap;

let maxflat = -1;
for (let ii=0; ii<flatmap.length; ii++) {
  if (flatmap[ii] > maxflat) { maxflat=flatmap[ii]; }
}
maxflat++;

let flatfreq = new Array(maxflat);
for (let ii=0; ii<flatfreq.length; ii++) { flatfreq[ii]=0; }

for (let ii=0; ii<flatmap.length; ii++) {
  flatfreq[flatmap[ii]]++;
}

for (let ii=0; ii<flatfreq.length; ii++) {
  let v = flatfreq[ii];
  if (v < 0.5) {
    console.log("sanity: flatfreq[", ii,"] has 0 frequency");
    continue;
  }
  flatfreq[ii] = 1/v;
}

for (let ii=0; ii<weight.length; ii++) {
  weight[ii] = flatfreq[flatmap[ii]];
}

console.log( JSON.stringify(poms,undefined,2) );





