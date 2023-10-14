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

#include "external/cJSON/cJSON.h"
#include "external/cJSON/cJSON_Utils.h"

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

#include "resource/resource.h"

#define PI 3.14159265359f
#define WAVEOUT_SEEKSTEP_IN_SAMPLES	(0x4000)


static bool s_paused = false;
static double s_fp64PausedTime = 0;
static int s_xReso = DEFAULT_SCREEN_XRESO;
static int s_yReso = DEFAULT_SCREEN_YRESO;
static int32_t s_waveOutSampleOffset = 0;
static int32_t s_frameCount = 0;
static struct ImGuiStatus {
	bool displayCurrentStatus;
	bool displayCameraSettings;
} s_imGuiStatus = {
	true,
	true
};
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
	float fovYInRadians;
	float mat4x4CameraInWorld[4][4];
	float mat4x4PrevCameraInWorld[4][4];
} s_camera = {0};
static CaptureScreenShotSettings s_captureScreenShotSettings = {
	/* char fileName[MAX_PATH]; */	{0},
	/* int xReso; */				DEFAULT_SCREEN_XRESO,
	/* int yReso; */				DEFAULT_SCREEN_YRESO,
	/* bool replaceAlphaByOne; */	true,
};
static CaptureCubemapSettings s_captureCubemapSettings = {
	/* char fileName[MAX_PATH]; */	{0},
	/* int reso; */					DEFAULT_CUBEMAP_RESO
};
static RenderSettings s_renderSettings = {
	/* PixelFormat pixelFormat; */			DEFAULT_PIXEL_FORMAT,
	/* bool enableMultipleRenderTargets; */	true,
	/* int numEnabledRenderTargets; */		4,

	/* bool enableBackBuffer; */			true,
	/* bool enableMipmapGeneration; */		true,
	/* TextureFilter textureFilter; */		DEFAULT_TEXTURE_FILTER,
	/* TextureWrap textureWrap; */			DEFAULT_TEXTURE_WRAP,

	/* bool enableSwapIntervalControl; */	true,
	/* SwapInterval swapInterval; */		DEFAULT_SWAP_INTERVAL,
};
static struct PreferenceSettings {
	bool enableAutoRestartByGraphicsShader;
	bool enableAutoRestartBySoundShader;
} s_preferenceSettings = {
	true, true
};
static ExecutableExportSettings s_executableExportSettings = {
	/* char fileName[MAX_PATH]; */				{0},
	/* int xReso; */							DEFAULT_SCREEN_XRESO,
	/* int yReso; */							DEFAULT_SCREEN_YRESO,
	/* float durationInSeconds; */				DEFAULT_DURATION_IN_SECONDS,
	/* int numSoundBufferSamples; */			NUM_SOUND_BUFFER_SAMPLES,
	/* int numSoundBufferAvailableSamples; */	NUM_SOUND_BUFFER_SAMPLES,
	/* int numSoundBufferSamplesPerDispatch; */	NUM_SOUND_BUFFER_SAMPLES_PER_DISPATCH,
	/* bool enableFrameCountUniform; */			true,
	/* bool enableSoundDispatchWait; */			true,
	/* struct ShaderMinifierOptions { */		{
	/*  bool noRenaming; */							false,
	/*  bool noSequence; */							false,
	/*  bool smoothstep; */							false,
	/* } shaderMinifierOptions; */				},
	/* struct CrinklerOptions { */				{
	/*  CrinklerCompMode compMode; */				DEFAULT_CRINKLER_COMP_MODE,
	/*  bool useTinyHeader; */						false,
	/*  bool useTinyImport; */						false,
	/* } crinklerOptions; */					}
};
static RecordImageSequenceSettings s_recordImageSequenceSettings = {
	/* char directoryName[MAX_PATH]; */	{0},
	/* int xReso; */					DEFAULT_SCREEN_XRESO,
	/* int yReso; */					DEFAULT_SCREEN_YRESO,
	/* float startTimeInSeconds; */		0.0f,
	/* float durationInSeconds; */		DEFAULT_DURATION_IN_SECONDS,
	/* float framesPerSecond; */		DEFAULT_FRAMES_PER_SECOND,
	/* bool replaceAlphaByOne; */		true,
};
static CaptureSoundSettings s_captureSoundSettings = {
	/* char fileName[MAX_PATH]; */	{0},
	/* float durationInSeconds; */	DEFAULT_DURATION_IN_SECONDS
};
static bool s_forceOverWrite = false;

/*=============================================================================
▼	プロジェクトファイル関連
-----------------------------------------------------------------------------*/
static char s_projectFileName[MAX_PATH] = "";

/*=============================================================================
▼	シェーダソースファイル関連
-----------------------------------------------------------------------------*/
static char s_soundShaderFileName[MAX_PATH] = "";
static char *s_soundShaderCode = NULL;
static struct stat s_soundShaderFileStat;
static bool s_soundCreateShaderSucceeded = false;

static char s_graphicsShaderFileName[MAX_PATH] = "";
static char *s_graphicsShaderCode = NULL;
static struct stat s_graphicsShaderFileStat;
static bool s_graphicsCreateShaderSucceeded = false;

static const char s_defaultGraphicsShaderCode[] =
	"#version 430\n"

	/* shader_minifier が SSBO を認識できない問題を回避するためのハック */
	"#if defined(EXPORT_EXECUTABLE)\n"
		/*
			以下の記述はシェーダコードとしては正しくないが、shader minifier に認識され
			minify が適用されたのち、work_around_begin: 以降のコードに置換される。
			%s は、shader minifier によるリネームが適用されたあとのシンボル名に
			置き換えらえる。

			buffer にはレイアウト名を付ける必要がある。ここでは、レイアウト名 = ssbo と
			している。レイアウト名は shader minifier が生成する他のシンボルと衝突しては
			いけないので、極端に短い名前を付けることは推奨されない。
		*/
		"#pragma work_around_begin:layout(std430,binding=0)buffer ssbo{vec2 %s[];};\n"
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

	"out vec4 g_vec4OutColor;\n"

	/* 経過時間 */
	"float g_time = g_waveOutPos /" TO_STRING(NUM_SOUND_SAMPLES_PER_SEC) ".;\n"

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
	"	g_vec4OutColor = vec4(\n"
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
		"#pragma work_around_begin:layout(std430,binding=0)buffer ssbo{vec2 %s[];};layout(local_size_x=1)in;\n"
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

void AppGetWindowFocus(){
	SetForegroundWindow(AppGetMainWindowHandle());
}

void AppUpdateWindowTitleBar(){
	char title[0x200] = {0};
	if (strcmp(s_projectFileName, "") != 0) {
		char fileName[MAX_PATH] = {0};
		SplitFileNameFromFilePath(fileName, sizeof(fileName), s_projectFileName);
		strncat_s(title, sizeof(title), fileName, _TRUNCATE);
		strncat_s(title, sizeof(title), "        ", _TRUNCATE);
	}
	if (strcmp(s_graphicsShaderFileName, "") != 0) {
		char fileName[MAX_PATH] = {0};
		SplitFileNameFromFilePath(fileName, sizeof(fileName), s_graphicsShaderFileName);
		strncat_s(title, sizeof(title), fileName, _TRUNCATE);
		strncat_s(title, sizeof(title), "        ", _TRUNCATE);
	}
	if (strcmp(s_soundShaderFileName, "") != 0) {
		char fileName[MAX_PATH] = {0};
		SplitFileNameFromFilePath(fileName, sizeof(fileName), s_soundShaderFileName);
		strncat_s(title, sizeof(title), fileName, _TRUNCATE);
		strncat_s(title, sizeof(title), "        ", _TRUNCATE);
	}

	SetWindowText(AppGetMainWindowHandle(), title);
}

/*=============================================================================
▼	メッセージボックス関連
-----------------------------------------------------------------------------*/
void AppMessageBox(const char *caption, const char *format, ...){
	AppGetWindowFocus();
	va_list arg;
	va_start(arg, format);
	char buffer[0x1000];
	_vsnprintf(buffer, sizeof(buffer), format, arg);
	va_end(arg);
	buffer[sizeof(buffer) - 1] = '\0';
	MessageBox(NULL, buffer, caption, MB_OK);
}

bool AppYesNoMessageBox(const char *caption, const char *format, ...){
	AppGetWindowFocus();
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
	AppGetWindowFocus();
	va_list arg;
	va_start(arg, format);
	char buffer[0x1000];
	_vsnprintf(buffer, sizeof(buffer), format, arg);
	va_end(arg);
	buffer[sizeof(buffer) - 1] = '\0';
	MessageBox(NULL, buffer, caption, MB_OK | MB_ICONERROR);
}

