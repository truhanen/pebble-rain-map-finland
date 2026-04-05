#pragma once

#include <pebble.h>

#include "types.h"

// This must be higher than the format "%Y%m%dT%H%M%S%z" + zero terminator
#define TIMESTAMP_STR_LENGTH 64

tm cstring_to_tm(const char* string);

void tm_to_cstring(const tm* timestamp, char* out, size_t out_size);

GPoint location_to_gpoint(
    const Layer* layer,
    const location_t* location,
    MapZoomLevel zoom_level
);

coordinate_bounds_t get_magnified_coordinate_bounds(
    coordinate_bounds_t bounds,
    uint8_t magnification_factor
);
