/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "config.h"
#include "common.h"
#include "app.h"
#include "dialog_export_executable.h"
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
			int xReso = 0, yReso = 0;
			AppExportExecutableGetResolution(&xReso, &yReso);
			SetDlgItemInt(
				hDwnd, IDD_EXPORT_EXECUTABLE_SCREEN_XRESO,
				xReso, false
			);
			SetDlgItemInt(
				hDwnd, IDD_EXPORT_EXECUTABLE_SCREEN_YRESO,
				yReso, false
			);

			/* 継続時間（秒）をエディットボックスに設定 */
			SetDlgItemFloat(
				hDwnd, IDD_EXPORT_EXECUTABLE_DURATION,
				AppExportExecutableGetDurationInSeconds(), false
			);

			/* frameCount uniform 有効化フラグをチェックボックスに設定 */
			SetDlgItemCheck(
				hDwnd, IDD_EXPORT_EXECUTABLE_ENABLE_FRAME_COUNT_UNIFORM,
				AppExportExecutableGetEnableFrameCountUniformFlag()
			);

			/* サウンドディスパッチのウェイト実行フラグをチェックボックスに設定 */
			SetDlgItemCheck(
				hDwnd, IDD_EXPORT_EXECUTABLE_ENABLE_SOUND_DISPATCH_WAIT,
				AppExportExecutableGetEnableSoundDispatchWaitFlag()
			);

			/* ShaderMinifier の no-renaming フラグをチェックボックスに設定 */
			SetDlgItemCheck(
				hDwnd, IDD_EXPORT_EXECUTABLE_SHADER_MINIFIER_NO_RENAMING,
				AppExportExecutableGetShaderMinifierOptionsNoRenaming()
			);

			/* ShaderMinifier の no-sequence フラグをチェックボックスに設定 */
			SetDlgItemCheck(
				hDwnd, IDD_EXPORT_EXECUTABLE_SHADER_MINIFIER_NO_SEQUENCE,
				AppExportExecutableGetShaderMinifierOptionsNoSequence()
			);

			/* ShaderMinifier の smoothstep フラグをチェックボックスに設定 */
			SetDlgItemCheck(
				hDwnd, IDD_EXPORT_EXECUTABLE_SHADER_MINIFIER_SMOOTHSTEP,
				AppExportExecutableGetShaderMinifierOptionsSmoothstep()
			);

			/* Crinkler の CompMode をラジオボタンに設定 */
			{
				int nIDDlgItem = 0;
				switch (AppExportExecutableGetCrinklerOptionsCompMode()) {
					case CrinklerCompModeInstant: {
						nIDDlgItem = IDD_EXPORT_EXECUTABLE_CRINKLER_COMPMODE_INSTANT;
					} break;
					case CrinklerCompModeFast: {
						nIDDlgItem = IDD_EXPORT_EXECUTABLE_CRINKLER_COMPMODE_FAST;
					} break;
					case CrinklerCompModeSlow: {
						nIDDlgItem = IDD_EXPORT_EXECUTABLE_CRINKLER_COMPMODE_SLOW;
					} break;
					case CrinklerCompModeVerySlow: {
						nIDDlgItem = IDD_EXPORT_EXECUTABLE_CRINKLER_COMPMODE_VERYSLOW;
					} break;
					default : {
						assert(false);
					} break;
				}
				SetDlgItemCheck(hDwnd, nIDDlgItem, true);
			}

			/* Crinkler の TinyHeader 使用フラグをチェックボックスに設定 */
			SetDlgItemCheck(
				hDwnd, IDD_EXPORT_EXECUTABLE_CRINKLER_USE_TINYHEADER,
				AppExportExecutableGetCrinklerOptionsUseTinyHeader()
			);

			/* Crinkler の TinyImport 使用フラグをチェックボックスに設定 */
			SetDlgItemCheck(
				hDwnd, IDD_EXPORT_EXECUTABLE_CRINKLER_USE_TINYIMPORT,
				AppExportExecutableGetCrinklerOptionsUseTinyImport()
			);

			/* 出力ファイル名をエディットボックスに設定 */
			SetDlgItemText(
				hDwnd, IDD_EXPORT_EXECUTABLE_OUTPUT_FILE,
				AppExportExecutableGetCurrentOutputFileName()
			);

			/* メッセージは処理された */
			return 1;
		} break;

		/* UI 入力を検出 */
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				/* Browse */
				case IDD_EXPORT_EXECUTABLE_BROWSE_OUTPUT_FILE: {
					/* 出力ファイル名をエディットボックスから取得 */
					char outputFileName[FILENAME_MAX] = {0};
					GetDlgItemText(
						hDwnd,
						IDD_EXPORT_EXECUTABLE_OUTPUT_FILE,
						outputFileName,
						sizeof(outputFileName)
					);

					/* ファイル選択 UI */
					OPENFILENAME ofn = {0};
					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = NULL;
					ofn.lpstrFilter =
						TEXT("EXE file (*.exe)\0*.exe\0");
					ofn.lpstrFile = outputFileName;
					ofn.nMaxFile = sizeof(outputFileName);
					ofn.lpstrTitle = (LPSTR)"Select output executable file";
					if (GetSaveFileName(&ofn)) {
						/* 出力ファイル名をエディットボックスに設定 */
						SetDlgItemText(
							hDwnd, IDD_EXPORT_EXECUTABLE_OUTPUT_FILE, outputFileName
						);
					}
				} break;

				/* OK */
				case IDOK: {
					/* 解像度をエディットボックスから取得 */
					BOOL xResoTranslated = FALSE;
					BOOL yResoTranslated = FALSE;
					int xReso = GetDlgItemInt(
						hDwnd, IDD_EXPORT_EXECUTABLE_SCREEN_XRESO,
						&xResoTranslated, false
					);
					int yReso = GetDlgItemInt(
						hDwnd, IDD_EXPORT_EXECUTABLE_SCREEN_YRESO,
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

					/* 継続時間（秒）をエディットボックスから取得 */
					BOOL durationTranslated = FALSE;
					float duration = GetDlgItemFloat(
						hDwnd, IDD_EXPORT_EXECUTABLE_DURATION,
						&durationTranslated, false
					);
					if (durationTranslated == FALSE) {
						AppErrorMessageBox(APP_NAME, "Invalid duration");
						return 0;	/* メッセージは処理されなかった */
					}

					/* 出力ファイル名をエディットボックスから取得 */
					char outputFileName[FILENAME_MAX] = {0};
					GetDlgItemText(
						hDwnd,
						IDD_EXPORT_EXECUTABLE_OUTPUT_FILE,
						outputFileName, sizeof(outputFileName)
					);
					if (strcmp(outputFileName, "") == 0) {
						AppErrorMessageBox(APP_NAME, "Invalid output file name");
						return 0;	/* メッセージは処理されなかった */
					}

					/* frameCount uniform 有効化フラグをチェックボックスに設定 */
					bool enableFrameCountUniform = GetDlgItemCheck(
						hDwnd, IDD_EXPORT_EXECUTABLE_ENABLE_FRAME_COUNT_UNIFORM
					);

					/* サウンドディスパッチのウェイト実行フラグをチェックボックスから取得 */
					bool enableSoundDispatchWait = GetDlgItemCheck(
						hDwnd, IDD_EXPORT_EXECUTABLE_ENABLE_SOUND_DISPATCH_WAIT
					);

					/* ShaderMinifier の no-renaming フラグをチェックボックスから取得 */
					bool shaderMinifierNoRenaming = GetDlgItemCheck(
						hDwnd, IDD_EXPORT_EXECUTABLE_SHADER_MINIFIER_NO_RENAMING
					);

					/* ShaderMinifier の no-sequence フラグをチェックボックスから取得 */
					bool shaderMinifierNoSequence = GetDlgItemCheck(
						hDwnd, IDD_EXPORT_EXECUTABLE_SHADER_MINIFIER_NO_SEQUENCE
					);

					/* ShaderMinifier の smoothstep フラグをチェックボックスから取得 */
					bool shaderMinifierSmoothstep = GetDlgItemCheck(
						hDwnd, IDD_EXPORT_EXECUTABLE_SHADER_MINIFIER_SMOOTHSTEP
					);

					/* Crinkler の CompMode をラジオボタンから取得 */
					CrinklerCompMode crinklerCompMode = CrinklerCompModeInstant;
					{
						if (GetDlgItemCheck(hDwnd, IDD_EXPORT_EXECUTABLE_CRINKLER_COMPMODE_INSTANT)) {
							crinklerCompMode = CrinklerCompModeInstant;
						} else
						if (GetDlgItemCheck(hDwnd, IDD_EXPORT_EXECUTABLE_CRINKLER_COMPMODE_FAST)) {
							crinklerCompMode = CrinklerCompModeFast;
						} else
						if (GetDlgItemCheck(hDwnd, IDD_EXPORT_EXECUTABLE_CRINKLER_COMPMODE_SLOW)) {
							crinklerCompMode = CrinklerCompModeSlow;
						} else
						if (GetDlgItemCheck(hDwnd, IDD_EXPORT_EXECUTABLE_CRINKLER_COMPMODE_VERYSLOW)) {
							crinklerCompMode = CrinklerCompModeVerySlow;
						} else {
							AppErrorMessageBox(APP_NAME, "Invalid compression mode.");
						}
					}

					/* Crinkler の TinyHeader 使用フラグをチェックボックスから取得 */
					bool crinklerUseTinyHeader = GetDlgItemCheck(
						hDwnd, IDD_EXPORT_EXECUTABLE_CRINKLER_USE_TINYHEADER
					);

					/* Crinkler の TinyImport 使用フラグをチェックボックスから取得 */
					bool crinklerUseTinyImport = GetDlgItemCheck(
						hDwnd, IDD_EXPORT_EXECUTABLE_CRINKLER_USE_TINYIMPORT
					);

					/* App に通知 */
					AppExportExecutableSetResolution(xReso, yReso);
					AppExportExecutableSetDurationInSeconds(duration);
					AppExportExecutableSetEnableFrameCountUniformFlag(enableFrameCountUniform);
					AppExportExecutableSetEnableSoundDispatchWaitFlag(enableSoundDispatchWait);
					AppExportExecutableSetCurrentOutputFileName(outputFileName);
					AppExportExecutableSetShaderMinifierOptionsNoRenaming(shaderMinifierNoRenaming);
					AppExportExecutableSetShaderMinifierOptionsNoSequence(shaderMinifierNoSequence);
					AppExportExecutableSetShaderMinifierOptionsSmoothstep(shaderMinifierSmoothstep);
					AppExportExecutableSetCrinklerOptionsCompMode(crinklerCompMode);
					AppExportExecutableSetCrinklerOptionsUseTinyHeader(crinklerUseTinyHeader);
					AppExportExecutableSetCrinklerOptionsUseTinyImport(crinklerUseTinyImport);

					/* ダイアログボックス終了 */
					EndDialog(hDwnd, DialogExportExecutableResult_Ok);

					/* メッセージは処理された */
					return 1;
				} break;

				/* キャンセル */
				case IDCANCEL: {
					/* ダイアログボックス終了 */
					EndDialog(hDwnd, DialogExportExecutableResult_Canceled);

					/* メッセージは処理された */
					return 1;
				} break;
			}
		} break;
	}

	/* メッセージは処理されなかった */
	return 0;
}


DialogExportExecutableResult
DialogExportExecutable()
{
	return (DialogExportExecutableResult)DialogBox(
		AppGetCurrentInstance(),
		"EXPORT_EXECUTABLE",
		AppGetMainWindowHandle(),
		DialogFunc
	);
}

