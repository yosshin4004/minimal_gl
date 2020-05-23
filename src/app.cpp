/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "common.h"
#include "config.h"
#include "sound.h"
#include "graphics.h"
#include "glext_util.h"
#include "high_precision_timer.h"
#include "export_executable.h"
#include "record_image_sequence.h"
#include "dialog_confirm_over_write.h"
#include "app.h"

#define PI 3.14159265359f
#define WAVEOUT_SEEKSTEP_IN_SAMPLES	(0x4000)


static bool s_paused = false;
static double s_fp64PausedTime = 0;
static int s_xReso = SCREEN_XRESO;
static int s_yReso = SCREEN_YRESO;
static float s_exeExportDuration = 120.0f;
static int32_t s_waveOutSampleOffset = 0;
static int32_t s_frameCount = 0;
static struct Mouse {
	int x;
	int y;
	int xDelta;
	int yDelta;
	int wheelDelta;
	int LButtonPressed;
	int MButtonPressed;
	int RButtonPressed;
} s_mouse = {0};
static struct Camera {
	float vec3Pos[3];
	float vec3Ang[3];
	float fovYAsRadian;
	float mat4x4InWorld[4][4];
} s_camera = {
	{0},
	{0},
	{0},
	PI / 8.0f
};
static CaptureScreenShotSettings s_captureScreenShotSettings = {
	/* char fileName[FILENAME_MAX]; */	{0},
	/* int xReso; */					SCREEN_XRESO,
	/* int yReso; */					SCREEN_YRESO,
	/* bool replaceAlphaByOne; */		true,
};
static CaptureCubemapSettings s_captureCubemapSettings = {
	/* char fileName[FILENAME_MAX]; */	{0},
	/* int reso; */						CUBEMAP_RESO
};
static RenderSettings s_renderSettings = {
	/* PixelFormat pixelFormat; */			PixelFormatUnorm8RGBA,
	/* bool enableMultipleRenderTargets; */	true,
	/* int numEnabledRenderTargets; */		4,

	/* bool enableBackBuffer; */			true,
	/* bool enableMipmapGeneration; */		true,
	/* TextureFilter textureFilter; */		TextureFilterLinear,
	/* TextureWrap textureWrap; */			TextureWrapClampToEdge,

	/* bool enableSwapIntervalControl; */	true,
	/* SwapInterval swapInterval; */		SwapIntervalVsync,
};
static struct PreferenceSettings {
	bool enableAutoRestartByGraphicsShader;
	bool enableAutoRestartBySoundShader;
} s_preferenceSettings = {
	true, true
};
static ExecutableExportSettings s_executableExportSettings = {
	/* char fileName[FILENAME_MAX]; */			{0},
	/* int xReso; */							SCREEN_XRESO,
	/* int yReso; */							SCREEN_YRESO,
	/* int numSoundBufferSamples; */			NUM_SOUND_BUFFER_SAMPLES,
	/* int numSoundBufferAvailableSamples; */	NUM_SOUND_BUFFER_SAMPLES,
	/* int numSoundBufferSamplesPerDispatch; */	NUM_SAMPLES_PER_DISPATCH,
	/* bool enableFrameCountUniform; */			true,
	/* bool enableSoundDispatchWait; */			true,
	/* struct ShaderMinifierOptions { */		{
	/*  bool noRenaming; */							false,
	/*  bool noSequence; */							false,
	/*  bool smoothstep; */							false,
	/* } shaderMinifierOptions; */				},
	/* struct CrinklerOptions { */				{
	/*  CrinklerCompMode compMode; */				CrinklerCompModeSlow,
	/*  bool useTinyHeader; */						false,
	/*  bool useTinyImport; */						false,
	/* } crinklerOptions; */					}
};
static RecordImageSequenceSettings s_recordImageSequenceSettings = {
	/* char directoryName[FILENAME_MAX]; */	{0},
	/* int xReso; */						SCREEN_XRESO,
	/* int yReso; */						SCREEN_YRESO,
	/* float startTime; */					0.0f,
	/* float duration; */					120.0f,
	/* float framesPerSecond; */			60.0f,
	/* bool replaceAlphaByOne; */			true,
};
static CaptureSoundSettings s_captureSoundSettings = {
	/* char fileName[FILENAME_MAX]; */	{0},
	/* float durationInSeconds; */		120.0f
};
static bool s_forceOverWrite = false;

/*=============================================================================
▼	シェーダソースファイル関連
-----------------------------------------------------------------------------*/
static char s_soundShaderFileName[FILENAME_MAX] = "default.snd.glsl";
static char *s_soundShaderCode = NULL;
static struct stat s_soundShaderFileStat;
static bool s_soundCreateShaderSucceeded = false;

static char s_graphicsShaderFileName[FILENAME_MAX] = "default.gfx.glsl";
static char *s_graphicsShaderCode = NULL;
static struct stat s_graphicsShaderFileStat;
static bool s_graphicsCreateShaderSucceeded = false;

