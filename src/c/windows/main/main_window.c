#include "./main_window.h"

#include <pebble.h>

#include "../../modules/radar_data_cache.h"
#include "../../modules/types.h"
#include "./draw.h"
#include "./draw_map.h"
#include "../legend/legend_window.h"

static Window* s_window;
static Layer* s_draw_layer;
static MapZoomLevel s_map_zoom_level = MAP_ZOOM_LEVEL_FAR;
static int s_timestep_index = -1;

static void update_draw_layer(Layer* layer, GContext* ctx) {
    if (s_timestep_index < 0 && radar_data_cache_get_max_item_count() > 0) {
        s_timestep_index = 0;
    }

    draw_radar(layer, ctx, s_timestep_index, s_map_zoom_level);
    draw_map(layer, ctx, s_map_zoom_level);
    draw_circle(layer, ctx, s_map_zoom_level);
    draw_crosshair(layer, ctx);
    draw_timestep_indicator(layer, ctx, s_timestep_index);
    draw_timestamp(layer, ctx, s_timestep_index);
}

static void window_load(const Window* window) {
    Layer* root_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(root_layer);

    // Drawing layer
    s_draw_layer = layer_create(bounds);
    layer_set_update_proc(s_draw_layer, update_draw_layer);
    layer_add_child(root_layer, s_draw_layer);
}

#if defined(PBL_TOUCH)
static void touch_handler(const TouchEvent* event, void* context) {
    if (event->type != TouchEvent_Touchdown && event->type != TouchEvent_PositionUpdate) {
        return;
    }
    int new_index = convert_y_to_timestep_index(s_draw_layer, event->y);
    if (new_index < 0 || new_index == s_timestep_index) {
        return;
    }
    s_timestep_index = new_index;
    main_window_update();
}

static void window_appear(Window* window) {
    if (touch_service_is_enabled()) {
        touch_service_subscribe(touch_handler, NULL);
    }
}

static void window_disappear(Window* window) {
    touch_service_unsubscribe();
}
#endif

static void window_unload(Window* window) {
    layer_destroy(s_draw_layer);
    window_destroy(window);
}

void zoom_closer() {
    if (s_map_zoom_level == MAP_ZOOM_LEVEL_FAR) {
        s_map_zoom_level = MAP_ZOOM_LEVEL_CLOSE;
    } else if (s_map_zoom_level == MAP_ZOOM_LEVEL_CLOSE) {
        s_map_zoom_level = MAP_ZOOM_LEVEL_CLOSEST;
    }
    main_window_update();
}

void zoom_farther() {
    if (s_map_zoom_level == MAP_ZOOM_LEVEL_CLOSEST) {
        s_map_zoom_level = MAP_ZOOM_LEVEL_CLOSE;
        main_window_update();
    } else if (s_map_zoom_level == MAP_ZOOM_LEVEL_CLOSE) {
        s_map_zoom_level = MAP_ZOOM_LEVEL_FAR;
        main_window_update();
    } else {
        window_stack_pop(true);
    }
}

void change_timestep_index(int timestep_index_change) {
    int timestep_index_changed = s_timestep_index + timestep_index_change;
    int max_item_count = (int) radar_data_cache_get_max_item_count();
    if (timestep_index_changed < 0) {
        timestep_index_changed = max_item_count - 1;
    }
    if (timestep_index_changed >= max_item_count) {
        timestep_index_changed = 0;
    }
    s_timestep_index = timestep_index_changed;

    main_window_update();
}

void select_single_click_handler(ClickRecognizerRef recognizer, void* context) {
#if defined(PBL_COLOR)
    if (s_map_zoom_level == MAP_ZOOM_LEVEL_CLOSEST) {
        legend_window_push();
        return;
    }
#endif
    zoom_closer();
}

void back_single_click_handler(ClickRecognizerRef recognizer, void* context) {
    zoom_farther();
}

void up_single_click_handler(ClickRecognizerRef recognizer, void* context) {
    change_timestep_index(1);
}

void down_single_click_handler(ClickRecognizerRef recognizer, void* context) {
    change_timestep_index(-1);
}

void click_config_provider(Window* window) {
    window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
    window_single_click_subscribe(BUTTON_ID_BACK, back_single_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, up_single_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_single_click_handler);
}

void main_window_push() {
    s_timestep_index = -1;
    s_map_zoom_level = MAP_ZOOM_LEVEL_FAR;

    s_window = window_create();
    window_set_click_config_provider(
        s_window, (ClickConfigProvider) click_config_provider
    );
    window_set_background_color(s_window, COLOR_DRAW_BACKGROUND);
    window_set_window_handlers(
        s_window,
        (WindowHandlers) {
            .load = (WindowHandler) window_load,
            .unload = (WindowHandler) window_unload,
#if defined(PBL_TOUCH)
            .appear = (WindowHandler) window_appear,
            .disappear = (WindowHandler) window_disappear,
#endif
        }
    );
    window_stack_push(s_window, true);
}

void main_window_update() {
    Layer* root_layer = window_get_root_layer(s_window);
    layer_mark_dirty(root_layer);
}
