/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "config.h"
#include "common.h"
#include "app.h"
#include "dialog_confirm_over_write.h"
#include "resource/resource.h"


static const char *s_fileName = NULL;

static LRESULT CALLBACK DialogFunc(
	HWND hDwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
){
	switch (uMsg) {
		/* ダイアログボックスの初期化 */
		case WM_INITDIALOG: {
			/* ファイル名をエディットボックスに設定 */
			SetDlgItemText(
				hDwnd, IDD_CONFIRM_OVER_WRITE_FILE_NAME,
				s_fileName
			);

			/* dontAskAgain フラグをチェックボックスに設定 */
			SetDlgItemCheck(
				hDwnd, IDD_CONFIRM_OVER_WRITE_DONT_ASK_AGAIN,
				AppGetForceOverWriteFlag()
			);

			/* メッセージは処理された */
			return 1;
		} break;

		/* UI 入力を検出 */
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				/* YES（上書き）*/
				case IDD_CONFIRM_OVER_WRITE_YES: {
					/* dontAskAgain フラグをチェックボックスから取得 */
					bool dontAskAgain = GetDlgItemCheck(
						hDwnd, IDD_CONFIRM_OVER_WRITE_DONT_ASK_AGAIN
					);

					/* App に通知 */
					AppSetForceOverWriteFlag(dontAskAgain);

					/* ダイアログボックス終了 */
					EndDialog(hDwnd, DialogConfirmOverWriteResult_Yes);
				} break;

				/* キャンセル */
				case IDCANCEL: {
					/* ダイアログボックス終了 */
					EndDialog(hDwnd, DialogConfirmOverWriteResult_Canceled);
				} break;
			}
		} break;
	}

	/* メッセージは処理されなかった */
	return 0;
}


DialogConfirmOverWriteResult
DialogConfirmOverWrite(
	const char *fileName
){
	bool dontAskAgain = AppGetForceOverWriteFlag();
	if (dontAskAgain) return DialogConfirmOverWriteResult_Yes;

	FILE *file = fopen(fileName, "rb");
	if (file == NULL) return DialogConfirmOverWriteResult_Yes;
	fclose(file);

	s_fileName = fileName;
	MessageBeep(MB_ICONEXCLAMATION);
	return (DialogConfirmOverWriteResult)DialogBox(
		AppGetCurrentInstance(),
		"CONFIRM_OVER_WRITE",
		AppGetMainWindowHandle(),
		DialogFunc
	);
}

