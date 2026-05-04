#pragma once

#include <pebble.h>

#include "types.h"

void radar_data_cache_set_item(radar_data_t* radar_data);

radar_data_t* radar_data_cache_get_item(
    size_t timestep_index,
    uint16_t width_km
);

size_t radar_data_cache_get_item_count(uint16_t width_km);

size_t radar_data_cache_get_max_item_count();

size_t radar_data_cache_get_total_item_count();
