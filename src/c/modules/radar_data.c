#include "radar_data.h"

#define RADAR_DATA_BITS_PER_POINT 3
#define RADAR_DATA_POINTS_PER_BYTE (8 / RADAR_DATA_BITS_PER_POINT)  // 2

// dBZ to mm/h conversion using Z = 200 * R^1.6 (Marshall-Palmer).
// See https://en.wikipedia.org/wiki/DBZ_(meteorology).
// | dBz  | mm/h  | Comment                                                |
// |------|-------|--------------------------------------------------------|
// | 5    | 0.07  | Trace accumulation or mist                             |
// | 7    | 0.1   | Trace accumulation or mist THRESHOLD                   |
// | 10   | 0.15  | Trace accumulation or mist                             |
// | 15   | 0.3   | Trace accumulation                                     |
// | 18   | 0.5   | Trace accumulation to light rain THRESHOLD             |
// | 20   | 0.6   | Light rain                                             |
// | 25   | 1.3   | Light rain                                             |
// | 28   | 2.0   | Light to moderate rain THRESHOLD                       |
// | 30   | 2.7   | Light to moderate rain                                 |
// | 35   | 5.6   | Moderate rain                                          |
// | 39   | 10    | Moderate to heavy rain THRESHOLD                       |
// | 40   | 11.53 | Moderate to heavy rain                                 |
// | 45   | 23.7  | Heavy rain                                             |
// | 50   | 48.6  | Heavy to very heavy rain THRESHOLD                     |
// | 55   | 100   | Very heavy rain, hail possible                         |
// | 60   | 205   | Very heavy rain, hail likely                           |
// | 65   | 421   | Very heavy rain, hail very likely, large hail possible |

typedef enum {
    RAIN_LEVEL_THRESHOLD_MIST = 0,
    RAIN_LEVEL_THRESHOLD_TRACE = 7,
    RAIN_LEVEL_THRESHOLD_LIGHT = 18,
    RAIN_LEVEL_THRESHOLD_MODERATE = 28,
    RAIN_LEVEL_THRESHOLD_HEAVY = 39,
    RAIN_LEVEL_THRESHOLD_VERY_HEAVY = 50,
} RainLevelThreshold;

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

void radar_data_pack_point_value(
    const radar_data_t* radar_data,
    uint8_t point_value,
    size_t point_index
) {
    uint8_t packed_value;

    // Conversion of point_value to radar reflectivity dBZ: 0.5 * point_value - 32
    // See https://en.ilmatieteenlaitos.fi/open-data-manual-radar-data
    int dbz = ((int) point_value + 1) / 2 - 32;

    // Determine the 3-bit packed value based on thresholds
    if (dbz < RAIN_LEVEL_THRESHOLD_MIST || point_value == 0 || point_value == 255) {
        packed_value = PACKED_RAIN_LEVEL_NONE;
    } else if (dbz < RAIN_LEVEL_THRESHOLD_TRACE) {
        packed_value = PACKED_RAIN_LEVEL_MIST;
    } else if (dbz < RAIN_LEVEL_THRESHOLD_LIGHT) {
        packed_value = PACKED_RAIN_LEVEL_TRACE;
    } else if (dbz < RAIN_LEVEL_THRESHOLD_MODERATE) {
        packed_value = PACKED_RAIN_LEVEL_LIGHT;
    } else if (dbz < RAIN_LEVEL_THRESHOLD_HEAVY) {
        packed_value = PACKED_RAIN_LEVEL_MODERATE;
    } else if (dbz < RAIN_LEVEL_THRESHOLD_VERY_HEAVY) {
        packed_value = PACKED_RAIN_LEVEL_HEAVY;
    } else {
        packed_value = PACKED_RAIN_LEVEL_VERY_HEAVY;
    }

    // Pack the 3-bit value into the appropriate position in the buffer

    size_t bit_index = point_index * RADAR_DATA_BITS_PER_POINT;
    size_t byte_offset = bit_index / 8;
    size_t bit_position = bit_index % 8;

    // Use the explicit byte-combining method for writing to avoid alignment issues
    unsigned short combined_short = radar_data->data[byte_offset];
    combined_short |= (unsigned short) radar_data->data[byte_offset + 1] << 8;

    // 1. Clear the old 3 bits at the position
    combined_short &= ~(0x07u << bit_position);

    // 2. Set the new 3 bits
    combined_short |= packed_value << bit_position;

    // Write back the two bytes
    radar_data->data[byte_offset] = (unsigned char) combined_short;
    radar_data->data[byte_offset + 1] = (unsigned char) (combined_short >> 8);
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
