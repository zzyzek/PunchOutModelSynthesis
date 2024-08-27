POMS 3D Viewer
===

This is a minimal application to watch 3D runs evolve in real-time.

| Input | Action |
|---|---|
| Left Mouse Button | Rotate |
| Right Mouse Button | Translate |

Quick Start
---

* `data/poms.json` holds the POMS JSON config file
* `data/patch.json` holds the Tiled JSON file that is being displayed
* `obj/` is the directory where the 3D `.obj` tilesets should be located, as referenced in the `data/poms.json` file

Run a web server in this directory:

```
$ python3 -m http.server
```

Run `poms` with the `patch-snapshot` setting, pointing the `patch.json` POMS patch file
in the `data/` directory:

```
$ poms ... -6 $V3DLOC/data/patch.json ...
```

URL Options
---

By default the web application looks for `data/poms.json` and `data/patch.json` but this can be changed
by specifying a `poms` and `patch` URL parameter.

| URL Parameter | Description |
|---|---|
| `poms` | POMS config file to use (default `data/poms.json`) |
| `patch` | POMS config file to use (default `data/patch.json`) |
| `poll` | Poll time to use in ms (default 1000ms) |
| `up` | Which direction is "up", options are `x`, `y`, `z` (default `z`) |


For example:

```
http://localhost:8000?poms=data/custom_poms.json&patch=data/custom_patch.json&up=y
```

Description
---

The web application renders the POMS patch JSON file (e.g. `patch.json`)
that represents the current snapshot of the POMS run.

This application still has many bugs and rough edges to it's mostly used
to help with exposition (documentation, illustration, etc.) and to help
debug and visualize POMS runs.

As of this writing, the web application polls the local webserver to reload the
`patch.json` file.
The `poms` program is smart enough to make the `patch.json` file copy/move atomic
to avoid partial loads of the `patch.json` file.


License
---

Unless otherwise stated, everything is CC0.

Check individual header files for details.
