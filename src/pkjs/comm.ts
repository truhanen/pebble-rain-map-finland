import { downloadRadarData } from "./fmi";
import {
    getCoordinateBounds,
    getCoordinateBoundsForWidthKm,
} from "./coordinates";
import { getRadarDataSpec, RADAR_DATA_SPECS } from "./radar_data_specs";
import { DISPLAY_DIMENSIONS } from "./pebble";

const DEBUG: boolean = false;

// From app_message_inbox_size_maximum(). Note that also other data, though
// smaller than the chunk itself may be transmitted in one message.
const MAX_CHUNK_SIZE: number = 8000;

function transmitCoordinateBounds(
    successCallback: (bounds: CoordinateBounds) => void,
) {
    getCoordinateBounds((bounds) => {
        const dict: Record<string, number> = {
            COORDINATES: 0,
            LONGITUDE_MIN: Math.round(bounds.longitudeMin * 1000),
            LATITUDE_MIN: Math.round(bounds.latitudeMin * 1000),
            LONGITUDE_MAX: Math.round(bounds.longitudeMax * 1000),
            LATITUDE_MAX: Math.round(bounds.latitudeMax * 1000),
        };
        Pebble.sendAppMessage(
            dict,
            () => {
                if (DEBUG) {
                    console.log("transmitCoordinateBounds succeeded");
                }
                successCallback(bounds);
            },
            () => {
                console.log("transmitCoordinateBounds failed");
            },
        );
    });
}

function transmitRadarDataSpecs(
    specIndex: number,
    coordinateBoundsFar: CoordinateBounds,
    latestTimestamp: Date | null,
    successCallback: (radarData: RadarData) => void,
) {
    const spec = getRadarDataSpec(specIndex);
    const coordinateBounds = getCoordinateBoundsForWidthKm(
        coordinateBoundsFar,
        spec.widthKm,
    );
    var heightPx = (spec.widthPx * DISPLAY_DIMENSIONS.height) / DISPLAY_DIMENSIONS.width;
    const minuteInMs = 60 * 1000;
    if (latestTimestamp == null) {
        latestTimestamp = new Date();
        // Round to the closest 5 minutes
        latestTimestamp = new Date(
            Math.round(latestTimestamp.getTime() / (5 * minuteInMs)) *
                (5 * minuteInMs),
        );
    }
    const timestamp = new Date(
        latestTimestamp.getTime() - spec.minutesBeforeLatest * minuteInMs,
    );
    downloadRadarData(
        coordinateBounds,
        spec.widthPx,
        heightPx,
        timestamp,
        spec.widthKm,
        0,
        (radarData) => {
            const specMessage: Record<string, any> = {
                RADAR_DATA_SPEC: 0,
                TIMESTAMP: radarData.timestamp.getTime() / 1000,
                WIDTH_PX: radarData.widthPx,
                HEIGHT_PX: radarData.heightPx,
                WIDTH_KM: radarData.widthKm,
            };
            Pebble.sendAppMessage(
                specMessage,
                () => {
                    if (DEBUG) {
                        console.log("transmitRadarDataSpecs succeeded");
                    }
                    successCallback(radarData);
                },
                () => {
                    console.log("transmitRadarDataSpecs failed");
                },
            );
        },
    );
}

interface RadarDataChunk {
    data: number[];
    size: number;
}

function getRadarDataChunk(
    radarData: DataView,
    chunkStartIndex: number,
): RadarDataChunk {
    let chunkSize: number;
    if (radarData.byteLength - chunkStartIndex < MAX_CHUNK_SIZE) {
        chunkSize = radarData.byteLength - chunkStartIndex;
    } else {
        chunkSize = MAX_CHUNK_SIZE;
    }
    const dataChunk: number[] = [];
    for (let i = 0; i < chunkSize; i++) {
        dataChunk.push(radarData.getUint8(chunkStartIndex + i));
    }
    return {
        data: dataChunk,
        size: chunkSize,
    };
}

function transmitRadarDataComplete(
    successCallback: () => void,
): void {
    Pebble.sendAppMessage(
        { RADAR_DATA_TRANSMIT_COMPLETE: 0 },
        () => {
            if (DEBUG) {
                console.log("transmitRadarDataComplete succeeded");
            }
            successCallback();
        },
        () => {
            console.log("transmitRadarDataComplete failed");
        },
    );
}

function transmitRadarData(
    specIndex: number,
    coordinateBoundsFar: CoordinateBounds,
    radarData: RadarData | null,
    chunkStartIndex: number | null,
    latestTimestamp: Date | null,
): void {
    if (radarData == null || chunkStartIndex == null) {
        transmitRadarDataSpecs(
            specIndex,
            coordinateBoundsFar,
            latestTimestamp,
            (radarData: RadarData) => {
                if (latestTimestamp == null) {
                    latestTimestamp = radarData.timestamp;
                }
                transmitRadarData(
                    specIndex,
                    coordinateBoundsFar,
                    radarData,
                    0,
                    latestTimestamp,
                );
            },
        );
        return;
    }

    const chunk: RadarDataChunk = getRadarDataChunk(
        radarData.dataView,
        chunkStartIndex,
    );
    const chunkMessage: Record<string, any> = {
        RADAR_DATA_CHUNK: chunk.data,
        RADAR_DATA_CHUNK_SIZE: chunk.size,
        RADAR_DATA_CHUNK_START_INDEX: chunkStartIndex,
    };
    Pebble.sendAppMessage(
        chunkMessage,
        () => {
            if (DEBUG) {
                console.log("transmitRadarData succeeded");
            }
            if (chunkStartIndex == null) {
                return;
            }
            chunkStartIndex += chunk.size;
            if (chunkStartIndex < radarData.dataView.byteLength) {
                transmitRadarData(
                    specIndex,
                    coordinateBoundsFar,
                    radarData,
                    chunkStartIndex,
                    latestTimestamp,
                );
            } else {
                transmitRadarDataComplete(
                    () => {
                        if (specIndex < RADAR_DATA_SPECS.length - 1) {
                            transmitRadarData(
                                specIndex + 1,
                                coordinateBoundsFar,
                                null,
                                null,
                                latestTimestamp,
                            );
                        }
                    },
                );
            }
        },
        () => {
            console.log("transmitRadarData failed");
        },
    );
}

function transmitData(): void {
    transmitCoordinateBounds((coordinateBoundsFar) => {
        transmitRadarData(0, coordinateBoundsFar, null, null, null);
    });
}

export function setupComm(): void {
    Pebble.addEventListener("ready", () => {
        Pebble.sendAppMessage(
            {JS_READY: 1 },
            () => {
                if (DEBUG) {
                    console.log("JS_READY message succeeded");
                }
            },
            () => {
                console.log("JS_READY message failed");
            }
        );
    });

    Pebble.addEventListener("appmessage", (e: any) => {
        const dict: Record<string, any> = e.payload;
        if ("TRANSMIT_DATA" in dict) {
            transmitData();
        }
    });
}
