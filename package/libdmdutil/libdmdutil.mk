################################################################################
#
# libdmdutil
#
################################################################################

# Version: Commits on Apr 5, 2024
LIBDMDUTIL_VERSION = f54f084e995782948d30554391ea57a02010d3db
LIBDMDUTIL_SITE = $(call github,vpinball,libdmdutil,$(LIBDMDUTIL_VERSION))
LIBDMDUTIL_LICENSE = BSD-3-Clause
LIBDMDUTIL_LICENSE_FILES = LICENSE
LIBDMDUTIL_DEPENDENCIES = libserialport sockpp cargs libzedmd libserum libpupdmd
LIBDMDUTIL_SUPPORTS_IN_SOURCE_BUILD = NO

LIBDMDUTIL_CONF_OPTS += -DCMAKE_BUILD_TYPE=Release
LIBDMDUTIL_CONF_OPTS += -DBUILD_STATIC=OFF
LIBDMDUTIL_CONF_OPTS += -DPLATFORM=linux
LIBDMDUTIL_CONF_OPTS += -DARCH=$(BUILD_ARCH)
LIBDMDUTIL_CONF_OPTS += -DPOST_BUILD_COPY_EXT_LIBS=OFF

ifeq ($(BR2_PACKAGE_RECALBOX_TARGET_X86_64),y)
    BUILD_ARCH = x64
endif

ifeq ($(BR2_PACKAGE_RECALBOX_TARGET_RPI5_64),y)
    BUILD_ARCH = aarch64
endif

# Install to staging to build Visual Pinball Standalone
LIBDMDUTIL_INSTALL_STAGING = YES

define LIBDMDUTIL_INSTALL_SERVER
	$(INSTALL) -D -m 0755 $(LIBDMDUTIL_BUILDDIR)/dmdserver $(TARGET_DIR)/usr/bin/dmdserver
endef

LIBDMDUTIL_POST_INSTALL_TARGET_HOOKS += LIBDMDUTIL_INSTALL_SERVER

$(eval $(cmake-package))
