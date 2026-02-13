#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOG_DIR="${SCRIPT_DIR}/build_logs"
CACHE_DIR="${SCRIPT_DIR}/.build_cache"
ARCH="${1:-x86_64}"

detect_environment() {
    echo "=========================================="
    echo "  Environment Detection"
    echo "=========================================="
    
    CPU_CORES=$(nproc)
    TOTAL_MEM=$(free -g | awk '/^Mem:/{print $2}')
    AVAILABLE_MEM=$(free -g | awk '/^Mem:/{print $7}')
    DISK_FREE=$(df -BG /home | awk 'NR==2 {print $4}' | sed 's/G//')
    
    echo "  CPU Cores:     ${CPU_CORES}"
    echo "  Total RAM:     ${TOTAL_MEM}GB"
    echo "  Available:     ${AVAILABLE_MEM}GB"
    echo "  Disk Free:     ${DISK_FREE}GB"
    
    if [ "$AVAILABLE_MEM" -ge 16 ]; then
        MAKE_JOBS=$CPU_CORES
    elif [ "$AVAILABLE_MEM" -ge 8 ]; then
        MAKE_JOBS=$((CPU_CORES / 2))
    else
        MAKE_JOBS=4
    fi
    
    export MAKE_JOBS
    echo "  Make Jobs:     ${MAKE_JOBS}"
    echo "=========================================="
    echo ""
}

init_directories() {
    mkdir -p "${LOG_DIR}"
    mkdir -p "${CACHE_DIR}/dl"
    mkdir -p "${CACHE_DIR}/ccache"
    mkdir -p "${SCRIPT_DIR}/output"
    mkdir -p "${CACHE_DIR}/artifacts/${ARCH}"
}

log() {
    echo "[$(date '+%H:%M:%S')] $1"
}

run_step() {
    local step_name="$1"
    local step_log="$2"
    local cmd="$3"
    
    log "Starting: ${step_name}"
    START_TIME=$(date +%s)
    
    eval "${cmd}" 2>&1 | tee "${LOG_DIR}/${step_log}.log"
    
    END_TIME=$(date +%s)
    ELAPSED=$((END_TIME - START_TIME))
    log "Done: ${step_name} (${ELAPSED}s)"
    
    echo "${step_name}|${ELAPSED}" >> "${LOG_DIR}/timing.csv"
}

get_git_changes() {
    local prev_commit="${1:-HEAD~1}"
    git diff --name-only "${prev_commit}"...HEAD -- 'package/*' 'board/*' 'Config.in*' 'makefile' 2>/dev/null || echo ""
}

analyze_dependencies() {
    log "Analyzing dependencies and changes..."
    
    local changes
    changes=$(get_git_changes)
    
    local needs_toolchain=false
    local needs_system=false
    local needs_packages=false
    local needs_image=true
    
    if echo "$changes" | grep -qE '^(toolchain/|package/.*/toolchain|Config.in|configs/)'; then
        needs_toolchain=true
        needs_system=true
        needs_packages=true
        log "  → Changes require: TOOLCHAIN + SYSTEM + PACKAGES"
    elif echo "$changes" | grep -qE '^board/'; then
        needs_system=true
        needs_packages=true
        log "  → Changes require: SYSTEM + PACKAGES"
    elif echo "$changes" | grep -qE '^package/'; then
        needs_packages=true
        log "  → Changes require: PACKAGES"
    else
        log "  → No significant changes detected"
    fi
    
    export NEEDS_TOOLCHAIN=$needs_toolchain
    export NEEDS_SYSTEM=$needs_system
    export NEEDS_PACKAGES=$needs_packages
    export NEEDS_IMAGE=$needs_image
    
    echo ""
    echo "=== Build Decision ==="
    echo "Toolchain: $needs_toolchain"
    echo "System:    $needs_system"
    echo "Packages:  $needs_packages"
    echo "Image:     $needs_image"
    echo ""
}

check_artifacts() {
    local artifact_dir="${CACHE_DIR}/artifacts/${ARCH}"
    
    log "Checking existing artifacts..."
    
    local has_toolchain=false
    local has_system=false
    local has_packages=false
    
    if [ -d "${artifact_dir}/toolchain" ] && [ -f "${artifact_dir}/.config" ]; then
        has_toolchain=true
        log "  ✓ Toolchain artifact found"
    fi
    
    if [ -d "${artifact_dir}/system" ]; then
        has_system=true
        log "  ✓ System artifact found"
    fi
    
    if [ -d "${artifact_dir}/packages" ]; then
        has_packages=true
        log "  ✓ Packages artifact found"
    fi
    
    export HAS_TOOLCHAIN=$has_toolchain
    export HAS_SYSTEM=$has_system
    export HAS_PACKAGES=$has_packages
    
    echo ""
}

check_blake3() {
    if command -v b3sum &> /dev/null; then
        echo "b3sum"
    elif command -v blake3 &> /dev/null; then
        echo "blake3"
    else
        echo "none"
    fi
}

