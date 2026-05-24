#include "comm.h"

#include <pebble.h>

#include "comm_coordinates.h"
#include "comm_radar_data.h"

void request_transmit() {
    DictionaryIterator* dict_iter;
    AppMessageResult result = app_message_outbox_begin(&dict_iter);
    if (result == APP_MSG_OK) {
        dict_write_uint8(dict_iter, MESSAGE_KEY_TRANSMIT_DATA, 0);
        result = app_message_outbox_send();
        if (result != APP_MSG_OK) {
            APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending outbox: %d", (int) result);
        }
    } else {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing outbox: %d", (int) result);
    }
}

static void inbox_received_handler(DictionaryIterator* dict_iter, void* context) {
    if (dict_find(dict_iter, MESSAGE_KEY_JS_READY)) {
        request_transmit();
    } else if (dict_find(dict_iter, MESSAGE_KEY_COORDINATES)) {
        comm_coordinates_handle_inbox(dict_iter);
    } else if (
        dict_find(dict_iter, MESSAGE_KEY_RADAR_DATA_SPEC) ||
        dict_find(dict_iter, MESSAGE_KEY_RADAR_DATA_CHUNK) ||
        dict_find(dict_iter, MESSAGE_KEY_RADAR_DATA_TRANSMIT_COMPLETE)
    ) {
        comm_radar_data_handle_inbox(dict_iter);
    }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message receive failed (dropped). Reason: %d", (int)reason);
}

static void outbox_sent_callback(DictionaryIterator *iter, void *context) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Message send succeeded");
}

static void outbox_failed_callback(DictionaryIterator *iter,
                                      AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message send failed. Reason: %d", (int)reason);
}

void comm_init() {
    APP_LOG(APP_LOG_LEVEL_INFO, "comm_init");

    app_message_register_inbox_received(inbox_received_handler);
    app_message_register_inbox_dropped(inbox_dropped_callback);

    app_message_register_outbox_sent(outbox_sent_callback);
    app_message_register_outbox_failed(outbox_failed_callback);

    uint32_t inbox_size = app_message_inbox_size_maximum();
    uint32_t outbox_size = 256;
    app_message_open(inbox_size, outbox_size);
}