static const char s_defaultGraphicsShaderCode[] =
	"#version 430\n"

	/* shader_minifier が SSBO を認識できない問題を回避するためのハック */
	"#if defined(EXPORT_EXECUTABLE)\n"
		/*
			以下の行はシェーダコードとしては正しくないが、
			shader_minifier に認識され、minify される。
			#pragma work_around_begin ~ #pragma work_around_end の範囲は、
			shader_minifier 通過後も残り、exe エクスポート処理時に
			正しいコードに置換される。
		*/
		"#pragma work_around_begin:layout(std430,binding=0)buffer _{vec2 %s[];};\n"
		"vec2 g_avec2Sample[];\n"
		"#pragma work_around_end\n"
	"#else\n"
		"layout(std430, binding = 0) buffer _{ vec2 g_avec2Sample[]; };\n"
	"#endif\n"

	"layout(location=" TO_STRING(UNIFORM_LOCATION_WAVE_OUT_POS) ")uniform int g_waveOutPos;\n"
	"#if defined(EXPORT_EXECUTABLE)\n"
		"vec2 g_vec2Reso = { SCREEN_XRESO, SCREEN_YRESO };\n"
	"#else\n"
		"layout(location=" TO_STRING(UNIFORM_LOCATION_RESO)         ")uniform vec2 g_vec2Reso;\n"
	"#endif\n"


	/* 現在時刻 */
	"float g_time = g_waveOutPos /" TO_STRING(NUM_SAMPLES_PER_SEC) ".;\n"

	"void main(){\n"
	"	vec3 vec3Col = vec3(0);\n"

	"	int pos0 = g_waveOutPos + int(gl_FragCoord.x);\n"
	"	int pos1 = g_waveOutPos + int(gl_FragCoord.x) + 1;\n"
	"	int sample0L = int((g_avec2Sample[pos0].x * .5 + .5) * (g_vec2Reso.y - 1.) + .5);\n"
	"	int sample0R = int((g_avec2Sample[pos0].y * .5 + .5) * (g_vec2Reso.y - 1.) + .5);\n"
	"	int sample1L = int((g_avec2Sample[pos1].x * .5 + .5) * (g_vec2Reso.y - 1.) + .5);\n"
	"	int sample1R = int((g_avec2Sample[pos1].y * .5 + .5) * (g_vec2Reso.y - 1.) + .5);\n"

	"	int y = int(gl_FragCoord.y);\n"
	"	if (sample0L > sample1L) {\n"
	"		if (sample0L >= y && y >= sample1L) {\n"
	"			vec3Col.r = 1;\n"
	"		}\n"
	"	} else {\n"
	"		if (sample0L <= y && y <= sample1L) {\n"
	"			vec3Col.r = 1;\n"
	"		}\n"
	"	}\n"
	"		if (sample0R > sample1R) {\n"
	"		if (sample0R >= y && y >= sample1R) {\n"
	"			vec3Col.g = 1;\n"
	"		}\n"
	"	} else {\n"
	"		if (sample0R <= y && y <= sample1R) {\n"
	"			vec3Col.g = 1;\n"
	"		}\n"
	"	}\n"

	"	vec3Col += (abs(g_avec2Sample[pos0].x) + abs(g_avec2Sample[pos0].y)) * vec3(.1, .1, .7);\n"
	"	vec3Col += sin(vec3(3,5,0) * .2 * g_time) * .1 + .1;\n"

	/* ガンマ補正しつつ結果の出力 */
	"	gl_FragColor = vec4(\n"
	"		vec3Col,\n"
	"		1\n"
	"	);\n"
	"}\n"
;
static const char s_defaultSoundShaderCode[] =
	"#version 430\n"
	"layout(location=" TO_STRING(UNIFORM_LOCATION_WAVE_OUT_POS) ")uniform int g_waveOutPos;\n"
	/* shader_minifier が SSBO を認識できない問題を回避するためのハック */
	"#ifdef EXPORT_EXECUTABLE\n"
		/* フラグメントシェーダと同様なので説明は省略 */
		"#pragma work_around_begin:layout(std430,binding=0)buffer _{vec2 %s[];};layout(local_size_x=1)in;\n"
		"vec2 g_avec2Sample[];\n"
		"#pragma work_around_end\n"
	"#else\n"
		"layout(std430, binding = 0) buffer _{ vec2 g_avec2Sample[]; };\n"
		"layout(local_size_x = 1) in;\n"
	"#endif\n"

	"void main(){\n"
	"	int pos = int(gl_GlobalInvocationID.x) + g_waveOutPos;\n"

	"	vec2 vec2Sample = vec2(\n"
	"		float((pos * 256 % 65536) - 32768) / 32768.,\n"
	"		float((pos * 256 % 65536) - 32768) / 32768.\n"
	"	) * .1 * exp(-float(pos) * .0001);\n"
	"	vec2Sample = clamp(vec2Sample, -1.0, 1.0);\n"

	"	g_avec2Sample[pos] = vec2Sample;\n"
	"}\n"
;

/*=============================================================================
▼	windows 関連
-----------------------------------------------------------------------------*/
static HINSTANCE s_hCurrentInstance = 0;
static HWND s_hMainWindow = 0;

void AppSetCurrentInstance(HINSTANCE hInstance){
	s_hCurrentInstance = hInstance;
}

HINSTANCE AppGetCurrentInstance(){
	return s_hCurrentInstance;
}

void AppSetMainWindowHandle(HWND hWindow){
	s_hMainWindow = hWindow;
}

HWND AppGetMainWindowHandle(){
	return s_hMainWindow;
}

/*=============================================================================
▼	メッセージボックス関連
-----------------------------------------------------------------------------*/
void AppMessageBox(const char *caption, const char *format, ...){
	va_list arg;
	va_start(arg, format);
	char buffer[0x1000];
	_vsnprintf(buffer, sizeof(buffer), format, arg);
	va_end(arg);
	buffer[sizeof(buffer) - 1] = '\0';
	MessageBox(NULL, buffer, caption, MB_OK);
}

bool AppYesNoMessageBox(const char *caption, const char *format, ...){
	va_list arg;
	va_start(arg, format);
	char buffer[0x1000];
	_vsnprintf(buffer, sizeof(buffer), format, arg);
	va_end(arg);
	buffer[sizeof(buffer) - 1] = '\0';
	int ret = MessageBox(NULL, buffer, caption, MB_YESNO | MB_ICONQUESTION);
	return (ret == IDYES);
}

void AppErrorMessageBox(const char *caption, const char *format, ...){
	va_list arg;
	va_start(arg, format);
	char buffer[0x1000];
	_vsnprintf(buffer, sizeof(buffer), format, arg);
	va_end(arg);
	buffer[sizeof(buffer) - 1] = '\0';
	MessageBox(NULL, buffer, caption, MB_OK | MB_ICONERROR);
}

void AppLastErrorMessageBox(const char *caption){
	DWORD errorCode = GetLastError();
	LPVOID lpMsgBuf;
	int ret = FormatMessage(
		/* DWORD   dwFlags */		FORMAT_MESSAGE_ALLOCATE_BUFFER |
									FORMAT_MESSAGE_FROM_SYSTEM |
									FORMAT_MESSAGE_IGNORE_INSERTS,
		/* LPCVOID lpSource */		NULL,
		/* DWORD   dwMessageId */	errorCode,
//		/* DWORD   dwLanguageId */	MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),		/* 日本語 */
		/* DWORD   dwLanguageId */	MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),	/* 英語 */
		/* LPTSTR  lpBuffer */		(LPTSTR)&lpMsgBuf,
		/* DWORD   nSize */			0,
		/* va_list *Arguments */	NULL
	);
	if (ret != 0) {
		AppErrorMessageBox(caption, (const char *)lpMsgBuf);
	} else {
		AppErrorMessageBox(caption, "FormatMessage failed. errorCode = %08X\n\n", errorCode);
	}
	LocalFree(lpMsgBuf);
}

