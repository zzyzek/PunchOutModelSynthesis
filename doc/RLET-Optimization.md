Run Length Encoded Tree Optimization
===

###### 2024-05-03

UPDATE: this optimization turns out not to be effective.
The spatial entropy of tile distribution isn't very nice
so the compressions gains aren't really realized.

Further, the cost of the data structures to manage
everything is many orders of magnitude slower than a naive
flat memory implementation.

I think the core idea of effectively using run length encoding (RLE)
compression by mapping N-d space to a 1-d curve is interesting but
I don't see how it's useful in this context.
Since it neither gives memory reduction or speed increase,
this optimziation has been abanonded.

---

Two major concerns when implementing
tiled constraint satisfaction problem are run-time speed and space.

For the problem in question, AC4 achieves optimal runtime
of $O(N \cdot D^2)$ in general, with the only possibility
of a speedup should there be extra structure in the problem
setup that can be exploited.
I suspect that for this problem, there could be additional
reductions of the $D$ dependence by considering the average
degree of the implied relation graph to the constraints,
giving an $O( N \cdot D ^ 2 _ { * })$ with $D _ { * }$ the
average degree of the implied relation graph.

For a uniform grid of $N = N _ x \cdot N _ y \cdot N _ z$ with $D$
tile values in the domain, a straight forward implementation
which stores values outright, would take $N \cdot D$ space.
When $D$ is large, this can be an unwieldy requiring potentially
`Gb` of storage for even moderately sized problems.

One idea is to take advantage of the way many algorithms in this
domain work to exploit the redundancy that results.
For example, many algorithms start from an indeterminate,
or wildcard, state and the proceed to resolve on a wave-front,
leaving completely resolved or very nearly completely resolved
cells in its wake.

For an implementation that holds a static array of domain values
for each cell will, in many cases, mostly be in one of two states,
with either all domain values being available in that cell (wild-card) or only
one domain value being present after resolution (resolved).
Even if the individual cell arrays were to be dynamically allocated,
the initial configuration would still need to hold the full compliment
of the domain.

If a generic problem effectively has two states, nearly fully resolved
and wildcard, then allocating extra storage to potential hold
all intermediate states for most of the lifetime of the algorithms
run-time is wasteful.

To help with this problem, a data structure can be introduced
that exploits the homogeneity of the system.

Call $\delta$ the average size of a run length encoding restricted to the
tile domain, presumably a small constant factor.
Here are three proposals:


* Slice the spatial domain into $D$ vector planes (option `opt.0`)
  - store a run length encoded bit vector that indicates whether
    a tile is present or not
  - map the linear vector onto the 2 or 3 dimensional region in some
    way
    + a natural choice is a generalized Hilbert curve to try and exploit
      spatial redundancy
  - tile existence test: $O( \lg N )$
  - tile index lookup: $O( D )$
  - space: $O( N \cdot \delta )$
* Keep a static array of $N$ cells (option `opt.1`)
  - at each cell, use a linear biased run length encoded scheme to store
    runs of increasing sequence reprsenting the valid domain values at
    that location
  - tile existence test: $O( \lg D )$
  - tile index lookup: $O( \lg D )$
  - space: $O( N \cdot \delta )$
* Think of the whole system as a long bit vector with each cell containing
  $D$ bits, chained together. (option `opt.2`)
  - tile existence test: $O( \lg (N) + \lg (D) )$
  - tile index lookup: $O( \lg (N) + \lg (D) )$
  - space: $O( N \cdot \delta )$


To allow for efficient tile index lookups for `opt.1`, a population count
mechanism is required,
which can be added to a self balanced tree implementation at no extra
computational complexity cost.
To allow for efficient tile index lookups in `opt.2`, a population count
mechanism is required,
which can be added to a self balancing tree implementation at no extra
computation complexity cost.

With the above in mind, while the generalized Hilbert curve idea is
interesting, it doesn't add any benefit compared with the other two
optimizations.

`opt.2` adds an $O( \lg N )$ overhead to each call while still requiring
the same generic memory size as `opt.1`.

So it looks like `opt.1` is the winner.


---

Normally, a rectangular grid would be set up with
structures to store the tiles remaining at each grid
cell position and support counters for the AC4 algorithm
in each of the neighboring directions.

The hope is that the tiles and support values will be mostly
homogeneous, allowing for efficiency in storage since
there will be contiguous identical values.

The supporting structure is a self balancing tree, for example a
red black tree, whose nodes have been augmented to hold
an integral value, an interval consisting of a start and end value,
which will be called a tree run length encoded (TRLE) structure.

Functionally, the TRLE should be a drop in structure, simulating
a simple array.

