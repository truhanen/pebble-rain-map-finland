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
    MapZoomLevel map_zoom_level
) {
    size_t map_zoom_level_timestep_index = 0;
    for (size_t i = 0; i < state.radar_data_cache_size; i++) {
        if (state.radar_data_cache[i]->zoom_level == map_zoom_level) {
            if (map_zoom_level_timestep_index == timestep_index) {
                return state.radar_data_cache[i];
            }
            map_zoom_level_timestep_index += 1;
        }
    }
    return NULL;
}

size_t radar_data_cache_get_zoom_level_item_count(MapZoomLevel map_zoom_level) {
    size_t zoom_level_item_count = 0;
    for (size_t i = 0; i < state.radar_data_cache_size; i++) {
        if (state.radar_data_cache[i] == NULL) {
            break;
        }
        if (state.radar_data_cache[i]->zoom_level == map_zoom_level) {
            zoom_level_item_count += 1;
        }
    }
    return zoom_level_item_count;
}
