/* Copyright (C) 2018 Yosshin(@yosshin4004) */


#include "common.h"
#include "pixel_format.h"


#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_


typedef enum {
	TextureFilterNearest,
	TextureFilterLinear,
} TextureFilter;
typedef enum {
	TextureWrapRepeat,
	TextureWrapClampToEdge,
	TextureWrapMirroredRepeat,
} TextureWrap;
typedef enum {
	SwapIntervalAllowTearing,
	SwapIntervalHsync,
	SwapIntervalVsync,
} SwapInterval;

struct CurrentFrameParams {
	int waveOutPos;
	int frameCount;
	float time;
	int xMouse;
	int yMouse;
	int mouseLButtonPressed;
	int mouseMButtonPressed;
	int mouseRButtonPressed;
	int xReso;
	int yReso;
	float fovYInRadians;
	float mat4x4CameraInWorld[4][4];
	float mat4x4PrevCameraInWorld[4][4];
};

struct RenderSettings {
	PixelFormat pixelFormat;
	bool enableMultipleRenderTargets;
	int numEnabledRenderTargets;

	bool enableBackBuffer;
	bool enableMipmapGeneration;
	TextureFilter textureFilter;
	TextureWrap textureWrap;

	bool enableSwapIntervalControl;
	SwapInterval swapInterval;
};

struct CaptureScreenShotSettings {
	char fileName[MAX_PATH];
	int xReso;
	int yReso;
	bool replaceAlphaByOne;
};

struct CaptureCubemapSettings {
	char fileName[MAX_PATH];
	int reso;
};

/* 全レンダーターゲットのクリア */
void GraphicsClearAllRenderTargets();

/* シェーダが frameCount uniform 変数を要求するか？ */
bool GraphicsShaderRequiresFrameCountUniform();

/* シェーダが camera control を利用しているか？ */
bool GraphicsShaderRequiresCameraControlUniforms();

/* ユーザーテクスチャの読み込み */
bool GraphicsLoadUserTexture(
	const char *fileName,
	int userTextureIndex
);

/* ユーザーテクスチャの破棄 */
bool GraphicsDeleteUserTexture(
	int userTextureIndex
);

/* 頂点シェーダの作成 */
bool GraphicsCreateVertexShader(
	const char *shaderCode
);

/* 頂点シェーダの削除 */
bool GraphicsDeleteVertexShader();

/* フラグメントシェーダの作成 */
bool GraphicsCreateFragmentShader(
	const char *shaderCode
);

/* フラグメントシェーダの削除 */
bool GraphicsDeleteFragmentShader();

/* シェーダパイプラインの作成 */
bool GraphicsCreateShaderPipeline();

/* シェーダパイプラインの削除 */
bool GraphicsDeleteShaderPipeline();

/* スクリーンショットキャプチャ */
bool GraphicsCaptureScreenShotOnMemory(
	void *buffer,
	size_t bufferSizeInBytes,
	const CurrentFrameParams *params,
	const RenderSettings *renderSettings,
	const CaptureScreenShotSettings *captureSettings
);

/* スクリーンショットキャプチャ */
bool GraphicsCaptureScreenShotAsPngTexture2d(
	const CurrentFrameParams *params,
	const RenderSettings *renderSettings,
	const CaptureScreenShotSettings *captureSettings
);

/* スクリーンショットキャプチャ */
bool GraphicsCaptureScreenShotAsDdsTexture2d(
	const CurrentFrameParams *params,
	const RenderSettings *renderSettings,
	const CaptureScreenShotSettings *captureSettings
);

/* キューブマップキャプチャ */
bool GraphicsCaptureAsDdsCubemap(
	const CurrentFrameParams *params,
	const RenderSettings *renderSettings,
	const CaptureCubemapSettings *captureSettings
);

/* グラフィクスの更新 */
void GraphicsUpdate(
	const CurrentFrameParams *params,
	const RenderSettings *settings
);

/* グラフィクスの初期化 */
bool GraphicsInitialize();

/* グラフィクスの終了処理 */
bool GraphicsTerminate();


#endif
