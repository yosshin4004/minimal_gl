/* Copyright (C) 2026 Yosshin(@yosshin4004) */

#include "common.h"
#include "config.h"
#include "json_util.h"
#include "external/simple_regex/re.h"
#include "midi_state_tracker.h"
#include "user_uniform.h"


/* ユーザーユニフォーム名の最大長 */
#define USER_UNIFORM_NAME_MAX_LENGTH_IN_BYTES		256

/* ユーザーユニフォーム名の location 最大値+1 */
#define USER_UNIFORM_LOCATION_LOCATION_END			256


typedef enum {
	UniformGuiType_Undefined,
	UniformGuiType_Checkbox,
	UniformGuiType_InputInt,
	UniformGuiType_InputFloat,
	UniformGuiType_SliderInt,
	UniformGuiType_SliderFloat,
	UniformGuiType_ColorEdit3,
	UniformGuiType_ColorEdit4,
	UniformGuiType_MidiInInt,
	UniformGuiType_MidiInFloat,
} UniformGuiType;

struct UserUniform {
	UniformGuiType type;
	char name[USER_UNIFORM_NAME_MAX_LENGTH_IN_BYTES];
	bool available;
	union {
		struct Checkbox {
			bool	defaultValue;
			bool	value;
		} asCheckbox;
		struct InputInt {
			int		defaultValue;
			int		value;
		} asInputInt;
		struct InputFloat {
			float	defaultValue;
			float	value;
		} asInputFloat;
		struct SliderInt {
			int		minValue;
			int		maxValue;
			int		defaultValue;
			int		value;
		} asSliderInt;
		struct SliderFloat {
			float	minValue;
			float	maxValue;
			float	defaultValue;
			float	value;
		} asSliderFloat;
		struct ColorEdit3 {
			float	defaultComponents[3];
			float	components[3];
		} asColorEdit3;
		struct ColorEdit4 {
			float	defaultComponents[4];
			float	components[4];
		} asColorEdit4;
		struct MidiInInt {
			int		ccNumber;
		} asMidiInInt;
		struct MidiInFloat {
			int		ccNumber;
		} asMidiInFloat;
	} u;
};
struct UserUniformCategory {
	UserUniform uniforms[USER_UNIFORM_LOCATION_LOCATION_END];
};
struct UserUniformDb {
	UserUniformCategory categories[UserUniformCategoryIndex_Count];
};
static UserUniformDb s_userUniformDb = {};


void UserUniformResetToDefault(UserUniformCategoryIndex categoryIndex){
	assert(0 <= categoryIndex && categoryIndex < UserUniformCategoryIndex_Count);
	UserUniformCategory *category = &s_userUniformDb.categories[categoryIndex];
	for (int location = 0; location < SIZE_OF_ARRAY(category->uniforms); location++) {
		UserUniform *uniform = &category->uniforms[location];
		switch (uniform->type) {
			case UniformGuiType_Checkbox: {
				uniform->u.asCheckbox.value = uniform->u.asCheckbox.defaultValue;
			} break;

			case UniformGuiType_InputInt: {
				uniform->u.asInputInt.value = uniform->u.asInputInt.defaultValue;
			} break;

			case UniformGuiType_InputFloat: {
				uniform->u.asInputFloat.value = uniform->u.asInputFloat.defaultValue;
			} break;

			case UniformGuiType_SliderInt: {
				uniform->u.asSliderInt.value = uniform->u.asSliderInt.defaultValue;
			} break;

			case UniformGuiType_SliderFloat: {
				uniform->u.asSliderFloat.value = uniform->u.asSliderFloat.defaultValue;
			} break;

			case UniformGuiType_ColorEdit3: {
				uniform->u.asColorEdit3.components[0] = uniform->u.asColorEdit3.defaultComponents[0];
				uniform->u.asColorEdit3.components[1] = uniform->u.asColorEdit3.defaultComponents[1];
				uniform->u.asColorEdit3.components[2] = uniform->u.asColorEdit3.defaultComponents[2];
			} break;

			case UniformGuiType_ColorEdit4: {
				uniform->u.asColorEdit4.components[0] = uniform->u.asColorEdit4.defaultComponents[0];
				uniform->u.asColorEdit4.components[1] = uniform->u.asColorEdit4.defaultComponents[1];
				uniform->u.asColorEdit4.components[2] = uniform->u.asColorEdit4.defaultComponents[2];
				uniform->u.asColorEdit4.components[3] = uniform->u.asColorEdit4.defaultComponents[3];
			} break;

			case UniformGuiType_MidiInInt:
			case UniformGuiType_MidiInFloat: {
				/* リセットという概念はない */
			} break;

			default: {
				assert(false);
			} break;
		}
	}
}

