/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#define WINDOWS_IGNORE_PACKING_MISMATCH
#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <stdint.h>

#include "config.h"


/* fopen 等のセキュアでないレガシー API に対する警告の抑制 */
#pragma warning(disable:4996)


/* 決め打ちのリソースハンドル */
#define ASSUMED_SOUND_SSBO											1
#define ASSUMED_FRAME_BUFFER_FBO									1
#define ASSUMED_MRT1_TEXTURE(frameBufferIndex)						((frameBufferIndex) + 1)
#define ASSUMED_MRT_TEXTURE(frameBufferIndex, renderTargetIndex)	((frameBufferIndex) + (renderTargetIndex) * 2 + 1)
#define AVOID_GL_RUNTIME_ERROR_AND_START_FROM_ZERO_FRAME			1

/* サウンドレンダリング先バッファの番号 */
#define BUFFER_INDEX_FOR_SOUND_OUTPUT			0


/* サウンドレンダリングの分割数 */
#define NUM_SOUND_BUFFER_SAMPLES_PER_DISPATCH	0x8000
#define NUM_SOUND_BUFFER_BYTES					NUM_SOUND_BUFFER_SAMPLES * NUM_SOUND_CHANNELS * sizeof(SOUND_SAMPLE_TYPE)


/*=============================================================================
▼	各種リソースの取り込み
-----------------------------------------------------------------------------*/
#include "resource.cpp"


/*=============================================================================
▼	OpenGL 関数テーブル
-----------------------------------------------------------------------------*/
static void *s_glExtFunctions[NUM_GLEXT_FUNCTIONS];


/*=============================================================================
▼	各種構造体
-----------------------------------------------------------------------------*/
#pragma data_seg(".s_waveFormat")
static /* const */ WAVEFORMATEX s_waveFormat = {
	/* WORD  wFormatTag */		WAVE_FORMAT_IEEE_FLOAT,
	/* WORD  nChannels */		NUM_SOUND_CHANNELS,
	/* DWORD nSamplesPerSec */	NUM_SOUND_SAMPLES_PER_SEC,
	/* DWORD nAvgBytesPerSec */	NUM_SOUND_SAMPLES_PER_SEC * sizeof(SOUND_SAMPLE_TYPE) * NUM_SOUND_CHANNELS,
	/* WORD  nBlockAlign */		sizeof(SOUND_SAMPLE_TYPE) * NUM_SOUND_CHANNELS,
	/* WORD  wBitsPerSample */	sizeof(SOUND_SAMPLE_TYPE) * 8,
	/* WORD  cbSize */			0	/* extension not needed */
};

#pragma data_seg(".s_waveHeader")
static WAVEHDR s_waveHeader = {
	/* LPSTR lpData */					NULL,
	/* DWORD dwBufferLength */			NUM_SOUND_BUFFER_SAMPLES * sizeof(SOUND_SAMPLE_TYPE) * NUM_SOUND_CHANNELS,
	/* DWORD dwBytesRecorded */			0,
	/* DWORD dwUser */					0,
	/* DWORD dwFlags */					WHDR_PREPARED,
	/* DWORD dwLoops */					0,
	/* struct wavehdr_tag* lpNext */	NULL,
	/* DWORD reserved */				0
};

#pragma data_seg(".s_mmTime")
static MMTIME s_mmTime = {
	/* DWORD wType */	TIME_SAMPLES,
	/* DWORD sample */	0
};

#pragma data_seg(".s_pixelFormatDescriptor")
static /* const */ uint32_t s_pixelFormatDescriptor[2] = {
	0, PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER
};

