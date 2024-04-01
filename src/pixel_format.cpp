/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#include "common.h"
#include "pixel_format.h"

static const GlPixelFormatInfo s_tblPixelFormatToGlPixelFormatInfo[] = {
	/* PixelFormatUnorm8Rgba */	{GL_RGBA8,		GL_RGBA,	GL_UNSIGNED_BYTE,	32},
	/* PixelFormatFp16Rgba */	{GL_RGBA16F,	GL_RGBA,	GL_HALF_FLOAT,		64},
	/* PixelFormatFp32Rgba */	{GL_RGBA32F,	GL_RGBA,	GL_FLOAT,			128},
};

GlPixelFormatInfo PixelFormatToGlPixelFormatInfo(
	PixelFormat pixelFormat
){
	assert(pixelFormat < SIZE_OF_ARRAY(s_tblPixelFormatToGlPixelFormatInfo));
	return s_tblPixelFormatToGlPixelFormatInfo[pixelFormat];
}
