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
const DBZ_THRESHOLD_MIST = 0;
const DBZ_THRESHOLD_TRACE = 7;
const DBZ_THRESHOLD_LIGHT = 18;
const DBZ_THRESHOLD_MODERATE = 28;
const DBZ_THRESHOLD_HEAVY = 39;
const DBZ_THRESHOLD_VERY_HEAVY = 50;

const PACKED_NONE = 0;       // 0b000
const PACKED_MIST = 1;       // 0b001
const PACKED_TRACE = 2;      // 0b010
const PACKED_LIGHT = 3;      // 0b011
const PACKED_MODERATE = 4;   // 0b100
const PACKED_HEAVY = 5;      // 0b101
const PACKED_VERY_HEAVY = 6; // 0b110

const BITS_PER_PIXEL = 3;

// See https://en.ilmatieteenlaitos.fi/open-data-manual-radar-data
function pointValueToDbz(pointValue: number): number {
    return Math.floor((pointValue + 1) / 2) - 32;
}

function getPackedRainLevel(pointValue: number): number {
    if (pointValue === 0 || pointValue === 255) {
        return PACKED_NONE;
    }
    const dbz = pointValueToDbz(pointValue);
    if (dbz < DBZ_THRESHOLD_MIST) return PACKED_NONE;
    if (dbz < DBZ_THRESHOLD_TRACE) return PACKED_MIST;
    if (dbz < DBZ_THRESHOLD_LIGHT) return PACKED_TRACE;
    if (dbz < DBZ_THRESHOLD_MODERATE) return PACKED_LIGHT;
    if (dbz < DBZ_THRESHOLD_HEAVY) return PACKED_MODERATE;
    if (dbz < DBZ_THRESHOLD_VERY_HEAVY) return PACKED_HEAVY;
    return PACKED_VERY_HEAVY;
}

export function packRadarData(dataView: DataView): Uint8Array {
    const pixelCount = dataView.byteLength;
    const bitCount = pixelCount * BITS_PER_PIXEL;
    // +1 extra byte matches the C allocation for safe 16-bit reads at the boundary
    const byteCount = Math.floor((bitCount + 7) / 8) + 1;
    const packed = new Uint8Array(byteCount);

    for (let pixelIndex = 0; pixelIndex < pixelCount; pixelIndex++) {
        const pointValue = dataView.getUint8(pixelIndex);
        const packedValue = getPackedRainLevel(pointValue);

        const bitIndex = pixelIndex * BITS_PER_PIXEL;
        const byteOffset = Math.floor(bitIndex / 8);
        const bitPosition = bitIndex % 8;

        // Read 16 bits spanning the byte boundary, set 3 bits, write back
        let combined = (packed[byteOffset] ?? 0) | ((packed[byteOffset + 1] ?? 0) << 8);
        combined = (combined & ~(0x07 << bitPosition)) | (packedValue << bitPosition);
        packed[byteOffset] = combined & 0xff;
        packed[byteOffset + 1] = (combined >> 8) & 0xff;
    }

    return packed;
}
