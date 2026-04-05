// src/pkjs/location.ts
import { DISPLAY_DIMENSIONS } from "./pebble";

const KM_PER_LATITUDE_DEGREE: number = 111.32;

const DEFAULT_COORDINATES: Coordinates = {
    latitude: 62.24247205261152,
    longitude: 25.748024833006454,
};

const OPTIONS: PositionOptions = {
    enableHighAccuracy: false,
    maximumAge: 10000,
    timeout: 10000,
};

function calcLatitudeRange(kilometers: number): number {
    return kilometers / KM_PER_LATITUDE_DEGREE;
}

function calcLongitudeRange(kilometers: number, latitude: number): number {
    const kmPerLongitudeDegree =
        KM_PER_LATITUDE_DEGREE * Math.cos((latitude * Math.PI) / 180);
    return kilometers / kmPerLongitudeDegree;
}

export function calcCoordinateBounds(
    coordinates: Coordinates,
    latitudeRangeKm: number,
    longitudeRangeKm: number,
): CoordinateBounds {
    const latitudeRange = calcLatitudeRange(latitudeRangeKm);
    const longitudeRange = calcLongitudeRange(
        longitudeRangeKm,
        coordinates.latitude,
    );

    return {
        longitudeMin: coordinates.longitude - longitudeRange / 2,
        longitudeMax: coordinates.longitude + longitudeRange / 2,
        latitudeMin: coordinates.latitude - latitudeRange / 2,
        latitudeMax: coordinates.latitude + latitudeRange / 2,
    };
}

function getLocation(
    successCallback: (coordinates: Coordinates) => void,
): void {
    if (typeof navigator === "undefined" || !navigator.geolocation) {
        console.log("Geolocation not available, using default location");
        successCallback(DEFAULT_COORDINATES);
    } else {
        navigator.geolocation.getCurrentPosition(
            (position: GeolocationPosition): void => {
                successCallback({
                    latitude: position.coords.latitude,
                    longitude: position.coords.longitude,
                });
            },
            (e: any) => {
                // throw Error(`Location error: ${e}`);
                console.log(`Location error: ${e}`);
                successCallback(DEFAULT_COORDINATES);
            },
            OPTIONS,
        );
    }
}

export function getCoordinateBounds(
    successCallback: (bounds: CoordinateBounds) => void,
) {
    const longitudeRangeKm = MapZoomLevel.Far;
    const latitudeRangeKm =
        (longitudeRangeKm * DISPLAY_DIMENSIONS.height) /
        DISPLAY_DIMENSIONS.width;
    getLocation((coordinates) => {
        const bounds = calcCoordinateBounds(
            coordinates,
            latitudeRangeKm,
            longitudeRangeKm,
        );
        successCallback(bounds);
    });
}

export function getCoordinateBoundsForZoomLevel(
    coordinateBoundsFar: CoordinateBounds,
    zoomLevel: MapZoomLevel,
): CoordinateBounds {
    let bounds = coordinateBoundsFar;
    if (zoomLevel == MapZoomLevel.Close) {
        const latitude =
            (coordinateBoundsFar.latitudeMin +
                coordinateBoundsFar.latitudeMax) /
            2;
        const longitude =
            (coordinateBoundsFar.longitudeMin +
                coordinateBoundsFar.longitudeMax) /
            2;
        const zoomLevelCloseFactor = MapZoomLevel.Far / MapZoomLevel.Close;
        const latitude_radius_close =
            (latitude - coordinateBoundsFar.latitudeMin) / zoomLevelCloseFactor;
        const longitude_radius_close =
            (longitude - coordinateBoundsFar.longitudeMin) /
            zoomLevelCloseFactor;
        bounds = {
            longitudeMin: longitude - longitude_radius_close,
            longitudeMax: longitude + longitude_radius_close,
            latitudeMin: latitude - latitude_radius_close,
            latitudeMax: latitude + latitude_radius_close,
        };
    }
    return bounds;
}
