/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#ifndef _DIALOG_GFX_UNIFORMS_H_
#define _DIALOG_GFX_UNIFORMS_H_


typedef enum {
	DialogGraphicsShaderUniformsResult_Ok,
	DialogGraphicsShaderUniformsResult_Canceled,
} DialogGraphicsShaderUniformsResult;

/* グラフィクスシェーダ用ユニフォーム解説ダイアログボックス */
DialogGraphicsShaderUniformsResult
DialogGraphicsShaderUniforms();


#endif
