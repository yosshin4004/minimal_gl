/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#ifndef _PNG_UTIL_H_
#define _PNG_UTIL_H_


/* raw 画像データを Unorm8 RGBA 形式 png ファイルに保存する */
bool SerializeAsUnorm8RgbaPng(
	const char *fileName,
	const void *data,
	int width,
	int height
);

/* Unorm8 RGBA 形式 png ファイルの読み込み */
bool ReadImageFileAsUnorm8RgbaPng(
	const char *fileName,
	void **dataRet,
	int *numComponentsRet,
	int *widthRet,
	int *heightRet
);

#endif
