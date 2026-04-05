#include "state.h"

#include "radar_data.h"
#include "types.h"

state_t state;

void state_init() {
    state.radar_data_cache = NULL;
    state.radar_data_cache_size = 0;
    coordinate_bounds_t initial_coordinate_bounds = {
        LATITUDE_NA, LATITUDE_NA, LONGITUDE_NA, LONGITUDE_NA
    };
    state.coordinate_bounds_far = initial_coordinate_bounds;
    state.coordinate_bounds_close = initial_coordinate_bounds;
}

static void radar_data_cache_deinit() {
    for (size_t i = 0; i < state.radar_data_cache_size; i++) {
        if (state.radar_data_cache[i] != NULL) {
            radar_data_deinit(state.radar_data_cache[i]);
            free(state.radar_data_cache[i]);
            state.radar_data_cache[i] = NULL;
        }
    }
    free(state.radar_data_cache);
}

void state_deinit() {
    radar_data_cache_deinit();
}
