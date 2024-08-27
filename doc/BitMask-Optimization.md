BitMask Optimization
===

###### 2024-05-03 UPDATE

This has been implemented with various tiers (tier4, tier6) and
various optimizations (different memory ordering for faster access/copying).
`tier4` has `(1,8,16,64)` bit options while `tier6` has `(1,2,4,8,16,64)` bit
options.
The motiviation for the choice is that bit length that is an exact multiple
of the byte or word size makes bit shifting and masking calculations a lot easier
and presumably faster.
The exact choice was motivated by two examples, discussed below, one for the
Kenney marble tileset which benefits from having a 1 bit tile support option,
but otherwise doesn't need too many other options, and the OLoZ tile set
which has a majority of tiles that have a spread of maximum tile supports from `1-255`.

A rough estimate is that, with proper memory ordering, it's about 10x slower
than the naive flat memory implementation and can save anywhere from
2.5-3.5x to 8-10x in memory footprint.

The OLoZ tile supports can be done in a byte of memory, with a spread across
1, 2, 4 and 8 bits needed for the tile support, giving a best estimate of
around 3.5x possible memory reduction by this method, with 2.5x memory reduction
being the more realistic estimate consider both runtime and memory footprint.

From the construction of the Kenney marble tile set, many tiles have only one neighbor
which gives a much better memory footprint reduction by this method as many tiles
only need 1 bit of support information.

The Kenney marble tile set was first restricted to only a subset of geometry provided.
This was done because of the underlying dock points for each of the tiles would have
needed to be something like 1/8 or 1/16 of a unit, needing 8x/16x (respectively) subdivision
of space to represent those tiles.
The tiles that needed the 1/8 or 1/16 were not that many and tended to be "bend" or "curve" tiles.
Instead of including them, they were discarded and the bulk of the rest of the geometry was
taken, with a fundamental subdivision of 1/2.

In addition to subdividing the space by a factor of 2,
many of the marble tiles are larger than a unit, so cross into other neighboring grid locations.
To represent these geometries as tiles, a new tile per occupided grid cell location is created,
with either an empty slot or the appropriate tile from the tile group as neighbors.

After all tile groups have been created, a post processing step is done to match the appropriate tiles
that have a dock on them to the appropriate tiles in other tile groups.
All tiles in a tile group have an "empty" geometry associated with them except for a representative
tile group element that has the underlying geometry associated with it, with the geometry translated
to the appropriate relative position of the representative tile associated with it.

Since most tiles in a tile group only have a single tile as its neighbor in each of the cardinal
directions, either the "empty" tile or another tile from the tile group,
most of the tiles can be represented by a single bit for the tile support.

For some concrete numbers, there are about 143 unique geometries after rotation and deduplication (from
about 57 base geometries).
From those 143 unique geometries, about 6673 (<6.7k) tiles are created.
Most of those 6k+ tiles have a single neighbor in each direction, with a small subset providing
connections to other tiles.

Since the naive AC4 implementation uses an `uint16_t` as its base storage, this allows for a maximum
of 16x memory footprint reduction when using variable tile support counters.
Using a four tier system of `(1,8,16,64)` bits, this gives about a 10x reduction in memory footprint.

Though there's some more verification that needs to be done, it looks like there's maybe a rough
10x speed slowdown from using the tiered tile support, using an appropriately ordered memory
layout.

Also, as a side note, there's a mini "bitter lesson" here as many of these memory optimization contortions
are alleviated by just increasing memory.
The first iteration of this was done on an 8Gb memory system which started thrasing when approaching 4Gb memory
usage.
Upgrading to a 64Gb memory system allows for some of the desired grid sizes (128x128 for OLoZ and 64x32x64 for marble)
with the haive `uint16_t` tile support option.

---

The idea is to use variable sized AC4 counts,
bit masks and pop counts to reduce the memory footprint.

The major culprits are the AC4 counters, the occupancy cell tile array.
To keep some semblence of performance, an auxiliary popcount on
the occupancy array can be used.

The main arrays we have to worry about:

* `m_tile`
* `m_tile_support`
* `m_prefatory`
* `m_cell_tile_visited`

An absolute must is that we can switch between representations without fanfare at
runtime.

The pin, entropy and other structures dependent on cells but not tile count are
most likely going to be small compared to the structures that need to store
cell count cross tile count, so can safely be ignored for this discussion.

---

From the rule graph we have an upper bound on each direction of the AC4 tile support.
Since this is a tile property, we only need to store which tile is associated with
with packed tile support tier.

