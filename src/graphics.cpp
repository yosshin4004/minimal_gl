/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#include <math.h>
#include "common.h"
#include "app.h"
#include "graphics.h"
#include "config.h"
#include "glext_util.h"
#include "dds_util.h"
#include "png_util.h"
#include "sound.h"
#include "dds_parser.h"


#define USER_TEXTURE_START_INDEX				(8)
#define BUFFER_INDEX_FOR_SOUND_VISUALIZER_INPUT	(0)

static GLuint s_mrtTextures[2 /* 裏表 */][NUM_RENDER_TARGETS] = {0};
static GLuint s_mrtFrameBuffer = 0;
static struct {
	GLenum target;
	GLuint id;
} s_userTextures[NUM_USER_TEXTURES] = {0};
static GLuint s_graphicsProgramId = 0;
static RenderSettings s_currentRenderSettings = {0};
static int s_xReso = DEFAULT_SCREEN_XRESO;
static int s_yReso = DEFAULT_SCREEN_YRESO;


static void GraphicsCreateFrameBuffer(
	int xReso,
	int yReso,
	const RenderSettings *settings
){
	/* MRT フレームバッファ作成 */
	glExtGenFramebuffers(
		/* GLsizei n */		1,
	 	/* GLuint *ids */	&s_mrtFrameBuffer
	);

	for (int doubleBufferIndex = 0; doubleBufferIndex < 2; doubleBufferIndex++) {
		/* テクスチャ作成 */
		glGenTextures(
			/* GLsizei n */				NUM_RENDER_TARGETS,
			/* GLuint * textures */		s_mrtTextures[doubleBufferIndex]
		);

		/* レンダーターゲットの巡回 */
		for (int renderTargetIndex = 0; renderTargetIndex < NUM_RENDER_TARGETS; renderTargetIndex++) {
			glBindTexture(
				/* GLenum target */		GL_TEXTURE_2D,
				/* GLuint texture */	s_mrtTextures[doubleBufferIndex][renderTargetIndex]
			);

			GLint internalformat = 0;
			GLenum type = 0;
			switch (settings->pixelFormat) {
				case PixelFormatUnorm8RGBA: {
					internalformat = GL_RGBA;
					type = GL_UNSIGNED_BYTE;
				} break;
				case PixelFormatFp16RGBA: {
					internalformat = GL_RGBA16F;
					type = GL_HALF_FLOAT;
				} break;
				case PixelFormatFp32RGBA: {
					internalformat = GL_RGBA32F;
					type = GL_FLOAT;
				} break;
				default : {
					assert(false);
				} break;
			}

			/* テクスチャリソースを生成 */
			glTexImage2D(
				/* GLenum target */			GL_TEXTURE_2D,
				/* GLint level */			0,
				/* GLint internalformat */	internalformat,
				/* GLsizei width */			xReso,
				/* GLsizei height */		yReso,
				/* GLint border */			0,
				/* GLenum format */			GL_RGBA,
				/* GLenum type */			type,
				/* const void * data */		NULL
			);
		}
	}
}

static void GraphicsDeleteFrameBuffer(
){
	/* フレームバッファアンバインド */
	glExtBindFramebuffer(
		/* GLenum target */			GL_FRAMEBUFFER,
		/* GLuint framebuffer */	0	/* unbind */
	);

	/* MRT フレームバッファ削除 */
	glExtDeleteFramebuffers(
		/* GLsizei n */				1,
	 	/* GLuint *ids */			&s_mrtFrameBuffer
	);

	for (int doubleBufferIndex = 0; doubleBufferIndex < 2; doubleBufferIndex++) {
		/* MRT テクスチャ削除 */
		for (int renderTargetIndex = 0; renderTargetIndex < NUM_RENDER_TARGETS; renderTargetIndex++) {
			/* テクスチャアンバインド */
			glExtActiveTexture(GL_TEXTURE0 + renderTargetIndex);
			glBindTexture(
				/* GLenum target */		GL_TEXTURE_2D,
				/* GLuint texture */	0	/* unbind */
			);
		}

		/* テクスチャ削除 */
		glDeleteTextures(
			/* GLsizei n */			NUM_RENDER_TARGETS,
			/* GLuint * textures */	s_mrtTextures[doubleBufferIndex]
		);
	}
}

void GraphicsClearAllRenderTargets(){
	GraphicsDeleteFrameBuffer();
	GraphicsCreateFrameBuffer(s_xReso, s_yReso, &s_currentRenderSettings);
}

bool GraphicsShaderRequiresFrameCountUniform(){
	if (s_graphicsProgramId != 0) {
		return ExistsShaderUniform(s_graphicsProgramId, UNIFORM_LOCATION_FRAME_COUNT, GL_INT);
	}
	return false;
}

bool GraphicsShaderRequiresCameraControlUniforms(){
	if (s_graphicsProgramId != 0) {
		return
			ExistsShaderUniform(s_graphicsProgramId, UNIFORM_LOCATION_TAN_FOVY, GL_FLOAT)
		||	ExistsShaderUniform(s_graphicsProgramId, UNIFORM_LOCATION_CAMERA_COORD, GL_FLOAT_MAT4);
	}
	return false;
}


