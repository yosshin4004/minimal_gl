/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#ifndef _DIALOG_PREFERENCE_SETTINGS_H_
#define _DIALOG_PREFERENCE_SETTINGS_H_


typedef enum {
	DialogPreferenceSettingsResult_Ok,
	DialogPreferenceSettingsResult_Canceled,
} DialogPreferenceSettingsResult;

/* プリファレンス設定ダイアログボックス */
DialogPreferenceSettingsResult
DialogPreferenceSettings();


#endif
