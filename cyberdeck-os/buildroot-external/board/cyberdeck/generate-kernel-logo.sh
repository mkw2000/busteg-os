#!/bin/sh

set -eu

out="${1:?usage: generate-kernel-logo.sh OUTPUT_PPM}"
src="$(dirname "$0")/assets/kernel-logo.ppm"

mkdir -p "$(dirname "$out")"

cp "$src" "$out"
