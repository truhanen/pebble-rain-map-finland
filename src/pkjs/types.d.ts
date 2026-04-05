declare const Pebble: {
    sendAppMessage(
        message: Record<string, any>,
        success?: (e?: any) => void,
        failure?: (e?: any) => void,
    ): void;
    addEventListener(event: string, cb: (e: any) => void): void;
    getActiveWatchInfo();
};

interface DisplayDimensions {
    width: number;
    height: number;
}

interface Coordinates {
    latitude: number;
    longitude: number;
}

interface CoordinateBounds {
    longitudeMin: number;
    longitudeMax: number;
    latitudeMin: number;
    latitudeMax: number;
}

// Values are widths in kilometers
const enum MapZoomLevel {
    Close = 200,
    Far = 400,
}

interface RadarDataSpec {
    minutesBeforeLatest: number;
    width: number;
    zoomLevel: MapZoomLevel;
}

interface RadarData {
    dataView: DataView;
    width: number;
    height: number;
    timestamp: Date;
    zoomLevel: MapZoomLevel;
}