/*=============================================================================
▼	カメラ関連
-----------------------------------------------------------------------------*/
static void
Mat4x4Copy(
	float mat4x4Dst[4][4],
	const float mat4x4Src[4][4]
){
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			mat4x4Dst[i][j] = mat4x4Src[i][j];
		}
	}
}


static void
Mat4x4SetUnit(
	float mat4x4[4][4]
){
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			mat4x4[i][j] = (i == j)? 1.0f : 0.0f;
		}
	}
}

static void
Mat4x4Mul(
	float mat4x4A[4][4],
	float mat4x4B[4][4],
	float mat4x4C[4][4]
){
	float mat4x4Tmp[4][4];
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			mat4x4Tmp[i][j] = 0.0f;
			for (int k = 0; k < 4; ++k) {
				mat4x4Tmp[i][j] += mat4x4B[i][k] * mat4x4C[k][j];
			}
		}
	}
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			mat4x4A[i][j] = mat4x4Tmp[i][j];
		}
	}
}

static void
Mat4x4SetAffineRotX(
	float mat4x4[4][4],
	float xAng
){
	const float s = sinf(xAng);
	const float c = cosf(xAng);
	Mat4x4SetUnit(mat4x4);
	mat4x4[1][1] = c;
	mat4x4[1][2] = s;
	mat4x4[2][1] = -s;
	mat4x4[2][2] = c;
}

static void
Mat4x4SetAffineRotY(
	float mat4x4[4][4],
	float yAng
){
	const float s = sinf(yAng);
	const float c = cosf(yAng);
	Mat4x4SetUnit(mat4x4);
	mat4x4[0][0] = c;
	mat4x4[0][2] = -s;
	mat4x4[2][0] = s;
	mat4x4[2][2] = c;
}

static void
Mat4x4SetAffineRotZ(
	float mat4x4[4][4],
	float zAng
){
	const float s = sinf(zAng);
	const float c = cosf(zAng);
	Mat4x4SetUnit(mat4x4);
	mat4x4[0][0] = c;
	mat4x4[0][1] = s;
	mat4x4[1][0] = -s;
	mat4x4[1][1] = c;
}

static void
CameraUpdate(){
	/*
		右手座標系を利用すると想定

		      [y+]
		       |
		       | /
		       |/
		-------+-------[x+]
		      /|
		     / |
		  [z+] |
	*/

	/* 角度を変更 */
	if (s_mouse.RButtonPressed) {
		float k = 0.005f;
		s_camera.vec3Ang[0] -= float(s_mouse.yDelta) * k;
		s_camera.vec3Ang[1] -= float(s_mouse.xDelta) * k;
	}

	/* 座標を変更 */
	if (s_mouse.LButtonPressed) {
		float k = 0.005f;
		s_camera.vec3Pos[0] -= s_camera.mat4x4InWorld[0][0] * float(s_mouse.xDelta) * k;
		s_camera.vec3Pos[1] -= s_camera.mat4x4InWorld[0][1] * float(s_mouse.xDelta) * k;
		s_camera.vec3Pos[2] -= s_camera.mat4x4InWorld[0][2] * float(s_mouse.xDelta) * k;
		s_camera.vec3Pos[0] += s_camera.mat4x4InWorld[1][0] * float(s_mouse.yDelta) * k;
		s_camera.vec3Pos[1] += s_camera.mat4x4InWorld[1][1] * float(s_mouse.yDelta) * k;
		s_camera.vec3Pos[2] += s_camera.mat4x4InWorld[1][2] * float(s_mouse.yDelta) * k;
	}

	/* 角度と座標から行列を作成 */
	Mat4x4SetUnit(s_camera.mat4x4InWorld);
	float mat4x4RotX[4][4];
	float mat4x4RotY[4][4];
	float mat4x4RotZ[4][4];
	Mat4x4SetAffineRotZ(mat4x4RotX, s_camera.vec3Ang[2]);
	Mat4x4SetAffineRotX(mat4x4RotY, s_camera.vec3Ang[0]);
	Mat4x4SetAffineRotY(mat4x4RotZ, s_camera.vec3Ang[1]);
	Mat4x4Mul(s_camera.mat4x4InWorld, mat4x4RotZ, s_camera.mat4x4InWorld);
	Mat4x4Mul(s_camera.mat4x4InWorld, mat4x4RotX, s_camera.mat4x4InWorld);
	Mat4x4Mul(s_camera.mat4x4InWorld, mat4x4RotY, s_camera.mat4x4InWorld);
	s_camera.mat4x4InWorld[3][0] = s_camera.vec3Pos[0];
	s_camera.mat4x4InWorld[3][1] = s_camera.vec3Pos[1];
	s_camera.mat4x4InWorld[3][2] = s_camera.vec3Pos[2];

	/* マウスホイール移動量に従い Z 軸方向に移動 */
	if (s_mouse.wheelDelta) {
		s_camera.vec3Pos[0] -= s_camera.mat4x4InWorld[2][0] * float(s_mouse.wheelDelta) * 0.001f;
		s_camera.vec3Pos[1] -= s_camera.mat4x4InWorld[2][1] * float(s_mouse.wheelDelta) * 0.001f;
		s_camera.vec3Pos[2] -= s_camera.mat4x4InWorld[2][2] * float(s_mouse.wheelDelta) * 0.001f;
	}

	/* マウスの移動量をクリア */
	s_mouse.wheelDelta = 0;
	s_mouse.xDelta = 0;
	s_mouse.yDelta = 0;
}

static bool
CameraInitialize(){
	s_camera.vec3Pos[0] = 0.0f;
	s_camera.vec3Pos[1] = 0.0f;
	s_camera.vec3Pos[2] = 0.0f;
	s_camera.vec3Ang[0] = 0.0f;
	s_camera.vec3Ang[1] = 0.0f;
	s_camera.vec3Ang[2] = 0.0f;
	s_camera.fovYAsRadian = PI / 8.0f;
	return true;
}

