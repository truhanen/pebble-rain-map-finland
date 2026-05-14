#pragma once

#include <pebble.h>

// Define macros that are eventually given by build arguments

#ifndef PBL_DISPLAY_WIDTH
#define PBL_DISPLAY_WIDTH 200
#endif

#ifndef PBL_DISPLAY_HEIGHT
#define PBL_DISPLAY_HEIGHT 228
#endif

#ifndef PBL_COLOR
#define PBL_COLOR
#endif

typedef enum {
    RAIN_LEVEL_NONE,
    RAIN_LEVEL_MIST,
    RAIN_LEVEL_TRACE,
    RAIN_LEVEL_LIGHT,
    RAIN_LEVEL_MODERATE,
    RAIN_LEVEL_HEAVY,
    RAIN_LEVEL_VERY_HEAVY,
} RainLevel;

typedef struct {
    int32_t latitude;
    int32_t longitude;
} location_t;

typedef struct {
    char* name;
    location_t location;
} place_t;

typedef struct {
    int32_t latitude_min;
    int32_t latitude_max;
    int32_t longitude_min;
    int32_t longitude_max;
} coordinate_bounds_t;

/**
 * Values are widths in kilometers
 */
typedef enum {
    MAP_ZOOM_LEVEL_CLOSEST = 100,
    MAP_ZOOM_LEVEL_CLOSE = 200,
    MAP_ZOOM_LEVEL_FAR = 400,
} MapZoomLevel;

typedef struct {
    time_t timestamp;
    uint16_t width_km;
    uint16_t width_px;
    uint16_t height_px;
    uint8_t* data;
} radar_data_t;
