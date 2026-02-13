################################################################################
#
# libaltsound
#
################################################################################

LIBALTSOUND_VERSION = b8f397858cbc7a879f7392c14a509f00c8bdc7dd
LIBALTSOUND_SITE = $(call github,vpinball,libaltsound,$(LIBALTSOUND_VERSION))
LIBALTSOUND_LICENSE = BSD-3-Clause
LIBALTSOUND_LICENSE_FILES = LICENSE
LIBALTSOUND_DEPENDENCIES =
LIBALTSOUND_SUPPORTS_IN_SOURCE_BUILD = NO
LIBALTSOUND_EXTRA_DOWNLOADS = https://www.un4seen.com/files/bass24-linux.zip

LIBALTSOUND_CONF_OPTS += -DCMAKE_BUILD_TYPE=Release
LIBALTSOUND_CONF_OPTS += -DBUILD_STATIC=OFF
LIBALTSOUND_CONF_OPTS += -DPLATFORM=linux
LIBALTSOUND_CONF_OPTS += -DARCH=$(BUILD_ARCH)

ifeq ($(BR2_PACKAGE_RECALBOX_TARGET_X86_64),y)
    BUILD_ARCH = x64
    BASS_ARCH = x86_64
endif

ifeq ($(BR2_PACKAGE_RECALBOX_TARGET_RPI5_64),y)
    BUILD_ARCH = aarch64
    BASS_ARCH = aarch64
endif

define LIBALTSOUND_BASS_HACKS
	ls $(LIBALTSOUND_DL_DIR)
	cd $(LIBALTSOUND_DL_DIR) && unzip -o -x bass24-linux.zip -d bass24
	if [ -f $(LIBALTSOUND_DL_DIR)/bass24/bass.h ] ; \
	then \
		cp $(LIBALTSOUND_DL_DIR)/bass24/bass.h $(@D)/third-party/include ; \
	else \
		cp $(LIBALTSOUND_DL_DIR)/bass24/c/bass.h $(@D)/third-party/include ; \
	fi;
	cp $(LIBALTSOUND_DL_DIR)/bass24/libs/$(BASS_ARCH)/libbass.so \
	$(@D)/third-party/runtime-libs/linux/$(BUILD_ARCH)
endef

# Install to staging to build Visual Pinball Standalone
LIBALTSOUND_INSTALL_STAGING = YES

LIBALTSOUND_PRE_CONFIGURE_HOOKS += LIBALTSOUND_BASS_HACKS

$(eval $(cmake-package))
