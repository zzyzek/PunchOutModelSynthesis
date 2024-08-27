POMS 2D Viewer
===

| Key | Action |
|---|---|
| `=`,`e` | Zoom in |
| `-`,`q` | Zoom out |
| `w`,`k` | Move up |
| `a`,`h` | Move left |
| `s`,`j` | Move down |
| `d`,`l` | Move right |

This is a minimal application to watch runs evolve in real-time.

Quick Start
---

* `data/snapshot.json` holds the Tiled JSON file that is being displayed
* `img/` is the directory where the tilesets should be located

Run a web server in this directory:

```
$ python3 -m http.server
```

Run `poms` with the `snapshot` setting, pointing the `snapshot.json` POMS file
in the `data/` directory:

```
$ poms ... -8 $V2DLOC/data/snapshot.json ...
```

Description
---

The web application renders the Tiled JSON file (e.g. `snapshot.json`)
that represents the current snapshot of the POMS run.

This application still has many bugs and rough edges to it's mostly used
to help with exposition (documentation, illustration, etc.) and to help
debug and visualize POMS runs.

As of this writing, the web application polls the local webserver to reload the
`snapshot.json` file.
The `poms` program is smart enough to make the `snapshot.json` file copy/move atomic
to avoid partial loads of the `snapshot.json` file.


License
---

Unless otherwise stated, everything is CC0.

Check individual header files for details.