static bool
CameraTerminate(){
	return true;
}

/*=============================================================================
▼	マウス操作関連
-----------------------------------------------------------------------------*/
void AppMouseLButtonDown(){
	s_mouse.LButtonPressed = 1;
}
void AppMouseMButtonDown(){
	s_mouse.MButtonPressed = 1;
}
void AppMouseRButtonDown(){
	s_mouse.RButtonPressed = 1;
}
void AppMouseLButtonUp(){
	s_mouse.LButtonPressed = 0;
}
void AppMouseMButtonUp(){
	s_mouse.MButtonPressed = 0;
}
void AppMouseRButtonUp(){
	s_mouse.RButtonPressed = 0;
}
void AppSetMouseWheelDelta(int delta, int mButton){
	if (mButton == 0) {
		s_mouse.wheelDelta += delta;
	} else {
		s_camera.fovYAsRadian += ((float)delta / 120.0f / 180.0f) * PI;
		if (s_camera.fovYAsRadian < 0.0f) s_camera.fovYAsRadian = 0.0f;
		if (s_camera.fovYAsRadian > PI * 0.5f) s_camera.fovYAsRadian = PI * 0.5f;
	}
}
void AppSetMousePosition(int x, int y){
	s_mouse.xDelta += x - s_mouse.x;
	s_mouse.yDelta += y - s_mouse.y;
	s_mouse.x = x;
	s_mouse.y = y;
}
void AppSetResolution(int xReso, int yReso){
	printf("\nchange the resolution to %dx%d.\n", xReso, yReso);
	s_xReso = xReso;
	s_yReso = yReso;
}
void AppGetResolution(int *xResoRet, int *yResoRet){
	*xResoRet = s_xReso;
	*yResoRet = s_yReso;
}
void AppGetMat4x4CameraInWorld(float mat4x4InWorld[4][4]){
	Mat4x4Copy(mat4x4InWorld, s_camera.mat4x4InWorld);
}

/*=============================================================================
▼	プリファレンス設定関連
-----------------------------------------------------------------------------*/
void AppPreferenceSettingsSetEnableAutoRestartByGraphicsShader(bool flag){
	s_preferenceSettings.enableAutoRestartByGraphicsShader = flag;
}
bool AppPreferenceSettingsGetEnableAutoRestartByGraphicsShader(){
	return s_preferenceSettings.enableAutoRestartByGraphicsShader;
}
void AppPreferenceSettingsSetEnableAutoRestartBySoundShader(bool flag){
	s_preferenceSettings.enableAutoRestartBySoundShader = flag;
}
bool AppPreferenceSettingsGetEnableAutoRestartBySoundShader(){
	return s_preferenceSettings.enableAutoRestartBySoundShader;
}

/*=============================================================================
▼	レンダリング設定関連
-----------------------------------------------------------------------------*/
void AppRenderSettingsSetEnableBackBufferFlag(bool flag){
	s_renderSettings.enableBackBuffer = flag;
}
bool AppRenderSettingsGetEnableBackBufferFlag(){
	return s_renderSettings.enableBackBuffer;
}
void AppRenderSettingsSetEnableMipmapGenerationFlag(bool flag){
	s_renderSettings.enableMipmapGeneration = flag;
}
bool AppRenderSettingsGetEnableMipmapGenerationFlag(){
	return s_renderSettings.enableMipmapGeneration;
}
void AppRenderSettingsSetEnableMultipleRenderTargetsFlag(bool flag){
	s_renderSettings.enableMultipleRenderTargets = flag;
}
bool AppRenderSettingsGetEnableMultipleRenderTargetsFlag(){
	return s_renderSettings.enableMultipleRenderTargets;
}
void AppRenderSettingsSetNumEnabledRenderTargets(int numEnabledRenderTargets){
	s_renderSettings.numEnabledRenderTargets = numEnabledRenderTargets;
}
int AppRenderSettingsGetNumEnabledRenderTargets(){
	return s_renderSettings.numEnabledRenderTargets;
}
void AppRenderSettingsSetPixelFormat(PixelFormat format){
	s_renderSettings.pixelFormat = format;
}
PixelFormat AppRenderSettingsGetPixelFormat(){
	return s_renderSettings.pixelFormat;
}
void AppRenderSettingsSetTextureFilter(TextureFilter filter){
	s_renderSettings.textureFilter = filter;
}
TextureFilter AppRenderSettingsGetTextureFilter(){
	return s_renderSettings.textureFilter;
}
void AppRenderSettingsSetTextureWrap(TextureWrap wrap){
	s_renderSettings.textureWrap = wrap;
}
TextureWrap AppRenderSettingsGetTextureWrap(){
	return s_renderSettings.textureWrap;
}
void AppRenderSettingsSetEnableSwapIntervalControlFlag(bool flag){
	s_renderSettings.enableSwapIntervalControl = flag;
}
bool AppRenderSettingsGetEnableSwapIntervalControlFlag(){
	return s_renderSettings.enableSwapIntervalControl;
}
void AppRenderSettingsSetSwapIntervalControl(SwapInterval interval){
	s_renderSettings.swapInterval = interval;
}
SwapInterval AppRenderSettingsGetSwapIntervalControl(){
	return s_renderSettings.swapInterval;
}

/*=============================================================================
▼	ユーザーテクスチャ関連
-----------------------------------------------------------------------------*/
static char s_currentUserTextureFileName[NUM_USER_TEXTURES][FILENAME_MAX] = {0};

bool AppUserTexturesLoad(int userTextureIndex, const char *fileName){
	if (userTextureIndex < 0 || NUM_USER_TEXTURES <= userTextureIndex) return false;
	strcpy_s(
		s_currentUserTextureFileName[userTextureIndex],
		sizeof(s_currentUserTextureFileName[userTextureIndex]),
		fileName
	);
	return GraphicsLoadUserTexture(fileName, userTextureIndex);
}
const char *AppUserTexturesGetCurrentFileName(int userTextureIndex){
	if (userTextureIndex < 0 || NUM_USER_TEXTURES <= userTextureIndex) return NULL;
	return s_currentUserTextureFileName[userTextureIndex];
}
bool AppUserTexturesDelete(int userTextureIndex){
	memset(s_currentUserTextureFileName[userTextureIndex], 0, sizeof(s_currentUserTextureFileName[userTextureIndex]));
	return GraphicsDeleteUserTexture(userTextureIndex);
}

