#!/bin/sh
set -eu

if [ -r /sys/kernel/debug/usb/devices ]; then
  awk '/^P:|^S:  Product|^S:  Manufacturer/ { print }' /sys/kernel/debug/usb/devices
elif command -v lsusb >/dev/null 2>&1; then
  lsusb
else
  echo "USB inventory unavailable: mount debugfs or install lsusb"
  exit 1
fi
