/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#ifndef _DIALOG_PREPROCESSOR_DEFINITIONS_H_
#define _DIALOG_PREPROCESSOR_DEFINITIONS_H_


typedef enum {
	DialogPreprocessorDefinitionsResult_Ok,
	DialogPreprocessorDefinitionsResult_Canceled,
} DialogPreprocessorDefinitionsResult;

/* プリプロセッサ定義解説ダイアログボックス */
DialogPreprocessorDefinitionsResult
DialogPreprocessorDefinitions();


#endif
