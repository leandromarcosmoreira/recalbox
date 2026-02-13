################################################################################
#
# KRONOS
#
################################################################################

# Commit of 2023/12/29
LIBRETRO_KRONOS_VERSION = b847da58fd741bb7351e6da92316ae2fb1f95045
LIBRETRO_KRONOS_SITE = $(call github,FCare,Kronos,$(LIBRETRO_KRONOS_VERSION))
LIBRETRO_KRONOS_LICENSE = GPL-2.0

ifeq ($(BR2_PACKAGE_HAS_LIBGLES),y)
  	LIBRETRO_KRONOS_GL = GLESv2
else
  	LIBRETRO_KRONOS_GL = GL
endif

define LIBRETRO_KRONOS_BUILD_CMDS
	$(SED) "s|-O2|-O3|g" $(@D)/yabause/src/libretro/Makefile
	$(SED) "s|-lGL|-l$(LIBRETRO_KRONOS_GL)|g" $(@D)/yabause/src/libretro/Makefile
	mkdir -p $(@D)/build_retro
	cd $(@D)/build_retro
	$(MAKE) -C $(@D)/yabause/src/libretro generate-files
	CFLAGS="$(TARGET_CFLAGS) $(COMPILER_COMMONS_CFLAGS_SO)" \
		CXXFLAGS="$(TARGET_CXXFLAGS) $(COMPILER_COMMONS_CXXFLAGS_SO)" \
		LDFLAGS="$(TARGET_LDFLAGS) $(COMPILER_COMMONS_LDFLAGS_SO)" \
		$(MAKE) CXX="$(TARGET_CXX)" CC="$(TARGET_CC)" AR="$(TARGET_AR)" LD="$(TARGET_LD)" RANLIB="$(TARGET_RANLIB)" -C $(@D)/yabause/src/libretro -f Makefile platform="unix-$(RETROARCH_LIBRETRO_PLATFORM)" $(LIBRETRO_KRONOS_SUPP_OPT) \
		GIT_VERSION="-$(shell echo $(LIBRETRO_KRONOS_VERSION) | cut -c 1-8)"
endef

define LIBRETRO_KRONOS_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/yabause/src/libretro/kronos_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/kronos_libretro.so
	mkdir -p $(TARGET_DIR)/recalbox/share_upgrade/bios/saturn
endef

$(eval $(generic-package))
