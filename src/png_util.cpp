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

bool SerializeAsUnorm8RgbaPng(
	const char *fileName,
	const void *data,
	int width,
	int height
){
	int numChannel = 4;
	stbi_flip_vertically_on_write(1);
	int ret = stbi_write_png(
		/* char const *filename */	(char const *)fileName,
		/* int w */					width,
		/* int h */					height,
		/* int comp */				numChannel,
		/* const void  *data */		data,
		/* int stride_in_bytes */	width * numChannel
	);

	return ret? true: false;
}

bool ReadImageFileAsUnorm8RgbaPng(
	const char *fileName,
	void **dataRet,
	int *numComponentsRet,
	int *widthRet,
	int *heightRet
){
	unsigned char *pixels;
	stbi_set_flip_vertically_on_load(1);
	pixels = stbi_load(fileName, widthRet, heightRet, numComponentsRet, 0);
	if (pixels == NULL) return false;
	*dataRet = pixels;
	return true;
}

