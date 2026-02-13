# utilities
# how to use this file:
# in your bash script:
#
# source /recalbox/scripts/recalbox-utils.sh
#

RECALBOX_ARCH_FILE="/recalbox/recalbox.arch"

# Return the current running arch (or board)
# sanitized
# returns 0 on success and the name of the board on stdout
# returns 1 on error and unknown on stdout
getArchName() {
  local ARCH
  if [ -f "$RECALBOX_ARCH_FILE" ]; then
    ARCH=$(head -n 1 "$RECALBOX_ARCH_FILE" | tr '[:upper:]' '[:lower:]')
    if [ -n "$ARCH" ]; then
      echo "$ARCH"
      return 0
    else
      echo "unknown"
      return 1
    fi
  else
    echo "unknown"
    return 1
  fi
}

# Test if the current board allows mounting external share (USB, not network)
# i.e sharedevice=ANYEXTERNAL or sharedevice=DEV fs-uuid from /boot/recalbox-boot.conf
# currently, odroidgo2 does not support mounting USB share as
# it becomes unavailable after power suspend
# return 0 if the current board support mounting an external share
# return 1 if the current board DOES NOT support mounting an external share
doesBoardSupportExternalShare() {
  if [ "$(getArchName)" = 'odroidgo2' ]; then
    return 1
  fi
  return 0
}

# Returns path to directory containing images resources
getInstallUpgradeImagePath() {
  echo "/recalbox/system/resources"
}

# Get the best CRT MPV Options
getCrtMpvOptions() {
  # RGBDual 2 have composite support
  if isRecalboxRGBDUAL2; then
    if isRecalboxRGBDUAL2ForceComposite; then
      local connector="$(findConnectedConnectors | grep Composite)"
      if [[ -z "${connector}" ]]; then return 1; fi
      if isRecalboxRGBDUAL2Force50Hz; then
        # Composite PAL
        echo "--vo=drm --drm-connector=${connector} --drm-mode=$(mpv --drm-mode=help | grep "720x576i" | awk '{print $2}' | cut -c '1')"
        return
      else
        # Composite NTSC
        echo "--vo=drm --drm-connector=${connector} --drm-mode=$(mpv --drm-mode=help | grep "720x480i" | awk '{print $2}' | cut -c '1')"
        return
      fi
    fi
  fi

  # Find the connected connector (contains VGA)
  local connector="$(findConnectedConnectors | grep VGA)"
  if [[ -z "${connector}" ]]; then return 1; fi
  local command="--vo=drm --drm-connector=${connector}"
  echo "${command}"
  # Force 640x480i/p mode if available, except on jamma
  # if ! isRecalboxRGBJamma; then
  #     if mpv --drm-mode=help | grep -q "640x480"; then
  #       command="${command} --drm-mode=$(mpv --drm-mode=help | grep 640x480 | awk '{print $2}' | cut -c '1')"
  #     fi
  # fi
}

# findConnectedConnectors
#   returns a list of connected connectors (using mpv)
findConnectedConnectors() {
  grep -l ^connected /sys/class/drm/card*/status | sed 's/.*card\([0-9]\+\)-\([^\/]\+\).*/\2/'
}

isLowDef() {
    if [ "$(cut -d, -f2 /sys/class/graphics/fb0/virtual_size)" -le 320 ] \
       && ! isCRTLoaded; then
      return 0
    fi
    return 1
}

# Return if a crt is connected
currentVideoOnCRT() {
    grep -q "connected" /sys/class/drm/card*-VGA*/status || \
    grep -q "connected" /sys/class/drm/card*-Composite*/status
}

# Check if we are on Recalbox RGB Dual
isRecalboxRGBDual() {
  local firmwareFile=/sys/firmware/devicetree/base/hat/product
  if [ -e "$firmwareFile" ] && grep -q "Recalbox RGB Dual" "$firmwareFile";then
    return 0
  else
    return 1
  fi
}

# Check if we are on Recalbox RGB Dual
isCRTLoaded() {
  lsmod | grep -q "^recalboxrgbdual"
}

isRecalboxRGBJamma() {
    isCRTLoaded && lsmod | grep -q "^recalboxrgbjamma"
}

isRecalboxRGBJamma2() {
    isRecalboxRGBJamma && test -f "/sys/firmware/devicetree/base/recalboxrgbjamma/recalbox-rgb-jamma-2"
}

