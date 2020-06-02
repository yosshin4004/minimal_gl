/* Copyright (C) 2018 Yosshin(@yosshin4004) */


#include "dds_parser.h"


#ifndef _DDS_UTIL_H_
#define _DDS_UTIL_H_


/* DxgiFormat から OpenGL のフォーマット情報に変換 */
struct DdsGlFormat {
	GLenum internalformat;
	GLenum format;
	GLenum type;
};
DdsGlFormat
DdsDxgiFormatToGlFormat(
	DxgiFormat format
);

/* raw 画像データを FP32RGBA 形式のキューブマップとして dds ファイルに保存する */
bool SerializeAsFp32RgbaCubemapDds(
	const char *fileName,
	float *(data[6]),
	int width,
	int height
);


#endif
