#!/usr/bin/env bash

set -euo pipefail

base_scene="./tests/caustic_glass_demo_test_photonmap/caustic_glass_demo.txt"
output_dir="./tests/caustic_glass_demo_test_photonmap/renders"
temp_dir="$(mktemp -d)"

# Format: "photons_per_light max_photons_gathered gather_radius"
param_sets=(
  "20000 200 0.04"
  "50000 240 0.05"
  "90000 260 0.06"
  "120000 320 0.07"
  "180000 500 0.08"
  "250000 800 0.10"
)

mkdir -p "$output_dir"

cleanup() {
  rm -rf "$temp_dir"
}

trap cleanup EXIT

for params in "${param_sets[@]}"; do
  read -r photons max_photons radius <<< "$params"

  radius_tag="${radius//./p}"
  temp_scene="$temp_dir/caustic_glass_demo_p${photons}_m${max_photons}_r${radius_tag}.txt"
  output_file="$output_dir/caustic_glass_demo_p${photons}_m${max_photons}_r${radius_tag}.ppm"

  cp "$base_scene" "$temp_scene"
  sed -E -i '' "s/^photon_mapping_params[[:space:]]+[0-9]+[[:space:]]+[0-9]+[[:space:]]+[0-9]*\.?[0-9]+.*$/photon_mapping_params ${photons} ${max_photons} ${radius}/" "$temp_scene"

  echo "Rendering photon_mapping_params ${photons} ${max_photons} ${radius}"
  ./ray_tracer -i "$temp_scene"
  mv output.ppm "$output_file"
done

echo "Done. Rendered images are in $output_dir"