# Check if we have to disable HDMI
# See S13crt
rrgbdHDMIPriority() {
  local CRT_OPTIONS_FILE="/boot/crt/recalbox-crt-options.cfg"
  ! grep -q "video.forcehdmi = 0" "${CRT_OPTIONS_FILE}"
}

isRotated() {
    if [ "$(getRotationIndex)" != "0" ];then
        return 0
    fi
    return 1
}

isRotatedLeftOrRight() {
    local rotation="$(getRotationIndex)"
    if [ "${rotation}" == "1" ] || [ "${rotation}" == "3" ];then
        return 0
    fi
    return 1
}
getRotationIndex() {
    if grep -q "^screen.rotation=.*" /boot/recalbox-boot.conf;then
        grep "^screen.rotation=.*" /boot/recalbox-boot.conf | cut -d"=" -f2
        return 0
    fi
    echo "0"
}
getRotationAngle() {
    local rot=$(getRotationIndex)
    if [ "${rot}" == "1" ];then echo "270"; return; fi
    if [ "${rot}" == "2" ];then echo "180"; return; fi
    if [ "${rot}" == "3" ];then echo "90"; return; fi
    echo "0"
}

# Get the best MPV Options considering board, rrgbd and rotation
getMpvOptions() {
    local rotationcli=""
    if isRotated; then
        local rotation="$(getRotationAngle)"
        if [ "${rotation}" == "270" ] || [ "${rotation}" == "180" ];then
            # Videos will be already rotated, but we need to rotate upsidedown
            rotationcli="--video-rotate=180"
        fi
    fi
    if currentVideoOnCRT; then
        echo "${rotationcli} $(getCrtMpvOptions)"
    else
        echo "${rotationcli}"
    fi
}

# Returns true if the platform may need to force enable DRM connector.
# This is related to black screen issue
doesBoardNeedKMSManager() {
  case "$(getArchName)" in
    rpizero2|rpi3|rpi4|rpi4_64|rpi5_64)
      return 0 ;;
    *)
      return 1 ;;
  esac
}

# Returns if any display connector is connected (kms dri compatible only)
isScreenConnected() {
    local RETURNCODE="-1"
    if [ -d /sys/class/drm ]; then
        readarray -t CARD_PATH < <(find /sys/class/drm -name "card?-*" ! -name "card?-VGA-*" ! -name "card?-Composite-*" -maxdepth 1 2>/dev/null)
        grep -q '^connected' "${CARD_PATH[@]/%//status}"
        RETURNCODE="$?"
    fi
    return $RETURNCODE
}

# Disable dpi connectors
disableDPIConnectors() {
    for card in $(ls /sys/class/drm/ | grep VGA); do
        echo "off" > "/sys/class/drm/${card}/status"
    done
}

# Disable HDMI connectors
disableHDMIConnectors() {
    for card in $(ls /sys/class/drm/ | grep HDMI); do
        echo "off" > "/sys/class/drm/${card}/status"
    done
}

# Enable the DPI connector
enableDPIConnectors() {
    for card in $(ls /sys/class/drm/ | grep VGA); do
        echo "on" > "/sys/class/drm/${card}/status"
    done
}
##
# getBoardName
# return the detected board from /boot/recalbox-boot.conf
# can be one of GPiV1, PiBoy, none.
# If detection has not already occured, the response will be empty
getBoardName() {
  sed -E '/^\s*case=/!d;s/\s*case=([^:]+):.*/\1/' /boot/recalbox-boot.conf
}

# getStepNumber
# return the step number (empty, 0 or 1) from /boot/recalbox-boot.conf
getStepNumber() {
  sed -E '/^\s*case=/!d;s/\s*case=[^:]+:(.*)/\1/' /boot/recalbox-boot.conf
}

# isOldIntelChipset
# chipset number is 4 hexa digit (without the 8086)
# return 0 if the given device id mathces a Gen3 and older GPU
isOldIntelChipset() {
  local OLD_CHIPSETS=(7800 1240 7121 7123 7125 1132 \
                      2562 3577 2572 3582 358e 3582 \
                      2582 258a 2592 2772 27a2 27ae 29d2 29b2 29c2 a001 a011 \
                     )
  local chipset=$(echo "$1" | tr "[:upper:]" "[:lower:]") 
  for old_chipset in "${OLD_CHIPSETS[@]}"; do
    if [ "$chipset" = "$old_chipset" ]; then
     return 0
    fi 
  done
  return 1
}

isThonEdition() {
  local CRT_OPTIONS_FILE="/boot/crt/recalbox-crt-options.cfg"
  grep -q "special.thon = 1" "${CRT_OPTIONS_FILE}"
}