#pragma data_seg(".s_screenSettings")
static DEVMODE s_screenSettings = {
	/* char dmDeviceName[32] */		{0},
	/* int16_t dmSpecVersion */		0,
	/* int16_t dmDriverVersion */	0,
	/* int16_t dmSize */			156,
	/* int16_t dmDriverExtra */		0,
	/* int32_t dmFields */			DM_PELSHEIGHT | DM_PELSWIDTH,
	/*
		union {
			struct {
				int16_t dmOrientation
				int16_t dmPaperSize
				int16_t dmPaperLength
				int16_t dmPaperWidth
				int16_t dmScale
				int16_t dmCopies
				int16_t dmDefaultSource
				int16_t dmPrintQuality
			};
			struct {
				int32_t dmPositionX
				int32_t dmPositionX
				int32_t dmDisplayOrientation
				int32_t dmDisplayFixedOutput
			};
		};
	*/								{0},			/* unused */
	/* int16_t dmColor */			0,				/* unused */
	/* int16_t dmDuplex */			0,				/* unused */
	/* int16_t dmYResolution */		0,				/* unused */
	/* int16_t dmTTOption */		0,				/* unused */
	/* int16_t dmCollate */			0,				/* unused */
	/* char dmFormName[32] */		{0},			/* unused */
	/* int16_t dmLogPixels */		0,				/* unused */
	/* int32_t dmBitsPerPel */		0,				/* unused */
	/* int32_t dmPelsWidth */		SCREEN_WIDTH,
	/* int32_t dmPelsHeight */		SCREEN_HEIGHT,
	/*
		union {
			int32_t dmDisplayFlags;
			int32_t dmNup;
		} DUMMYUNIONNAME2;
	*/								{0},			/* unused */
	/* int32_t dmDisplayFrequency */0,				/* unused */
	/* int32_t dmICMMethod */		0,				/* unused */
	/* int32_t dmICMIntent */		0,				/* unused */
	/* int32_t dmMediaType */		0,				/* unused */
	/* int32_t dmDitherType */		0,				/* unused */
	/* int32_t dmReserved1 */		0,				/* unused */
	/* int32_t dmReserved2 */		0,				/* unused */
	/* int32_t dmPanningWidth */	0,				/* unused */
	/* int32_t dmPanningHeight */	0				/* unused */
};


/*=============================================================================
▼	サンプラの設定
-----------------------------------------------------------------------------*/
static __forceinline void
SetTextureSampler(
){
#if (TEXTURE_FILTER == TEXTURE_FILTER_NEAREST)
	glTexParameteri(
		/* GLenum target */	GL_TEXTURE_2D,
		/* GLenum pname */	GL_TEXTURE_MIN_FILTER,
		/* GLint param */	ENABLE_MIPMAP_GENERATION? GL_NEAREST_MIPMAP_NEAREST: GL_NEAREST
	);
	glTexParameteri(
		/* GLenum target */	GL_TEXTURE_2D,
		/* GLenum pname */	GL_TEXTURE_MAG_FILTER,
		/* GLint param */	GL_NEAREST
	);
#elif (TEXTURE_FILTER == TEXTURE_FILTER_LINEAR)
	/*
		デフォルトで LINEAR なので設定省略可能。
		しかし GL_TEXTURE_MIN_FILTER の設定は省略できない。
	*/
	glTexParameteri(
		/* GLenum target */	GL_TEXTURE_2D,
		/* GLenum pname */	GL_TEXTURE_MIN_FILTER,
		/* GLint param */	ENABLE_MIPMAP_GENERATION? GL_LINEAR_MIPMAP_LINEAR: GL_LINEAR
	);
#else
#	error
#endif

#if (TEXTURE_WRAP == TEXTURE_WRAP_REPEAT)
	/* デフォルトで REPEAT なので設定不要 */
#elif (TEXTURE_WRAP == TEXTURE_WRAP_CLAMP_TO_EDGE)
	glTexParameteri(
		/* GLenum target */	GL_TEXTURE_2D,
		/* GLenum pname */	GL_TEXTURE_WRAP_S,
		/* GLint param */	GL_CLAMP_TO_EDGE
	);
	glTexParameteri(
		/* GLenum target */	GL_TEXTURE_2D,
		/* GLenum pname */	GL_TEXTURE_WRAP_T,
		/* GLint param */	GL_CLAMP_TO_EDGE
	);
#elif (TEXTURE_WRAP == TEXTURE_WRAP_MIRRORED_REPEAT)
	glTexParameteri(
		/* GLenum target */	GL_TEXTURE_2D,
		/* GLenum pname */	GL_TEXTURE_WRAP_S,
		/* GLint param */	GL_MIRRORED_REPEAT
	);
	glTexParameteri(
		/* GLenum target */	GL_TEXTURE_2D,
		/* GLenum pname */	GL_TEXTURE_WRAP_T,
		/* GLint param */	GL_MIRRORED_REPEAT
	);
#else
#	error
#endif
}


