/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#ifndef _DIALOG_CAPTURE_SCREEN_SHOT_H_
#define _DIALOG_CAPTURE_SCREEN_SHOT_H_


typedef enum {
	DialogCaptureScreenShotResult_Ok,
	DialogCaptureScreenShotResult_Canceled,
} DialogCaptureScreenShotResult;

/* スクリーンショットキャプチャダイアログボックス */
DialogCaptureScreenShotResult
DialogCaptureScreenShot();


#endif
