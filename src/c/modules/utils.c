#include "utils.h"

#include "state.h"

// Convert C string to tm
tm cstring_to_tm(const char* string) {
    tm timestamp = {0};

    if (string == NULL) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "string is NULL");
        return timestamp;
    }

    APP_LOG(APP_LOG_LEVEL_DEBUG, "parsing timestamp string: %s", string);

    if (strlen(string) >= 15) {
        char year[5], month[3], day[3], hour[3], minute[3], second[3];
        strncpy(year, string, 4);
        year[4] = '\0';
        strncpy(month, string + 4, 2);
        month[2] = '\0';
        strncpy(day, string + 6, 2);
        day[2] = '\0';
        strncpy(hour, string + 9, 2);
        hour[2] = '\0';
        strncpy(minute, string + 11, 2);
        minute[2] = '\0';
        strncpy(second, string + 13, 2);
        second[2] = '\0';

        timestamp.tm_year = atoi(year) - 1900;
        timestamp.tm_mon = atoi(month) - 1;
        timestamp.tm_mday = atoi(day);
        timestamp.tm_hour = atoi(hour);
        timestamp.tm_min = atoi(minute);
        timestamp.tm_sec = atoi(second);
    } else {
        APP_LOG(APP_LOG_LEVEL_ERROR, "malformed timestamp string");
    }

    return timestamp;
}

// Convert tm to C string
void tm_to_cstring(const tm* timestamp, char* out, size_t out_size) {
    strftime(out, out_size, "%Y%m%dT%H%M%S", timestamp);
}

GPoint location_to_gpoint(
    const Layer* layer,
    const location_t* location,
    MapZoomLevel zoom_level
) {
    GRect bounds = layer_get_bounds(layer);

    coordinate_bounds_t coordinate_bounds;
    if (zoom_level == MAP_ZOOM_LEVEL_FAR) {
        coordinate_bounds = state.coordinate_bounds_far;
    } else if (zoom_level == MAP_ZOOM_LEVEL_CLOSE) {
        coordinate_bounds = state.coordinate_bounds_close;
    } else {
        coordinate_bounds = state.coordinate_bounds_closest;
    }

    int32_t longitude_range =
        coordinate_bounds.longitude_max - coordinate_bounds.longitude_min;
    int32_t latitude_range =
        coordinate_bounds.latitude_max - coordinate_bounds.latitude_min;
    int16_t longitude_in_pixels =
        (int16_t) ((location->longitude - coordinate_bounds.longitude_min) *
                   bounds.size.w / longitude_range);
    int16_t latitude_in_pixels =
        (int16_t) ((coordinate_bounds.latitude_max - location->latitude) *
                   bounds.size.h / latitude_range);

    GPoint gpoint = {.x = longitude_in_pixels, .y = latitude_in_pixels};

    return gpoint;
}