void UserUniformSerializeToJson(
	cJSON *jsonRoot
){
	cJSON *jsonDb = cJSON_AddObjectToObject(jsonRoot, "userUniformDb");
	cJSON *jsonCategories = cJSON_AddArrayToObject(jsonDb, "categories");

	for (int categoryIndex = 0; categoryIndex < UserUniformCategoryIndex_Count; categoryIndex++) {
		const UserUniformCategory *category = &s_userUniformDb.categories[categoryIndex];

		cJSON *jsonCategory = cJSON_CreateObject();
		cJSON_AddItemToArray(jsonCategories, jsonCategory);
		cJSON *jsonUniforms = cJSON_AddArrayToObject(jsonCategory, "uniforms");

		for (int location = 0; location < SIZE_OF_ARRAY(category->uniforms); location++) {
			const UserUniform *uniform = &category->uniforms[location];
			if (uniform->type != UniformGuiType_Undefined) {
				cJSON *jsonUniform = cJSON_CreateObject();
				cJSON_AddItemToArray(jsonUniforms, jsonUniform);
				cJSON_AddNumberToObject(jsonUniform, "location", location);
				cJSON_AddStringToObject(jsonUniform, "name", uniform->name);
				switch (uniform->type) {
					case UniformGuiType_Checkbox: {
						cJSON_AddBoolToObject(jsonUniform, "checkbox", uniform->u.asCheckbox.value);
					} break;
					case UniformGuiType_InputInt: {
						cJSON_AddNumberToObject(jsonUniform, "inputInt", uniform->u.asInputInt.value);
					} break;
					case UniformGuiType_InputFloat: {
						cJSON_AddNumberToObject(jsonUniform, "inputFloat", uniform->u.asInputFloat.value);
					} break;
					case UniformGuiType_SliderInt: {
						cJSON_AddNumberToObject(jsonUniform, "sliderInt", uniform->u.asSliderInt.value);
					} break;
					case UniformGuiType_SliderFloat: {
						cJSON_AddNumberToObject(jsonUniform, "sliderFloat", uniform->u.asSliderFloat.value);
					} break;
					case UniformGuiType_ColorEdit3: {
						cJSON *jsonColorEdit3 = cJSON_AddArrayToObject(jsonUniform, "colorEdit3");
						cJSON_AddItemToArray(jsonColorEdit3, cJSON_CreateNumber(uniform->u.asColorEdit3.components[0]));
						cJSON_AddItemToArray(jsonColorEdit3, cJSON_CreateNumber(uniform->u.asColorEdit3.components[1]));
						cJSON_AddItemToArray(jsonColorEdit3, cJSON_CreateNumber(uniform->u.asColorEdit3.components[2]));
					} break;
					case UniformGuiType_ColorEdit4: {
						cJSON *jsonColorEdit4 = cJSON_AddArrayToObject(jsonUniform, "colorEdit4");
						cJSON_AddItemToArray(jsonColorEdit4, cJSON_CreateNumber(uniform->u.asColorEdit4.components[0]));
						cJSON_AddItemToArray(jsonColorEdit4, cJSON_CreateNumber(uniform->u.asColorEdit4.components[1]));
						cJSON_AddItemToArray(jsonColorEdit4, cJSON_CreateNumber(uniform->u.asColorEdit4.components[2]));
						cJSON_AddItemToArray(jsonColorEdit4, cJSON_CreateNumber(uniform->u.asColorEdit4.components[3]));
					} break;
					case UniformGuiType_MidiInInt:
					case UniformGuiType_MidiInFloat: {
						/* シリアライズする必要はない */
					} break;
					default: { 
						assert(false);
					} break;
				}
			}
		}
	}
}

bool UserUniformDeserializeFromJson(
	cJSON *jsonRoot
){
	/* 現在のデータベースの一時コピー */
	UserUniformDb tmpUserUniformDb = s_userUniformDb;

	cJSON *jsonCategories = cJSONUtils_GetPointer(jsonRoot, "/userUniformDb/categories");
	if (jsonCategories == NULL) {
		return false;
	}
	for (int categoryIndex = 0; categoryIndex < UserUniformCategoryIndex_Count; categoryIndex++) {
		UserUniformCategory *category = &tmpUserUniformDb.categories[categoryIndex];

		/* まずはすべての uniform を n/a 状態に変更 */
		for (int location = 0; location < SIZE_OF_ARRAY(category->uniforms); location++) {
			category->uniforms[location].available = false;
		}

		/* uniform 情報の読み取り */
		{
			cJSON *jsonCategory = cJSON_GetArrayItem(jsonCategories, categoryIndex);
			if (jsonCategory == NULL) {
				return false;
			}
			cJSON *jsonUniforms = cJSONUtils_GetPointer(jsonCategory, "/uniforms");
			if (jsonUniforms == NULL) {
				return false;
			}
			int numUniforms = cJSON_GetArraySize(jsonUniforms);
			for (int i = 0; i < numUniforms; i++) {
				cJSON *jsonUniform = cJSON_GetArrayItem(jsonUniforms, i);
				if (jsonUniform == NULL) {
					return false;
				}

				int location = 0;
				JsonGetAsInt(jsonUniform, "/location", &location, 0);

				/* location の範囲チェック */
				if (location < UNIFORM_LOCATION_USER || location >= SIZE_OF_ARRAY(category->uniforms)) {
					return false;
				}

				UserUniform *uniform = &category->uniforms[location];
				uniform->available = true;

				char proposalName[USER_UNIFORM_NAME_MAX_LENGTH_IN_BYTES];
				JsonGetAsString(jsonUniform, "/name", proposalName, sizeof(proposalName), "");
				if (strcmp(uniform->name, proposalName) == 0) {
					float vec3Defalut[3] = {0,0,0};
					float vec4Defalut[4] = {0,0,0,1};

					switch (uniform->type) {
						case UniformGuiType_Checkbox: {
							JsonGetAsBool(jsonUniform, "/checkbox", &uniform->u.asCheckbox.value, 0);
						} break;
						case UniformGuiType_InputInt: {
							JsonGetAsInt(jsonUniform, "/inputInt", &uniform->u.asInputInt.value, 0);
						} break;
						case UniformGuiType_InputFloat: {
							JsonGetAsFloat(jsonUniform, "/inputFloat", &uniform->u.asInputFloat.value, 0);
						} break;
						case UniformGuiType_SliderInt: {
							JsonGetAsInt(jsonUniform, "/sliderInt", &uniform->u.asSliderInt.value, 0);
						} break;
						case UniformGuiType_SliderFloat: {
							JsonGetAsFloat(jsonUniform, "/sliderFloat", &uniform->u.asSliderFloat.value, 0);
						} break;
						case UniformGuiType_ColorEdit3: {
							JsonGetAsVec3(jsonUniform, "/colorEdit3", uniform->u.asColorEdit3.components, vec3Defalut);
						} break;
						case UniformGuiType_ColorEdit4: {
							JsonGetAsVec4(jsonUniform, "/colorEdit4", uniform->u.asColorEdit4.components, vec4Defalut);
						} break;
						case UniformGuiType_MidiInInt:
						case UniformGuiType_MidiInFloat: {
							/* デシリアライズする必要はない */
						} break;
						default: {
							assert(false);
						} break;
					}
				}
			}
		}

		/* パースが終わっても n/a 状態のものは type を無効化 */
		for (int location = 0; location < SIZE_OF_ARRAY(category->uniforms); location++) {
			if (category->uniforms[location].available == false) {
				category->uniforms[location].type = UniformGuiType_Undefined;
			}
		}
	}

	/* 一時コピーを本番データベースに反映 */
	s_userUniformDb = tmpUserUniformDb;

	return true;
}