/*=============================================================================
▼	カメラパラメータのエディット関連
-----------------------------------------------------------------------------*/
void AppEditCameraParamsSetPosition(const float vec3Pos[3]){
	s_camera.vec3Pos[0] = vec3Pos[0];
	s_camera.vec3Pos[1] = vec3Pos[1];
	s_camera.vec3Pos[2] = vec3Pos[2];
}
void AppEditCameraParamsGetPosition(float vec3Pos[3]){
	vec3Pos[0] = s_camera.vec3Pos[0];
	vec3Pos[1] = s_camera.vec3Pos[1];
	vec3Pos[2] = s_camera.vec3Pos[2];
}
void AppEditCameraParamsSetAngleAsRadian(const float vec3Ang[3]){
	s_camera.vec3Ang[0] = vec3Ang[0];
	s_camera.vec3Ang[1] = vec3Ang[1];
	s_camera.vec3Ang[2] = vec3Ang[2];
}
void AppEditCameraParamsGetAngleAsRadian(float vec3Ang[3]){
	vec3Ang[0] = s_camera.vec3Ang[0];
	vec3Ang[1] = s_camera.vec3Ang[1];
	vec3Ang[2] = s_camera.vec3Ang[2];
}
void AppEditCameraParamsSetFovYAsRadian(float rad){
	s_camera.fovYAsRadian = rad;
	if (s_camera.fovYAsRadian < 0.0f) s_camera.fovYAsRadian = 0.0f;
	if (s_camera.fovYAsRadian > PI * 0.5f) s_camera.fovYAsRadian = PI * 0.5f;
}
float AppEditCameraParamsGetFovYAsRadian(){
	return s_camera.fovYAsRadian;
}

/*=============================================================================
▼	スクリーンショットキャプチャ関連
-----------------------------------------------------------------------------*/
void AppCaptureScreenShotSetCurrentOutputFileName(const char *fileName){
	strcpy_s(
		s_captureScreenShotSettings.fileName,
		sizeof(s_captureScreenShotSettings.fileName),
		fileName
	);
}
const char *AppCaptureScreenShotGetCurrentOutputFileName(){
	return s_captureScreenShotSettings.fileName;
}
void AppCaptureScreenShotSetResolution(int xReso, int yReso){
	s_captureScreenShotSettings.xReso = xReso;
	s_captureScreenShotSettings.yReso = yReso;
}
void AppCaptureScreenShotGetResolution(int *xResoRet, int *yResoRet){
	*xResoRet = s_captureScreenShotSettings.xReso;
	*yResoRet = s_captureScreenShotSettings.yReso;
}
void AppCaptureScreenShotSetForceReplaceAlphaByOneFlag(bool flag){
	s_captureScreenShotSettings.replaceAlphaByOne = flag;
}
bool AppCaptureScreenShotGetForceReplaceAlphaByOneFlag(){
	return s_captureScreenShotSettings.replaceAlphaByOne;
}
void AppCaptureScreenShot(){
	if (s_graphicsCreateShaderSucceeded) {
		if (DialogConfirmOverWrite(s_captureScreenShotSettings.fileName) == DialogConfirmOverWriteResult_Yes) {
			bool ret = GraphicsCaptureScreenShotAsUnorm8RgbaImage(
				SoundGetWaveOutPos(), s_frameCount, float(HighPrecisionTimerGet()),
				s_camera.fovYAsRadian, s_camera.mat4x4InWorld,
				&s_renderSettings, &s_captureScreenShotSettings
			);
			if (ret) {
				AppMessageBox(APP_NAME, "Capture screen shot as png file completed successfully.");
			} else {
				AppErrorMessageBox(APP_NAME, "Failed to capture screen shot as png file.");
			}
		}
	} else {
		AppErrorMessageBox(APP_NAME, "Invalid graphics shader.");
	}
}

/*=============================================================================
▼	キューブマップキャプチャ関連
-----------------------------------------------------------------------------*/
void AppCaptureCubemapSetCurrentOutputFileName(const char *fileName){
	strcpy_s(
		s_captureCubemapSettings.fileName,
		sizeof(s_captureCubemapSettings.fileName),
		fileName
	);
}
const char *AppCaptureCubemapGetCurrentOutputFileName(){
	return s_captureCubemapSettings.fileName;
}
void AppCaptureCubemapSetResolution(int reso){
	s_captureCubemapSettings.reso = reso;
}
int AppCaptureCubemapGetResolution(){
	return s_captureCubemapSettings.reso;
}
void AppCaptureCubemap(){
	if (s_graphicsCreateShaderSucceeded) {
		if (DialogConfirmOverWrite(s_captureCubemapSettings.fileName) == DialogConfirmOverWriteResult_Yes) {
			bool ret = GraphicsCaptureCubemap(
				SoundGetWaveOutPos(), s_frameCount, float(HighPrecisionTimerGet()),
				s_camera.mat4x4InWorld, &s_renderSettings, &s_captureCubemapSettings
			);
			if (ret) {
				AppMessageBox(APP_NAME, "Capture cubemap as dds file completed successfully.");
			} else {
				AppErrorMessageBox(APP_NAME, "Failed to capture cubemap as dds file.");
			}
		}
	} else {
		AppErrorMessageBox(APP_NAME, "Invalid graphics shader.");
	}
}

/*=============================================================================
▼	サウンドキャプチャ関連
-----------------------------------------------------------------------------*/
void AppCaptureSoundSetCurrentOutputFileName(const char *fileName){
	strcpy_s(
		s_captureSoundSettings.fileName,
		sizeof(s_captureSoundSettings.fileName),
		fileName
	);
}
const char *AppCaptureSoundGetCurrentOutputFileName(){
	return s_captureSoundSettings.fileName;
}
void AppCaptureSoundSetDurationInSeconds(float duration){
	s_captureSoundSettings.durationInSeconds = duration;
}
float AppCaptureSoundGetDurationInSeconds(){
	return s_captureSoundSettings.durationInSeconds;
}
void AppCaptureSound(){
	printf("\ncapture the sound.\n");
	if (s_soundCreateShaderSucceeded) {
		if (DialogConfirmOverWrite(s_captureSoundSettings.fileName) == DialogConfirmOverWriteResult_Yes) {
			bool ret = SoundCaptureSound(&s_captureSoundSettings);
			if (ret) {
				AppMessageBox(APP_NAME, "Capture sound as wav file completed successfully.");
			} else {
				AppErrorMessageBox(APP_NAME, "Failed to capture sound as wav file.");
			}
		}
	} else {
		AppErrorMessageBox(APP_NAME, "Invalid sound shader.");
	}
}

