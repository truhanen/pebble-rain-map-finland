#pragma once

#include <pebble.h>

#include "./types.h"

radar_data_t* radar_data_init(
    time_t timestamp,
    uint16_t width_km,
    uint16_t width_px,
    uint16_t height_px
);

void radar_data_deinit(const radar_data_t* radar_data);

void radar_data_pack_point_value(
    const radar_data_t* radar_data,
    uint8_t point_value,
    size_t point_index
);

RainLevel radar_data_get_point_rain_level(
    const radar_data_t* radar_data,
    size_t y,
    size_t x
);
