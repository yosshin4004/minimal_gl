/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#ifndef _DIALOG_CAMERA_SETTINGS_H_
#define _DIALOG_CAMERA_SETTINGS_H_


typedef enum {
	DialogCameraSettingsResult_Ok,
	DialogCameraSettingsResult_Canceled,
} DialogCameraSettingsResult;

/* カメラ設定ダイアログボックス */
DialogCameraSettingsResult
DialogCameraSettings();


#endif
