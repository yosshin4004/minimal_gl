/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "config.h"
#include "common.h"
#include "app.h"
#include "dialog_record_image_sequence.h"
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
				int xReso = 0, yReso = 0;
				AppRecordImageSequenceGetResolution(&xReso, &yReso);
				SetDlgItemInt(
					hDwnd, IDD_RECORD_IMAGE_SEQUENCE_SCREEN_XRESO,
					xReso, FALSE
				);
				SetDlgItemInt(
					hDwnd, IDD_RECORD_IMAGE_SEQUENCE_SCREEN_YRESO,
					yReso, FALSE
				);
			}

			/* 開始位置（秒）をエディットボックスに設定 */
			SetDlgItemFloat(
				hDwnd, IDD_RECORD_IMAGE_SEQUENCE_START_TIME,
				AppRecordImageSequenceGetStartTimeInSeconds(), FALSE
			);

			/* 継続時間（秒）をエディットボックスに設定 */
			SetDlgItemFloat(
				hDwnd, IDD_RECORD_IMAGE_SEQUENCE_DURATION,
				AppRecordImageSequenceGetDurationInSeconds(), FALSE
			);

			/* FPS をエディットボックスに設定 */
			SetDlgItemFloat(
				hDwnd, IDD_RECORD_IMAGE_SEQUENCE_FRAMES_PER_SECOND,
				AppRecordImageSequenceGetFramesPerSecond(), FALSE
			);

			/* 現在の出力先ディレクトリをエディットボックスに設定 */
			{
				const char *directoryName = AppRecordImageSequenceGetCurrentOutputDirectoryName();
				if (IsValidDirectoryName(directoryName)) {
					SetDlgItemText(
						hDwnd, IDD_RECORD_IMAGE_SEQUENCE_OUTPUT_DIRECTORY,
						directoryName
					);
				}
			}

			/* αチャンネル 1.0 強制置換フラグをチェックボックスに設定 */
			SetDlgItemCheck(
				hDwnd, IDD_RECORD_IMAGE_SEQUENCE_FORCE_REPLACE_ALPHA_BY_1,
				AppRecordImageSequenceGetForceReplaceAlphaByOneFlag()
			);

			/* メッセージは処理された */
			return 1;
		} break;

		/* UI 入力を検出 */
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				/* Browse */
				case IDD_RECORD_IMAGE_SEQUENCE_BROWSE_OUTPUT_DIRECTORY: {
					/* 出力先ディレクトリをエディットボックスから取得 */
					char outputDirectoryName[MAX_PATH] = {0};
					GetDlgItemText(
						hDwnd, IDD_RECORD_IMAGE_SEQUENCE_OUTPUT_DIRECTORY,
						outputDirectoryName, sizeof(outputDirectoryName)
					);

					/* 妥当なディレクトリでない場合はデフォルトディレクトリに置き換え */
					if (IsValidDirectoryName(outputDirectoryName) == false) {
						AppGetDefaultDirectoryName(outputDirectoryName, sizeof(outputDirectoryName));
					}

					/* ディレクトリ選択 UI */
					if (
						SelectDirectory(
							APP_NAME " : Select output directory",
							outputDirectoryName,
							outputDirectoryName, sizeof(outputDirectoryName)
						)
					) {
						/* 出力先ディレクトリをエディットボックスに設定 */
						SetDlgItemText(
							hDwnd, IDD_RECORD_IMAGE_SEQUENCE_OUTPUT_DIRECTORY,
							outputDirectoryName
						);
					}
				} break;

				/* OK */
				case IDOK: {
					/* 解像度をエディットボックスから取得 */
					BOOL xResoTranslated = FALSE;
					BOOL yResoTranslated = FALSE;
					int xReso = 0, yReso = 0;
					xReso = GetDlgItemInt(
						hDwnd, IDD_RECORD_IMAGE_SEQUENCE_SCREEN_XRESO,
						&xResoTranslated, FALSE
					);
					yReso = GetDlgItemInt(
						hDwnd, IDD_RECORD_IMAGE_SEQUENCE_SCREEN_YRESO,
						&yResoTranslated, FALSE
					);
					if (xResoTranslated == FALSE) {
						AppErrorMessageBox(APP_NAME, "Invalid X resolution");
						return 0;	/* メッセージは処理されなかった */
					}
					if (yResoTranslated == FALSE) {
						AppErrorMessageBox(APP_NAME, "Invalid Y resolution");
						return 0;	/* メッセージは処理されなかった */
					}

					/* 開始位置（秒）をエディットボックスから取得 */
					BOOL startTimeTranslated = FALSE;
					float startTime = GetDlgItemFloat(
						hDwnd, IDD_RECORD_IMAGE_SEQUENCE_START_TIME,
						&startTimeTranslated, FALSE
					);
					if (startTimeTranslated == FALSE) {
						AppErrorMessageBox(APP_NAME, "Invalid start time");
						return 0;	/* メッセージは処理されなかった */
					}

					/* 継続時間（秒）をエディットボックスから取得 */
					BOOL durationTranslated = FALSE;
					float duration = GetDlgItemFloat(
						hDwnd, IDD_RECORD_IMAGE_SEQUENCE_DURATION,
						&durationTranslated, false
					);
					if (durationTranslated == FALSE) {
						AppErrorMessageBox(APP_NAME, "Invalid duration");
						return 0;	/* メッセージは処理されなかった */
					}

					/* FPS をエディットボックスから取得 */
					BOOL framesPerSecondTranslated = FALSE;
					float framesPerSecond = GetDlgItemFloat(
						hDwnd, IDD_RECORD_IMAGE_SEQUENCE_FRAMES_PER_SECOND,
						&framesPerSecondTranslated, FALSE
					);
					if (framesPerSecondTranslated == FALSE) {
						AppErrorMessageBox(APP_NAME, "Invalid frames per second");
						return 0;	/* メッセージは処理されなかった */
					}

					/* 現在の出力先ディレクトリをエディットボックスから取得 */
					char outputDirectoryName[MAX_PATH] = {0};
					GetDlgItemText(
						hDwnd, IDD_RECORD_IMAGE_SEQUENCE_OUTPUT_DIRECTORY,
						outputDirectoryName, sizeof(outputDirectoryName)
					);
					if (IsValidDirectoryName(outputDirectoryName) == false) {
						AppErrorMessageBox(APP_NAME, "Invalid output directory name");
						return 0;	/* メッセージは処理されなかった */
					}

					/* αチャンネル 1.0 強制置換フラグをチェックボックスから取得 */
					bool forceReplaceAlphaByOne = GetDlgItemCheck(
						hDwnd, IDD_RECORD_IMAGE_SEQUENCE_FORCE_REPLACE_ALPHA_BY_1
					);

					/* App に通知 */
					AppRecordImageSequenceSetResolution(xReso, yReso);
					AppRecordImageSequenceSetStartTimeInSeconds(startTime);
					AppRecordImageSequenceSetDurationInSeconds(duration);
					AppRecordImageSequenceSetFramesPerSecond(framesPerSecond);
					AppRecordImageSequenceSetCurrentOutputDirectoryName(outputDirectoryName);
					AppRecordImageSequenceSetForceReplaceAlphaByOneFlag(forceReplaceAlphaByOne);

					/* ダイアログボックス終了 */
					EndDialog(hDwnd, DialogRecordImageSequenceResult_Ok);

					/* メッセージは処理された */
					return 1;
				} break;

				/* キャンセル */
				case IDCANCEL: {
					/* ダイアログボックス終了 */
					EndDialog(hDwnd, DialogRecordImageSequenceResult_Canceled);

					/* メッセージは処理された */
					return 1;
				} break;
			}
		} break;
	}

	/* メッセージは処理されなかった */
	return 0;
}


DialogRecordImageSequenceResult
DialogRecordImageSequence()
{
	return (DialogRecordImageSequenceResult)DialogBox(
		AppGetCurrentInstance(),
		"RECORD_IMAGE_SEQUENCE",
		AppGetMainWindowHandle(),
		DialogFunc
	);
}

