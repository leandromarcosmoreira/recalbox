#!/bin/bash

set -euo pipefail

cd /home/lemoreira/git/recalbox

LOG_DIR="/home/lemoreira/git/recalbox/build-logs"
TIMESTAMPS_FILE="$LOG_DIR/timestamps.log"
CACHE_DIR="${HOME}/.recalbox-cache"
DL_DIR="${CACHE_DIR}/dl"
CCACHE_DIR="${CACHE_DIR}/ccache"

mkdir -p "$LOG_DIR"
mkdir -p "$DL_DIR"
mkdir -p "$CCACHE_DIR"

log_time() {
    local label="$1"
    local timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    local epoch=$(date '+%s')
    echo "[$timestamp] $label" | tee -a "$TIMESTAMPS_FILE"
    echo "$epoch $label" >> "$LOG_DIR/timing.csv"
}

calculate_duration() {
    local start=$1
    local end=$2
    local duration=$((end - start))
    echo "$duration"
}

to_minutes() {
    local secs=$1
    echo "scale=1; $secs/60" | awk '{printf "%.1f", $1}'
}

echo "=========================================="
echo "  Recalbox Build Audit & Performance"
echo "=========================================="
echo ""
echo "Log directory: $LOG_DIR"
echo "Cache dir: $CACHE_DIR"
echo ""

START_TOTAL=$(date '+%s')
log_time "BUILD START"

log_time "STEP 1: Docker image already built"
DOCKER_TIME=0
echo "Docker image: using cached (already built)"
echo ""

log_time "STEP 2: Setup buildroot + merge recalbox..."
START_STEP=$(date '+%s')

if [ ! -d "/home/lemoreira/git/recalbox/buildroot" ]; then
    echo "Cloning buildroot..."
    git clone --depth 1 --branch 2024.02 https://git.busybox.net/buildroot /home/lemoreira/git/recalbox/buildroot 2>&1 | tee -a "$LOG_DIR/buildroot-clone.log"
fi

cd /home/lemoreira/git/recalbox/buildroot
git fetch --depth 1 origin e4a5ab3b319753f41e2b5cf22e90b6b0304ca225 2>&1 | tee -a "$LOG_DIR/buildroot-update.log"
git checkout e4a5ab3b319753f41e2b5cf22e90b6b0304ca225 2>&1 | tee -a "$LOG_DIR/buildroot-update.log"

chmod -R 777 /home/lemoreira/git/recalbox/buildroot 2>/dev/null || true
chmod -R 777 /home/lemoreira/git/recalbox/output 2>/dev/null || true

cd /home/lemoreira/git/recalbox

END_STEP=$(date '+%s')
BUILDROOT_TIME=$(calculate_duration $START_STEP $END_STEP)
log_time "Buildroot setup COMPLETE (${BUILDROOT_TIME}s)"
echo "Buildroot setup time: ${BUILDROOT_TIME}s"
echo ""

log_time "STEP 3: Configure x86_64..."
START_STEP=$(date '+%s')

docker run --rm --security-opt seccomp=unconfined \
    -v "/home/lemoreira/git/recalbox:/recalbox" \
    -v "/home/lemoreira/git/recalbox/buildroot:/buildroot" \
    -v "${DL_DIR}:/share/dl" \
    -v "${CCACHE_DIR}:/share/ccache" \
    -e "ARCH=x86_64" \
    -e "FORCE_UNSAFE_CONFIGURE=1" \
    -e "RECALBOX_CCACHE_ENABLED=1" \
    -e "BR2_DL_DIR=/share/dl" \
    -e "BR2_EXTERNAL=/recalbox" \
    -e "BR2_PACKAGE_OVERRIDE_FILE=/recalbox/local.mk" \
    -e "O=/recalbox/output" \
    -w /recalbox \
    recalbox-build:latest \
    make recalbox-x86_64_defconfig 2>&1 | tee "$LOG_DIR/config-x86_64.log"

END_STEP=$(date '+%s')
CONFIG_TIME=$(calculate_duration $START_STEP $END_STEP)
log_time "Configure x86_64 COMPLETE (${CONFIG_TIME}s)"
echo "Config time: ${CONFIG_TIME}s"
echo ""

