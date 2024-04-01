/* Copyright (C) 2018 Yosshin(@yosshin4004) */


#include "dds_parser.h"
#include "pixel_format.h"


#ifndef _DDS_UTIL_H_
#define _DDS_UTIL_H_


/* PixelFormat から DxgiFormat に変換 */
DxgiFormat PixelFormatToDxgiFormat(
	PixelFormat pixelFormat
);

/* DxgiFormat から OpenGL のピクセルフォーマット情報に変換 */
GlPixelFormatInfo DxgiFormatToGlPixelFormatInfo(
	DxgiFormat dxgiFormat
);

/* raw 画像データを 2D テクスチャとして dds ファイルに保存する */
bool SerializeAsDdsTexture2d(
	const char *fileName,
	DxgiFormat dxgiFormat,
	const void *data,
	int width,
	int height,
	bool verticalFlip
);

/* raw 画像データをキューブマップとして dds ファイルに保存する */
bool SerializeAsDdsCubemap(
	const char *fileName,
	DxgiFormat dxgiFormat,
	const void *(data[6]),
	int reso,
	bool verticalFlip
);


#endif
