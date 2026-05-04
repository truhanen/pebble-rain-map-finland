#include "radar_data_cache.h"

#include "logging.h"
#include "state.h"

void radar_data_cache_set_item(radar_data_t* radar_data) {
    size_t new_cache_size = state.radar_data_cache_size + 1;
    if (new_cache_size == 1) {
        state.radar_data_cache = (radar_data_t**) malloc(sizeof(radar_data_t*));
    } else {
        radar_data_t** new_cache =
            realloc(state.radar_data_cache, new_cache_size * sizeof(radar_data_t*));
        if (new_cache == NULL) {
            APP_LOG(APP_LOG_LEVEL_ERROR, "realloc failed in radar_data_cache_set_item");
            return;
        }
        state.radar_data_cache = new_cache;
    }
    size_t new_item_index = new_cache_size - 1;
    state.radar_data_cache[new_item_index] = radar_data;
    state.radar_data_cache_size = new_cache_size;
}

radar_data_t* radar_data_cache_get_item(
    size_t timestep_index,
    uint16_t width_km
) {
    size_t item_timestep_index = 0;
    for (size_t i = 0; i < state.radar_data_cache_size; i++) {
        if (state.radar_data_cache[i]->width_km == width_km) {
            if (item_timestep_index == timestep_index) {
                return state.radar_data_cache[i];
            }
            item_timestep_index += 1;
        }
    }
    return NULL;
}

size_t radar_data_cache_get_zoom_level_item_count(uint16_t width_km) {
    size_t zoom_level_item_count = 0;
    for (size_t i = 0; i < state.radar_data_cache_size; i++) {
        if (state.radar_data_cache[i] == NULL) {
            break;
        }
        if (state.radar_data_cache[i]->width_km == width_km) {
            zoom_level_item_count += 1;
        }
    }
    return zoom_level_item_count;
}

size_t radar_data_cache_get_max_item_count() {
    size_t far_count =
        radar_data_cache_get_zoom_level_item_count((uint16_t) MAP_ZOOM_LEVEL_FAR);
    size_t close_count =
        radar_data_cache_get_zoom_level_item_count((uint16_t) MAP_ZOOM_LEVEL_CLOSE);
    return far_count > close_count ? far_count : close_count;
}
