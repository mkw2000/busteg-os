BUILDROOT_DIR ?= ../buildroot
BUILDROOT_OUTPUT ?= ../buildroot-output
BR2_EXTERNAL := $(CURDIR)/cyberdeck-os/buildroot-external
BUILDROOT_OUTPUT_ABS := $(abspath $(BUILDROOT_OUTPUT))
BUILDROOT_ENV := env -i HOME="$(HOME)" USER="$(USER)" LOGNAME="$(LOGNAME)" TERM="$(TERM)" SHELL="$(SHELL)" PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"

.PHONY: help app app-run app-clean pi-defconfig pi-image pi-host-m4-clean pi-rpi-userland-clean pi-rpi-firmware-clean pi-linux-clean pi-app-clean pi-boot-clean pi-boot-image

help:
	@echo "Targets:"
	@echo "  app              Build the local desktop app"
	@echo "  app-run          Build and run the local desktop app"
	@echo "  app-clean        Clean the local desktop app"
	@echo "  pi-defconfig     Configure Buildroot for Raspberry Pi B+"
	@echo "  pi-image         Build the Raspberry Pi SD card image"
	@echo "  pi-host-m4-clean Clean Buildroot's failed host-m4 build"
	@echo "  pi-rpi-userland-clean Clean Buildroot's failed rpi-userland build"
	@echo "  pi-rpi-firmware-clean Clean Raspberry Pi firmware/cmdline build artifacts"
	@echo "  pi-linux-clean   Clean Linux build artifacts"
	@echo "  pi-app-clean      Clean Buildroot's cyberdeck-os package build"
	@echo "  pi-boot-clean    Clean all boot-customization-sensitive packages"
	@echo "  pi-boot-image    Rebuild boot-sensitive packages and image"
	@echo
	@echo "Variables:"
	@echo "  BUILDROOT_DIR=$(BUILDROOT_DIR)"
	@echo "  BUILDROOT_OUTPUT=$(BUILDROOT_OUTPUT)"

app:
	$(MAKE) -C cyberdeck-os/app

app-run:
	$(MAKE) -C cyberdeck-os/app run

app-clean:
	$(MAKE) -C cyberdeck-os/app clean

pi-defconfig:
	$(BUILDROOT_ENV) $(MAKE) -C "$(BUILDROOT_DIR)" \
		O="$(BUILDROOT_OUTPUT_ABS)" \
		BR2_EXTERNAL="$(BR2_EXTERNAL)" \
		cyberdeck_rpi_b_plus_defconfig

pi-image:
	$(BUILDROOT_ENV) $(MAKE) -C "$(BUILDROOT_DIR)" \
		O="$(BUILDROOT_OUTPUT_ABS)"

pi-host-m4-clean:
	$(BUILDROOT_ENV) $(MAKE) -C "$(BUILDROOT_DIR)" \
		O="$(BUILDROOT_OUTPUT_ABS)" \
		host-m4-dirclean

pi-rpi-userland-clean:
	$(BUILDROOT_ENV) $(MAKE) -C "$(BUILDROOT_DIR)" \
		O="$(BUILDROOT_OUTPUT_ABS)" \
		rpi-userland-dirclean

pi-rpi-firmware-clean:
	$(BUILDROOT_ENV) $(MAKE) -C "$(BUILDROOT_DIR)" \
		O="$(BUILDROOT_OUTPUT_ABS)" \
		rpi-firmware-dirclean

pi-linux-clean:
	$(BUILDROOT_ENV) $(MAKE) -C "$(BUILDROOT_DIR)" \
		O="$(BUILDROOT_OUTPUT_ABS)" \
		linux-dirclean

pi-app-clean:
	$(BUILDROOT_ENV) $(MAKE) -C "$(BUILDROOT_DIR)" \
		O="$(BUILDROOT_OUTPUT_ABS)" \
		cyberdeck-os-dirclean

pi-boot-clean: pi-rpi-firmware-clean pi-linux-clean pi-app-clean

pi-boot-image: pi-boot-clean pi-image
