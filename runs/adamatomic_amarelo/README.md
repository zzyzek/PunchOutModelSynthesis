Adam Saltsman's Amarelo
===


* [Amarelo on itch.io](https://adamatomic.itch.io/amarelo)

Tileset license: CC0


**WIP**

POMS has major problems converging for the Amarelo tile set,
so this run directory still needs some work to find example
runs that reliably work.

Currently, this has good success when straight BMS is used
(patch size is the whole quilt size).


| Example | Sample Run |
|---|---|
| ![input tileset](data/amarelo_sample.png) | ![generation](data/amarelo_128x128.png) |


| Simple ACCL | Frequency ACCL |
|---|---|
| ![simple accl](aux/amarelo_accl.png) | ![frequency accl](aux/amarelo_freq_accl.png) |


Parameters
---

```
../../bin/poms \
  -C ./data/amarelo_poms.json \
  -s 128,128,1 \
  -q 128,128,1 \
  -b 1 \
  -B 24:32  \
  -J 10000 \
  -w 1.5  \
  -E -1.25 \
  -P 'wf=xyz' \
  -O 'viz_step=50' \
  -O 'patch-policy=pending' \
  -S 1337 \
  -V 1 \
  -1 ./data/amarelo_128x128.json \
  -8 ./data/amarelo_snapshot.json
```

Tile set creation
---


```
node ../../src.js/img2tile.js \
  -E ./data/amarelo_sample.png \
  -P ./data/amarelo_poms.json \
  -T ./data/amarelo_tileset.png \
  -t ./data/amarelo_flat_tileset.png \
  -M ./data/amarelo_tilemap.json \
  -m ./data/amarelo_flat_tilemap.json \
  -s 32 \
  -w 64 \
  -D 128,128,1 \
  -q 128,128,1 \
  -W flat
```


Notes
---

To try and get more buildings in the middle, the height can be reduced.

This is a little bit of a difficult tileset so its sensitive to
the frequency used.
