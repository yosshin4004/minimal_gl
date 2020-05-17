/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#include "stdint.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif


/*=============================================================================
▼	各種リソース
-----------------------------------------------------------------------------*/

/* 結合された文字列 */
/*
	crinkler に /OVERRIDEALIGNMENTS オプションを指定すると、末尾に _align? が
	付いたシンボルは、? で指定したアライメントで配置される。
	文字列情報はアライメントを問わないので _align0（= 1 byte アライン）を指定
	すると良い。
*/
#pragma data_seg("s_concatenatedString")
char g_concatenatedString_align0[] =
	/* GL 拡張関数名テーブル（なるべく似た関数名が連続するように配置）*/
#if ALLOW_TEARING_FLIP
	"wglSwapIntervalEXT\0"			/* 頻出ワードを持たないので先に配置 */
#endif
	"glDispatchCompute\0"			/* 頻出ワードを持たないので先に配置 */
	"glUniform1i\0"					/* 頻出ワードを持たないので先に配置 */
#if ENABLE_MIPMAP_GENERATION
	"glGenerateMipmap\0"			/* 既出ワードを持たないので先に配置 */
#endif
	"glCreateShaderProgramv\0"		/* Generate の ate が 既出ワード */
	"glUseProgram\0"				/* Program が既出ワード */
	"glBufferData\0"
	"glMapBuffer\0"					/* Buffer が既出ワード */
	"glBindBufferBase\0"			/* Buffer が既出ワード */
#if ENABLE_BACK_BUFFER && ((NUM_RENDER_TARGETS > 1) || (PIXEL_FORMAT != PIXEL_FORMAT_UNORM8_RGBA))
	"glBindFramebuffer\0"			/* Bind と Buffer の uffer が既出ワード */
	"glGenFramebuffers\0"			/* Framebuffer が既出ワード */
	"glBlitNamedFramebuffer\0"		/* Framebuffer が既出ワード */
	"glFramebufferTexture\0"		/* Framebuffer が既出ワード */
	#if (NUM_RENDER_TARGETS > 1)
	"glActiveTexture\0"				/* Texture が既出ワード */
	#endif
	#if PREFER_GL_TEX_STORAGE_2D
	"glTexStorage2D\0"				/* Tex が既出ワード */
	#endif
	"glDrawBuffers\0"				/* Buffer が既出ワード（ちょっと遠いけど）*/
#endif
	"\xFF"			/* end mark */

	/* 描画用シェーダ */
	#include "graphics_fragment_shader.inl"
	"\0"			/* end mark */

	/* サウンド用シェーダ */
	#include "sound_compute_shader.inl"
;

/* GL 拡張関数テーブルのインデクス */
typedef enum {
#if ALLOW_TEARING_FLIP
	kWglSwapIntervalEXT,
#endif
	GlExtDispatchCompute,
	GlExtUniform1i,
#if ENABLE_MIPMAP_GENERATION
	GlExtGenerateMipmap,
#endif
	GlExtCreateShaderProgramv,
	GlExtUseProgram,
	GlExtBufferData,
	GlExtMapBuffer,
	GlExtBindBufferBase,
#if ENABLE_BACK_BUFFER && ((NUM_RENDER_TARGETS > 1) || (PIXEL_FORMAT != PIXEL_FORMAT_UNORM8_RGBA))
	GlExtBindFramebuffer,
	GlExtGenFramebuffers,
	GlExtBlitNamedFramebuffer,
	GlExtFramebufferTexture,
	#if (NUM_RENDER_TARGETS > 1)
	GlExtActiveTexture,
	#endif
	#if PREFER_GL_TEX_STORAGE_2D
	GlExtTexStorage2D,
	#endif
	GlExtDrawBuffers,
#endif
	NUM_GLEXT_FUNCTIONS
} GlExt;

/* GL 拡張関数 */
#define wglSwapIntervalEXT			((BOOL(WINAPI*)(int))           s_glExtFunctions[kWglSwapIntervalEXT])
#define glExtDispatchCompute		((PFNGLDISPATCHCOMPUTEPROC)     s_glExtFunctions[GlExtDispatchCompute])
#define glExtUniform1i				((PFNGLUNIFORM1IPROC)           s_glExtFunctions[GlExtUniform1i])
#define glExtGenerateMipmap			((PFNGLGENERATEMIPMAPPROC)      s_glExtFunctions[GlExtGenerateMipmap])
#define glExtCreateShaderProgramv	((PFNGLCREATESHADERPROGRAMVPROC)s_glExtFunctions[GlExtCreateShaderProgramv])
#define glExtUseProgram				((PFNGLUSEPROGRAMPROC)          s_glExtFunctions[GlExtUseProgram])
#define glExtBufferData				((PFNGLBUFFERDATAPROC)          s_glExtFunctions[GlExtBufferData])
#define glExtMapBuffer				((PFNGLMAPBUFFERPROC)	        s_glExtFunctions[GlExtMapBuffer])
#define glExtBindBufferBase			((PFNGLBINDBUFFERBASEPROC)      s_glExtFunctions[GlExtBindBufferBase])
#define glExtBindFramebuffer		((PFNGLBINDFRAMEBUFFERPROC)     s_glExtFunctions[GlExtBindFramebuffer])
#define glExtGenFramebuffers		((PFNGLGENFRAMEBUFFERSPROC)     s_glExtFunctions[GlExtGenFramebuffers])
#define glExtBlitNamedFramebuffer	((PFNGLBLITNAMEDFRAMEBUFFERPROC)s_glExtFunctions[GlExtBlitNamedFramebuffer])
#define glExtFramebufferTexture		((PFNGLFRAMEBUFFERTEXTUREPROC)  s_glExtFunctions[GlExtFramebufferTexture])
#define glExtActiveTexture			((PFNGLACTIVETEXTUREPROC)       s_glExtFunctions[GlExtActiveTexture])
#define glExtTexStorage2D			((PFNGLTEXSTORAGE2DPROC)        s_glExtFunctions[GlExtTexStorage2D])
#define glExtDrawBuffers			((PFNGLDRAWBUFFERSPROC)         s_glExtFunctions[GlExtDrawBuffers])
#define glExtUniform2f				((PFNGLUNIFORM2FPROC)           s_glExtFunctions[GlExtUniform2f])
#define glExtGetUniformLocation		((PFNGLGETUNIFORMLOCATIONPROC)  s_glExtFunctions[GlExtGetUniformLocation])
#define glExtGetProgramiv			((PFNGLGETPROGRAMIVPROC)        s_glExtFunctions[GlExtGetProgramiv])
#define glExtGetProgramInfoLog		((PFNGLGETPROGRAMINFOLOGPROC)   s_glExtFunctions[GlExtGetProgramInfoLog])



#ifdef __cplusplus
}
#endif
