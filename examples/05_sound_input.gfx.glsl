#version 430	/* version ディレクティブが必要な場合は必ず 1 行目に書くこと */
/* Copyright (C) 2020 Yosshin(@yosshin4004) */

/*
	サウンド入力サンプルコード。

	04_sound_output.snd.glsl と組み合わせることでサウンドが可視化される。
*/

layout(location = 0) uniform int waveOutPosition;
#if defined(EXPORT_EXECUTABLE)
	vec2 resolution = {SCREEN_XRESO, SCREEN_YRESO};
	#define NUM_SAMPLES_PER_SEC 48000.
	float time = waveOutPosition / NUM_SAMPLES_PER_SEC;
#else
	layout(location = 2) uniform float time;
	layout(location = 3) uniform vec2 resolution;
#endif

#if defined(EXPORT_EXECUTABLE)
	/*
		shader minifier が SSBO を認識できない問題を回避するためのハック。
		以下の記述はシェーダコードとしては正しくないが、shader minifier に認識され
		minify が適用されたのち、work_around_begin: 以降のコードに置換される。
		%s は、shader minifier によるリネームが適用されたあとのシンボル名に
		置き換えらえる。

		buffer にはレイアウト名を付ける必要がある。ここでは、レイアウト名 = ssbo と
		している。レイアウト名は shader minifier が生成する他のシンボルと衝突しては
		いけないので、極端に短い名前を付けることは推奨されない。
	*/
	#pragma work_around_begin:layout(std430,binding=0)buffer ssbo{vec2 %s[];};
	vec2 waveOutSamples[];
	#pragma work_around_end
#else
	layout(std430, binding = 0) buffer _{ vec2 waveOutSamples[]; };
#endif

out vec4 outColor;

void main(){
	vec2 pos = gl_FragCoord.xy * 2 / resolution - 1;
	vec2 waveOutSample = waveOutSamples[waveOutPosition + int(gl_FragCoord.x)];
	vec3 color = vec3(0);
	if (abs(pos.y) < abs(waveOutSample.x)) color += vec3(1, .5, 0);
	if (abs(pos.y) < abs(waveOutSample.y)) color += vec3(0, .5, 1);
	outColor = vec4(color, 1);
}

