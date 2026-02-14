#!/bin/bash
set -e

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

# Run build
echo "=== Running make ==="
make -C buildroot \
    BR2_DL_DIR="$BR2_DL_DIR" \
    BR2_EXTERNAL="$BR2_EXTERNAL" \
    O="$O" \
    RECALBOX_IMAGES="recalbox"

echo "=== Build complete ==="
ls -la output/images/ 2>/dev/null || echo "No images directory"
ls -la output/images/recalbox/ 2>/dev/null || echo "No recalbox directory"
