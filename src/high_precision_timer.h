/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#ifndef _HIGH_PRECISION_TIMER_H_
#define _HIGH_PRECISION_TIMER_H_


/* 高精度タイマー初期化 */
bool HighPrecisionTimerInitialize();

/* 高精度タイマー終了処理 */
bool HighPrecisionTimerTerminate();

/* 起動からの経過時間（秒）を取得 */
double HighPrecisionTimerGet();

/* 起動からの経過時間（秒）をリセット */
void HighPrecisionTimerReset(double time);


#endif
