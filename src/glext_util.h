/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#ifndef _GLEXT_UTIL_H_
#define _GLEXT_UTIL_H_


#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <GL/gl.h>
#include <GL/glext.h>


/* OpenGL 拡張関数の種類 */
typedef enum {
	GlExtCreateShaderProgramv = 0,
	GlExtCreateProgram,
	GlExtDeleteProgram,
	GlExtCreateShader,
	GlExtDeleteShader,
	GlExtShaderSource,
	GlExtCompileShader,
	GlExtAttachShader,
	GlExtDetachShader,
	GlExtLinkProgram,
	GlExtUseProgram,
	GlExtUniform1f,
	GlExtUniform1i,
	GlExtUniform3i,
	GlExtUniform2f,
	GlExtUniformMatrix4fv,
	GlExtGetUniformLocation,
	GlExtActiveTexture,
	GlExtDispatchCompute,
	GlExtGenBuffers,
	GlExtDeleteBuffers,
	GlExtBindBuffer,
	GlExtBindBufferBase,
	GlExtBufferData,
	GlExtBufferSubData,
	GlExtGetBufferSubData,
	GlExtGetNamedBufferSubData,
	GlExtCopyNamedBufferSubData,
	GlExtMapBuffer,
	GlExtMapNamedBuffer,
	GlExtUnmapBuffer,
	GlExtUnmapNamedBuffer,
	GlExtTransformFeedbackVaryings,
	GlExtBeginTransformFeedback,
	GlExtEndTransformFeedback,
	GlExtGetShaderiv,
	GlExtGetProgramiv,
	GlExtGetShaderInfoLog,
	GlExtGetProgramInfoLog,
	GlExtGenFramebuffers,
	GlExtDeleteFramebuffers,
	GlExtBindFramebuffer,
	GlExtDrawBuffers,
	GlExtFramebufferTexture,
	GlExtFramebufferTexture2D,
	GlExtTexStorage2D,
	GlExtBlitNamedFramebuffer,
	GlExtFenceSync,
	GlExtWaitSync,
	GlExtGenerateMipmap,
	GlExtGetProgramInterfaceiv,
	GlExtGetProgramResourceiv,
	GlExtGetProgramResourceName,
	GlExtCompressedTexImage2D,
	WglSwapIntervalEXT,
	NUM_GLEXT_FUNCTIONS
} GlExt;

