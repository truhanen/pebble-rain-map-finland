#pragma once

#include <pebble.h>

#include "../modules/types.h"

#define COLOR_DRAW_FOREGROUND GColorBlack
#define COLOR_DRAW_BACKGROUND GColorWhite

void draw_radar(
    const Layer* layer,
    GContext* ctx,
    int timestep_index,
    MapZoomLevel zoom_level
);

void draw_circle(const Layer* layer, GContext* ctx, MapZoomLevel zoom_level);

void draw_crosshair(const Layer* layer, GContext* ctx);

void draw_timestep_indicator(
    const Layer* layer,
    GContext* ctx,
    int timestep_index
);

void draw_timestamp(
    const Layer* layer,
    GContext* ctx,
    int timestep_index
);
