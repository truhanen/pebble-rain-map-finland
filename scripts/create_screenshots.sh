#!/usr/bin/env bash
set -euo pipefail

EMULATOR="${PEBBLE_EMULATOR:-emery}"
SS_DIR="screenshots"
PNG_DIR="$SS_DIR/png"
mkdir -p "$PNG_DIR"

ss_index=0
ss() {
    local filename
    filename="$(printf '%s/%02d_%s.png' "$PNG_DIR" "$ss_index" "$1")"
    pebble screenshot --emulator "$EMULATOR" --no-open "$filename"
    ss_index=$((ss_index + 1))
}

btn() {
    pebble emu-button --emulator "$EMULATOR" click "$1"
}

echo "Building and installing..."
pebble build
pebble install -vv --emulator "$EMULATOR"

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

echo "Done. Screenshots saved to $PNG_DIR/"

echo "Copying selected screenshots..."
cp "$PNG_DIR/00_start.png"    "$SS_DIR/screenshot_02.png"
cp "$PNG_DIR/08_select_1.png" "$SS_DIR/screenshot_03.png"
cp "$PNG_DIR/16_down_6.png"   "$SS_DIR/screenshot_04.png"

echo "Creating GIF..."
magick -delay 50 -loop 0 ${PNG_DIR}/*.png -layers Optimize "$SS_DIR/screenshot_01.gif"
echo "GIF saved to $SS_DIR/screenshot_01.gif"
