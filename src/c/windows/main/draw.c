#include "./draw.h"

#include <pebble.h>

#include "../../modules/radar_data.h"
#include "../../modules/radar_data_cache.h"
#include "../../modules/state.h"
#include "../../modules/types.h"
#include "../../modules/utils.h"

#define COLOR_RAIN_NONE GColorWhite
#define COLOR_RAIN_MIST GColorCyan
#define COLOR_RAIN_TRACE GColorJaegerGreen
#define COLOR_RAIN_LIGHT GColorChromeYellow  // Light orange
#define COLOR_RAIN_MODERATE GColorRed
#define COLOR_RAIN_HEAVY GColorPurple
#define COLOR_RAIN_VERY_HEAVY GColorImperialPurple

#define COLOR_BW_RAIN_NONE GColorWhite
#define COLOR_BW_RAIN GColorBlack

#define FONT_SMALL FONT_KEY_GOTHIC_09
#define FONT_MEDIUM FONT_KEY_GOTHIC_14_BOLD

#if defined(PBL_COLOR)
GColor get_rain_level_color(RainLevel level) {
    switch (level) {
        case RAIN_LEVEL_MIST:       return COLOR_RAIN_MIST;
        case RAIN_LEVEL_TRACE:      return COLOR_RAIN_TRACE;
        case RAIN_LEVEL_LIGHT:      return COLOR_RAIN_LIGHT;
        case RAIN_LEVEL_MODERATE:   return COLOR_RAIN_MODERATE;
        case RAIN_LEVEL_HEAVY:      return COLOR_RAIN_HEAVY;
        case RAIN_LEVEL_VERY_HEAVY: return COLOR_RAIN_VERY_HEAVY;
        default:                    return COLOR_RAIN_NONE;
    }
}
#endif

static void draw_radar_layer(
    const Layer* layer,
    GContext* ctx,
    const radar_data_t* radar_data,
    uint16_t view_width_km,
    bool rounded
) {
    if (radar_data == NULL) {
        return;
    }

    GRect layer_bounds = layer_get_bounds(layer);

    // Scale factor: screen pixels per radar data point, enlarged when data covers
    // more geographic area than the current view (e.g. FAR data in CLOSE view)
    int32_t pixels_per_point = (int32_t) layer_bounds.size.w * radar_data->width_km /
                               ((int32_t) radar_data->width_px * view_width_km);

    // Center the raster on screen (offset is negative when raster overflows screen)
    int32_t x_offset =
        ((int32_t) layer_bounds.size.w - (int32_t) radar_data->width_px * pixels_per_point) / 2;
    int32_t y_offset =
        ((int32_t) layer_bounds.size.h - (int32_t) radar_data->height_px * pixels_per_point) / 2;

    // Circle clipping: inscribed circle of the rendered raster
    int32_t center_x = x_offset + (int32_t) radar_data->width_px * pixels_per_point / 2;
    int32_t center_y = y_offset + (int32_t) radar_data->height_px * pixels_per_point / 2;
    int32_t radius = (int32_t) radar_data->width_px * pixels_per_point / 2;
    int32_t radius_sq = radius * radius;

    for (int32_t screen_y = 0; screen_y < (int32_t) layer_bounds.size.h; screen_y++) {
        int32_t radar_y = (screen_y - y_offset) / pixels_per_point;
        if (radar_y < 0 || radar_y >= (int32_t) radar_data->height_px) {
            continue;
        }
        for (int32_t screen_x = 0; screen_x < (int32_t) layer_bounds.size.w; screen_x++) {
            int32_t radar_x = (screen_x - x_offset) / pixels_per_point;
            if (radar_x < 0 || radar_x >= (int32_t) radar_data->width_px) {
                continue;
            }
            if (rounded) {
                int32_t dx = screen_x - center_x;
                int32_t dy = screen_y - center_y;
                if (dx * dx + dy * dy > radius_sq) {
                    continue;
                }
            }
            RainLevel rain_level =
                radar_data_get_point_rain_level(radar_data, radar_y, radar_x);

            GColor color = COLOR_BW_RAIN_NONE;

#if defined(PBL_BW)
            bool is_rain_pixel = true;  // High rain
            if (rain_level == RAIN_LEVEL_NONE) {
                is_rain_pixel = false;
            } else if (
                rain_level == RAIN_LEVEL_MIST || rain_level == RAIN_LEVEL_TRACE ||
                rain_level == RAIN_LEVEL_LIGHT
            ) {
                // Sparse pattern
                is_rain_pixel =
                    ((screen_x % 2 == 0 && screen_y % 2 == 0 && screen_y % 4 == 0) ||
                     (screen_x % 2 == 1 && screen_y % 2 == 0 && screen_y % 4 != 0));
            } else if (rain_level == RAIN_LEVEL_MODERATE) {
                // Every other pixel black
                is_rain_pixel = ((screen_x + screen_y) % 2 == 0);
            }

            color = is_rain_pixel ? COLOR_BW_RAIN : COLOR_BW_RAIN_NONE;
#elif defined(PBL_COLOR)
            color = get_rain_level_color(rain_level);
#endif

            graphics_context_set_stroke_color(ctx, color);
            graphics_draw_pixel(ctx, GPoint(screen_x, screen_y));
        }
    }
}

