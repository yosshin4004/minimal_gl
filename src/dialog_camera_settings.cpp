/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "config.h"
#include "common.h"
#include "app.h"
#include "dialog_camera_settings.h"
#include "resource/resource.h"

#define PI 3.14159265359


static LRESULT CALLBACK DialogFunc(
	HWND hDwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
){
	switch (uMsg) {
		/* ダイアログボックスの初期化 */
		case WM_INITDIALOG: {
			/* カメラパラメータを App から取得 */
			float vec3Pos[3];
			float vec3Ang[3];
			float fovY;
			AppCameraSettingsGetPosition(vec3Pos);
			AppCameraSettingsGetAngleAsRadian(vec3Ang);
			fovY = AppCameraSettingsGetFovYAsRadian();

			/* radian → degree */
			vec3Ang[0] *= (float)(180.0 / PI);
			vec3Ang[1] *= (float)(180.0 / PI);
			vec3Ang[2] *= (float)(180.0 / PI);
			fovY *= (float)(180.0 / PI);

			/* カメラパラメータをエディットボックスに設定 */
			SetDlgItemFloat(hDwnd, IDD_CAMERA_SETTINGS_POS_X, vec3Pos[0], TRUE);
			SetDlgItemFloat(hDwnd, IDD_CAMERA_SETTINGS_POS_Y, vec3Pos[1], TRUE);
			SetDlgItemFloat(hDwnd, IDD_CAMERA_SETTINGS_POS_Z, vec3Pos[2], TRUE);
			SetDlgItemFloat(hDwnd, IDD_CAMERA_SETTINGS_ANG_X, vec3Ang[0], TRUE);
			SetDlgItemFloat(hDwnd, IDD_CAMERA_SETTINGS_ANG_Y, vec3Ang[1], TRUE);
			SetDlgItemFloat(hDwnd, IDD_CAMERA_SETTINGS_ANG_Z, vec3Ang[2], TRUE);
			SetDlgItemFloat(hDwnd, IDD_CAMERA_SETTINGS_FOV_Y, fovY, FALSE);

			/* メッセージは処理された */
			return 1;
		} break;

		/* UI 入力を検出 */
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				/* OK */
				case IDOK: {
					/* カメラパラメータをエディットボックスから受け取り */
					BOOL posXTranslated = FALSE;
					BOOL posYTranslated = FALSE;
					BOOL posZTranslated = FALSE;
					BOOL angXTranslated = FALSE;
					BOOL angYTranslated = FALSE;
					BOOL angZTranslated = FALSE;
					BOOL fovYTranslated = FALSE;
					float vec3Pos[3];
					float vec3Ang[3];
					float fovY;
					vec3Pos[0] = GetDlgItemFloat(hDwnd, IDD_CAMERA_SETTINGS_POS_X, &posXTranslated, TRUE);
					vec3Pos[1] = GetDlgItemFloat(hDwnd, IDD_CAMERA_SETTINGS_POS_Y, &posYTranslated, TRUE);
					vec3Pos[2] = GetDlgItemFloat(hDwnd, IDD_CAMERA_SETTINGS_POS_Z, &posZTranslated, TRUE);
					vec3Ang[0] = GetDlgItemFloat(hDwnd, IDD_CAMERA_SETTINGS_ANG_X, &angXTranslated, TRUE);
					vec3Ang[1] = GetDlgItemFloat(hDwnd, IDD_CAMERA_SETTINGS_ANG_Y, &angYTranslated, TRUE);
					vec3Ang[2] = GetDlgItemFloat(hDwnd, IDD_CAMERA_SETTINGS_ANG_Z, &angZTranslated, TRUE);
					fovY = GetDlgItemFloat(hDwnd, IDD_CAMERA_SETTINGS_FOV_Y, &fovYTranslated, FALSE);
					if (posXTranslated == FALSE) {
						AppErrorMessageBox(APP_NAME, "Invalid position X.");
						return 0;	/* メッセージは処理されなかった */
					}
					if (posYTranslated == FALSE) {
						AppErrorMessageBox(APP_NAME, "Invalid position Y.");
						return 0;	/* メッセージは処理されなかった */
					}
					if (posZTranslated == FALSE) {
						AppErrorMessageBox(APP_NAME, "Invalid position Z.");
						return 0;	/* メッセージは処理されなかった */
					}
					if (angXTranslated == FALSE) {
							AppErrorMessageBox(APP_NAME, "Invalid angle X.");
						return 0;	/* メッセージは処理されなかった */
					}
					if (angYTranslated == FALSE) {
							AppErrorMessageBox(APP_NAME, "Invalid angle Y.");
						return 0;	/* メッセージは処理されなかった */
					}
					if (angZTranslated == FALSE) {
							AppErrorMessageBox(APP_NAME, "Invalid angle Z.");
						return 0;	/* メッセージは処理されなかった */
					}
					if (fovYTranslated == FALSE) {
							AppErrorMessageBox(APP_NAME, "Invalid fov Y.");
						return 0;	/* メッセージは処理されなかった */
					}

					/* degree → radian */
					vec3Ang[0] *= (float)(PI / 180.0);
					vec3Ang[1] *= (float)(PI / 180.0);
					vec3Ang[2] *= (float)(PI / 180.0);
					fovY *= (float)(PI / 180.0);

					/* App に通知 */
					AppCameraSettingsSetPosition(vec3Pos);
					AppCameraSettingsSetAngleAsRadian(vec3Ang);
					AppCameraSettingsSetFovYAsRadian(fovY);

					/* ダイアログボックス終了 */
					EndDialog(hDwnd, DialogCameraSettingsResult_Ok);

					/* メッセージは処理された */
					return 1;
				} break;

				/* キャンセル */
				case IDCANCEL: {
					/* ダイアログボックス終了 */
					EndDialog(hDwnd, DialogCameraSettingsResult_Canceled);

					/* メッセージは処理された */
					return 1;
				} break;
			}
		} break;
	}

	/* メッセージは処理されなかった */
	return 0;
}


DialogCameraSettingsResult
DialogCameraSettings()
{
	return (DialogCameraSettingsResult)DialogBox(
		AppGetCurrentInstance(),
		"CAMERA_SETTINGS",
		AppGetMainWindowHandle(),
		DialogFunc
	);
}

