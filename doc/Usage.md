Usage
===

###### 2024-12-09

```
$ poms
must provide POMS config JSON file

poms bin version: 0.18.0
poms lib version: 0.23.0

  -s,--size         size of map
  -q,--quilt-size   quilt size of map
  -b,--block        block size
  -B,--soften       soften block size
  -J,--iter         iteration count
  -w,--rand-w       random coefficient
  -E,--rand-E       random exponent
  -P,--block-policy block-policy (min|max|seq)
  -C,--config       input config file
  -1,--tiled        output Tiled JSON file
  -2,--stl          output STL file (requires objMap)
  -3,--tiled-poms   output Tiled POMS JSON file
  -5,--sliced-tiled-snapshot output sliced Tiled snapshot JSON file
  -6,--patch-snapshot JSON patch snapshot file
  -7,--stl-snapshot STL snapshot file
  -8,--tiled-snapshot tiled snapshot JSON file
  -9,--tiled-slideshow-dir directory for tiled slideshow JSON files
  -N,--noise        custom noise G function (<freq>:<seed>:<type>) (type {0:none,1:linear,2:threshold,3:tierd})
  -@,--viz          gnuplot visualization file
  -S,--seed         seed
  -O,--option       option
  -V,--verbose      verbose level
  -h,--help         help (this screen)
  -v,--version      show version
```