/* OpenGL 拡張関数 */
extern void *g_glExtFunctions[NUM_GLEXT_FUNCTIONS];
#define glExtCreateShaderProgramv		((PFNGLCREATESHADERPROGRAMVPROC)g_glExtFunctions[GlExtCreateShaderProgramv])
#define glExtCreateProgram				((PFNGLCREATEPROGRAMPROC)g_glExtFunctions[GlExtCreateProgram])
#define glExtDeleteProgram				((PFNGLDELETEPROGRAMPROC)g_glExtFunctions[GlExtDeleteProgram])
#define glExtCreateShader				((PFNGLCREATESHADERPROC)g_glExtFunctions[GlExtCreateShader])
#define glExtDeleteShader				((PFNGLDELETESHADERPROC)g_glExtFunctions[GlExtDeleteShader])
#define glExtShaderSource				((PFNGLSHADERSOURCEPROC)g_glExtFunctions[GlExtShaderSource])
#define glExtCompileShader				((PFNGLCOMPILESHADERPROC)g_glExtFunctions[GlExtCompileShader])
#define glExtAttachShader				((PFNGLATTACHSHADERPROC)g_glExtFunctions[GlExtAttachShader])
#define glExtDetachShader				((PFNGLDETACHSHADERPROC)g_glExtFunctions[GlExtDetachShader])
#define glExtLinkProgram				((PFNGLLINKPROGRAMPROC)g_glExtFunctions[GlExtLinkProgram])
#define glExtUseProgram					((PFNGLUSEPROGRAMPROC)g_glExtFunctions[GlExtUseProgram])
#define glExtUniform1f					((PFNGLUNIFORM1FPROC)g_glExtFunctions[GlExtUniform1f])
#define glExtUniform1i					((PFNGLUNIFORM1IPROC)g_glExtFunctions[GlExtUniform1i])
#define glExtUniform3i					((PFNGLUNIFORM3IPROC)g_glExtFunctions[GlExtUniform3i])
#define glExtUniform2f					((PFNGLUNIFORM2FPROC)g_glExtFunctions[GlExtUniform2f])
#define glExtUniformMatrix4fv			((PFNGLUNIFORMMATRIX4FVPROC)g_glExtFunctions[GlExtUniformMatrix4fv])
#define glExtGetUniformLocation			((PFNGLGETUNIFORMLOCATIONPROC)g_glExtFunctions[GlExtGetUniformLocation])
#define glExtActiveTexture				((PFNGLACTIVETEXTUREPROC)g_glExtFunctions[GlExtActiveTexture])
#define glExtDispatchCompute			((PFNGLDISPATCHCOMPUTEPROC)g_glExtFunctions[GlExtDispatchCompute])
#define glExtGenBuffers					((PFNGLGENBUFFERSPROC)g_glExtFunctions[GlExtGenBuffers])
#define glExtDeleteBuffers				((PFNGLDELETEBUFFERSPROC)g_glExtFunctions[GlExtDeleteBuffers])
#define glExtBindBuffer					((PFNGLBINDBUFFERPROC)g_glExtFunctions[GlExtBindBuffer])
#define glExtBindBufferBase				((PFNGLBINDBUFFERBASEPROC)g_glExtFunctions[GlExtBindBufferBase])
#define glExtBufferData					((PFNGLBUFFERDATAPROC)g_glExtFunctions[GlExtBufferData])
#define glExtBufferSubData				((PFNGLBUFFERSUBDATAPROC)g_glExtFunctions[GlExtBufferSubData])
#define glExtGetBufferSubData			((PFNGLGETBUFFERSUBDATAPROC)g_glExtFunctions[GlExtGetBufferSubData])
#define glExtGetNamedBufferSubData		((PFNGLGETNAMEDBUFFERSUBDATAPROC)g_glExtFunctions[GlExtGetNamedBufferSubData])
#define glExtCopyNamedBufferSubData		((PFNGLCOPYNAMEDBUFFERSUBDATAPROC)g_glExtFunctions[GlExtCopyNamedBufferSubData])
#define glExtMapBuffer					((PFNGLMAPBUFFERPROC)g_glExtFunctions[GlExtMapBuffer])
#define glExtMapNamedBuffer				((PFNGLMAPNAMEDBUFFERPROC)g_glExtFunctions[GlExtMapNamedBuffer])
#define glExtUnmapBuffer				((PFNGLUNMAPBUFFERPROC)g_glExtFunctions[GlExtUnmapBuffer])
#define glExtUnmapNamedBuffer			((PFNGLUNMAPNAMEDBUFFERPROC)g_glExtFunctions[GlExtUnmapNamedBuffer])
#define glExtTransformFeedbackVaryings	((PFNGLTRANSFORMFEEDBACKVARYINGSPROC)g_glExtFunctions[GlExtTransformFeedbackVaryings])
#define glExtBeginTransformFeedback		((PFNGLBEGINTRANSFORMFEEDBACKPROC)g_glExtFunctions[GlExtBeginTransformFeedback])
#define glExtEndTransformFeedback		((PFNGLENDTRANSFORMFEEDBACKPROC)g_glExtFunctions[GlExtEndTransformFeedback])
#define glExtGetShaderiv				((PFNGLGETSHADERIVPROC)g_glExtFunctions[GlExtGetShaderiv])
#define glExtGetProgramiv				((PFNGLGETPROGRAMIVPROC)g_glExtFunctions[GlExtGetProgramiv])
#define glExtGetShaderInfoLog			((PFNGLGETSHADERINFOLOGPROC)g_glExtFunctions[GlExtGetShaderInfoLog])
#define glExtGetProgramInfoLog			((PFNGLGETPROGRAMINFOLOGPROC)g_glExtFunctions[GlExtGetProgramInfoLog])
#define glExtGenFramebuffers			((PFNGLGENFRAMEBUFFERSPROC)g_glExtFunctions[GlExtGenFramebuffers])
#define glExtDeleteFramebuffers			((PFNGLDELETEFRAMEBUFFERSPROC)g_glExtFunctions[GlExtDeleteFramebuffers])
#define glExtBindFramebuffer			((PFNGLBINDFRAMEBUFFERPROC)g_glExtFunctions[GlExtBindFramebuffer])
#define glExtDrawBuffers				((PFNGLDRAWBUFFERSPROC)g_glExtFunctions[GlExtDrawBuffers])
#define glExtFramebufferTexture			((PFNGLFRAMEBUFFERTEXTUREPROC)g_glExtFunctions[GlExtFramebufferTexture])
#define glExtFramebufferTexture2D		((PFNGLFRAMEBUFFERTEXTURE2DPROC)g_glExtFunctions[GlExtFramebufferTexture2D])
#define glExtTexStorage2D				((PFNGLTEXSTORAGE2DPROC)g_glExtFunctions[GlExtTexStorage2D])
#define glExtBlitNamedFramebuffer		((PFNGLBLITNAMEDFRAMEBUFFERPROC)g_glExtFunctions[GlExtBlitNamedFramebuffer])
#define glExtFenceSync					((PFNGLFENCESYNCPROC)g_glExtFunctions[GlExtFenceSync])
#define glExtWaitSync					((PFNGLWAITSYNCPROC)g_glExtFunctions[GlExtWaitSync])
#define glExtGenerateMipmap				((PFNGLGENERATEMIPMAPPROC)g_glExtFunctions[GlExtGenerateMipmap])
#define glExtGetProgramInterfaceiv		((PFNGLGETPROGRAMINTERFACEIVPROC)g_glExtFunctions[GlExtGetProgramInterfaceiv])
#define glExtGetProgramResourceiv		((PFNGLGETPROGRAMRESOURCEIVPROC)g_glExtFunctions[GlExtGetProgramResourceiv])
#define glExtGetProgramResourceName		((PFNGLGETPROGRAMRESOURCENAMEPROC)g_glExtFunctions[GlExtGetProgramResourceName])
#define glExtCompressedTexImage2D		((PFNGLCOMPRESSEDTEXIMAGE2DPROC)g_glExtFunctions[GlExtCompressedTexImage2D])
#define wglSwapIntervalEXT				((BOOL(WINAPI*)(int))g_glExtFunctions[WglSwapIntervalEXT])


/* OpenGL 拡張初期化 */
bool OpenGlExtInitialize();

/* OpenGL 拡張終了処理 */
bool OpenGlExtTerminate();


#endif
