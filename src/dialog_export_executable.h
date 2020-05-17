/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#ifndef _DIALOG_EXPORT_EXECUTABLE_H_
#define _DIALOG_EXPORT_EXECUTABLE_H_


typedef enum {
	DialogExportExecutableResult_Ok,
	DialogExportExecutableResult_Canceled,
} DialogExportExecutableResult;

/* exe エクスポートダイアログボックス */
DialogExportExecutableResult
DialogExportExecutable();


#endif
