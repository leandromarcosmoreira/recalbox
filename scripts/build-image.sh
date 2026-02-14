#!/bin/bash

echo "=== Starting image build ==="
echo "ARCH: $ARCH"
echo "O: $O"
echo "BR2_EXTERNAL: $BR2_EXTERNAL"
echo "BR2_DL_DIR: $BR2_DL_DIR"

cd /recalbox

# Create directories
mkdir -p output build dl

# Set permissions
chmod +x buildroot/support/scripts/* buildroot/scripts/* 2>/dev/null || true
chmod -R 777 output build dl 2>/dev/null || true

# List current state
echo "=== Output directory ==="
ls -la output/ 2>/dev/null || echo "No output dir"

# Run build - compile all packages then create image
echo "=== Running make all ==="
make -C buildroot \
    BR2_DL_DIR="$BR2_DL_DIR" \
    BR2_EXTERNAL="$BR2_EXTERNAL" \
    O="$O" \
    all 2>&1 | tee /recalbox/build.log

BUILD_STATUS=${PIPESTATUS[0]}
echo "Build exit status: $BUILD_STATUS"

echo "=== Build log tail ==="
tail -50 /recalbox/build.log || true

echo "=== Output images ==="
ls -la output/images/ 2>/dev/null || echo "No images dir"
ls -la output/images/recalbox/ 2>/dev/null || echo "No recalbox dir"

exit $BUILD_STATUS
