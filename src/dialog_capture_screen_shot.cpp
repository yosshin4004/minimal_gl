/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "config.h"
#include "common.h"
#include "app.h"
#include "dialog_capture_screen_shot.h"
#include "resource/resource.h"


static LRESULT CALLBACK DialogFunc(
	HWND hDwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
){
	switch (uMsg) {
		/* ダイアログボックスの初期化 */
		case WM_INITDIALOG: {
			/* 解像度をエディットボックスに設定 */
			{
				int xReso = 0;
				int yReso = 0;
				AppCaptureScreenShotGetResolution(&xReso, &yReso);
				SetDlgItemInt(
					hDwnd, IDD_CAPTURE_SCREEN_SHOT_XRESO,
					xReso, false
				);
				SetDlgItemInt(
					hDwnd, IDD_CAPTURE_SCREEN_SHOT_YRESO,
					yReso, false
				);
			}

			/* 出力ファイル名をエディットボックスに設定 */
			SetDlgItemText(
				hDwnd, IDD_CAPTURE_SCREEN_SHOT_OUTPUT_FILE,
				AppCaptureScreenShotGetCurrentOutputFileName()
			);

			/* αチャンネル 1.0 強制置換フラグをチェックボックスに設定 */
			SetDlgItemCheck(
				hDwnd, IDD_CAPTURE_SCREEN_SHOT_FORCE_REPLACE_ALPHA_BY_1,
				AppCaptureScreenShotGetForceReplaceAlphaByOneFlag()
			);

			/* メッセージは処理された */
			return 1;
		} break;

		/* UI 入力を検出 */
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				/* Browse */
				case IDD_CAPTURE_SCREEN_SHOT_BROWSE_OUTPUT_FILE: {
					/* 出力ファイル名をエディットボックスから取得 */
					char outputFileName[MAX_PATH] = {0};
					GetDlgItemText(
						hDwnd, IDD_CAPTURE_SCREEN_SHOT_OUTPUT_FILE,
						outputFileName, sizeof(outputFileName)
					);

					/* ファイル選択 UI */
					OPENFILENAME ofn = {0};
					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = NULL;
					ofn.lpstrFilter =
						TEXT("PNG file (*.png)\0*.png\0");
					ofn.lpstrFile = outputFileName;
					ofn.nMaxFile = sizeof(outputFileName);
					ofn.lpstrTitle = (LPSTR)"Select output image file";
					if (GetSaveFileName(&ofn)) {
						/* 出力ファイル名をエディットボックスに設定 */
						SetDlgItemText(hDwnd, IDD_CAPTURE_SCREEN_SHOT_OUTPUT_FILE, outputFileName);
					}
				} break;

				/* OK */
				case IDOK: {
					/* 解像度をエディットボックスから取得 */
					BOOL xResoTranslated = FALSE;
					int xReso = GetDlgItemInt(
						hDwnd, IDD_CAPTURE_SCREEN_SHOT_XRESO,
						&xResoTranslated, false
					);
					BOOL yResoTranslated = FALSE;
					int yReso = GetDlgItemInt(
						hDwnd, IDD_CAPTURE_SCREEN_SHOT_YRESO,
						&yResoTranslated, false
					);
					if (xResoTranslated == FALSE) {
						AppErrorMessageBox(APP_NAME, "Invalid X resolution");
						return 0;	/* メッセージは処理されなかった */
					}
					if (yResoTranslated == FALSE) {
						AppErrorMessageBox(APP_NAME, "Invalid Y resolution");
						return 0;	/* メッセージは処理されなかった */
					}
					if (xReso > MAX_RESO || yReso > MAX_RESO) {
						AppErrorMessageBox(APP_NAME, "Invalid resolution (mast be <= %d)", MAX_RESO);
						return 0;	/* メッセージは処理されなかった */
					}

					/* 出力ファイル名をエディットボックスから取得 */
					char outputFileName[MAX_PATH] = {0};
					GetDlgItemText(
						hDwnd,
						IDD_CAPTURE_SCREEN_SHOT_OUTPUT_FILE,
						outputFileName, sizeof(outputFileName)
					);
					if (strcmp(outputFileName, "") == 0) {
						AppErrorMessageBox(APP_NAME, "Invalid output file name");
						return 0;	/* メッセージは処理されなかった */
					}

					/* αチャンネル 1.0 強制置換フラグをチェックボックスから取得 */
					bool forceReplaceAlphaByOne = GetDlgItemCheck(
						hDwnd, IDD_CAPTURE_SCREEN_SHOT_FORCE_REPLACE_ALPHA_BY_1
					);

					/* App に通知 */
					AppCaptureScreenShotSetResolution(xReso, yReso);
					AppCaptureScreenShotSetCurrentOutputFileName(outputFileName);
					AppCaptureScreenShotSetForceReplaceAlphaByOneFlag(forceReplaceAlphaByOne);

					/* ダイアログボックス終了 */
					EndDialog(hDwnd, DialogCaptureScreenShotResult_Ok);

					/* メッセージは処理された */
					return 1;
				} break;

				/* キャンセル */
				case IDCANCEL: {
					/* ダイアログボックス終了 */
					EndDialog(hDwnd, DialogCaptureScreenShotResult_Canceled);

					/* メッセージは処理された */
					return 1;
				} break;
			}
		} break;
	}

	/* メッセージは処理されなかった */
	return 0;
}


DialogCaptureScreenShotResult
DialogCaptureScreenShot()
{
	return (DialogCaptureScreenShotResult)DialogBox(
		AppGetCurrentInstance(),
		"CAPTURE_SCREEN_SHOT",
		AppGetMainWindowHandle(),
		DialogFunc
	);
}

