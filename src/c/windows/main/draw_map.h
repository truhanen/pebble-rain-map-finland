#pragma once

#include "../../modules/types.h"

void draw_map(const Layer* layer, GContext* ctx, MapZoomLevel zoom_level);

void draw_map_init();

void draw_map_deinit();
