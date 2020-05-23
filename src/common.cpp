/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#include "common.h"
#include "glext_util.h"


int CeilAlign(
	int x,
	int align
){
	return x + (align - 1) & ~(align - 1);
}

int CalcNumMipmapLevelsFromResolution(
	int xReso,
	int yReso
){
	int mipmapLevel = 0;
	while ((1 << mipmapLevel) < xReso && (1 << mipmapLevel) < yReso) mipmapLevel++;
	return mipmapLevel + 1;
}

bool IsValidDirectoryName(
	const char *directoryName
){
	struct stat directoryStat;
	if (stat(directoryName, &directoryStat) == 0 /* 成功 */) {
		if (directoryStat.st_mode & S_IFDIR) {
			return true;
		}
	}
	return false;
}


bool IsFileUpdated(
	const char *fileName,
	struct stat *fileStat
){
	struct stat newStat;
	bool ret = false;
#if 0
	{
		FILE *file = fopen(fileName, "rt");
		if (file != NULL) {
			if (fstat(_fileno(file), &newStat) == 0 /* 成功 */) {
				/* 最終修正時刻が異なる？ */
				if (fileStat->st_mtime != newStat.st_mtime) {
					ret = true;
				}
				*fileStat = newStat;
			}
			fclose(file);
		}
	}
#else
	/* read open することで書き込み完了待ち */
	{
		FILE *file = fopen(fileName, "rb");
		if (file) fclose(file);
	}

	/* ファイルステータス取得 */
	if (stat(fileName, &newStat) == 0 /* 成功 */) {
		/* 最終修正時刻が異なる？ */
		if (fileStat->st_mtime != newStat.st_mtime) {
			ret = true;
		}
		*fileStat = newStat;
	}
#endif
	return ret;
}


bool IsSuffix(
	const char *fileName,
	const char *suffix
){
	const char *found = strstr(fileName, suffix);
	if (found != NULL) {
		if (strlen(found) == strlen(suffix)) return true;
	}
	return false;
}


GLuint CreateShader(
	GLenum type,
	GLsizei count,
	const GLchar* const *strings
){
	GLuint programId = glExtCreateShaderProgramv(
		/* (GLenum type */					type,
		/* GLsizei count */					count,
		/* const GLchar* const *strings */	strings
	);

	/* エラーチェック */
	{
		GLint status;
		glExtGetProgramiv(programId, GL_LINK_STATUS, &status);
		if (!status) {
			char info[0x10000];
			glExtGetProgramInfoLog(programId, sizeof(info), NULL, info);
			printf("	compile error : %s", info);
			glExtDeleteProgram(programId);
			return 0;
		}
	}

	return programId;
}


void DumpShaderInterfaces(
	GLuint programId
){
	static const struct {
		GLenum programInterface;
		const char *name;
	} s_categories[] = {
		{GL_UNIFORM,				"GL_UNIFORM"},
		{GL_SHADER_STORAGE_BLOCK,	"GL_SHADER_STORAGE_BLOCK"},
	};

	for (int categoryIndex = 0; categoryIndex < SIZE_OF_ARRAY(s_categories); ++categoryIndex) {
		GLint numActiveInterfaces = 0;
		glExtGetProgramInterfaceiv(
			/* GLuint program */			programId,
			/* GLenum programInterface */	s_categories[categoryIndex].programInterface,
			/* GLenum pname */				GL_ACTIVE_RESOURCES,
			/* GLint * params */			&numActiveInterfaces
		);
		for (int interfaceIndex = 0; interfaceIndex < numActiveInterfaces; ++interfaceIndex) {
			char resourceName[0x100];
			glExtGetProgramResourceName(
				/* GLuint program */			programId,
				/* GLenum programInterface */	s_categories[categoryIndex].programInterface,
				/* GLuint index */				interfaceIndex,
				/* GLsizei bufSize */			sizeof(resourceName),
				/* GLsizei * length */			NULL,
				/* char * name */				&resourceName[0]
			);

			switch (s_categories[categoryIndex].programInterface) {
				case GL_UNIFORM: {
					GLenum properties[3] = {
						GL_TYPE,
						GL_OFFSET,
						GL_LOCATION,
					};
					GLint values[3];
					glExtGetProgramResourceiv(
						/* GLuint program */			programId,
						/* GLenum programInterface */	s_categories[categoryIndex].programInterface,
						/* GLuint index */				interfaceIndex,
						/* GLsizei propCount */			SIZE_OF_ARRAY(properties),
						/* const GLenum * props */		&properties[0],
						/* GLsizei bufSize */			SIZE_OF_ARRAY(values),		/* これ sizeof(values) */
						/* GLsizei * length */			NULL,
						/* GLint * params */			&values[0]
					);
					printf(
						"	%s, index %d, type 0x%04X, offset %d, location %d, name %s\n",
						s_categories[categoryIndex].name,
						interfaceIndex,
						values[0],
						values[1],
						values[2],
						&resourceName[0]
					);
				} break;

				case GL_SHADER_STORAGE_BLOCK: {
					GLenum properties[2] = {
						GL_BUFFER_BINDING,
						GL_BUFFER_DATA_SIZE,
					};
					GLint values[2];
					glExtGetProgramResourceiv(
						/* GLuint program */			programId,
						/* GLenum programInterface */	s_categories[categoryIndex].programInterface,
						/* GLuint index */				interfaceIndex,
						/* GLsizei propCount */			SIZE_OF_ARRAY(properties),
						/* const GLenum * props */		&properties[0],
						/* GLsizei bufSize */			SIZE_OF_ARRAY(values),		/* これ sizeof(values) */
						/* GLsizei * length */			NULL,
						/* GLint * params */			&values[0]
					);
					printf(
						"	%s, index %d, buffer binding %d, buffer data size %d, name %s\n",
						s_categories[categoryIndex].name,
						interfaceIndex,
						values[0],
						values[1],
						&resourceName[0]
					);
				} break;
			}
		}
	}
}


