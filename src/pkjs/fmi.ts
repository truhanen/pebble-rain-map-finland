// src/pkjs/fmi.ts
import { readTiff } from "./tiff.js";

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
    width: number,
    height: number,
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
    requestParams["width"] = String(width);
    requestParams["height"] = String(height);
    requestParams["time"] = timestamp.toISOString();

    return requestParams;
}

function getRadarUrl(
    coordinateBounds: CoordinateBounds,
    width: number,
    height: number,
    timestamp: Date,
): string {
    const requestParams = getWmsRequestParams(
        coordinateBounds,
        width,
        height,
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
    width: number,
    height: number,
    timestamp: Date,
    zoomLevel: MapZoomLevel,
    retryNumber: number,
    successCallback: (radarData: RadarData) => void,
): void {
    const fiveMinutesInMs = 5 * 60 * 1000;

    let radarUrl = getRadarUrl(coordinateBounds, width, height, timestamp);

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
                    width,
                    height,
                    timestamp,
                    zoomLevel,
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
            const radarData: RadarData = {
                dataView,
                width,
                height,
                timestamp,
                zoomLevel,
            };
            successCallback(radarData);
        }
    };
    request.responseType = "arraybuffer";
    request.open("GET", radarUrl);
    request.send();
}