---


If we only care about single cell entropy, without using the block entropy and without
using the distance modifier, we can discard that auxiliary structure.

---

`m_tile` and `m_prefatory` can be done via a packed bit field with an auxiliary popcount
structure for efficient-ish lookup.

---

`m_tile_suppot` can be implemented via a multi tiered packed representation, with 1 bit, 8 bits,
16 bits and 32 bits (maybe 64).

The 1 bit can be done wastefully, with 1 bit per cell, whereas the others can be done with a complicated
storage mechanism.

---

Here is an estimate:

| Structure | Terms |
|---|---|
| `m_tile` | $2 \cdot (\frac{N \cdot M}{8} + \frac{2 \cdot N \cdot M}{16})$ |
| `m_tile_support` | $2 \cdot (\frac{6 \cdot N \cdot M}{8} + \sum _ {k=1}^{s-1} b _ k \cdot C _ k)$ |
| `m_prefatory` | $\frac{N \cdot M}{8} + \frac{2 \cdot N \cdot M}{16})$ |
| `m_cell_tile_visited` | $\frac{N \cdot M}{8}$ |

Compared with the naive implementation with 2 bytes per AC4 support and `m_tile` entry:


| Structure | Terms |
|---|---|
| `m_tile` | $2 \cdot (2 \cdot N \cdot M)$ |
| `m_tile_support` | $2 \cdot (6 \cdot 2 \cdot N \cdot M)$ |
| `m_prefatory` | $2 \cdot N \cdot M$ |
| `m_cell_tile_visited` | $N \cdot M$ |


* `m_tile`:  one bit per cell cross tile plus the population count needing 2 bytes to hold the running total at a stride of 16 (16 bits cover per popcount entry)
* `m_tile_support`: $b _ k$ is the bytes used at tier $k$ and $C _ k$ is the count of tiles needing to be stored for tier $k$, with tier 0 being assumed to be be wasteful and use 1 bit per entry, even if its not needed


See `sandbox/estimate-mem.js` for more detail. Here are some sample runs:

```
profile: oloz
N: 65536 ( 256 256 1 ) M: 11979
----
naive estimate: (b: 2 )
  m_tile: 2.924560546875 Gb
  m_prefatory: 1.4622802734375 Gb
  m_tile_support: 35.0947265625 Gb
  m_cell_tile_visited: 0.73114013671875 Gb
total: 40.21270751953125 Gb
----
estimate w/ bitpacked m_tile & m_prefatory, 4 tiered m_tile_support, bitvector m_cell_tile_visited
  bv tile: 0.365570068359375 Gb
  bv pref: 0.1827850341796875 Gb
  tiered ac4: 4.9362945556640625 Gb
  bv visited: 0.1827850341796875 Gb
tot: 5.6674346923828125 Gb
```

```
profile: marble
N: 1048576 ( 128 64 128 ) M: 4137
----
naive estimate: (b: 2 )
  m_tile: 16.16015625 Gb
  m_prefatory: 8.080078125 Gb
  m_tile_support: 193.921875 Gb
  m_cell_tile_visited: 4.0400390625 Gb
total: 222.2021484375 Gb
----
estimate w/ bitpacked m_tile & m_prefatory, 4 tiered m_tile_support, bitvector m_cell_tile_visited
  bv tile: 2.02001953125 Gb
  bv pref: 1.010009765625 Gb
  tiered ac4: 6.587890625 Gb
  bv visited: 1.010009765625 Gb
tot: 10.6279296875 Gb
```

Implementation
---

### `m_tile_bv`

For the `m_tile`, we need a data structure with the following API:

| Function | Description |
|---|---|
| `rank(s,e)` | returns number of bit set in range from $\{s \dots (e-1)\}$ |
| `val(p,v)` | set bit $p$ to val $v$ if $v \in \{0,1\}$, return value at position $p$ |
| `pos(idx)` | return bit vector position of the `idx` set bit |

We can represent this by two structures, one a bit vector array and the other a tree
of counts for each subtree with leaves holding a strides worth of bit counts.

```
typedef struct bvpc_type {
  int64_t n;
  uint8_t stride;

  uint8_t *bv;
  int16_t *rank;
} bvpc_t;
```

Where `.rank` will be a fully balanced tree represented as an array.

The assumption here is that the size and stride will be static where the bit vector
will be dynamically updated.

