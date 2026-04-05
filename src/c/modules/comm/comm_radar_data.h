#pragma once

#include <pebble.h>

void comm_radar_data_request_download();

void comm_radar_data_handle_inbox(const DictionaryIterator* dict_iter);
