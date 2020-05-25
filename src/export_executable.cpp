/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#include "config.h"
#include "common.h"
#include "app.h"
#include "dialog_confirm_over_write.h"
#include "export_executable.h"


#define USE_MAIN_CPP	1


/*=============================================================================
▼	ディレクトリを強制的に作成する
-----------------------------------------------------------------------------*/
static bool
ForceCreateDirectory(
	const char *dirFullPath
){
	printf("create directory %s.\n", dirFullPath);
	BOOL ret = CreateDirectoryA(dirFullPath, NULL);
	if (ret == FALSE) {
		if (GetLastError() != ERROR_ALREADY_EXISTS) {
			AppErrorMessageBox(APP_NAME, "Failed to create %s.", dirFullPath);
			return false;
		}
	}
	return true;
}


/*=============================================================================
▼	リソースからファイルにコピー
-----------------------------------------------------------------------------*/
static bool
CopyFileFromResource(
	LPCSTR  lpName,
	LPCSTR  lpType,
	const char *dstFullPath
){
	HRSRC hResource = FindResourceA(
		/* HMODULE hModule */	NULL,		/* 実行ファイル自身の場合は NULL */
		/* LPCSTR  lpName */	lpName,
		/* LPCSTR  lpType */	lpType
	);
	if (hResource == NULL) {
		AppErrorMessageBox(APP_NAME, "Can not find resource %s %s.", lpName, lpType);
		return false;
	}
	size_t sizeInBytes = SizeofResource(
		/* HMODULE hModule */	NULL,		/* 実行ファイル自身の場合は NULL */
		/* HRSRC   hResInfo */	hResource
	);
	HGLOBAL hGlobal = LoadResource(
		/* HMODULE hModule */	NULL,		/* 実行ファイル自身の場合は NULL */
		/* HRSRC   hResInfo */	hResource
	);
	FILE *file = fopen(dstFullPath, "wb");
	if (file == NULL) {
		AppErrorMessageBox(APP_NAME, "Can not open %s.", dstFullPath);
		return false;
	}
	fwrite((const void *)hGlobal, 1, sizeInBytes, file);
	fclose(file);
	return true;
}


/*=============================================================================
▼	コマンドラインの実行
-----------------------------------------------------------------------------*/
static DWORD
ExecuteCommandLine(
	char *commandLine,
	const char *currentDirectory
){
	/* エラー情報をクリアする */
	SetLastError(NO_ERROR);

	/* プロセスの起動 */
	STARTUPINFO startUpInfo = {0};
	PROCESS_INFORMATION processInformation = {0};
	BOOL ret = CreateProcess(
		/* LPCSTR                lpApplicationName */		NULL,
		/* LPSTR                 lpCommandLine */			commandLine,
		/* LPSECURITY_ATTRIBUTES lpProcessAttributes */		NULL,
		/* LPSECURITY_ATTRIBUTES lpThreadAttributes */		NULL,
		/* BOOL                  bInheritHandles */			FALSE,
		/* DWORD                 dwCreationFlags */			CREATE_NO_WINDOW * 0,
		/* LPVOID                lpEnvironment */			NULL,
		/* LPCSTR                lpCurrentDirectory */		currentDirectory,
		/* LPSTARTUPINFOA        lpStartupInfo */			&startUpInfo,
		/* LPPROCESS_INFORMATION lpProcessInformation */	&processInformation
	);
	if (ret == false) {
		AppLastErrorMessageBox(APP_NAME);
	}

	/* プロセスの終了を待機する */
	DWORD exitCode = 0;
	CloseHandle(processInformation.hThread);
	if (WaitForSingleObject(processInformation.hProcess,INFINITE) == WAIT_OBJECT_0) {
		GetExitCodeProcess(processInformation.hProcess, &exitCode);
	}
	CloseHandle(processInformation.hProcess);

	/* 正常コード */
	return exitCode;
}


/*=============================================================================
▼	バッチファイルの実行
-----------------------------------------------------------------------------*/
static DWORD
ExecuteBat(
	const char *fileName,
	const char *currentDirectory
){
	char commandLine[MAX_PATH + 100];
	snprintf(
		commandLine,
		sizeof(commandLine),
		"cmd.exe /c %s",
		fileName
	);
	return ExecuteCommandLine(commandLine, currentDirectory);
}


/*=============================================================================
▼	指定ファイルの末端カンマを除去
-----------------------------------------------------------------------------*/
static bool
RemoveComma(
	const char *fileName
){
	char *fileImage = MallocReadTextFile(fileName);
	if (fileImage == NULL) {
		AppErrorMessageBox(APP_NAME, "Failed to open %s.", fileName);
		return false;
	}

	for (size_t i = strlen(fileImage); i != 0; i--) {
		if (fileImage[i] == ',') {
			fileImage[i] =  ' ';
			break;
		}
	}

	{
		FILE *file = fopen(fileName, "wt");
		if (file == NULL) {
			AppErrorMessageBox(APP_NAME, "Failed to open %s.", fileName);
			return false;
		}
		fputs(fileImage, file);
		fclose(file);
	}

	free(fileImage);

	return true;
}