static bool GraphicsLoadUserTextureSubAsPng(
	const char *fileName,
	int userTextureIndex
){
	/* png ファイルの読み込み */
	void *data = NULL;
	int numComponents = 0;
	int width = 0;
	int height = 0;
	bool ret = ReadImageFileAsUnorm8RgbaPng(
		/* const char *fileName */	fileName,
		/* void **dataRet */		&data,
		/* int *numComponentsRet */	&numComponents,
		/* int *widthRet */			&width,
		/* int *heightRet */		&height
	);
	if (ret == false) return false;

	/* target を決定 */
	s_userTextures[userTextureIndex].target = GL_TEXTURE_2D;

	/* テクスチャのバインド */
	glBindTexture(
		/* GLenum target */			GL_TEXTURE_2D,
		/* GLuint texture */		s_userTextures[userTextureIndex].id
	);

	/* テクスチャの設定 */
	GLint internalformat = 0;
	switch (numComponents) {
		case 1: {
			internalformat = GL_RED;
		} break;
		case 2: {
			internalformat = GL_RG;
		} break;
		case 3: {
			internalformat = GL_RGB;
		} break;
		case 4: {
			internalformat = GL_RGBA;
		} break;
	}
	if (internalformat != 0) {
		glTexImage2D(
			/* GLenum target */			GL_TEXTURE_2D,
			/* GLint level */			0,
			/* GLint internalformat */	internalformat,
			/* GLsizei width */			width,
			/* GLsizei height */		height,
			/* GLint border */			0,
			/* GLenum format */			internalformat,
			/* GLenum type */			GL_UNSIGNED_BYTE,
			/* const void * data */		data
		);

		/* 常にミップマップ生成 */
		glExtGenerateMipmap(GL_TEXTURE_2D);
	}

	/* テクスチャのアンバインド */
	glBindTexture(
		/* GLenum target */			GL_TEXTURE_2D,
		/* GLuint texture */		0
	);

	/* 画像データの破棄 */
	free(data);

	return true;
}


static bool GraphicsLoadUserTextureSubAsDds(
	const char *fileName,
	int userTextureIndex
){
	/* dds ファイルの読み込み */
	size_t ddsFileSizeInBytes;
	void *ddsFileImage = MallocReadFile(fileName, &ddsFileSizeInBytes);

	/* dds ファイルのパース */
	DdsParser parser;
	if (DdsParser_Initialize(&parser, ddsFileImage, (int)ddsFileSizeInBytes) == false) {
		free(ddsFileImage);
		return false;
	}

	/* DxgiFormat から OpenGL のフォーマット情報に変換 */
	DdsGlFormat glFormat = DdsDxgiFormatToGlFormat(parser.info.dxgiFormat);
	if (glFormat.internalformat == 0) {
		free(ddsFileImage);
		return false;
	}

	/* パース結果の確認 */
	printf(
		"\n"
		"DdsParser\n"
		"	dxgiFormat      %d\n"
		"	numBitsPerPixel %d\n"
		"	width           %d\n"
		"	height          %d\n"
		"	depth           %d\n"
		"	arraySize       %d\n"
		"	hasCubemap      %d\n"
		"	numMips         %d\n"
		"	blockCompressed %d\n",
		parser.info.dxgiFormat,
		parser.info.numBitsPerPixel,
		parser.info.width,
		parser.info.height,
		parser.info.depth,
		parser.info.arraySize,
		parser.info.hasCubemap,
		parser.info.numMips,
		parser.info.blockCompressed
	);

	/* 対応していない形式ならエラー */
	if (
		parser.info.depth != 1
	||	parser.info.arraySize != 1
	) {
		free(ddsFileImage);
		return false;
	}

	/* face 数を決定（デフォルトで 1、キューブマップで 6）*/
	int numFace = 1;
	GLenum targetFace = GL_TEXTURE_2D;
	s_userTextures[userTextureIndex].target = GL_TEXTURE_2D;
	if (parser.info.hasCubemap) {
		numFace = 6;
		targetFace = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
		s_userTextures[userTextureIndex].target = GL_TEXTURE_CUBE_MAP;
	}

	/* テクスチャのバインド */
	glBindTexture(
		/* GLenum target */			s_userTextures[userTextureIndex].target,
		/* GLuint texture */		s_userTextures[userTextureIndex].id
	);

	/* ミップレベルの巡回 */
	for (int faceIndex = 0; faceIndex < numFace; faceIndex++) {
		for (int mipLevel = 0; mipLevel < parser.info.numMips; mipLevel++) {
			DdsSubData subData;
			DdsParser_GetSubData(&parser, 0, faceIndex, mipLevel, &subData);

			if (parser.info.blockCompressed) {
				CheckGlError("pre glExtCompressedTexImage2D");
				glExtCompressedTexImage2D(
					/* GLenum target */			targetFace + faceIndex,
					/* GLint level */			mipLevel,
					/* GLenum internalformat */	glFormat.internalformat,
					/* GLsizei width */			subData.width,
					/* GLsizei height */		subData.height,
					/* GLint border */			false,
					/* GLsizei imageSize */		(GLsizei)subData.sizeInBytes,
					/* const void * data */		subData.buff
				);
				CheckGlError("post glExtCompressedTexImage2D");
			} else {
				CheckGlError("pre glTexImage2D");
				glTexImage2D(
					/* GLenum target */			targetFace + faceIndex,
					/* GLint level */			mipLevel,
					/* GLint internalformat */	glFormat.internalformat,
					/* GLsizei width */			subData.width,
					/* GLsizei height */		subData.height,
					/* GLint border */			false,
					/* GLenum format */			glFormat.format,
					/* GLenum type */			glFormat.type,
					/* const void * data */		subData.buff
				);
				CheckGlError("post glTexImage2D");
			}
		}
	}

	/* ミップ数上限の設定（これによりミップマップが有効化される）*/
	glTexParameteri(
		/* GLenum target */	s_userTextures[userTextureIndex].target,
		/* GLenum pname */	GL_TEXTURE_MAX_LEVEL,
		/* GLint param */	parser.info.numMips - 1
	);

	/* ミップマップ生成 */
//	glExtGenerateMipmap(s_userTextures[userTextureIndex].target);

	/* テクスチャのアンバインド */
	glBindTexture(
		/* GLenum target */			s_userTextures[userTextureIndex].target,
		/* GLuint texture */		0
	);

	/* dds ファイルイメージの破棄 */
	free(ddsFileImage);

	return true;
}