/*=============================================================================
▼	exe エクスポート関連
-----------------------------------------------------------------------------*/
void AppExportExecutableSetCurrentOutputFileName(const char *fileName){
	strcpy_s(
		s_executableExportSettings.fileName,
		sizeof(s_executableExportSettings.fileName),
		fileName
	);
}
const char *AppExportExecutableGetCurrentOutputFileName(){
	return s_executableExportSettings.fileName;
}
void AppExportExecutableSetResolution(int xReso, int yReso){
	s_executableExportSettings.xReso = xReso;
	s_executableExportSettings.yReso = yReso;
}
void AppExportExecutableGetResolution(int *xResoRet, int *yResoRet){
	*xResoRet = s_executableExportSettings.xReso;
	*yResoRet = s_executableExportSettings.yReso;
}
void AppExportExecutableSetDurationInSeconds(float duration){
	s_exeExportDuration = duration;
	int numSamples = (int)(duration * NUM_SAMPLES_PER_SEC);
	int numSamplesPerDispatch = NUM_SAMPLES_PER_DISPATCH;
	numSamples = CeilAlign(numSamples, numSamplesPerDispatch);
	s_executableExportSettings.numSoundBufferSamples = numSamples;
	s_executableExportSettings.numSoundBufferAvailableSamples = numSamples;
	s_executableExportSettings.numSoundBufferSamplesPerDispatch = numSamplesPerDispatch;
}
float AppExportExecutableGetDurationInSeconds(){
	return s_exeExportDuration;
}
void AppExportExecutableSetEnableFrameCountUniformFlag(bool flag){
	s_executableExportSettings.enableFrameCountUniform = flag;
}
bool AppExportExecutableGetEnableFrameCountUniformFlag(){
	return s_executableExportSettings.enableFrameCountUniform;
}
void AppExportExecutableSetEnableSoundDispatchWaitFlag(bool flag){
	s_executableExportSettings.enableSoundDispatchWait = flag;
}
bool AppExportExecutableGetEnableSoundDispatchWaitFlag(){
	return s_executableExportSettings.enableSoundDispatchWait;
}
void AppExportExecutableSetShaderMinifierOptionsNoRenaming(bool flag){
	s_executableExportSettings.shaderMinifierOptions.noRenaming = flag;
}
bool AppExportExecutableGetShaderMinifierOptionsNoRenaming(){
	return s_executableExportSettings.shaderMinifierOptions.noRenaming;
}
void AppExportExecutableSetShaderMinifierOptionsNoSequence(bool flag){
	s_executableExportSettings.shaderMinifierOptions.noSequence = flag;
}
bool AppExportExecutableGetShaderMinifierOptionsNoSequence(){
	return s_executableExportSettings.shaderMinifierOptions.noSequence;
}
void AppExportExecutableSetShaderMinifierOptionsSmoothstep(bool flag){
	s_executableExportSettings.shaderMinifierOptions.smoothstep = flag;
}
bool AppExportExecutableGetShaderMinifierOptionsSmoothstep(){
	return s_executableExportSettings.shaderMinifierOptions.smoothstep;
}
void AppExportExecutableSetCrinklerOptionsCompMode(CrinklerCompMode mode){
	s_executableExportSettings.crinklerOptions.compMode = mode;
}
CrinklerCompMode AppExportExecutableGetCrinklerOptionsCompMode(){
	return s_executableExportSettings.crinklerOptions.compMode;
}
void AppExportExecutableSetCrinklerOptionsUseTinyHeader(bool flag){
	s_executableExportSettings.crinklerOptions.useTinyHeader = flag;
}
bool AppExportExecutableGetCrinklerOptionsUseTinyHeader(){
	return s_executableExportSettings.crinklerOptions.useTinyHeader;
}
void AppExportExecutableSetCrinklerOptionsUseTinyImport(bool flag){
	s_executableExportSettings.crinklerOptions.useTinyImport = flag;
}
bool AppExportExecutableGetCrinklerOptionsUseTinyImport(){
	return s_executableExportSettings.crinklerOptions.useTinyImport;
}
void AppExportExecutable(){
	printf("\nexport a executable file.\n");
	if (s_soundCreateShaderSucceeded
	&&	s_graphicsCreateShaderSucceeded
	) {
		bool ret = ExportExecutable(
			s_graphicsShaderCode,
			s_soundShaderCode,
			&s_renderSettings,
			&s_executableExportSettings
		);
	} else {
		AppErrorMessageBox(APP_NAME, "Please fix shader compile errors before export.");
	}
}

/*=============================================================================
▼	連番画像保存関連
-----------------------------------------------------------------------------*/
void AppRecordImageSequenceSetCurrentOutputDirectoryName(const char *directoryName){
	strcpy_s(
		s_recordImageSequenceSettings.directoryName,
		sizeof(s_recordImageSequenceSettings.directoryName),
		directoryName
	);
}
const char *AppRecordImageSequenceGetCurrentOutputDirectoryName(){
	return s_recordImageSequenceSettings.directoryName;
}
void AppRecordImageSequenceSetResolution(int xReso, int yReso){
	s_recordImageSequenceSettings.xReso = xReso;
	s_recordImageSequenceSettings.yReso = yReso;
}
void AppRecordImageSequenceGetResolution(int *xResoRet, int *yResoRet){
	*xResoRet = s_recordImageSequenceSettings.xReso;
	*yResoRet = s_recordImageSequenceSettings.yReso;
}
void AppRecordImageSequenceSetStartTimeInSeconds(float startTime){
	s_recordImageSequenceSettings.startTime = startTime;
}
float AppRecordImageSequenceGetStartTimeInSeconds(){
	return s_recordImageSequenceSettings.startTime;
}
void AppRecordImageSequenceSetDurationInSeconds(float duration){
	s_recordImageSequenceSettings.duration = duration;
}
float AppRecordImageSequenceGetDurationInSeconds(){
	return s_recordImageSequenceSettings.duration;
}
void AppRecordImageSequenceSetFramesPerSecond(float framesPerSecond){
	s_recordImageSequenceSettings.framesPerSecond = framesPerSecond;
}
float AppRecordImageSequenceGetFramesPerSecond(){
	return s_recordImageSequenceSettings.framesPerSecond;
}
void AppRecordImageSequenceSetForceReplaceAlphaByOneFlag(bool flag){
	s_recordImageSequenceSettings.replaceAlphaByOne = flag;
}
bool AppRecordImageSequenceGetForceReplaceAlphaByOneFlag(){
	return s_recordImageSequenceSettings.replaceAlphaByOne;
}
void AppRecordImageSequence(){
	printf("\nrecord image sequence.\n");
	if (s_soundCreateShaderSucceeded
	&&	s_graphicsCreateShaderSucceeded
	) {
		bool ret = RecordImageSequence(
			&s_renderSettings,
			&s_recordImageSequenceSettings
		);
	} else {
		AppErrorMessageBox(APP_NAME, "Please fix shader compile errors before export.");
	}
}