/*=============================================================================
▼	テクスチャリソースの生成
-----------------------------------------------------------------------------*/
static __forceinline void
CreateTextureResource(
){
#if PREFER_GL_TEX_STORAGE_2D
	glExtTexStorage2D(
		/* GLenum target */			GL_TEXTURE_2D,
		/* GLsizei levels */		8,
		/* GLint internalformat */	(PIXEL_FORMAT == PIXEL_FORMAT_UNORM8_RGBA)? GL_RGBA8:
									(PIXEL_FORMAT == PIXEL_FORMAT_FP16_RGBA)? GL_RGBA16F:
									(PIXEL_FORMAT == PIXEL_FORMAT_FP32_RGBA)? GL_RGBA32F:
									-1 /* error */,
		/* GLsizei width */			SCREEN_WIDTH,
		/* GLsizei height */		SCREEN_HEIGHT
	);
#else
	glTexImage2D(
		/* GLenum target */			GL_TEXTURE_2D,
		/* GLint level */			0,
		/* GLint internalformat */	(PIXEL_FORMAT == PIXEL_FORMAT_UNORM8_RGBA)? GL_RGBA:
									(PIXEL_FORMAT == PIXEL_FORMAT_FP16_RGBA)? GL_RGBA16F:
									(PIXEL_FORMAT == PIXEL_FORMAT_FP32_RGBA)? GL_RGBA32F:
									-1 /* error */,
		/* GLsizei width */			SCREEN_WIDTH,
		/* GLsizei height */		SCREEN_HEIGHT,
		/* GLint border */			0,
		/* GLenum format */			GL_RGBA,
		/* GLenum type */			(PIXEL_FORMAT == PIXEL_FORMAT_UNORM8_RGBA)? GL_UNSIGNED_BYTE:
									(PIXEL_FORMAT == PIXEL_FORMAT_FP16_RGBA)? GL_HALF_FLOAT:
									(PIXEL_FORMAT == PIXEL_FORMAT_FP32_RGBA)? GL_FLOAT:
									-1 /* error */,
		/* const void * data */		NULL
	);
#endif
}