void AppLastErrorMessageBox(const char *caption){
	AppGetWindowFocus();
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
▼	簡易ベクトル演算関連
-----------------------------------------------------------------------------*/
static void
Vec4Copy(
	float vec4Dst[4],
	const float vec4Src[4]
){
	for (int i = 0; i < 4; ++i) {
		vec4Dst[i] = vec4Src[i];
	}
}

static void
Vec4MulScalar(
	float vec4A[4],
	const float vec4B[4],
	float scalar
){
	for (int i = 0; i < 4; ++i) {
		vec4A[i] = vec4B[i] * scalar;
	}
}

static void
Vec4MacScalar(
	float vec4A[4],
	const float vec4B[4],
	const float vec4C[4],
	float scalar
){
	for (int i = 0; i < 4; ++i) {
		vec4A[i] = vec4B[i] + vec4C[i] * scalar;
	}
}

static void
Vec4Transform(
	float vec4A[4],
	const float mat4x4B[4][4],
	const float vec4C[4]
){
	float vec4Tmp[4];
	Vec4MulScalar(vec4Tmp, mat4x4B[0], vec4C[0]);
	Vec4MacScalar(vec4Tmp, vec4Tmp, mat4x4B[1], vec4C[1]);
	Vec4MacScalar(vec4Tmp, vec4Tmp, mat4x4B[2], vec4C[2]);
	Vec4MacScalar(vec4Tmp, vec4Tmp, mat4x4B[3], vec4C[3]);
	Vec4Copy(vec4A, vec4Tmp);
}

static void
Mat4x4Copy(
	float mat4x4Dst[4][4],
	const float mat4x4Src[4][4]
){
	for (int i = 0; i < 4; ++i) {
		Vec4Copy(mat4x4Dst[i], mat4x4Src[i]);
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
		Vec4Transform(mat4x4Tmp[i], mat4x4B, mat4x4C[i]);
	}
	Mat4x4Copy(mat4x4A, mat4x4Tmp);
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
		s_camera.fovYInRadians += ((float)delta / 120.0f / 180.0f) * PI;
		if (s_camera.fovYInRadians < 0.0f) s_camera.fovYInRadians = 0.0f;
		if (s_camera.fovYInRadians > PI * 0.5f) s_camera.fovYInRadians = PI * 0.5f;
	}
}
void AppSetMousePosition(int x, int y){
	s_mouse.xDelta += x - s_mouse.x;
	s_mouse.yDelta += y - s_mouse.y;
	s_mouse.x = x;
	s_mouse.y = y;
}
void AppSetResolution(int xReso, int yReso){
	printf("change the resolution to %dx%d.\n", xReso, yReso);
	s_xReso = xReso;
	s_yReso = yReso;
}
void AppGetResolution(int *xResoRet, int *yResoRet){
	*xResoRet = s_xReso;
	*yResoRet = s_yReso;
}
void AppGetMat4x4CameraInWorld(float mat4x4CameraInWorld[4][4]){
	Mat4x4Copy(mat4x4CameraInWorld, s_camera.mat4x4CameraInWorld);
}
void AppGetMat4x4PrevCameraInWorld(float mat4x4PrevCameraInWorld[4][4]){
	Mat4x4Copy(mat4x4PrevCameraInWorld, s_camera.mat4x4PrevCameraInWorld);
}

/*=============================================================================
▼	カメラ関連
-----------------------------------------------------------------------------*/
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

	/* 前回フレームの Camera -> World 変換行列の保存 */
	Mat4x4Copy(s_camera.mat4x4PrevCameraInWorld, s_camera.mat4x4CameraInWorld);

	/* Camera -> World 変換行列の取得 */
	{
		Mat4x4SetUnit(s_camera.mat4x4CameraInWorld);
		float mat4x4RotX[4][4];
		float mat4x4RotY[4][4];
		float mat4x4RotZ[4][4];
		Mat4x4SetAffineRotX(mat4x4RotX, s_camera.vec3Ang[0]);
		Mat4x4SetAffineRotY(mat4x4RotY, s_camera.vec3Ang[1]);
		Mat4x4SetAffineRotZ(mat4x4RotZ, s_camera.vec3Ang[2]);
		Mat4x4Mul(s_camera.mat4x4CameraInWorld, s_camera.mat4x4CameraInWorld, mat4x4RotY);
		Mat4x4Mul(s_camera.mat4x4CameraInWorld, s_camera.mat4x4CameraInWorld, mat4x4RotX);
		Mat4x4Mul(s_camera.mat4x4CameraInWorld, s_camera.mat4x4CameraInWorld, mat4x4RotZ);
		s_camera.mat4x4CameraInWorld[3][0] = s_camera.vec3Pos[0];
		s_camera.mat4x4CameraInWorld[3][1] = s_camera.vec3Pos[1];
		s_camera.mat4x4CameraInWorld[3][2] = s_camera.vec3Pos[2];
	}

	/* 座標を変更 */
	if (s_mouse.LButtonPressed) {
		float k = 0.005f;
		s_camera.vec3Pos[0] -= s_camera.mat4x4CameraInWorld[0][0] * float(s_mouse.xDelta) * k;
		s_camera.vec3Pos[1] -= s_camera.mat4x4CameraInWorld[0][1] * float(s_mouse.xDelta) * k;
		s_camera.vec3Pos[2] -= s_camera.mat4x4CameraInWorld[0][2] * float(s_mouse.xDelta) * k;
		s_camera.vec3Pos[0] += s_camera.mat4x4CameraInWorld[1][0] * float(s_mouse.yDelta) * k;
		s_camera.vec3Pos[1] += s_camera.mat4x4CameraInWorld[1][1] * float(s_mouse.yDelta) * k;
		s_camera.vec3Pos[2] += s_camera.mat4x4CameraInWorld[1][2] * float(s_mouse.yDelta) * k;
	}

	/* マウスホイール移動量に従い Z 軸方向に移動 */
	if (s_mouse.wheelDelta) {
		s_camera.vec3Pos[0] -= s_camera.mat4x4CameraInWorld[2][0] * float(s_mouse.wheelDelta) * 0.001f;
		s_camera.vec3Pos[1] -= s_camera.mat4x4CameraInWorld[2][1] * float(s_mouse.wheelDelta) * 0.001f;
		s_camera.vec3Pos[2] -= s_camera.mat4x4CameraInWorld[2][2] * float(s_mouse.wheelDelta) * 0.001f;
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
	s_camera.fovYInRadians = PI * DEFAULT_CAMERA_FOVY_IN_DEGREES / 180.0f;
	return true;
}

static bool
CameraTerminate(){
	return true;
}

/*=============================================================================
▼	ImGui 設定関連
-----------------------------------------------------------------------------*/
void AppImGuiSetDisplayCurrentStatusFlag(bool flag){
	s_imGuiStatus.displayCurrentStatus = flag;
}

bool AppImGuiGetDisplayCurrentStatusFlag(){
	return s_imGuiStatus.displayCurrentStatus;
}

void AppImGuiSetDisplayCameraSettingsFlag(bool flag){
	s_imGuiStatus.displayCameraSettings = flag;
}

