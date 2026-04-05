export function readTiff(arrayBuffer: ArrayBuffer) {
    const dataView = new DataView(arrayBuffer);
    const littleEndian = dataView.getUint16(0, false) === 0x4949; // Check endianness

    function getUint16(offset: number) {
        return dataView.getUint16(offset, littleEndian);
    }

    function getUint32(offset: number) {
        return dataView.getUint32(offset, littleEndian);
    }

    // TIFF header starts at byte 0
    let offset = getUint32(4); // Offset to first IFD (Image File Directory)

    // Read IFD
    const numEntries = getUint16(offset);
    offset += 2;

    let width: number | undefined;
    let height: number | undefined;
    let bitsPerSample: number | undefined;
    let stripOffsets: number | undefined;
    // let stripByteCounts: number | undefined

    for (let i = 0; i < numEntries; i++) {
        const tag = getUint16(offset);
        const type = getUint16(offset + 2);
        const count = getUint32(offset + 4);
        let valueOffset = offset + 8;

        if (type === 3 && count === 1) {
            // SHORT type
            valueOffset = offset + 8;
        } else if (type === 4 && count === 1) {
            // LONG type
            valueOffset = offset + 8;
        } else {
            valueOffset = getUint32(offset + 8);
        }

        switch (tag) {
            case 256: // ImageWidth
                width = getUint16(valueOffset);
                break;
            case 257: // ImageLength
                height = getUint16(valueOffset);
                break;
            case 258: // BitsPerSample
                bitsPerSample = getUint16(valueOffset);
                break;
            case 273: // StripOffsets
                stripOffsets = getUint32(valueOffset);
                break;
            // case 279: // StripByteCounts
            //     stripByteCounts = getUint32(valueOffset);
            //     break;
        }

        offset += 12;
    }

    if (
        width == null ||
        height == null ||
        bitsPerSample == null ||
        stripOffsets == null
    ) {
        throw new Error(
            "Missing required TIFF tags: width, height, bitsPerSample, or stripOffsets",
        );
    }

    const bytesPerPixel = bitsPerSample / 8;
    const pixelCount = width * height;
    const pixelDataOffset = stripOffsets;

    if (bytesPerPixel !== 1) {
        throw new Error("TIFF not 8-bit");
    }

    return new DataView(arrayBuffer, pixelDataOffset, pixelCount);
}
