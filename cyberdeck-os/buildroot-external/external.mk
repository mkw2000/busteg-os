define CYBERDECK_INSTALL_KERNEL_LOGO
	$(BR2_EXTERNAL_CYBERDECK_PATH)/board/cyberdeck/generate-kernel-logo.sh \
		$(@D)/drivers/video/logo/logo_linux_clut224.ppm
endef
LINUX_POST_PATCH_HOOKS += CYBERDECK_INSTALL_KERNEL_LOGO

include $(sort $(wildcard $(BR2_EXTERNAL_CYBERDECK_PATH)/package/*/*.mk))
