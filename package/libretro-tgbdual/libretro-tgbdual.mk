################################################################################
#
# TGBDUAL
#
################################################################################

# Commit of 2022/08/06
LIBRETRO_TGBDUAL_VERSION = a6f3018e6a23030afc1873845ee54d4b2d8ec9d3
LIBRETRO_TGBDUAL_SITE = $(call github,libretro,tgbdual-libretro,$(LIBRETRO_TGBDUAL_VERSION))
LIBRETRO_TGBDUAL_LICENSE = GPL-2.0

define LIBRETRO_TGBDUAL_BUILD_CMDS
	$(SED) "s|-O2|-O3|g" $(@D)/Makefile
	CFLAGS="$(TARGET_CFLAGS) $(COMPILER_COMMONS_CFLAGS_SO)" \
		CXXFLAGS="$(TARGET_CXXFLAGS) $(COMPILER_COMMONS_CXXFLAGS_SO)" \
		LDFLAGS="$(TARGET_LDFLAGS) $(COMPILER_COMMONS_LDFLAGS_SO)" \
		$(MAKE) CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" -C $(@D)
endef

define LIBRETRO_TGBDUAL_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/tgbdual_libretro.so \
		$(TARGET_DIR)/usr/lib/libretro/tgbdual_libretro.so
endef

$(eval $(generic-package))
