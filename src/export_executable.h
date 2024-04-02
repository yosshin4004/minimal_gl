/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#ifndef _EXPORT_EXECUTABLE_H_
#define _EXPORT_EXECUTABLE_H_


#include "graphics.h"


/* ShaderMinifier の NoRenamingList 最大文字数 */
#define SHADER_MINIFIER_NO_RENAMING_LIST_MAX	0x1000


typedef enum {
	CrinklerCompModeInstant,
	CrinklerCompModeFast,
	CrinklerCompModeSlow,
	CrinklerCompModeVerySlow,
	CrinklerCompModeDisable,
} CrinklerCompMode;

struct ExecutableExportSettings {
	char fileName[MAX_PATH];
	int xReso;
	int yReso;
	float durationInSeconds;
	int numSoundBufferSamples;
	int numSoundBufferAvailableSamples;
	int numSoundBufferSamplesPerDispatch;
	bool enableFrameCountUniform;
	bool enableSoundDispatchWait;
	struct ShaderMinifierOptions {
		bool enableFieldNames;
		int fieldNameIndex;
		bool noRenaming;
		bool enableNoRenamingList;
		char noRenamingList[SHADER_MINIFIER_NO_RENAMING_LIST_MAX];
		bool noSequence;
		bool smoothstep;
	} shaderMinifierOptions;
	struct CrinklerOptions {
		CrinklerCompMode compMode;
		bool useTinyHeader;
		bool useTinyImport;
	} crinklerOptions;
};

/* exe ファイルにエクスポート */
bool ExportExecutable(
	const char *graphicsShaderCode,
	const char *soundShaderCode,
	const RenderSettings *renderSettings,
	const ExecutableExportSettings *executableExportSettings
);


#endif
