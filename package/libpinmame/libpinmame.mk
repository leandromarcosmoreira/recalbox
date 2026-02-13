################################################################################
#
# libpinmame
#
################################################################################

# Version: Commits on Mar 23, 2024
LIBPINMAME_VERSION = a23b70acaedd5ab3ffce45d251a3802c7fe77906
LIBPINMAME_SITE = $(call github,vpinball,pinmame,$(LIBPINMAME_VERSION))
LIBPINMAME_LICENSE = BSD-3-Clause
LIBPINMAME_LICENSE_FILES = LICENSE
LIBPINMAME_DEPENDENCIES = zlib
LIBPINMAME_SUPPORTS_IN_SOURCE_BUILD = NO

LIBPINMAME_CONF_OPTS += -DPLATFORM=linux
ifeq ($(BR2_PACKAGE_RECALBOX_TARGET_RPI5_64),y)
LIBPINMAME_CONF_OPTS += -DARCH=arm64
endif

LIBPINMAME_CONF_OPTS += -DCMAKE_BUILD_TYPE=Release

define LIBPINMAME_RENAME_CMAKE
	cp $(@D)/cmake/libpinmame/CMakeLists.txt $(@D)/CMakeLists.txt
	rm $(@D)/makefile
endef

define LIBPINMAME_INSTALL_TARGET_CMDS
	# staging files
	$(INSTALL) -D -m 0755 $(@D)/buildroot-build/libpinmame.so.3.7.0 \
		$(STAGING_DIR)/usr/lib
	ln -sf $(STAGING_DIR)/usr/lib/libpinmame.so.3.7.0 $(STAGING_DIR)/usr/lib/libpinmame.so
	cp $(@D)/src/libpinmame/libpinmame.h $(STAGING_DIR)/usr/include
	cp $(@D)/src/libpinmame/pinmamedef.h $(STAGING_DIR)/usr/include
	# copy to target
	$(INSTALL) -D -m 0755 $(@D)/buildroot-build/libpinmame.so.3.7.0 \
		$(TARGET_DIR)/usr/lib
	ln -sf /usr/lib/libpinmame.so.3.7.0 $(TARGET_DIR)/usr/lib/libpinmame.so
endef

LIBPINMAME_PRE_CONFIGURE_HOOKS += LIBPINMAME_RENAME_CMAKE

$(eval $(cmake-package))