void draw_radar(
    const Layer* layer,
    GContext* ctx,
    int timestep_index,
    MapZoomLevel zoom_level
) {
    if (timestep_index < 0) {
        return;
    }

    int widths_km_closest[] = {100, 50};
    int widths_km_close[] = {400, 100, 50};
    int widths_km_far[] = {400, 100};
    int* widths_km;
    int n_widths;

    if (zoom_level == MAP_ZOOM_LEVEL_CLOSEST) {
        widths_km = widths_km_closest;
        n_widths = 2;
    } else if (zoom_level == MAP_ZOOM_LEVEL_CLOSE) {
        widths_km = widths_km_close;
        n_widths = 3;
    } else {
        widths_km = widths_km_far;
        n_widths = 2;
    }

    for (int i = 0; i < n_widths; i++) {
        draw_radar_layer(
            layer, ctx,
            radar_data_cache_get_item(timestep_index, widths_km[i]),
            (uint16_t) zoom_level,
            i != 0
        );
    }
}

static void draw_text(
    GContext* ctx,
    const char* text,
    GRect rect,
    GFont font,
    GTextAlignment alignment
) {
    GRect rect_shadow =
        GRect(rect.origin.x + 1, rect.origin.y + 1, rect.size.w, rect.size.h);
    graphics_context_set_text_color(ctx, COLOR_DRAW_BACKGROUND);
    graphics_draw_text(
        ctx, text, font, rect_shadow, GTextOverflowModeWordWrap, alignment, NULL
    );

    graphics_context_set_text_color(ctx, COLOR_DRAW_FOREGROUND);
    graphics_draw_text(
        ctx, text, font, rect, GTextOverflowModeWordWrap, alignment, NULL
    );
}

void draw_circle(const Layer* layer, GContext* ctx, MapZoomLevel zoom_level) {
    GRect layer_bounds = layer_get_bounds(layer);

    size_t width_kilometers = zoom_level;

    uint16_t pixels_per_100_km = layer_bounds.size.w * 100 / width_kilometers;
    int32_t circle_radius_km = 50;
    uint16_t circle_radius = (uint16_t) (circle_radius_km * pixels_per_100_km / 100);
    GRect circle_rect = GRect(
        layer_bounds.size.w / 2 - circle_radius,
        layer_bounds.size.h / 2 - circle_radius,
        circle_radius * 2,
        circle_radius * 2
    );

    int32_t angle_start = 205;
    int32_t angle_end = 515;
    // Less angle is needed for the text when magnified
    if (zoom_level == MAP_ZOOM_LEVEL_CLOSE) {
        angle_start = 192;
        angle_end = 528;
    } else if (zoom_level == MAP_ZOOM_LEVEL_CLOSEST) {
        angle_start = 186;
        angle_end = 534;
    }

    graphics_context_set_stroke_color(ctx, COLOR_DRAW_FOREGROUND);
    graphics_context_set_stroke_width(ctx, 1);
    graphics_draw_arc(
        ctx,
        circle_rect,
        GOvalScaleModeFillCircle,
        DEG_TO_TRIGANGLE(angle_start),
        DEG_TO_TRIGANGLE(angle_end)
    );

    GRect text_rect = GRect(0, circle_rect.origin.y + circle_rect.size.h - 6, layer_bounds.size.w, 10);

    draw_text(
        ctx, "50km", text_rect, fonts_get_system_font(FONT_SMALL), GTextAlignmentCenter
    );
}

