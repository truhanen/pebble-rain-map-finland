#include "comm_coordinates.h"

#include "../../windows/main/main_window.h"
#include "../state.h"

void comm_coordinates_handle_inbox(const DictionaryIterator* dict_iter) {
    Tuple* latitude_min_t = dict_find(dict_iter, MESSAGE_KEY_LATITUDE_MIN);
    Tuple* latitude_max_t = dict_find(dict_iter, MESSAGE_KEY_LATITUDE_MAX);
    Tuple* longitude_min_t = dict_find(dict_iter, MESSAGE_KEY_LONGITUDE_MIN);
    Tuple* longitude_max_t = dict_find(dict_iter, MESSAGE_KEY_LONGITUDE_MAX);

    coordinate_bounds_t coordinate_bounds_far;
    coordinate_bounds_far.latitude_min = latitude_min_t->value->int32;
    coordinate_bounds_far.latitude_max = latitude_max_t->value->int32;
    coordinate_bounds_far.longitude_min = longitude_min_t->value->int32;
    coordinate_bounds_far.longitude_max = longitude_max_t->value->int32;

    int32_t latitude =
        (coordinate_bounds_far.latitude_min + coordinate_bounds_far.latitude_max) / 2;
    int32_t longitude =
        (coordinate_bounds_far.longitude_min + coordinate_bounds_far.longitude_max) / 2;

    uint8_t zoom_level_close_factor = MAP_ZOOM_LEVEL_FAR / MAP_ZOOM_LEVEL_CLOSE;
    coordinate_bounds_t coordinate_bounds_close;
    int32_t latitude_radius_close =
        (latitude - coordinate_bounds_far.latitude_min) / zoom_level_close_factor;
    int32_t longitude_radius_close =
        (longitude - coordinate_bounds_far.longitude_min) / zoom_level_close_factor;
    coordinate_bounds_close.latitude_min = latitude - latitude_radius_close;
    coordinate_bounds_close.latitude_max = latitude + latitude_radius_close;
    coordinate_bounds_close.longitude_min = longitude - longitude_radius_close;
    coordinate_bounds_close.longitude_max = longitude + longitude_radius_close;

    uint8_t zoom_level_closest_factor = MAP_ZOOM_LEVEL_FAR / MAP_ZOOM_LEVEL_CLOSEST;
    coordinate_bounds_t coordinate_bounds_closest;
    int32_t latitude_radius_closest =
        (latitude - coordinate_bounds_far.latitude_min) / zoom_level_closest_factor;
    int32_t longitude_radius_closest =
        (longitude - coordinate_bounds_far.longitude_min) / zoom_level_closest_factor;
    coordinate_bounds_closest.latitude_min = latitude - latitude_radius_closest;
    coordinate_bounds_closest.latitude_max = latitude + latitude_radius_closest;
    coordinate_bounds_closest.longitude_min = longitude - longitude_radius_closest;
    coordinate_bounds_closest.longitude_max = longitude + longitude_radius_closest;

    state.coordinate_bounds_far = coordinate_bounds_far;
    state.coordinate_bounds_close = coordinate_bounds_close;
    state.coordinate_bounds_closest = coordinate_bounds_closest;

    main_window_update();
}
