// src/pkjs/location.ts
import { DISPLAY_DIMENSIONS } from "./pebble";

const KM_PER_LATITUDE_DEGREE: number = 111.32;

const DEBUG: boolean = false;

const DEFAULT_COORDINATES: Coordinates = {
    // Jyväskylä
    // latitude: 62.2425,
    // longitude: 25.7480,

    // Joensuu
    // latitude: 62.6011,
    // longitude: 29.7636,

    // Rovaniemi
    latitude: 66.5039,
    longitude: 25.7294,
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
    if (DEBUG) {
        successCallback(DEFAULT_COORDINATES);
        return;
    }

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
    const longitudeRangeKm = 400;
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

export function getCoordinateBoundsForWidthKm(
    coordinateBounds400: CoordinateBounds,
    widthKm: number,
): CoordinateBounds {
    let bounds = coordinateBounds400;
    if (widthKm != 400) {
        const latitude =
            (coordinateBounds400.latitudeMin +
                coordinateBounds400.latitudeMax) /
            2;
        const longitude =
            (coordinateBounds400.longitudeMin +
                coordinateBounds400.longitudeMax) /
            2;
        const scaleFactor = 400 / widthKm;
        const latitude_radius_scaled =
            (latitude - coordinateBounds400.latitudeMin) / scaleFactor;
        const longitude_radius_scaled =
            (longitude - coordinateBounds400.longitudeMin) /
            scaleFactor;
        bounds = {
            longitudeMin: longitude - longitude_radius_scaled,
            longitudeMax: longitude + longitude_radius_scaled,
            latitudeMin: latitude - latitude_radius_scaled,
            latitudeMax: latitude + latitude_radius_scaled,
        };
    }
    return bounds;
}
