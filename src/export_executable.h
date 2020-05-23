/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#ifndef _EXPORT_EXECUTABLE_H_
#define _EXPORT_EXECUTABLE_H_


#include "graphics.h"


typedef enum {
	CrinklerCompModeInstant,
	CrinklerCompModeFast,
	CrinklerCompModeSlow,
	CrinklerCompModeVerySlow,
} CrinklerCompMode;

struct ExecutableExportSettings {
	char fileName[FILENAME_MAX];
	int xReso;
	int yReso;
	int numSoundBufferSamples;
	int numSoundBufferAvailableSamples;
	int numSoundBufferSamplesPerDispatch;
	bool enableFrameCountUniform;
	bool enableSoundDispatchWait;
	struct ShaderMinifierOptions {
		bool noRenaming;
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
