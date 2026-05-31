#!/bin/sh

set -e

# Remove any getty on tty1 so the Cyberdeck UI owns the display
if [ -e "$TARGET_DIR/etc/inittab" ]; then
	sed -i '/^tty1::.*getty/d' "$TARGET_DIR/etc/inittab"
fi

chmod +x "$TARGET_DIR/etc/init.d/S99cyberdeck"
mkdir -p "$TARGET_DIR/etc/cyberdeck"
