/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "config.h"
#include "common.h"
#include "app.h"
#include "sound.h"
#include "dialog_capture_sound.h"
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
			/* 継続時間（秒）をエディットボックスに設定 */
			SetDlgItemFloat(
				hDwnd, IDD_CAPTURE_SOUND_DURATION,
				AppCaptureSoundGetDurationInSeconds(), FALSE
			);

			/* 出力ファイル名をエディットボックスに設定 */
			{
				const char *fileName = AppCaptureSoundGetCurrentOutputFileName();
				if (IsValidFileName(fileName)) {
					SetDlgItemText(
						hDwnd, IDD_CAPTURE_SOUND_OUTPUT_FILE,
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
				/* 継続時間の自動検出 */
				case IDD_CAPTURE_SOUND_AUTO_DETECT_DURATION: {
					/* 継続時間の検出 */
					float duration = SoundDetectDurationInSeconds();

					/* 継続時間（秒）をエディットボックスに設定 */
					SetDlgItemFloat(
						hDwnd, IDD_CAPTURE_SOUND_DURATION,
						duration, FALSE
					);

					/* メッセージは処理された */
					return 1;
				} break;

				/* Browse */
				case IDD_CAPTURE_SOUND_BROWSE_OUTPUT_FILE: {
					/* 出力ファイル名をエディットボックスから取得 */
					char outputFileName[MAX_PATH] = {0};
					GetDlgItemText(
						hDwnd, IDD_CAPTURE_SOUND_OUTPUT_FILE,
						outputFileName, sizeof(outputFileName)
					);

					/* ファイル選択 UI */
					OPENFILENAME ofn = {0};
					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = NULL;
					ofn.lpstrFilter =
						"WAV file (*.wav)\0*.wav\0"
						"\0";
					ofn.lpstrFile = outputFileName;
					ofn.nMaxFile = sizeof(outputFileName);
					ofn.lpstrTitle = (LPSTR)"Select output sound file";
					if (GetSaveFileName(&ofn)) {
						/* 出力ファイル名をエディットボックスに設定 */
						SetDlgItemText(
							hDwnd, IDD_CAPTURE_SOUND_OUTPUT_FILE, outputFileName
						);
					}
				} break;

				/* OK */
				case IDOK: {
					/* 継続時間（秒）をエディットボックスから取得 */
					BOOL durationTranslated;
					float duration = GetDlgItemFloat(
						hDwnd, IDD_CAPTURE_SOUND_DURATION,
						&durationTranslated, FALSE
					);
					if (durationTranslated == FALSE) {
						AppErrorMessageBox(APP_NAME, "Invalid duration");
						return 0;	/* メッセージは処理されなかった */
					}

					/* 出力ファイル名をエディットボックスから取得 */
					char outputFileName[MAX_PATH] = {0};
					GetDlgItemText(
						hDwnd, IDD_CAPTURE_SOUND_OUTPUT_FILE,
						outputFileName, sizeof(outputFileName)
					);
					if (strcmp(outputFileName, "") == 0) {
						AppErrorMessageBox(APP_NAME, "Invalid output file name");
						return 0;	/* メッセージは処理されなかった */
					}

					/* App に通知 */
					AppCaptureSoundSetDurationInSeconds(duration);
					AppCaptureSoundSetCurrentOutputFileName(outputFileName);

					/* ダイアログボックス終了 */
					EndDialog(hDwnd, DialogCaptureSoundResult_Ok);

					/* メッセージは処理された */
					return 1;
				} break;

				/* キャンセル */
				case IDCANCEL: {
					/* ダイアログボックス終了 */
					EndDialog(hDwnd, DialogCaptureSoundResult_Canceled);

					/* メッセージは処理された */
					return 1;
				} break;
			}
		} break;
	}

	/* メッセージは処理されなかった */
	return 0;
}


DialogCaptureSoundResult
DialogCaptureSound()
{
	return (DialogCaptureSoundResult)DialogBox(
		AppGetCurrentInstance(),
		"CAPTURE_SOUND",
		AppGetMainWindowHandle(),
		DialogFunc
	);
}

