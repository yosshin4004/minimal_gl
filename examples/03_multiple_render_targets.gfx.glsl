#version 430	/* version ディレクティブが必要な場合は必ず 1 行目に書くこと */
/* Copyright (C) 2020 Yosshin(@yosshin4004) */

/*
	MRT4 バックバッファ利用サンプル。

	MRT4 バックバッファを利用する場合は、Render Settings の Enable back buffer
	のチェックボックスを有効にした上で、Frame buffer に MRT4 を選択する必要が
	ある（デフォルトでこの状態になっている）。
*/

layout(binding = 0) uniform sampler2D backBuffer0;
layout(binding = 1) uniform sampler2D backBuffer1;
layout(binding = 2) uniform sampler2D backBuffer2;
layout(binding = 3) uniform sampler2D backBuffer3;
layout(location = 0) uniform int waveOutPosition;
#if defined(EXPORT_EXECUTABLE)
	vec2 resolution = {SCREEN_XRESO, SCREEN_YRESO};
	#define NUM_SAMPLES_PER_SEC 48000.
	float time = waveOutPosition / NUM_SAMPLES_PER_SEC;
#else
	layout(location = 2) uniform float time;
	layout(location = 3) uniform vec2 resolution;
#endif

layout(location = 0) out vec4 outColor0;
layout(location = 1) out vec4 outColor1;
layout(location = 2) out vec4 outColor2;
layout(location = 3) out vec4 outColor3;

void main(){
	float t = time;

	vec2 uv = gl_FragCoord.xy / resolution;

	vec2 pos = uv - .5;
	pos += vec2(sin(t*3), cos(t*3)) * .5;

	vec4 backColor0 = textureLod(backBuffer0, uv, 6);
	vec4 backColor1 = textureLod(backBuffer1, uv, 4);
	vec4 backColor2 = textureLod(backBuffer2, uv, 2);
	vec4 backColor3 = textureLod(backBuffer3, uv, 0);

	vec4 backColor = backColor0;
	if (uv.x > 1./4) backColor = backColor1;
	if (uv.x > 2./4) backColor = backColor2;
	if (uv.x > 3./4) backColor = backColor3;

	vec4 outColor = vec4(
		vec3(4) * .0001 / (abs(sin(pos.x) * sin(pos.y)) +.0001)
	+	backColor.rgb * .9,
		1
	);

	outColor0 = outColor;
	outColor1 = outColor * vec4(0.9, 1.0, 1.0, 1.0);
	outColor2 = outColor * vec4(1.0, 0.9, 1.0, 1.0);
	outColor3 = outColor * vec4(1.0, 1.0, 0.9, 1.0);
}