static bool UserUniformParseShaderLine(
	UserUniformCategoryIndex categoryIndex,
	const char *shaderLine
){
	/* GLSL 表記の float にマッチする正規表現 */
	#define REGEX_FLOAT R"([-+]?(?:\d+\.\d*|\.\d+|\d+)(?:[eE][-+]?\d+)?[fF]?)"

	/*
		例:
			layout (location = 16) uniform bool userFlag;	// @ui checkbox (default=false)
	*/
	re_t regexCheckbox = {};
	const char *patternCheckbox =
		"\\s*"
		"layout"
		"\\s*"
		"\\("
			"\\s*"
			"location"
			"\\s*"
			"="
			"\\s*"
			"(\\d+)"				// location
			"\\s*"
		"\\)"
		"\\s*"
		"uniform"
		"\\s+"
		"bool"
		"\\s+"
		"(\\w+)"					// uniform名
		"\\s*"
		";"
		"\\s*"
		"//"
		"\\s*"
		"@ui"
		"\\s+"
		"checkbox"
		"\\s*"
		"\\("
			"\\s*"
			"default"
			"\\s*"
			"="
			"\\s*"
			"(\\w+)"				// default値
			"\\s*"
		"\\)"
		"\\s*"
		"$"
	;
	if (re_compile(&regexCheckbox, patternCheckbox) != 0) {
		assert(false);
		return false;
	}

	/*
		例:
			layout (location = 16) uniform int   userInt;		// @ui input (default=123)
			layout (location = 16) uniform float userFloat;		// @ui input (default=123)
	*/
	re_t regexInput = {};
	const char *patternInput =
		"\\s*"
		"layout"
		"\\s*"
		"\\("
			"\\s*"
			"location"
			"\\s*"
			"="
			"\\s*"
			"(\\d+)"				// location
			"\\s*"
		"\\)"
		"\\s*"
		"uniform"
		"\\s+"
		"(\\w+)"					// uniform型名
		"\\s+"
		"(\\w+)"					// uniform名
		"\\s*"
		";"
		"\\s*"
		"//"
		"\\s*"
		"@ui"
		"\\s+"
		"input"
		"\\s*"
		"\\("
			"\\s*"
			"default"
			"\\s*"
			"="
			"\\s*"
			"(" REGEX_FLOAT ")"		// default値
			"\\s*"
		"\\)"
		"\\s*"
		".*"
		"$"
	;
	if (re_compile(&regexInput, patternInput) != 0) {
		assert(false);
		return false;
	}

	/*
		例:
			layout (location = 16) uniform int   userInt;	// @ui slider (min=0, max=100, default=50)
			layout (location = 16) uniform float userFloat;	// @ui slider (min=0, max=100, default=50)
	*/
	re_t regexSlider = {};
	const char *patternSlider =
		"\\s*"
		"layout"
		"\\s*"
		"\\("
			"\\s*"
			"location"
			"\\s*"
			"="
			"\\s*"
			"(\\d+)"				// location
			"\\s*"
		"\\)"
		"\\s*"
		"uniform"
		"\\s+"
		"(\\w+)"					// uniform型名
		"\\s+"
		"(\\w+)"					// uniform名
		"\\s*"
		";"
		"\\s*"
		"//"
		"\\s*"
		"@ui"
		"\\s+"
		"slider"
		"\\s*"
		"\\("
			"\\s*"
			"min"
			"\\s*"
			"="
			"\\s*"
			"(\\d+)"				// min値
			"\\s*"
			","
			"\\s*"
			"max"
			"\\s*"
			"="
			"\\s*"
			"(\\d+)"				// max値
			"\\s*"
			","
			"\\s*"
			"default"
			"\\s*"
			"="
			"\\s*"
			"(" REGEX_FLOAT ")"		// default値
			"\\s*"
		"\\)"
		"\\s*"
		".*"
		"$"
	;
	if (re_compile(&regexSlider, patternSlider) != 0) {
		assert(false);
		return false;
	}

	/*
		例:
			layout (location = 16) uniform vec3 userColor;	// @ui color (default=vec3(1,1,1))
	*/
	re_t regexColorEdit3 = {};
	const char *patternColorEdit3 =
		"\\s*"
		"layout"
		"\\s*"
		"\\("
			"\\s*"
			"location"
			"\\s*"
			"="
			"\\s*"
			"(\\d+)"				// location
			"\\s*"
		"\\)"
		"\\s*"
		"uniform"
		"\\s+"
		"vec3"
		"\\s+"
		"(\\w+)"					// uniform名
		"\\s*"
		";"
		"\\s*"
		"//"
		"\\s*"
		"@ui"
		"\\s+"
		"color"
		"\\s*"
		"\\("
			"\\s*"
			"default"
			"\\s*"
			"="
			"\\s*"
			"\\{"
				"\\s*"
				"(" REGEX_FLOAT ")"	// defaultR値
				"\\s*"
				","
				"\\s*"
				"(" REGEX_FLOAT ")"	// defaultG値
				"\\s*"
				","
				"\\s*"
				"(" REGEX_FLOAT ")"	// defaultB値
				"\\s*"
			"\\}"
			"\\s*"
		"\\)"
		"\\s*"
		".*"
		"$"
	;
	if (re_compile(&regexColorEdit3, patternColorEdit3) != 0) {
		assert(false);
		return false;
	}

	/*
		例:
			layout (location = 16) uniform vec4 userColor;	// @ui color (default=vec4(1,1,1,1))
	*/
	re_t regexColorEdit4 = {};
	const char *patternColorEdit4 =
		"\\s*"
		"layout"
		"\\s*"
		"\\("
			"\\s*"
			"location"
			"\\s*"
			"="
			"\\s*"
			"(\\d+)"				// location
			"\\s*"
		"\\)"
		"\\s*"
		"uniform"
		"\\s+"
		"vec4"
		"\\s+"
		"(\\w+)"					// uniform名
		"\\s*"
		";"
		"\\s*"
		"//"
		"\\s*"
		"@ui"
		"\\s+"
		"color"
		"\\s*"
		"\\("
			"\\s*"
			"default"
			"\\s*"
			"="
			"\\s*"
			"\\{"
				"\\s*"
				"(" REGEX_FLOAT ")"	// defaultR値
				"\\s*"
				","
				"\\s*"
				"(" REGEX_FLOAT ")"	// defaultG値
				"\\s*"
				","
				"\\s*"
				"(" REGEX_FLOAT ")"	// defaultB値
				"\\s*"
				","
				"\\s*"
				"(" REGEX_FLOAT ")"	// defaultA値
				"\\s*"
			"\\}"
			"\\s*"
		"\\)"
		"\\s*"
		".*"
		"$"
	;
	if (re_compile(&regexColorEdit4, patternColorEdit4) != 0) {
		assert(false);
		return false;
	}

	/*
		例:
			layout (location = 16) uniform int   userInt;	// @ui midi (cc=123)
			layout (location = 16) uniform float userFloat;	// @ui midi (cc=123)
	*/
	re_t regexMidiIn = {};
	const char *patternMidiIn =
		"\\s*"
		"layout"
		"\\s*"
		"\\("
			"\\s*"
			"location"
			"\\s*"
			"="
			"\\s*"
			"(\\d+)"				// location
			"\\s*"
		"\\)"
		"\\s*"
		"uniform"
		"\\s+"
		"(\\w+)"					// uniform型名
		"\\s+"
		"(\\w+)"					// uniform名
		"\\s*"
		";"
		"\\s*"
		"//"
		"\\s*"
		"@ui"
		"\\s+"
		"midi"
		"\\s*"
		"\\("
			"\\s*"
			"cc"
			"\\s*"
			"="
			"\\s*"
			"(\\d+)"				// min値
			"\\s*"
		"\\)"
		"\\s*"
		".*"
		"$"
	;
	if (re_compile(&regexMidiIn, patternMidiIn) != 0) {
		assert(false);
		return false;
	}


	assert(0 <= categoryIndex && categoryIndex < UserUniformCategoryIndex_Count);
	UserUniformCategory *category = &s_userUniformDb.categories[categoryIndex];

	/* checkbox? */
	{
		re_cap_t caps[4] = {};
		int numMatches = re_match(&regexCheckbox, shaderLine, caps, SIZE_OF_ARRAY(caps));
		if (numMatches == SIZE_OF_ARRAY(caps)) {
			int location = atoi(caps[1].start);
			char szUniformName[256] = {};
			char szDefaultValue[256] = {};
			re_cap_copy(&caps[2], szUniformName, sizeof(szUniformName));
			re_cap_copy(&caps[3], szDefaultValue, sizeof(szDefaultValue));

			/* location の範囲チェック */
			if (location < UNIFORM_LOCATION_USER || location >= SIZE_OF_ARRAY(category->uniforms)) {
				/* 認識失敗 */
				return false;
			}

			/* 当該 uniform のポインタ */
			UserUniform *uniform = &category->uniforms[location];

			/* bool 値に変換 */
			bool defaultValue;
			if (strcmp(szDefaultValue, "true") == 0) {
				defaultValue = true;
			} else
			if (strcmp(szDefaultValue, "false") == 0) {
				defaultValue = false;
			} else {
				/* 認識失敗 */
				return false;
			}

			/* 更新があるか？ */
			if (
				uniform->type != UniformGuiType_Checkbox
			||	strcmp(uniform->name, szUniformName) != 0
			||	uniform->u.asCheckbox.defaultValue != defaultValue
			) {
				uniform->type = UniformGuiType_Checkbox;
				strcpy_s(uniform->name, sizeof(uniform->name), szUniformName);
				uniform->u.asCheckbox.defaultValue = defaultValue;
				uniform->u.asCheckbox.value = defaultValue;
			}

			/* 認識成功 */
			printf(
				"	layout (location = %d) uniform bool %s: checkbox (default = %s)\n",
				location, szUniformName, szDefaultValue
			);

			/* 利用可能状態に変更 */
			uniform->available = true;
			return true;
		}
	}

	/* input? */
	{
		re_cap_t caps[5] = {};
		int numMatches = re_match(&regexInput, shaderLine, caps, SIZE_OF_ARRAY(caps));
		if (numMatches == SIZE_OF_ARRAY(caps)) {
			int location = atoi(caps[1].start);
			char szTypeName[256] = {};
			char szUniformName[256] = {};
			char szDefaultValue[256] = {};
			re_cap_copy(&caps[2], szTypeName, sizeof(szTypeName));
			re_cap_copy(&caps[3], szUniformName, sizeof(szUniformName));
			re_cap_copy(&caps[4], szDefaultValue, sizeof(szDefaultValue));

			/* location の範囲チェック */
			if (location < UNIFORM_LOCATION_USER || location >= SIZE_OF_ARRAY(category->uniforms)) {
				/* 認識失敗 */
				return false;
			}

			/* 当該 uniform のポインタ */
			UserUniform *uniform = &category->uniforms[location];

			/* 型に応じた分岐 */
			if (strcmp(szTypeName, "int") == 0) {
				/* int 値に変換 */
				int defaultValue = atoi(szDefaultValue);

				/* 更新があるか？ */
				if (
					uniform->type != UniformGuiType_InputInt
				||	strcmp(uniform->name, szUniformName) != 0
				||	uniform->u.asInputInt.defaultValue != defaultValue
				) {
					uniform->type = UniformGuiType_InputInt;
					strcpy_s(uniform->name, sizeof(uniform->name), szUniformName);
					uniform->u.asInputInt.defaultValue = defaultValue;
					uniform->u.asInputInt.value = defaultValue;
				}

				/* 認識成功 */
				printf(
					"	layout (location = %d) uniform int %s: inputInt (default = %d)\n",
					location, szUniformName, defaultValue
				);

				/* 利用可能状態に変更 */
				uniform->available = true;
				return true;
			} else
			if (strcmp(szTypeName, "float") == 0) {
				/* float 値に変換 */
				float defaultValue = (float)atof(szDefaultValue);

				/* 更新があるか？ */
				if (
					uniform->type != UniformGuiType_InputFloat
				||	strcmp(uniform->name, szUniformName) != 0
				||	uniform->u.asInputFloat.defaultValue != defaultValue
				) {
					uniform->type = UniformGuiType_InputFloat;
					strcpy_s(uniform->name, sizeof(uniform->name), szUniformName);
					uniform->u.asInputFloat.defaultValue = defaultValue;
					uniform->u.asInputFloat.value = defaultValue;
				}

				/* 認識成功 */
				printf(
					"	layout (location = %d) uniform float %s: inputFloat (default = %f)\n",
					location, szUniformName, defaultValue
				);

				/* 利用可能状態に変更 */
				uniform->available = true;
				return true;
			} else {
				/* 認識失敗 */
				return false;
			}
		}
	}

	/* slider? */
	{
		re_cap_t caps[7] = {};
		int numMatches = re_match(&regexSlider, shaderLine, caps, SIZE_OF_ARRAY(caps));
		if (numMatches == SIZE_OF_ARRAY(caps)) {
			int location = atoi(caps[1].start);
			char szTypeName[256] = {};
			char szUniformName[256] = {};
			char szMinValue[256] = {};
			char szMaxValue[256] = {};
			char szDefaultValue[256] = {};
			re_cap_copy(&caps[2], szTypeName, sizeof(szTypeName));
			re_cap_copy(&caps[3], szUniformName, sizeof(szUniformName));
			re_cap_copy(&caps[4], szMinValue, sizeof(szMinValue));
			re_cap_copy(&caps[5], szMaxValue, sizeof(szMaxValue));
			re_cap_copy(&caps[6], szDefaultValue, sizeof(szDefaultValue));

			/* location の範囲チェック */
			if (location < UNIFORM_LOCATION_USER || location >= SIZE_OF_ARRAY(category->uniforms)) {
				/* 認識失敗 */
				return false;
			}

			/* 当該 uniform のポインタ */
			UserUniform *uniform = &category->uniforms[location];

			/* 型に応じた分岐 */
			if (strcmp(szTypeName, "int") == 0) {
				/* int 値に変換 */
				int minValue = atoi(szMinValue);
				int maxValue = atoi(szMaxValue);
				int defaultValue = atoi(szDefaultValue);

				/* 更新があるか？ */
				if (
					uniform->type != UniformGuiType_SliderInt
				||	strcmp(uniform->name, szUniformName) != 0
				||	uniform->u.asSliderInt.minValue != minValue
				||	uniform->u.asSliderInt.maxValue != maxValue
				||	uniform->u.asSliderInt.defaultValue != defaultValue
				) {
					uniform->type = UniformGuiType_SliderInt;
					strcpy_s(uniform->name, sizeof(uniform->name), szUniformName);
					uniform->u.asSliderInt.minValue = minValue;
					uniform->u.asSliderInt.maxValue = maxValue;
					uniform->u.asSliderInt.defaultValue = defaultValue;
					uniform->u.asSliderInt.value = defaultValue;
				}

				/* 認識成功 */
				printf(
					"	layout (location = %d) uniform int %s: sliderInt (min = %d, max = %d, default = %d)\n",
					location, szUniformName, minValue, maxValue, defaultValue
				);

				/* 利用可能状態に変更 */
				uniform->available = true;
				return true;
			} else
			if (strcmp(szTypeName, "float") == 0) {
				/* float 値に変換 */
				float minValue = (float)atof(szMinValue);
				float maxValue = (float)atof(szMaxValue);
				float defaultValue = (float)atof(szDefaultValue);

				/* 更新があるか？ */
				if (
					uniform->type != UniformGuiType_SliderFloat
				||	strcmp(uniform->name, szUniformName) != 0
				||	uniform->u.asSliderFloat.minValue != minValue
				||	uniform->u.asSliderFloat.maxValue != maxValue
				||	uniform->u.asSliderFloat.defaultValue != defaultValue
				) {
					uniform->type = UniformGuiType_SliderFloat;
					strcpy_s(uniform->name, sizeof(uniform->name), szUniformName);
					uniform->u.asSliderFloat.minValue = minValue;
					uniform->u.asSliderFloat.maxValue = maxValue;
					uniform->u.asSliderFloat.defaultValue = defaultValue;
					uniform->u.asSliderFloat.value = defaultValue;
				}

				/* 認識成功 */
				printf(
					"	layout (location = %d) uniform float %s: sliderFloat (min = %f, max = %f, default = %f)\n",
					location, szUniformName, minValue, maxValue, defaultValue
				);

				/* 利用可能状態に変更 */
				uniform->available = true;
				return true;
			}
		}
	}	

	/* vec3 color? */
	{
		re_cap_t caps[6] = {};
		int numMatches = re_match(&regexColorEdit3, shaderLine, caps, SIZE_OF_ARRAY(caps));
		if (numMatches == SIZE_OF_ARRAY(caps)) {
			int location = atoi(caps[1].start);
			char szUniformName[256] = {};
			char szDefaultR[256] = {};
			char szDefaultG[256] = {};
			char szDefaultB[256] = {};
			re_cap_copy(&caps[2], szUniformName, sizeof(szUniformName));
			re_cap_copy(&caps[3], szDefaultR, sizeof(szDefaultR));
			re_cap_copy(&caps[4], szDefaultG, sizeof(szDefaultG));
			re_cap_copy(&caps[5], szDefaultB, sizeof(szDefaultB));

			/* float 値に変換 */
			float defaultR = (float)atof(szDefaultR);
			float defaultG = (float)atof(szDefaultG);
			float defaultB = (float)atof(szDefaultB);

			/* location の範囲チェック */
			if (location < UNIFORM_LOCATION_USER || location >= SIZE_OF_ARRAY(category->uniforms)) {
				/* 認識失敗 */
				return false;
			}

			/* 当該 uniform のポインタ */
			UserUniform *uniform = &category->uniforms[location];

			/* 更新があるか？ */
			if (
				uniform->type != UniformGuiType_ColorEdit3
			||	strcmp(uniform->name, szUniformName) != 0
			||	uniform->u.asColorEdit3.defaultComponents[0] != defaultR
			||	uniform->u.asColorEdit3.defaultComponents[1] != defaultG
			||	uniform->u.asColorEdit3.defaultComponents[2] != defaultB
			) {
				uniform->type = UniformGuiType_ColorEdit3;
				strcpy_s(uniform->name, sizeof(uniform->name), szUniformName);
				uniform->u.asColorEdit3.defaultComponents[0] = defaultR;
				uniform->u.asColorEdit3.defaultComponents[1] = defaultG;
				uniform->u.asColorEdit3.defaultComponents[2] = defaultB;
				uniform->u.asColorEdit3.components[0] = defaultR;
				uniform->u.asColorEdit3.components[1] = defaultG;
				uniform->u.asColorEdit3.components[2] = defaultB;
			}

			/* 認識成功 */
			printf(
				"	layout (location = %d) uniform float %s: colorEdit3 (default = {%f, %f, %f})\n",
				location, szUniformName, defaultR, defaultG, defaultB
			);

			/* 利用可能状態に変更 */
			uniform->available = true;
			return true;
		}
	}

	/* vec4 color? */
	{
		re_cap_t caps[7] = {};
		int numMatches = re_match(&regexColorEdit4, shaderLine, caps, SIZE_OF_ARRAY(caps));
		if (numMatches == SIZE_OF_ARRAY(caps)) {
			int location = atoi(caps[1].start);
			char szUniformName[256] = {};
			char szDefaultR[256] = {};
			char szDefaultG[256] = {};
			char szDefaultB[256] = {};
			char szDefaultA[256] = {};
			re_cap_copy(&caps[2], szUniformName, sizeof(szUniformName));
			re_cap_copy(&caps[3], szDefaultR, sizeof(szDefaultR));
			re_cap_copy(&caps[4], szDefaultG, sizeof(szDefaultG));
			re_cap_copy(&caps[5], szDefaultB, sizeof(szDefaultB));
			re_cap_copy(&caps[6], szDefaultA, sizeof(szDefaultA));

			/* float 値に変換 */
			float defaultR = (float)atof(szDefaultR);
			float defaultG = (float)atof(szDefaultG);
			float defaultB = (float)atof(szDefaultB);
			float defaultA = (float)atof(szDefaultA);

			/* location の範囲チェック */
			if (location < UNIFORM_LOCATION_USER || location >= SIZE_OF_ARRAY(category->uniforms)) {
				/* 認識失敗 */
				return false;
			}

			/* 当該 uniform のポインタ */
			UserUniform *uniform = &category->uniforms[location];

			/* 更新があるか？ */
			if (
				uniform->type != UniformGuiType_ColorEdit4
			||	strcmp(uniform->name, szUniformName) != 0
			||	uniform->u.asColorEdit4.defaultComponents[0] != defaultR
			||	uniform->u.asColorEdit4.defaultComponents[1] != defaultG
			||	uniform->u.asColorEdit4.defaultComponents[2] != defaultB
			||	uniform->u.asColorEdit4.defaultComponents[3] != defaultA
			) {
				uniform->type = UniformGuiType_ColorEdit4;
				strcpy_s(uniform->name, sizeof(uniform->name), szUniformName);
				uniform->u.asColorEdit4.defaultComponents[0] = defaultR;
				uniform->u.asColorEdit4.defaultComponents[1] = defaultG;
				uniform->u.asColorEdit4.defaultComponents[2] = defaultB;
				uniform->u.asColorEdit4.defaultComponents[3] = defaultA;
				uniform->u.asColorEdit4.components[0] = defaultR;
				uniform->u.asColorEdit4.components[1] = defaultG;
				uniform->u.asColorEdit4.components[2] = defaultB;
				uniform->u.asColorEdit4.components[3] = defaultA;
			}

			/* 認識成功 */
			printf(
				"	layout (location = %d) uniform float %s: colorEdit4 (default = {%f, %f, %f, %f})\n",
				location, szUniformName, defaultR, defaultG, defaultB, defaultA
			);

			/* 利用可能状態に変更 */
			uniform->available = true;
			return true;
		}
	}

	/* midi? */
	{
		re_cap_t caps[5] = {};
		int numMatches = re_match(&regexMidiIn, shaderLine, caps, SIZE_OF_ARRAY(caps));
		if (numMatches == SIZE_OF_ARRAY(caps)) {
			int location = atoi(caps[1].start);
			char szTypeName[256] = {};
			char szUniformName[256] = {};
			char szCcNumber[256] = {};
			re_cap_copy(&caps[2], szTypeName, sizeof(szTypeName));
			re_cap_copy(&caps[3], szUniformName, sizeof(szUniformName));
			re_cap_copy(&caps[4], szCcNumber, sizeof(szCcNumber));

			/* location の範囲チェック */
			if (location < UNIFORM_LOCATION_USER || location >= SIZE_OF_ARRAY(category->uniforms)) {
				/* 認識失敗 */
				return false;
			}

			/* 当該 uniform のポインタ */
			UserUniform *uniform = &category->uniforms[location];

			/* int 値に変換 */
			int ccNumber = atoi(szCcNumber);

			/* 型に応じた分岐 */
			if (strcmp(szTypeName, "int") == 0) {
				uniform->type = UniformGuiType_MidiInInt;
				strcpy_s(uniform->name, sizeof(uniform->name), szUniformName);
				uniform->u.asMidiInInt.ccNumber = ccNumber;

				/* 認識成功 */
				printf(
					"	layout (location = %d) uniform int %s: midiInt (cc = %d)\n",
					location, szUniformName, ccNumber
				);

				/* 利用可能状態に変更 */
				uniform->available = true;
				return true;
			} else
			if (strcmp(szTypeName, "float") == 0) {
				uniform->type = UniformGuiType_MidiInFloat;
				strcpy_s(uniform->name, sizeof(uniform->name), szUniformName);
				uniform->u.asMidiInFloat.ccNumber = ccNumber;

				/* 認識成功 */
				printf(
					"	layout (location = %d) uniform float %s: midiFloat (cc = %d)\n",
					location, szUniformName, ccNumber
				);

				/* 利用可能状態に変更 */
				uniform->available = true;
				return true;
			}
		}
	}

	/* 認識失敗 */
	return false;
}