void draw_crosshair(const Layer* layer, GContext* ctx) {
    GRect bounds = layer_get_bounds(layer);
    GPoint center = GPoint(bounds.size.w / 2, bounds.size.h / 2);

    int arm = 15;
    int hole = 9;
    int half_base = 1;

    graphics_context_set_stroke_color(ctx, COLOR_DRAW_FOREGROUND);
    // graphics_context_set_fill_color(ctx, COLOR_DRAW_FOREGROUND);

    // Left triangle (pointing right toward center)
    GPoint left_points[] = {
        GPoint(center.x - hole, center.y),
        GPoint(center.x - arm, center.y - half_base),
        GPoint(center.x - arm, center.y + half_base),
    };
    GPathInfo left_info = { .num_points = 3, .points = left_points };
    GPath* left_path = gpath_create(&left_info);
    gpath_draw_outline(ctx, left_path);
    // gpath_draw_filled(ctx, left_path);
    gpath_destroy(left_path);

    // Right triangle (pointing left toward center)
    GPoint right_points[] = {
        GPoint(center.x + hole, center.y),
        GPoint(center.x + arm, center.y - half_base),
        GPoint(center.x + arm, center.y + half_base),
    };
    GPathInfo right_info = { .num_points = 3, .points = right_points };
    GPath* right_path = gpath_create(&right_info);
    gpath_draw_outline(ctx, right_path);
    // gpath_draw_filled(ctx, right_path);
    gpath_destroy(right_path);

    // Top triangle (pointing down toward center)
    GPoint top_points[] = {
        GPoint(center.x, center.y - hole),
        GPoint(center.x - half_base, center.y - arm),
        GPoint(center.x + half_base, center.y - arm),
    };
    GPathInfo top_info = { .num_points = 3, .points = top_points };
    GPath* top_path = gpath_create(&top_info);
    gpath_draw_outline(ctx, top_path);
    // gpath_draw_filled(ctx, top_path);
    gpath_destroy(top_path);

    // Bottom triangle (pointing up toward center)
    GPoint bottom_points[] = {
        GPoint(center.x, center.y + hole),
        GPoint(center.x - half_base, center.y + arm),
        GPoint(center.x + half_base, center.y + arm),
    };
    GPathInfo bottom_info = { .num_points = 3, .points = bottom_points };
    GPath* bottom_path = gpath_create(&bottom_info);
    gpath_draw_outline(ctx, bottom_path);
    // gpath_draw_filled(ctx, bottom_path);
    gpath_destroy(bottom_path);
}

void draw_timestep_indicator(
    const Layer* layer,
    GContext* ctx,
    int timestep_index
) {
    if (timestep_index < 0) {
        return;
    }

    size_t radar_data_item_count =
        radar_data_cache_get_max_item_count();
    if (radar_data_item_count == 0) {
        return;
    }

    GRect bounds = layer_get_bounds(layer);

    int width = 5;
    int height = bounds.size.h / (int) radar_data_item_count + 1;
    int y = bounds.size.h - height * (timestep_index + 1);
    GRect rect = GRect(bounds.size.w - width, y, width, height);

    graphics_context_set_fill_color(ctx, COLOR_DRAW_FOREGROUND);
    graphics_fill_rect(ctx, rect, 2, GCornersLeft);
}

int convert_y_to_timestep_index(const Layer* layer, int y) {
    size_t count = radar_data_cache_get_max_item_count();
    if (count == 0) {
        return -1;
    }

    GRect bounds = layer_get_bounds(layer);
    int height = bounds.size.h / (int) count + 1;
    int index = (bounds.size.h - y - 1) / height;

    if (index < 0) {
        index = 0;
    } else if (index >= (int) count) {
        index = (int) count - 1;
    }

    return index;
}

void draw_timestamp(
    const Layer* layer,
    GContext* ctx,
    int timestep_index
) {
    if (timestep_index < 0) {
        return;
    }

    radar_data_t* radar_data = radar_data_cache_get_item(timestep_index, 400);

    if (radar_data == NULL) {
        return;
    }

    time_t timestamp = radar_data->timestamp;
    tm* time_info = localtime(&timestamp);
    char timestamp_text[16];
    strftime(timestamp_text, sizeof(timestamp_text), "%H:%M", time_info);

    GFont font = fonts_get_system_font(FONT_MEDIUM);
    GTextAlignment alignment = GTextAlignmentCenter;
    GTextOverflowMode overflow_mode = GTextOverflowModeTrailingEllipsis;
    int16_t rect_height = 16;

    GRect layer_bounds = layer_get_bounds(layer);

    // Calculate the width of the text rectangle dynamically
    GSize text_size = graphics_text_layout_get_content_size(
        timestamp_text,
        font,
        GRect(0, 0, layer_bounds.size.w, rect_height),
        overflow_mode,
        alignment
    );
    int rect_width = text_size.w + 4;

    // Negative y-coordinate for fine-tuned text position
    GRect rect =
        GRect((layer_bounds.size.w - rect_width) / 2, -3, rect_width, rect_height);

    graphics_context_set_fill_color(ctx, COLOR_DRAW_FOREGROUND);
    graphics_context_set_text_color(ctx, COLOR_DRAW_BACKGROUND);

    graphics_fill_rect(ctx, rect, 2, GCornersBottom);
    graphics_draw_text(ctx, timestamp_text, font, rect, overflow_mode, alignment, NULL);
}
