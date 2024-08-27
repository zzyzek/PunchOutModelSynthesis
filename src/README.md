POMS Source Files
===

This directory is loosely structured as follows:

| | |
|---|---|
| `poms.hpp` | POMS class definition and helper functions |
| `pomsLoad.cpp` | POMS JSON config file processing |
| `pomsExport.cpp` | Export functions (Tiled, POMS, etc) |
| `pomsAlgorithm.cpp` | WFC, MMS, BMS, HEMP algorithm implementations |
| `pomsConstraintPropagation.cpp` | AC4 and other implementations |
| `pomsEntropy.cpp` | Entropy functions (cell choice, block choice, etc.) |
| `pomsDebug.cpp` | Debug utilities |
| `main_poms.cpp` | Linux command line program |

Grids are cuboids (rectangular prisms) whose dimensions are defined by the `m_size` array.
Currently, only fixed boundary tile conditions apply but this might change in the future.

In the following, $M$ is the number of tiles and $X$, $Y$ and $Z$ represent the size of the cuboid in each
dimension with $N = X \cdot Y \cdot Z$.
The $D$ value is twice the number of dimensions which, in our case, is `6` ($6 = 2 \cdot 3$).

$\mu$ is the average tile adjacency count.

Structures:

| Name | Size | Description |
|---|---|---|
| `m_tile_count` | $1$ | Number of tiles ($M$) |
| `m_cell_count` | $1$ | Number of cells ($N$) |
| `m_tile_rule` | $D \cdot M \cdot M$ | Admissible tile pairings in each of the index directions |
| `m_tileAdj` | $D \cdot M \cdot \mu$ | Tile pairings with direction and tile as indices to an array of admissible neighbors |
| `m_tile_weight` | $M$ | Individual tile weights (probabilities calculated dynamically depending on cell occupancy) |
| `m_tile_name` | $M$ | String array of tile names |
| `m_size` | $3$ | Size of block ($X =$ `m_size[0]`, $Y =$ `m_size[1]`, $Z =$ `m_size[2]`) |
| `m_tile` | $M \cdot  N$ | Block state where each cell holds a list of tiles |
| `m_tile_size` | $N$ | Number of tiles at cell position |
| `m_tile_bp` | $M \cdot N$ | Back pointer into `m_tile` of what a tiles index is into `m_tile` ( `m_tile[ m_tile_bp[tileVal] ] == tileVal` ) |
| `m_dir_oppo` | $6$ | Index of opposite direction ( `[1,0,3,2,5,4]` ) |
| `m_visited` | $N$ | Used by constraint propagate to mark visited cells |
| `m_cell_pin` | $N$ | Array of which cells are pinned (block) |
| `m_cell_queue` | $N$ | Used by constraint propagate to keep a list of cells to consider (AC3) |
| `m_cell_queue_size` | $1$ | `m_cell_queue` number of entries |
| `m_tile_support` | $D \cdot M \cdot N$ | Arc support of each tile in each direction (AC4) |
| `m_cell_tile_visited` | $M \cdot N$ | Used by constraint propagate to mark visited `(cell,tile)` pairs (AC4) |
| `m_cell_tile_queue` | $2 \cdot M \cdot N$ | Used by constraint propagate to keep a lit of `(cell,tile)` pairs to consider  (AC4) |
| `m_cell_tile_queue_size` | $1$ | `m_cell_tile_queue` number of `(cell,tile)` entries (AC4) (`*`) |
| `m_prefatory` | $M \cdot N$ | Holds the prefatory state of the grid |
| `m_prefatory_size` | $N$ | Number of tiles a cell position in `m_prefatory` |
| `m_entropy` | $N$ | Cell level entropy |
| `m_block_entropy` | $N$ | Block level entropy |
| `m_block_size` | $3$ | Size of block in each dimension |
| `m_block` | $6$ | Start and non-inclusive end of block (dynamic updated) |
| `m_soften_size` | $3$ | Size of soften block in each dimension (soften block will be of size `2 * m_soften_size[.] + m_block_size[.]`)  |
| `m_soften_block` | $6$ | Start and non-inclusive end of soften block (dynamic updated) |
| `m_quilt_tile` | $N _ G$ | Grid tile state (quilt). Holds tile at grid position or `-1` for indeterminate. |
| `m_quilt_pin` | $N _ G$ | Indication of whether grid cell is pinned |
| `m_quilt_size` | $3$ | Size of grid ($X =$ `m_quilt_size[0]`, $Y =$ `m_quilt_size[1]`, $Z =$ `m_quilt_size[2]`) |
| `m_quilt_cell_count` | $1$ | Number of cells in grid (quilt) |
| `m_patch_region` | $6$ | Rectangular cuboid region, start and end, for each dimension |
| `m_objMap` | $M$ | Array of strings to `obj` files for helper function to render output scene (to STL, say) |
| `m_ac4_dirty` | $N$ | Array of to hold which cells need updating for AC4 (block) |


`*` - Note: `m_cell_tile_queue` has two elements per position, so `m_cell_tile_queue_size`
      will always be half the in memory stored number of elements for `m_cell_tile_queue`.
| Name | Description |
|---|---|
| `m_seed` | Seed for PRNG |
| `m_zero` | Epsilon value to consider 0 |
| `m_eps` | Epsilon value |

When constraint propagation fails, conflicts are stored in the following elements:

| Name | Description |
|---|---|
| `m_conflict_cell` | Conflicting cell |
| `m_conflict_tile` | Conflicting tile |
| `m_conflict_idir` | Conflicting direction index |
| `m_conflict_type` | Conflict type, Currently `POMS_CONFLICT_BOUNDARY`, `POMS_CONFLICT_NO_SUPPORT`, `POMS_CONFLICT_EMPTY` |

| |
|---|
| `F` |
| `G` |
| `vec2cell` |
| `cell2vec` |
| `xyz2cell` |
| `neiCell` |
| `cellTile` |
| `cellTileIndex` |
| `cellSize` |