bool GraphicsLoadUserTexture(
	const char *fileName,
	int userTextureIndex
){
	/* エラーチェック */
	if (userTextureIndex < 0 || NUM_USER_TEXTURES <= userTextureIndex) return false;

	/* 既存のテクスチャがあるなら破棄 */
	if (s_userTextures[userTextureIndex].id != 0) {
		glDeleteTextures(
			/* GLsizei n */					1,
			/* const GLuint * textures */	&s_userTextures[userTextureIndex].id
		);
		s_userTextures[userTextureIndex].id = 0;
	}

	/* テクスチャ作成 */
	glGenTextures(
		/* GLsizei n */				1,
		/* GLuint * textures */		&s_userTextures[userTextureIndex].id
	);

	/* 画像ファイルの読み込み */
	bool succeeded = false;
	if (GraphicsLoadUserTextureSubAsPng(fileName, userTextureIndex)) {
		succeeded = true;
	} else
	if (GraphicsLoadUserTextureSubAsDds(fileName, userTextureIndex)) {
		succeeded = true;
	}

	return succeeded;
}

bool GraphicsDeleteUserTexture(
	int userTextureIndex
){
	/* エラーチェック */
	if (userTextureIndex < 0 || NUM_USER_TEXTURES <= userTextureIndex) return false;

	/* 既存のテクスチャがあるなら破棄 */
	if (s_userTextures[userTextureIndex].id != 0) {
		glDeleteTextures(
			/* GLsizei n */					1,
			/* const GLuint * textures */	&s_userTextures[userTextureIndex].id
		);
		s_userTextures[userTextureIndex].id = 0;
	}

	return true;
}

bool GraphicsCreateShader(
	const char *shaderCode
){
	printf("setup the graphics shader ...\n");
	const GLchar *(strings[]) = {
		SkipBomConst(shaderCode)
	};
	assert(s_graphicsProgramId == 0);
	s_graphicsProgramId = CreateShader(GL_FRAGMENT_SHADER, SIZE_OF_ARRAY(strings), strings);
	if (s_graphicsProgramId == 0) {
		printf("setup the graphics shader ... fialed.\n");
		return false;
	}
	DumpShaderInterfaces(s_graphicsProgramId);
	printf("setup the graphics shader ... done.\n");

	return true;
}

bool GraphicsDeleteShader(
){
	if (s_graphicsProgramId == 0) return false;
	glFinish();
	glExtDeleteProgram(s_graphicsProgramId);
	s_graphicsProgramId = 0;
	return true;
}

static void GraphicsSetTextureSampler(
	GLenum target,
	TextureFilter textureFilter,
	TextureWrap textureWrap,
	bool useMipmap
){
	{
		GLint minFilter = 0;
		GLint magFilter = 0;
		switch (textureFilter) {
			case TextureFilterNearest: {
				if (useMipmap) {
					minFilter = GL_NEAREST_MIPMAP_NEAREST;
				} else {
					minFilter = GL_NEAREST;
				}
				magFilter = GL_NEAREST;
			} break;
			case TextureFilterLinear: {
				if (useMipmap) {
					minFilter = GL_LINEAR_MIPMAP_LINEAR;
				} else {
					minFilter = GL_LINEAR;
				}
				magFilter = GL_LINEAR;
			} break;
			default : {
				assert(false);
			} break;
		}
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter);
	}

	{
		GLint param = GL_REPEAT;
		switch (textureWrap) {
			case TextureWrapRepeat: {
				param = GL_REPEAT;
			} break;
			case TextureWrapClampToEdge: {
				param = GL_CLAMP_TO_EDGE;
			} break;
			case TextureWrapMirroredRepeat: {
				param = GL_MIRRORED_REPEAT;
			} break;
			default : {
				assert(false);
			} break;
		}
		if (target == GL_TEXTURE_CUBE_MAP) {
			param = GL_CLAMP_TO_EDGE;
		}
		glTexParameteri(target, GL_TEXTURE_WRAP_S, param);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, param);
		glTexParameteri(target, GL_TEXTURE_WRAP_R, param);
	}
}