bool UserUniformParseShaderAnnotations(
	UserUniformCategoryIndex categoryIndex,
	const char *shaderCode
){
	printf("parse shader annotations ...\n");

	assert(0 <= categoryIndex && categoryIndex < UserUniformCategoryIndex_Count);
	UserUniformCategory *category = &s_userUniformDb.categories[categoryIndex];

	/* まずはすべての uniform ui を n/a 状態に変更 */
	for (int location = 0; location < SIZE_OF_ARRAY(category->uniforms); location++) {
		category->uniforms[location].available = false;
	}

	/* BOM をスキップ */
	const char *p = SkipBomConst(shaderCode);

	/* シェーダコードを全行パースする */
	const char *lineStart = shaderCode;
	while (*p) {
		if (*p == '\n' || *p == '\r') {
			size_t len = p - lineStart;
			char line[512] = {};
			if (len >= sizeof(line)) len = sizeof(line) - 1;
			memcpy(line, lineStart, len);
			line[len] = '\0';
			UserUniformParseShaderLine(categoryIndex, line);
			lineStart = p + 1;
		}
		p++;
	}

	/* パースが終わっても n/a 状態のものは type を無効化 */
	for (int location = 0; location < SIZE_OF_ARRAY(category->uniforms); location++) {
		if (category->uniforms[location].available == false) {
			category->uniforms[location].type = UniformGuiType_Undefined;
		};
	}

	printf("parse shader annotations ... done.\n");
	return true;
}

