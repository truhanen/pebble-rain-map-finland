import { DISPLAY_DIMENSIONS } from "./pebble";

const PIXEL_WIDTH_1 = DISPLAY_DIMENSIONS.width;
const PIXEL_WIDTH_2 = DISPLAY_DIMENSIONS.width / 2;
const PIXEL_WIDTH_3 = DISPLAY_DIMENSIONS.width / 4;

export const RADAR_DATA_SPECS: RadarDataSpec[] = [
    {
        minutesBeforeLatest: 0,
        width: PIXEL_WIDTH_2,
        zoomLevel: MapZoomLevel.Far,
    },
    {
        minutesBeforeLatest: 0,
        width: PIXEL_WIDTH_1,
        zoomLevel: MapZoomLevel.Close,
    },
    {
        minutesBeforeLatest: 10,
        width: PIXEL_WIDTH_2,
        zoomLevel: MapZoomLevel.Far,
    },
    {
        minutesBeforeLatest: 10,
        width: PIXEL_WIDTH_1,
        zoomLevel: MapZoomLevel.Close,
    },
    {
        minutesBeforeLatest: 20,
        width: PIXEL_WIDTH_2,
        zoomLevel: MapZoomLevel.Far,
    },
    {
        minutesBeforeLatest: 20,
        width: PIXEL_WIDTH_2,
        zoomLevel: MapZoomLevel.Close,
    },
    {
        minutesBeforeLatest: 30,
        width: PIXEL_WIDTH_3,
        zoomLevel: MapZoomLevel.Far,
    },
    {
        minutesBeforeLatest: 30,
        width: PIXEL_WIDTH_2,
        zoomLevel: MapZoomLevel.Close,
    },
    {
        minutesBeforeLatest: 40,
        width: PIXEL_WIDTH_3,
        zoomLevel: MapZoomLevel.Far,
    },
    {
        minutesBeforeLatest: 40,
        width: PIXEL_WIDTH_3,
        zoomLevel: MapZoomLevel.Close,
    },
    {
        minutesBeforeLatest: 50,
        width: PIXEL_WIDTH_3,
        zoomLevel: MapZoomLevel.Far,
    },
    {
        minutesBeforeLatest: 50,
        width: PIXEL_WIDTH_3,
        zoomLevel: MapZoomLevel.Close,
    },
    {
        minutesBeforeLatest: 60,
        width: PIXEL_WIDTH_3,
        zoomLevel: MapZoomLevel.Far,
    },
    {
        minutesBeforeLatest: 60,
        width: PIXEL_WIDTH_3,
        zoomLevel: MapZoomLevel.Close,
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
