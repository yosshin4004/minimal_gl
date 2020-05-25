/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "config.h"
#include "common.h"
#include "app.h"
#include "dialog_capture_cubemap.h"
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
			SetDlgItemInt(
				hDwnd, IDD_CAPTURE_CUBEMAP_RESO,
				AppCaptureCubemapGetResolution(), false
			);

			/* 出力ファイル名をエディットボックスに設定 */
			{
				const char *fileName = AppCaptureCubemapGetCurrentOutputFileName();
				if (IsValidFileName(fileName)) {
					SetDlgItemText(
						hDwnd, IDD_CAPTURE_CUBEMAP_OUTPUT_FILE,
						fileName
					);
				}
			}

			/* メッセージは処理された */
			return 1;
		} break;

		/* UI 入力を検出 */
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				/* Browse */
				case IDD_CAPTURE_CUBEMAP_BROWSE_OUTPUT_FILE: {
					/* 出力ファイル名をエディットボックスから取得 */
					char outputFileName[MAX_PATH] = {0};
					GetDlgItemText(
						hDwnd, IDD_CAPTURE_CUBEMAP_OUTPUT_FILE,
						outputFileName, sizeof(outputFileName)
					);

					/* ファイル選択 UI */
					OPENFILENAME ofn = {0};
					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = NULL;
					ofn.lpstrFilter =
						TEXT("DDS file (*.dds)\0*.dds\0");
					ofn.lpstrFile = outputFileName;
					ofn.nMaxFile = sizeof(outputFileName);
					ofn.lpstrTitle = (LPSTR)"Select output cubemap file";
					if (GetSaveFileName(&ofn)) {
						/* 出力ファイル名をエディットボックスに設定 */
						SetDlgItemText(hDwnd, IDD_CAPTURE_CUBEMAP_OUTPUT_FILE, outputFileName);
					}
				} break;

				/* OK */
				case IDOK: {
					/* 解像度をエディットボックスから取得 */
					BOOL resoTranslated = FALSE;
					int reso = GetDlgItemInt(
						hDwnd, IDD_CAPTURE_CUBEMAP_RESO,
						&resoTranslated, false
					);
					if (resoTranslated == FALSE) {
						AppErrorMessageBox(APP_NAME, "Invalid resolution");
						return 0;	/* メッセージは処理されなかった */
					}
					if (reso > MAX_RESO) {
						AppErrorMessageBox(APP_NAME, "Invalid resolution (mast be <= %d)", MAX_RESO);
						return 0;	/* メッセージは処理されなかった */
					}

					/* 出力ファイル名をエディットボックスから取得 */
					char outputFileName[MAX_PATH] = {0};
					GetDlgItemText(
						hDwnd,
						IDD_CAPTURE_CUBEMAP_OUTPUT_FILE,
						outputFileName, sizeof(outputFileName)
					);
					if (strcmp(outputFileName, "") == 0) {
						AppErrorMessageBox(APP_NAME, "Invalid output file name");
						return 0;	/* メッセージは処理されなかった */
					}

					/* App に通知 */
					AppCaptureCubemapSetResolution(reso);
					AppCaptureCubemapSetCurrentOutputFileName(outputFileName);

					/* ダイアログボックス終了 */
					EndDialog(hDwnd, DialogCaptureCubemapResult_Ok);

					/* メッセージは処理された */
					return 1;
				} break;

				/* キャンセル */
				case IDCANCEL: {
					/* ダイアログボックス終了 */
					EndDialog(hDwnd, DialogCaptureCubemapResult_Canceled);

					/* メッセージは処理された */
					return 1;
				} break;
			}
		} break;
	}

	/* メッセージは処理されなかった */
	return 0;
}


DialogCaptureCubemapResult
DialogCaptureCubemap()
{
	return (DialogCaptureCubemapResult)DialogBox(
		AppGetCurrentInstance(),
		"CAPTURE_CUBEMAP",
		AppGetMainWindowHandle(),
		DialogFunc
	);
}

