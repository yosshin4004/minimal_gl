/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#ifndef _DIALOG_SND_UNIFORMS_H_
#define _DIALOG_SND_UNIFORMS_H_


typedef enum {
	DialogSoundShaderUniformsResult_Ok,
	DialogSoundShaderUniformsResult_Canceled,
} DialogSoundShaderUniformsResult;

/* サウンドシェーダ用ユニフォーム解説ダイアログボックス */
DialogSoundShaderUniformsResult
DialogSoundShaderUniforms();


#endif