bool AppImGuiGetDisplayCameraSettingsFlag(){
	return s_imGuiStatus.displayCameraSettings;
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
static char s_currentUserTextureFileName[NUM_USER_TEXTURES][MAX_PATH] = {0};

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
▼	カメラ設定関連
-----------------------------------------------------------------------------*/
void AppCameraSettingsSetPosition(const float vec3Pos[3]){
	s_camera.vec3Pos[0] = vec3Pos[0];
	s_camera.vec3Pos[1] = vec3Pos[1];
	s_camera.vec3Pos[2] = vec3Pos[2];
}
void AppCameraSettingsGetPosition(float vec3Pos[3]){
	vec3Pos[0] = s_camera.vec3Pos[0];
	vec3Pos[1] = s_camera.vec3Pos[1];
	vec3Pos[2] = s_camera.vec3Pos[2];
}
void AppCameraSettingsSetAngleInRadians(const float vec3Ang[3]){
	s_camera.vec3Ang[0] = vec3Ang[0];
	s_camera.vec3Ang[1] = vec3Ang[1];
	s_camera.vec3Ang[2] = vec3Ang[2];
}
void AppCameraSettingsGetAngleInRadians(float vec3Ang[3]){
	vec3Ang[0] = s_camera.vec3Ang[0];
	vec3Ang[1] = s_camera.vec3Ang[1];
	vec3Ang[2] = s_camera.vec3Ang[2];
}
void AppCameraSettingsSetFovYInRadians(float rad){
	s_camera.fovYInRadians = rad;
	if (s_camera.fovYInRadians < 0.0f) s_camera.fovYInRadians = 0.0f;
	if (s_camera.fovYInRadians > PI * 0.5f) s_camera.fovYInRadians = PI * 0.5f;
}
float AppCameraSettingsGetFovYInRadians(){
	return s_camera.fovYInRadians;
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
				s_camera.fovYInRadians, s_camera.mat4x4CameraInWorld,
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
				s_camera.mat4x4CameraInWorld, &s_renderSettings, &s_captureCubemapSettings
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
void AppCaptureSoundSetDurationInSeconds(float durationInSeconds){
	s_captureSoundSettings.durationInSeconds = durationInSeconds;
}
float AppCaptureSoundGetDurationInSeconds(){
	return s_captureSoundSettings.durationInSeconds;
}
void AppCaptureSound(){
	printf("capture the sound.\n");
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
void AppExportExecutableSetDurationInSeconds(float durationInSeconds){
	s_executableExportSettings.durationInSeconds = durationInSeconds;
	int numSamples = (int)(durationInSeconds * NUM_SOUND_SAMPLES_PER_SEC);
	int numSamplesPerDispatch = NUM_SOUND_BUFFER_SAMPLES_PER_DISPATCH;
	numSamples = CeilAlign(numSamples, numSamplesPerDispatch);
	s_executableExportSettings.numSoundBufferSamples = numSamples;
	s_executableExportSettings.numSoundBufferAvailableSamples = numSamples;
	s_executableExportSettings.numSoundBufferSamplesPerDispatch = numSamplesPerDispatch;
}
float AppExportExecutableGetDurationInSeconds(){
	return s_executableExportSettings.durationInSeconds;
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
	printf("export an executable file.\n");
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
void AppRecordImageSequenceSetStartTimeInSeconds(float startTimeInSeconds){
	s_recordImageSequenceSettings.startTimeInSeconds = startTimeInSeconds;
}
float AppRecordImageSequenceGetStartTimeInSeconds(){
	return s_recordImageSequenceSettings.startTimeInSeconds;
}
void AppRecordImageSequenceSetDurationInSeconds(float durationInSeconds){
	s_recordImageSequenceSettings.durationInSeconds = durationInSeconds;
}
float AppRecordImageSequenceGetDurationInSeconds(){
	return s_recordImageSequenceSettings.durationInSeconds;
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
	printf("record image sequence.\n");
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
▼	JSON ユーティリティ関連
-----------------------------------------------------------------------------*/
static bool JsonGetAsString(
	cJSON *json,
	const char *pointer,
	char *dstString,
	size_t dstStringSizeInBytes,
	const char *defaultString
){
	strcpy_s(dstString, dstStringSizeInBytes, defaultString);
	cJSON *jsonFound = cJSONUtils_GetPointer(json, pointer);
	if (jsonFound == NULL) return false;
	if (cJSON_IsString(jsonFound) == false || (jsonFound->valuestring == NULL)) return false;
	if (strlen(jsonFound->valuestring) + 1 /* 末端 \0 分 */ > dstStringSizeInBytes) return false;
	strcpy_s(dstString, dstStringSizeInBytes, jsonFound->valuestring);
	return true;
}

static bool JsonGetAsInt(
	cJSON *json,
	const char *pointer,
	int *dst,
	int defaultValue
){
	*dst = defaultValue;
	cJSON *jsonFound = cJSONUtils_GetPointer(json, pointer);
	if (jsonFound == NULL) return false;
	if (cJSON_IsNumber(jsonFound) == false) return false;
	*dst = (int)jsonFound->valuedouble;
	return true;
}

static bool JsonGetAsFloat(
	cJSON *json,
	const char *pointer,
	float *dst,
	float defaultValue
){
	*dst = defaultValue;
	cJSON *jsonFound = cJSONUtils_GetPointer(json, pointer);
	if (jsonFound == NULL) return false;
	if (cJSON_IsNumber(jsonFound) == false) return false;
	*dst = (float)jsonFound->valuedouble;
	return true;
}

static bool JsonGetAsBool(
	cJSON *json,
	const char *pointer,
	bool *dst,
	bool defaultValue
){
	*dst = defaultValue;
	cJSON *jsonFound = cJSONUtils_GetPointer(json, pointer);
	if (jsonFound == NULL) return false;
	if (cJSON_IsBool(jsonFound) == false) return false;
	*dst = cJSON_IsTrue(jsonFound);
	return true;
}

static bool JsonGetAsVec3(
	cJSON *json,
	const char *pointer,
	float vec3Dst[3],
	const float vec3Default[3]
){
	vec3Dst[0] = vec3Default[0];
	vec3Dst[1] = vec3Default[1];
	vec3Dst[2] = vec3Default[2];
	cJSON *jsonFound = cJSONUtils_GetPointer(json, pointer);
	if (jsonFound == NULL) return false;

	if (cJSON_IsArray(jsonFound) == false) return false;
	if (cJSON_GetArraySize(jsonFound) != 3) return false;
	cJSON *jsonFoundElement0 = cJSON_GetArrayItem(jsonFound, 0);
	cJSON *jsonFoundElement1 = cJSON_GetArrayItem(jsonFound, 1);
	cJSON *jsonFoundElement2 = cJSON_GetArrayItem(jsonFound, 2);
	if (cJSON_IsNumber(jsonFoundElement0) == false) return false;
	if (cJSON_IsNumber(jsonFoundElement1) == false) return false;
	if (cJSON_IsNumber(jsonFoundElement2) == false) return false;
	vec3Dst[0] = (float)jsonFoundElement0->valuedouble;
	vec3Dst[1] = (float)jsonFoundElement1->valuedouble;
	vec3Dst[2] = (float)jsonFoundElement2->valuedouble;
	return true;
}

/*=============================================================================
▼	プロジェクトのシリアライズ/デシリアライズ関連
-----------------------------------------------------------------------------*/
static bool AppProjectDeserializeFromJson(cJSON *jsonRoot, const char *projectBasePath){
	bool result = true;

	{
		char graphicsShaderRelativeFileName[MAX_PATH] = {0};
		char soundShaderRelativeFileName[MAX_PATH] = {0};

		JsonGetAsString(jsonRoot, "/app/graphicsShaderFileName", graphicsShaderRelativeFileName, sizeof(graphicsShaderRelativeFileName), "");
		JsonGetAsString(jsonRoot, "/app/soundShaderFileName",    soundShaderRelativeFileName,    sizeof(soundShaderRelativeFileName),    "");
		JsonGetAsInt   (jsonRoot, "/app/xReso",                 &s_xReso, DEFAULT_SCREEN_XRESO);
		JsonGetAsInt   (jsonRoot, "/app/yReso",                 &s_yReso, DEFAULT_SCREEN_YRESO);

		if (strcmp(graphicsShaderRelativeFileName, "") == 0) {
			s_graphicsShaderFileName[0] = '\0';
			AppOpenDefaultGraphicsShader();
		} else {
			GenerateCombinedPath(
				/* char *combinedPath */				s_graphicsShaderFileName,
				/* size_t combinedPathSizeInBytes */	sizeof(s_graphicsShaderFileName),
				/* const char *directoryPath */			projectBasePath,
				/* const char *filePath */				graphicsShaderRelativeFileName
			);
			if (IsValidFileName(s_graphicsShaderFileName) == false) {
				AppErrorMessageBox(APP_NAME, "Failed to load graphics shader %s.", s_graphicsShaderFileName);
				AppOpenDefaultGraphicsShader();
				result = false;
			}
		}

		if (strcmp(soundShaderRelativeFileName, "") == 0) {
			s_soundShaderFileName[0] = '\0';
			AppOpenDefaultSoundShader();
		} else {
			GenerateCombinedPath(
				/* char *combinedPath */				s_soundShaderFileName,
				/* size_t combinedPathSizeInBytes */	sizeof(s_soundShaderFileName),
				/* const char *directoryPath */			projectBasePath,
				/* const char *filePath */				soundShaderRelativeFileName
			);
			if (IsValidFileName(s_soundShaderFileName) == false) {
				AppErrorMessageBox(APP_NAME, "Failed to load sound shader %s.", s_soundShaderFileName);
				AppOpenDefaultSoundShader();
				result = false;
			}
		}
	}
	{
		float vec3Zero[3] = {0};
		JsonGetAsVec3 (jsonRoot, "/camera/vec3Pos",       s_camera.vec3Pos, vec3Zero);
		JsonGetAsVec3 (jsonRoot, "/camera/vec3Ang",       s_camera.vec3Ang, vec3Zero);
		JsonGetAsFloat(jsonRoot, "/camera/fovYInRadians", &s_camera.fovYInRadians, PI * DEFAULT_CAMERA_FOVY_IN_DEGREES / 180.0f);
	}
	{
		char relativeFileName[MAX_PATH] = {0};

		JsonGetAsString(jsonRoot, "/captureScreenShotSettings/fileName",          relativeFileName, sizeof(relativeFileName), "");
		JsonGetAsInt   (jsonRoot, "/captureScreenShotSettings/xReso",             &s_captureScreenShotSettings.xReso, DEFAULT_SCREEN_XRESO);
		JsonGetAsInt   (jsonRoot, "/captureScreenShotSettings/yReso",             &s_captureScreenShotSettings.yReso, DEFAULT_SCREEN_YRESO);
		JsonGetAsBool  (jsonRoot, "/captureScreenShotSettings/replaceAlphaByOne", &s_captureScreenShotSettings.replaceAlphaByOne, true);

		if (strcmp(relativeFileName, "") == 0) {
			s_captureScreenShotSettings.fileName[0] = '\0';
		} else {
			GenerateCombinedPath(
				/* char *combinedPath */				s_captureScreenShotSettings.fileName,
				/* size_t combinedPathSizeInBytes */	sizeof(s_captureScreenShotSettings.fileName),
				/* const char *directoryPath */			projectBasePath,
				/* const char *filePath */				relativeFileName
			);
		}
	}
	{
		char relativeFileName[MAX_PATH] = {0};

		JsonGetAsString(jsonRoot, "/captureCubemapSettings/fileName", relativeFileName, sizeof(relativeFileName), "");
		JsonGetAsInt   (jsonRoot, "/captureCubemapSettings/reso",     &s_captureCubemapSettings.reso, DEFAULT_CUBEMAP_RESO);

		if (strcmp(relativeFileName, "") == 0) {
			s_captureCubemapSettings.fileName[0] = '\0';
		} else {
			GenerateCombinedPath(
				/* char *combinedPath */				s_captureCubemapSettings.fileName,
				/* size_t combinedPathSizeInBytes */	sizeof(s_captureCubemapSettings.fileName),
				/* const char *directoryPath */			projectBasePath,
				/* const char *filePath */				relativeFileName
			);
		}
	}
	{
		JsonGetAsInt (jsonRoot, "/renderSettings/pixelFormat",                 (int *)&s_renderSettings.pixelFormat, DEFAULT_PIXEL_FORMAT);
		JsonGetAsBool(jsonRoot, "/renderSettings/enableMultipleRenderTargets", &s_renderSettings.enableMultipleRenderTargets, true);
		JsonGetAsInt (jsonRoot, "/renderSettings/numEnabledRenderTargets",     &s_renderSettings.numEnabledRenderTargets, 4);
		JsonGetAsBool(jsonRoot, "/renderSettings/enableBackBuffer",            &s_renderSettings.enableBackBuffer, true);
		JsonGetAsBool(jsonRoot, "/renderSettings/enableMipmapGeneration",      &s_renderSettings.enableMipmapGeneration, true);
		JsonGetAsInt (jsonRoot, "/renderSettings/textureFilter",               (int *)&s_renderSettings.textureFilter, DEFAULT_TEXTURE_FILTER);
		JsonGetAsInt (jsonRoot, "/renderSettings/textureWrap",                 (int *)&s_renderSettings.textureWrap, DEFAULT_TEXTURE_WRAP);
		JsonGetAsBool(jsonRoot, "/renderSettings/enableSwapIntervalControl",   &s_renderSettings.enableSwapIntervalControl, true);
		JsonGetAsInt (jsonRoot, "/renderSettings/swapInterval",                (int *)&s_renderSettings.swapInterval, DEFAULT_SWAP_INTERVAL);
	}
	{
		JsonGetAsBool(jsonRoot, "/preferenceSettings/enableAutoRestartByGraphicsShader", &s_preferenceSettings.enableAutoRestartByGraphicsShader, true);
		JsonGetAsBool(jsonRoot, "/preferenceSettings/enableAutoRestartBySoundShader",    &s_preferenceSettings.enableAutoRestartBySoundShader, true);
	}
	{
		char relativeFileName[MAX_PATH] = {0};

		JsonGetAsString(jsonRoot, "/executableExportSettings/fileName",                         relativeFileName, sizeof(relativeFileName), "");
		JsonGetAsInt   (jsonRoot, "/executableExportSettings/xReso",                            &s_executableExportSettings.xReso, DEFAULT_SCREEN_XRESO);
		JsonGetAsInt   (jsonRoot, "/executableExportSettings/yReso",                            &s_executableExportSettings.yReso, DEFAULT_SCREEN_YRESO);
		JsonGetAsFloat (jsonRoot, "/executableExportSettings/durationInSeconds",                &s_executableExportSettings.durationInSeconds, DEFAULT_DURATION_IN_SECONDS);
		JsonGetAsInt   (jsonRoot, "/executableExportSettings/numSoundBufferSamples",            &s_executableExportSettings.numSoundBufferSamples, NUM_SOUND_BUFFER_SAMPLES);
		JsonGetAsInt   (jsonRoot, "/executableExportSettings/numSoundBufferAvailableSamples",   &s_executableExportSettings.numSoundBufferAvailableSamples, NUM_SOUND_BUFFER_SAMPLES);
		JsonGetAsInt   (jsonRoot, "/executableExportSettings/numSoundBufferSamplesPerDispatch", &s_executableExportSettings.numSoundBufferSamplesPerDispatch, NUM_SOUND_BUFFER_SAMPLES_PER_DISPATCH);
		JsonGetAsBool  (jsonRoot, "/executableExportSettings/enableFrameCountUniform",          &s_executableExportSettings.enableFrameCountUniform, true);
		JsonGetAsBool  (jsonRoot, "/executableExportSettings/enableSoundDispatchWait",          &s_executableExportSettings.enableSoundDispatchWait, true);
		JsonGetAsBool  (jsonRoot, "/executableExportSettings/shaderMinifierOptions/noRenaming", &s_executableExportSettings.shaderMinifierOptions.noRenaming, false);
		JsonGetAsBool  (jsonRoot, "/executableExportSettings/shaderMinifierOptions/noSequence", &s_executableExportSettings.shaderMinifierOptions.noSequence, false);
		JsonGetAsBool  (jsonRoot, "/executableExportSettings/shaderMinifierOptions/smoothstep", &s_executableExportSettings.shaderMinifierOptions.smoothstep, false);
		JsonGetAsInt   (jsonRoot, "/executableExportSettings/crinklerOptions/compMode",         (int *)&s_executableExportSettings.crinklerOptions.compMode, DEFAULT_CRINKLER_COMP_MODE);
		JsonGetAsBool  (jsonRoot, "/executableExportSettings/crinklerOptions/useTinyHeader",    &s_executableExportSettings.crinklerOptions.useTinyHeader, false);
		JsonGetAsBool  (jsonRoot, "/executableExportSettings/crinklerOptions/useTinyImport",    &s_executableExportSettings.crinklerOptions.useTinyImport, false);

		if (strcmp(relativeFileName, "") == 0) {
			s_executableExportSettings.fileName[0] = '\0';
		} else {
			GenerateCombinedPath(
				/* char *combinedPath */				s_executableExportSettings.fileName,
				/* size_t combinedPathSizeInBytes */	sizeof(s_executableExportSettings.fileName),
				/* const char *directoryPath */			projectBasePath,
				/* const char *filePath */				relativeFileName
			);
		}
	}
	{
		char relativeDirectoryName[MAX_PATH] = {0};

		JsonGetAsString(jsonRoot, "/recordImageSequenceSettings/directoryName",      relativeDirectoryName, sizeof(relativeDirectoryName), "");
		JsonGetAsInt   (jsonRoot, "/recordImageSequenceSettings/xReso",              &s_recordImageSequenceSettings.xReso, DEFAULT_SCREEN_XRESO);
		JsonGetAsInt   (jsonRoot, "/recordImageSequenceSettings/yReso",              &s_recordImageSequenceSettings.yReso, DEFAULT_SCREEN_YRESO);
		JsonGetAsFloat (jsonRoot, "/recordImageSequenceSettings/startTimeInSeconds", &s_recordImageSequenceSettings.startTimeInSeconds, 0.0f);
		JsonGetAsFloat (jsonRoot, "/recordImageSequenceSettings/durationInSeconds",  &s_recordImageSequenceSettings.durationInSeconds, DEFAULT_DURATION_IN_SECONDS);
		JsonGetAsFloat (jsonRoot, "/recordImageSequenceSettings/framesPerSecond",    &s_recordImageSequenceSettings.framesPerSecond, DEFAULT_FRAMES_PER_SECOND);
		JsonGetAsBool  (jsonRoot, "/recordImageSequenceSettings/replaceAlphaByOne",  &s_recordImageSequenceSettings.replaceAlphaByOne, true);

		if (strcmp(relativeDirectoryName, "") == 0) {
			s_recordImageSequenceSettings.directoryName[0] = '\0';
		} else {
			GenerateCombinedPath(
				/* char *combinedPath */				s_recordImageSequenceSettings.directoryName,
				/* size_t combinedPathSizeInBytes */	sizeof(s_recordImageSequenceSettings.directoryName),
				/* const char *directoryPath */			projectBasePath,
				/* const char *filePath */				relativeDirectoryName
			);
		}
	}
	{
		char relativeFileName[MAX_PATH] = {0};

		JsonGetAsString(jsonRoot, "/captureSoundSettings/fileName",          relativeFileName, sizeof(relativeFileName), "");
		JsonGetAsFloat (jsonRoot, "/captureSoundSettings/durationInSeconds", &s_captureSoundSettings.durationInSeconds, DEFAULT_DURATION_IN_SECONDS);

		if (strcmp(relativeFileName, "") == 0) {
			s_captureSoundSettings.fileName[0] = '\0';
		} else {
			GenerateCombinedPath(
				/* char *combinedPath */				s_captureSoundSettings.fileName,
				/* size_t combinedPathSizeInBytes */	sizeof(s_captureSoundSettings.fileName),
				/* const char *directoryPath */			projectBasePath,
				/* const char *filePath */				relativeFileName
			);
		}
	}
	{
		JsonGetAsBool(jsonRoot, "/imGuiStatus/displayCurrentStatus",  &s_imGuiStatus.displayCurrentStatus, true);
		JsonGetAsBool(jsonRoot, "/imGuiStatus/displayCameraSettings", &s_imGuiStatus.displayCameraSettings, true);
	}
	{
		for (int i = 0; i < NUM_USER_TEXTURES; i++) {
			AppUserTexturesDelete(i);
		}
		for (int i = 0; i < NUM_USER_TEXTURES; i++) {
			char pointer[0x100];
			snprintf(pointer, sizeof(pointer), "/userTextures/%d/fileName", i);

			char relativeFileName[MAX_PATH] = {0};
			JsonGetAsString(jsonRoot, pointer, relativeFileName, sizeof(relativeFileName), "");

			char fileName[MAX_PATH] = {0};
			if (strcmp(relativeFileName, "") == 0) {
				fileName[0] = '\0';
			} else {
				GenerateCombinedPath(
					/* char *combinedPath */				fileName,
					/* size_t combinedPathSizeInBytes */	sizeof(fileName),
					/* const char *directoryPath */			projectBasePath,
					/* const char *filePath */				relativeFileName
				);
				if (AppUserTexturesLoad(i, fileName) == false) {
					AppErrorMessageBox(APP_NAME, "Failed to load texture %s.", fileName);
					result = false;
				}
			}
		}
	}

	return result;
}

static void AppProjectSerializeToJson(cJSON *jsonRoot, const char *projectBasePath){
	{
		cJSON *jsonApp = cJSON_AddObjectToObject(jsonRoot, "app");

		char graphicsShaderRelativeFileName[MAX_PATH] = {0};
		if (strcmp(s_graphicsShaderFileName, "") != 0) {
			GenerateRelativePathFromDirectoryToFile(
				/* char *relativePath */				graphicsShaderRelativeFileName,
				/* size_t relativePathSizeInBytes */	sizeof(graphicsShaderRelativeFileName),
				/* const char *fromDirectoryPath */		projectBasePath,
				/* const char *toFilePath */			s_graphicsShaderFileName
			);
		}
		char soundShaderRelativeFileName[MAX_PATH] = {0};
		if (strcmp(s_soundShaderFileName, "") != 0) {
			GenerateRelativePathFromDirectoryToFile(
				/* char *relativePath */				soundShaderRelativeFileName,
				/* size_t relativePathSizeInBytes */	sizeof(soundShaderRelativeFileName),
				/* const char *fromDirectoryPath */		projectBasePath,
				/* const char *toFilePath */			s_soundShaderFileName
			);
		}

		cJSON_AddStringToObject(jsonApp, "graphicsShaderFileName" , graphicsShaderRelativeFileName);
		cJSON_AddStringToObject(jsonApp, "soundShaderFileName"    , soundShaderRelativeFileName);
		cJSON_AddNumberToObject(jsonApp, "xReso"                  , s_xReso);
		cJSON_AddNumberToObject(jsonApp, "yReso"                  , s_yReso);
	}
	{
		cJSON *jsonCamera = cJSON_AddObjectToObject(jsonRoot, "camera");

		cJSON *jsonVec3Pos = cJSON_AddArrayToObject(jsonCamera, "vec3Pos");
		cJSON_AddItemToArray(jsonVec3Pos, cJSON_CreateNumber(s_camera.vec3Pos[0]));
		cJSON_AddItemToArray(jsonVec3Pos, cJSON_CreateNumber(s_camera.vec3Pos[1]));
		cJSON_AddItemToArray(jsonVec3Pos, cJSON_CreateNumber(s_camera.vec3Pos[2]));

		cJSON *jsonVec3Ang = cJSON_AddArrayToObject(jsonCamera, "vec3Ang");
		cJSON_AddItemToArray(jsonVec3Ang, cJSON_CreateNumber(s_camera.vec3Ang[0]));
		cJSON_AddItemToArray(jsonVec3Ang, cJSON_CreateNumber(s_camera.vec3Ang[1]));
		cJSON_AddItemToArray(jsonVec3Ang, cJSON_CreateNumber(s_camera.vec3Ang[2]));

		cJSON_AddNumberToObject(jsonCamera, "fovYInRadians", s_camera.fovYInRadians);
	}
	{
		cJSON *jsonSettings = cJSON_AddObjectToObject(jsonRoot, "captureScreenShotSettings");

		char relativeFileName[MAX_PATH] = {0};
		if (strcmp(s_captureScreenShotSettings.fileName, "") != 0) {
			GenerateRelativePathFromDirectoryToFile(
				/* char *relativePath */				relativeFileName,
				/* size_t relativePathSizeInBytes */	sizeof(relativeFileName),
				/* const char *fromDirectoryPath */		projectBasePath,
				/* const char *toFilePath */			s_captureScreenShotSettings.fileName
			);
		}

		cJSON_AddStringToObject(jsonSettings, "fileName"         , relativeFileName);
		cJSON_AddNumberToObject(jsonSettings, "xReso"            , s_captureScreenShotSettings.xReso);
		cJSON_AddNumberToObject(jsonSettings, "yReso"            , s_captureScreenShotSettings.yReso);
		cJSON_AddBoolToObject  (jsonSettings, "replaceAlphaByOne", s_captureScreenShotSettings.replaceAlphaByOne);
	}
	{
		cJSON *jsonSettings = cJSON_AddObjectToObject(jsonRoot, "captureCubemapSettings");

		char relativeFileName[MAX_PATH] = {0};
		if (strcmp(s_captureCubemapSettings.fileName, "") != 0) {
			GenerateRelativePathFromDirectoryToFile(
				/* char *relativePath */				relativeFileName,
				/* size_t relativePathSizeInBytes */	sizeof(relativeFileName),
				/* const char *fromDirectoryPath */		projectBasePath,
				/* const char *toFilePath */			s_captureCubemapSettings.fileName
			);
		}

		cJSON_AddStringToObject(jsonSettings, "fileName", relativeFileName);
		cJSON_AddNumberToObject(jsonSettings, "reso"    , s_captureCubemapSettings.reso);
	}
	{
		cJSON *jsonSettings = cJSON_AddObjectToObject(jsonRoot, "renderSettings");
		cJSON_AddNumberToObject(jsonSettings, "pixelFormat",                s_renderSettings.pixelFormat);
		cJSON_AddBoolToObject  (jsonSettings, "enableMultipleRenderTargets",s_renderSettings.enableMultipleRenderTargets);
		cJSON_AddNumberToObject(jsonSettings, "numEnabledRenderTargets",    s_renderSettings.numEnabledRenderTargets);
		cJSON_AddBoolToObject  (jsonSettings, "enableBackBuffer",           s_renderSettings.enableBackBuffer);
		cJSON_AddBoolToObject  (jsonSettings, "enableMipmapGeneration",     s_renderSettings.enableMipmapGeneration);
		cJSON_AddNumberToObject(jsonSettings, "textureFilter",              s_renderSettings.textureFilter);
		cJSON_AddNumberToObject(jsonSettings, "textureWrap",                s_renderSettings.textureWrap);
		cJSON_AddBoolToObject  (jsonSettings, "enableSwapIntervalControl",  s_renderSettings.enableSwapIntervalControl);
		cJSON_AddNumberToObject(jsonSettings, "swapInterval",               s_renderSettings.swapInterval);
	}
	{
		cJSON *jsonSettings = cJSON_AddObjectToObject(jsonRoot, "preferenceSettings");
		cJSON_AddBoolToObject(jsonSettings, "enableAutoRestartByGraphicsShader", s_preferenceSettings.enableAutoRestartByGraphicsShader);
		cJSON_AddBoolToObject(jsonSettings, "enableAutoRestartBySoundShader",    s_preferenceSettings.enableAutoRestartBySoundShader);
	}
	{
		cJSON *jsonSettings = cJSON_AddObjectToObject(jsonRoot, "executableExportSettings");

		char relativeFileName[MAX_PATH] = {0};
		if (strcmp(s_executableExportSettings.fileName, "") != 0) {
			GenerateRelativePathFromDirectoryToFile(
				/* char *relativePath */				relativeFileName,
				/* size_t relativePathSizeInBytes */	sizeof(relativeFileName),
				/* const char *fromDirectoryPath */		projectBasePath,
				/* const char *toFilePath */			s_executableExportSettings.fileName
			);
		}

		cJSON_AddStringToObject(jsonSettings, "fileName",                         relativeFileName);
		cJSON_AddNumberToObject(jsonSettings, "xReso",                            s_executableExportSettings.xReso);
		cJSON_AddNumberToObject(jsonSettings, "yReso",                            s_executableExportSettings.yReso);
		cJSON_AddNumberToObject(jsonSettings, "durationInSeconds",                s_executableExportSettings.durationInSeconds);
		cJSON_AddNumberToObject(jsonSettings, "numSoundBufferSamples",            s_executableExportSettings.numSoundBufferSamples);
		cJSON_AddNumberToObject(jsonSettings, "numSoundBufferAvailableSamples",   s_executableExportSettings.numSoundBufferAvailableSamples);
		cJSON_AddNumberToObject(jsonSettings, "numSoundBufferSamplesPerDispatch", s_executableExportSettings.numSoundBufferSamplesPerDispatch);
		cJSON_AddBoolToObject  (jsonSettings, "enableFrameCountUniform",          s_executableExportSettings.enableFrameCountUniform);
		cJSON_AddBoolToObject  (jsonSettings, "enableSoundDispatchWait",          s_executableExportSettings.enableSoundDispatchWait);
		{
			cJSON *jsonOptions = cJSON_AddObjectToObject(jsonSettings, "shaderMinifierOptions");
			cJSON_AddBoolToObject(jsonOptions, "noRenaming", s_executableExportSettings.shaderMinifierOptions.noRenaming);
			cJSON_AddBoolToObject(jsonOptions, "noSequence", s_executableExportSettings.shaderMinifierOptions.noSequence);
			cJSON_AddBoolToObject(jsonOptions, "smoothstep", s_executableExportSettings.shaderMinifierOptions.smoothstep);
		}
		{
			cJSON *jsonOptions = cJSON_AddObjectToObject(jsonSettings, "crinklerOptions");
			cJSON_AddNumberToObject(jsonOptions, "compMode",      s_executableExportSettings.crinklerOptions.compMode);
			cJSON_AddBoolToObject  (jsonOptions, "useTinyHeader", s_executableExportSettings.crinklerOptions.useTinyHeader);
			cJSON_AddBoolToObject  (jsonOptions, "useTinyImport", s_executableExportSettings.crinklerOptions.useTinyImport);
		}
	}
	{
		cJSON *jsonSettings = cJSON_AddObjectToObject(jsonRoot, "recordImageSequenceSettings");

		char relativeDirectoryName[MAX_PATH] = {0};
		if (strcmp(s_recordImageSequenceSettings.directoryName, "") != 0) {
			GenerateRelativePathFromDirectoryToDirectory(
				/* char *relativePath */				relativeDirectoryName,
				/* size_t relativePathSizeInBytes */	sizeof(relativeDirectoryName),
				/* const char *fromDirectoryPath */		projectBasePath,
				/* const char *toFilePath */			s_recordImageSequenceSettings.directoryName
			);
		}

		cJSON_AddStringToObject(jsonSettings, "directoryName",      relativeDirectoryName);
		cJSON_AddNumberToObject(jsonSettings, "xReso",              s_recordImageSequenceSettings.xReso);
		cJSON_AddNumberToObject(jsonSettings, "yReso",              s_recordImageSequenceSettings.yReso);
		cJSON_AddNumberToObject(jsonSettings, "startTimeInSeconds", s_recordImageSequenceSettings.startTimeInSeconds);
		cJSON_AddNumberToObject(jsonSettings, "durationInSeconds",  s_recordImageSequenceSettings.durationInSeconds);
		cJSON_AddNumberToObject(jsonSettings, "framesPerSecond",    s_recordImageSequenceSettings.framesPerSecond);
		cJSON_AddBoolToObject  (jsonSettings, "replaceAlphaByOne",  s_recordImageSequenceSettings.replaceAlphaByOne);
	}
	{
		cJSON *jsonSettings = cJSON_AddObjectToObject(jsonRoot, "captureSoundSettings");

		char relativeFileName[MAX_PATH] = {0};
		if (strcmp(s_captureSoundSettings.fileName, "") != 0) {
			GenerateRelativePathFromDirectoryToFile(
				/* char *relativePath */				relativeFileName,
				/* size_t relativePathSizeInBytes */	sizeof(relativeFileName),
				/* const char *fromDirectoryPath */		projectBasePath,
				/* const char *toFilePath */			s_captureSoundSettings.fileName
			);
		}

		cJSON_AddStringToObject(jsonSettings, "fileName",          relativeFileName);
		cJSON_AddNumberToObject(jsonSettings, "durationInSeconds", s_captureSoundSettings.durationInSeconds);
	}
	{
		cJSON *jsonImGuiStatus = cJSON_AddObjectToObject(jsonRoot, "imGuiStatus");
		cJSON_AddBoolToObject(jsonImGuiStatus, "displayCurrentStatus",  s_imGuiStatus.displayCurrentStatus);
		cJSON_AddBoolToObject(jsonImGuiStatus, "displayCameraSettings", s_imGuiStatus.displayCameraSettings);
	}
	{
		cJSON *jsonUserTextures = cJSON_AddArrayToObject(jsonRoot, "userTextures");
		for (int i = 0; i < NUM_USER_TEXTURES; i++) {
			cJSON *jsonUserTexture = cJSON_CreateObject();
			cJSON_AddItemToArray(jsonUserTextures, jsonUserTexture);

			char relativeFileName[MAX_PATH] = {0};
			if (strcmp(AppUserTexturesGetCurrentFileName(i), "") != 0) {
				GenerateRelativePathFromDirectoryToFile(
					/* char *relativePath */				relativeFileName,
					/* size_t relativePathSizeInBytes */	sizeof(relativeFileName),
					/* const char *fromDirectoryPath */		projectBasePath,
					/* const char *toFilePath */			AppUserTexturesGetCurrentFileName(i)
				);
			}

			cJSON_AddStringToObject(jsonUserTexture, "fileName", relativeFileName);
		}
	}
}

/*=============================================================================
▼	プロジェクト管理
-----------------------------------------------------------------------------*/
const char *AppProjectGetCurrentFileName(){
	return s_projectFileName;
}

bool AppProjectImport(const char *fileName){
	/* プロジェクトのベースパス抽出 */
	char projectBasePath[MAX_PATH] = {0};
	SplitDirectoryPathFromFilePath(projectBasePath, sizeof(projectBasePath), fileName);

	/* シリアライズされたプロジェクトの読み込み */
	char *text = MallocReadTextFile(fileName);
	if (text == NULL) {
		AppErrorMessageBox(APP_NAME, "Failed to import project %s.", fileName);
		return false;
	}

	/* プロジェクトのデシリアライズ */
	cJSON *jsonRoot = cJSON_Parse(text);
	bool result = AppProjectDeserializeFromJson(jsonRoot, projectBasePath);

	/* デシリアライズ結果をメッセージボックスで通知 */
	if (result) {
		/*
			プロジェクトファイルをとっかえひっかえ閲覧する場合に煩わしいので、
			インポート成功のメッセージボックスは出さない。
			下記のコードをコメントアウトした。
				AppMessageBox(APP_NAME, "Import project successfully.");
		*/

		/* 現在のプロジェクトファイル名を保存 */
		strcpy_s(s_projectFileName, sizeof(s_projectFileName), fileName);
	} else {
		AppErrorMessageBox(APP_NAME, "Failed to import project %s.", fileName);
	}

	/* タイトルバーの表示を更新 */
	AppUpdateWindowTitleBar();

	/* シェーダは強制的に再読み込み */
	s_soundShaderFileStat.st_mtime = 0;
	s_graphicsShaderFileStat.st_mtime = 0;

	/* リソース解放して終了 */
	cJSON_Delete(jsonRoot);
	free(text);
	return result;
}

bool AppProjectExport(const char *fileName){
	/* プロジェクトのベースパス抽出 */
	char projectBasePath[MAX_PATH] = {0};
	SplitDirectoryPathFromFilePath(projectBasePath, sizeof(projectBasePath), fileName);

	/* プロジェクトをメモリ上にシリアライズ */
	cJSON *jsonRoot = cJSON_CreateObject();
	AppProjectSerializeToJson(jsonRoot, projectBasePath);

	/* ファイルに保存 */
	bool result;
	{
		FILE *file = fopen(fileName, "wb");
		if (file == NULL) {
			AppErrorMessageBox(APP_NAME, "Failed to export project %s.", fileName);
			result = false;
		} else {
			fputs(cJSON_Print(jsonRoot), file);
			fclose(file);
			result = true;
			AppMessageBox(APP_NAME, "Export project successfully.");

			strcpy_s(s_projectFileName, sizeof(s_projectFileName), fileName);
			AppUpdateWindowTitleBar();
		}
	}

	/* リソース解放して終了 */
	cJSON_Delete(jsonRoot);
	return result;
}

bool AppProjectAutoExport(bool confirm){
	/* プロジェクトを読み込んでいないならキャンセル */
	if (strcmp(s_projectFileName, "") == 0) return false;

	/* プロジェクトのベースパス抽出 */
	char projectBasePath[MAX_PATH] = {0};
	SplitDirectoryPathFromFilePath(projectBasePath, sizeof(projectBasePath), s_projectFileName);

	/* プロジェクトをメモリ上にシリアライズ */
	cJSON *jsonRoot = cJSON_CreateObject();
	AppProjectSerializeToJson(jsonRoot, projectBasePath);

	/* シリアライズ結果が既存のプロジェクトファイルと一致するならキャンセル */
	{
		char *text = MallocReadTextFile(s_projectFileName);
		if (text != NULL) {
			bool compareResult = strcmp(cJSON_Print(jsonRoot), text);
			free(text);
			if (compareResult == 0) {
				cJSON_Delete(jsonRoot);
				return true;
			}
		}
	}

	/* エクスポート前に確認 */
	bool flag = true;
	if (confirm) {
		if (AppYesNoMessageBox(APP_NAME, "The project has been updated.\n\nDo you want to overwrite %s?", s_projectFileName) == false) {
			flag = false;
		}
	}

	/* エクスポートを実行するか？ */
	bool result = true;
	if (flag) {
		/* ファイルに保存 */
		FILE *file = fopen(s_projectFileName, "wb");
		if (file == NULL) {
			AppErrorMessageBox(APP_NAME, "Failed to export project %s.", s_projectFileName);
			result = false;
		} else {
			fputs(cJSON_Print(jsonRoot), file);
			fclose(file);
			result = true;
		}
	}

	/* リソース解放して終了 */
	cJSON_Delete(jsonRoot);
	return result;
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
static bool AppReloadGraphicsShader(){
	GraphicsDeleteShader();
	s_graphicsCreateShaderSucceeded = GraphicsCreateShader(s_graphicsShaderCode);
	if (s_preferenceSettings.enableAutoRestartByGraphicsShader) {
		AppRestart();
	}
	return s_graphicsCreateShaderSucceeded;
}

static bool AppReloadSoundShader(){
	/*
		シェーダリロード時のサウンド周りのリセットは厄介な問題。
		シェーダコンパイル中にも再生位置は進んでしまう。
		一時停止して先頭にシーク、サウンド生成が完了したのち再生する。
	*/

	SoundDeleteShader();
	s_soundCreateShaderSucceeded = SoundCreateShader(s_soundShaderCode);
	if (s_preferenceSettings.enableAutoRestartBySoundShader) {
		SoundPauseWaveOut();
		SoundSeekWaveOut(0);
	}
	SoundClearOutputBuffer();
	SoundUpdate(s_frameCount);
	if (s_preferenceSettings.enableAutoRestartBySoundShader) {
		AppRestart();
	}
	return s_soundCreateShaderSucceeded;
}

void AppGetDefaultDirectoryName(char *directoryName, size_t directoryNameSizeInBytes){
	/*
		デフォルトディレクトリとして妥当なパスを現在のプロジェクトファイルや
		シェーダのパスなどから作成する。
	*/
	if (strcmp(s_projectFileName, "") != 0) {
		SplitDirectoryPathFromFilePath(directoryName, directoryNameSizeInBytes, s_projectFileName);
		return;
	}
	if (strcmp(s_graphicsShaderFileName, "") != 0) {
		SplitDirectoryPathFromFilePath(directoryName, directoryNameSizeInBytes, s_graphicsShaderFileName);
		return;
	}
	if (strcmp(s_soundShaderFileName, "") != 0) {
		SplitDirectoryPathFromFilePath(directoryName, directoryNameSizeInBytes, s_soundShaderFileName);
		return;
	}
	char selfPath[MAX_PATH] = {0};
	GetModuleFileName(NULL, selfPath, sizeof(selfPath));
	SplitDirectoryPathFromFilePath(directoryName, directoryNameSizeInBytes, selfPath);
}

bool AppOpenDefaultGraphicsShader(){
	memset(s_graphicsShaderFileName, 0, sizeof(s_graphicsShaderFileName));
	if (s_graphicsShaderCode != NULL) free(s_graphicsShaderCode);
	s_graphicsShaderCode = MallocCopyString(s_defaultGraphicsShaderCode);
	return AppReloadGraphicsShader();
}

bool AppOpenDefaultSoundShader(){
	memset(s_soundShaderFileName, 0, sizeof(s_soundShaderFileName));
	if (s_soundShaderCode != NULL) free(s_soundShaderCode);
	s_soundShaderCode = MallocCopyString(s_defaultSoundShaderCode);
	return AppReloadSoundShader();
}

bool AppOpenGraphicsShaderFile(const char *fileName){
	if (IsValidFileName(fileName) == false) {
		AppErrorMessageBox(APP_NAME, "Invalid file name %s.", fileName);
		return false;
	}

	printf("open a graphics shader file %s.\n", fileName);
	strcpy_s(s_graphicsShaderFileName, sizeof(s_graphicsShaderFileName), fileName);
	AppUpdateWindowTitleBar();
	s_graphicsShaderFileStat.st_mtime = 0;	/* 強制的に再読み込み */

	return true;
}
bool AppOpenSoundShaderFile(const char *fileName){
	if (IsValidFileName(fileName) == false) {
		AppErrorMessageBox(APP_NAME, "Invalid file name %s.", fileName);
		return false;
	}

	printf("open a sound shader file %s.\n", fileName);
	strcpy_s(s_soundShaderFileName, sizeof(s_soundShaderFileName), fileName);
	AppUpdateWindowTitleBar();
	s_soundShaderFileStat.st_mtime = 0;		/* 強制的に再読み込み */

	return true;
}

bool AppOpenDragAndDroppedFile(const char *fileName){
	if (IsValidFileName(fileName) == false) {
		AppErrorMessageBox(APP_NAME, "Invalid file name %s.", fileName);
		return false;
	}

	if (IsSuffix(fileName, ".gfx.glsl")) {
		return AppOpenGraphicsShaderFile(fileName);
	} else
	if (IsSuffix(fileName, ".snd.glsl")) {
		return AppOpenSoundShaderFile(fileName);
	} else
	if (IsSuffix(fileName, ".json")) {
		AppProjectAutoExport(true);
		return AppProjectImport(fileName);
	} else
	if (IsSuffix(fileName, ".png")
	||	IsSuffix(fileName, ".dds")
	) {
		if (AppUserTexturesLoad(0, fileName) == false) {
			AppErrorMessageBox(APP_NAME, "Failed to load texture %s.", fileName);
			return false;
		}
		return true;
	} else {
		AppErrorMessageBox(APP_NAME, "Cannot recognize file type.");
		return false;
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
void AppClearAllRenderTargets(){
	printf("clear all render targets.\n");
	GraphicsClearAllRenderTargets();
}

void AppRestart(){
	printf("restart.\n");
	HighPrecisionTimerReset(0);
	SoundRestartWaveOut();
	s_paused = false;
	s_frameCount = 0;
}

void AppResetCamera(){
	printf("reset the camera.\n");
	CameraInitialize();
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
		printf("pause.\n");
		s_paused = true;
		SoundPauseWaveOut();
		s_fp64PausedTime = HighPrecisionTimerGet();
	}
}

void AppResume(){
	if (s_paused) {
		printf("resume.\n");
		s_paused = false;
		SoundResumeWaveOut();
		HighPrecisionTimerReset(s_fp64PausedTime);
	}
}

static void AppSeekInSamples(int samples){
	s_waveOutSampleOffset = SoundGetWaveOutPos() + samples;
	if (s_waveOutSampleOffset < 0) s_waveOutSampleOffset = 0;
	double fp64OffsetInSeconds = s_waveOutSampleOffset / (double)NUM_SOUND_SAMPLES_PER_SEC;
	HighPrecisionTimerReset(fp64OffsetInSeconds);
	SoundSeekWaveOut(s_waveOutSampleOffset);
	if (s_paused) {
		s_fp64PausedTime = fp64OffsetInSeconds;
	}
}

void AppSlowForward(){
	AppPause();
	AppSeekInSamples(0x100);
	s_frameCount++;
}

void AppSlowBackward(){
	AppPause();
	AppSeekInSamples(-0x100);
	s_frameCount--;
}

void AppFastForward(){
	AppSeekInSamples(WAVEOUT_SEEKSTEP_IN_SAMPLES);
	s_frameCount++;
}

void AppFastBackward(){
	AppSeekInSamples(-WAVEOUT_SEEKSTEP_IN_SAMPLES);
	s_frameCount--;
}


bool AppUpdate(){
	/* 経過時間を取得 */
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
		static double s_fp64Fps = 60.0;
		++s_frameSkip;

		/* pause 中でなければ 1 秒ごとに FPS を求める */
		if (s_paused == false) {
			if (floor(fp64CurrentTime) > floor(s_fp64PrevTime)) {
				if (s_graphicsCreateShaderSucceeded) {
					s_fp64Fps = (double)s_frameSkip / (fp64CurrentTime - s_fp64PrevTime);
				}
				s_fp64PrevTime = fp64CurrentTime;
				s_frameSkip = 0;
			}
		}

		/* 時間が巻き戻っているなら修正 */
		if (fp64CurrentTime < s_fp64PrevTime) {
			s_fp64PrevTime = fp64CurrentTime;
		}

		/* ステートを ImGui で表示 */
		if (s_imGuiStatus.displayCurrentStatus) {
			ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoSavedSettings;
			ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
			if (ImGui::Begin("Current Status", NULL, window_flags)) {
				ImGui::Text(
					"time       %.2f\n"
					"FPS        %.2f\n"
					"frameCount %d\n"
					"waveOutPos 0x%08x\n"
					,
					fp64CurrentTime,
					s_fp64Fps,
					s_frameCount,
					SoundGetWaveOutPos()
				);
			}
			ImGui::End();
		}
	}

	/* カメラコントロールを要求するシェーダでは、カメラの設定を ImGui で表示 */
	if (GraphicsShaderRequiresCameraControlUniforms()) {
		if (s_imGuiStatus.displayCameraSettings) {
			ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoSavedSettings;
			ImGui::SetNextWindowPos(ImVec2(200, 0), ImGuiCond_FirstUseEver);
			if (ImGui::Begin("Camera Settings", NULL, window_flags)) {
				ImGui::PushItemWidth(ImGui::GetFontSize() * 16);

				ImGui::InputFloat3("position", s_camera.vec3Pos);
				ImGui::InputFloat3("angle", s_camera.vec3Ang);
				float fovYInDegrees = s_camera.fovYInRadians * 180.0f / PI;
				ImGui::SliderFloat("fovY", &fovYInDegrees, 0.0f, 90.0f, "%.2f");
				s_camera.fovYInRadians = fovYInDegrees * PI / 180.0f;
				bool ret = ImGui::Button("Reset");
				if (ret) AppResetCamera();

				ImGui::PopItemWidth();
			}
			ImGui::End();
		}
	}

	/* サウンドシェーダの更新 */
	if (IsValidFileName(s_soundShaderFileName)) {
		if (IsFileUpdated(s_soundShaderFileName, &s_soundShaderFileStat)) {
			printf("update the sound shader.\n");
			if (s_soundShaderCode != NULL) free(s_soundShaderCode);
			/* ファイルのロック状態が継続していることがあるため、リトライしながら読む */
			for (int retryCount = 0; retryCount < 10; retryCount++) {
				s_soundShaderCode = MallocReadTextFile(s_soundShaderFileName);
				if (s_soundShaderCode != NULL) break;
				printf("retry %d ... \n", retryCount);
				Sleep(100);
			}
			if (s_soundShaderCode == NULL) {
				AppErrorMessageBox(APP_NAME, "Failed to read %s.\n", s_soundShaderFileName);
			} else {
				AppReloadSoundShader();
			}
		}
	}

	/* グラフィクスシェーダの更新 */
	if (IsValidFileName(s_graphicsShaderFileName)) {
		if (IsFileUpdated(s_graphicsShaderFileName, &s_graphicsShaderFileStat)) {
			printf("update the graphics shader.\n");
			if (s_graphicsShaderCode != NULL) free(s_graphicsShaderCode);
			/* ファイルのロック状態が継続していることがあるため、リトライしながら読む */
			for (int retryCount = 0; retryCount < 10; retryCount++) {
				s_graphicsShaderCode = MallocReadTextFile(s_graphicsShaderFileName);
				if (s_graphicsShaderCode != NULL) break;
				printf("retry %d ... \n", retryCount);
				Sleep(100);
			}
			if (s_graphicsShaderCode == NULL) {
				AppErrorMessageBox(APP_NAME, "Failed to read %s.\n", s_graphicsShaderFileName);
			} else {
				AppReloadGraphicsShader();
			}
		}
	}

	/* カメラコントロールが必要ならカメラ更新 */
	if (GraphicsShaderRequiresCameraControlUniforms()) {
		CameraUpdate();
	}

	/* サウンドの更新 */
	CheckGlError("pre SoundUpdate");
	if (s_soundCreateShaderSucceeded) {
		SoundUpdate(s_frameCount);
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
			s_camera.fovYInRadians,
			s_camera.mat4x4CameraInWorld, s_camera.mat4x4PrevCameraInWorld,
			&s_renderSettings
		);
	}
	CheckGlError("post GraphicsUpdate");

	/* フレームの終わり */
	CheckGlError("pre glFlush");
	glFlush();
	CheckGlError("post glFlush");

	if (s_paused == false) s_frameCount++;
	return true;
}

/*=============================================================================
▼	ヘルプ表示関連
-----------------------------------------------------------------------------*/
bool AppHelpAbout(
){
	AppMessageBox(
		APP_NAME,
		"MinimalGL\n\n"
		"Copyright (c)2020 @yosshin4004\n"
		"https://github.com/yosshin4004/minimal_gl"
	);
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
	AppOpenDefaultSoundShader();
	if (GraphicsInitialize() == false) {
		AppErrorMessageBox(APP_NAME, "GraphicsInitialize() failed.");
		return false;
	}
	AppOpenDefaultGraphicsShader();
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
