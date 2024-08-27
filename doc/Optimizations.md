Optimizations
===

This is a rough document outlining some of the optimizations and the motivation
for choosing them.

Use `AC4` over `AC3`
---

An `AC4` algorithm was chosen over an `AC3` algorithm because of the large tile count.

`AC3` has a cubic domain factor which is effectively a constant for small or fixed domains.
As domains increase, that factor dominates and becomes prohibitive.

`AC4` has a quadratic domain factor which scales better.

Briefly, `AC4` works by having a `support` counter for each valid tile at each cell
location for each direction.
If this support counter is ever `0` in any direction, the tile can be removed.

Initially, the support counters need a full sweep but this can be updated incrementally
as tiles are removed.

The big drawback is a large memory footprint but in most cases `POMS` cares about, this
is a valid trade off.

Expected speedup: $O(D)$

Process min of valid neighbors or tiles left in cell for `AC4` support
---

Naively, one might iterate through all neighboring tiles when updating support for
an individual tile.

Many tilesets have an average degree which is much lower than the number of tiles.
That is, the rule adjacency matrix is very sparse.

When calculating support, this optimization iterates through only the valid
neighbors of a tile, checking to see if they exist in the neighboring cell.
If the neighboring cell has fewer tiles left than the tiles adjacency list,
iterate through the cells tiles instead.

The `m_tileAdj` is the vector of vectors that holds a list of valid neighbors
for each tile.

Expected speedup: $O(D)$

Dirty cells
---

During the course of various algorithms, the grid needs to be saved, restored
or entropy on a cell level needs to be calculated.

Only those cells that have been touched since the last update need be processed,
relieving the effort of copying the whole grid.

What turns out to be overly costly is not just processing cells that have remained
unchanged but the tiles within them.
This causes various tasks to scale non-linearly as the tile count effectively adds
another dimension.

Processing only marked cells helps alleviate this non-linear scaling by requiring
only a potentially small subset of cells to be processed during the normal
coarse of the algorithm.

Expected speedup: $O( \frac{ N _ x \cdot N _ y \cdot N _ z }{ I _ r } )$

Misc.
---

Other optimizations that are lower level:

* Keep a "back pointer" array (`m_tile_bp`) so that tile in cell queries and tile
  removal can be done in constant time
* I've been fiddling with data sizes but a reasonable choice is `int16_t` for support,
  `int8_t` for visited, `int32_t` for cell tile queue. This is not so much a runtime
  speedup as trying to keep the memory footprint down

###### 2024-03-13

