/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "config.h"
#include "common.h"
#include "app.h"
#include "dialog_preference_settings.h"
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
			/* グラフィクスシェーダ更新によるリスタート有効化フラグをチェックボックスに設定 */
			SetDlgItemCheck(
				hDwnd, IDD_PREFERENCE_SETTINGS_AUTO_RESTART_BY_GRAPHICS_SHADER,
				AppPreferenceSettingsGetEnableAutoRestartByGraphicsShader()
			);

			/* サウンドシェーダ更新によるリスタート有効化フラグをチェックボックスに設定 */
			SetDlgItemCheck(
				hDwnd, IDD_PREFERENCE_SETTINGS_AUTO_RESTART_BY_SOUND_SHADER,
				AppPreferenceSettingsGetEnableAutoRestartBySoundShader()
			);

			/* メッセージは処理された */
			return 1;
		} break;

		/* UI 入力を検出 */
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				/* OK */
				case IDOK: {
					/* グラフィクスシェーダ更新によるリスタート有効化フラグをチェックボックスから取得 */
					bool enableAutoRestartByGraphicsShader = GetDlgItemCheck(
						hDwnd, IDD_PREFERENCE_SETTINGS_AUTO_RESTART_BY_GRAPHICS_SHADER
					);

					/* サウンドシェーダ更新によるリスタート有効化フラグをチェックボックスから取得 */
					bool enableAutoRestartBySoundShader = GetDlgItemCheck(
						hDwnd, IDD_PREFERENCE_SETTINGS_AUTO_RESTART_BY_SOUND_SHADER
					);

					/* App に通知 */
					AppPreferenceSettingsSetEnableAutoRestartByGraphicsShader(enableAutoRestartByGraphicsShader);
					AppPreferenceSettingsSetEnableAutoRestartBySoundShader(enableAutoRestartBySoundShader);

					/* ダイアログボックス終了 */
					EndDialog(hDwnd, DialogPreferenceSettingsResult_Ok);

					/* メッセージは処理された */
					return 1;
				} break;

				/* キャンセル */
				case IDCANCEL: {
					/* ダイアログボックス終了 */
					EndDialog(hDwnd, DialogPreferenceSettingsResult_Canceled);

					/* メッセージは処理された */
					return 1;
				} break;
			}
		} break;
	}

	/* メッセージは処理されなかった */
	return 0;
}


DialogPreferenceSettingsResult
DialogPreferenceSettings()
{
	return (DialogPreferenceSettingsResult)DialogBox(
		AppGetCurrentInstance(),
		"PREFERENCE_SETTINGS",
		AppGetMainWindowHandle(),
		DialogFunc
	);
}

