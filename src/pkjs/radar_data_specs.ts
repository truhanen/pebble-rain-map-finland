import { DISPLAY_DIMENSIONS } from "./pebble";

const WIDTH_KM_SMALLEST = 50;
const WIDTH_KM_SMALL = 100;
const WIDTH_KM_LARGE = 400;

const WIDTH_PX_SMALL = DISPLAY_DIMENSIONS.width * (WIDTH_KM_SMALL / WIDTH_KM_LARGE);
const WIDTH_PX_SMALLEST = WIDTH_PX_SMALL;
const WIDTH_PX_LARGE_1 = DISPLAY_DIMENSIONS.width / 2;
const WIDTH_PX_LARGE_2 = DISPLAY_DIMENSIONS.width / 4;

export const RADAR_DATA_SPECS: RadarDataSpec[] = [
    {
        minutesBeforeLatest: 0,
        widthPx: WIDTH_PX_LARGE_1,
        widthKm: WIDTH_KM_LARGE,
    },
    {
        minutesBeforeLatest: 0,
        widthPx: WIDTH_PX_SMALL,
        widthKm: WIDTH_KM_SMALL,
    },
    {
        minutesBeforeLatest: 0,
        widthPx: WIDTH_PX_SMALLEST,
        widthKm: WIDTH_KM_SMALLEST,
    },
    {
        minutesBeforeLatest: 10,
        widthPx: WIDTH_PX_LARGE_1,
        widthKm: WIDTH_KM_LARGE,
    },
    {
        minutesBeforeLatest: 10,
        widthPx: WIDTH_PX_SMALL,
        widthKm: WIDTH_KM_SMALL,
    },
    {
        minutesBeforeLatest: 10,
        widthPx: WIDTH_PX_SMALLEST,
        widthKm: WIDTH_KM_SMALLEST,
    },
    {
        minutesBeforeLatest: 20,
        widthPx: WIDTH_PX_LARGE_2,
        widthKm: WIDTH_KM_LARGE,
    },
    {
        minutesBeforeLatest: 20,
        widthPx: WIDTH_PX_SMALL,
        widthKm: WIDTH_KM_SMALL,
    },
    {
        minutesBeforeLatest: 30,
        widthPx: WIDTH_PX_LARGE_2,
        widthKm: WIDTH_KM_LARGE,
    },
    {
        minutesBeforeLatest: 30,
        widthPx: WIDTH_PX_SMALL,
        widthKm: WIDTH_KM_SMALL,
    },
    {
        minutesBeforeLatest: 40,
        widthPx: WIDTH_PX_LARGE_2,
        widthKm: WIDTH_KM_LARGE,
    },
    {
        minutesBeforeLatest: 40,
        widthPx: WIDTH_PX_SMALL,
        widthKm: WIDTH_KM_SMALL,
    },
    {
        minutesBeforeLatest: 50,
        widthPx: WIDTH_PX_LARGE_2,
        widthKm: WIDTH_KM_LARGE,
    },
    {
        minutesBeforeLatest: 50,
        widthPx: WIDTH_PX_SMALL,
        widthKm: WIDTH_KM_SMALL,
    },
    {
        minutesBeforeLatest: 60,
        widthPx: WIDTH_PX_LARGE_2,
        widthKm: WIDTH_KM_LARGE,
    },
    {
        minutesBeforeLatest: 60,
        widthPx: WIDTH_PX_SMALL,
        widthKm: WIDTH_KM_SMALL,
    },
    {
        minutesBeforeLatest: 20,
        widthPx: WIDTH_PX_SMALLEST,
        widthKm: WIDTH_KM_SMALLEST,
    },
    {
        minutesBeforeLatest: 30,
        widthPx: WIDTH_PX_SMALLEST,
        widthKm: WIDTH_KM_SMALLEST,
    },
    {
        minutesBeforeLatest: 40,
        widthPx: WIDTH_PX_SMALLEST,
        widthKm: WIDTH_KM_SMALLEST,
    },
    {
        minutesBeforeLatest: 50,
        widthPx: WIDTH_PX_SMALLEST,
        widthKm: WIDTH_KM_SMALLEST,
    },
    {
        minutesBeforeLatest: 60,
        widthPx: WIDTH_PX_SMALLEST,
        widthKm: WIDTH_KM_SMALLEST,
    },
];

export function getRadarDataSpec(specIndex: number): RadarDataSpec {
    const spec = RADAR_DATA_SPECS[specIndex];
    if (!spec) {
        throw Error("specIndex out of bounds");
    } else {
        return spec;
    }
}
