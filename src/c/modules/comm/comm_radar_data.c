#include "comm_radar_data.h"

#include "../../windows/main_window.h"
#include "../logging.h"
#include "../radar_data.h"
#include "../radar_data_cache.h"

static radar_data_t* s_radar_data = NULL;

void comm_radar_data_handle_inbox(const DictionaryIterator* dict_iter) {
    Tuple* radar_data_spec_t = dict_find(dict_iter, MESSAGE_KEY_RADAR_DATA_SPEC);
    if (radar_data_spec_t) {
        Tuple* timestamp_t = dict_find(dict_iter, MESSAGE_KEY_TIMESTAMP);
        time_t timestamp = timestamp_t->value->int32;

        Tuple* width_t = dict_find(dict_iter, MESSAGE_KEY_WIDTH);
        uint16_t width = width_t->value->int32;

        Tuple* height_t = dict_find(dict_iter, MESSAGE_KEY_HEIGHT);
        uint16_t height = height_t->value->int32;

        Tuple* zoom_level_t = dict_find(dict_iter, MESSAGE_KEY_ZOOM_LEVEL);
        MapZoomLevel zoom_level = (MapZoomLevel) zoom_level_t->value->int32;

        s_radar_data = radar_data_init(timestamp, zoom_level, width, height);

        log_heap_bytes();
    }

    Tuple* chunk_t = dict_find(dict_iter, MESSAGE_KEY_RADAR_DATA_CHUNK);
    if (chunk_t) {
        uint8_t* chunk_data = chunk_t->value->data;

        Tuple* chunk_size_t = dict_find(dict_iter, MESSAGE_KEY_RADAR_DATA_CHUNK_SIZE);
        int chunk_size = chunk_size_t->value->int32;

        Tuple* chunk_start_index_t =
            dict_find(dict_iter, MESSAGE_KEY_RADAR_DATA_CHUNK_START_INDEX);
        int chunk_start_index = chunk_start_index_t->value->int32;

        for (int i = 0; i < chunk_size; i++) {
            int point_index = chunk_start_index + i;
            uint8_t point_value = chunk_data[i];
            radar_data_pack_point_value(s_radar_data, point_value, point_index);
        }
    }

    Tuple* complete_t = dict_find(dict_iter, MESSAGE_KEY_RADAR_DATA_TRANSMIT_COMPLETE);
    if (complete_t) {
        radar_data_cache_set_item(s_radar_data);
        s_radar_data = NULL;
        main_window_update();
    }
}
