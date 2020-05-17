/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#ifndef _DIALOG_EDIT_CAMERA_PARAMS_H_
#define _DIALOG_EDIT_CAMERA_PARAMS_H_


typedef enum {
	DialogEditCameraParamsResult_Ok,
	DialogEditCameraParamsResult_Canceled,
} DialogEditCameraParamsResult;

/* カメラパラメータエディットダイアログボックス */
DialogEditCameraParamsResult
DialogEditCameraParams();


#endif
