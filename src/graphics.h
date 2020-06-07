/* Copyright (C) 2018 Yosshin(@yosshin4004) */

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
	PixelFormatUnorm8RGBA,
	PixelFormatFp16RGBA,
	PixelFormatFp32RGBA,
} PixelFormat;
typedef enum {
	SwapIntervalAllowTearing,
	SwapIntervalHsync,
	SwapIntervalVsync,
} SwapInterval;

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

/* グラフィクス用シェーダの作成 */
bool GraphicsCreateShader(
	const char *shaderCode
);

/* グラフィクス用シェーダの削除 */
bool GraphicsDeleteShader();

/* スクリーンショットキャプチャ */
bool GraphicsCaptureScreenShotAsUnorm8RgbaImageMemory(
	void *buffer,
	size_t bufferSizeInBytes,
	int waveOutPos,
	int frameCount,
	float time,
	float fovYAsRadian,
	const float mat4x4CameraInWorld[4][4],
	const RenderSettings *renderSettings,
	const CaptureScreenShotSettings *captureSettings
);

/* スクリーンショットキャプチャ */
bool GraphicsCaptureScreenShotAsUnorm8RgbaImage(
	int waveOutPos,
	int frameCount,
	float time,
	float fovYAsRadian,
	const float mat4x4CameraInWorld[4][4],
	const RenderSettings *renderSettings,
	const CaptureScreenShotSettings *captureSettings
);

/* キューブマップキャプチャ */
bool GraphicsCaptureCubemap(
	int waveOutPos,
	int frameCount,
	float time,
	const float mat4x4CameraInWorld[4][4],
	const RenderSettings *renderSettings,
	const CaptureCubemapSettings *captureSettings
);

/* グラフィクスの更新 */
void GraphicsUpdate(
	int waveOutPos,
	int frameCount,
	float time,
	int xMouse,
	int yMouse,
	int mouseLButtonPressed,
	int mouseMButtonPressed,
	int mouseRButtonPressed,
	int xReso,
	int yReso,
	float fovYAsRadian,
	const float mat4x4CameraInWorld[4][4],
	const RenderSettings *settings
);

/* グラフィクスの初期化 */
bool GraphicsInitialize();

/* グラフィクスの終了処理 */
bool GraphicsTerminate();


#endif