log_time "STEP 4: Build Toolchain..."
START_STEP=$(date '+%s')

docker run --rm --security-opt seccomp=unconfined \
    -v "/home/lemoreira/git/recalbox:/recalbox" \
    -v "/home/lemoreira/git/recalbox/buildroot:/buildroot" \
    -v "${DL_DIR}:/share/dl" \
    -v "${CCACHE_DIR}:/share/ccache" \
    -e "ARCH=x86_64" \
    -e "FORCE_UNSAFE_CONFIGURE=1" \
    -e "RECALBOX_CCACHE_ENABLED=1" \
    -e "BR2_DL_DIR=/share/dl" \
    -e "BR2_EXTERNAL=/recalbox" \
    -e "BR2_PACKAGE_OVERRIDE_FILE=/recalbox/local.mk" \
    -e "O=/recalbox/output" \
    -e "CCACHE_DIR=/share/ccache" \
    -w /recalbox \
    recalbox-build:latest \
    make toolchain 2>&1 | tee "$LOG_DIR/toolchain.log" || true

END_STEP=$(date '+%s')
TOOLCHAIN_TIME=$(calculate_duration $START_STEP $END_STEP)
log_time "Toolchain build COMPLETE (${TOOLCHAIN_TIME}s)"
echo "Toolchain time: ${TOOLCHAIN_TIME}s"
echo ""

log_time "STEP 5: Build System..."
START_STEP=$(date '+%s')

docker run --rm --security-opt seccomp=unconfined \
    -v "/home/lemoreira/git/recalbox:/recalbox" \
    -v "/home/lemoreira/git/recalbox/buildroot:/buildroot" \
    -v "${DL_DIR}:/share/dl" \
    -v "${CCACHE_DIR}:/share/ccache" \
    -e "ARCH=x86_64" \
    -e "FORCE_UNSAFE_CONFIGURE=1" \
    -e "RECALBOX_CCACHE_ENABLED=1" \
    -e "BR2_DL_DIR=/share/dl" \
    -e "BR2_EXTERNAL=/recalbox" \
    -e "BR2_PACKAGE_OVERRIDE_FILE=/recalbox/local.mk" \
    -e "O=/recalbox/output" \
    -e "CCACHE_DIR=/share/ccache" \
    -w /recalbox \
    recalbox-build:latest \
    make system 2>&1 | tee "$LOG_DIR/system.log" || true

END_STEP=$(date '+%s')
SYSTEM_TIME=$(calculate_duration $START_STEP $END_STEP)
log_time "System build COMPLETE (${SYSTEM_TIME}s)"
echo "System time: ${SYSTEM_TIME}s"
echo ""

log_time "STEP 6: Build Packages..."
START_STEP=$(date '+%s')

docker run --rm --security-opt seccomp=unconfined \
    -v "/home/lemoreira/git/recalbox:/recalbox" \
    -v "/home/lemoreira/git/recalbox/buildroot:/buildroot" \
    -v "${DL_DIR}:/share/dl" \
    -v "${CCACHE_DIR}:/share/ccache" \
    -e "ARCH=x86_64" \
    -e "FORCE_UNSAFE_CONFIGURE=1" \
    -e "RECALBOX_CCACHE_ENABLED=1" \
    -e "BR2_DL_DIR=/share/dl" \
    -e "BR2_EXTERNAL=/recalbox" \
    -e "BR2_PACKAGE_OVERRIDE_FILE=/recalbox/local.mk" \
    -e "O=/recalbox/output" \
    -e "CCACHE_DIR=/share/ccache" \
    -w /recalbox \
    recalbox-build:latest \
    make packages 2>&1 | tee "$LOG_DIR/packages.log" || true

END_STEP=$(date '+%s')
PACKAGES_TIME=$(calculate_duration $START_STEP $END_STEP)
log_time "Packages build COMPLETE (${PACKAGES_TIME}s)"
echo "Packages time: ${PACKAGES_TIME}s"
echo ""

log_time "STEP 7: Build Final Image..."
START_STEP=$(date '+%s')

