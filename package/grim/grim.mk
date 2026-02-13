################################################################################
#
# grim
#
################################################################################

GRIM_VERSION = 7ba46364ab95141c79e0e18093aa66597256182c
GRIM_SITE = https://gitlab.freedesktop.org/emersion/grim/-/archive/$(GRIM_VERSION)
GRIM_LICENSE = MIT
GRIM_LICENSE_FILES = LICENSE

GRIM_DEPENDENCIES = wayland libpng

GRIM_CONF_OPTS = \
	-Dman-pages=disabled \
	-Djpeg=disabled

$(eval $(meson-package))
