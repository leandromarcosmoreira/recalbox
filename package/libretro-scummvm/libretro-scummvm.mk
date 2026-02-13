################################################################################
#
# LIBRETRO-SCUMMVM
#
################################################################################

# Release version 2.10.0 - Commit of 2025/04/05
LIBRETRO_SCUMMVM_VERSION = 9d31b31c179fd4a43f7cfc383a3435a9070c6aa8
LIBRETRO_SCUMMVM_SITE = $(call github,libretro,scummvm,$(LIBRETRO_SCUMMVM_VERSION))
LIBRETRO_SCUMMVM_LICENSE = GPL2
LIBRETRO_SCUMMVM_LICENSE_FILES = COPYING
LIBRETRO_SCUMMVM_DEPENDENCIES = zlib jpeg-turbo libmpeg2 libogg libvorbis flac libmad libpng libtheora faad2 fluidsynth freetype libfribidi

ifeq ($(BR2_PACKAGE_RECALBOX_TARGET_RPI4_64),y)
LIBRETRO_SCUMMVM_PLATFORM=rpi4_64
LIBRETRO_SCUMMVM_OPTS += FORCE_OPENGLES2=1
else ifeq ($(BR2_PACKAGE_RECALBOX_TARGET_RPI5_64),y)
LIBRETRO_SCUMMVM_PLATFORM=rpi5_64
LIBRETRO_SCUMMVM_OPTS += FORCE_OPENGLES2=1
else ifeq ($(BR2_PACKAGE_RECALBOX_TARGET_RPI3)$(BR2_PACKAGE_RECALBOX_TARGET_RPIZERO2),y)
LIBRETRO_SCUMMVM_PLATFORM=rpi3
else ifeq ($(BR2_PACKAGE_RECALBOX_TARGET_ODROIDGO2),y)
LIBRETRO_SCUMMVM_PLATFORM=odroidgo2
else ifeq ($(BR2_PACKAGE_RECALBOX_TARGET_RG353X),y)
LIBRETRO_SCUMMVM_PLATFORM=rg353x
else
LIBRETRO_SCUMMVM_PLATFORM = $(RETROARCH_LIBRETRO_PLATFORM)
endif

ifeq ($(BR2_ARCH_IS_64),y)
LIBRETRO_SCUMMVM_OPTS += BUILD_64BIT=1
endif

LIBRETRO_SCUMMVM_OPTS += USE_MT32EMU=1 \
			 USE_SYSTEM_fluidsynth=1 \
			 USE_SYSTEM_FLAC=1 \
			 USE_SYSTEM_z=1 \
			 USE_SYSTEM_mad=1 \
			 USE_SYSTEM_faad=1 \
			 USE_SYSTEM_png=1 \
			 USE_SYSTEM_jpeg=1 \
			 USE_SYSTEM_theora=1 \
			 USE_SYSTEM_freetype=1 \
			 USE_SYSTEM_fribidi=1

define LIBRETRO_SCUMMVM_BUILD_CMDS
	$(SED) "s|-O3|-O2|g" $(@D)/backends/platform/libretro/Makefile
	CFLAGS="$(TARGET_CFLAGS) $(LIBRETRO_SCUMMVM_CFLAGS_SO)" \
		CXXFLAGS="$(TARGET_CXXFLAGS) $(COMPILER_COMMONS_CXXFLAGS_SO)" \
		LDFLAGS="$(TARGET_LDFLAGS) $(COMPILER_COMMONS_LDFLAGS_SO)" \
		$(MAKE) CXX="$(TARGET_CXX)" CC="$(TARGET_CC)" -C $(@D)/backends/platform/libretro -f Makefile platform="$(LIBRETRO_SCUMMVM_PLATFORM)" $(LIBRETRO_SCUMMVM_OPTS)
		$(MAKE) datafiles CXX="$(TARGET_CXX)" CC="$(TARGET_CC)" -C $(@D)/backends/platform/libretro -f Makefile platform="$(LIBRETRO_SCUMMVM_PLATFORM)" $(LIBRETRO_SCUMMVM_OPTS)
endef

define LIBRETRO_SCUMMVM_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/backends/platform/libretro/scummvm_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/scummvm_libretro.so
endef

define LIBRETRO_SCUMMVM_INSTALL_DATAFILES
	mkdir -p $(TARGET_DIR)/recalbox/share_init/bios
	mkdir -p $(TARGET_DIR)/recalbox/share_upgrade/bios
	unzip -o $(@D)/backends/platform/libretro/scummvm.zip \
		-d $(TARGET_DIR)/recalbox/share_init/bios \
		scummvm/extra/*
	cp -r $(TARGET_DIR)/recalbox/share_init/bios/scummvm/extra/* $(TARGET_DIR)/recalbox/share_init/bios/scummvm/
	rm -Rf $(TARGET_DIR)/recalbox/share_init/bios/scummvm/extra
	unzip -o $(@D)/backends/platform/libretro/scummvm.zip \
		-d $(TARGET_DIR)/recalbox/share_upgrade/bios \
		scummvm/extra/*
	cp -r $(TARGET_DIR)/recalbox/share_upgrade/bios/scummvm/extra/* $(TARGET_DIR)/recalbox/share_init/bios/scummvm/
	rm -Rf $(TARGET_DIR)/recalbox/share_upgrade/bios/scummvm/extra
endef

define LIBRETRO_SCUMMVM_INSTALL_THEMES
	mkdir -p $(TARGET_DIR)/usr/share/libretro-scummvm/theme
	unzip -j -o $(@D)/backends/platform/libretro/scummvm.zip \
		-d $(TARGET_DIR)/usr/share/libretro-scummvm/theme \
		scummvm/theme/*

endef

LIBRETRO_SCUMMVM_POST_INSTALL_TARGET_HOOKS += LIBRETRO_SCUMMVM_INSTALL_DATAFILES
LIBRETRO_SCUMMVM_POST_INSTALL_TARGET_HOOKS += LIBRETRO_SCUMMVM_INSTALL_THEMES

$(eval $(generic-package))
