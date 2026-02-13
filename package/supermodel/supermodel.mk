################################################################################
#
# Supermodel
#
################################################################################

# Commit of 2025/05/28
SUPERMODEL_VERSION = 487d642a808273f99611bcee9badeddc317849f0
SUPERMODEL_SITE = https://github.com/trzy/Supermodel.git
SUPERMODEL_SITE_METHOD = git
SUPERMODEL_LICENSE = GPL2
SUPERMODEL_LICENSE_FILES = Docs/LICENSE.txt
SUPERMODEL_DEPENDENCIES = zlib libpng libogg libvorbis sdl2_net sdl2 libglu arcade-dats

define SUPERMODEL_BUILD_CMDS
	$(call InstallArcadeFiles,supermodel,supermodel,$(SUPERMODEL_VERSION))
	cp $(@D)/Makefiles/Makefile.UNIX $(@D)/Makefile
	$(SED) "s|CC = gcc|CC = $(TARGET_CC) $(COMPILER_COMMONS_CFLAGS_NOLTO)|g" $(@D)/Makefile
	$(SED) "s|CXX = g++|CXX = $(TARGET_CXX) $(COMPILER_COMMONS_CXXFLAGS_NOLTO)|g" $(@D)/Makefile
	$(SED) "s|LD = gcc|LD = $(TARGET_CC) $(COMPILER_COMMONS_LDFLAGS_NOLTO)|g" $(@D)/Makefile
	$(SED) "s|sdl2-config|$(STAGING_DIR)/usr/bin/sdl2-config|g" $(@D)/Makefile
	$(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) -f Makefile \
	NET_BOARD=1 DEBUG=0 NEW_FRAME_TIMING=1 ENABLE_DEBUGGER=0
endef

define SUPERMODEL_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/bin/supermodel \
		$(TARGET_DIR)/usr/bin/supermodel
	$(INSTALL) -D -m 0644 $(@D)/Config/Games.xml \
		$(TARGET_DIR)/recalbox/share_init/system/configs/model3/Games.xml
	mkdir -p $(TARGET_DIR)/usr/share/supermodel/Assets
	$(INSTALL) -D -m 0644 $(@D)/Assets/* \
		$(TARGET_DIR)/usr/share/supermodel/Assets/
endef

define SUPERMODEL_LINE_ENDINGS_FIXUP
	# DOS2UNIX Supermodel.ini and Main.cpp - patch system does not support different line endings
	sed -i -E -e "s|\r$$||g" $(@D)/Src/OSD/SDL/Main.cpp
	sed -i -E -e "s|\r$$||g" $(@D)/Src/Inputs/Inputs.cpp
	sed -i -E -e "s|\r$$||g" $(@D)/Src/Graphics/New3D/R3DShaderTriangles.h
endef

# SUPERMODEL_PRE_PATCH_HOOKS += SUPERMODEL_LINE_ENDINGS_FIXUP

$(eval $(generic-package))
