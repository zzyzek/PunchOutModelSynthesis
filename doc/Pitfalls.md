Pitfalls
===

This is a scratch space for talking about some pitfalls with various ideas.


| |
|---|
| ![img/pitfalls.svg](img/pitfalls.svg) |

The figure above shows three types of pitfalls.

Figure `a)` shows a frustrated region in the middle of a "River Runs Through It" (RRTI) tileset.
The frustrated region will never resolve until the region migrates up to resolve the two
rivers into one or down, to resolve the one river into two, both at the boundary.

The frustrated region might have been consolidated with HEMP but will remain frustrated
until it reaches one of the two top or bottom edges.
Without biasing, this could have trouble finding a solution in a reasonable amount of time.

Figure `b)` shows that HEMP is still needed (?).

Figure `c)` highlights a pitfall of a potential heuristic which sequentially goes through chunks, applying
BMS only on those sub-regions.
This chunked-BMS heuristic would take care of both `a)` and `b)` but could fail on `c)` if it chose empty
space to fill in the corridor connecting the left and right regions of the grid.

---

So, one idea is to run the chunked-BMS on the graph to accelerate the realization of the frozen region.
Once found, HEMP can be used to try and resolve further.

HEMP will still have trouble with configurations as in Figure `c)` but there will always be problems that
will either be impossible or require higher level involvement to solve.
Other methods can also be layered on top to bias HEMP to move the regions together. For example, `A*` or
similar can be employed if there's a way to discover impossible paths.

---

An idea to help HEMP is to somehow create a gradient field associated with the entropy or frustrated
regions.
Somehow use the localized blobs of frustrated regions to produce a gradient field or Voronoi like region
that can be used to inform which way to bias the frustrated region placement.
I'm not sure if Dijkstra maps are totally applicable, but that's another source of inspiration.

The problem comes with understanding how to route around obstacles as it degrades into take the average
of some sort without it.
Localizing the frustrated regions effectively allows us to consider them as point-like sources.

---

Some random thoughts:

* Areas around the corners or edges are more constrained and so should have lower entropy, in some sense
* Two frustrated regions separated from each other, each of area $\frac{A}{2}$, should have lower entropy
  than a single frustrated region of area $A$ because the boundary, $\partial A < \partial \frac{A}{2} + \partial \frac{A}{2}$.
* Entropy driven Monte-Carlo (EdMC)

