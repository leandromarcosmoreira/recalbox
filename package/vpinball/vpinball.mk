################################################################################
#
# vpinball
#
################################################################################

VPINBALL_VERSION = 7b36d7b9a7f42b21775eba45cf8f71804a1bdd98
VPINBALL_SITE = $(call github,vpinball,vpinball,$(VPINBALL_VERSION))
VPINBALL_LICENSE = GPLv3+
VPINBALL_LICENSE_FILES = LICENSE
VPINBALL_DEPENDENCIES = libfreeimage libpinmame libaltsound libdmdutil libdof sdl3 sdl3_image sdl3_ttf sdl3_mixer ffmpeg
VPINBALL_SUPPORTS_IN_SOURCE_BUILD = NO
VPINBALL_EXTRA_DOWNLOADS = https://www.un4seen.com/files/bass24-linux.zip

ifeq ($(BR2_PACKAGE_RECALBOX_TARGET_X86_64),y)
    SOURCE = CMakeLists_gl-linux-x64.txt
    SOURCE_DIR = linux-x64
    ARCH = x86_64
endif

ifeq ($(BR2_PACKAGE_RECALBOX_TARGET_RPI5_64),y)
    SOURCE = CMakeLists_gl-linux-aarch64.txt
    SOURCE_DIR = linux-aarch64
    ARCH = aarch64
    VPINBALL_CONF_OPTS += "-DBUILD_RPI=ON"
endif

define VPINBALL_CMAKE_HACKS
	## derived from standalone/linux/external.sh ##
	# copy linux x64
	cp $(@D)/make/$(SOURCE) $(@D)/CMakeLists.txt
	$(SED) 's:$${CMAKE_SOURCE_DIR}/standalone/$(SOURCE_DIR)/external/include:$(STAGING_DIR)/usr/include/:g' $(@D)/CMakeLists.txt
	$(SED) 's:$${CMAKE_SOURCE_DIR}/standalone/$(SOURCE_DIR)/external/lib/:$(STAGING_DIR)/usr/lib/:g' $(@D)/CMakeLists.txt
	cd $(VPINBALL_DL_DIR) && unzip -o -x bass24-linux.zip -d bass24
	$(INSTALL) -D -m 0755 $(VPINBALL_DL_DIR)/bass24/libs/$(ARCH)/libbass.so $(STAGING_DIR)/usr/lib
	$(INSTALL) -D -m 0755 $(VPINBALL_DL_DIR)/bass24/libs/$(ARCH)/libbass.so $(TARGET_DIR)/usr/lib
endef

VPINBALL_CONF_OPTS += -DCMAKE_BUILD_TYPE=Release
VPINBALL_CONF_OPTS += -DBUILD_SHARED_LIBS=OFF
VPINBALL_CONF_OPTS += -DPOST_BUILD_COPY_EXT_LIBS=OFF

define VPINBALL_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/bin/vpinball
	# install binary
	$(INSTALL) -D -m 0755 $(@D)/buildroot-build/VPinballX_GL \
	    $(TARGET_DIR)/usr/bin/vpinball
	# copy folders
	cp -R $(@D)/buildroot-build/flexdmd $(TARGET_DIR)/usr/bin/vpinball/
	cp -R $(@D)/buildroot-build/assets $(TARGET_DIR)/usr/bin/vpinball/
	cp -R $(@D)/buildroot-build/scripts $(TARGET_DIR)/usr/bin/vpinball/
	cp -R $(@D)/buildroot-build/shaders-10.8.1 $(TARGET_DIR)/usr/bin/vpinball/
endef

VPINBALL_PRE_CONFIGURE_HOOKS += VPINBALL_CMAKE_HACKS

$(eval $(cmake-package))
