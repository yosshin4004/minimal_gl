/* Copyright (C) 2026 Yosshin(@yosshin4004) */


#include "external/cJSON/cJSON.h"
#include "external/cJSON/cJSON_Utils.h"


#ifndef _JSON_UTIL_H_
#define _JSON_UTIL_H_


/* 指定パスの値を文字列として読み取る */
bool JsonGetAsString(
	cJSON *json,
	const char *pointer,
	char *dstString,
	size_t dstStringSizeInBytes,
	const char *defaultString
);

/* 指定パスの値を int として読み取る */
bool JsonGetAsInt(
	cJSON *json,
	const char *pointer,
	int *dst,
	int defaultValue
);

/* 指定パスの値を float として読み取る */
bool JsonGetAsFloat(
	cJSON *json,
	const char *pointer,
	float *dst,
	float defaultValue
);

/* 指定パスの値を bool として読み取る */
bool JsonGetAsBool(
	cJSON *json,
	const char *pointer,
	bool *dst,
	bool defaultValue
);

/* 指定パスの値を vec3 として読み取る */
bool JsonGetAsVec3(
	cJSON *json,
	const char *pointer,
	float vec3Dst[3],
	const float vec3Default[3]
);

/* 指定パスの値を vec4 として読み取る */
bool JsonGetAsVec4(
	cJSON *json,
	const char *pointer,
	float vec4Dst[4],
	const float vec4Default[4]
);


#endif
