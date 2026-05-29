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

interface RadarDataSpec {
    minutesBeforeLatest: number;
    widthPx: number;
    widthKm: number;
}

interface RadarData {
    packedData: Uint8Array;
    widthPx: number;
    heightPx: number;
    timestamp: Date;
    widthKm: number;
}
