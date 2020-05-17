/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "config.h"
#include "common.h"
#include "app.h"
#include "dialog_preprocessor_definitions.h"
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
			const char string1[] = 
				"EXPORT_EXECUTABLE\r\n"
				"    defined as 1.\r\n"
				"\r\n"
				"SCREEN_XRESO\r\n"
				"    screen X resolution.\r\n"
				"\r\n"
				"SCREEN_YRESO\r\n"
				"    screen Y resolution.\r\n"
				;
			SetDlgItemText(hDwnd, IDD_PREPROCESSOR_DEFINITIONS, string1);

			/* メッセージは処理された */
			return 1;
		} break;

		/* UI 入力を検出 */
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				/* OK */
				case IDOK:{
					/* ダイアログボックス終了 */
					EndDialog(hDwnd, DialogPreprocessorDefinitionsResult_Ok);

					/* メッセージは処理された */
					return 1;
				} break;

				/* キャンセル */
				case IDCANCEL: {
					/* ダイアログボックス終了 */
					EndDialog(hDwnd, DialogPreprocessorDefinitionsResult_Canceled);

					/* メッセージは処理された */
					return 1;
				} break;
			}
		} break;
	}

	/* メッセージは処理されなかった */
	return 0;
}


DialogPreprocessorDefinitionsResult
DialogPreprocessorDefinitions()
{
	return (DialogPreprocessorDefinitionsResult)DialogBox(
		AppGetCurrentInstance(),
		"PREPROCESSOR_DEFINITIONS",
		AppGetMainWindowHandle(),
		DialogFunc
	);
}

