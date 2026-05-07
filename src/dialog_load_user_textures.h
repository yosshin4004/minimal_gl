/* Copyright (C) 2026 Yosshin(@yosshin4004) */

#ifndef _DIALOG_LOAD_USER_TEXTURES_H_
#define _DIALOG_LOAD_USER_TEXTURES_H_


typedef enum {
	DialogLoadUserTexturesResult_Ok,
	DialogLoadUserTexturesResult_Canceled,
} DialogLoadUserTexturesResult;

/* ユーザーテクスチャロードダイアログボックス */
DialogLoadUserTexturesResult
DialogLoadUserTextures();


#endif
