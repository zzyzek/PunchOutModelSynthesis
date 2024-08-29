Emscripten Port of Punch Out Model Synthesis
===

This is code related to porting the Punch Out Model Synthesis (POMS)
code to javascript for use in the browser.

**THIS CODE IS UNDER ACTIVE DEVELOPEMNT**

Motivation
---

Code is written in C/C++ and it'd be nice to re-use the code already written
as well as try and get some performance increases.

The C/C++ source files are sym linked from the `src/` directory and a compile
bash script, `emcmp` is used to compile the library.

The `yajl` library needed to be compiled with `emscripten` so has been provided
in this directory for comppilation purposes.

The `html/` directory is the inteded target of where the compiled JavaScript should
go as well as the sample application, etc. but this is all highly volatile and
subject to change.

Notes
---

* C++ functions need to be wrapped in `extern "C" { ... }` in order to not get their
  linking names mangled (so they can be referenced via `_<function_name>` in JS)
* The `EMSCRIPTEN_KEEPALIVE` macro prefixing the C function declerations automatically
  exposes them to the JS Module runtime and is probably a good convention to use
  (instead of, say, explicitely mentioning them in the `-sEXPORTED_FUNCTIONS=['_<function_name>']`
  compiler directive)
* After fighting with `empscripten` to figure out how to pass an array fo strings,
  I stumbled on an [SO answer](https://stackoverflow.com/a/70267473/4002265) that
  formats things properly so the function can be called directly. A wrapper function
  is provided to call the C/C++ `main` from JS (in `js/poms_helper.js`)
* Memory is an issue so some combination of `-sALLOW_MEMORY_GROWTH=1` and `-sMAXIMUM_MEMORY=4294967296`
  is necessary/desirable
* Having some "templates" preloaded is a good idea, especially if they're small, so I'm adding in
  the **Pill Mortal** tile set. I'm still fussing with figuring out where in the JS virtual (memory)
  file system the file is located...
  - `emscrpten`'s `--preload-file` is following the symlink for the virutal file system location????
  - There's a difference between where the data file is located on the server (where the compiled JS
    will make the xhr request) and where the file is located in the virutal file system
    + I didn't figure out how to really change it but [here's what I think is the relevant documentation link](https://emscripten.org/docs/porting/files/packaging_files.html#changing-the-data-file-location)
      for the underlying server file location (and a potentially relevant
      [SO](https://stackoverflow.com/questions/73973701/emscripten-how-to-override-locatefile-when-compiled-with-modularize-options) answer)
    + Change the virtual file location with `--preload-file srcpath@dstpath`
  - I'm punting on fiddling with data file lcoations and just providing a symlink in `html/` to the `poms.data` that gets created
    in the `html/js` directory 
* Ideally, it would be good to have a 'real time' view of POMS working. One method that might work:
  - Create a web worker to execute the POMS run
  - Run POMS with a snapshot file
  - Provide a custom visualization callback, `web_viz_cb`, to the run from the `web_poms.cpp`
  - From `web_viz_cb`, call a JS function, `web_worker_cb`
  - From `web_worker_cb`, load the snapshot file and hand that back to the parent

The 'real time' JS idea is trying to hit a few things at once:

* minimal alteration of underlying POMS codebase
* allow real time visualization updates

Emscripten relevant API quick links:

* [File System API](https://emscripten.org/docs/api_reference/Filesystem-API.html)
  - [Packaging Files](https://emscripten.org/docs/porting/files/packaging_files.html#packaging-files)
* [Compiler Command Line Options](https://emscripten.org/docs/tools_reference/emcc.html#command-line-syntax)
* [Compiler Settings](https://emscripten.org/docs/tools_reference/settings_reference.html)
