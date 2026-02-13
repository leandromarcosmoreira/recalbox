################################################################################
#
# Recalbox themes for EmulationStation : https://gitlab.com/recalbox/recalbox-themes
#
################################################################################

ifneq ($(GITLAB_TOKEN_THEMES),)
RECALBOX_THEMES_VERSION = febce03c483cca4fdcd873938e974e0a52930418
RECALBOX_THEMES_SITE = https://gitlab-ci-token:$(GITLAB_TOKEN_THEMES)@gitlab.com/recalbox/recalbox-themes-prime
else
RECALBOX_THEMES_VERSION = 284af2f1a598ba5ff3c9fb32ab0509f6bbd6291f
RECALBOX_THEMES_SITE = https://gitlab.com/recalbox/recalbox-themes
endif
RECALBOX_THEMES_SITE_METHOD = git
RECALBOX_THEMES_LICENSE = CC-BY-NC-ND-4.0

define RECALBOX_THEMES_DOWNLOAD_NEXT_PIXEL
	@echo "Downloading Next Pixel theme..." && \
	wget -q -O /tmp/es-theme-next-pixel-v2-16x9-1080p.zip \
	https://github.com/SamYStudiO/es-theme-next-pixel/releases/download/v2.1/es-theme-next-pixel-v2-16x9-1080p.zip && \
	unzip -q -o /tmp/es-theme-next-pixel-v2-16x9-1080p.zip -d /tmp/ && \
	mv /tmp/16x9-1080p $(@D)/themes/es-theme-next-pixel-v2-16x9-1080p && \
	rm -f /tmp/es-theme-next-pixel-v2-16x9-1080p.zip
endef

define RECALBOX_THEMES_COMMON
	mkdir -p $(TARGET_DIR)/recalbox/share_init/system/.emulationstation/themes/
	cp -r $(@D)/themes/recalbox-next \
		$(TARGET_DIR)/recalbox/share_init/system/.emulationstation/themes/
	cp -r $(@D)/themes/recalbox-next-v9 \
		$(TARGET_DIR)/recalbox/share_init/system/.emulationstation/themes/
	cp -r $(@D)/themes/recalbox-goa2 \
		$(TARGET_DIR)/recalbox/share_init/system/.emulationstation/themes/
	cp -r $(@D)/themes/recalbox-240p \
		$(TARGET_DIR)/recalbox/share_init/system/.emulationstation/themes/
	cp -r $(@D)/themes/es-theme-next-pixel-v2-16x9-1080p \
		$(TARGET_DIR)/recalbox/share_init/system/.emulationstation/themes/
endef

define RECALBOX_THEMES_GPICASE2W
	cp -r $(@D)/themes/recalbox-goa-gpicase \
		$(TARGET_DIR)/recalbox/share_init/system/.emulationstation/themes/
endef

RECALBOX_THEMES_POST_DOWNLOAD_HOOKS += RECALBOX_THEMES_DOWNLOAD_NEXT_PIXEL
RECALBOX_THEMES_INSTALL_TARGET_CMDS = $(RECALBOX_THEMES_COMMON)
ifeq ($(BR2_PACKAGE_RECALBOX_TARGET_RPIZERO2),y)
RECALBOX_THEMES_INSTALL_TARGET_CMDS += && $(RECALBOX_THEMES_GPICASE2W)
endif

$(eval $(generic-package))
