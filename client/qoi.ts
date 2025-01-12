export type QoiHeader = {
	width: number,
	height: number,
	colorspace: 0 | 1,
	channels: 3 | 4,
}

export type QoiFile = QoiHeader & {
	data: ArrayBuffer
}

export function decodeQoiHeader(data: ArrayBuffer): QoiHeader {
	const input = new Uint8Array(data)

	if (input[0] !== 0x71 || input[1] !== 0x6F || input[2] !== 0x69 || input[3] !== 0x66) {
		throw new Error('Invalid magic number.');
	}

	const width = ((input[4] << 24) | (input[5] << 16) | (input[6] << 8) | input[7]) >>> 0;
	const height = ((input[8] << 24) | (input[9] << 16) | (input[10] << 8) | input[11]) >>> 0;
	const channels = input[12]
	const colorspace = input[13]

	// The maximum image size is arbitrary. Can be enlarged if needed.
	if (width < 0 || height < 0 || width * height > 4096 ** 2) {
		throw new Error(`Invalid image dimension: ${width} x ${height}.`);
	}
	if (channels != 3 && channels != 4) {
		throw new Error(`Invalid input channels. Expected 3 or 4, got ${channels}.`)
	}
	if (colorspace != 0 && colorspace != 1) {
		throw new Error(`Invald input colorspace. Expected 0 or 1, got ${colorspace}`)
	}

	return { width, height, colorspace, channels }
}

/**
 * Decode a QOI file given as an ArrayBuffer.
 *
 * Ported with small adjustmensd from https://github.com/kchapelier/qoijs. 
 **/
export function decodeQoi(data: ArrayBuffer, params?: {
	outChannels?: 3 | 4,
	outBuffer?: ArrayBuffer,
	flipX?: boolean,
	flipY?: boolean,
}): QoiFile {
	const header = decodeQoiHeader(data)
	const { width, height, channels } = header
	const inData = new Uint8Array(data)

	const { outChannels = channels, flipX = false, flipY = false } = params ?? {}
	const outLength = width * height * outChannels
	const outData = params?.outBuffer ? new Uint8Array(params.outBuffer) : new Uint8Array(outLength)

	if (outData.length < outLength) {
		throw new Error(`Output buffer is too small. Expected at least ${outLength} bytes, available: ${outData.length}.`);
	}

	const outStep = flipX ? -outChannels : outChannels
	const topDownOutRowStartStep = width * outChannels
	const topDownRowStartStart = flipX ? topDownOutRowStartStep + outStep : 0
	const outRowStartStep = flipY ? -topDownOutRowStartStep : topDownOutRowStartStep
	const outRowStartStart = flipY ? outLength - outRowStartStep + topDownRowStartStart : topDownRowStartStart
	const outRowEndOffset = width * outStep
	const outRowStartEnd = outRowStartStart + height * outRowStartStep
	const outAlphaOffset = outChannels - 1 // Trick to write alpha unconditionally

	const pallette = new Uint8Array(64 * 4)
	let red = 0, green = 0, blue = 0, alpha = 255
	let run = 0
	let inIndex = 14, inEnd = inData.length
	let outIndex = 0, decodedCount = 0
	for (let outRowStart = outRowStartStart; outRowStart !== outRowStartEnd && inIndex < inEnd; outRowStart += outRowStartStep) {
		const outRowEnd = outRowStart + outRowEndOffset
		for (outIndex = outRowStart; outIndex !== outRowEnd && inIndex < inEnd; outIndex += outStep) {
			if (run > 0) {
				--run
			} else {
				const byte1 = inData[inIndex++]
				if (byte1 === 0b11111110) { // QOI_OP_RGB
					red = inData[inIndex++]
					green = inData[inIndex++]
					blue = inData[inIndex++]
				} else if (byte1 === 0b11111111) { // QOI_OP_RGBA
					red = inData[inIndex++]
					green = inData[inIndex++]
					blue = inData[inIndex++]
					alpha = inData[inIndex++]
				} else if ((byte1 & 0b11000000) === 0b00000000) { // QOI_OP_INDEX
					const palletteOffset = byte1 * 4
					red = pallette[palletteOffset]
					green = pallette[palletteOffset + 1]
					blue = pallette[palletteOffset + 2]
					alpha = pallette[palletteOffset + 3]
				} else if ((byte1 & 0b11000000) === 0b01000000) { // QOI_OP_DIFF
					red += ((byte1 >> 4) & 0b00000011) - 2
					green += ((byte1 >> 2) & 0b00000011) - 2
					blue += (byte1 & 0b00000011) - 2
					red = (red + 256) % 256
					green = (green + 256) % 256
					blue = (blue + 256) % 256
				} else if ((byte1 & 0b11000000) === 0b10000000) { // QOI_OP_LUMA
					const byte2 = inData[inIndex++]
					const greenDiff = (byte1 & 0b00111111) - 32
					const redDiff = greenDiff + ((byte2 >> 4) & 0b00001111) - 8
					const blueDiff = greenDiff + (byte2 & 0b00001111) - 8
					red = (red + redDiff + 256) % 256
					green = (green + greenDiff + 256) % 256
					blue = (blue + blueDiff + 256) % 256
				} else if ((byte1 & 0b11000000) === 0b11000000) { // QOI_OP_RUN
					run = byte1 & 0b00111111;
				} else {
					inIndex = inEnd
					break;
				}

				const palleteOffset = ((red * 3 + green * 5 + blue * 7 + alpha * 11) % 64) * 4
				pallette[palleteOffset] = red
				pallette[palleteOffset + 1] = green
				pallette[palleteOffset + 2] = blue
				pallette[palleteOffset + 3] = alpha
			}

			outData[outIndex + outAlphaOffset] = alpha
			outData[outIndex] = red
			outData[outIndex + 1] = green
			outData[outIndex + 2] = blue
			++decodedCount
		}
	}

	if (decodedCount != width * height) {
		throw new Error(`Malformed image. Decoded ${decodedCount} pixels, out of expected ${width * height}.`)
	}

	return { ...header, channels: outChannels, data: outData }
}

