#!/usr/bin/env bash

set -u

base_scene="./tests/ball_of_light_test_spp/cornell_box_ball_of_light.txt"
output_dir="./tests/ball_of_light_test_spp/renders"
temp_dir="$(mktemp -d)"

spp_values=(1 4 16 32 64 100 250 500 1000 2000 4000 10000)

mkdir -p "$output_dir"

cleanup() {
    rm -rf "$temp_dir"
}

trap cleanup EXIT

for spp in "${spp_values[@]}"; do
    temp_scene="$temp_dir/cornell_box_ball_of_light_spp_${spp}.txt"
    output_file="$output_dir/cornell_box_ball_of_light_spp_${spp}.ppm"

    cp "$base_scene" "$temp_scene"
    sed -E -i '' "s/^spp[[:space:]]+[0-9]+.*$/spp ${spp}/" "$temp_scene"

    echo "Rendering spp=${spp}"
    ./ray_tracer -i "$temp_scene"
    mv output.ppm "$output_file"
done

echo "Done. Rendered images are in $output_dir"