bool
ExistsShaderUniform(
	GLuint	programId,
	GLint	location,
	GLint	typeEnum
){
	GLenum properties[2] = {GL_TYPE, GL_LOCATION};
	GLint values[2];

	GLenum programInterface = GL_UNIFORM;
	GLint numActiveInterfaces = 0;
	glExtGetProgramInterfaceiv(
		/* GLuint program */			programId,
		/* GLenum programInterface */	programInterface,
		/* GLenum pname */				GL_ACTIVE_RESOURCES,
		/* GLint * params */			&numActiveInterfaces
	);
	for (int interfaceIndex = 0; interfaceIndex < numActiveInterfaces; ++interfaceIndex) {
		glExtGetProgramResourceiv(
			/* GLuint program */			programId,
			/* GLenum programInterface */	programInterface,
			/* GLuint index */				interfaceIndex,
			/* GLsizei propCount */			SIZE_OF_ARRAY(properties),
			/* const GLenum * props */		&properties[0],
			/* GLsizei bufSize */			SIZE_OF_ARRAY(values),
			/* GLsizei * length */			NULL,
			/* GLint * params */			&values[0]
		);
		if (values[0] == typeEnum
		&&	values[1] == location
		) {
			return true;
		}
	}
	return false;
}


void CheckGlError(
	const char *string
){
	GLenum errorCode = glGetError();
	if (errorCode != GL_NO_ERROR) {
		do {
			const char *msg = "";
			switch (errorCode) {
				case GL_INVALID_ENUM:      msg = "INVALID_ENUM";      break;
				case GL_INVALID_VALUE:     msg = "INVALID_VALUE";     break;
				case GL_INVALID_OPERATION: msg = "INVALID_OPERATION"; break;
				case GL_OUT_OF_MEMORY:     msg = "OUT_OF_MEMORY";     break;
				case GL_INVALID_FRAMEBUFFER_OPERATION:  msg = "INVALID_FRAMEBUFFER_OPERATION"; break;
				default: msg = "Unknown"; break;
			}
			printf("%s : GL ERROR : %s\n", string, msg);
			errorCode = glGetError();
		} while (errorCode != GL_NO_ERROR);
	}
}


size_t GetFileSize(
	const char *fileName
){
	FILE *file = fopen(fileName, "rb");
	if (file == NULL) return 0;

	struct stat fileStat;
	if (fstat(_fileno(file), &fileStat) == -1) {
		fclose(file);
		return 0;
	}
	fclose(file);

	return (size_t)fileStat.st_size;
}


char *MallocCopyString(
	const char *string
){
	size_t sizeInBytes = strlen(string) + 1;	/* 末端 \0 分余分に確保 */
	char *p = (char *)malloc(sizeInBytes);
	memcpy(p, string, sizeInBytes);
	return p;
}


char *MallocReadTextFile(
	const char *fileName
){
	char *buff = NULL;

	/*
		注意
		ここで、"rt" で読み込むと不正な動作となる。
		テキスト読み込みでは、\n を \r\n に置換する動作が入るため、実際の
		ファイルサイズよりも、メモリ読み込み後の方が肥大する可能性がある。
		"rb" にすることで、この問題は解消される。
	*/
	FILE *file = fopen(fileName, "rb");
	if (file == NULL) return NULL;

	struct stat fileStat;
	if (fstat(_fileno(file), &fileStat) == -1) {
		fclose(file);
		return NULL;
	}
	assert(fileStat.st_size < 0x100000000LL);
	uint32_t size = (uint32_t)fileStat.st_size;

	buff = (char *)malloc(size + 1 /* 終点 \0 分 */);
	if (buff == NULL) {
		fclose(file);
		return NULL;
	}

	fread(buff, 1, size, file);
	buff[size] = '\0';	/* 終点に \0 を書き込む */

	fclose(file);

	return buff;
}

