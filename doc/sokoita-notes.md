Sokoita Notes
===

###### 2024-12-12

Sokoita is a constraint based tiling generation (CBTG) problem Sokoban solver using
Breakout Model Synthesis and, potentially later, Punch Out Model Synthesis.

Initial results aren't promising but this document is here to write
down the idea, work so far and some future ideas.

Setup
---

A sokoban solver can be created using the CBTG framework by creating
tiles and constraints that encode local states of a Sokoban level and their
valid transitions.
The $z$ dimensions holds the temporal aspect with the $x$ and $y$ holding
a state in time.

There are 7 "flat" tile types, taken from the `XSB` file format:

* `#` wall
* ` ` moveable space
* `$` crate
* `.` goal/storage
* `*` crate on storage
* `@` player
* `+` player on storage

Along with an empty (`_`) tile used internally for the boundary/non-displayable
tile (this is an artifact of how POMS/BMS sets up the problem and needs another
tile that can be put out of bounds of the grid for convenience).

A super tile is created using a cross pattern with the center tile
being the displayable tile.

Here is an example:

```
  #
# @ $
  .
```

Here, there is a wall on top and left, a crate on the right, a goal/storage on the
bottom and the center, displayable tile, as the player.

To build the super tile library, each of the 5 positions is enumerated ($7^{5}=16807$ possibilities)
and then whittled down by removing invalid super tiles.

Invalid super tiles are super tiles that have:

* more than one player
* have a non wall simple tile surrounded by walls

A special "empty" super tile is created that is used as a virtual tile for connections
out of grid bounds.

The total super tiles is $9,370$.

Super tiles are further marked as being transitional, goal or traps, where:

* `transitional` - has at least one crate not on a platform or one platform not occupied by a crate
* `goal` - any goals or crates are on a platform
* `trap` - a crate (in the center space) is deadlocked by walls

Rules in the $(x,y)$ plane (holding $z$ constant) are created by finding a `2x1` overlap to other super tiles.

$z$ transition rules are created from valid Sokoban rules, which are:

* Super tiles without a player can transition to itself
* If a player is at the center of a super tile, they can move to a non-wall space,
  moving a crate and transitioning the player and/or crate in the process if it goes
  on a storage tile
* If a player is at the border, they can move out or move to the center, transitioning
  the simple tile as appropriate
* If a player doesn't exist in the super tile, a player can appear on the border of a non-wall
  space in a super tile
* If a player doesn't exist in the super tile, a crate can appear on the border of a non-wall
  space in a super tile

Note that stationary player transition isn't allowed, so the player must always move.

The $z$ transforms encode the "mechanics" of the Sokoban play.

A note:

> the cross pattern observes a "conservation of players and crates" as no crates or
> players can be created or destroyed.
> Using a `2x2` window, for example, doesn't reach far enough in each of the major dimensions
> to encode these mechanics.
> Larger super tile windows can be used but lead to a combinatorial explosion.
> The cross pattern offers a compromise between providing enough space for the mechanics
> while keeping the super tile count manageable.

Empty tiles can be next to any tile in the $z$ direction.
If a super tile has two simple wall tiles in line, a rule to the empty super tile
is added in the appropriate direction.

The final rule count is $8,903,224$.

Level Setup
---

To solve a particular level, the level is first normalized by filling in all "moveable"
spaces with walls.

A POMS file is created that:

* chooses a user specified $z$ parameter, $z _ {fin}$, that represents the maximum number of moves allowed
  moves to solve the level
* removes all empty (0) super tiles from the whole grid
* removes all trap super tiles from the whole grid
* has constraints in the $z=0$ plane that force to the super tiles as they appear in the `XSB` file.
* removes all transition tiles from the $z _ {fin}-1$ plane

With these constraints, the initial $z=0$ plane is forced to the initial configuration and the last
configuration is forced to only allow for a solution.


Setup Discussion
---

The above just sets up the mechanics of Sokoban formulated as a CBTG problem and then provides constraints
so that the only valid configuration is a solution.

From some initial experimentation, the setup by itself won't have a good chance of finding solutions
and struggles to find solutions even for a small test setup.

From observation, there are a few immediate dynamics that jump out during an attempted solution:

* players can appear anywhere on the grid
* multiple players can appear
* more crates can appear than were originally specified

These all happen because the solver is local and if it tries to solve a portion of the grid
that's buffered by a frustrated region, a locally consistent but globally inconsistent configuration
can appear where multiple players can appear, players can appear much further then they could
move and multiple crates can appear.

In order to help the solver, additional heuristics can be employed to try and further refine
allowable tile choices.

Heuristics
---

*WIP*

### Redundant State Check

If an identical fully realized $(x,y)$ plane is found, it can be considered
a contradiction as identical states represent no progress.

Care has to be taken as this can lead to a global contradiction if $z _ {fin}$ is large enough a
redundant state is guaranteed.

### Player and Crate Count Check

After resolution a player check can be performed, looking for any cell locations
that have either a fully resolved player or tiles that only have players at their center
cells.

If the count of these number of players in an $(x,y)$ plane is greater than 1,
this can be considered a contradiction and softening can be imposed.

The same can be done for crates but now making sure the create count in an $(x,y)$
plane doesn't exceed the number of crates in the level.


### Maximum Player Move

A flood fill can be done, flooding backward and forward in time, using the a resolved
player cell as the starting point.
The flood fill gets stopped by walls and crate cell locations but goes through a cell
if at least one tile has a moveable space.

The intersection of the flood fills is done to find the admissible player locations.
Any tile that has a player at its center is then removed from the grid.