The `size`, `quilt-size`, `block`, `soften` and `seed` parameters can be specified in the `config` file (see [schema](https://github.com/zzyzek/PunchOutModelSynthesis/blob/main/doc/Schemas.md) for details).

## Example

In [`runs/pillMortal`](https://github.com/zzyzek/PunchOutModelSynthesis/tree/main/runs/pillMortal):

```
poms \
  -C ./data/pillMortal_poms.json \
  -s "48,48,1 \
  -q "128,128,1" \
  -b 1 \
  -B 8,8,1 \
  -J 10000 \
  -w 1.0 \
  -E -1.95 \
  -1 ./data/pillMortal_128x128.json \
  -8 ./data/pillMortal_snapshot.json \
  -P min \
  -O patch-policy=pending \
  -S 1337 \
  -V 1
```

The `data/pillMortal_poms.json` config file is used.

This will run the `pillMortal` tile set and try to create a 128x128 grid, running BMS on 48x48 blocks with a soften window size of 8x8.
BMS will resolve 1 cell at a time with a minimum entropy heuristic.
The patch will be chosen randomly from unresolved cells in the grid.

BMS will try 10,000 rounds of resolution and softening before it gives up.

The random seed is 1337.
The random coefficient is set to 1.0 with the random exponent set to -1.95.

Verbosity is set to 1, printing after each patch resolution (success or failure).

A Tiled (like) snapshot file, `data/pillMortal_snapshot.json`, is produced intra-run.
On successful resolution, the Tiled `data/pillMortal_128x128.json` file is produced.

Since the `size` and `quilt-size` are specified on the command line, they will override the values in the `data/pillMortal_poms.json`
config file.


## Parameter Description

### `-C`, `--config`

POMS config file name

Example:

```
-C pillMortal_poms.json
```

### `-s`, `--size`

Patch size.

1-3 valued integer tuple, comma separated.

Later size values will be automatically filled in with the value before it.

Required if not specified in POMS JSON config file.

Example (creates 12x12x12 block):

```
-s 12
```

Example (creates 7x11x1 block):

```
-s 7,11,1
```

### `-q`, `--quilt-size`

Quilt size.

1-3 valued integer tuple, comma separated.

Later size values will be automatically filled in with the value before it.

Automatically inherits from `size` if not specified on the command line
or POMS JSON config file.

Example (creates 15x15x15 block):

```
-s 15
```

Example (creates 24x21x23 block):

```
-s 24,21,23
```

### `-b`, `--block`


BMS resolution block size.

Default 1.

Example:

```
-b 1
```

### `-B`, `--soften`

BMS Soften size to use within resolution blocks.

1-3 valued integer tuple, comma (`,`) separated.
Tuple ranges can be specified with a colon (`:`) separator.

Example:

```
-B 3,3,3
```

### `-J`, `--iter`


Max BMS iteration count.

Default value is patch size (`-s`) divided by BMS resolution block size (`-b`).

Example:

```
-J 10000
```

### `-w`, `--rand-w`


Linear random coefficient to use when choosing cell resolution.

Default 0.

Example:

```
-w 1.0
```

### `-E`, `--rand-E`

Exponent for random element of choosing which cell to resolve.

Default 0.

Should be negative.

Example:

```
-E -1.75
```

### `-P`, `--block-policy`

Cell resolution policy used by BMS.

One of `min`, `max`, `seq` for minimum entropy, maximum entropy and sequential,
respectively.

Default `min`.

Example:

```
-P min
```

### `-1`, `--tiled-poms`

Filename of the Tiled output file to generate on successfully resolution.

Example:

```
-1 tiled_output.json
```

### `-2`, `--stl`

Filename of the STL file to generate on successful resolution.

POMS config file must have `objMap` correctly setup.

Example:

```
-2 3d_output.stl
```

### `-3`, `--tiled-poms`

Filename of the Tiled POMS JSON file to generate on successful resolution.

Example:

```
-3 poms_tiled_output.json
```

### `-5`, `--sliced-tiled-snapshot`


Filename of the run time sliced Tiled JSON file to generate.

This assumes the tile set is 2d with the third dimension of time.
The Tiled file creates an expanded tile map with the Z (time) dimension
unfurled.

Example:

```
-5 sliced_tiled_output.json
```

### `-6`, `--patch-snapshot`


Filename of the run time POMS patch snapshot file to generate.

Example:

```
-6 snapshot.json
```

### `-7`, `--stl-snapshot`


Filename of the run time STL file to generate.

Example:

```
-7 snapshot.stl
```

### `-8`, `--tiled-snapshot`


Filename of the run time Tiled JSON file to generate.

Example:

```
-8 tiled_snapshot.json
```

### `-9`, `--tiled-slideshow-dir`


Directory to deposit run time Tiled JSON files generated.

Example:

```
-9 tiled_snapshot/
```

### `-N`, `--noise`


Add noise to individual tile probabilities.

Max of three parameters, colon (`:`) separated.

First parameter is the frequency, second is the seed and third is the type.

Type can be one of `0` for none, `1` for linear, `2` for threshold and `3` for tiered.

Example:

```
-N 10:11223344:1
```

### `-@`, `--viz`



Filename of the run time gnuplot visualization to use.

Example:

```
-@ snapshot.gp
```

### `-S`, `--seed`


Seed to use for the random number generator.

Example:

```
-S 1337
```

### `-O`, `--option`


Catch all for various other options.

Currently, the list is:

#### `ac4opt`

Which AC4 optimization to use.

One of `none`, `flat`, `tier4`, `tier4m1`, `tier4m2`, `tier6`, `tier6m1`

Default `flat`.

Example:

```
-O ac4opt=flat
```

#### `viz_step`

Frequency of snapshots.

Example:

```
-O viz_step=10
```

#### `retry`

BMS parameter to specify how many retries before softening.

Example:

```
-O retry=4
```

#### `erode_count`

POMS parameter to specify how many failures before an erosion occurs.

Example:

```
-O erode_count=3
```


#### `erode_p`

POMS parameter to specify erosion probability.

Start and end erosion probabilities can be separated by a colon (`:`)

Example:

```
-O erode_p=0.2:0.8
```

#### `patch-policy`

POMS parameter to specify patch choice schedule.

One of `x+y+`, `xpyp`, `x-y+`, `xnyp`, `x-y-`, `xnyn`, `rand`,
`conflict`, `cone`, `cone1`, `cone-`, `wf`, `wf-`, `wf2`, `wf3`.

`rand` default.

Example:

```
-O patch-policy=pending
```

### `-V`, `--verbose`


Set verbosity level:

* `-2`  - error
* `-1`  - warning
* `0`  - none (default)
* `1`  - iter
* `2`  - run
* `3`  - step
* `4`  - intrastep
* `5`  - DEBUG
* `6`  - DEBUG0
* `7`  - DEBUG1
* `8`  - DEBUG2
* `9`  - DEBUG3

Example:

```
-V 1
```

### `-v`, `--version`


Print out version.

Example:

```
-v
```

### `-h`, `--help`


Print out help.

Example:

```
-h
```

