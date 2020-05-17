/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <mmsystem.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "config.h"
#include "high_precision_timer.h"

/* 初期化時の時刻 */
static double s_fp64BaseTime = 0.0f;

/* システム起動からの経過時間を秒単位で取得 */
static double GetSystemCounter(){
	LARGE_INTEGER liPerfFreq;
	LARGE_INTEGER liPerfCount;
	memset(&liPerfFreq, 0, sizeof(liPerfFreq));
	memset(&liPerfCount, 0, sizeof(liPerfCount));

	if (QueryPerformanceCounter(&liPerfCount)) {
		if (QueryPerformanceFrequency(&liPerfFreq)) {
			double	dCounter		= (double)liPerfCount.QuadPart;
			double	dFrequency		= (double)liPerfFreq.QuadPart;
			double	dCurrentTime	= dCounter / dFrequency;
			return dCurrentTime;
		}
	}

	/* パフォーマンスカウンタが利用できない場合 */
	return (double)timeGetTime() * 0.001;
}

bool HighPrecisionTimerInitialize(){
	s_fp64BaseTime = GetSystemCounter();
	return true;
}

bool HighPrecisionTimerTerminate(){
	return true;
}

double HighPrecisionTimerGet(){
	return GetSystemCounter() - s_fp64BaseTime;
}

void HighPrecisionTimerReset(double time){
	s_fp64BaseTime = GetSystemCounter() - time;
}
