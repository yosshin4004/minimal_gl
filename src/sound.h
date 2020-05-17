/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#ifndef _SOUND_H_
#define _SOUND_H_


#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <GL/gl.h>


/* 再生一時停止 */
void SoundPauseWaveOut();

/* 再生再開 */
void SoundResumeWaveOut();

/* 先頭から再生再開 */
void SoundRestartWaveOut();

/* 再生位置の seek */
void SoundSeekWaveOut(uint32_t offset);

/* 再生位置の取得 */
int SoundGetWaveOutPos();

/* サウンド用シェーダの作成 */
bool SoundCreateShader(
	const char *shaderCode
);

/* サウンド用シェーダの削除 */
bool SoundDeleteShader();

/* サウンドの持続時間を自動検出 */
float SoundDetectDurationInSeconds();

/* サウンド出力バッファのクリア */
void SoundClearOutputBuffer();

/* サウンド生成先となる SSBO を取得 */
GLuint SoundGetOutputSsbo();

/* サウンドを wav ファイルに保存 */
bool SoundCaptureSound(
	const char *fileName,
	float durationInSeconds
);

/* サウンドの更新 */
void SoundUpdate();

/* サウンドの初期化 */
bool SoundInitialize();

/* サウンドの終了処理 */
bool SoundTerminate();


#endif
