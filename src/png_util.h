/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#ifndef _PNG_UTIL_H_
#define _PNG_UTIL_H_


/* raw 画像データを png ファイルに保存する */
bool SerializeAsPng(
	const char *fileName,
	const void *data,
	int numChannels,
	int width,
	int height,
	bool verticalFlip
);

/* png ファイルの読み込み */
bool ReadImageFileAsPng(
	const char *fileName,
	void **dataRet,
	int *numComponentsRet,
	int *widthRet,
	int *heightRet,
	bool verticalFlip
);

#endif
