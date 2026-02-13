################################################################################
#
# GEOLITH
#
################################################################################

# Commit version of 05/14/2024
LIBRETRO_GEOLITH_VERSION = 4d9692a48bbb375556527bd1aade29b1de9e497e
LIBRETRO_GEOLITH_SITE = $(call github,libretro,geolith-libretro,$(LIBRETRO_GEOLITH_VERSION))
LIBRETRO_GEOLITH_LICENSE = BSD 3
LIBRETRO_GEOLITH_LICENSE_FILES = LICENSE.txt

ifeq ($(BR2_PACKAGE_RECALBOX_TARGET_RPI4_64)$(BR2_PACKAGE_RECALBOX_TARGET_RPI5_64),y)
LIBRETRO_GEOLITH_PLATFORM = unix
else
LIBRETRO_GEOLITH_PLATFORM = $(RETROARCH_LIBRETRO_BOARD)
endif

define LIBRETRO_GEOLITH_BUILD_CMDS
	$(SED) "s|-O2|-O3|g" $(@D)/libretro/Makefile
	CFLAGS="$(TARGET_CFLAGS) $(COMPILER_COMMONS_CFLAGS_SO)" \
		CXXFLAGS="$(TARGET_CXXFLAGS) $(COMPILER_COMMONS_CXXFLAGS_SO)" \
		LDFLAGS="$(TARGET_LDFLAGS) $(COMPILER_COMMONS_LDFLAGS_SO)" \
		$(MAKE) CXX="$(TARGET_CXX)" CC="$(TARGET_CC)" -C $(@D)/libretro -f Makefile platform="$(LIBRETRO_GEOLITH_PLATFORM)"
endef

define LIBRETRO_GEOLITH_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/libretro/geolith_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/geolith_libretro.so
	mkdir -p $(TARGET_DIR)/recalbox/share_upgrade/bios/geolith
endef

$(eval $(generic-package))
