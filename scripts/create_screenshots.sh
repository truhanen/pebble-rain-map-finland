#!/usr/bin/env bash
set -euo pipefail

EMULATOR="${PEBBLE_EMULATOR:-emery}"
SS_DIR="screenshots"
SEQUENCE_PNG_DIR="$SS_DIR/sequence_png"
mkdir -p "$SEQUENCE_PNG_DIR"

ss_index=0
ss() {
    local filename
    filename="$(printf '%s/%02d_%s.png' "$SEQUENCE_PNG_DIR" "$ss_index" "$1")"
    pebble screenshot --emulator "$EMULATOR" --no-open "$filename"
    ss_index=$((ss_index + 1))
}

btn() {
    pebble emu-button --emulator "$EMULATOR" click "$1"
}

echo "Building and installing..."
EMULATOR="$EMULATOR" make install

echo "Waiting 10 seconds for app to start..."
sleep 10

ss "start"
btn up;     ss "up_1"
btn up;     ss "up_2"
btn up;     ss "up_3"
btn up;     ss "up_4"
btn up;     ss "up_5"
btn up;     ss "up_6";
ss "before_select"
btn select; ss "select_1"
btn select; ss "select_2"
btn select; ss "legend"
btn back
ss "before_down"
btn down;   ss "down_1"
btn down;   ss "down_2"
btn down;   ss "down_3"
btn down;   ss "down_4"
btn down;   ss "down_5"
btn down;   ss "down_6"
ss "before_back"
btn back; ss "back_1"
btn back; ss "back_2"
ss "end"

echo "Done. Screenshots saved to $SEQUENCE_PNG_DIR/"

echo "Copying/moving selected screenshots..."
mv "$SEQUENCE_PNG_DIR/10_legend.png"   "$SS_DIR/screenshot_02.png"
cp "$SEQUENCE_PNG_DIR/00_start.png"    "$SS_DIR/screenshot_03.png"
cp "$SEQUENCE_PNG_DIR/17_down_6.png"   "$SS_DIR/screenshot_04.png"
cp "$SEQUENCE_PNG_DIR/08_select_1.png" "$SS_DIR/screenshot_05.png"

echo "Creating GIF..."
magick -delay 50 -loop 0 ${SEQUENCE_PNG_DIR}/*.png -layers Optimize "$SS_DIR/screenshot_01.gif"
xattr -dr com.apple.quarantine "$SS_DIR/screenshot_01.gif"
echo "GIF saved to $SS_DIR/screenshot_01.gif"
