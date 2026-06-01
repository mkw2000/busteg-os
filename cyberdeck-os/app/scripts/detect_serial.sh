#!/bin/sh
set -eu

found=0
for dev in \
  /dev/ttyUSB* \
  /dev/ttyACM* \
  /dev/tty.usbserial* \
  /dev/cu.usbserial* \
  /dev/tty.usbmodem* \
  /dev/cu.usbmodem* \
  /dev/tty.wchusbserial* \
  /dev/cu.wchusbserial*; do
  [ -e "$dev" ] || continue
  echo "$dev"
  found=1
done

if [ "$found" -eq 0 ]; then
  echo "No USB serial tty devices detected"
fi
