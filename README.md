# Cyberdeck OS Core

Lightweight C17/SDL2 foundation for a custom Raspberry Pi cyberdeck appliance UI. The target is a Raspberry Pi Model B+ v1.2 with 512 MB RAM, so the code avoids desktop stacks and keeps the runtime small: one SDL2 window or framebuffer/KMS surface, a fixed 480x320 virtual framebuffer, simple module screens, and read-only hardware diagnostics.

This project intentionally does not implement RF attacks, replay, jamming, credential capture, unauthorized interception, exploitation, or attack automation. RFcat and RF-NANO screens are inventory and status views only.

## Architecture

```text
main
  |
  +-- core/app
      |
      +-- core/screen_manager   BOOT -> SPLASH -> HOME -> MODULE
      +-- core/module_manager   System, RFcat, RF-NANO, Touch Test, Settings
      +-- core/event_system     NAV_UP/DOWN/LEFT/RIGHT/SELECT/BACK/POINTER
      +-- hal/display           SDL2/SDL2_ttf hidden behind draw calls
      +-- hal/input             keyboard/mouse/touch mapped to nav events
      +-- hal/serial            ttyUSB/ttyACM enumeration
      +-- hal/system            /proc, /sys, filesystem status
      +-- ui                    renderer, menu, widgets, theme helpers
```

All application screens render to a 480x320 virtual framebuffer. The display HAL scales that target to the actual window or future touchscreen resolution, which keeps layout work stable when moving from HDMI development to a 5.6 inch panel.

## Project Layout

```text
Makefile
README.md
config.ini
scripts/
  detect_usb.sh
  detect_serial.sh
src/
  main.c
  core/
  hal/
  ui/
  modules/
  screens/
```

## Module System

Each module is a `Module` struct with function pointers:

```c
init()
update()
render()
handle_event()
shutdown()
```

The home menu is generated from the module registry. Adding a new module requires creating its source/header pair and registering it in `App_Init`; the menu rendering code does not need per-module changes.

## Controls

Keyboard input is mapped to navigation events:

```text
Up/W          NAV_UP
Down/S        NAV_DOWN
Left/A        NAV_LEFT
Right/D       NAV_RIGHT
Enter/Space   NAV_SELECT
Esc/Backspace NAV_BACK
Q             NAV_QUIT
Mouse/touch   NAV_POINTER
```

Future GPIO buttons and a rotary encoder should feed the same `EventSystem_Push()` API so the rest of the app remains device-agnostic.

## Raspberry Pi OS Lite Setup

Install the compiler and SDL development packages:

```sh
sudo apt update
sudo apt install -y build-essential pkg-config libsdl2-dev libsdl2-ttf-dev fonts-dejavu-core
```

Build and run:

```sh
make
make run
```

Clean:

```sh
make clean
```

The display layer searches for `assets/font.ttf` first, then common DejaVu paths, then Windows console fonts for local development.

## Configuration

Edit `config.ini`:

```ini
[display]
fullscreen=false
window_width=800
window_height=480
font_size=16

[serial]
baud_rate=115200

[theme]
background=0,0,0
primary=0,255,90
secondary=92,106,100
accent=255,255,255
```

## Buildroot Guidance

This repository contains a Buildroot external tree, not Buildroot itself. Use it
with a Buildroot checkout:

```sh
cd ~/Desktop
git clone --depth 1 --branch 2025.05 https://gitlab.com/buildroot.org/buildroot.git buildroot
cd ~/Desktop/busteg-os
make pi-defconfig
make pi-image
```

The expected flashable image is:

```text
../buildroot-output/images/sdcard.img
```

Flash it to the SD card device, replacing `/dev/sdX` with the real whole-device
path from `lsblk`:

```sh
sudo dd if=../buildroot-output/images/sdcard.img of=/dev/sdX bs=4M conv=fsync status=progress
```

The defconfig is based on Buildroot's Raspberry Pi 1/B/B+ image flow and enables
the Pi firmware, kernel, root filesystem, and `genimage` steps needed to produce
`sdcard.img`. The repository Makefile also uses a clean build `PATH`, and the
external tree carries host-package compatibility patches for newer rolling
distributions.

For Buildroot package configuration, enable:

```text
Target packages -> Graphic libraries and applications -> SDL2
Target packages -> Graphic libraries and applications -> SDL2_ttf
Target packages -> Fonts, cursors, icons, sounds and themes -> dejavu
Toolchain -> C library support suitable for SDL2
Kernel -> DRM/KMS or framebuffer support for the chosen display
```

For Raspberry Pi Model B+ v1.2, start with the matching ARM1176 target and Raspberry Pi firmware packages. Prefer SDL2 KMS/DRM or framebuffer output and boot directly into this binary from an init script, BusyBox init entry, or a small supervisor.

Example production launch script:

```sh
#!/bin/sh
export SDL_VIDEODRIVER=kmsdrm
exec /usr/bin/cyberdeck-os
```

If the display stack is framebuffer-only, use the SDL2 backend available in your Buildroot configuration and test early on the actual panel.

## Touchscreen Integration

Keep touch support inside `hal/input.c`. Convert raw touchscreen coordinates into the 480x320 virtual coordinate space, then emit `NAV_POINTER` events. If physical buttons or rotary inputs are added, map them to `NAV_UP`, `NAV_DOWN`, `NAV_SELECT`, and `NAV_BACK`; screens and modules should not read GPIO directly.

## Diagnostics Scripts

The scripts are safe read-only helpers:

```sh
./scripts/detect_usb.sh
./scripts/detect_serial.sh
```

`detect_usb.sh` reads `/sys/kernel/debug/usb/devices` when available or falls back to `lsusb`. `detect_serial.sh` lists `ttyUSB` and `ttyACM` devices.