static void GraphicsDrawFullScreenQuad(
	GLuint outputFrameBuffer,
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
	const float tanFovY,
	const float mat4x4CameraInWorld[4][4],
	const RenderSettings *settings
){
	/* RenderSettings 更新を認識 */
	if (s_xReso != xReso
	||	s_yReso != yReso
	||	memcmp(&s_currentRenderSettings, settings, sizeof(RenderSettings)) != 0
	) {
		s_xReso = xReso;
		s_yReso = yReso;
		s_currentRenderSettings = *settings;
		GraphicsDeleteFrameBuffer();
		GraphicsCreateFrameBuffer(xReso, yReso, settings);
	}

	/* シェーダのバインド */
	glExtUseProgram(
		/* GLuint program */	s_graphicsProgramId
	);

	/* MRT テクスチャの設定 */
	if (settings->enableBackBuffer) {
		for (int renderTargetIndex = 0; renderTargetIndex < NUM_RENDER_TARGETS; renderTargetIndex++) {
			/* 裏テクスチャのバインド */
			glExtActiveTexture(GL_TEXTURE0 + renderTargetIndex);
			glBindTexture(
				/* GLenum target */		GL_TEXTURE_2D,
				/* GLuint texture */	s_mrtTextures[(frameCount & 1) ^ 1] [renderTargetIndex]
			);

			/* サンプラの設定 */
			GraphicsSetTextureSampler(GL_TEXTURE_2D, settings->textureFilter, settings->textureWrap, settings->enableMipmapGeneration);

			/* ミップマップ生成 */
			if (settings->enableMipmapGeneration) {
				glExtGenerateMipmap(GL_TEXTURE_2D);
			}
		}
	}

	/* ユーザーテクスチャの設定 */
	for (int userTextureIndex = 0; userTextureIndex < NUM_USER_TEXTURES; userTextureIndex++) {
		if (s_userTextures[userTextureIndex].id) {
			/* テクスチャのバインド */
			glExtActiveTexture(GL_TEXTURE0 + USER_TEXTURE_START_INDEX + userTextureIndex);
			glBindTexture(
				/* GLenum target */		s_userTextures[userTextureIndex].target,
				/* GLuint texture */	s_userTextures[userTextureIndex].id
			);

			/* サンプラの設定 */
			GraphicsSetTextureSampler(s_userTextures[userTextureIndex].target, settings->textureFilter, settings->textureWrap, true);
		}
	}

	/* サウンドバッファのバインド */
	glExtBindBufferBase(
		/* GLenum target */		GL_SHADER_STORAGE_BUFFER,
		/* GLuint index */		BUFFER_INDEX_FOR_SOUND_VISUALIZER_INPUT,
		/* GLuint buffer */		SoundGetOutputSsbo()
	);

	/* ユニフォームパラメータ設定 */
	if (ExistsShaderUniform(s_graphicsProgramId, UNIFORM_LOCATION_WAVE_OUT_POS, GL_INT)) {
		glExtUniform1i(
			/* GLint location */	UNIFORM_LOCATION_WAVE_OUT_POS,
			/* GLint v0 */			waveOutPos
		);
	}
	if (ExistsShaderUniform(s_graphicsProgramId, UNIFORM_LOCATION_FRAME_COUNT, GL_INT)) {
		glExtUniform1i(
			/* GLint location */	UNIFORM_LOCATION_FRAME_COUNT,
			/* GLint v0 */			frameCount
		);
	}
	if (ExistsShaderUniform(s_graphicsProgramId, UNIFORM_LOCATION_TIME, GL_FLOAT)) {
		glExtUniform1f(
			/* GLint location */	UNIFORM_LOCATION_TIME,
			/* GLfloat v0 */		time
		);
	}
	if (ExistsShaderUniform(s_graphicsProgramId, UNIFORM_LOCATION_RESO, GL_FLOAT_VEC2)) {
		glExtUniform2f(
			/* GLint location */	UNIFORM_LOCATION_RESO,
			/* GLfloat v0 */		(GLfloat)xReso,
			/* GLfloat v1 */		(GLfloat)yReso
		);
	}
	if (ExistsShaderUniform(s_graphicsProgramId, UNIFORM_LOCATION_MOUSE_POS, GL_FLOAT_VEC2)) {
		glExtUniform2f(
			/* GLint location */	UNIFORM_LOCATION_MOUSE_POS,
			/* GLfloat v0 */		(GLfloat)xMouse / (GLfloat)xReso,
			/* GLfloat v1 */		1.0f - (GLfloat)yMouse / (GLfloat)yReso
		);
	}
	if (ExistsShaderUniform(s_graphicsProgramId, UNIFORM_LOCATION_MOUSE_BUTTONS, GL_INT_VEC3)) {
		glExtUniform3i(
			/* GLint location */	UNIFORM_LOCATION_MOUSE_BUTTONS,
			/* GLint v0 */			mouseLButtonPressed,
			/* GLint v1 */			mouseMButtonPressed,
			/* GLint v2 */			mouseRButtonPressed
		);
	}
	if (ExistsShaderUniform(s_graphicsProgramId, UNIFORM_LOCATION_TAN_FOVY, GL_FLOAT)) {
		glExtUniform1f(
			/* GLint location */	UNIFORM_LOCATION_TAN_FOVY,
			/* GLfloat v0 */		tanFovY
		);
	}
	if (ExistsShaderUniform(s_graphicsProgramId, UNIFORM_LOCATION_CAMERA_COORD, GL_FLOAT_MAT4)) {
		glExtUniformMatrix4fv(
			/* GLint location */		UNIFORM_LOCATION_CAMERA_COORD,
			/* GLsizei count */			1,
			/* GLboolean transpose */	false,
			/* const GLfloat *value */	&mat4x4CameraInWorld[0][0]
		);
	}

	/* MRT フレームバッファのバインド */
	glExtBindFramebuffer(
		/* GLenum target */			GL_FRAMEBUFFER,
		/* GLuint framebuffer */	s_mrtFrameBuffer
	);

	/* ビューポートの設定 */
	glViewport(0, 0, xReso, yReso);

	/* MRT の設定 */
	{
		GLuint bufs[NUM_RENDER_TARGETS] = {0};
		assert(settings->numEnabledRenderTargets <= NUM_RENDER_TARGETS);
		int numRenderTargets = settings->enableMultipleRenderTargets? settings->numEnabledRenderTargets: 1;
		for (int renderTargetIndex = 0; renderTargetIndex < numRenderTargets; renderTargetIndex++) {
			/* 表テクスチャを MRT として登録 */
			glExtFramebufferTexture(
				/* GLenum target */			GL_FRAMEBUFFER,
				/* GLenum attachment */		GL_COLOR_ATTACHMENT0 + renderTargetIndex,
				/* GLuint texture */		s_mrtTextures[frameCount & 1] [renderTargetIndex],
				/* GLint level */			0
			);
			bufs[renderTargetIndex] = GL_COLOR_ATTACHMENT0 + renderTargetIndex;
		}
		glExtDrawBuffers(
			/* GLsizei n */				numRenderTargets,
			/* const GLenum *bufs */	bufs
		);
	}

	/* 描画 */
	glRects(-1, -1, 1, 1);

	/* 描画結果をデフォルトフレームバッファにコピー */
	glExtBlitNamedFramebuffer(
		/* GLuint readFramebuffer */	s_mrtFrameBuffer,
		/* GLuint drawFramebuffer */	outputFrameBuffer,
		/* GLint srcX0 */				0,
		/* GLint srcY0 */				0,
		/* GLint srcX1 */				xReso,
		/* GLint srcY1 */				yReso,
		/* GLint dstX0 */				0,
		/* GLint dstY0 */				0,
		/* GLint dstX1 */				xReso,
		/* GLint dstY1 */				yReso,
		/* GLbitfield mask */			GL_COLOR_BUFFER_BIT,
		/* GLenum filter */				GL_NEAREST
	);

	/* MRT フレームバッファのアンバインド */
	glExtBindFramebuffer(
		/* GLenum target */			GL_FRAMEBUFFER,
		/* GLuint framebuffer */	0	/* unbind */
	);

	/* サウンドバッファのアンバインド */
	glExtBindBufferBase(
		/* GLenum target */			GL_SHADER_STORAGE_BUFFER,
		/* GLuint index */			BUFFER_INDEX_FOR_SOUND_VISUALIZER_INPUT,
		/* GLuint buffer */			0	/* unbind */
	);

	/* ユーザーテクスチャのアンバインド */
	for (int userTextureIndex = 0; userTextureIndex < NUM_USER_TEXTURES; userTextureIndex++) {
		if (s_userTextures[userTextureIndex].id) {
			glExtActiveTexture(GL_TEXTURE0 + USER_TEXTURE_START_INDEX + userTextureIndex);
			glBindTexture(
				/* GLenum target */		s_userTextures[userTextureIndex].target,
				/* GLuint texture */	0	/* unbind */
			);
		}
	}

	/* MRT テクスチャのアンバインド */
	for (int renderTargetIndex = 0; renderTargetIndex < NUM_RENDER_TARGETS; renderTargetIndex++) {
		glExtActiveTexture(GL_TEXTURE0 + renderTargetIndex);
		glBindTexture(
			/* GLenum target */		GL_TEXTURE_2D,
			/* GLuint texture */	0	/* unbind */
		);
	}

	/* シェーダのアンバインド */
	glExtUseProgram(NULL);
}

