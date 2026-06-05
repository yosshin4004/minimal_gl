/* Copyright (C) 2026 Yosshin(@yosshin4004) */

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "common.h"
#include "config.h"
#include "json_util.h"


bool JsonGetAsString(
	cJSON *json,
	const char *pointer,
	char *dstString,
	size_t dstStringSizeInBytes,
	const char *defaultString
){
	strcpy_s(dstString, dstStringSizeInBytes, defaultString);
	cJSON *jsonFound = cJSONUtils_GetPointer(json, pointer);
	if (jsonFound == NULL) return false;
	if (cJSON_IsString(jsonFound) == false || (jsonFound->valuestring == NULL)) return false;
	if (strlen(jsonFound->valuestring) + 1 /* 末端 \0 分 */ > dstStringSizeInBytes) return false;
	strcpy_s(dstString, dstStringSizeInBytes, jsonFound->valuestring);
	return true;
}

bool JsonGetAsInt(
	cJSON *json,
	const char *pointer,
	int *dst,
	int defaultValue
){
	*dst = defaultValue;
	cJSON *jsonFound = cJSONUtils_GetPointer(json, pointer);
	if (jsonFound == NULL) return false;
	if (cJSON_IsNumber(jsonFound) == false) return false;
	*dst = (int)jsonFound->valuedouble;
	return true;
}

bool JsonGetAsFloat(
	cJSON *json,
	const char *pointer,
	float *dst,
	float defaultValue
){
	*dst = defaultValue;
	cJSON *jsonFound = cJSONUtils_GetPointer(json, pointer);
	if (jsonFound == NULL) return false;
	if (cJSON_IsNumber(jsonFound) == false) return false;
	*dst = (float)jsonFound->valuedouble;
	return true;
}

bool JsonGetAsBool(
	cJSON *json,
	const char *pointer,
	bool *dst,
	bool defaultValue
){
	*dst = defaultValue;
	cJSON *jsonFound = cJSONUtils_GetPointer(json, pointer);
	if (jsonFound == NULL) return false;
	if (cJSON_IsBool(jsonFound) == false) return false;
	*dst = cJSON_IsTrue(jsonFound);
	return true;
}

bool JsonGetAsVec3(
	cJSON *json,
	const char *pointer,
	float vec3Dst[3],
	const float vec3Default[3]
){
	vec3Dst[0] = vec3Default[0];
	vec3Dst[1] = vec3Default[1];
	vec3Dst[2] = vec3Default[2];
	cJSON *jsonFound = cJSONUtils_GetPointer(json, pointer);
	if (jsonFound == NULL) return false;

	if (cJSON_IsArray(jsonFound) == false) return false;
	if (cJSON_GetArraySize(jsonFound) != 3) return false;
	cJSON *jsonFoundElement0 = cJSON_GetArrayItem(jsonFound, 0);
	cJSON *jsonFoundElement1 = cJSON_GetArrayItem(jsonFound, 1);
	cJSON *jsonFoundElement2 = cJSON_GetArrayItem(jsonFound, 2);
	if (cJSON_IsNumber(jsonFoundElement0) == false) return false;
	if (cJSON_IsNumber(jsonFoundElement1) == false) return false;
	if (cJSON_IsNumber(jsonFoundElement2) == false) return false;
	vec3Dst[0] = (float)jsonFoundElement0->valuedouble;
	vec3Dst[1] = (float)jsonFoundElement1->valuedouble;
	vec3Dst[2] = (float)jsonFoundElement2->valuedouble;
	return true;
}

bool JsonGetAsVec4(
	cJSON *json,
	const char *pointer,
	float vec4Dst[4],
	const float vec4Default[4]
){
	vec4Dst[0] = vec4Default[0];
	vec4Dst[1] = vec4Default[1];
	vec4Dst[2] = vec4Default[2];
	vec4Dst[3] = vec4Default[3];
	cJSON *jsonFound = cJSONUtils_GetPointer(json, pointer);
	if (jsonFound == NULL) return false;

	if (cJSON_IsArray(jsonFound) == false) return false;
	if (cJSON_GetArraySize(jsonFound) != 4) return false;
	cJSON *jsonFoundElement0 = cJSON_GetArrayItem(jsonFound, 0);
	cJSON *jsonFoundElement1 = cJSON_GetArrayItem(jsonFound, 1);
	cJSON *jsonFoundElement2 = cJSON_GetArrayItem(jsonFound, 2);
	cJSON *jsonFoundElement3 = cJSON_GetArrayItem(jsonFound, 3);
	if (cJSON_IsNumber(jsonFoundElement0) == false) return false;
	if (cJSON_IsNumber(jsonFoundElement1) == false) return false;
	if (cJSON_IsNumber(jsonFoundElement2) == false) return false;
	if (cJSON_IsNumber(jsonFoundElement3) == false) return false;
	vec4Dst[0] = (float)jsonFoundElement0->valuedouble;
	vec4Dst[1] = (float)jsonFoundElement1->valuedouble;
	vec4Dst[2] = (float)jsonFoundElement2->valuedouble;
	vec4Dst[3] = (float)jsonFoundElement3->valuedouble;
	return true;
}
