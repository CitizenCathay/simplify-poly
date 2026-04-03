#!/usr/bin/env bash

set -e

echo "Compiling overlay_svg.cpp..."
g++ -std=c++17 -O2 overlay_svg.cpp -o overlay_svg

echo "Running overlay_svg..."
./overlay_svg

echo "Done. SVG overlays are in tests/svg-overlay"
echo "Experimental SVG overlays are in tests/svg-overlay/experimental"