Initially, the tree is set up with one node of some value with
0 start and $N$ end, where $N = N _ x \cdot N _ y \cdot N _ z$.

The basic API is as follows:

| Function | Effect | $O(\cdot)$ |
|---|---|---|
| `TRLE.init(s,e,v)` | initialize a `TRLE` with an interval of $\[s,e)$ and value $v$ | $O(1)$ |
| `TRLE.update(p,v)` | add value $v$ at index position $p$ | $O(\ln N)$ |
| `TRLE.read(p)` | get value at index position $p$ | $O(\ln N)$ |

The `TRLE.update()` function finds the appropriate node, splits the interval adding
in the new entry between two new nodes, if necessary.
As all these operations are happening on a self balancing binary tree, they happen in $O(\ln N)$ number
of steps, allowing for efficient reading and updating.

---

With the TRLE structure as above, we can now use this to simulate flat arrays.
A space filling curve can be used to map the rectangular region to a linear index for use with
the TRLE.
The hope is that the space filling curve maintains enough local consistency to effectively
exploit the contiguous regions of identical values, should they appear.

There is a separate TRLE structure for each tile, effectively a bit vector, indicating whether
a tile is admissible at the cell referenced by its index.
There are six TRLE structures for every tile allowing for the auxiliary support count information
needed by AC4.

```
vector< TRLE > m_trle_tile;
vector< TRLE > m_trle_support[6];
...

m_trle_tile[tile_val].read( cell_id ) ...
m_trle_tile[tile_val].update( cell_id, 1 ) ... // tile_val @ cell_id now admissible
m_trle_tile[tile_val].update( cell_id, 0 ) ... // tile_val @ cell_id now inadmissible

m_trle_support[0][tile_val].read( cell_id )  ... // idir=0 (x+) for tile_val @ cell_id AC4 support
m_trle_support[0][tile_val].update( cell_id, support )  ... // idir=0 (x+) for tile_val @ cell_id AC4 support

```



---

The major drawback of this method is that if the implicit array doesn't have
large contiguous regions of identical values, the memory footprint will be
many times larger than storing them in simple arrays.

At the time of this writing, it's unclear whether this method can give significant improvements.

### Notes on Implementation

* `m_ac4_opt` should be set to `2` and `AC4Init` and `AC4Update` should have a conditional to accept it.
* New functions `AC4Init_trle` and `AC4Update_trle` should be implemented (`pomsConstraintPropagate.cpp`)
* `saveGrid/restoreGrid` should have alternate paths for whether we're using `TRLE` (`pomsAlgorithm.cpp`)

There are only a few places where `m_tile` (and related) are used outside of their accessor functions.
Those places should be updated to account for the trle portions.


* `cellTileIndex` is only used basically in one place which can be replaced easily
* `cellTile` is the main accessor function which assumes a tile index to get the value.

Some random thoughts:

* Storing information per cell is not that bad, it's the `(cell) x (tile count)` that's the issue
* The current implementation has an optimization that only considers tiles left in a given cell.
  This can be done, with fixed length memory, but keeping the number of remaining tiles and
  then swapping them out as they're discarded.
* To replicate the number of tiles left functionality, options:
  - keep a (linear biased?) TRLE that would allow us to get successor tiles efficiently.
    This is not great as this could balloon in size for bad configurations.
  - keep a pop count for a run of tiles at a cell position and use that to skip over
    some stride (e.g. 256) of them if pop count is 0.

We're still in validation stage.
So doing a straight enumeration of tiles per cell without the tile removal optimization
is probably the most straight forward thing to do.
The issue is whether this will provide enough validation of the method.

I'm thrashing because the interchange of rows for columns is essential invalidated
when storing fine grained tile list information on a cell level.
Pop count could be an optimization but this is highly dependent on the data and
is a non-trivial optimization.

Keeping a linear biased TRLE for each cell is probably a drop in replacement for most.

To be more verbose, a linear biased TRLE can have a node that has a start and end range
to implicitly store the tile from start to end (non inclusive).
The jump from one node to another represents 'no tile present'.
Successor operations can be done to get the next run.
As with any RLE structure of this sort, a checkerboard pattern represents the worst case.

Both these optimizations could be lucrative or very bad, depending on the situation.
From observation, the spatial RLE could be lucrative since there seem to be two major
regions, one that is completely determined and one that is almost completely indeterminate,
with the interface being highly variable.
If the interface is restricted, presumably to a band that's proportional to the size of the
influence radius, this gives bounds on how memory intensive this optimization can get.

For the lineary biased TRLE (lb-TLRE) for tile information per cell, the same observation manifests as having 
one lb-TRLE with one node per cell for realized regions, effectively one lb-TRLE for unrealized regions
and a band as above.




###### 2024-02-05

