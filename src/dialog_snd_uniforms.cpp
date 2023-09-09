/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "config.h"
#include "common.h"
#include "app.h"
#include "dialog_snd_uniforms.h"
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
				"// current wave out position (in samples).\r\n"
				"layout(location = 0) uniform int waveOutPosition;\r\n"
				"\r\n"
				"// read/write sound buffer.\r\n"
				"layout(std430, binding = 0) buffer ssbo{ vec2 samples[]; };\r\n"
				"\r\n"
				;
			SetDlgItemText(hDwnd, IDD_SOUND_SHADER_UNIFORMS_AVAILABLE_ON_EXE, string1);

			/* メッセージは処理された */
			return 1;
		} break;

		/* UI 入力を検出 */
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				/* OK */
				case IDOK:{
					/* ダイアログボックス終了 */
					EndDialog(hDwnd, DialogSoundShaderUniformsResult_Ok);

					/* メッセージは処理された */
					return 1;
				} break;

				/* キャンセル */
				case IDCANCEL: {
					/* ダイアログボックス終了 */
					EndDialog(hDwnd, DialogSoundShaderUniformsResult_Canceled);

					/* メッセージは処理された */
					return 1;
				} break;
			}
		} break;
	}

	/* メッセージは処理されなかった */
	return 0;
}


DialogSoundShaderUniformsResult
DialogSoundShaderUniforms()
{
	return (DialogSoundShaderUniformsResult)DialogBox(
		AppGetCurrentInstance(),
		"SOUND_SHADER_UNIFORMS",
		AppGetMainWindowHandle(),
		DialogFunc
	);
}

