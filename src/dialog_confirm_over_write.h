/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#ifndef _DIALOG_CONFIRM_OVER_WRITE_H_
#define _DIALOG_CONFIRM_OVER_WRITE_H_


typedef enum {
	DialogConfirmOverWriteResult_Yes,
	DialogConfirmOverWriteResult_Canceled,
} DialogConfirmOverWriteResult;

/* ファイル上書き確認ダイアログボックス */
DialogConfirmOverWriteResult
DialogConfirmOverWrite(
	const char *fileName
);


#endif