# showIntroBackground
#   displays our intro background via the framebuffer
displayFrameBufferImage() {
    if isRecalboxRGBJamma; then
        if isThonEdition; then SPECIAL="-thon";fi
        fbv2 -f -i "/recalbox/system/resources/splash/rrgbj/recalbox-rgb-jamma${SPECIAL}-$(getRotationIndex).png"
        return 0
    fi
    if isRotated; then
        fbv2 -k -i "/recalbox/system/resources/splash/tate/logo-$(getRotationIndex).png"
    else
        if isLowDef && ! isRecalboxRGBDual; then
            fbv2 -k -i /recalbox/system/resources/splash/240p/logo-version.png
        else
            fbv2 -k -i /recalbox/system/resources/splash/logo-version.png
        fi
    fi
}

# doesBoardNeedSDLRotation
#   return true (0) if board has a 270° rotated screen
doesBoardNeedSDLRotation() {
  grep -E -q '(ODROID-GO[123]?$|RG351(P|M))' /proc/cpuinfo
}

# RGBDual 2
function isRecalboxRGBDUAL2Plugged {
  test -d "/sys/bus/platform/devices/recalboxrgbdual2" || \
  test -f "/tmp/recalboxhardware/recalboxrgbdual2-proto1" || \
  test -f "/tmp/recalboxhardware/recalboxrgbdual2-proto2" || \
  test -f "/tmp/recalboxhardware/recalboxrgbdual2-proto3"
}

function isRecalboxRGBDUAL2Loaded {
  test -d "/sys/bus/platform/devices/recalboxrgbdual2"
}

function isRecalboxRGBDUAL2 {
  test -d "/sys/bus/platform/devices/recalboxrgbdual2" && isCRTLoaded
}

function isRecalboxRGBDUAL2ForceComposite {
  grep -q "0" /sys/devices/platform/recalboxrgbdual2/dipswitch-composite/value
}
function isRecalboxRGBDUAL2Force50Hz {
  grep -q "0" /sys/devices/platform/recalboxrgbdual2/dipswitch-50hz/value
}

# getRotation
#   returns rotation in degrees to operate on video to make it appears correctly
#   drm is checked for a given connector
# input:
# * drm connector
# output:
# * 0, 90 180 or 270
getRotation() {
  local conn="$1"

  local panel_orientation="0"
  if [ -f "/usr/bin/recalbox-drm-info" ]; then
    panel_orientation=$(recalbox-drm-info -d | grep "$conn" | cut -d : -f 4)
  fi
  local panel_rotation=""
  case "$panel_orientation" in
    "Right Side Up") panel_rotation="90";;
    "Left Side Up") panel_rotation="270";;
    "Upside Down") panel_rotation="180";;
   *) panel_rotation="0";;
  esac
  # No we should add tate rotation
  tate_rotation=$(getRotationAngle)
  total_rotation=$(( ( tate_rotation + panel_rotation) % 360 ))
  echo "$total_rotation"
}

useWayland() {
  count=$(ps aux | grep dwl | grep -v grep | wc -l)
  if [ "$count" -gt "0" ]; then
    return 0
  else
    return 1
  fi
}

function getCurrentCrtDriDevice() {
  for card in "$(ls /sys/class/drm/ | grep VGA)" "$(ls /sys/class/drm/ | grep Composite)"; do
    if [ -n "$card" ] && grep -q "^connected" /sys/class/drm/"$card"/status; then 
      dev="/dev/$(grep DEVNAME /sys/class/drm/"$card"/device/uevent | awk -F= '{print $2}')"
      echo "${dev}"
      return 0
    fi
  done
  return 1
}

autoRunEnabled() {
  local systemsetting="recalbox_settings"
  autorun="$($systemsetting -command load -key autorun.enabled -source /boot/recalbox-backup.conf -default 1)"
  uuid="$($systemsetting -command load -key autorun.uuid -source /boot/recalbox-backup.conf)"
  path="$($systemsetting -command load -key autorun.path -source /boot/recalbox-backup.conf)"
  if [[ "$autorun" == "1" && ! -z "$uuid" && ! -z "$path" ]]; then
    return 0
  fi
  return 1
}

displayImage() {
  killall mpv
  findConnectedConnectors |while read -r conn; do
	rotate_option=$(getRotation "$conn")
	mpv $(getCrtMpvOptions) --image-display-duration="${2}" --video-rotate="${rotate_option}" "${1}" &
  done
}
