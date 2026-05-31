#!/bin/sh

set -e

if [ -e "$TARGET_DIR/etc/inittab" ]; then
	grep -qE '^tty1::' "$TARGET_DIR/etc/inittab" || \
		sed -i '/GENERIC_SERIAL/a\
tty1::respawn:/sbin/getty -L  tty1 0 vt100 # HDMI console' "$TARGET_DIR/etc/inittab"
fi

chmod +x "$TARGET_DIR/etc/init.d/S99cyberdeck"
mkdir -p "$TARGET_DIR/etc/cyberdeck"
