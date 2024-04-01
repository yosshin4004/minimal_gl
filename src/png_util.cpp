/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "external/stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "external/stb/stb_image_write.h"
#include "png_util.h"

bool SerializeAsPng(
	const char *fileName,
	const void *data,
	int numChannels,
	int width,
	int height,
	bool verticalFlip
){
	stbi_flip_vertically_on_write(verticalFlip? 1: 0);
	int ret = stbi_write_png(
		/* char const *filename */	(char const *)fileName,
		/* int w */					width,
		/* int h */					height,
		/* int comp */				numChannels,
		/* const void  *data */		data,
		/* int stride_in_bytes */	width * numChannels
	);

	return ret? true: false;
}

bool ReadImageFileAsPng(
	const char *fileName,
	void **dataRet,
	int *numComponentsRet,
	int *widthRet,
	int *heightRet,
	bool verticalFlip
){
	unsigned char *pixels;
	stbi_set_flip_vertically_on_load(verticalFlip? 1: 0);
	pixels = stbi_load(
		/* char const *filename */	fileName,
		/* int *x */				widthRet,
		/* int *y */				heightRet,
		/* int *comp */				numComponentsRet,
		/* int req_comp */			0
	);
	if (pixels == NULL) return false;
	*dataRet = pixels;
	return true;
}

