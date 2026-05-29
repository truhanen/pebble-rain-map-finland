#include "radar_data.h"

#define RADAR_DATA_BITS_PER_POINT 3
#define RADAR_DATA_POINTS_PER_BYTE (8 / RADAR_DATA_BITS_PER_POINT)  // 2

typedef enum {
    PACKED_RAIN_LEVEL_NONE = 0b000,
    PACKED_RAIN_LEVEL_MIST = 0b001,
    PACKED_RAIN_LEVEL_TRACE = 0b010,
    PACKED_RAIN_LEVEL_LIGHT = 0b011,
    PACKED_RAIN_LEVEL_MODERATE = 0b100,
    PACKED_RAIN_LEVEL_HEAVY = 0b101,
    PACKED_RAIN_LEVEL_VERY_HEAVY = 0b110,
} PackedRainLevel;

size_t get_radar_data_pixel_height(size_t pixel_width) {
    return pixel_width * PBL_DISPLAY_HEIGHT / PBL_DISPLAY_WIDTH;
}

radar_data_t* radar_data_init(
    time_t timestamp,
    uint16_t width_km,
    uint16_t width_px,
    uint16_t height_px
) {
    size_t point_count = width_px * height_px;
    size_t bit_count = RADAR_DATA_BITS_PER_POINT * point_count;
    // Calculate the minimum number of bytes required, plus one extra byte
    size_t byte_count = (bit_count + 7) / 8 + 1;

    uint8_t* data = malloc(byte_count * sizeof(uint8_t));

    if (data != NULL) {
        // Initialize memory to zero to ensure all bits are clean
        for (size_t i = 0; i < byte_count; i++) {
            data[i] = 0;
        }
    }

    if (!data) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "malloc failed for size: %u", byte_count);
        return NULL;
    }
    memset(data, 0, byte_count);

    radar_data_t* radar_data = malloc(sizeof(radar_data_t));
    if (!radar_data) {
        APP_LOG(
            APP_LOG_LEVEL_ERROR, "malloc failed for size: %u", sizeof(radar_data_t)
        );
        return NULL;
    }

    radar_data->data = data;
    radar_data->timestamp = timestamp;
    radar_data->width_km = width_km;
    radar_data->width_px = width_px;
    radar_data->height_px = height_px;

    return radar_data;
}

void radar_data_deinit(const radar_data_t* radar_data) {
    free(radar_data->data);
}

RainLevel radar_data_get_point_rain_level(
    const radar_data_t* radar_data,
    size_t y,
    size_t x
) {
    size_t point_index = y * radar_data->width_px + x;

    size_t bit_index = point_index * RADAR_DATA_BITS_PER_POINT;
    size_t byte_offset = bit_index / 8;
    size_t bit_position = bit_index % 8;

    // We treat the current byte offset as the start of a 16-bit block.
    // WARNING: This assumes proper memory alignment and that the memory
    // following the byte_offset is accessible (i.e., we don't read past the end).
    // This is the "simpler" version requested, which skips boundary checks.
    unsigned short* short_ptr = (unsigned short*) (radar_data->data + byte_offset);
    unsigned short combined_value = *short_ptr;

    // Right-shift by the starting bit position (P0) to move the 3 bits to LSB
    combined_value >>= bit_position;

    // Isolate the final three bits
    uint8_t packed_value = combined_value & 0b111u;

    switch (packed_value) {
        case PACKED_RAIN_LEVEL_NONE: return RAIN_LEVEL_NONE;
        case PACKED_RAIN_LEVEL_MIST: return RAIN_LEVEL_MIST;
        case PACKED_RAIN_LEVEL_TRACE: return RAIN_LEVEL_TRACE;
        case PACKED_RAIN_LEVEL_LIGHT: return RAIN_LEVEL_LIGHT;
        case PACKED_RAIN_LEVEL_MODERATE: return RAIN_LEVEL_MODERATE;
        case PACKED_RAIN_LEVEL_HEAVY: return RAIN_LEVEL_HEAVY;
        case PACKED_RAIN_LEVEL_VERY_HEAVY: return RAIN_LEVEL_VERY_HEAVY;
        default: return RAIN_LEVEL_NONE;
    }
}
