/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "config.h"
#include "common.h"
#include "app.h"
#include "dialog_load_user_textures.h"
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
			/* ファイル名をエディットボックスに設定 */
			for (int userTextureIndex = 0; userTextureIndex < NUM_USER_TEXTURES; userTextureIndex++) {
				SetDlgItemText(
					hDwnd, IDD_LOAD_USER_TEXTURES_FILE_0 + userTextureIndex,
					AppUserTexturesGetCurrentFileName(userTextureIndex)
				);
			}

			/* メッセージは処理された */
			return 1;
		} break;

		/* UI 入力を検出 */
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				/* OK */
				case IDOK: {
					for (int userTextureIndex = 0; userTextureIndex < NUM_USER_TEXTURES; userTextureIndex++) {
						/* ファイル名をエディットボックスから取得 */
						char textureFileName[MAX_PATH] = {0};
						GetDlgItemText(
							hDwnd,
							IDD_LOAD_USER_TEXTURES_FILE_0 + userTextureIndex,
							textureFileName, sizeof(textureFileName)
						);

						/* App に通知 */
						if (strcmp(textureFileName, "") != 0) {
							if (AppUserTexturesLoad(userTextureIndex, textureFileName) == false) {
								AppErrorMessageBox(
									APP_NAME,
									"Load texture failed.\n\n"
									"file : %s",
									textureFileName
								);
								return 0;	/* メッセージは処理されなかった */
							}
						} else {
							AppUserTexturesDelete(userTextureIndex);
						}
					}

					/* ダイアログボックス終了 */
					EndDialog(hDwnd, DialogLoadUserTexturesResult_Ok);

					/* メッセージは処理された */
					return 1;
				} break;

				/* キャンセル */
				case IDCANCEL: {
					/* ダイアログボックス終了 */
					EndDialog(hDwnd, DialogLoadUserTexturesResult_Canceled);

					/* メッセージは処理された */
					return 1;
				} break;

				default :{
					/* Browse */
					int idd = LOWORD(wParam);
					if (IDD_LOAD_USER_TEXTURES_BROWSE_FILE_0 <= idd
					&&	idd < IDD_LOAD_USER_TEXTURES_BROWSE_FILE_0 + NUM_USER_TEXTURES
					){
						for (int userTextureIndex = 0; userTextureIndex < NUM_USER_TEXTURES; userTextureIndex++) {
							if (idd == IDD_LOAD_USER_TEXTURES_BROWSE_FILE_0 + userTextureIndex) {
								/* ファイル名をエディットボックスから取得 */
								char textureFileName[MAX_PATH] = {0};
								GetDlgItemText(
									hDwnd, IDD_LOAD_USER_TEXTURES_FILE_0 + userTextureIndex,
									textureFileName, sizeof(textureFileName)
								);

								/* ファイル選択 UI */
								OPENFILENAME ofn = {0};
								ofn.lStructSize = sizeof(OPENFILENAME);
								ofn.hwndOwner = NULL;
								ofn.lpstrFilter =
									"PNG file (*.png)\0*.png\0"
									"DDS file (*.dds)\0*.dds\0"
									"All files (*.*)\0*.*\0"
									"\0";
								ofn.lpstrFile = textureFileName;
								ofn.nMaxFile = sizeof(textureFileName);
								ofn.lpstrTitle = (LPSTR)"Select user texture file";
								if (GetOpenFileName(&ofn)) {
									/* ファイル名をエディットボックスに設定 */
									SetDlgItemText(hDwnd, IDD_LOAD_USER_TEXTURES_FILE_0 + userTextureIndex, textureFileName);
								}
							}
						}
					}
				} break;
			}
		} break;
	}

	/* メッセージは処理されなかった */
	return 0;
}


DialogLoadUserTexturesResult
DialogLoadUserTextures()
{
	return (DialogLoadUserTexturesResult)DialogBox(
		AppGetCurrentInstance(),
		"LOAD_USER_TEXTURES",
		AppGetMainWindowHandle(),
		DialogFunc
	);
}

