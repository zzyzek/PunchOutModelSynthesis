Adam Saltsman's Amarelo
===

* [Amarelo on itch.io](https://adamatomic.itch.io/amarelo)

Tileset license: CC0

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
  -b 1 -B 32 \
  -w 1 -E -1.5 \
  -S 1337 \
  -V 2 \
  -1 ./amarelo_128x128.json
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
