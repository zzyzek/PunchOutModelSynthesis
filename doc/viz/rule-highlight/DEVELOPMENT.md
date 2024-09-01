Developer Notes
===

**THIS IS EXPERIMENTAL CODE, SOME BUGS REMAIN AND SOME FEATURES ARE NOT IMPLEMENTED**

The Tile Rule Highlighter is an interactive web application to allow exploration
of the tile rule creation from exemplar images (2D).

These notes are on program flow and how it's designed.
The application is relatively small but I feel like these notes are still
helpful in case the application needs to be altered or extended.

Overview
---

The Tile Rule Highlighter is a Pixi application using Skeleton.css as the HTML/CSS tempalte.

From an exemplar image, a call to `img2tile.js` is done to create the implied rules.
The tile set image is created along with the supertiles used to create the tile
set and both are displayed in their respective regions in the application.

A modal is present to allow for uploading custom images and specifying the tile,
window and supertile size.

Program Organization
---

* `index.html` - main entr point, defines most HTML elements
* `js/pixiapp.js` - main application to do the tile highlighting
* `js/trh.js` - helper functions to flatten images, convert to/from base64 etc.
* `js/img2tile.js` - pointer to `src.js/img2tile.js` to do the heavy lifting of conversion for
   the tile set


Program flow
---

On init:
```
init ->                       // (index.html)
  _runImg2Tile ->             // (index.html)
    flatten_img_data          // (trh.js)
    img2tile_run              // (img2tile.js)
    imgDataLoad               // (trh.js)
  pixi_init                   // (pixiapp.js)

skeleton_modal_init           // (skeleton_modal_init.js)
```

On change of option:

```
img_option.onchange ->
  imgOptionChange ->          // (index.html)
    hiddenExemplarOnLoad ->   // (index.html)
      pixi_load               // (pixiapp.js)
      _runImg2Tile ->         // (index.html)
        flatten_img_data      // (trh.js)
        img2tile_run          // (img2tile.js)
        imgDataLoad           // (trh.js)
```


TODO
---

* Allow for non 2x2 windows
  - call to `img2tile` is set up correctly (I believe) but the supertile display needs work
* Bug for some tilesets with rule highlighting
  - I believe this happens because one of the exemplar image or tile set image is large and
    invisibly spills over to other areas, capturing mouse move requests and highlighting things
    improperly
  - To investigate, look at mouse events in more detail to see they're getting to where they're
    supposed to
* Highlight supertiles and neighbors
