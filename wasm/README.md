# WASM Viewer

This folder contains a browser UI for the ray tracer.

## Layout

- `index.html` - split screen page with a scene textbox on the left and the rendered output on the right.
- `app.js` - browser glue that writes the textbox contents into Emscripten's virtual filesystem and calls the wasm renderer.
- `wasm_main.cpp` - wasm entrypoint that parses a scene file path and renders into the existing pixel buffer.
- `build_wasm.sh` - Emscripten build script.

## Build

Run this from a shell where Emscripten is installed and activated:

```bash
./wasm/build_wasm.sh
```

## Run

Serve the project from a local web server so the browser can load the JS and wasm files, then open `wasm/index.html`.

For example:

```bash
python3 -m http.server
```

Then open:

```text
http://localhost:8000/wasm/
```

The page uses the scene text from `tests/ball_of_light_test_spp/cornell_box_ball_of_light.txt` as the initial text in the textbox.