generate_checksums() {
    local dir="$1"
    local checksum_tool=$(check_blake3)
    
    cd "${dir}"
    
    if [ "$checksum_tool" != "none" ]; then
        log "Generating BLAKE3 checksums..."
        $checksum_tool * > BLAKE3SUMS.txt
        
        if [ "$checksum_tool" = "b3sum" ]; then
            chmod 444 BLAKE3SUMS.txt
        fi
        
        cat BLAKE3SUMS.txt
    else
        log "BLAKE3 not available, using SHA256..."
        sha256sum * > SHA256SUMS.txt
        cat SHA256SUMS.txt
    fi
}

verify_checksums() {
    local dir="$1"
    local checksum_tool=$(check_blake3)
    
    cd "${dir}"
    
    if [ "$checksum_tool" != "none" ] && [ -f "BLAKE3SUMS.txt" ]; then
        log "Verifying BLAKE3 checksums..."
        if $checksum_sum --check BLAKE3SUMS.txt; then
            log "  ✓ BLAKE3 verification passed"
            return 0
        else
            log "  ✗ BLAKE3 verification FAILED"
            return 1
        fi
    elif [ -f "SHA256SUMS.txt" ]; then
        log "Verifying SHA256 checksums..."
        if sha256sum --check SHA256SUMS.txt; then
            log "  ✓ SHA256 verification passed"
            return 0
        else
            log "  ✗ SHA256 verification FAILED"
            return 1
        fi
    fi
    
    return 0
}

