/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#ifndef _DIALOG_CAPTURE_SOUND_H_
#define _DIALOG_CAPTURE_SOUND_H_


typedef enum {
	DialogCaptureSoundResult_Ok,
	DialogCaptureSoundResult_Canceled,
} DialogCaptureSoundResult;

/* サウンドキャプチャダイアログボックス */
DialogCaptureSoundResult
DialogCaptureSound();


#endif
