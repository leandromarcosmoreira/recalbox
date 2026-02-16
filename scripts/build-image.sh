#!/bin/bash

set -e

echo "=== Starting image build ==="
echo "ARCH: $ARCH"
echo "O: $O"
echo "BR2_EXTERNAL: $BR2_EXTERNAL"
echo "BR2_DL_DIR: $BR2_DL_DIR"

cd /recalbox

mkdir -p output build dl

chmod +x buildroot/support/scripts/* buildroot/scripts/* 2>/dev/null || true
chmod -R 777 output build dl 2>/dev/null || true

echo "=== Output directory ==="
ls -la output/ 2>/dev/null || echo "No output dir"

echo "=== Running make world (includes image generation) ==="
make -C buildroot \
    BR2_DL_DIR="$BR2_DL_DIR" \
    BR2_EXTERNAL="$BR2_EXTERNAL" \
    O="$O" \
    world 2>&1 | tee /recalbox/build.log

BUILD_STATUS=${PIPESTATUS[0]}
echo "Build exit status: $BUILD_STATUS"

if [ $BUILD_STATUS -ne 0 ]; then
    echo "Build failed! Showing last 100 lines of log:"
    tail -100 /recalbox/build.log || true
    exit $BUILD_STATUS
fi

echo "=== Build log tail ==="
tail -50 /recalbox/build.log || true

echo "=== Output images ==="
ls -la output/images/ 2>/dev/null || echo "No images dir"
ls -la output/images/recalbox/ 2>/dev/null || echo "No recalbox dir"

if [ ! -d "output/images/recalbox" ]; then
    echo "ERROR: output/images/recalbox directory not found!"
    exit 1
fi

if [ -z "$(ls -A output/images/recalbox/)" ]; then
    echo "ERROR: output/images/recalbox directory is empty!"
    exit 1
fi

exit 0