/*=============================================================================
▼	上書き確認関連
-----------------------------------------------------------------------------*/
void AppSetForceOverWriteFlag(bool flag){
	s_forceOverWrite = flag;
}
bool AppGetForceOverWriteFlag(){
	return s_forceOverWrite;
}

/*=============================================================================
▼	シェーダファイル関連
-----------------------------------------------------------------------------*/
void AppOpenGraphicsShaderFile(const char *fileName){
	printf("\nopen graphics shader file %s.\n", fileName);
	strcpy_s(
		s_graphicsShaderFileName,
		sizeof(s_graphicsShaderFileName),
		fileName
	);
	s_graphicsShaderFileStat.st_mtime = 0;	/* 強制的に再読み込み */
}
void AppOpenSoundShaderFile(const char *fileName){
	printf("\nopen sound shader file %s.\n", fileName);
	strcpy_s(
		s_soundShaderFileName,
		sizeof(s_soundShaderFileName),
		fileName
	);
	s_soundShaderFileStat.st_mtime = 0;		/* 強制的に再読み込み */
}
void AppOpenDragAndDroppedFile(const char *fileName){
	if (IsSuffix(fileName, ".gfx.glsl")) {
		AppOpenGraphicsShaderFile(fileName);
	} else
	if (IsSuffix(fileName, ".snd.glsl")) {
		AppOpenSoundShaderFile(fileName);
	} else {
		AppErrorMessageBox(APP_NAME, "Cannot recognize file type.");
	}
}
const char *AppGetCurrentGraphicsShaderFileName(){
	return s_graphicsShaderFileName;
}
const char *AppGetCurrentSoundShaderFileName(){
	return s_soundShaderFileName;
}
const char *AppGetCurrentGraphicsShaderCode(){
	return s_graphicsShaderCode;
}
const char *AppGetCurrentSoundShaderCode(){
	return s_soundShaderCode;
}

/*=============================================================================
▼	その他
-----------------------------------------------------------------------------*/
void AppClearAllTexturesAndFremeBuffers(){
	GraphicsClearAllTexturesAndFremeBuffers();
}

void AppRestart(){
	printf("\nrestart.\n");
	HighPrecisionTimerReset(0);
	SoundRestartWaveOut();
	s_paused = false;
	s_frameCount = 0;
}

void AppResetCamera(){
	printf("\nreset the camera.\n");
	s_camera.vec3Pos[0] = 0;
	s_camera.vec3Pos[1] = 0;
	s_camera.vec3Pos[2] = 0;
	s_camera.vec3Ang[0] = 0;
	s_camera.vec3Ang[1] = 0;
	s_camera.vec3Ang[2] = 0;
}

void AppTogglePauseAndResume(){
	if (s_paused) {
		AppResume();
	} else {
		AppPause();
	}
}

void AppPause(){
	if (s_paused == false) {
		printf("\npause.\n");
		s_paused = true;
		SoundPauseWaveOut();
		s_fp64PausedTime = HighPrecisionTimerGet();
	}
}

void AppResume(){
	if (s_paused) {
		printf("\nresume.\n");
		s_paused = false;
		SoundResumeWaveOut();
		HighPrecisionTimerReset(s_fp64PausedTime);
	}
}

static void AppSeekInSamples(int samples){
	s_waveOutSampleOffset = SoundGetWaveOutPos() + samples;
	if (s_waveOutSampleOffset < 0) s_waveOutSampleOffset = 0;
	double fp64OffsetInSeconds = s_waveOutSampleOffset / (double)NUM_SAMPLES_PER_SEC;
	HighPrecisionTimerReset(fp64OffsetInSeconds);
	SoundSeekWaveOut(s_waveOutSampleOffset);
	if (s_paused) {
		s_fp64PausedTime = fp64OffsetInSeconds;
	}
}

void AppSlowForward(){
	AppPause();
	AppSeekInSamples(0x100);
}

void AppSlowBackward(){
	AppPause();
	AppSeekInSamples(-0x100);
}

void AppFastForward(){
	AppSeekInSamples(WAVEOUT_SEEKSTEP_IN_SAMPLES);
}

void AppFastBackward(){
	AppSeekInSamples(-WAVEOUT_SEEKSTEP_IN_SAMPLES);
}


