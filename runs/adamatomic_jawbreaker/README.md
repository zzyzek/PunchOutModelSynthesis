Adam Saltsman's Jawbreaker
===

* [Jawbreaker on itch.io](https://adamatomic.itch.io/jawbreaker)

Tileset license: CC0

Results
---

| Example | Sample Run |
|---|---|
| ![input map](data/jawbreaker_fixed.png) | ![generated map](data/jawbreaker_128x128.png) |

| Simple ACCL | Frequency ACCL |
|---|---|
| ![jawbreaker simple ACCL](aux/jawbreaker_accl.png) | ![jawbreaker freq ACCL](aux/jawbreaker_freq_accl.png) |

Parameters
---

```
../../bin/poms \
  -C ./data/jawbreaker_poms.json \
  -b 1 -B 8 \
  -w 2 -E -1.7 \
  -S 1337 \
  -V 2 \
  -1 ./jawbreaker_128x128.json
```

Tile set creation
---

```
node ../../src.js/img2tile.js \
  -E ./data/jawbreaker_fixed.png \
  -P ./data/jawbreaker_poms.json \
  -T ./data/jawbreaker_tileset.png \
  -t ./data/jawbreaker_flat_tileset.png \
  -M ./data/jawbreaker_tilemap.json \
  -m ./data/jawbreaker_flat_tilemap.json \
  -s 8 \
  -w 16 \
  -D 128,128,1 \
  -q 128,128,1 \
  -W flat
```

Notes
---

The exemplar image had to be hand crafted so as not to be overly restrictive.

