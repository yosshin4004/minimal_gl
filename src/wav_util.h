/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#ifndef _WAV_UTIL_H_
#define _WAV_UTIL_H_


#include <stdint.h>

/* 生波形データを wav ファイルに保存する */
bool SerializeAsWav(
	const char *fileName,
	const void *buffer,
	int numChannels,
	int numSamples,
	int numSamplesPerSec,
	uint16_t formatID,
	int	bitsPerSampleComponent
);


#endif
