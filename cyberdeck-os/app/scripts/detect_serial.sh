#!/bin/sh
set -eu

found=0
for dev in /dev/ttyUSB* /dev/ttyACM*; do
  [ -e "$dev" ] || continue
  echo "$dev"
  found=1
done

if [ "$found" -eq 0 ]; then
  echo "No ttyUSB or ttyACM devices detected"
fi