void UserUniformApplyToImgui(){
	int numAvailable = 0;
	for (int categoryIndex = 0; categoryIndex < UserUniformCategoryIndex_Count; categoryIndex++) {
		for (int location = 0; location < SIZE_OF_ARRAY(s_userUniformDb.categories[categoryIndex].uniforms); location++) {
			if (s_userUniformDb.categories[categoryIndex].uniforms[location].type != UniformGuiType_Undefined) {
				numAvailable++;
			}
		}
	}

	if (numAvailable) {
//		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoSavedSettings;
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;
		ImGuiIO &io = ImGui::GetIO();
		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x, 0), ImGuiCond_FirstUseEver, ImVec2(1.0f, 0.0f)/* 画面右上を起点とする */);
		ImGui::Begin("Uniforms", NULL, window_flags);
		int uniqueIdForImGui = 0;

		for (int categoryIndex = 0; categoryIndex < UserUniformCategoryIndex_Count; categoryIndex++) {
			UserUniformCategory *category = &s_userUniformDb.categories[categoryIndex];

			int numAvailableInCategory = 0;
			for (int location = 0; location < SIZE_OF_ARRAY(category->uniforms); location++) {
				if (category->uniforms[location].type != UniformGuiType_Undefined) {
					numAvailableInCategory++;
				}
			}
			if (numAvailableInCategory) {
				ImGui::SetNextItemOpen(true, ImGuiCond_Once);/* 初期状態で折りたたまない */
				if (
					ImGui::CollapsingHeader(
						(categoryIndex == UserUniformCategoryIndex_Graphics)? "Graphics":(
							(categoryIndex == UserUniformCategoryIndex_Sound)?    "Sound": ""
						)
					)
				) {
					/*
						グラフィクスシェーダとサウンドシェーダで同一名の uniform が存在すると、
						ImGui のラベル衝突問題が発生する。これを回避するには、ImGui::PushID(id) を
						利用する。id には、ImGui::Begin～ImGui::End 内でユニークな値を指定すれば良い。
					*/
					ImGui::PushID(uniqueIdForImGui++);

					for (int location = 0; location < SIZE_OF_ARRAY(category->uniforms); location++) {
						UserUniform *uniform = &category->uniforms[location];
						switch (uniform->type) {
							case UniformGuiType_Undefined: {
								/* スキップ */
							} break;

							case UniformGuiType_Checkbox: {
								ImGui::Checkbox(
									uniform->name,
									&uniform->u.asCheckbox.value
								);
							} break;

							case UniformGuiType_InputInt: {
								ImGui::InputInt(
									uniform->name,
									&uniform->u.asInputInt.value
								);
							} break;

							case UniformGuiType_InputFloat: {
								ImGui::InputFloat(
									uniform->name,
									&uniform->u.asInputFloat.value
								);
							} break;

							case UniformGuiType_SliderInt: {
								ImGui::SliderInt(
									uniform->name,
									&uniform->u.asSliderInt.value,
									uniform->u.asSliderInt.minValue,
									uniform->u.asSliderInt.maxValue
								);
							} break;

							case UniformGuiType_SliderFloat: {
								ImGui::SliderFloat(
									uniform->name,
									&uniform->u.asSliderFloat.value,
									uniform->u.asSliderFloat.minValue,
									uniform->u.asSliderFloat.maxValue
								);
							} break;

							case UniformGuiType_ColorEdit3: {
								ImGui::ColorEdit3(
									uniform->name,
									uniform->u.asColorEdit3.components
								);
							} break;

							case UniformGuiType_ColorEdit4: {
								ImGui::ColorEdit4(
									uniform->name,
									uniform->u.asColorEdit4.components
								);
							} break;

							case UniformGuiType_MidiInInt: {
								ImGui::BeginDisabled();
								int value = MidiStateGet(uniform->u.asMidiInInt.ccNumber);
								ImGui::SliderInt(
									uniform->name,
									&value,
									0,
									127
								);
								ImGui::EndDisabled();
							} break;

							case UniformGuiType_MidiInFloat: {
								ImGui::BeginDisabled();
								float value = (float)MidiStateGet(uniform->u.asMidiInFloat.ccNumber) / 127.0f;
								ImGui::SliderFloat(
									uniform->name,
									&value,
									0.0f,
									1.0f
								);
								ImGui::EndDisabled();
							} break;

							default: {
								assert(false);
							} break;
						}
					}

					ImGui::PopID();
				}
			}
		}

		ImGui::End();
	}
}