bool GraphicsCaptureScreenShotAsUnorm8RgbaImageMemory(
	void *buffer,
	size_t bufferSizeInBytes,
	int waveOutPos,
	int frameCount,
	float time,
	float fovYInRadians,
	const float mat4x4CameraInWorld[4][4],
	const RenderSettings *renderSettings,
	const CaptureScreenShotSettings *captureSettings
){
	/* バッファ容量が不足しているならエラー */
	if (bufferSizeInBytes < (size_t)(captureSettings->xReso * captureSettings->yReso * 4)) return false;

	/* FBO 作成 */
	GLuint offscreenRenderTargetFbo = 0;
	GLuint offscreenRenderTargetTexture = 0;
	glExtGenFramebuffers(
		/* GLsizei n */				1,
	 	/* GLuint *ids */			&offscreenRenderTargetFbo
	);
	glExtBindFramebuffer(
		/* GLenum target */			GL_FRAMEBUFFER,
		/* GLuint framebuffer */	offscreenRenderTargetFbo
	);

	/* レンダーターゲットとなるテクスチャ作成 */
	glGenTextures(
		/* GLsizei n */				1,
		/* GLuint * textures */		&offscreenRenderTargetTexture
	);
	glBindTexture(
		/* GLenum target */			GL_TEXTURE_2D,
		/* GLuint texture */		offscreenRenderTargetTexture
	);
	glExtTexStorage2D(
		/* GLenum target */			GL_TEXTURE_2D,
		/* GLsizei levels */		1,
		/* GLenum internalformat */	GL_RGBA8,
		/* GLsizei width */			captureSettings->xReso,
		/* GLsizei height */		captureSettings->yReso
	);

	/* レンダーターゲットのバインド */
	glExtFramebufferTexture(
		/* GLenum target */			GL_FRAMEBUFFER,
		/* GLenum attachment */		GL_COLOR_ATTACHMENT0,
		/* GLuint texture */		offscreenRenderTargetTexture,
		/* GLint level */			0
	);

	/* FBO 設定、ビューポート設定 */
	glExtBindFramebuffer(
		/* GLenum target */			GL_FRAMEBUFFER,
		/* GLuint framebuffer */	offscreenRenderTargetFbo
	);

	/* 画面全体に四角形を描画 */
	float tanFovY = tanf(fovYInRadians);
	GraphicsDrawFullScreenQuad(
		offscreenRenderTargetFbo,
		waveOutPos,
		frameCount,
		time,
		0,		/* キャプチャ時はマウス座標は常に 0 */
		0,		/* キャプチャ時はマウス座標は常に 0 */
		0,		/* キャプチャ時はマウスボタンは常に 0 */
		0,		/* キャプチャ時はマウスボタンは常に 0 */
		0,		/* キャプチャ時はマウスボタンは常に 0 */
		captureSettings->xReso,
		captureSettings->yReso,
		tanFovY,
		mat4x4CameraInWorld,
		renderSettings
	);

	/* 描画結果の取得 */
	glFinish();		/* 不要と信じたいが念のため */
	glExtBindFramebuffer(
		/* GLenum target */			GL_FRAMEBUFFER,
		/* GLuint framebuffer */	offscreenRenderTargetFbo
	);
	glReadPixels(
		/* GLint x */				0,
		/* GLint y */				0,
		/* GLsizei width */			captureSettings->xReso,
		/* GLsizei height */		captureSettings->yReso,
		/* GLenum format */			GL_RGBA,
		/* GLenum type */			GL_UNSIGNED_BYTE,
		/* GLvoid * data */			buffer
	);

	/* αチャンネルの強制 1.0 置換 */
	if (captureSettings->replaceAlphaByOne) {
		for (int y = 0; y < captureSettings->yReso; y++) {
			for (int x = 0; x < captureSettings->xReso; x++) {
				((uint8_t *)buffer)[(y * captureSettings->xReso + x) * 4 + 3] = 255;
			}
		}
	}

	/* オフスクリーンレンダーターゲット、FBO 破棄 */
	glDeleteTextures(
		/* GLsizei n */						1,
		/* const GLuint * textures */		&offscreenRenderTargetTexture
	);
	glExtDeleteFramebuffers(
		/* GLsizei n */						1,
		/* const GLuint * framebuffers */	&offscreenRenderTargetFbo
	);

	return true;
}

