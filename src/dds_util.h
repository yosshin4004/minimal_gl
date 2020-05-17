/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#ifndef _DDS_UTIL_H_
#define _DDS_UTIL_H_


/* raw 画像データを FP32RGBA 形式のキューブマップとして dds ファイルに保存する */
bool SerializeAsFp32RgbaCubemapDds(
	const char *fileName,
	float *(data[6]),
	int width,
	int height
);


#endif