One reason to require the `rank` function is because we need to enumerate
by remaining tiles that are set or access elements by their index of remaining
set values.
Rank updates and queries can be done in $O(\lg n)$ time.
Space requirements are $\lceil \frac{n}{8} \rceil + 2 \cdot \lceil \frac{n}{\text{stride}} \rceil \cdot b_s$,
where $b_s$ is the number of bytes needed for the rank counter, in our case, typically 2.



### `m_tile_support_tier`

For the tile support, we need to represent the support coming from each direction
for each tile for each cell position.
Counts can have a wide range but have an upper bound, implied by the rule graph,
so we want to represent the support counts from a set of different counter sizes.

Here's the proposal:

```
typedef struct tile_support_vb_type {
  int64_t m_cell_count;
  int32_t m_tile_count;

  int8_t  n_tier;
  int8_t  *tier_size;

  int8_t  *tile_tier;

  void    **support[6];
} tile_support_vb_t;
```

The API:

| Function | Description |
|---|---|
| `setCellTile(cell,tile,val)` | ... |

Where `.tile_tier` is of length `.m_tile_count` and holds a code to indicate which
tier is being used for that tile and `.support` is `.support[<idir>][<tile>][<cell>]`
and holds the array of packed support counts.
`.n_tier` and `.tier_size` holds the information about the different tiers.

It might be better to interleave the direction so try and take advantage of locality
but maybe we can play around with that in the future.


### Update

###### 2024-04-21

AC4 tiering is implemented, for 4 and 6 tiers (`{1,8,16,64}` and `{1,2,4,8,16,64}` tiers resp.) and it looks
to be working.
For OLoZ the memory reduction is anywhere from 2.5 to 3.5 and for marble it looks to be 8-10.
For OLoZ, the rule graph has adjacency less than 256, with about $\frac{1}{4}$ only needing one bit and
the rest needing more, so the 2.5-3.5 reduction from a naive 16bit storage is reasonable.
The marble tile set has many tiles that only have one neighbor, as an artifact of the tile creation process,
so this is where the 8-10 reduction comes as a large portion of the tiles get a 16x reduction, with others
needing more storage.

Unfortunately the run times are 10x or more slower.
I suspect this has to do with memory layout as successive accesses for neighboring tiles and cells will
be very far apart in memory now, but I'm not sure.
I think it's going to be just as easy to implement and test on real data than to try and isolate and test,
so here's the proposal:

```
// m_tile_bit_size:     size, in bits, of tier
// m_tile_tier:         what tier tile is in (|m_tile_tier| = m_tile_count)
// m_tile_tier_count:   how many tiles are in tier (|m_tile_tier_count| = n_tier)
// m_tile_offset:       index offset in bit packed representation
// m_tier_group:        packed bits for each tier's tile grouping
//
// each tier has subset of tiles packed together in their bit representations:
// cell_j[  tile_{d_0,g_0}:[s_0 .. s_{t_k-1}] ... tile_{d_5,g_0}:[s_0 .. s_{t_k-1}]
//          tile_{d_0,g_1}:[s_0 .. s_{t_k-1}] ... tile_{d_5,g_1}:[s_0 .. s_{t_k-1}]
//          ...
//       ]
//
std::vector< int8_t >   m_tier_bit_size;
std::vector< int32_t >  m_tier_tile_count;

std::vector< int8_t >   m_tile_tier;
std::vector< int32_t >  m_tile_offset;
std::vector< std::vector< uint8_t > >   m_tier_group;
```

* `m_tile_tier` holds which tier the tile is in
* `m_tile_offset` holds the index order of where in the tier the tile is
* `m_tier_group` holds the packed data, for each tier, for each tile in the
  tier and each direction for that tile in the tier.

We make the simplifying assumption that tier bit sizes are powers of two.
For tier bit sizes greater than 8, access is easy.
When tier bit sizes are less than 8, the following hopefully encapsulates the
idea:

Here, we are given `tile` at `cell` with `idir`:

```
tier    = m_tile_tier[tile]
offset  = m_tile_offset[tile]

tilecount = m_tier_tile_count[tier]
bitsize   = m_tier_bit_size[tier]

bit_start_pos = (cell*tilecount*bitsize*6) + (offset*bitsize*6) + (bitsize*idir)

byte8 = m_tier_group[bit_start_pos/(8/bitsize)]
```

We've done at least two more accesses to auxiliary vectors but the packed
bit vector should have more locality both in terms of neighboring tiles and cells
but with direction as well.

