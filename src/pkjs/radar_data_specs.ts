import { DISPLAY_DIMENSIONS } from "./pebble";

const DISPLAY_WIDTH_KM_ZOOM_LEVEL_FAR = 400;

const WIDTH_KM_SMALL = 100;
const WIDTH_KM_LARGE = DISPLAY_WIDTH_KM_ZOOM_LEVEL_FAR;

const WIDTH_PX_SMALL = DISPLAY_DIMENSIONS.width * (WIDTH_KM_SMALL / WIDTH_KM_LARGE);
const WIDTH_PX_LARGE_1 = DISPLAY_DIMENSIONS.width / 2;
const WIDTH_PX_LARGE_2 = DISPLAY_DIMENSIONS.width / 4;

export const RADAR_DATA_SPECS: RadarDataSpec[] = [
    {
        minutesBeforeLatest: 0,
        widthPx: WIDTH_PX_LARGE_1,
        widthKm: WIDTH_KM_LARGE,
        isSquare: false,
    },
    {
        minutesBeforeLatest: 0,
        widthPx: WIDTH_PX_SMALL,
        widthKm: WIDTH_KM_SMALL,
        isSquare: true,
    },
    {
        minutesBeforeLatest: 10,
        widthPx: WIDTH_PX_LARGE_1,
        widthKm: WIDTH_KM_LARGE,
        isSquare: false,
    },
    {
        minutesBeforeLatest: 10,
        widthPx: WIDTH_PX_SMALL,
        widthKm: WIDTH_KM_SMALL,
        isSquare: true,
    },
    {
        minutesBeforeLatest: 20,
        widthPx: WIDTH_PX_LARGE_2,
        widthKm: WIDTH_KM_LARGE,
        isSquare: false,
    },
    {
        minutesBeforeLatest: 20,
        widthPx: WIDTH_PX_SMALL,
        widthKm: WIDTH_KM_SMALL,
        isSquare: true,
    },
    {
        minutesBeforeLatest: 30,
        widthPx: WIDTH_PX_LARGE_2,
        widthKm: WIDTH_KM_LARGE,
        isSquare: false,
    },
    {
        minutesBeforeLatest: 30,
        widthPx: WIDTH_PX_SMALL,
        widthKm: WIDTH_KM_SMALL,
        isSquare: true,
    },
    {
        minutesBeforeLatest: 40,
        widthPx: WIDTH_PX_LARGE_2,
        widthKm: WIDTH_KM_LARGE,
        isSquare: false,
    },
    {
        minutesBeforeLatest: 40,
        widthPx: WIDTH_PX_SMALL,
        widthKm: WIDTH_KM_SMALL,
        isSquare: true,
    },
    {
        minutesBeforeLatest: 50,
        widthPx: WIDTH_PX_LARGE_2,
        widthKm: WIDTH_KM_LARGE,
        isSquare: false,
    },
    {
        minutesBeforeLatest: 50,
        widthPx: WIDTH_PX_SMALL,
        widthKm: WIDTH_KM_SMALL,
        isSquare: true,
    },
    {
        minutesBeforeLatest: 60,
        widthPx: WIDTH_PX_LARGE_2,
        widthKm: WIDTH_KM_LARGE,
        isSquare: false,
    },
    {
        minutesBeforeLatest: 60,
        widthPx: WIDTH_PX_SMALL,
        widthKm: WIDTH_KM_SMALL,
        isSquare: true,
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
