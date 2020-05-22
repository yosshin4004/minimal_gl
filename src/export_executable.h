/* Copyright (C) 2018 Yosshin(@yosshin4004) */


#include "graphics.h"


#ifndef _EXPORT_EXECUTABLE_H_
#define _EXPORT_EXECUTABLE_H_

typedef enum {
	CrinklerCompModeInstant,
	CrinklerCompModeFast,
	CrinklerCompModeSlow,
	CrinklerCompModeVerySlow,
} CrinklerCompMode;

struct ExecutableExportSettings {
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
	const char *exeFileName,
	const char *graphicsShaderCode,
	const char *soundShaderCode,
	const RenderSettings *renderSettings,
	const ExecutableExportSettings *executableExportSettings
);


#endif
