/*
 * To the extent possible under law, the person who associated CC0 with
 * this file has waived all copyright and related or neighboring rights
 * to this file.
 *
 * You should have received a copy of the CC0 legalcode along with this
 * work. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 *
 */


var g_poms_default = {

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

  // 0x72
  //
  "2bmmv": {
    "-C" : "data/2bmmv_poms.json",
    "-s": "48,48,1",
    "-q": "128,128,1",
    "-b": "1",
    "-B": "8:24",
    "-J":"10000",
    "-w": "4",
    "-E": "-1.5",
    "-1": "2bmmv_tiled.json",
    "-8": "2bmmv_snapshot.json",
    "-P": "min",
    "-O": "viz_step=50",
    "-O": "patch-policy=pending",
    "-S": "1337",
    "-V": "1"
  },

  // adam atomic
  //
  "1985": {
    "-C" : "data/1985_poms.json",
    "-s": "48,48,1",
    "-q": "128,128,1",
    "-b": "1",
    "-B": "16:32",
    "-J":"10000",
    "-w": "4",
    "-E": "-1.5",
    "-1": "1985_tiled.json",
    "-8": "1985_snapshot.json",
    "-P": "min",
    "-O": "viz_step=50",
    "-O": "patch-policy=pending",
    "-S": "1337",
    "-V": "1"
  },

  "amarelo": {
    "-C" : "data/amarelo_poms.json",
    "-s": "48,48,1",
    "-q": "128,128,1",
    "-b": "1",
    "-B": "24:48",
    "-J":"10000",
    "-w": "2",
    "-E": "-1.25",
    "-1": "amarelo_tiled.json",
    "-8": "amarelo_snapshot.json",
    "-P": "min",
    "-O": "patch-policy=pending",
    "-O": "viz_step=50",
    "-S": "1337",
    "-V": "1"
  },

  "kyst": {
    "-C" : "data/kyst_poms.json",
    "-s": "42,42,1",
    "-q": "128,128,1",
    "-b": "1",
    "-B": "8:32",
    "-J":"10000",
    "-w": "1",
    "-E": "-1.5",
    "-1": "kyst_tiled.json",
    "-8": "kyst_snapshot.json",
    "-P": "min",
    "-O": "patch-policy=pending",
    "-O": "viz_step=50",
    "-S": "1337",
    "-V": "1"
  },

  "mccaves": {
    "-C" : "data/mccaves_poms.json",
    "-s": "48,48,1",
    "-q": "128,128,1",
    "-b": "1",
    "-B": "12:24",
    "-J":"10000",
    "-w": "2",
    "-E": "-1.7",
    "-1": "mccaves_tiled.json",
    "-8": "mccaves_snapshot.json",
    "-P": "min",
    "-O": "patch-policy=pending",
    "-O": "viz_step=50",
    "-S": "1337",
    "-V": "1"
  },

  // Piiixl
  //
  "island": {
    "-C" : "data/island_poms.json",
    "-s": "32,32,1",
    "-q": "128,128,1",
    "-b": "1",
    "-B": "8:16",
    "-J":"10000",
    "-w": "2",
    "-E": "-1.7",
    "-1": "island_tiled.json",
    "-8": "island_snapshot.json",
    "-P": "min",
    "-O": "patch-policy=pending",
    "-O": "viz_step=50",
    "-S": "1337",
    "-V": "1"
  },

  // surt
  //
  "blowharder": {
    "-C" : "data/blowharder_poms.json",
    "-s": "32,32,1",
    "-q": "128,128,1",
    "-b": "1",
    "-B": "8:16",
    "-J":"10000",
    "-w": "1",
    "-E": "-1.5",
    "-1": "blowharder_tiled.json",
    "-8": "blowharder_snapshot.json",
    "-P": "min",
    "-O": "patch-policy=pending",
    "-O": "viz_step=50",
    "-S": "1337",
    "-V": "1"
  },

  "psygen": {
    "-C" : "data/psygen_poms.json",
    "-s": "52,52,1",
    "-q": "128,128,1",
    "-b": "1",
    "-B": "24:32",
    "-J":"10000",
    "-w": "1",
    "-E": "-1.5",
    "-1": "psygen_tiled.json",
    "-8": "psygen_snapshot.json",
    "-P": "min",
    "-O": "patch-policy=cone-",
    "-O": "viz_step=50",
    "-S": "1337",
    "-V": "1"
  },

  "vilenes": {
    "-C" : "data/vilenes_poms.json",
    "-s": "52,52,1",
    "-q": "128,128,1",
    "-b": "1",
    "-B": "8:32",
    "-J":"10000",
    "-w": "1",
    "-E": "-1.0",
    "-1": "vilenes_tiled.json",
    "-8": "vilenes_snapshot.json",
    "-P": "wf",
    "-O": "patch-policy=pending",
    "-O": "viz_step=50",
    "-S": "1337",
    "-V": "1"
  },

  // thkcasper
  //
  "forestmicro": {
    "-C" : "data/forestmicro_poms.json",
    "-s": "48,48,1",
    "-q": "128,128,1",
    "-b": "1",
    "-B": "8,8,1",
    "-J":"10000",
    "-w": "1.0",
    "-E": "-1.95",
    "-1": "forestmicro_tiled.json",
    "-8": "forestmicro_snapshot.json",
    "-P": "wf",
    "-O": "viz_step=50",
    "-O": "patch-policy=wf",
    "-S": "1337",
    "-V": "1"
  },

  "neondungeon": {
    "-C" : "data/neondungeon_poms.json",
    "-s": "32,32,1",
    "-q": "128,128,1",
    "-b": "1",
    "-B": "16",
    "-J":"10000",
    "-w": "1.0",
    "-E": "-1.5",
    "-1": "neondungeon_tiled.json",
    "-8": "neondungeon_snapshot.json",
    "-P": "min",
    "-O": "viz_step=50",
    "-O": "patch-policy=pending",
    "-S": "1337",
    "-V": "1"
  },

  "neondirt": {
    "-C" : "data/neondirt_poms.json",
    "-s": "58,58,1",
    "-q": "128,128,1",
    "-b": "1",
    "-B": "32",
    "-J":"10000",
    "-w": "1.0",
    "-E": "-1.5",
    "-1": "neondirt_tiled.json",
    "-8": "neondirt_snapshot.json",
    "-P": "min",
    "-O": "viz_step=50",
    "-O": "patch-policy=pending",
    "-S": "1337",
    "-V": "1"
  },

  "neonsnow": {
    "-C" : "data/neonsnow_poms.json",
    "-s": "32,32,1",
    "-q": "128,128,1",
    "-b": "1",
    "-B": "32",
    "-J":"10000",
    "-w": "1.0",
    "-E": "-1.5",
    "-1": "neonsnow_tiled.json",
    "-8": "neonsnow_snapshot.json",
    "-P": "min",
    "-O": "viz_step=50",
    "-O": "patch-policy=pending",
    "-S": "1337",
    "-V": "1"
  },

  // lunarsignals
  //
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
  }

};
