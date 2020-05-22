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

/* テクスチャ及びフレームバッファのクリア */
void GraphicsClearAllTexturesAndFremeBuffers();

/* シェーダが frameCount uniform 変数を要求するか？ */
bool GraphicsShaderRequiresFrameCountUniform();

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
	int xReso,
	int yReso,
	float fovYAsRadian,
	const float mat4x4CameraInWorld[4][4],
	bool replaceAlphaByOne,
	const RenderSettings *settings
);

/* スクリーンショットキャプチャ */
bool GraphicsCaptureScreenShotAsUnorm8RgbaImage(
	const char *fileName,
	int waveOutPos,
	int frameCount,
	float time,
	int xReso,
	int yReso,
	float fovYAsRadian,
	const float mat4x4CameraInWorld[4][4],
	bool replaceAlphaByOne,
	const RenderSettings *settings
);

/* キューブマップキャプチャ */
bool GraphicsCaptureCubemap(
	const char *fileName,
	int waveOutPos,
	int frameCount,
	float time,
	int cubemapReso,
	const float mat4x4CameraInWorld[4][4],
	const RenderSettings *settings
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
