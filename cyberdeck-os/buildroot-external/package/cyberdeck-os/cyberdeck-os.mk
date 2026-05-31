################################################################################
#
# cyberdeck-os
#
################################################################################

CYBERDECK_OS_VERSION = 0.1.0
CYBERDECK_OS_SITE = $(BR2_EXTERNAL_CYBERDECK_PATH)/../app
CYBERDECK_OS_SITE_METHOD = local
CYBERDECK_OS_DEPENDENCIES = sdl2 sdl2_ttf dejavu

define CYBERDECK_OS_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) clean
	$(TARGET_MAKE_ENV) \
		CFLAGS="$(TARGET_CFLAGS) -std=c17 -Wall -Wextra -Wpedantic -O2" \
		$(MAKE) \
		CC="$(TARGET_CC)" \
		PKG_CONFIG="$(PKG_CONFIG_HOST_BINARY) --define-prefix" \
		LDLIBS="`$(PKG_CONFIG_HOST_BINARY) --libs sdl2 SDL2_ttf`" \
		-C $(@D)
	$(TARGET_CC) $(TARGET_CFLAGS) -std=c17 -Wall -Wextra -Wpedantic -O2 \
		$(@D)/tools/fb_splash.c -o $(@D)/build/fb-splash
endef

define CYBERDECK_OS_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/build/cyberdeck-os \
		$(TARGET_DIR)/usr/bin/cyberdeck-os

	$(INSTALL) -D -m 0644 $(@D)/config.ini \
		$(TARGET_DIR)/etc/cyberdeck/config.ini

	$(INSTALL) -D -m 0644 $(BR2_EXTERNAL_CYBERDECK_PATH)/board/cyberdeck/assets/bootsplash.ppm \
		$(TARGET_DIR)/etc/cyberdeck/bootsplash.ppm

	$(INSTALL) -D -m 0755 $(@D)/build/fb-splash \
		$(TARGET_DIR)/usr/bin/fb-splash

	$(INSTALL) -D -m 0755 $(@D)/scripts/detect_usb.sh \
		$(TARGET_DIR)/usr/bin/detect_usb.sh

	$(INSTALL) -D -m 0755 $(@D)/scripts/detect_serial.sh \
		$(TARGET_DIR)/usr/bin/detect_serial.sh
endef

$(eval $(generic-package))
