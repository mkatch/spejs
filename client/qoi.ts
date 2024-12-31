/**
 * Decode a QOI file given as an ArrayBuffer.
 *
 * Ported with small adjustmensd from https://github.com/kchapelier/qoijs. 
 **/
export function qoiDecode(inputBuffer: ArrayBuffer, params?: {
	byteOffset?: number,
	byteLength?: number,
	outputChannels?: 3 | 4,
	outputBuffer?: ArrayBuffer,
}): {
	width: number,
	height: number,
	colorspace: 0 | 1,
	channels: 3 | 4,
	data: ArrayBuffer
} {
	const byteOffset = params?.byteOffset ?? 0
	const byteLength = params?.byteLength ?? inputBuffer.byteLength - byteOffset
	const input = new Uint8Array(inputBuffer, byteOffset, byteLength)

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

	const outputChannels = params?.outputChannels ?? channels
	const pixelLength = width * height * outputChannels
	const output = params?.outputBuffer ? new Uint8Array(params.outputBuffer) : new Uint8Array(pixelLength)

	if (output.length < pixelLength) {
		throw new Error(`Output buffer is too small. Expected at least ${pixelLength} bytes, available: ${output.length}.`);
	}

	const index = new Uint8Array(64 * 4)
	const chunksLength = byteLength - 8;
	let arrayPosition = 14
	let pixelPosition = 0;
	let red = 0, green = 0, blue = 0, alpha = 255
	let run = 0;

	for (; pixelPosition < pixelLength && arrayPosition < byteLength - 4; pixelPosition += outputChannels) {
		if (run > 0) {
			run--
		} else if (arrayPosition < chunksLength) {
			const byte1 = input[arrayPosition++]

			if (byte1 === 0b11111110) { // QOI_OP_RGB
				red = input[arrayPosition++]
				green = input[arrayPosition++]
				blue = input[arrayPosition++]
			} else if (byte1 === 0b11111111) { // QOI_OP_RGBA
				red = input[arrayPosition++]
				green = input[arrayPosition++]
				blue = input[arrayPosition++]
				alpha = input[arrayPosition++]
			} else if ((byte1 & 0b11000000) === 0b00000000) { // QOI_OP_INDEX
				red = index[byte1 * 4]
				green = index[byte1 * 4 + 1]
				blue = index[byte1 * 4 + 2]
				alpha = index[byte1 * 4 + 3]
			} else if ((byte1 & 0b11000000) === 0b01000000) { // QOI_OP_DIFF
				red += ((byte1 >> 4) & 0b00000011) - 2
				green += ((byte1 >> 2) & 0b00000011) - 2
				blue += (byte1 & 0b00000011) - 2

				// handle wraparound
				red = (red + 256) % 256
				green = (green + 256) % 256
				blue = (blue + 256) % 256
			} else if ((byte1 & 0b11000000) === 0b10000000) { // QOI_OP_LUMA
				const byte2 = input[arrayPosition++]
				const greenDiff = (byte1 & 0b00111111) - 32
				const redDiff = greenDiff + ((byte2 >> 4) & 0b00001111) - 8
				const blueDiff = greenDiff + (byte2 & 0b00001111) - 8

				// handle wraparound
				red = (red + redDiff + 256) % 256
				green = (green + greenDiff + 256) % 256
				blue = (blue + blueDiff + 256) % 256
			} else if ((byte1 & 0b11000000) === 0b11000000) { // QOI_OP_RUN
				run = byte1 & 0b00111111;
			}

			const indexPosition = ((red * 3 + green * 5 + blue * 7 + alpha * 11) % 64) * 4
			index[indexPosition] = red
			index[indexPosition + 1] = green
			index[indexPosition + 2] = blue
			index[indexPosition + 3] = alpha
		}

		if (outputChannels === 4) { // RGBA
			output[pixelPosition] = red
			output[pixelPosition + 1] = green
			output[pixelPosition + 2] = blue
			output[pixelPosition + 3] = alpha
		} else { // RGB
			output[pixelPosition] = red
			output[pixelPosition + 1] = green
			output[pixelPosition + 2] = blue
		}
	}

	if (pixelPosition < pixelLength) {
		throw new Error('Incomplete image');
	}

	// checking the 00000001 padding is not required, as per specs

	new ImageData(new Uint8ClampedArray(output.buffer), width, height)

	return {
		width,
		height,
		colorspace,
		channels: outputChannels,
		data: output
	};
}