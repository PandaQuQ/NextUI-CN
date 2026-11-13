#!/bin/sh

SDCARD_PATH="/mnt/SDCARD"
REAL_SDCARD_PATH="/mnt/sdcard"
SYSTEM_PATH="$SDCARD_PATH/.system/h700"
TF1_DMENU="/mnt/mmc/dmenu.bin"
TF2_DMENU="$SYSTEM_PATH/dat/dmenu.bin"

if ! mountpoint -q "$SDCARD_PATH" && [ ! -L "$SDCARD_PATH" ]; then
	if [ -e "$SDCARD_PATH" ]; then
		mount --bind "$REAL_SDCARD_PATH" "$SDCARD_PATH" 2>/dev/null || true
	else
		ln -s "$REAL_SDCARD_PATH" "$SDCARD_PATH" 2>/dev/null || true
	fi
fi

mkdir -p "$SYSTEM_PATH/dat"

if [ -f "$TF2_DMENU" ] && mountpoint -q /mnt/mmc; then
	if ! cmp -s "$TF2_DMENU" "$TF1_DMENU" 2>/dev/null; then
		cp "$TF2_DMENU" "$TF1_DMENU" 2>/dev/null || true
		sync
	fi
fi


# NextUI-CN: Airlock is not part of the distributed image. Remove it if a
# previous package unzipped it into Tools (fresh installs and updates).
rm -rf "$SDCARD_PATH/Tools/tg5040/Airlock.pak"
rm -rf "$SDCARD_PATH/Tools/"*/Airlock.pak

exit 0