/*=============================================================================
▼	version ディレクティブの除去と保存
-----------------------------------------------------------------------------*/
static bool
RemoveAndSaveVersionDirective(
	const char *fileName,
	char *versionDirectiveBuffer,
	size_t versionDirectiveBufferSizeInBytes
){
	char *fileImage = MallocReadTextFile(fileName);
	if (fileImage == NULL) {
		AppErrorMessageBox(APP_NAME, "Failed to open %s.", fileName);
		return false;
	}
	char *p = fileImage;

	/* BOM をスキップ */
	p = SkipBom(p);

	/* 1 行目に書かれている version デレクティブを探し、除去と保存 */
	{
		/* 行頭スペースのスキップ */
		p = StrSkipChars(p, " \t");

		/* 現在位置に version ディレクティブが存在するか？ */
		if (strstr(p, "#version") == p) { 
			/* 退避（改行コードを含まない）*/
			strcpy_s(versionDirectiveBuffer, versionDirectiveBufferSizeInBytes, p);
			char *q = StrFindChars(versionDirectiveBuffer, "\r\n");
			if (q != NULL) *q = '\0';

			/* コメントアウト */
			p[0] = '/';
			p[1] = '/';
		}
	}

	{
		FILE *file = fopen(fileName, "wt");
		if (file == NULL) {
			AppErrorMessageBox(APP_NAME, "Failed to open %s.", fileName);
			free(fileImage);
			return false;
		}
		fputs(fileImage, file);
		fclose(file);
	}

	free(fileImage);

	return true;
}


/*=============================================================================
▼	ワークアラウンドの適用
-----------------------------------------------------------------------------*/
static bool
ApplyWorkAround(
	const char *fileName
){
	char *fileImage = MallocReadTextFile(fileName);

	FILE *file = fopen(fileName, "wt");
	if (file == NULL) {
		AppErrorMessageBox(APP_NAME, "Failed to open %s.", fileName);
		free(fileImage);
		return false;
	}

	/* ステート */
	enum {
		StateOutsideWorkAround,
		StateInsideWorkAround,
	} state = StateOutsideWorkAround;

	char *p = fileImage;
	char replace[0x100] = {0};
	for (;;) {
		bool eof = false;

		/* 行頭空白のスキップ */
		p = StrSkipChars(p, "\r\n");

		/* 行頭と行末 */
		char *beginOfLine = p;
		char *endOfLine = StrFindChars(p, "\r\n");
		if (endOfLine == NULL) {
			endOfLine = beginOfLine + strlen(beginOfLine);
			eof = true;
		}

		/* ワークアラウンドの始点を認識 */
		if (state == StateOutsideWorkAround) {
			const char search[] = "#pragma work_around_begin";
			char *found = strstr(p, search);
			if (beginOfLine <= found && found < endOfLine) {
				state = StateInsideWorkAround;

				/* 置換パターン取得 */
				p = found + strlen(search);
				p = StrSkipChars(p, " \t");
				if (*p == ':') {
					p++;
					size_t length = strstr(p, "\\n\"") - p;
					if (length > sizeof(replace) - 1) length = sizeof(replace) - 1;
					memcpy(replace, p, length);
					printf("replace [%s]\n", replace);
				}

				/* スキップ */
				p = endOfLine;
				continue;
			}
		}

		/* ワークアラウンドの終点を認識 */
		if (state == StateInsideWorkAround) {
			char *found = strstr(p, "#pragma work_around_end");
			if (beginOfLine <= found && found < endOfLine) {
				state = StateOutsideWorkAround;

				/* スキップ */
				p = endOfLine;
				continue;
			}
		}

		/* ワークアラウンド内 */
		if (state == StateInsideWorkAround) {
			char varName[0x100];
			char *found = strstr(p, "vec2 ");
			if (beginOfLine <= found && found < endOfLine) {
				p = found + strlen("vec2 ");
				char *varBegin = p;
				found = strstr(p, "[];");
				if (beginOfLine <= found && found < endOfLine) {
					char *varEnd = found;
					*varEnd = '\0';
					/*
						vec2 %s[];
						は、minifier により 
						vec2%s[];
						のように短縮されてしまう。
						そのため varName 先頭にスペースを一文字挿入している。
					*/
					snprintf(varName, sizeof(varName), " %s", varBegin);
					printf("work around var name = [%s]\n", varName);

					/* ワークアラウンドコードに置き換え */
					if (state == StateInsideWorkAround) {
						fprintf(file, " \"");
						fprintf(file, replace, varName);
						fprintf(file, "\"\n");
					}

					/* スキップ */
					p = endOfLine;
					continue;
				}
			}
		}

		/* 一行出力 */
		*endOfLine = '\0';
		fprintf(file, "%s\n", beginOfLine);

		/* ファイル末端を検出していたら終了 */
		if (eof) break;

		/* 改行スキップ */
		p = StrSkipChars(endOfLine + 1, "\r\n");
	}

	fclose(file);

	return true;
}