/*=============================================================================
▼	エントリポイント
-----------------------------------------------------------------------------*/
#pragma code_seg(".entrypoint")
void
entrypoint(
	void
){
	/* フルスクリーン化 */
	ChangeDisplaySettings(
		/* DEVMODEA *lpDevMode */	&s_screenSettings,
		/* DWORD    dwFlags */		CDS_FULLSCREEN
	);

	/*
		ウィンドウ作成
		CreateWindowEx ではなく CreateWindow を使うことで、c++ コード上では
		引数を一つ減らせるが、CreateWindow は CreateWindowEx のエイリアスに
		過ぎず、アセンブリレベルでは差がない。
		ここでは CreateWindowEx を使う。
	*/
	HWND hWnd = CreateWindowEx(
		/* DWORD dwExStyle */		0,
		/* LPCTSTR lpClassName */	(LPCSTR)0xC018	/* "edit" を意味する ATOM */,
		/* LPCTSTR lpWindowName */	NULL,
		/* DWORD dwStyle */			WS_POPUP | WS_VISIBLE | WS_MAXIMIZE,
		/* int x */					0,
		/* int y */					0,
		/* int nWidth */			0,
		/* int nHeight */			0,
		/* HWND hWndParent */		(HWND)NULL,
		/* HMENU hMenu */			(HMENU)NULL,
		/* HINSTANCE hInstance */	(HINSTANCE)NULL,
		/* LPVOID lpParam */		NULL
	);

	/* ディスプレイデバイスコンテキストのハンドルを取得 */
	HDC hDC = GetDC(
		/* HWND hWnd */	hWnd
	);

	/* ピクセルフォーマットの選択 */
	int pixelFormat = ChoosePixelFormat(
		/* HDC                         hdc */	hDC,
		/* const PIXELFORMATDESCRIPTOR *ppfd */	(PIXELFORMATDESCRIPTOR*)s_pixelFormatDescriptor
	);

	/* ピクセルフォーマットの設定 */
	SetPixelFormat(
		/* HDC hdc */							hDC,
		/* int format */						pixelFormat,
		/* const PIXELFORMATDESCRIPTOR *ppfd */	(PIXELFORMATDESCRIPTOR*)s_pixelFormatDescriptor
	);

	/* OopenGL のコンテキストを作成 */
	HGLRC hRC = wglCreateContext(
		/* HDC Arg1 */	hDC
	);

	/* OopenGL のカレントコンテキストを設定する */
	wglMakeCurrent(
		/* HDC   Arg1 */	hDC,
		/* HGLRC Arg2 */	hRC
	);

	/* カーソルを消す */
	ShowCursor(0);

	const char *p = g_concatenatedString_align0;

	/*
		GL 拡張関数アドレスの取得
		（OpenGL の初期化が終わってからでないと実行できないので注意）
	*/
	{
		int i = 0;
		char byte;
		do {
			PROC procAddress = wglGetProcAddress(
				/* LPCSTR Arg1 */	p
			);
			s_glExtFunctions[i] = procAddress;
			i++;
			do {
				byte = *p;
				p++;
				--byte;
			} while (byte >= 0);
			++byte;
		} while (byte == 0);
	}

	/* フラグメントシェーダのポインタ配列 */
	const char *graphicsShaderCode = p;
	const char *graphicsShaderCodes[] = {graphicsShaderCode};

	/* コンピュートシェーダコードの開始位置を検索 */
	while (*p != '\0') { p++; }
	p++;

	/* コンピュートシェーダのポインタ配列 */
	const char *soundShaderCode = p;
	const char *soundShaderCodes[] = {soundShaderCode};

	/* サウンド出力バッファの作成 */
	/*
		glExtGenBuffers の実行は省略している。
	*/
	glExtBindBufferBase(
		/* GLenum target */			GL_SHADER_STORAGE_BUFFER,
		/* GLuint index */			BUFFER_INDEX_FOR_SOUND_OUTPUT,
		/* GLuint buffer */			ASSUMED_SOUND_SSBO
	);
	glExtBufferStorage(
		/* GLenum target */			GL_SHADER_STORAGE_BUFFER,
		/* GLsizeiptr size */		NUM_SOUND_BUFFER_BYTES,
		/* const void * data */		NULL,
		/* GLbitfield flags */			GL_MAP_READ_BIT			/* 0x0001 */
									|	GL_MAP_WRITE_BIT		/* 0x0002 */
									|	GL_MAP_PERSISTENT_BIT	/* 0x0040 */
	);

	/* サウンドバッファのポインタを waveHeader に設定 */
	s_waveHeader.lpData = (LPSTR)glExtMapBuffer(
		/* GLenum target */			GL_SHADER_STORAGE_BUFFER,
		/* GLenum access */			GL_READ_WRITE
	);

	/* コンピュートシェーダの作成 */
	int soundCsProgramId = glExtCreateShaderProgramv(
		/* (GLenum type */					GL_COMPUTE_SHADER,
		/* GLsizei count */					1,
		/* const GLchar* const *strings */	soundShaderCodes
	);

	/* コンピュートシェーダのバインド */
	glExtUseProgram(
		/* GLuint program */	soundCsProgramId
	);

	/* サウンド生成 */
	for (
		int i = NUM_SOUND_BUFFER_SAMPLES - NUM_SOUND_BUFFER_SAMPLES_PER_DISPATCH;
		i >= 0;
		i -= NUM_SOUND_BUFFER_SAMPLES_PER_DISPATCH
	) {
		glExtUniform1i(
			/* GLint location */	0,
			/* GLfloat v0 */		i
		);
		glExtDispatchCompute(
			/* GLuint num_groups_x */	NUM_SOUND_BUFFER_SAMPLES_PER_DISPATCH,
			/* GLuint num_groups_y */	1,
			/* GLuint num_groups_z */	1
		);

#if ENABLE_SOUND_DISPATCH_WAIT
		/*
			サウンド生成の dispatch は重い処理になりがちである。
			GPU タイムアウト対策のため dispatch 毎に glFinish() を
			実行している。
			この glFinish() は dispatch 完了待ちも兼ねる。
		*/
		glFinish();
#endif
	}

	/* フラグメントシェーダの作成 */
	int graphicsFsProgramId = glExtCreateShaderProgramv(
		/* GLenum type */					GL_FRAGMENT_SHADER,
		/* GLsizei count */					1,
		/* const GLchar* const *strings */	graphicsShaderCodes
	);

	/* フラグメントシェーダのバインド */
	glExtUseProgram(
		/* GLuint program */	graphicsFsProgramId
	);

#if ENABLE_BACK_BUFFER
#	if (NUM_RENDER_TARGETS == 1) && (PIXEL_FORMAT == PIXEL_FORMAT_UNORM8_RGBA)
	/* サンプラの設定 */
	SetTextureSampler();
#	else
	/*
		フレームバッファ作成
		得られる frameBufferFBO の値は予想できるが、
		この関数呼び出しはキャンセルできない。
	*/
	GLuint frameBufferFBO;
	glExtGenFramebuffers(
		/* GLsizei n */		1,
	 	/* GLuint *ids */	&frameBufferFBO
	);

	/* フレームバッファのバインド */
	glExtBindFramebuffer(
		/* GLenum target */			GL_FRAMEBUFFER,
		/* GLuint framebuffer */	ASSUMED_FRAME_BUFFER_FBO
	);

	/* 有効なレンダーターゲットの指定 */
	{
		GLuint bufs[NUM_RENDER_TARGETS] = {
#		if (NUM_RENDER_TARGETS >= 1)
			GL_COLOR_ATTACHMENT0,
#		endif
#		if (NUM_RENDER_TARGETS >= 2)
			GL_COLOR_ATTACHMENT0 + 1,
#		endif
#		if (NUM_RENDER_TARGETS >= 3)
			GL_COLOR_ATTACHMENT0 + 2,
#		endif
#		if (NUM_RENDER_TARGETS >= 4)
			GL_COLOR_ATTACHMENT0 + 3,
#		endif
		};
		glExtDrawBuffers(
			/* GLsizei n */				NUM_RENDER_TARGETS,
			/* const GLenum *bufs */	bufs
		);
	}
#	endif
#endif

#if ENABLE_SWAP_INTERVAL_CONTROL
	/* フリップの挙動を指定 */
#	if SWAP_INTERVAL == SWAP_INTERVAL_ALLOW_TEARING
	wglSwapIntervalEXT(
		/* int interval */	-1		/* ティアリング許可 */
	);
#	elif SWAP_INTERVAL == SWAP_INTERVAL_HSYNC
	wglSwapIntervalEXT(
		/* int interval */	0		/* HSYNC */
	);
#	elif SWAP_INTERVAL == SWAP_INTERVAL_VSYNC
	wglSwapIntervalEXT(
		/* int interval */	1		/* VSYNC */
	);
#	else
#		error
#	endif
#endif

	/* サウンド再生 */
	HWAVEOUT hWaveOut;
	waveOutOpen(
		/* LPHWAVEOUT      phwo */			&hWaveOut,
		/* UINT            uDeviceID */		WAVE_MAPPER,
		/* LPCWAVEFORMATEX pwfx */			&s_waveFormat,
		/* DWORD_PTR       dwCallback */	NULL,
		/* DWORD_PTR       dwInstance */	0,
		/* DWORD           fdwOpen */		CALLBACK_NULL
	);
	waveOutWrite(
		/* HWAVEOUT  hwo */		hWaveOut,
		/* LPWAVEHDR pwh */		&s_waveHeader,
		/* UINT      cbwh */	sizeof(s_waveHeader)
	);

	/* メインループ */
#if ENABLE_BACK_BUFFER && ((NUM_RENDER_TARGETS > 1) || (PIXEL_FORMAT != PIXEL_FORMAT_UNORM8_RGBA))
	int frameCount = -3;
#else
#	if ENABLE_FRAME_COUNT_UNIFORM
	int frameCount = 0;
#	endif
#endif
	do {
		/* 再生位置の取得 */
		waveOutGetPosition(
			/* HWAVEOUT hwo */	hWaveOut,
			/* LPMMTIME pmmt */	&s_mmTime,
			/* UINT cbmmt */	sizeof(MMTIME)
		);

		/*
			再生位置をシェーダに渡す。
			シェーダ側に該当 uniform の宣言を持たない場合、
			この操作は GPU 側のデスクリプタを破壊するので要注意。
		*/
		int32_t waveOutPos = s_mmTime.u.sample;
		glExtUniform1i(
			/* GLint location */	0,
			/* GLfloat v0 */		waveOutPos
		);

#if ENABLE_FRAME_COUNT_UNIFORM
		/*
			フレームカウンタをシェーダに渡す。
			シェーダ側に該当 uniform の宣言を持たない場合、
			この操作は GPU 側のデスクリプタを破壊するので要注意。
		*/
		glExtUniform1i(
			/* GLint location */	1,
			/* GLfloat v0 */		frameCount
		);
#endif


#if ENABLE_BACK_BUFFER
#	if (NUM_RENDER_TARGETS == 1) && (PIXEL_FORMAT == PIXEL_FORMAT_UNORM8_RGBA)

#		if ENABLE_FRAME_COUNT_UNIFORM
		frameCount++;
#		endif

#		if ENABLE_MIPMAP_GENERATION
		/* ミップマップ生成 */
		glExtGenerateMipmap(
			/* GLenum target */		GL_TEXTURE_2D
		);
#		endif

#	else
#		if (NUM_RENDER_TARGETS == 1) && (PIXEL_FORMAT != PIXEL_FORMAT_UNORM8_RGBA)
		/* 裏テクスチャをバインド */
		glBindTexture(
			/* GLenum target */		GL_TEXTURE_2D,
			/* GLuint texture */	ASSUMED_MRT1_TEXTURE((frameCount & 1) ^ 1)
		);

		/* 裏テクスチャのリソース生成 */
		if (++frameCount < 0) {
			CreateTextureResource();
#			if AVOID_GL_RUNTIME_ERROR_AND_START_FROM_ZERO_FRAME
			continue;		/* これを省略すると初回フレームでランタイムエラー */
#			endif
		}

		/* 表テクスチャを MRT として登録 */
		glExtFramebufferTexture(
			/* GLenum target */		GL_FRAMEBUFFER,
			/* GLenum attachment */	GL_COLOR_ATTACHMENT0,
			/* GLuint texture */	ASSUMED_MRT1_TEXTURE(frameCount & 1),
			/* GLint level */		0
		);

		/* サンプラの設定 */
		SetTextureSampler();

#			if ENABLE_MIPMAP_GENERATION
		/* ミップマップ生成 */
		glExtGenerateMipmap(
			/* GLenum target */		GL_TEXTURE_2D
		);
#			endif

#		elif (NUM_RENDER_TARGETS > 1)
		++frameCount;
		for (
			int renderTargetIndex = NUM_RENDER_TARGETS - 1;
			renderTargetIndex >= 0;
			renderTargetIndex--
		) {
			/* 裏テクスチャをバインド */
			glExtActiveTexture(
				/* GLenum texture */		GL_TEXTURE0 + renderTargetIndex
			);
			glBindTexture(
				/* GLenum target */			GL_TEXTURE_2D,
				/* GLuint texture */		ASSUMED_MRT_TEXTURE((frameCount & 1) ^ 1, renderTargetIndex)
			);

			/* 裏テクスチャのリソース生成 */
			if (frameCount < 0) {
				CreateTextureResource();
#			if AVOID_GL_RUNTIME_ERROR_AND_START_FROM_ZERO_FRAME
				continue;		/* これを省略すると初回フレームでランタイムエラー */
#			endif
			}

			/* 表テクスチャを MRT として登録 */
			glExtFramebufferTexture(
				/* GLenum target */			GL_FRAMEBUFFER,
				/* GLenum attachment */		GL_COLOR_ATTACHMENT0 + renderTargetIndex,
				/* GLuint texture */		ASSUMED_MRT_TEXTURE(frameCount & 1, renderTargetIndex),
				/* GLint level */			0
			);

			/* サンプラの設定 */
			SetTextureSampler();

#			if ENABLE_MIPMAP_GENERATION
			/* ミップマップ生成 */
			glExtGenerateMipmap(
				/* GLenum target */		GL_TEXTURE_2D
			);
#			endif
		}
#			if AVOID_GL_RUNTIME_ERROR_AND_START_FROM_ZERO_FRAME
		if (frameCount < 0) continue;		/* これを省略すると初回フレームでランタイムエラー */
#			endif
#		else
#			error
#		endif
#	endif
#endif

		/* 画面全体を塗りつぶす矩形を描画 */
		glRects(
			/* GLshort x1 */	-1,
			/* GLshort y1 */	-1,
			/* GLshort x2 */	1,
			/* GLshort y2 */	1
		);

#if ENABLE_BACK_BUFFER
#	if (NUM_RENDER_TARGETS == 1) && (PIXEL_FORMAT == PIXEL_FORMAT_UNORM8_RGBA)
		/*
			バックバッファをテクスチャにコピーする。
			OpenGL ではテクスチャ 0 番はデフォルトで存在し、
			バインドもされているので、それを利用する。
		*/
		glCopyTexImage2D(
			/* GLenum target */			GL_TEXTURE_2D,
			/* GLint level */			0,
			/* GLenum internalformat */	GL_RGBA8,
			/* GLint x */				0,
			/* GLint y */				0,
			/* GLsizei width */			SCREEN_WIDTH,
			/* GLsizei height */		SCREEN_HEIGHT,
			/* GLint border */			0
		);
#	else
		/* 描画結果をデフォルトフレームバッファにコピー */
		glExtBlitNamedFramebuffer(
			/* GLuint readFramebuffer */	ASSUMED_FRAME_BUFFER_FBO,
			/* GLuint drawFramebuffer */	0,	/* 0 = デフォルトフレームバッファ */
			/* GLint srcX0 */				0,
			/* GLint srcY0 */				0,
			/* GLint srcX1 */				SCREEN_WIDTH,
			/* GLint srcY1 */				SCREEN_HEIGHT,
			/* GLint dstX0 */				0,
			/* GLint dstY0 */				0,
			/* GLint dstX1 */				SCREEN_WIDTH,
			/* GLint dstY1 */				SCREEN_HEIGHT,
			/* GLbitfield mask */			GL_COLOR_BUFFER_BIT,
			/* GLenum filter */				GL_NEAREST
		);
#	endif
#endif

		/* 画面フリップ */
		SwapBuffers(
			/* HDC  Arg1 */	hDC
		);

		/*
			届いたメッセージを除去して捨てる。
			一見不要な処理に思われるが、これを行わないと環境によっては
			正しく動作しない。
		*/
		PeekMessage(
			/* LPMSG lpMsg */			NULL,
			/* HWND  hWnd */			0,
			/* UINT  wMsgFilterMin */	0,
			/* UINT  wMsgFilterMax */	0,
			/* UINT  wRemoveMsg */		PM_REMOVE
		);

		/* ESC キーの状態を取得 */
		uint16_t escapeKeyState = GetAsyncKeyState(
			/* int vKey */		VK_ESCAPE
		);

		/* ESC キーが押されているならループを抜ける */
		if (escapeKeyState != 0) break;
	} while (
		/* サウンド再生位置が終点に達するまで継続 */
		s_mmTime.u.sample < NUM_SOUND_BUFFER_AVAILABLE_SAMPLES
	);

	/* デモを終了する */
	ExitProcess(
		/* UINT uExitCode */	0
	);
}
