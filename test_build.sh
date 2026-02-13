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
    echo "  Available RAM: ${AVAILABLE_MEM}GB"
    echo "  Disk Free:     ${DISK_FREE}GB"
    
    if [ "$AVAILABLE_MEM" -ge 16 ]; then
        MAKE_JOBS=$((CPU_CORES))
        echo "  Make Jobs:     ${MAKE_JOBS} (optimal)"
    elif [ "$AVAILABLE_MEM" -ge 8 ]; then
        MAKE_JOBS=$((CPU_CORES / 2))
        echo "  Make Jobs:     ${MAKE_JOBS} (limited by memory)"
    else
        MAKE_JOBS=4
        echo "  Make Jobs:     ${MAKE_JOBS} (conservative)"
    fi
    
    export MAKE_JOBS
    echo "=========================================="
    echo ""
}

init_directories() {
    mkdir -p "${LOG_DIR}"
    mkdir -p "${CACHE_DIR}/dl"
    mkdir -p "${CACHE_DIR}/ccache"
    mkdir -p "${SCRIPT_DIR}/output"
}

start_step() {
    local step_name="$1"
    local step_log="$2"
    echo "[STEP] Starting: ${step_name}"
    START_TIME=$(date +%s)
    START_MEM=$(free -m | awk '/^Mem:/{print $7}')
    echo "  Memory before: ${START_MEM}MB"
}

end_step() {
    local step_name="$1"
    local step_log="$2"
    END_TIME=$(date +%s)
    ELAPSED=$((END_TIME - START_TIME))
    END_MEM=$(free -m | awk '/^Mem:/{print $7}')
    MEM_USED=$((START_MEM - END_MEM))
    
    echo "[DONE] ${step_name} - Time: ${ELAPSED}s, Memory delta: ${MEM_USED}MB"
    echo "  → Log: ${LOG_DIR}/${step_log}.log"
    
    echo "${step_name}|${ELAPSED}|${MEM_USED}" >> "${LOG_DIR}/timing_summary.csv"
}

log_step() {
    echo "[$(date '+%H:%M:%S')] $1"
}

run_step() {
    local step_name="$1"
    local step_log="$2"
    local cmd="$3"
    
    start_step "${step_name}" "${step_log}"
    
    eval "${cmd}" 2>&1 | tee "${LOG_DIR}/${step_log}.log"
    
    end_step "${step_name}" "${step_log}"
}

echo "=========================================="
echo "  Recalbox Build System v2.0"
echo "=========================================="
echo ""

detect_environment
init_directories

BUILD_START=$(date +%s)
echo "Build started at: $(date)"

echo ""
echo "=========================================="
echo "  [1/7] Setup - Repository & Buildroot"
echo "=========================================="
run_step "Setup repository" "01_setup" "
    cd ${SCRIPT_DIR}
    if [ ! -d 'buildroot' ]; then
        log_step 'Cloning buildroot...'
        git clone --depth 1 --branch 2024.02 https://git.busybox.net/buildroot buildroot
    else
        log_step 'Buildroot already exists'
    fi
    
    cd buildroot
    git fetch --depth 1 origin e4a5ab3b319753f41e2b5cf22e90b6b0304ca225
    git checkout e4a5ab3b319753f41e2b5cf22e90b6b0304ca225
    
    cd ${SCRIPT_DIR}
    log_step 'Making defconfig...'
    make recalbox-${ARCH}_defconfig
"

echo ""
echo "=========================================="
echo "  [2/7] Docker Image"
echo "=========================================="
run_step "Build Docker image" "02_docker" "
    cd ${SCRIPT_DIR}
    docker build -t recalbox-build:latest .
"

echo ""
echo "=========================================="
echo "  [3/7] Toolchain"
echo "=========================================="
run_step "Build toolchain" "03_toolchain" "
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
        make BR2_DL_DIR='/share/dl' -j${MAKE_JOBS} toolchain
"

echo ""
echo "=========================================="
echo "  [4/7] System"
echo "=========================================="
run_step "Build system" "04_system" "
    cd \${SCRIPT_DIR}
    docker run --rm --security-opt seccomp=unconfined \
        -v \${SCRIPT_DIR}:\/work \
        -v \${CACHE_DIR}\/dl:\/share\/dl \
        -v \${CACHE_DIR}\/ccache:\/share\/ccache \
        -e ARCH=${ARCH} \
        -e FORCE_UNSAFE_CONFIGURE=1 \
        -e RECALBOX_CCACHE_ENABLED=1 \
        -e MAKE_JOBS=${MAKE_JOBS} \
        recalbox-build:latest \
        make BR2_DL_DIR='/share/dl' -j${MAKE_JOBS} system
"

echo ""
echo "=========================================="
echo "  [5/7] Packages"
echo "=========================================="
run_step "Build packages" "05_packages" "
    cd \${SCRIPT_DIR}
    docker run --rm --security-opt seccomp=unconfined \
        -v \${SCRIPT_DIR}:\/work \
        -v \${CACHE_DIR}\/dl:\/share\/dl \
        -v \${CACHE_DIR}\/ccache:\/share\/ccache \
        -e ARCH=${ARCH} \
        -e FORCE_UNSAFE_CONFIGURE=1 \
        -e RECALBOX_CCACHE_ENABLED=1 \
        -e MAKE_JOBS=${MAKE_JOBS} \
        recalbox-build:latest \
        make BR2_DL_DIR='/share/dl' -j${MAKE_JOBS} packages
"

echo ""
echo "=========================================="
echo "  [6/7] Final Image"
echo "=========================================="
run_step "Build final image" "06_image" "
    cd \${SCRIPT_DIR}
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
        make BR2_DL_DIR='/share/dl' -j${MAKE_JOBS}
"

echo ""
echo "=========================================="
echo "  [7/7] Artifacts & Summary"
echo "=========================================="
run_step "Prepare artifacts" "07_artifacts" "
    cd \${SCRIPT_DIR}
    DIST_DIR='dist\/${ARCH}'
    mkdir -p \${DIST_DIR}
    
    if [ -f 'output/images/recalbox/recalbox.img' ]; then
        cp output/images/recalbox/* \${DIST_DIR}/
        cp output/images/recalbox/*.zip \${DIST_DIR}/ 2>/dev/null || true
        
        cd \${DIST_DIR}
        sha256sum * > SHA256SUMS.txt
        
        echo '=== Output Files ==='
        ls -lh
    else
        echo 'ERROR: No image generated!'
        exit 1
    fi
"

BUILD_END=$(date +%s)
TOTAL_TIME=$((BUILD_END - BUILD_START))

echo ""
echo "=========================================="
echo "  BUILD COMPLETE!"
echo "=========================================="
echo ""
echo "Total time: $((TOTAL_TIME / 60))m $((TOTAL_TIME % 60))s"
echo ""

echo "=== Timing Summary ==="
echo "Step|Time(s)|Memory(MB)" | tee "${LOG_DIR}/timing_summary.csv"
cat "${LOG_DIR}/timing_summary.csv"

echo ""
echo "=== Output Files ==="
ls -lh "${SCRIPT_DIR}/dist/${ARCH}/"

echo ""
echo "=== Cache Status ==="
echo "Download cache: $(du -sh ${CACHE_DIR}/dl 2>/dev/null || echo '0B')"
echo "ccache: $(du -sh ${CACHE_DIR}/ccache 2>/dev/null || echo '0B')"
