/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "wav_util.h"

typedef struct {
	uint8_t		chunkName[4];
	uint32_t	chunkSize;
} WavChunkHeader;
typedef struct {
	uint8_t		riff[4];
	uint32_t	fileSizeMinus8;
	uint8_t		wave[4];
} WavHeader;
typedef struct {
	uint16_t	formatID;
	uint16_t	channelCount;
	uint32_t	numSamplesPerSec;
	uint32_t	bytesPerSec;
	uint16_t	bytesPerSample;
	uint16_t	bitsPerSample;
} WavFormatChunk;
typedef struct {
	WavHeader		wavHeader;
	WavChunkHeader	formatChunkHeader;
	WavFormatChunk	formatChunk;
	WavChunkHeader	dataChunkHeader;
} Header;

bool SerializeAsWav(
	const char *fileName,
	const void *buffer,
	int numChannels,
	int numSamples,
	int numSamplesPerSec,
	uint16_t formatID,
	int	bitsPerSampleComponent
){
	FILE *file = fopen(fileName, "wb");
	if (file == NULL) return false;

	int	bytesPerSampleComponent = bitsPerSampleComponent >> 3;
	int	dataChunkSizeInBytes = bytesPerSampleComponent * numChannels * numSamples;

	Header header = {
		{
			{'R' , 'I', 'F', 'F'},
			(uint32_t)(sizeof(header) + dataChunkSizeInBytes - 8),
			{'W', 'A', 'V', 'E'}
		},{
			{'f', 'm', 't', ' '},
			(uint32_t)sizeof(WavFormatChunk)
		},{
			formatID,
			(uint16_t)numChannels,
			(uint32_t)numSamplesPerSec,
			(uint32_t)(bytesPerSampleComponent * numChannels * numSamplesPerSec),
			(uint16_t)(bytesPerSampleComponent * numChannels),
			(uint16_t)bitsPerSampleComponent
		},{
			{'d', 'a', 't', 'a'},
			(uint32_t)dataChunkSizeInBytes
		}
	};

	fwrite(&header, 1, sizeof(header), file);
	fwrite(buffer, 1, bytesPerSampleComponent * numChannels * numSamples, file);
	fclose(file);
	return true;
}



