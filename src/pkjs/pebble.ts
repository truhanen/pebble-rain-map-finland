export function getDisplayDimensions(): DisplayDimensions {
    const watchInfo = Pebble.getActiveWatchInfo();
    let width: number = 0;
    let height: number = 0;
    if (watchInfo.platform == "flint") {
        width = 144;
        height = 168;
    } else if (watchInfo.platform == "emery") {
        width = 200;
        height = 228;
    } else if (watchInfo.platform == "gabbro") {
        width = 260;
        height = 260;
    } else {
        throw Error("Unknown Pebble platform");
    }
    return { width, height };
}

export const DISPLAY_DIMENSIONS = getDisplayDimensions();
