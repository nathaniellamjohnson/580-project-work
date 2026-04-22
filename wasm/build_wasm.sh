#!/usr/bin/env bash

set -euo pipefail

root_dir="$(cd "$(dirname "$0")/.." && pwd)"
output_dir="$root_dir/wasm"

em++ -std=c++11 -O3 \
  -s WASM=1 \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s NO_EXIT_RUNTIME=1 \
  -s MODULARIZE=1 \
  -s EXPORT_NAME='RayTracerModule' \
  -s FORCE_FILESYSTEM=1 \
  -s EXPORTED_FUNCTIONS='["_render_scene_file","_get_pixels","_get_width","_get_height"]' \
  -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap","FS","HEAPU8","HEAPU32"]' \
  -I"$root_dir" \
  "$root_dir/wasm/wasm_main.cpp" \
  "$root_dir/camera.cpp" \
  "$root_dir/hierarchy.cpp" \
  "$root_dir/flat_shader.cpp" \
  "$root_dir/parse.cpp" \
  "$root_dir/phong_shader.cpp" \
  "$root_dir/plane.cpp" \
  "$root_dir/reflective_shader.cpp" \
  "$root_dir/render_world.cpp" \
  "$root_dir/sphere.cpp" \
  "$root_dir/box.cpp" \
  "$root_dir/mesh.cpp" \
  "$root_dir/glass_shader.cpp" \
  "$root_dir/caustic_map.cpp" \
  -o "$output_dir/ray_tracer.js"

echo "WASM build complete: $output_dir/ray_tracer.js"