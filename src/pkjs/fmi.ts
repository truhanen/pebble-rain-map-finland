// src/pkjs/fmi.ts
import { readTiff } from "./tiff.js";
import { packRadarData } from "./radar_data.js";

const DEBUG: boolean = false;

const BASE_URL: string = "https://openwms.fmi.fi/geoserver/Radar/wms";

const CONSTANT_REQUEST_PARAMS: Record<string, string> = {
    service: "WMS",
    version: "1.3",
    request: "GetMap",
    format: "image/geotiff",
    styles: "raster",
    layers: "Radar:suomi_dbz_eureffin",
    crs: "CRS:84",
};

function getWmsRequestParams(
    coordinateBounds: CoordinateBounds,
    widthPx: number,
    heightPx: number,
    timestamp: Date,
): Record<string, string> {
    const bbox = [
        coordinateBounds.longitudeMin,
        coordinateBounds.latitudeMin,
        coordinateBounds.longitudeMax,
        coordinateBounds.latitudeMax,
    ].join(",");
    const requestParams: Record<string, string> = {
        ...CONSTANT_REQUEST_PARAMS,
    };

    requestParams["bbox"] = bbox;
    requestParams["width"] = String(widthPx);
    requestParams["height"] = String(heightPx);
    requestParams["time"] = timestamp.toISOString();

    return requestParams;
}

function getRadarUrl(
    coordinateBounds: CoordinateBounds,
    widthPx: number,
    heightPx: number,
    timestamp: Date,
): string {
    const requestParams = getWmsRequestParams(
        coordinateBounds,
        widthPx,
        heightPx,
        timestamp,
    );

    const queryString: string[] = [];
    for (const key in requestParams) {
        queryString.push(
            encodeURIComponent(key) +
                "=" +
                encodeURIComponent(String(requestParams[key])),
        );
    }
    const queryParams = queryString.join("&");
    return BASE_URL + "?" + queryParams;
}

export function downloadRadarData(
    coordinateBounds: CoordinateBounds,
    widthPx: number,
    heightPx: number,
    timestamp: Date,
    widthKm: number,
    retryNumber: number,
    successCallback: (radarData: RadarData) => void,
): void {
    const fiveMinutesInMs = 5 * 60 * 1000;

    let radarUrl = getRadarUrl(coordinateBounds, widthPx, heightPx, timestamp);

    if (DEBUG) {
        console.log("downloadRadarData radarUrl:", radarUrl);
    }

    const request = new XMLHttpRequest();
    request.onload = function (this: XMLHttpRequest) {
        if (
            this.responseText.indexOf(
                '<ServiceException code="InvalidDimensionValue"',
            ) !== -1
        ) {
            console.error("InvalidDimensionValue in response");
            retryNumber = retryNumber + 1;
            if (retryNumber <= 4) {
                console.error("Trying with earlier timestamp");
                timestamp = new Date(timestamp.getTime() - fiveMinutesInMs);
                downloadRadarData(
                    coordinateBounds,
                    widthPx,
                    heightPx,
                    timestamp,
                    widthKm,
                    retryNumber,
                    successCallback,
                );
            } else {
                console.error(
                    "Failed to download radar data after several attempts",
                );
            }
        } else {
            const dataView = readTiff(this.response);
            const packedData = packRadarData(dataView);
            const radarData: RadarData = {
                packedData,
                widthPx,
                heightPx,
                timestamp,
                widthKm,
            };
            successCallback(radarData);
        }
    };
    request.responseType = "arraybuffer";
    request.open("GET", radarUrl);
    request.send();
}
