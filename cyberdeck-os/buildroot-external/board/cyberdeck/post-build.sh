#!/bin/sh

set -e

# Remove any virtual-console getty so the Cyberdeck UI owns the display.
if [ -e "$TARGET_DIR/etc/inittab" ]; then
	tmp_inittab="$TARGET_DIR/etc/inittab.$$"
	sed \
		-e '/^tty[0-9][0-9]*::.*getty/d' \
		-e 's|^::sysinit:/etc/init.d/rcS$|::sysinit:/etc/init.d/rcS >/dev/null 2>\&1|' \
		"$TARGET_DIR/etc/inittab" > "$tmp_inittab"
	cat "$tmp_inittab" > "$TARGET_DIR/etc/inittab"
	rm -f "$tmp_inittab"
fi

if [ -e "$TARGET_DIR/etc/cyberdeck/config.ini" ]; then
	tmp_config="$TARGET_DIR/etc/cyberdeck/config.ini.$$"
	sed 's/^fullscreen=.*/fullscreen=true/' "$TARGET_DIR/etc/cyberdeck/config.ini" > "$tmp_config"
	cat "$tmp_config" > "$TARGET_DIR/etc/cyberdeck/config.ini"
	rm -f "$tmp_config"
fi

chmod +x "$TARGET_DIR/etc/init.d/S00bootsplash"
chmod +x "$TARGET_DIR/etc/init.d/S99cyberdeck"
mkdir -p "$TARGET_DIR/etc/cyberdeck"
