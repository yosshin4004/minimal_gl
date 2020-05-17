/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#ifndef _DIALOG_RECORD_IMAGE_SEQUENCE_H_
#define _DIALOG_RECORD_IMAGE_SEQUENCE_H_


typedef enum {
	DialogRecordImageSequenceResult_Ok,
	DialogRecordImageSequenceResult_Canceled,
} DialogRecordImageSequenceResult;

/* 連番画像保存ダイアログボックス */
DialogRecordImageSequenceResult
DialogRecordImageSequence();


#endif
