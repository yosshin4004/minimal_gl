#version 430	/* version ディレクティブが必要な場合は必ず 1 行目に書くこと */
/* Copyright (C) 2026 Yosshin(@yosshin4004) */

/*
	サウンド出力サンプルコード。
*/
#define NUM_SAMPLES_PER_SEC 48000.
#define NUM_SOUND_BUFFER_SAMPLES 0x1000000
#define PI (3.14159265358979323846)


layout(location = 0) uniform int waveOutPosition;
#if defined(EXPORT_EXECUTABLE)
	/*
		shader minifier が compute シェーダに対応していない問題を回避するためのハック。
		以下の記述はシェーダコードとしては正しくないが、shader minifier に認識され
		minify が適用されたのち、work_around_begin: 以降のコードに置換される。
		%s は、shader minifier によるリネームが適用されたあとのシンボル名に
		置き換えられる。

		buffer にはレイアウト名を付ける必要がある。ここでは、レイアウト名 = ssbo と
		している。レイアウト名は shader minifier が生成する他のシンボルと衝突しては
		いけないので、極端に短い名前を付けることは推奨されない。
	*/
	#pragma work_around_begin:layout(std430,binding=0)buffer ssbo{vec2 %s[];};layout(local_size_x=1)in;
	vec2 waveOutSamples[];
	#pragma work_around_end
#else
	layout(std430, binding = 0) buffer SoundOutput{ vec2 waveOutSamples[]; };
	layout(local_size_x = 1) in;
#endif




float
EqualTemperament12(
	float key
){
	float freq = pow(2.0, key / 12.0);
	return freq;
}


void main(){
	int offset = int(gl_GlobalInvocationID.x) + waveOutPosition;
	float sec = float(offset) / NUM_SAMPLES_PER_SEC;
	float freq = 440 * EqualTemperament12(0);
	waveOutSamples[offset % NUM_SOUND_BUFFER_SAMPLES] = sign(sin(vec2(sec * PI * freq))) * exp(-fract(sec) * 100);
}