/*=============================================================================
▼	exe ファイルのエクスポート（下請け）
-----------------------------------------------------------------------------*/
bool ExportExecutableSub(
	const char *workDirName,
	const char *graphicsShaderCode,
	const char *soundShaderCode,
#if USE_MAIN_CPP
	const char *mainCppFullPath,
	const char *glextHeaderFullPath,
	const char *khrplatformHeaderFullPath,
#else
	const char *mainAsmFullPath,
#endif
	const char *mainObjFullPath,
	const char *configHeaderFullPath,
	const char *resourceCppFullPath,
	const char *resourceObjFullPath,
	const char *graphicsFragmentShaderGlslFullPath,
	const char *graphicsFragmentShaderInlFullPath,
	const char *soundComputeShaderGlslFullPath,
	const char *soundComputeShaderInlFullPath,
	const char *crinklerReportFullPath,
	const char *crinklerReuseFullPath,
	const char *minifyBatFullPath,
	const char *buildBatFullPath,
	const char *outputGraphicsFragmentShaderInlFullPath,
	const char *outputSoundComputeShaderInlFullPath,
	const RenderSettings *renderSettings,
	const ExecutableExportSettings *executableExportSettings
){
	/* main.asm/main.cpp 生成 */
#if USE_MAIN_CPP
	CopyFileFromResource("IDR_MAIN_CPP", "MY_EMBEDDED_RESOURCE", mainCppFullPath);
	CopyFileFromResource("IDR_GLEXT_H", "MY_EMBEDDED_RESOURCE", glextHeaderFullPath);
	CopyFileFromResource("IDR_KHRPLATFORM_H", "MY_EMBEDDED_RESOURCE", khrplatformHeaderFullPath);
#else
	CopyFileFromResource("IDR_MAIN_ASM", "MY_EMBEDDED_RESOURCE", mainAsmFullPath);
#endif

	/* config.h 生成 */
	CopyFileFromResource("IDR_CONFIG_H", "MY_EMBEDDED_RESOURCE", configHeaderFullPath);

	/* resource.cpp 生成 */
	CopyFileFromResource("IDR_RESOURCE_CPP", "MY_EMBEDDED_RESOURCE", resourceCppFullPath);

	/* graphics_fragment_shader.glsl 生成 */
	{
		printf("generate %s.\n", graphicsFragmentShaderGlslFullPath);
		FILE *file = fopen(graphicsFragmentShaderGlslFullPath, "wt");
		if (file == NULL) {
			AppErrorMessageBox(APP_NAME, "Failed to generate %s.", graphicsFragmentShaderGlslFullPath);
			return false;
		}

		fprintf(
			file,
			"%s\n",
			graphicsShaderCode
		);
		fclose(file);
	}

	/* sound_compute_shader.glsl 生成 */
	{
		printf("generate %s.\n", soundComputeShaderGlslFullPath);
		FILE *file = fopen(soundComputeShaderGlslFullPath, "wt");
		if (file == NULL) {
			AppErrorMessageBox(APP_NAME, "Failed to generate %s.", soundComputeShaderGlslFullPath);
			return false;
		}

		fprintf(
			file,
			"%s\n",
			soundShaderCode
		);
		fclose(file);
	}

	/* version ディレクティブの除去と保存 */
	char graphicsFragmentShaderVersionDirectiveBuffer[0x1000];
	char soundComputeShaderVersionDirectiveBuffer[0x1000];
	if (RemoveAndSaveVersionDirective(
			graphicsFragmentShaderGlslFullPath,
			graphicsFragmentShaderVersionDirectiveBuffer,
			sizeof(graphicsFragmentShaderVersionDirectiveBuffer)
		) == false
	) {
		return false;
	}
	if (RemoveAndSaveVersionDirective(
			soundComputeShaderGlslFullPath,
			soundComputeShaderVersionDirectiveBuffer,
			sizeof(soundComputeShaderVersionDirectiveBuffer)
		) == false
	) {
		return false;
	}

	/* VisualStudio コマンドプロンプトを起動する定型文（失敗したら exit /b 1）*/
	#define OPEN_DEVELOPER_COMMAND_PROMPT \
		"@echo off\n"\
		"setlocal \n"\
		\
		/* vswhere.exe の場所を探す */\
		"if exist \"%%ProgramFiles%%\\Microsoft Visual Studio\\Installer\\vswhere.exe\" set ProgFile=%%ProgramFiles%%\n"\
		"if exist \"%%ProgramFiles(x86)%%\\Microsoft Visual Studio\\Installer\\vswhere.exe\" set ProgFile=%%ProgramFiles(x86)%%\n"\
		\
		/* vswhere.exe を使って VisualStudio のインストールディレクトリを調べる */\
		"for /f \"usebackq tokens=*\" %%%%i in (`\"%%ProgFile%%\\Microsoft Visual Studio\\Installer\\vswhere.exe\" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (\n"\
		"  set InstallDir=%%%%i\n"\
		")\n"\
		\
		/* インストールディレクトリが特定できないならエラー終了 */\
		"if not defined InstallDir exit /b 1\n" \
		\
		/* VisualStudio 環境変数設定バッチを起動 */\
		"if not defined VSINSTALLDIR (\n"\
		"	endlocal & call \"%%InstallDir%%\\Common7\\Tools\\VsDevCmd.bat\"\n"\
		")\n"\
		\
		"endlocal \n"\
		"@echo on\n"\

	/* minify.bat 生成 */
	{
		char shaderMinifierOptions[0x100] = {0};
		snprintf(
			shaderMinifierOptions,
			sizeof(shaderMinifierOptions),
			"%s%s%s",
			(executableExportSettings->shaderMinifierOptions.noRenaming? "--no-renaming ": ""),
			(executableExportSettings->shaderMinifierOptions.noSequence? "--no-sequence ": ""),
			(executableExportSettings->shaderMinifierOptions.smoothstep? "--smoothstep ": "")
		);

		printf("generate %s.\n", minifyBatFullPath);
		FILE *file = fopen(minifyBatFullPath, "wt");
		if (file == NULL) {
			AppErrorMessageBox(APP_NAME, "Failed to generate %s.", minifyBatFullPath);
			return false;
		}
		fprintf(
			file,

			/* VisualStudio コマンドプロンプトを起動（失敗したら exit /b 1）*/
			OPEN_DEVELOPER_COMMAND_PROMPT

			/*
				プリプロセッサの適用
				graphics_fragment_shader.glsl -> graphics_fragment_shader.i
				sound_compute_shader.glsl -> sound_compute_shader.i

				cl の引数
					/P	ファイルに前処理します
					/EP	stdout に前処理します。#line なし
					/D<name>{=|#}<text> マクロを定義します
			*/
			"cl /P /EP /DEXPORT_EXECUTABLE=1 /DSCREEN_XRESO=%d /DSCREEN_YRESO=%d graphics_fragment_shader.glsl || exit /b 2\n"
			"cl /P /EP /DEXPORT_EXECUTABLE=1 /DSCREEN_XRESO=%d /DSCREEN_YRESO=%d sound_compute_shader.glsl || exit /b 3\n"

			/*
				version ディレクティブの復元
			*/
			"echo %s > graphics_fragment_shader.version\n"
			"echo %s > sound_compute_shader.version\n"
			"copy /b graphics_fragment_shader.version + graphics_fragment_shader.i graphics_fragment_shader.tmp\n"
			"copy /b sound_compute_shader.version + sound_compute_shader.i sound_compute_shader.tmp\n"
			"del graphics_fragment_shader.version\n"
			"del sound_compute_shader.version\n"
			"del graphics_fragment_shader.i\n"
			"del sound_compute_shader.i\n"
			"rename graphics_fragment_shader.tmp graphics_fragment_shader.i\n"
			"rename sound_compute_shader.tmp sound_compute_shader.i\n"

			/*
				shader_minifier の存在チェック
			*/
			"where shader_minifier.exe || exit /b 4\n"

			/*
				graphics_fragment_shader.i -> graphics_fragment_shader.inl
				sound_compute_shader.i -> sound_compute_shader.inl
			*/
			"shader_minifier %s graphics_fragment_shader.i -o graphics_fragment_shader.inl --format c-array || exit /b 5\n"
			"shader_minifier %s sound_compute_shader.i -o sound_compute_shader.inl --format c-array || exit /b 6\n"
			,
			executableExportSettings->xReso, executableExportSettings->yReso,
			executableExportSettings->xReso, executableExportSettings->yReso,
			graphicsFragmentShaderVersionDirectiveBuffer,
			soundComputeShaderVersionDirectiveBuffer,
			shaderMinifierOptions,
			shaderMinifierOptions
		);
		fclose(file);
	}

	/* minify.bat 実行 */
	{
		DWORD exitCode = ExecuteBat(minifyBatFullPath, workDirName);
		if (exitCode != 0) {
			switch (exitCode) {
				case 1: {
					AppErrorMessageBox(APP_NAME, "Failed to detect the directory where Visual Studio is installed.");
				} break;
				case 2: {
					AppErrorMessageBox(APP_NAME, "Failed to pre-process graphics fragment shader.");
				} break;
				case 3: {
					AppErrorMessageBox(APP_NAME, "Failed to pre-process sound compute shader.");
				} break;
				case 4: {
					AppErrorMessageBox(
						APP_NAME,
						"Failed to execute shader_minifier.exe.\n"
						"Please install it and edit your PATH to include the installed directory.\n"
						"\n"
						"https://github.com/laurentlb/Shader_Minifier"
					);
				} break;
				case 5: {
					AppErrorMessageBox(APP_NAME, "Failed to minify graphics fragment shader.");
				} break;
				case 6: {
					AppErrorMessageBox(APP_NAME, "Failed to minify sound compute shader.");
				} break;
				default : {
					assert(false);
				} break;
			}
			return false;
		}
	}

	/* 末端カンマの除去	*/
	if (RemoveComma(graphicsFragmentShaderInlFullPath) == false) {
		return false;
	};
	if (RemoveComma(soundComputeShaderInlFullPath) == false) {
		return false;
	}

	/* ワークアラウンドの適用 */
	if (ApplyWorkAround(graphicsFragmentShaderInlFullPath) == false) {
		return false;
	}
	if (ApplyWorkAround(soundComputeShaderInlFullPath) == false) {
		return false;
	}

	/* build.bat 生成 */
	{
		char crinklerOptions[0x100] = {0};
		static const char *(s_compModes[]) = {
			"INSTANT",
			"FAST",
			"SLOW",
			"VERYSLOW"
		};
		assert(executableExportSettings->crinklerOptions.compMode < SIZE_OF_ARRAY(s_compModes));
		snprintf(
			crinklerOptions,
			sizeof(crinklerOptions),
			"/COMPMODE:%s %s%s",
			s_compModes[executableExportSettings->crinklerOptions.compMode],
			(executableExportSettings->crinklerOptions.useTinyHeader? "/TINYHEADER ": ""),
			(executableExportSettings->crinklerOptions.useTinyImport? "/TINYIMPORT ": "")
		);

		int numRenderTargets = renderSettings->enableMultipleRenderTargets? renderSettings->numEnabledRenderTargets: 1;
		int numMipmapLevels = CalcNumMipmapLevelsFromResolution(
			executableExportSettings->xReso,
			executableExportSettings->yReso
		);

		/*
			frameCount uniform 変数は、location 固定で引き渡す。
			シェーダ側に該当 uniform の宣言を持たない場合、この操作は
			GPU 側のデスクリプタを破壊することになり予想外の動作を起こす。
			エクスポートに先立って、シェーダ側に該当 uniform が存在するか確認し、
			存在しないなら、frameCount uniform は無効とする。
		*/
		bool enableFrameCountUniform = executableExportSettings->enableFrameCountUniform?
			GraphicsShaderRequiresFrameCountUniform(): false;

		printf("generate %s.\n", buildBatFullPath);
		FILE *file = fopen(buildBatFullPath, "wt");
		if (file == NULL) {
			AppErrorMessageBox(APP_NAME, "Failed to generate %s.", buildBatFullPath);
			return false;
		}
		fprintf(
			file,

			/* VisualStudio コマンドプロンプトを起動（失敗したら exit /b 1）*/
			OPEN_DEVELOPER_COMMAND_PROMPT

#if USE_MAIN_CPP
			/* main.cpp -> main.obj */
			"cl.exe "
				"/c "			/* コンパイルのみ。リンクは行わない */
				"/w "			/* 警告をすべて無効にします */
				"/O1 "			/* 最大限の最適化 (スペースを優先) */
				"/Os "			/* コード スペースを優先する */
				"/Oy "			/* フレーム ポインターの省略を有効にする */
				"/GS- "			/* セキュリティ チェックを無効にする */
				"/arch:IA32 "
				"/DARG_SCREEN_WIDTH=%d "							/* arg 1 */
				"/DARG_SCREEN_HEIGHT=%d "							/* arg 2 */
				"/DARG_NUM_SOUND_BUFFER_SAMPLES=%d "				/* arg 3 */
				"/DARG_NUM_SOUND_BUFFER_AVAILABLE_SAMPLES=%d "		/* arg 4 */
				"/DARG_NUM_SOUND_BUFFER_SAMPLES_PER_DISPATCH=%d "	/* arg 5 */
				"/DARG_ENABLE_SWAP_INTERVAL_CONTROL=%d "			/* arg 6 */
				"/DARG_SWAP_INTERVAL=%d "							/* arg 7 */
				"/DARG_ENABLE_BACK_BUFFER=%d "						/* arg 8 */
				"/DARG_ENABLE_FRAME_COUNT_UNIFORM=%d "				/* arg 9 */
				"/DARG_ENABLE_MIPMAP_GENERATION=%d "				/* arg 10 */
				"/DARG_NUM_MIPMAP_LEVELS=%d "						/* arg 11 */
				"/DARG_NUM_RENDER_TARGETS=%d "						/* arg 12 */
				"/DARG_PIXEL_FORMAT=%d "							/* arg 13 */
				"/DARG_TEXTURE_FILTER=%d "							/* arg 14 */
				"/DARG_TEXTURE_WRAP=%d "							/* arg 15 */
				"/DARG_ENABLE_SOUND_DISPATCH_WAIT=%d "				/* arg 16 */
				"/DARG_USE_TINYHEADER=%d "							/* arg 17 */
				"/DARG_USE_TINYIMPORT=%d "							/* arg 18 */
				"/I%s "												/* arg 19 */
				"main.cpp || exit /b 2\n"
#else
			/* main.asm -> main.obj */
			"ml.exe "
				"/c "			/* Assemble without linking */
				"/DARG_SCREEN_WIDTH=%d "							/* arg 1 */
				"/DARG_SCREEN_HEIGHT=%d "							/* arg 2 */
				"/DARG_NUM_SOUND_BUFFER_SAMPLES=%d "				/* arg 3 */
				"/DARG_NUM_SOUND_BUFFER_AVAILABLE_SAMPLES=%d "		/* arg 4 */
				"/DARG_NUM_SOUND_BUFFER_SAMPLES_PER_DISPATCH=%d "	/* arg 5 */
				"/DARG_ENABLE_SWAP_INTERVAL_CONTROL=%d "			/* arg 6 */
				"/DARG_SWAP_INTERVAL=%d "							/* arg 7 */
				"/DARG_ENABLE_BACK_BUFFER=%d "						/* arg 8 */
				"/DARG_ENABLE_FRAME_COUNT_UNIFORM=%d "				/* arg 9 */
				"/DARG_ENABLE_MIPMAP_GENERATION=%d "				/* arg 10 */
				"/DARG_NUM_MIPMAP_LEVELS=%d "						/* arg 11 */
				"/DARG_NUM_RENDER_TARGETS=%d "						/* arg 12 */
				"/DARG_PIXEL_FORMAT=%d "							/* arg 13 */
				"/DARG_TEXTURE_FILTER=%d "							/* arg 14 */
				"/DARG_TEXTURE_WRAP=%d "							/* arg 15 */
				"/DARG_ENABLE_SOUND_DISPATCH_WAIT=%d "				/* arg 16 */
				"/DARG_USE_TINYHEADER=%d "							/* arg 17 */
				"/DARG_USE_TINYIMPORT=%d "							/* arg 18 */
				"main.asm || exit /b 2\n"

			/* resource.cpp -> resource.obj */
			"cl.exe "
				"/c "			/* コンパイルのみ。リンクは行わない */
				"/w "			/* 警告をすべて無効にします */
				"/DEXPORT_EXECUTABLE=1 "
				"/DARG_ENABLE_SWAP_INTERVAL_CONTROL=%d "			/* arg 19 */
				"/DARG_ENABLE_BACK_BUFFER=%d "						/* arg 20 */
				"/DARG_ENABLE_FRAME_COUNT_UNIFORM=%d "				/* arg 21 */
				"/DARG_ENABLE_MIPMAP_GENERATION=%d "				/* arg 22 */
				"/DARG_NUM_MIPMAP_LEVELS=%d "						/* arg 23 */
				"/DARG_NUM_RENDER_TARGETS=%d "						/* arg 24 */
				"/DARG_PIXEL_FORMAT=%d "							/* arg 25 */
				"/DARG_TEXTURE_FILTER=%d "							/* arg 26 */
				"/DARG_TEXTURE_WRAP=%d "							/* arg 27 */
				"/DARG_ENABLE_SOUND_DISPATCH_WAIT=%d "				/* arg 28 */
				"/DARG_USE_TINYHEADER=%d "							/* arg 29 */
				"/DARG_USE_TINYIMPORT=%d "							/* arg 30 */
				"resource.cpp || exit /b 3\n"
#endif

			/* crinkler の存在チェック */
			"where crinkler.exe || exit /b 4\n"

			/* main.obj -> main.exe */
			"crinkler.exe "
				"OPENGL32.LIB WINMM.LIB KERNEL32.LIB USER32.LIB GDI32.LIB "
				"/SUBSYSTEM:WINDOWS "
				"/ENTRY:entrypoint "
				"/UNSAFEIMPORT "
				"/CRINKLER "
				"/HASHTRIES:300 "
				"/ORDERTRIES:300 "
				"/UNALIGNCODE "
				"/PRINT:LABELS "
				"/PRINT:IMPORTS "
				"/PRINT:MODELS "
				"/REPORT:%s "			/* crinkler arg 1 */
				"/PROGRESSGUI "
				"/NOINITIALIZERS "
				"/HASHSIZE:300 "
				"/OVERRIDEALIGNMENTS "
				"/REUSE:%s "			/* crinkler arg 2 */
				"/REUSEMODE:IMPROVE "
				"%s"					/* crinkler arg 3 */
				"/OUT:%s "				/* crinkler arg 4 */
#if USE_MAIN_CPP == 0
				"resource.obj "
#endif
				"main.obj "
			"|| exit /b 5\n"

			/* 生成結果は exe のディレクトリにもコピーされる */
			"copy graphics_fragment_shader.inl %s || exit /b 6\n"	/* crinkler arg 5 */
			"copy sound_compute_shader.inl %s || exit /b 7\n"		/* crinkler arg 6 */
			,
			executableExportSettings->xReso,								/* arg 1 */
			executableExportSettings->yReso,								/* arg 2 */
			executableExportSettings->numSoundBufferSamples,				/* arg 3 */
			executableExportSettings->numSoundBufferAvailableSamples,		/* arg 4 */
			executableExportSettings->numSoundBufferSamplesPerDispatch,		/* arg 5 */
			renderSettings->enableSwapIntervalControl? 1:0,					/* arg 6 */
			renderSettings->swapInterval,									/* arg 7 */
			renderSettings->enableBackBuffer? 1:0,							/* arg 8 */
			enableFrameCountUniform? 1:0,									/* arg 9 */
			renderSettings->enableMipmapGeneration? 1:0,					/* arg 10 */
			numMipmapLevels,												/* arg 11 */
			numRenderTargets,												/* arg 12 */
			renderSettings->pixelFormat,									/* arg 13 */
			renderSettings->textureFilter,									/* arg 14 */
			renderSettings->textureWrap,									/* arg 15 */
			executableExportSettings->enableSoundDispatchWait? 1:0,			/* arg 16 */
			executableExportSettings->crinklerOptions.useTinyHeader? 1:0,	/* arg 17 */
			executableExportSettings->crinklerOptions.useTinyImport? 1:0,	/* arg 18 */

#if USE_MAIN_CPP
			workDirName,													/* arg 19 */
#else
			renderSettings->enableSwapIntervalControl? 1:0,					/* arg 19 */
			renderSettings->enableBackBuffer? 1:0,							/* arg 20 */
			enableFrameCountUniform? 1:0,									/* arg 21 */
			renderSettings->enableMipmapGeneration? 1:0,					/* arg 22 */
			numMipmapLevels,												/* arg 23 */
			numRenderTargets,												/* arg 24 */
			renderSettings->pixelFormat,									/* arg 25 */
			renderSettings->textureFilter,									/* arg 26 */
			renderSettings->textureWrap,									/* arg 27 */
			executableExportSettings->enableSoundDispatchWait? 1:0,			/* arg 28 */
			executableExportSettings->crinklerOptions.useTinyHeader? 1:0,	/* arg 29 */
			executableExportSettings->crinklerOptions.useTinyImport? 1:0,	/* arg 30 */
#endif

			crinklerReportFullPath,											/* crinkler arg 1 */
			crinklerReuseFullPath,											/* crinkler arg 2 */
			crinklerOptions,												/* crinkler arg 3 */
			executableExportSettings->fileName,								/* crinkler arg 4 */
			outputGraphicsFragmentShaderInlFullPath,						/* crinkler arg 5 */
			outputSoundComputeShaderInlFullPath								/* crinkler arg 6 */
		);
		fclose(file);
	}

	/* build.bat 実行 */
	{
		DWORD exitCode = ExecuteBat(buildBatFullPath, workDirName);
		if (exitCode != 0) {
			switch (exitCode) {
				case 1: {
					AppErrorMessageBox(APP_NAME, "Failed to detect the directory where Visual Studio is installed.");
				} break;
				case 2: {
#if USE_MAIN_CPP
					AppErrorMessageBox(APP_NAME, "Failed to compile main.cpp.");
#else
					AppErrorMessageBox(APP_NAME, "Failed to assemble main.asm.");
#endif
				} break;
				case 3: {
					AppErrorMessageBox(APP_NAME, "Failed to compile resource.cpp.");
				} break;
				case 4: {
					AppErrorMessageBox(
						APP_NAME,
						"Failed to execute crinkler.exe.\n"
						"Please install it and edit your PATH to include the installed directory.\n"
						"\n"
						"http://www.crinkler.net/"
					);
				} break;
				case 5: {
					AppErrorMessageBox(APP_NAME, "Failed to link and compress exe.");
				} break;
				case 6: {
					AppErrorMessageBox(APP_NAME, "Failed to copy to %s.", outputGraphicsFragmentShaderInlFullPath);
				} break;
				case 7: {
					AppErrorMessageBox(APP_NAME, "Failed to copy to %s.", outputSoundComputeShaderInlFullPath);
				} break;
				default : {
					assert(false);
				} break;
			}
			return false;
		}
	}

	return true;
}


