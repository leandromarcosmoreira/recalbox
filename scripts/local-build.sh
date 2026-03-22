#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOG_DIR="${SCRIPT_DIR}/build_logs"
CACHE_DIR="${SCRIPT_DIR}/.build_cache"
FORCE_CLEAN=""
DRY_RUN=""
VALIDATE_ONLY=""

usage() {
    cat << EOF
Recalbox Local Build Validation Script
========================================

Usage: $0 [OPTIONS]

OPTIONS:
    -a, --arch ARCH      Architecture to build (x86_64, rpi5_64, all) [default: x86_64]
    -c, --clean          Force clean build (ignore cache)
    -d, --dry-run        Show what would be built without building
    -v, --validate       Only validate existing artifacts
    -h, --help           Show this help

EXAMPLES:
    $0                      # Build x86_64 using cache if available
    $0 -a rpi5_64           # Build Raspberry Pi 5 64-bit
    $0 -a all               # Build all architectures
    $0 -c                   # Clean build (ignore cache)
    $0 -d                   # Show what would be built

EOF
    exit 0
}

ARCH="x86_64"
while [[ $# -gt 0 ]]; do
    case $1 in
        -a|--arch) ARCH="$2"; shift 2 ;;
        -c|--clean) FORCE_CLEAN="1"; shift ;;
        -d|--dry-run) DRY_RUN="1"; shift ;;
        -v|--validate) VALIDATE_ONLY="1"; shift ;;
        -h|--help) usage ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
done

detect_environment() {
    echo "============================================"
    echo "  Environment Detection"
    echo "============================================"
    
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
    echo "============================================"
    echo ""
}

init_directories() {
    mkdir -p "${LOG_DIR}"
    mkdir -p "${CACHE_DIR}/dl"
    mkdir -p "${CACHE_DIR}/ccache"
    mkdir -p "${SCRIPT_DIR}/output"
    
    if [ "$ARCH" = "all" ]; then
        for a in x86_64 rpi5_64; do
            mkdir -p "${CACHE_DIR}/artifacts/${a}"
        done
    else
        mkdir -p "${CACHE_DIR}/artifacts/${ARCH}"
    fi
}

log() {
    echo "[$(date '+%H:%M:%S')] $1"
}

get_git_changes() {
    local prev_commit="${1:-HEAD~1}"
    git diff --name-only "${prev_commit}"...HEAD -- 'package/*' 'board/*' 'Config.in*' 'configs/*' 'makefile' 'scripts/*' 2>/dev/null || echo ""
}

get_git_diff_hash() {
    local prev_commit="${1:-HEAD~1}"
    git diff "${prev_commit}"...HEAD -- 'package/*' 'board/*' 'Config.in*' 'configs/*' 2>/dev/null | sha256sum | cut -d' ' -f1
}

analyze_dependencies() {
    log "Analyzing dependencies and changes..."
    
    local changes
    changes=$(get_git_changes)
    DIFF_HASH=$(get_git_diff_hash)
    
    local needs_toolchain=false
    local needs_system=false
    local needs_packages=false
    local needs_image=true
    
    if [ -z "$changes" ]; then
        log "  No changes detected"
    elif echo "$changes" | grep -qE '^(toolchain/|package/.*/toolchain|Config.in|configs/)'; then
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
        log "  → Changes in scripts/configs, partial rebuild"
    fi
    
    echo ""
    echo "=== Changed Files (${#changes} files) ==="
    echo "$changes" | head -20
    echo "..."
    echo ""
    
    export NEEDS_TOOLCHAIN=$needs_toolchain
    export NEEDS_SYSTEM=$needs_system
    export NEEDS_PACKAGES=$needs_packages
    export NEEDS_IMAGE=$needs_image
    export CHANGES=$changes
    export DIFF_HASH
    
    echo "=== Build Decision ==="
    echo "Diff Hash:    ${DIFF_HASH:0:16}..."
    echo "Toolchain:    $needs_toolchain"
    echo "System:       $needs_system"
    echo "Packages:     $needs_packages"
    echo "Image:        $needs_image"
    echo ""
}

