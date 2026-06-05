/* Copyright (C) 2026 Yosshin(@yosshin4004) */

#ifndef _MIDI_STATE_TRACKER_H_
#define _MIDI_STATE_TRACKER_H_


#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>


/* MIDI ステートの取得 */
uint8_t MidiStateGet(uint8_t ccNumber);

/* MIDI ステートトラッカーの初期化 */
bool MidiStateTrackerInitialize();

/* MIDI ステートトラッカーの終了処理 */
bool MidiStateTrackerTerminate();


#endif
