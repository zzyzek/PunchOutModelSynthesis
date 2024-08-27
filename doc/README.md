Punch Out Model Synthesis Auxiliary Documentation
===

This sub directory cosnists of some notes on ideas directly or indirectly related
to the Punch Out Model Synthesis project.

| File | Description |
|---|---|
| [`README.md`](README.md) | This file |
| [`BitMask-Optimization.md`](BitMask-Optimization.md) | An optimization idea to use variable integer sizes for AC4 counts |
| [`References.md`](References.md)  |  References for WFC, MMS, CBTGs, CSPs, etc. |
| [`Pitfalls.md`](Pitfalls.md) | Rough notes on some global constraint pitfalls |
| [`Optimizations.md`](Optimizations.md) | Optimizations that are used in POMS, and some reasoning behind them |
| [`RLET-Optimization.md`](RLET-Optimization.md) | An abandonded optimization trying a run length encoding (RLE) scheme on the AC4 space by mapping to a 1D array via a space filling curve |
| [`Schemas.md`](Schemas.md) | Documenting various config files used, in particular the POMS JSON file |
| [`Overview.md`](Overview.md) | Out of date overview of the problem definition, a CSP formulation and a review of automated rule deduction, MMS, WFC, BMS and POMS |

rule-graph
---

The `rule-graph` subdirectory has two directories containing scripts to create rule graphs for the *Pill Mortal* and *Forest Micro* tile set.

One rule graph for the *Forest Micro* tile set can be seen in the [`Overview.md`](Overview.md) file.

See the `rule-graph` [README.md](rule-graph/README.md) for the rule graphs for the *Forest Micro* and *Pill Mortal* tile set.

viz
---

The `viz` subdirectory holds three auxiliary web applications used for visualizations:

* `rule-highlight` : Web application that can be used to explore 2D automated rule creation (limited to 2x2 tile windows)
* `viewer2d` : Web application that can be used to see POMS working in real time via a 2D "snapshot" Tiled-like output file
* `viewer3d` : Web application that can be used to see POMS working in real time via a 3D "snapshot" POMS patch output file

The `viewer2d` and `viewer3d` are pretty clunky to use, requiring a local web server to be run along with various files
to be placed or linked correctly.
Their use should be considered for "expert only".


###### 2024-08-26