/*=============================================================================
▼	exe ファイルのエクスポート
-----------------------------------------------------------------------------*/
bool ExportExecutable(
	const char *graphicsShaderCode,
	const char *soundShaderCode,
	const RenderSettings *renderSettings,
	const ExecutableExportSettings *executableExportSettings
){
	/* テンポラリディレクトリのパスを取得 */
	char tempPathName[MAX_PATH];
	{
		int ret = GetTempPathA(sizeof(tempPathName), tempPathName);
		if (ret == 0) {
			AppErrorMessageBox(APP_NAME, "Failed to get temp path.");
			return false;
		}
	}

	/* 作業ディレクトリのパスを生成 */
	char workDirName[MAX_PATH];
	{
		int ret = GetTempFileName(
			/* LPCTSTR lpPathName */		tempPathName,
			/* LPCTSTR lpPrefixString */	"mgl",
			/* UINT uUnique */				1,
			/* LPTSTR lpTempFileName */		workDirName
		);
		if (ret == 0) {
			AppErrorMessageBox(APP_NAME, "Failed to get a work dir name.");
			return false;
		}
	}

	/* 作業ディレクトリを生成 */
	if (ForceCreateDirectory(workDirName) == false) return false;

#if USE_MAIN_CPP
	/* システムヘッダディレクトリの作成 */
	char glDirName[MAX_PATH];
	char khrDirName[MAX_PATH];
	{
		snprintf(glDirName, sizeof(glDirName), "%s\\GL", workDirName);
		printf("create directory %s.\n", glDirName);
		if (ForceCreateDirectory(glDirName) == false) return false;
	}
	{
		snprintf(khrDirName, sizeof(khrDirName), "%s\\KHR", workDirName);
		printf("create directory %s.\n", khrDirName);
		if (ForceCreateDirectory(khrDirName) == false) return false;
	}
#endif

	/* 旧ファイルサイズを確認 */
	size_t prevExeFileSize = GetFileSize(executableExportSettings->fileName);

	/* 各種ファイル名生成 */
#if USE_MAIN_CPP
	char mainCppFullPath[MAX_PATH] = {0};
	char glextHeaderFullPath[MAX_PATH] = {0};
	char khrplatformHeaderFullPath[MAX_PATH] = {0};
#else
	char mainAsmFullPath[MAX_PATH] = {0};
#endif
	char mainObjFullPath[MAX_PATH] = {0};
	char configHeaderFullPath[MAX_PATH] = {0};
	char resourceCppFullPath[MAX_PATH] = {0};
	char resourceObjFullPath[MAX_PATH] = {0};
	char graphicsFragmentShaderGlslFullPath[MAX_PATH] = {0};
	char graphicsFragmentShaderTmpFullPath[MAX_PATH] = {0};
	char graphicsFragmentShaderInlFullPath[MAX_PATH] = {0};
	char soundComputeShaderGlslFullPath[MAX_PATH] = {0};
	char soundComputeShaderTmpFullPath[MAX_PATH] = {0};
	char soundComputeShaderInlFullPath[MAX_PATH] = {0};
	char crinklerReportFullPath[MAX_PATH] = {0};
	char crinklerReuseFullPath[MAX_PATH] = {0};
	char minifyBatFullPath[MAX_PATH] = {0};
	char buildBatFullPath[MAX_PATH] = {0};
	char outputGraphicsFragmentShaderInlFullPath[MAX_PATH] = {0};
	char outputSoundComputeShaderInlFullPath[MAX_PATH] = {0};
#if USE_MAIN_CPP
	snprintf(mainCppFullPath, sizeof(mainCppFullPath), "%s\\main.cpp", workDirName);
	snprintf(glextHeaderFullPath, sizeof(glextHeaderFullPath), "%s\\GL\\glext.h", workDirName);
	snprintf(khrplatformHeaderFullPath, sizeof(khrplatformHeaderFullPath), "%s\\KHR\\khrplatform.h", workDirName);
#else
	snprintf(mainAsmFullPath, sizeof(mainAsmFullPath), "%s\\main.asm", workDirName);
#endif
	snprintf(mainObjFullPath, sizeof(mainObjFullPath), "%s\\main.obj", workDirName);
	snprintf(configHeaderFullPath, sizeof(configHeaderFullPath), "%s\\config.h", workDirName);
	snprintf(resourceCppFullPath, sizeof(resourceCppFullPath), "%s\\resource.cpp", workDirName);
	snprintf(resourceObjFullPath, sizeof(resourceObjFullPath), "%s\\resource.obj", workDirName);
	snprintf(graphicsFragmentShaderGlslFullPath, sizeof(graphicsFragmentShaderGlslFullPath), "%s\\graphics_fragment_shader.glsl", workDirName);
	snprintf(graphicsFragmentShaderTmpFullPath,  sizeof(graphicsFragmentShaderTmpFullPath),  "%s\\graphics_fragment_shader.i",  workDirName);
	snprintf(graphicsFragmentShaderInlFullPath,  sizeof(graphicsFragmentShaderInlFullPath),  "%s\\graphics_fragment_shader.inl",  workDirName);
	snprintf(soundComputeShaderGlslFullPath, sizeof(soundComputeShaderGlslFullPath), "%s\\sound_compute_shader.glsl", workDirName);
	snprintf(soundComputeShaderTmpFullPath,  sizeof(soundComputeShaderTmpFullPath),  "%s\\sound_compute_shader.i",  workDirName);
	snprintf(soundComputeShaderInlFullPath,  sizeof(soundComputeShaderInlFullPath),  "%s\\sound_compute_shader.inl",  workDirName);
	snprintf(crinklerReportFullPath, sizeof(crinklerReportFullPath), "%s.crinkler_report.html", executableExportSettings->fileName);
	snprintf(crinklerReuseFullPath, sizeof(crinklerReuseFullPath), "%s.crinkler_reuse.txt", executableExportSettings->fileName);
	snprintf(minifyBatFullPath, sizeof(minifyBatFullPath), "%s\\minify.bat", workDirName);
	snprintf(buildBatFullPath, sizeof(buildBatFullPath), "%s\\build.bat", workDirName);
	snprintf(outputGraphicsFragmentShaderInlFullPath, sizeof(outputGraphicsFragmentShaderInlFullPath), "%s.gfx.inl", executableExportSettings->fileName);
	snprintf(outputSoundComputeShaderInlFullPath, sizeof(outputSoundComputeShaderInlFullPath), "%s.snd.inl", executableExportSettings->fileName);

	/* 上書き確認 */
	if (DialogConfirmOverWrite(executableExportSettings->fileName) == DialogConfirmOverWriteResult_Canceled
	||	DialogConfirmOverWrite(crinklerReportFullPath) == DialogConfirmOverWriteResult_Canceled
	||	DialogConfirmOverWrite(crinklerReuseFullPath) == DialogConfirmOverWriteResult_Canceled
	||	DialogConfirmOverWrite(outputGraphicsFragmentShaderInlFullPath) == DialogConfirmOverWriteResult_Canceled
	||	DialogConfirmOverWrite(outputSoundComputeShaderInlFullPath) == DialogConfirmOverWriteResult_Canceled
	) {
		return false;
	}

	/* エラー処理の都合、下請け関数に丸投げ */
	bool ret = ExportExecutableSub(
		workDirName,
		graphicsShaderCode,
		soundShaderCode,
#if USE_MAIN_CPP
		mainCppFullPath,
		glextHeaderFullPath,
		khrplatformHeaderFullPath,
#else
		mainAsmFullPath,
#endif
		mainObjFullPath,
		configHeaderFullPath,
		resourceCppFullPath,
		resourceObjFullPath,
		graphicsFragmentShaderGlslFullPath,
		graphicsFragmentShaderInlFullPath,
		soundComputeShaderGlslFullPath,
		soundComputeShaderInlFullPath,
		crinklerReportFullPath,
		crinklerReuseFullPath,
		minifyBatFullPath,
		buildBatFullPath,
		outputGraphicsFragmentShaderInlFullPath,
		outputSoundComputeShaderInlFullPath,
		renderSettings,
		executableExportSettings
	);

	/* 完了の通知 */
	if (ret) {
		size_t exeFileSize = GetFileSize(executableExportSettings->fileName);
		int exeFileSizeDiff = (int)(exeFileSize - prevExeFileSize);
		AppMessageBox(
			APP_NAME, "Export executable file completed successfully.\n\nFile size = %d bytes. (change %+d bytes)\n\n",
			(int)exeFileSize, exeFileSizeDiff
		);
	} else {
		AppErrorMessageBox(APP_NAME, "Failed to export executable file.");
	}

	/* テンポラリファイル群の除去 */
	remove(buildBatFullPath);
	remove(configHeaderFullPath);
	remove(graphicsFragmentShaderGlslFullPath);
	remove(graphicsFragmentShaderTmpFullPath);
	remove(graphicsFragmentShaderInlFullPath);
#if USE_MAIN_CPP
	remove(mainCppFullPath);
	remove(glextHeaderFullPath);
	remove(khrplatformHeaderFullPath);
#else
	remove(mainAsmFullPath);
#endif
	remove(mainObjFullPath);
	remove(minifyBatFullPath);
	remove(resourceCppFullPath);
	remove(resourceObjFullPath);
	remove(soundComputeShaderGlslFullPath);
	remove(soundComputeShaderTmpFullPath);
	remove(soundComputeShaderInlFullPath);
#if USE_MAIN_CPP
	if (RemoveDirectory(glDirName) == FALSE) {
		AppLastErrorMessageBox(APP_NAME);
		return false;
	}
	if (RemoveDirectory(khrDirName) == FALSE) {
		AppLastErrorMessageBox(APP_NAME);
		return false;
	}
#endif
	if (RemoveDirectory(workDirName) == FALSE) {
		AppLastErrorMessageBox(APP_NAME);
		return false;
	}

	return ret;
}