docker run --rm --security-opt seccomp=unconfined \
    -v "/home/lemoreira/git/recalbox:/recalbox" \
    -v "/home/lemoreira/git/recalbox/buildroot:/buildroot" \
    -v "${DL_DIR}:/share/dl" \
    -v "${CCACHE_DIR}:/share/ccache" \
    -e "ARCH=x86_64" \
    -e "RECALBOX_VERSION=local-test" \
    -e "FORCE_UNSAFE_CONFIGURE=1" \
    -e "RECALBOX_CCACHE_ENABLED=1" \
    -e "BR2_DL_DIR=/share/dl" \
    -e "BR2_EXTERNAL=/recalbox" \
    -e "BR2_PACKAGE_OVERRIDE_FILE=/recalbox/local.mk" \
    -e "O=/recalbox/output" \
    -e "CCACHE_DIR=/share/ccache" \
    -w /recalbox \
    recalbox-build:latest \
    make 2>&1 | tee "$LOG_DIR/image.log" || true

END_STEP=$(date '+%s')
IMAGE_TIME=$(calculate_duration $START_STEP $END_STEP)
log_time "Image build COMPLETE (${IMAGE_TIME}s)"
echo "Image time: ${IMAGE_TIME}s"
echo ""

END_TOTAL=$(date '+%s')
TOTAL_TIME=$(calculate_duration $START_TOTAL $END_TOTAL)
log_time "BUILD END - TOTAL: ${TOTAL_TIME}s"

echo ""
echo "=========================================="
echo "  BUILD PERFORMANCE SUMMARY"
echo "=========================================="
echo ""
echo "| Step              | Time (s)  | Time (min) |"
echo "|-------------------|-----------|------------|"
printf "| Docker Build      | %7d | %9s |\n" "$DOCKER_TIME" "0.0"
printf "| Buildroot Setup  | %7d | %9s |\n" "$BUILDROOT_TIME" "$(to_minutes $BUILDROOT_TIME)"
printf "| Configure        | %7d | %9s |\n" "$CONFIG_TIME" "$(to_minutes $CONFIG_TIME)"
printf "| Toolchain        | %7d | %9s |\n" "$TOOLCHAIN_TIME" "$(to_minutes $TOOLCHAIN_TIME)"
printf "| System           | %7d | %9s |\n" "$SYSTEM_TIME" "$(to_minutes $SYSTEM_TIME)"
printf "| Packages         | %7d | %9s |\n" "$PACKAGES_TIME" "$(to_minutes $PACKAGES_TIME)"
printf "| Image            | %7d | %9s |\n" "$IMAGE_TIME" "$(to_minutes $IMAGE_TIME)"
echo "|-------------------|-----------|------------|"
printf "| TOTAL            | %7d | %9s |\n" "$TOTAL_TIME" "$(to_minutes $TOTAL_TIME)"
echo ""

echo "Log files saved to: $LOG_DIR"
echo ""

echo "=========================================="
echo "  TOP GARGALOS (by build time)"
echo "=========================================="
echo ""

echo "Top 10 slowest package builds:"
if [ -f "$LOG_DIR/packages.log" ]; then
    grep -E ">>> .*" "$LOG_DIR/packages.log" 2>/dev/null | head -20 || echo "  (analyzing log...)"
fi
echo ""

echo "Percentage of total time:"
if [ "$TOTAL_TIME" -gt 0 ]; then
    echo "  Docker:        0% (cached)"
    printf "  Buildroot:     %d%%\n" $((BUILDROOT_TIME * 100 / TOTAL_TIME))
    printf "  Config:        %d%%\n" $((CONFIG_TIME * 100 / TOTAL_TIME))
    printf "  Toolchain:     %d%%\n" $((TOOLCHAIN_TIME * 100 / TOTAL_TIME))
    printf "  System:        %d%%\n" $((SYSTEM_TIME * 100 / TOTAL_TIME))
    printf "  Packages:      %d%%\n" $((PACKAGES_TIME * 100 / TOTAL_TIME))
    printf "  Image:         %d%%\n" $((IMAGE_TIME * 100 / TOTAL_TIME))
fi
