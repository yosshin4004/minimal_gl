/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#ifndef _DIALOG_RENDER_SETTINGS_H_
#define _DIALOG_RENDER_SETTINGS_H_


typedef enum {
	DialogRenderSettingsResult_Ok,
	DialogRenderSettingsResult_Canceled,
} DialogRenderSettingsResult;

/* レンダリング設定ダイアログボックス */
DialogRenderSettingsResult
DialogRenderSettings();


#endif
