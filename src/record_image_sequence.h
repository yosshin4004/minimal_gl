/* Copyright (C) 2018 Yosshin(@yosshin4004) */


#include "graphics.h"


#ifndef _RECORD_IMAGE_SEQUENCE_H_
#define _RECORD_IMAGE_SEQUENCE_H_


struct RecordImageSequenceSettings {
	int xReso;
	int yReso;
	float startTime;
	float duration;
	float framesPerSecond;
	bool replaceAlphaByOne;
};

/* 連番画像の保存 */
bool RecordImageSequence(
	const char *directoryName,
	const RenderSettings *renderSettings,
	const RecordImageSequenceSettings *recordImageSequenceSettings
);


#endif
