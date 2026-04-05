#pragma once

#include "types.h"

#define LATITUDE_NA 100000000
#define LONGITUDE_NA 100000000

typedef struct {
    coordinate_bounds_t coordinate_bounds_far;
    coordinate_bounds_t coordinate_bounds_close;
    radar_data_t** radar_data_cache;
    size_t radar_data_cache_size;
} state_t;

extern state_t state;

void state_init();

void state_deinit();