check_artifacts() {
    local artifact_dir="${CACHE_DIR}/artifacts/${ARCH}"
    
    log "Checking existing artifacts..."
    
    local has_toolchain=false
    local has_system=false
    local has_packages=false
    
    if [ -d "${artifact_dir}/toolchain" ] && [ -f "${artifact_dir}/.config" ]; then
        local cached_hash
        cached_hash=$(cat "${artifact_dir}/toolchain/.diff_hash" 2>/dev/null || echo "")
        if [ "$cached_hash" = "$DIFF_HASH" ] || [ "$FORCE_CLEAN" = "1" ]; then
            has_toolchain=true
            log "  ✓ Toolchain artifact found (hash match)"
        else
            log "  ⚠ Toolchain artifact found but hash mismatch (will rebuild)"
            rm -rf "${artifact_dir}/toolchain"
        fi
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
    else
        log "BLAKE3 not available, using SHA256..."
        sha256sum * > SHA256SUMS.txt
    fi
}

verify_checksums() {
    local dir="$1"
    local checksum_tool=$(check_blake3)
    
    cd "${dir}"
    
    if [ "$checksum_tool" != "none" ] && [ -f "BLAKE3SUMS.txt" ]; then
        log "Verifying BLAKE3 checksums..."
        if $checksum_tool --check BLAKE3SUMS.txt 2>/dev/null; then
            log "  ✓ BLAKE3 verification passed"
            return 0
        else
            log "  ✗ BLAKE3 verification FAILED"
            return 1
        fi
    elif [ -f "SHA256SUMS.txt" ]; then
        log "Verifying SHA256 checksums..."
        if sha256sum --check SHA256SUMS.txt 2>/dev/null; then
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
            mkdir -p buildroot/output 2>/dev/null || true
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
    rm -rf "${artifact_dir}"/* 2>/dev/null || true
    
    cp -r buildroot/output "${artifact_dir}/" 2>/dev/null || true
    cp buildroot/.config "${artifact_dir}/" 2>/dev/null || true
    
    echo "$DIFF_HASH" > "${artifact_dir}/.diff_hash"
    
    generate_checksums "${artifact_dir}"
}

build_in_docker() {
    local target="$1"
    local log_file="$2"
    
    local cache_volumes="-v ${CACHE_DIR}/dl:/share/dl -v ${CACHE_DIR}/ccache:/share/ccache"
    
    docker run --rm --security-opt seccomp=unconfined \
        -v "${SCRIPT_DIR}:/work" \
        $cache_volumes \
        -e "ARCH=${ARCH}" \
        -e "FORCE_UNSAFE_CONFIGURE=1" \
        -e "RECALBOX_CCACHE_ENABLED=1" \
        -e "MAKE_JOBS=${MAKE_JOBS}" \
        -e "BR2_DL_DIR=/share/dl" \
        recalbox-build:latest \
        make -j${MAKE_JOBS} ${target} 2>&1 | tee "${LOG_DIR}/${log_file}"
}

setup_buildroot() {
    log "Setting up buildroot..."
    
    if [ ! -d 'buildroot' ]; then
        log "Cloning buildroot..."
        git clone --depth 1 --branch 2024.02 https://git.busybox.net/buildroot buildroot
    fi
    
    cd buildroot
    git fetch --depth 1 origin e4a5ab3b319753f41e2b5cf22e90b6b0304ca225
    git checkout e4a5ab3b319753f41e2b5cf22e90b6b0304ca225
    cd ..
    
    log "Making defconfig..."
    docker run --rm \
        -v "${SCRIPT_DIR}:/work" \
        -v "${CACHE_DIR}/dl:/share/dl" \
        -w /work \
        recalbox-build:latest \
        make "recalbox-${ARCH}_defconfig"
}

build_docker_image() {
    log "Building Docker image..."
    docker build -t recalbox-build:latest "${SCRIPT_DIR}"
}

run_step() {
    local step_name="$1"
    local step_log="$2"
    local cmd="$3"
    
    log "Starting: ${step_name}"
    START_TIME=$(date +%s)
    
    eval "${cmd}" 2>&1 | tee "${LOG_DIR}/${step_log}.log" || {
        log "ERROR: ${step_name} failed!"
        return 1
    }
    
    END_TIME=$(date +%s)
    ELAPSED=$((END_TIME - START_TIME))
    log "Done: ${step_name} (${ELAPSED}s)"
    
    echo "${step_name}|${ELAPSED}" >> "${LOG_DIR}/timing.csv"
}

show_summary() {
    echo ""
    echo "============================================"
    echo "  Build Summary"
    echo "============================================"
    
    if [ -f "${LOG_DIR}/timing.csv" ]; then
        echo ""
        echo "=== Timing ==="
        while IFS='|' read -r step time; do
            printf "  %-20s %6ds\n" "$step" "$time"
        done < "${LOG_DIR}/timing.csv"
    fi
    
    echo ""
    echo "=== Cache Usage ==="
    du -sh "${CACHE_DIR}/artifacts/${ARCH}" 2>/dev/null || echo "  No artifacts cached"
    du -sh "${CACHE_DIR}/dl" 2>/dev/null || echo "  No downloads cached"
    du -sh "${CACHE_DIR}/ccache" 2>/dev/null || echo "  No ccache cached"
    
    echo ""
    echo "=== Artifacts ==="
    for a in $(get_architectures); do
        local dist_dir="${SCRIPT_DIR}/dist/${a}"
        if [ -d "$dist_dir" ]; then
            echo "  ${a}:"
            ls -lh "$dist_dir" | tail -n +2 | awk '{print "    " $9 " (" $5 ")"}'
        fi
    done
}

get_architectures() {
    if [ "$ARCH" = "all" ]; then
        echo "x86_64 rpi5_64"
    else
        echo "$ARCH"
    fi
}

build_architecture() {
    local build_arch="$1"
    ARCH="$build_arch"
    
    log "============================================"
    log "  Building: ${build_arch}"
    log "============================================"
    
    if [ "$DRY_RUN" = "1" ]; then
        log "DRY RUN: Would build ${build_arch}"
        return 0
    fi
    
    analyze_dependencies
    check_artifacts
    
    if [ "$VALIDATE_ONLY" = "1" ]; then
        log "Validation complete (validate only mode)"
        return 0
    fi
    
    setup_buildroot
    
    if [ ! -d "buildroot/output" ] || [ "$FORCE_CLEAN" = "1" ]; then
        restore_artifact "toolchain" || true
        restore_artifact "system" || true
        restore_artifact "packages" || true
    fi
    
    if [ "$NEEDS_TOOLCHAIN" = "true" ] && [ "$HAS_TOOLCHAIN" = "false" ]; then
        run_step "Toolchain" "${build_arch}_toolchain" "build_in_docker toolchain ${build_arch}_toolchain"
        save_artifact "toolchain"
    elif [ "$HAS_TOOLCHAIN" = "true" ]; then
        log "Skipping toolchain (using cached artifact)"
    fi
    
    if [ "$NEEDS_SYSTEM" = "true" ]; then
        run_step "System" "${build_arch}_system" "build_in_docker system ${build_arch}_system"
        save_artifact "system"
    else
        log "System not required"
    fi
    
    if [ "$NEEDS_PACKAGES" = "true" ]; then
        run_step "Packages" "${build_arch}_packages" "build_in_docker packages ${build_arch}_packages"
        save_artifact "packages"
    else
        log "Packages not required"
    fi
    
    run_step "Image" "${build_arch}_image" "build_in_docker '' ${build_arch}_image"
    
    local dist_dir="${SCRIPT_DIR}/dist/${build_arch}"
    mkdir -p "${dist_dir}"
    
    if [ -f "output/images/recalbox/recalbox.img" ]; then
        cp output/images/recalbox/* "${dist_dir}/"
        cp output/images/recalbox/*.zip "${dist_dir}/" 2>/dev/null || true
        
        generate_checksums "${dist_dir}"
        verify_checksums "${dist_dir}"
        
        log "Build complete for ${build_arch}!"
        ls -lh "${dist_dir}"
    else
        log "ERROR: No image generated for ${build_arch}!"
        return 1
    fi
}

main() {
    echo "============================================"
    echo "  Recalbox Local Build Validation"
    echo "  Version: 1.0"
    echo "============================================"
    echo ""
    
    detect_environment
    init_directories
    
    if [ "$VALIDATE_ONLY" = "1" ]; then
        log "Running in validation-only mode"
        for a in $(get_architectures); do
            ARCH="$a" check_artifacts
        done
        return 0
    fi
    
    if ! docker image inspect recalbox-build:latest >/dev/null 2>&1; then
        build_docker_image
    else
        log "Docker image already exists (recalbox-build:latest)"
    fi
    
    if [ "$DRY_RUN" = "1" ]; then
        log "Running in dry-run mode"
        analyze_dependencies
        echo ""
        log "Would build for architectures: $(get_architectures)"
        return 0
    fi
    
    BUILD_START=$(date +%s)
    
    for a in $(get_architectures); do
        build_architecture "$a"
    done
    
    BUILD_END=$(date +%s)
    TOTAL_TIME=$((BUILD_END - BUILD_START))
    
    echo ""
    echo "============================================"
    echo "  BUILD COMPLETE"
    echo "============================================"
    echo "Total time: $((TOTAL_TIME / 60))m $((TOTAL_TIME % 60))s"
    
    show_summary
    
    echo ""
    echo "Ready to push? Run:"
    echo "  git push"
    echo ""
}

main "$@"
