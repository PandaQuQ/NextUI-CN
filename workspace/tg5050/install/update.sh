#!/bin/sh

SDCARD_PATH=/mnt/SDCARD

# --------------------------------------
# migration code here
# --------------------------------------

# NextUI-CN: Airlock is not part of the distributed image. Remove it if a
# previous package unzipped it into Tools (fresh installs and updates).
rm -rf "$SDCARD_PATH/Tools/tg5040/Airlock.pak"
rm -rf "$SDCARD_PATH/Tools/"*/Airlock.pak
