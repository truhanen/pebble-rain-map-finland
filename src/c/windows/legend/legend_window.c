#if defined(PBL_COLOR)

#include "./legend_window.h"

#include <pebble.h>

#include "../../modules/types.h"
#include "../main/draw.h"

#define ROW_HEIGHT 38
#define SWATCH_LEFT 0
#define SWATCH_SIZE 15
#define TEXT_LEFT 27
#define TEXT_WIDTH (PBL_DISPLAY_WIDTH - TEXT_LEFT - 4)
#define NAME_FONT FONT_KEY_GOTHIC_14_BOLD
#define RATE_FONT FONT_KEY_GOTHIC_14

typedef struct {
    RainLevel level;
    const char* name;
    const char* rate;
} LegendEntry;

static const LegendEntry s_entries[] = {
    {RAIN_LEVEL_MIST,       "Mist",               "< 0.15 mm/h" },
    {RAIN_LEVEL_TRACE,      "Trace accumulation", ">= 0.15 mm/h"},
    {RAIN_LEVEL_LIGHT,      "Light",              ">= 0.6 mm/h" },
    {RAIN_LEVEL_MODERATE,   "Moderate",           ">= 2.7 mm/h" },
    {RAIN_LEVEL_HEAVY,      "Heavy",              ">= 11.5 mm/h"},
    {RAIN_LEVEL_VERY_HEAVY, "Very heavy or hail", ">= 48.6 mm/h"},
};

#define N_ENTRIES ((int)(sizeof(s_entries) / sizeof(s_entries[0])))

static Window* s_window;
static Layer* s_draw_layer;

static void update_draw_layer(Layer* layer, GContext* ctx) {
    GRect bounds = layer_get_bounds(layer);

    graphics_context_set_fill_color(ctx, COLOR_DRAW_BACKGROUND);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);

    GFont name_font = fonts_get_system_font(NAME_FONT);
    GFont rate_font = fonts_get_system_font(RATE_FONT);

    for (int i = 0; i < N_ENTRIES; i++) {
        const LegendEntry* entry = &s_entries[i];
        int row_y = i * ROW_HEIGHT;

        // Color swatch
        int swatch_y = row_y;
        GRect swatch_rect = GRect(SWATCH_LEFT, swatch_y, SWATCH_SIZE, ROW_HEIGHT);

        GColor swatch_color = get_rain_level_color(entry->level);
        graphics_context_set_fill_color(ctx, swatch_color);
        graphics_fill_rect(ctx, swatch_rect, 0, GCornerNone);

        graphics_context_set_stroke_color(ctx, swatch_color);
        graphics_draw_rect(ctx, swatch_rect);

        // Level name
        GRect name_rect = GRect(TEXT_LEFT, row_y + 4, TEXT_WIDTH, 16);
        graphics_context_set_text_color(ctx, COLOR_DRAW_FOREGROUND);
        graphics_draw_text(
            ctx, entry->name, name_font, name_rect,
            GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL
        );

        // Rain rate threshold
        GRect rate_rect = GRect(TEXT_LEFT, row_y + 20, TEXT_WIDTH, 16);
        graphics_draw_text(
            ctx, entry->rate, rate_font, rate_rect,
            GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL
        );
    }
}

static void window_load(const Window* window) {
    Layer* root_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(root_layer);

    s_draw_layer = layer_create(bounds);
    layer_set_update_proc(s_draw_layer, update_draw_layer);
    layer_add_child(root_layer, s_draw_layer);
}

static void window_unload(Window* window) {
    layer_destroy(s_draw_layer);
    window_destroy(window);
}

void legend_window_push(void) {
    s_window = window_create();
    window_set_background_color(s_window, COLOR_DRAW_BACKGROUND);
    window_set_window_handlers(
        s_window,
        (WindowHandlers){
            .load = (WindowHandler) window_load,
            .unload = (WindowHandler) window_unload,
        }
    );
    window_stack_push(s_window, false);
}

#endif  // PBL_COLOR