void UserUniformApplyToShader(GLuint shaderId, UserUniformCategoryIndex categoryIndex){
	assert(0 <= categoryIndex && categoryIndex < UserUniformCategoryIndex_Count);
	const UserUniformCategory *category = &s_userUniformDb.categories[categoryIndex];

	/* ユニフォームパラメータ設定（ImGui 由来）*/
	for (int location = 0; location < SIZE_OF_ARRAY(category->uniforms); location++) {
		const UserUniform *uniform = &category->uniforms[location];
		switch (uniform->type) {
			case UniformGuiType_Undefined: {
			} break;

			case UniformGuiType_Checkbox: {
				if (ExistsShaderUniform(shaderId, location, GL_BOOL)) {
					glUniform1i(
						/* GLint location */	location,
						/* GLint v0 */			uniform->u.asCheckbox.value? 1: 0
					);
				}
			} break;

			case UniformGuiType_InputInt: {
				if (ExistsShaderUniform(shaderId, location, GL_INT)) {
					glUniform1i(
						/* GLint location */	location,
						/* GLint v0 */			uniform->u.asInputInt.value
					);
				}
			} break;

			case UniformGuiType_InputFloat: {
				if (ExistsShaderUniform(shaderId, location, GL_FLOAT)) {
					glUniform1f(
						/* GLint location */	location,
						/* GLfloat v0 */		uniform->u.asInputFloat.value
					);
				}
			} break;

			case UniformGuiType_SliderInt: {
				if (ExistsShaderUniform(shaderId, location, GL_INT)) {
					glUniform1i(
						/* GLint location */	location,
						/* GLint v0 */			uniform->u.asSliderInt.value
					);
				}
			} break;

			case UniformGuiType_SliderFloat: {
				if (ExistsShaderUniform(shaderId, location, GL_FLOAT)) {
					glUniform1f(
						/* GLint location */	location,
						/* GLfloat v0 */		uniform->u.asSliderFloat.value
					);
				}
			} break;

			case UniformGuiType_ColorEdit3: {
				if (ExistsShaderUniform(shaderId, location, GL_FLOAT_VEC3)) {
					glUniform3f(
						/* GLint location */	location,
						/* GLfloat v0 */		uniform->u.asColorEdit3.components[0],
						/* GLfloat v1 */		uniform->u.asColorEdit3.components[1],
						/* GLfloat v2 */		uniform->u.asColorEdit3.components[2]
					);
				}
			} break;

			case UniformGuiType_ColorEdit4: {
				if (ExistsShaderUniform(shaderId, location, GL_FLOAT_VEC4)) {
					glUniform4f(
						/* GLint location */	location,
						/* GLfloat v0 */		uniform->u.asColorEdit4.components[0],
						/* GLfloat v1 */		uniform->u.asColorEdit4.components[1],
						/* GLfloat v2 */		uniform->u.asColorEdit4.components[2],
						/* GLfloat v3 */		uniform->u.asColorEdit4.components[3]
					);
				}
			} break;

			case UniformGuiType_MidiInInt: {
				if (ExistsShaderUniform(shaderId, location, GL_INT)) {
					glUniform1i(
						/* GLint location */	location,
						/* GLint v0 */			MidiStateGet(uniform->u.asMidiInInt.ccNumber)
					);
				}
			} break;

			case UniformGuiType_MidiInFloat: {
				if (ExistsShaderUniform(shaderId, location, GL_FLOAT)) {
					glUniform1f(
						/* GLint location */	location,
						/* GLfloat v0 */		(float)MidiStateGet(uniform->u.asMidiInFloat.ccNumber) / 127.0f
					);
				}
			} break;

			default: {
				assert(false);
			} break;
		}
	}
}

void UserUniformClear(UserUniformCategoryIndex categoryIndex){
	assert(0 <= categoryIndex && categoryIndex < UserUniformCategoryIndex_Count);
	memset(&s_userUniformDb.categories[categoryIndex], 0, sizeof(s_userUniformDb.categories[categoryIndex]));
	for (int location = 0; location < SIZE_OF_ARRAY(s_userUniformDb.categories[categoryIndex].uniforms); location++) {
		s_userUniformDb.categories[categoryIndex].uniforms[location].type = UniformGuiType_Undefined;
	}
}

bool UserUniformInitialize(){
	for (int categoryIndex = 0; categoryIndex < UserUniformCategoryIndex_Count; categoryIndex++) {
		UserUniformClear((UserUniformCategoryIndex)categoryIndex);
	}
	return true;
}

bool UserUniformTerminate(){
	return true;
}

