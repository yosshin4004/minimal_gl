/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "config.h"
#include "common.h"
#include "app.h"
#include "dialog_gfx_uniforms.h"
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
				"// frame counter.\r\n"
				"layout(location = 1) uniform int frameCount;\r\n"
				"\r\n"
				"// read/write sound buffer.\r\n"
				"layout(std430, binding = 0) buffer _{ vec2 samples[]; };\r\n"
				"\r\n"
				"// back buffers.\r\n"
				"layout(binding = 0) uniform sampler2D backBuffer0;\r\n"
				"layout(binding = 1) uniform sampler2D backBuffer1; // requires MRT2 or above.\r\n"
				"layout(binding = 2) uniform sampler2D backBuffer2; // requires MRT3 or above.\r\n"
				"layout(binding = 3) uniform sampler2D backBuffer3; // requires MRT4 or above.\r\n"
				"\r\n"
				"// user textures.\r\n"
				"layout(binding =  8) uniform sampler2D userTexture0;\r\n"
				"layout(binding =  9) uniform sampler2D userTexture1;\r\n"
				"layout(binding = 10) uniform sampler2D userTexture2;\r\n"
				"layout(binding = 11) uniform sampler2D userTexture3;\r\n"
				"\r\n"
				"// color outputs.\r\n"
				"layout(location = 0) out vec4 outColor0;\r\n"
				"layout(location = 1) out vec4 outColor1; // requires MRT2 or above.\r\n"
				"layout(location = 2) out vec4 outColor2; // requires MRT3 or above.\r\n"
				"layout(location = 3) out vec4 outColor3; // requires MRT4 or above.\r\n"
				"\r\n"
				;
			const char string2[] = 
				"// elapsed time (in seconds).\r\n"
				"layout(location = 2) uniform float time;\r\n"
				"\r\n"
				"// screen resolution (in pixels).\r\n"
				"layout(location = 3) uniform vec2 resolution;\r\n"
				"\r\n"
				"// mouse position.\r\n"
				"//   bottom_left = (0,0)\r\n"
				"//   top_right = (1,1)\r\n"
				"layout(location = 4) uniform vec2 mouse;\r\n"
				"\r\n"
				"// mouse buttons (.x = L_button, .y = M_button, .z = R_button).\r\n"
				"//   0 = not pressed\r\n"
				"//   1 = pressed\r\n"
				"layout(location = 5) uniform ivec3 mouseButtons;\r\n"
				"\r\n"
				"// fov Y control.\r\n"
				"//   1.0 means 90degrees.\r\n"
				"layout(location = 6) uniform float tanFovY;\r\n"
				"\r\n"
				"// camera coordinate system.\r\n"
				"layout(location = 7) uniform mat4 cameraInWorld;\r\n"
				"\r\n"
				;
			SetDlgItemText(hDwnd, IDD_GRAPHICS_SHADER_UNIFORMS_AVAILABLE_ON_EXE, string1);
			SetDlgItemText(hDwnd, IDD_GRAPHICS_SHADER_UNIFORMS_NOT_AVAILABLE_ON_EXE, string2);

			/* メッセージは処理された */
			return 1;
		} break;

		/* UI 入力を検出 */
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				/* OK */
				case IDOK:{
					/* ダイアログボックス終了 */
					EndDialog(hDwnd, DialogGraphicsShaderUniformsResult_Ok);

					/* メッセージは処理された */
					return 1;
				} break;

				/* キャンセル */
				case IDCANCEL: {
					/* ダイアログボックス終了 */
					EndDialog(hDwnd, DialogGraphicsShaderUniformsResult_Canceled);

					/* メッセージは処理された */
					return 1;
				} break;
			}
		} break;
	}

	/* メッセージは処理されなかった */
	return 0;
}


DialogGraphicsShaderUniformsResult
DialogGraphicsShaderUniforms()
{
	return (DialogGraphicsShaderUniformsResult)DialogBox(
		AppGetCurrentInstance(),
		"GRAPHICS_SHADER_UNIFORMS",
		AppGetMainWindowHandle(),
		DialogFunc
	);
}