restore_artifact() {
    local artifact_type="$1"
    local artifact_dir="${CACHE_DIR}/artifacts/${ARCH}/${artifact_type}"
    
    if [ -d "${artifact_dir}" ]; then
        log "Restoring ${artifact_type} from cache..."
        
        if verify_checksums "${artifact_dir}"; then
            cp -r "${artifact_dir}"/* buildroot/ 2>/dev/null || true
            return 0
        else
            log "  ⚠ Artifact verification failed, rebuilding..."
            rm -rf "${artifact_dir}"
            return 1
        fi
    fi
    return 1
}

save_artifact() {
    local artifact_type="$1"
    local artifact_dir="${CACHE_DIR}/artifacts/${ARCH}/${artifact_type}"
    
    log "Saving ${artifact_type} artifact..."
    mkdir -p "${artifact_dir}"
    cp -r buildroot/output "${artifact_dir}/" 2>/dev/null || true
    cp buildroot/.config "${artifact_dir}/" 2>/dev/null || true
    
    generate_checksums "${artifact_dir}"
}

echo "=========================================="
echo "  Recalbox Smart Build System v3.1"
echo "  Checksum: BLAKE3"
echo "=========================================="
echo ""

detect_environment
init_directories
analyze_dependencies
check_artifacts

BUILD_START=$(date +%s)
log "Build started at: $(date)"

echo ""
echo "=========================================="
echo "  [1/6] Setup - Repository & Buildroot"
echo "=========================================="
run_step "Setup" "01_setup" "
    cd ${SCRIPT_DIR}
    
    if [ ! -d 'buildroot' ]; then
        log 'Cloning buildroot...'
        git clone --depth 1 --branch 2024.02 https://git.busybox.net/buildroot buildroot
    else
        log 'Buildroot exists, updating...'
    fi
    
    cd buildroot
    git fetch --depth 1 origin e4a5ab3b319753f41e2b5cf22e90b6b0304ca225
    git checkout e4a5ab3b319753f41e2b5cf22e90b6b0304ca225
    
    cd ${SCRIPT_DIR}
    log 'Making defconfig (using Docker)...'
    docker run --rm \
        -v \${SCRIPT_DIR}:\/work \
        -v \${CACHE_DIR}\/dl:\/share\/dl \
        -w \/work \
        recalbox-build:latest \
        make recalbox-${ARCH}_defconfig
"

echo ""
echo "=========================================="
echo "  [2/6] Docker Image"
echo "=========================================="
run_step "Docker" "02_docker" "
    cd ${SCRIPT_DIR}
    docker build -t recalbox-build:latest .
"

echo ""
echo "=========================================="
echo "  [3/6] Toolchain"
echo "=========================================="

if [ "$NEEDS_TOOLCHAIN" = "true" ] && [ "$HAS_TOOLCHAIN" = "false" ]; then
    log "Building toolchain (no cache)..."
    run_step "Toolchain" "03_toolchain" "
        cd ${SCRIPT_DIR}
        docker run --rm --security-opt seccomp=unconfined \
            -v \${SCRIPT_DIR}:\/work \
            -v \${CACHE_DIR}\/dl:\/share\/dl \
            -v \${CACHE_DIR}\/ccache:\/share\/ccache \
            -e ARCH=${ARCH} \
            -e FORCE_UNSAFE_CONFIGURE=1 \
            -e RECALBOX_CCACHE_ENABLED=1 \
            -e MAKE_JOBS=${MAKE_JOBS} \
            recalbox-build:latest \
            make BR2_DL_DIR='/share/dl' -j\${MAKE_JOBS} toolchain
    "
    save_artifact "toolchain"
elif [ "$HAS_TOOLCHAIN" = "true" ]; then
    log "Skipping toolchain (using cached artifact)"
    restore_artifact "toolchain" || true
else
    log "Toolchain not required"
fi

echo ""
echo "=========================================="
echo "  [4/6] System"
echo "=========================================="

if [ "$NEEDS_SYSTEM" = "true" ]; then
    if [ "$HAS_SYSTEM" = "true" ]; then
        log "Restoring system from cache..."
        restore_artifact "system" || true
    fi
    
    run_step "System" "04_system" "
        cd ${SCRIPT_DIR}
        docker run --rm --security-opt seccomp=unconfined \
            -v \${SCRIPT_DIR}:\/work \
            -v \${CACHE_DIR}\/dl:\/share\/dl \
            -v \${CACHE_DIR}\/ccache:\/share\/ccache \
            -e ARCH=${ARCH} \
            -e FORCE_UNSAFE_CONFIGURE=1 \
            -e RECALBOX_CCACHE_ENABLED=1 \
            -e MAKE_JOBS=${MAKE_JOBS} \
            recalbox-build:latest \
            make BR2_DL_DIR='/share/dl' -j\${MAKE_JOBS} system
    "
    save_artifact "system"
else
    log "System not required"
fi

echo ""
echo "=========================================="
echo "  [5/6] Packages"
echo "=========================================="

if [ "$NEEDS_PACKAGES" = "true" ]; then
    if [ "$HAS_PACKAGES" = "true" ]; then
        log "Restoring packages from cache..."
        restore_artifact "packages" || true
    fi
    
    run_step "Packages" "05_packages" "
        cd ${SCRIPT_DIR}
        docker run --rm --security-opt seccomp=unconfined \
            -v \${SCRIPT_DIR}:\/work \
            -v \${CACHE_DIR}\/dl:\/share\/dl \
            -v \${CACHE_DIR}\/ccache:\/share\/ccache \
            -e ARCH=${ARCH} \
            -e FORCE_UNSAFE_CONFIGURE=1 \
            -e RECALBOX_CCACHE_ENABLED=1 \
            -e MAKE_JOBS=${MAKE_JOBS} \
            recalbox-build:latest \
            make BR2_DL_DIR='/share/dl' -j\${MAKE_JOBS} packages
    "
    save_artifact "packages"
else
    log "Packages not required"
fi

echo ""
echo "=========================================="
echo "  [6/6] Final Image"
echo "=========================================="

run_step "Image" "06_image" "
    cd ${SCRIPT_DIR}
    OUTPUTDIR=\${SCRIPT_DIR}\/output
    mkdir -p \${OUTPUTDIR}
    
    docker run --rm --security-opt seccomp=unconfined \
        -v \${SCRIPT_DIR}:\/work \
        -v \${OUTPUTDIR}:\/work\/output \
        -v \${CACHE_DIR}\/dl:\/share\/dl \
        -v \${CACHE_DIR}\/ccache:\/share\/ccache \
        -e ARCH=${ARCH} \
        -e RECALBOX_VERSION='dev test' \
        -e FORCE_UNSAFE_CONFIGURE=1 \
        -e RECALBOX_CCACHE_ENABLED=1 \
        -e MAKE_JOBS=${MAKE_JOBS} \
        recalbox-build:latest \
        make BR2_DL_DIR='/share/dl' -j\${MAKE_JOBS}
"

echo ""
echo "=========================================="
echo "  Artifacts & Verification"
echo "=========================================="

DIST_DIR="${SCRIPT_DIR}/dist/${ARCH}"
mkdir -p "${DIST_DIR}"

if [ -f "output/images/recalbox/recalbox.img" ]; then
    cp output/images/recalbox/* "${DIST_DIR}/"
    cp output/images/recalbox/*.zip "${DIST_DIR}/" 2>/dev/null || true
    
    log "Generating final checksums..."
    generate_checksums "${DIST_DIR}"
    
    log "Verifying final image..."
    verify_checksums "${DIST_DIR}"
    
    log "Build complete!"
    ls -lh "${DIST_DIR}"
else
    log "ERROR: No image generated!"
    exit 1
fi

BUILD_END=$(date +%s)
TOTAL_TIME=$((BUILD_END - BUILD_START))

echo ""
echo "=========================================="
echo "  BUILD COMPLETE"
echo "=========================================="
echo "Total time: $((TOTAL_TIME / 60))m $((TOTAL_TIME % 60))s"
echo ""
echo "=== Timing ==="
cat "${LOG_DIR}/timing.csv"
echo ""
echo "=== Cache ==="
du -sh "${CACHE_DIR}/artifacts/${ARCH}" 2>/dev/null || echo "No artifacts cached"