### Maximum Crate Move

(not implemented)

The same idea can be done for crates but some care has to be taken because crates
can only be moved by players.

Only start the flood fill for $(x,y)$ planes that have the full crate count.
At the very least, this should be the first and last configuration.

For every crate cell in the full crate $(x,y)$ plane, use that as the start for the flood fill but only allow a
flood to a cell if the there's at least one player center tile in the direction.
As with the player, flood fill backward and forward in time ($z$).

Keep the intersection of all floods and then remove crate center tiles from
the grid at every location not touched by the flood.

This should be done after the player flood fill to further restrict the space.

### Knockout Patterns

Some patterns are higher order deadlocks.
If they can be detected, then either an early backtrack detection can occur if the pattern
is found or a tiles from a single cell position can be removed ("knocked out") if the rest
of the deadlock pattern is scene.

For example, the following is a higher order deadlock pattern:

#### A

```
-----
-###-
-# $-
-#$--
-----
```

#### B

```
-------
-#####-
-# $ #-
-------
```

#### C

```
-------
-####--
-#  $--
-#$$---
-------
```

In the example of `A` if the following pattern is encountered:

```
-----
-###-
-# x-
-#$--
-----
```

Then we know we can remove any super block with a crate at its center 
at position `x` as this leads to a deadlock.

One option to is to precompute a pattern library, doing exhaustive search
or hand crafting examples, then try to match them during run-time.
Another is to try to create a library at run time to try and adapt
the search to the specific level encountered.

A simple test for knockout patterns can be setup by creating a small temporal example
with the initial pattern restricted to the initial, test, knockout pattern, and the
end pattern empty for non-wall positions.
If the final configuration can be achieved, through simple constraint propagation,
we know this isn't a knockout pattern.
Alternatively, if we do find a contradiction, we know it's a deadlock pattern.

The point here is that if constraint propagation can be used to find a contradiction,
for some suitable $z$ value, we can avoid doing the full tree search while still
having a small proof of non-validity.

For example, if `A` were set up with a boundary pinned and all super tiles with the
appropriate centers forced:

```
&&&&&&& &&&&&&&     &&&&&&& &&&&&&&
&-----& &-----&     &-----& &-----&
&-###-& &-###-& ... &-###-& &-###-&
&-# $-& &-#---& ... &-#---& &-#  -&
&-#$--& &-#---& ... &-#---& &-# --&
&-----& &-----&     &-----& &-----&
&&&&&&& &&&&&&&     &&&&&&& &&&&&&&
```

(`&` pinned wildcard).

If there's a contradiction, we know it's a deadlock.

```
&&&&&&&&&&&     &&&&&&&&&&&
&---------&     &---------&
&-#######-&     &-#######-&
&-# $   #-& ... &-#     #-&
&---------&     &---------&
&&&&&&&&&&&     &&&&&&&&&&&
```

Here the crate (`$`) can move within a 3 space region but not below,
so is a deadlock configuration.





Discussion
---

The fundamental assumption is that random player and crate moves will be able to find
solutions, given enough guiding.
If this method works at all, it will be because the search space is constrained enough
to make local progress with a minimum of global information.
Knockout removal will help guide the local search and global state removal can further refine
the search.

Global information will almost surely be needed and it's unclear at this time how much
in order to make reliable progress.
There is the possibility of breaking down the larger problem into independent sub-problems
but I don't see how to do that effectively.

I don't have high hopes for this method as it seems like there's going to need to be more
long term planning to find solutions.


###### 2024-12-16

I've pretty much reached an impasse. The solver is pretty abysmal and it's not clear to me
how to make progress.

It seems like it should do better than it is but it's clear, in its current state,
it's not very powerful and gets easily confused.

Some thoughts:

* The cell count is in mostly in the single to low double digit count, meaning
  that the space of possibility is vastly reduced for a given level
* Better visualization is probably the way forward, to try and figure out what the
  issues are and how to address them. Some ideas for visualization and misc.:
  - confirm values that should be removed, either first order or higher order,
    are getting removed
  - frequency of each center tile in the super tile
  - frequency of player and crate movement
  - overlay of crate and player movement direction options
* Instead of implementing knockouts or the like, try and get a sense for how
  much benefit it would actually give by counting beforehand
* The player count restriction hurts the validation0 level, presumably because
  it gets locked into a player in an area that's too far away
* Atomic moves are really player movement to adjacent crates, so they're non-local
  in that sense. I don't see an easy way to coerce the solver into nudging the player
  to the valid next move location. Some ideas that probably won't work:
  - knockout player center tile at (x,y,z+1) if player at (x,y,z) and (x,y,z-1)
    ('no-return' mode)
  - knockout all but player valid moves to next crate location (finicky as crates
    need to be fully resolved)

Also, providing sample levels where higher order planning and moves are effectively
forced might be a better route to understand where the solver gets hung up.

I maintain constraint propagation can still be powerful for understanding the search
space, especially if it has some relation to belief propagation, but I suspect a
better model/method is more appropriate for this problem.

For example, doing Monte Carlo Tree Search and represent the next valid state as
the atomic move (with move cost) to a neighbor crate, then representing the implied
state as a graph that then can use CSP to provide some extra inference.
Adjoining some neural network to improve pattern matching on which direction to prioritize
might provide a big boost in solver power.

As previously discussed, this method relied on the assumption that stochastic moves of player
or crates could result in a solution.
I suspect that this pretty much isn't true, at least as setup by this CBTG problem.
If we really wanted to validate this idea, we can make a pure stochastic solver, with some
states to avoid, to see how well it would do.

  