bool AppUpdate(){
	/* 現在時刻を取得 */
	double fp64CurrentTime;
	if (s_paused) {
		fp64CurrentTime = s_fp64PausedTime;
	} else {
		fp64CurrentTime = HighPrecisionTimerGet();
	}

	/* FPS 算出 */
	if (s_graphicsCreateShaderSucceeded
	&&	s_soundCreateShaderSucceeded
	) {
		static int s_frameSkip = 0;
		static double s_fp64PrevTime = 0.0;
		static double s_fp64Fps = 0.0;
		++s_frameSkip;

		/* 1 秒ごとに FPS を求める */
		if (floor(fp64CurrentTime) > floor(s_fp64PrevTime)) {
			if (s_graphicsCreateShaderSucceeded) {
				s_fp64Fps = (double)s_frameSkip / (fp64CurrentTime - s_fp64PrevTime);
			}
			s_fp64PrevTime = fp64CurrentTime;
			s_frameSkip = 0;
		}

		/* 時間が巻き戻っているなら修正 */
		if (fp64CurrentTime < s_fp64PrevTime) {
			s_fp64PrevTime = fp64CurrentTime;
		}

		/* ステートの TTY 出力 */
		printf(
			"time %.1f, waveOutPos %08x, FPS %.1f, camPos(%.2f, %.2f, %.2f), camAng(%.1f, %.1f, %.1f), fovY %.1f    \r",
			fp64CurrentTime,
			SoundGetWaveOutPos(),
			s_fp64Fps,
			s_camera.vec3Pos[0],
			s_camera.vec3Pos[1],
			s_camera.vec3Pos[2],
			s_camera.vec3Ang[0] * 180.0f / PI,
			s_camera.vec3Ang[1] * 180.0f / PI,
			s_camera.vec3Ang[2] * 180.0f / PI,
			s_camera.fovYAsRadian * 180.0f / PI
		);
	}

	/* サウンドシェーダの更新 */
	if (IsFileUpdated(s_soundShaderFileName, &s_soundShaderFileStat)) {
		SoundClearOutputBuffer();
		printf("\nupdate the sound shader.\n");
		if (s_soundShaderCode != NULL) free(s_soundShaderCode);
		s_soundShaderCode = MallocReadTextFile(s_soundShaderFileName);
		if (s_soundShaderCode == NULL) {
			AppErrorMessageBox(APP_NAME, "Failed to read %s.\n", s_soundShaderFileName);
		} else {
			SoundDeleteShader();
			s_soundCreateShaderSucceeded = SoundCreateShader(s_soundShaderCode);
			if (s_preferenceSettings.enableAutoRestartBySoundShader) {
				AppRestart();
			}
		}
	}

	/* グラフィクスシェーダの更新 */
	if (IsFileUpdated(s_graphicsShaderFileName, &s_graphicsShaderFileStat)) {
		printf("\nupdate the graphics shader.\n");
		if (s_graphicsShaderCode != NULL) free(s_graphicsShaderCode);
		s_graphicsShaderCode = MallocReadTextFile(s_graphicsShaderFileName);
		if (s_graphicsShaderFileName == NULL) {
			AppErrorMessageBox(APP_NAME, "Failed to read %s.\n", s_graphicsShaderFileName);
		} else {
			GraphicsDeleteShader();
			s_graphicsCreateShaderSucceeded = GraphicsCreateShader(s_graphicsShaderCode);
			if (s_preferenceSettings.enableAutoRestartByGraphicsShader) {
				AppRestart();
			}
		}
	}

	/* カメラ更新 */
	CameraUpdate();

	/* サウンドの更新 */
	CheckGlError("pre SoundUpdate");
	if (s_soundCreateShaderSucceeded) {
		SoundUpdate();
	}
	CheckGlError("post SoundUpdate");

	/* グラフィクスの更新 */
	CheckGlError("pre GraphicsUpdate");
	if (s_graphicsCreateShaderSucceeded) {
		GraphicsUpdate(
			SoundGetWaveOutPos(), s_frameCount, float(fp64CurrentTime),
			s_mouse.x, s_mouse.y,
			s_mouse.LButtonPressed, s_mouse.MButtonPressed, s_mouse.RButtonPressed,
			s_xReso, s_yReso,
			s_camera.fovYAsRadian, s_camera.mat4x4InWorld, &s_renderSettings
		);
	}
	CheckGlError("post GraphicsUpdate");

	/* フレームの終わり */
	CheckGlError("pre glFlush");
	glFlush();
	CheckGlError("post glFlush");

	s_frameCount++;
	return true;
}

/*=============================================================================
▼	初期化 & 終了処理
-----------------------------------------------------------------------------*/
bool AppInitialize(){
	memset(&s_graphicsShaderFileStat, 0, sizeof(s_graphicsShaderFileStat));
	memset(&s_soundShaderFileStat, 0, sizeof(s_soundShaderFileStat));

	if (OpenGlExtInitialize() == false) {
		AppErrorMessageBox(APP_NAME, "OpenGlExtInitialize() failed.");
		return false;
	}
	if (HighPrecisionTimerInitialize() == false) {
		AppErrorMessageBox(APP_NAME, "HighPrecisionTimerInitialize() failed.");
		return false;
	}
	if (CameraInitialize() == false) {
		AppErrorMessageBox(APP_NAME, "CameraInitialize() failed.");
		return false;
	}
	if (SoundInitialize() == false) {
		AppErrorMessageBox(APP_NAME, "SoundInitialize() failed.");
		return false;
	}
	s_soundShaderCode = MallocCopyString(s_defaultSoundShaderCode);
	s_soundCreateShaderSucceeded = SoundCreateShader(s_soundShaderCode);
	if (GraphicsInitialize() == false) {
		AppErrorMessageBox(APP_NAME, "GraphicsInitialize() failed.");
		return false;
	}
	s_graphicsShaderCode = MallocCopyString(s_defaultGraphicsShaderCode);
	s_graphicsCreateShaderSucceeded = GraphicsCreateShader(s_graphicsShaderCode);
	SoundRestartWaveOut();
	return true;
}

bool AppTerminate(){
	if (s_soundShaderCode != NULL) {
		free(s_soundShaderCode);
		s_soundShaderCode = NULL;
	}
	if (s_graphicsShaderCode != NULL) {
		free(s_graphicsShaderCode);
		s_graphicsShaderCode = NULL;
	}

	if (GraphicsTerminate() == false) {
		AppErrorMessageBox(APP_NAME, "GraphicsTerminate() failed.");
		return false;
	}
	if (SoundTerminate() == false) {
		AppErrorMessageBox(APP_NAME, "SoundTerminate() failed.");
		return false;
	}
	if (CameraTerminate() == false) {
		AppErrorMessageBox(APP_NAME, "CameraTerminate() failed.");
		return false;
	}
	if (HighPrecisionTimerTerminate() == false) {
		AppErrorMessageBox(APP_NAME, "HighPrecisionTimerTerminate() failed.");
		return false;
	}
	if (OpenGlExtTerminate() == false) {
		AppErrorMessageBox(APP_NAME, "OpenGlExtTerminate() failed.");
		return false;
	}
	return true;
}