char *StrFindChars(
	char *string,
	const char *chars
){
	char *p = string;

	for (;*p != '\0'; ++p) {
		bool found = false;
		for (const char *q = chars; *q != '\0'; ++q) {
			if (*p == *q) {found = true; break;}
		}
		if (found) return p;
	}

	return NULL;
}

char *StrSkipChars(
	char *string,
	const char *chars
){
	char *p = string;

	for (;*p != '\0'; ++p) {
		bool found = false;
		for (const char *q = chars; *q != '\0'; ++q) {
			if (*p == *q) {found = true; break;}
		}
		if (found == false) break;
	}

	return p;
}

char *SkipBom(
	char *string
){
	/* BOM をスキップ */
	char *p = string;
	if (strstr(p, "\xEF\xBB\xBF") == p) {
		p += 3;
	} else
	if (strstr(p, "\xFE\xFF") == p) {
		p += 2;
	} else
	if (strstr(p, "\xFF\xFE") == p) {
		p += 2;
	}
	return p;
}
const char *SkipBomConst(
	const char *string
){
	/* BOM をスキップ */
	const char *p = string;
	if (strstr(p, "\xEF\xBB\xBF") == p) {
		p += 3;
	} else
	if (strstr(p, "\xFE\xFF") == p) {
		p += 2;
	} else
	if (strstr(p, "\xFF\xFE") == p) {
		p += 2;
	}
	return p;
}

static int CALLBACK SHBrowseProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	/* 初期状態のパスを設定 */
	if (uMsg == BFFM_INITIALIZED && lpData) {
		SendMessage(hWnd, BFFM_SETSELECTION, TRUE, lpData);
	}
	return 0;
}
bool SelectDirectory(
	const char *title,
	const char *defaultDirectoryName,
	char *selectedDirectoryName,
	size_t selectedDirectoryNameSizeInBytes
){
	BROWSEINFO browseInfo = {0};
	browseInfo.lpfn			= SHBrowseProc;					/* コールバック関数 */
	browseInfo.lParam		= (LPARAM)defaultDirectoryName;	/* コールバック関数に渡される引数 */
	browseInfo.lpszTitle	= title;
	browseInfo.ulFlags		= BIF_RETURNONLYFSDIRS;

	LPITEMIDLIST itemIdList = SHBrowseForFolder(&browseInfo);
	if (itemIdList) {
		/* 選択されたパスを取得 */
		char path[_MAX_PATH];
		SHGetPathFromIDList(itemIdList, path);
		CoTaskMemFree(itemIdList);
		if (strlen(path) + 1 /* 末端 \0 分 */ < selectedDirectoryNameSizeInBytes) {
			strcpy_s(selectedDirectoryName, selectedDirectoryNameSizeInBytes, path);
			return true;
		} else {
			return false;
		}
	}
	return false;
}

BOOL SetDlgItemFloat(
	HWND hDlg,
	int nIDDlgItem,
	float fValue,
	BOOL bSigned
){
	if (bSigned == false && fValue < 0) return false;
	char valueAsString[0x100];
	snprintf(valueAsString, sizeof(valueAsString), "%f", fValue);
	if (SetDlgItemText(hDlg, nIDDlgItem, valueAsString) == FALSE) return false;
	return true;
}

float GetDlgItemFloat(
	HWND hDlg,
	int nIDDlgItem,
	BOOL *lpTranslated,
	BOOL bSigned
){
	char valueAsString[0x100] = {0};
	GetDlgItemText(hDlg, nIDDlgItem, valueAsString, sizeof(valueAsString));
	char *ep;
	float fValue = strtof(valueAsString, &ep);
	if (*ep != '\0') return false;
	if (lpTranslated != NULL) {
		*lpTranslated = TRUE;
		if (bSigned == false && fValue < 0) {
			*lpTranslated = FALSE;
		}
	}
	return fValue;
}

void SetDlgItemCheck(
	HWND hDlg,
	int nIDDlgItem,
	BOOL check
){
	SendDlgItemMessage(
		hDlg,
		nIDDlgItem,
		BM_SETCHECK,
		(WPARAM)check? 1: 0,
		(LPARAM)0
	);
}

BOOL GetDlgItemCheck(
	HWND hDlg,
	int nIDDlgItem
){
	return (BOOL)SendDlgItemMessage(
		hDlg,
		nIDDlgItem,
		BM_GETCHECK,
		(WPARAM)0,
		(LPARAM)0
	);
}
