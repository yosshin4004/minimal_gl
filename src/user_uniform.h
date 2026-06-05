/* Copyright (C) 2026 Yosshin(@yosshin4004) */


#include "external/cJSON/cJSON.h"
#include "external/cJSON/cJSON_Utils.h"


#ifndef _USER_UNIFORM_H_
#define _USER_UNIFORM_H_


/* ユーザーユニフォームのカテゴリ */
typedef enum {
	UserUniformCategoryIndex_Graphics,
	UserUniformCategoryIndex_Sound,
	UserUniformCategoryIndex_Count
} UserUniformCategoryIndex;

/* ユーザーユニフォームの現在値をデフォルト値でリセット */
void UserUniformResetToDefault(UserUniformCategoryIndex categoryIndex);

/* ユーザーユニフォームを json からデシリアライズ */
bool UserUniformDeserializeFromJson(
	cJSON *jsonRoot
);

/* ユーザーユニフォームを json にシリアライズ */
void UserUniformSerializeToJson(
	cJSON *jsonRoot
);

/* ユーザーユニフォームを ImGui に適用 */
void UserUniformApplyToImgui();

/* ユーザーユニフォームをシェーダに適用 */
void UserUniformApplyToShader(GLuint shaderId, UserUniformCategoryIndex categoryIndex);

/* ユーザーユニフォームをシェーダアノテーション情報から読み取る */
bool UserUniformParseShaderAnnotations(
	UserUniformCategoryIndex categoryIndex,
	const char *shaderCode
);

/* ユーザーユニフォームの全クリア */
void UserUniformClear(UserUniformCategoryIndex categoryIndex);

/* ユーザーユニフォームの初期化 */
bool UserUniformInitialize();

/* ユーザーユニフォームの終了処理 */
bool UserUniformTerminate();


#endif
