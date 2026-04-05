#pragma once

#include <pebble.h>

#include "types.h"

void radar_data_cache_set_item(radar_data_t* radar_data);

radar_data_t* radar_data_cache_get_item(
    size_t timestep_index,
    MapZoomLevel map_zoom_level
);

size_t radar_data_cache_get_zoom_level_item_count(MapZoomLevel map_zoom_level);

size_t radar_data_cache_get_total_item_count();
