/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#ifndef _COMMON_H_
#define _COMMON_H_


#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <shlobj.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <sys/types.h>
#include <sys/stat.h>


/* レガシー API に対する警告の抑制 */
#pragma warning(disable:4996)


/* ceil アライメント */
int CeilAlign(
	int x,
	int align
);

/* 解像度からミップマップレベル総数を求める */
int CalcNumMipmapLevelsFromResolution(
	int xReso,
	int yReso
);

/* 妥当なディレクトリパスであるか確認 */
bool IsValidDirectoryName(
	const char *directoryName
);

/*
	ファイルは更新されたか？
	*fileStat と現在のタイムスタンプを比較する。
	*fileStat は現在のファイルの状態で上書きされる。
*/
bool IsFileUpdated(
	const char *fileName,
	struct stat *fileStat
);

/* ファイルの拡張子を検査 */
bool IsSuffix(
	const char *fileName,
	const char *suffix
);

/* シェーダの作成（エラーチェック付き）*/
GLuint CreateShader(
	GLenum type,
	GLsizei count,
	const GLchar* const *strings
);

/* シェーダ入出力インターフェースを解析し TTY に出力する */
void DumpShaderInterfaces(
	GLuint programId
);

/* 指定のシェーダ入出力インターフェースが存在することを確認する */
bool
ExistsShaderUniform(
	GLuint	programId,
	GLint	location,
	GLint	typeEnum			/* GL_FLOAT, GL_FLOAT_VEC2/3/4, etc... */
);

/* エラーチェック */
void CheckGlError(
	const char *string
);

/* ファイルサイズを取得する */
size_t GetFileSize(
	const char *fileName
);

/* メモリ確保して文字列をコピーする */
char *MallocCopyString(
	const char *string
);

/* メモリ確保してファイル読み込み（文字列終端の \0 が付加される）*/
char *MallocReadTextFile(
	const char *fileName
);

/* 指定文字のいずれかが見つかるまで読み飛ばす */
char *StrFindChars(
	char *string,
	const char *chars
);

/* 指定文字以外が見つかるまで読み飛ばす */
char *StrSkipChars(
	char *string,
	const char *chars
);

/* ディレクトリの選択 */
bool SelectDirectory(
	const char *title,
	const char *defaultDirectoryName,
	char *selectedDirectoryName,
	size_t selectedDirectoryNameSizeInBytes
);

/* ダイアログアイテムに float 値を設定 */
BOOL SetDlgItemFloat(
	HWND hDlg,
	int nIDDlgItem,
	float fValue,
	BOOL bSigned
);

/* ダイアログアイテムから float 値を取得 */
float GetDlgItemFloat(
	HWND hDlg,
	int nIDDlgItem,
	BOOL *lpTranslated,
	BOOL bSigned
);

/* チェックボックスを設定 */
void SetDlgItemCheck(
	HWND hDlg,
	int nIDDlgItem,
	BOOL check
);

/* チェックボックスを取得 */
BOOL GetDlgItemCheck(
	HWND hDlg,
	int nIDDlgItem
);

#define SIZE_OF_ARRAY(a) (sizeof(a) / sizeof(a[0]))


#endif
