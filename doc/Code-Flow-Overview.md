Code Flow Overview
===

This is a brief overview of how code flows during normal operation.

The focus will be on Punch Out Model Synthesis (POMS) using the Breakout Model Synthesis (BMS) algorithm
as the block solver.

Overview
---

* POMS selects patches and runs BMS on them
  - patches are chosen from a patch scheduler
  - `setupQuiltPatch` initializes the patch for use with
    the block solver
    + `refreshConstraints` : needed to update constraints local to the patch
    + `resetAC4Dirty` : clears the AC4 dirty structure
    + `applyConstraints` : apply patch level constraints
    + `applyQuiltConstraints` : apply whole grid (quilt) constraints
    + pin any cells in the patch as they appear pinned in the quilt
    + pin the boundary of the patch if they don't fall on the quilt edge
    + `AC4Init` : run initial constraint propagation, initializing the AC4 support structure
    + `savePrefatory` : save the prefatory state (what BMS will soften to)
    + `applyStartConstraints` & `AC4Init`: run any start constraints, if they exist, rerunning
      the AC4 initialization if need be (note: don't save prefatory since, by definition,
      start constraints are only run once on initialization)
  - Run BMS on the patch
    + `BMSInit` : run once at the beginning
      * `AC4Init`
      * `markAC4Dirty` : probably redundant
    + `BMSBegin` : run inside the patch resolution loop
      * `saveGrid` : only on first retry (initial run, or initial run after softening)
      * `chooseBlock` : one of many options, we'll use the minimum entropy version here
        - `chooseBlock_minEntropyBlock`
          + `computeCellEntropy` : compute individual cell entropies
          + `computeBlockEntropy` : compute BMS block entropies. This is mostly a hold over from 
            an earlier version of BMS. BMS blocks are usually chosen to be size 1x1x1, so the
            block entropy and cell entropy should be the same
          + choose BMS block (aka cell) based on computed entropy with some randomness thrown in
      * `resetAC4Dirty`
      * `soften` -> `soften_ac4`: on the whole patch, reverting elements within soften window
        (here, the whole patch) to the prefatory state, marking softened cells with `markAC4Dirty`
        and running `AC4InitBlock`
    + `BMSStep` : run inside the patch resolution loop (after `BMSBegin`)
      * `WFCBlock_ac4` : resolve a single cell in the patch 
  
    


