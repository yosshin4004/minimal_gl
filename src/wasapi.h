/* Copyright (C) 2026 Yosshin(@yosshin4004) */

#ifndef _WASAPI_H_
#define _WASAPI_H_

#include <stdint.h>
#include <stdbool.h>
#include "config.h"


/* WASAPI の初期化 */
bool WasapiInitialize(int numSamplesPerSec, int numChannels);

/* WASAPI の終了処理 */
void WasapiTerminate();

/* 再生の一時停止 */
void WasapiPause();

/* 再生の再開 */
void WasapiResume();

/* 再生位置のリセット */
void WasapiReset();

/* 現在の再生位置（サンプル数）の取得 */
uint32_t WasapiGetPosition();

/* 書き込み可能なサンプルフレーム数の取得 */
uint32_t WasapiGetAvailableFrames();

/* 次のバッファ要求イベントまで待機 */
bool WasapiWaitEvent(uint32_t timeoutMs);

/* 音声データの書き込み */
bool WasapiWrite(const SOUND_SAMPLE_TYPE *samples, uint32_t numFrames);


#endif

