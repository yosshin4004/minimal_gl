#version 430	/* version ディレクティブが必要な場合は必ず 1 行目に書くこと */
/* Copyright (C) 2020 Yosshin(@yosshin4004) */

/*
	バックバッファ利用サンプルコード。

	バックバッファを利用する場合は、Render Settings の Enable back buffer の
	チェックボックスが有効である必要がある（デフォルトで有効）。
*/

layout(binding = 0) uniform sampler2D backBuffer;
layout(location = 0) uniform int waveOutPosition;
#if defined(EXPORT_EXECUTABLE)
	vec2 resolution = {SCREEN_XRESO, SCREEN_YRESO};
	#define NUM_SAMPLES_PER_SEC 48000.
	float time = waveOutPosition / NUM_SAMPLES_PER_SEC;
#else
	layout(location = 2) uniform float time;
	layout(location = 3) uniform vec2 resolution;
#endif

out vec4 outColor;

void main(){
	vec2 texCoord = gl_FragCoord.xy / resolution;
	vec2 position = (gl_FragCoord.xy - resolution / 2) / resolution.y;
	position += vec2(sin(time * 5), cos(time * 7)) * .4;
	vec3 color = vec3(.005) / length(position);
	vec3 colorBack = texture(backBuffer, texCoord).rgb;
	outColor = vec4(color + colorBack * .95, 1);
}

