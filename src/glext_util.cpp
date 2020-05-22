/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#include <stdio.h>
#include <stdlib.h>
#include "glext_util.h"


static const char s_margedFunctionName[] =
	"glCreateShaderProgramv\0"
	"glCreateProgram\0"
	"glDeleteProgram\0"
	"glCreateShader\0"
	"glDeleteShader\0"
	"glShaderSource\0"
	"glCompileShader\0"
	"glAttachShader\0"
	"glDetachShader\0"
	"glLinkProgram\0"
	"glUseProgram\0"
	"glUniform1f\0"
	"glUniform1i\0"
	"glUniform3i\0"
	"glUniform2f\0"
	"glUniformMatrix4fv\0"
	"glGetUniformLocation\0"
	"glActiveTexture\0"
	"glDispatchCompute\0"
	"glGenBuffers\0"
	"glDeleteBuffers\0"
	"glBindBuffer\0"
	"glBindBufferBase\0"
	"glBufferData\0"
	"glBufferSubData\0"
	"glGetBufferSubData\0"
	"glMapBuffer\0"
	"glUnmapBuffer\0"
	"glTransformFeedbackVaryings\0"
	"glBeginTransformFeedback\0"
	"glEndTransformFeedback\0"
	"glGetShaderiv\0"
	"glGetProgramiv\0"
	"glGetShaderInfoLog\0"
	"glGetProgramInfoLog\0"
	"glGenFramebuffers\0"
	"glDeleteFramebuffers\0"
	"glBindFramebuffer\0"
	"glDrawBuffers\0"
	"glFramebufferTexture\0"
	"glFramebufferTexture2D\0"
	"glTexStorage2D\0"
	"glBlitNamedFramebuffer\0"
	"glFenceSync\0"
	"glWaitSync\0"
	"glGenerateMipmap\0"
	"glGetProgramInterfaceiv\0"
	"glGetProgramResourceiv\0"
	"glGetProgramResourceName\0"
	"wglSwapIntervalEXT\0"
;
void *g_glExtFunctions[NUM_GLEXT_FUNCTIONS];


bool OpenGlExtInitialize(
){
	const char *p = &s_margedFunctionName[0];
	for (int i = 0; i < NUM_GLEXT_FUNCTIONS; ++i) {
		printf("wglGetProcAddress %s ... ", p);
		g_glExtFunctions[i] = wglGetProcAddress(p);
		if (g_glExtFunctions[i] == NULL) {
			printf("failed.\n");
			return false;
		}
		printf("OK.\n");
		do {
			++p;
		} while (*p != '\0');
		++p;
	}
	return true;
}

bool OpenGlExtTerminate(
){
	return true;
}