bool GraphicsCaptureScreenShotAsUnorm8RgbaImage(
	int waveOutPos,
	int frameCount,
	float time,
	float fovYInRadians,
	const float mat4x4CameraInWorld[4][4],
	const RenderSettings *renderSettings,
	const CaptureScreenShotSettings *captureSettings
){
	size_t bytesPerPixel = 4;
	size_t bufferSizeInBytes = (size_t)(captureSettings->xReso * captureSettings->yReso) * bytesPerPixel;
	void *buffer = malloc(bufferSizeInBytes);
	if (
		GraphicsCaptureScreenShotAsUnorm8RgbaImageMemory(
			buffer, bufferSizeInBytes, waveOutPos, frameCount, time,
			fovYInRadians, mat4x4CameraInWorld,
			renderSettings, captureSettings
		) == false
	) {
		free(buffer);
		return false;
	}
	if (
		SerializeAsUnorm8RgbaPng(
			captureSettings->fileName, buffer, captureSettings->xReso, captureSettings->yReso
		) == false
	) {
		free(buffer);
		return false;
	};
	free(buffer);
	return true;
}

bool GraphicsCaptureCubemap(
	int waveOutPos,
	int frameCount,
	float time,
	const float mat4x4CameraInWorld[4][4],
	const RenderSettings *renderSettings,
	const CaptureCubemapSettings *captureSettings
){
	/* 先だって全レンダーターゲットのクリア */
	GraphicsClearAllRenderTargets();

	/* FBO 作成 */
	GLuint offscreenRenderTargetFbo = 0;
	GLuint offscreenRenderTargetTexture = 0;
	glExtGenFramebuffers(
		/* GLsizei n */				1,
	 	/* GLuint *ids */			&offscreenRenderTargetFbo
	);
	glExtBindFramebuffer(
		/* GLenum target */			GL_FRAMEBUFFER,
		/* GLuint framebuffer */	offscreenRenderTargetFbo
	);

	/* レンダーターゲットとなるテクスチャ作成 */
	glGenTextures(
		/* GLsizei n */				1,
		/* GLuint * textures */		&offscreenRenderTargetTexture
	);
	glBindTexture(
		/* GLenum target */			GL_TEXTURE_2D,
		/* GLuint texture */		offscreenRenderTargetTexture
	);
	glExtTexStorage2D(
		/* GLenum target */			GL_TEXTURE_2D,
		/* GLsizei levels */		1,
		/* GLenum internalformat */	GL_RGBA32F,
		/* GLsizei width */			captureSettings->reso,
		/* GLsizei height */		captureSettings->reso
	);

	/* レンダーターゲットのバインド */
	glExtFramebufferTexture(
		/* GLenum target */			GL_FRAMEBUFFER,
		/* GLenum attachment */		GL_COLOR_ATTACHMENT0,
		/* GLuint texture */		offscreenRenderTargetTexture,
		/* GLint level */			0
	);

	/* キューブマップ各面の描画と結果の取得 */
	float *(data[6]);
	for (int iFace = 0; iFace < 6; iFace++) {
		/*
			dds の cubemap face の配置

		                                 [5]
		                           +-------------+
		                          /             /|
		                         /     [2]     / |
		       [+y]             /             /  |  face0:+x:right
		        | /[  ]        +-------------+   |  face1:-x:left
		        |/             |             |   |  face2:+y:top
		[  ]----+----[+x]   [1]|             |[0]|  face3:-y:bottom
		       /|              |             |   |  face4:+z:front
		  [+z]/ |              |     [4]     |   +  face5:-z:back
		       [  ]            |             |  /
		                       |             | /
		                       |             |/
		                       +-------------+
		                             [3]

		                  +-----------------+
		                  |       [-z]      |
		                  |        | /[  ]  |
		                  |        |/       |
		                  |[  ]----+----[+x]|
		                  |       /|        |
		                  |  [+y]/ |        |
		                  |       [  ]      |
		                  |                 |
		                  | face2:+y:top    |
		+-----------------+-----------------+-----------------+-----------------+
		|       [+y]      |       [+y]      |       [+y]      |       [+y]      |
		|        | /[  ]  |        | /[  ]  |        | /[  ]  |        | /[  ]  |
		|        |/       |        |/       |        |/       |        |/       |
		|[  ]----+----[+z]|[  ]----+----[+x]|[  ]----+----[-z]|[  ]----+----[-x]|
		|       /|        |       /|        |       /|        |       /|        |
		|  [-x]/ |        |  [+z]/ |        |  [+x]/ |        |  [-z]/ |        |
		|       [  ]      |       [  ]      |       [  ]      |       [  ]      |
		|                 |                 |                 |                 |
		| face1:-x:left   | face4:+z:front  | face0:+x:right  | face5:-z:back   |
		+-----------------+-----------------+-----------------+-----------------+
		                  |       [+z]      |
		                  |        | /[  ]  |
		                  |        |/       |
		                  |[  ]----+----[+x]|
		                  |       /|        |
		                  |  [-y]/ |        |
		                  |       [  ]      |
		                  |                 |
		                  | face3:-y:bottom |
		                  +-----------------+
		*/
		const float mat4x4FaceInWorldTbl[6][4][4] = {
			/*
				+-----------------+
				|       [+y]      |
				|        | /[  ]  |
				|        |/       |
				|[  ]----+----[-z]|
				|       /|        |
				|  [+x]/ |        |
				|       [  ]      |
				|                 |
				| face0:+x:right  |
				+-----------------+
			*/
			{
				{ 0, 0,-1, 0},
				{ 0, 1, 0, 0},
				{-1, 0, 0, 0},		/* 裏表反転のため Z 軸の符号を反転 */
				{ 0, 0, 0, 1}
			},
			/*
				+-----------------+
				|       [+y]      |
				|        | /[  ]  |
				|        |/       |
				|[  ]----+----[+z]|
				|       /|        |
				|  [-x]/ |        |
				|       [  ]      |
				|                 |
				| face1:-x:left   |
				+-----------------+
			*/
			{
				{ 0, 0, 1, 0},
				{ 0, 1, 0, 0},
				{ 1, 0, 0, 0},		/* 裏表反転のため Z 軸の符号を反転 */
				{ 0, 0, 0, 1}
			},
			/*
				+-----------------+
				|       [-z]      |
				|        | /[  ]  |
				|        |/       |
				|[  ]----+----[+x]|
				|       /|        |
				|  [+y]/ |        |
				|       [  ]      |
				|                 |
				| face2:+y:top    |
				+-----------------+
			*/
			{
				{ 1, 0, 0, 0},
				{ 0, 0,-1, 0},
				{ 0,-1, 0, 0},		/* 裏表反転のため Z 軸の符号を反転 */
				{ 0, 0, 0, 1}
			},
			/*
				+-----------------+
				|       [+z]      |
				|        | /[  ]  |
				|        |/       |
				|[  ]----+----[+x]|
				|       /|        |
				|  [-y]/ |        |
				|       [  ]      |
				|                 |
				| face3:-y:bottom |
				+-----------------+
			*/
			{
				{ 1, 0, 0, 0},
				{ 0, 0, 1, 0},
				{ 0, 1, 0, 0},		/* 裏表反転のため Z 軸の符号を反転 */
				{ 0, 0, 0, 1}
			},
			/*
				+-----------------+
				|       [+y]      |
				|        | /[  ]  |
				|        |/       |
				|[  ]----+----[+x]|
				|       /|        |
				|  [+z]/ |        |
				|       [  ]      |
				|                 |
				| face4:+z:front  |
				+-----------------+
			*/
			{
				{ 1, 0, 0, 0},
				{ 0, 1, 0, 0},
				{ 0, 0,-1, 0},		/* 裏表反転のため Z 軸の符号を反転 */
				{ 0, 0, 0, 1}
			},
			/*
				+-----------------+
				|       [+y]      |
				|        | /[  ]  |
				|        |/       |
				|[  ]----+----[-x]|
				|       /|        |
				|  [-z]/ |        |
				|       [  ]      |
				|                 |
				| face5:-z:back   |
				+-----------------+
			*/
			{
				{-1, 0, 0, 0},
				{ 0, 1, 0, 0},
				{ 0, 0, 1, 0},		/* 裏表反転のため Z 軸の符号を反転 */
				{ 0, 0, 0, 1}
			}
		};

		/* キューブマップの指定の面の方向を向き、カメラ位置を原点とする座標系 */
		float mat4x4FaceInWorld[4][4];
		memcpy(
			mat4x4FaceInWorld,
			mat4x4FaceInWorldTbl[iFace],
			sizeof(mat4x4FaceInWorld)
		);
		memcpy(
			mat4x4FaceInWorld[3],
			mat4x4CameraInWorld[3],
			sizeof(mat4x4FaceInWorld[3])
		);

		/* 画面全体に四角形を描画 */
		float tanFovY = 1.0f;	/* 垂直方向画角90度 */
		GraphicsDrawFullScreenQuad(
			offscreenRenderTargetFbo,
			waveOutPos,
			frameCount,
			time,
			0,		/* キューブマップキャプチャ時はマウス座標は常に 0 */
			0,		/* キューブマップキャプチャ時はマウス座標は常に 0 */
			0,		/* キューブマップキャプチャ時はマウスボタンは常に 0 */
			0,		/* キューブマップキャプチャ時はマウスボタンは常に 0 */
			0,		/* キューブマップキャプチャ時はマウスボタンは常に 0 */
			captureSettings->reso,
			captureSettings->reso,
			tanFovY,
			mat4x4FaceInWorld,
			renderSettings
		);

		/* 描画結果の取得 */
		data[iFace] = (float *)malloc(sizeof(float) * 4 * captureSettings->reso * captureSettings->reso);
		glFinish();		/* 不要と信じたいが念のため */
		glExtBindFramebuffer(
			/* GLenum target */			GL_FRAMEBUFFER,
			/* GLuint framebuffer */	offscreenRenderTargetFbo
		);
		glReadPixels(
			/* GLint x */				0,
			/* GLint y */				0,
			/* GLsizei width */			captureSettings->reso,
			/* GLsizei height */		captureSettings->reso,
			/* GLenum format */			GL_RGBA,
			/* GLenum type */			GL_FLOAT,
			/* GLvoid * data */			data[iFace]
		);

		/* 上下反転 */
		for (int y = 0; y < captureSettings->reso/2; y++) {
			for (int x = 0; x < captureSettings->reso; x++) {
				for (int i = 0; i < 4; i++) {
					float tmp = data[iFace][(y * captureSettings->reso + x) * 4 + i];
					data[iFace][(y * captureSettings->reso + x) * 4 + i]
					=	data[iFace][((captureSettings->reso - y - 1) * captureSettings->reso + x) * 4 + i];
					data[iFace][((captureSettings->reso - y - 1) * captureSettings->reso + x) * 4 + i] = tmp;
				}
			}
		}
	}

	/* ファイルに書き出し */
	bool ret = SerializeAsFp32RgbaCubemapDds(
		/* const char *fileName */	captureSettings->fileName,
		/* float *(data[6]) */		data,
		/* int width */				captureSettings->reso,
		/* int height */			captureSettings->reso
	);

	/* メモリ破棄 */
	for (int iFace = 0; iFace < 6; iFace++) {
		free(data[iFace]);
	}

	/* オフスクリーンレンダーターゲット、FBO 破棄 */
	glDeleteTextures(
		/* GLsizei n */						1,
		/* const GLuint * textures */		&offscreenRenderTargetTexture
	);
	glExtDeleteFramebuffers(
		/* GLsizei n */						1,
		/* const GLuint * framebuffers */	&offscreenRenderTargetFbo
	);

	return ret;
}

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
	float fovYInRadians,
	const float mat4x4CameraInWorld[4][4],
	const RenderSettings *settings
){
	/* 画面全体に四角形を描画 */
	float tanFovY = tanf(fovYInRadians);
	GraphicsDrawFullScreenQuad(
		0,		/* デフォルトフレームバッファに出力 */
		waveOutPos,
		frameCount,
		time,
		xMouse,
		yMouse,
		mouseLButtonPressed,
		mouseMButtonPressed,
		mouseRButtonPressed,
		xReso,
		yReso,
		tanFovY,
		mat4x4CameraInWorld,
		settings
	);

	/* スワップ設定 */
	if (settings->enableSwapIntervalControl) {
		switch (settings->swapInterval) {
			case SwapIntervalAllowTearing: {
				wglSwapIntervalEXT(-1);
			} break;
			case SwapIntervalHsync: {
				wglSwapIntervalEXT(0);
			} break;
			case SwapIntervalVsync: {
				wglSwapIntervalEXT(1);
			} break;
			default: {
				assert(false);
			} break;
		}
	}
}

bool GraphicsInitialize(
){
	GraphicsCreateFrameBuffer(s_xReso, s_yReso, &s_currentRenderSettings);
	return true;
}

bool GraphicsTerminate(
){
	GraphicsDeleteShader(); /* false が得られてもエラー扱いとしない */
	return true;
}

