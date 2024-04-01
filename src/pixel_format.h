/* Copyright (C) 2018 Yosshin(@yosshin4004) */


#ifndef _PIXEL_FORMAT_H_
#define _PIXEL_FORMAT_H_


typedef enum {
	PixelFormatUnorm8Rgba,
	PixelFormatFp16Rgba,
	PixelFormatFp32Rgba,
} PixelFormat;
struct GlPixelFormatInfo {
	GLenum internalformat;
	GLenum format;
	GLenum type;
	int numBitsPerPixel;
};

GlPixelFormatInfo
PixelFormatToGlPixelFormatInfo(
	PixelFormat pixelFormat
);


#endif
