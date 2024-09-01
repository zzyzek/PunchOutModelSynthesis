Punch Out Model Synthesis
===

**THIS IS RESEARCH QUALITY CODE AND SUBJECT TO CHANGE**

Introduction
---

Punch Out Model Synthesis (POMS) is a Constraint Based Tiling Generation (CBTG) algorithm,
extending Merrell's modify in block Model Synthesis (MMS) and Gumin's Wave Function Collapse (WFC)
algorithms.

POMS can work on large grids with minimal requirements for initial setup state.

See the [Gallery](runs/README.md) for some example outputs for various tile sets.

This project has a set of tools to automatically create tile sets and tile rule constraints
from exemplar (2D) images as well as some example code to programatically generate 3D tilesets.
For advanced users, there are a set of auxiliary web applications to allow for run visualization.

[Demo](https://zzyzek.github.io/PunchOutModelSynthesis)
---

[Online demo](https://zzyzek.github.io/PunchOutModelSynthesis)

Quick Start
---

```
git clone https://zzyzek/PunchOutModelSynthesis
cd PunchOutModelSynthesis/src ; make ; make ir
cd ../runs/pillMortal
./run.sh  # run POMS to create a 128x128 pillMortal level
```

![Pill Mortal 128x128 level](runs/pillMortal/data/pillMortal_128x128.png)


LICENSE
---

Unless otherwise stated, all source code is under a Creative Commons Zero 1.0 (CC0) license.

![CC0](img/cc0_88x31.png)

Third party libraries used were chosen to be libre/free licensed and should have license information
listed in their headers.

All art assets were chosen to be libre/free licensed and are appropriately attributed in the places
they are used.

