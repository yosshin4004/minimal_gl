#version 430	/* version ディレクティブが必要な場合は必ず 1 行目に書くこと */
/* Copyright (C) 2020 Yosshin(@yosshin4004) */

/*
	サウンド出力サンプル
*/

layout(location = 0) uniform int waveOutPosition;
#if defined(EXPORT_EXECUTABLE)
	/*
		shader minifier が compute シェーダに対応していない問題を回避するためのハック。
		以下の記述はシェーダコードとしては正しくないが、shader minifier に認識され
		minify が適用されたのち、work_around_begin: 以降のコードに置換される。
		%s は、shader minifier によるリネームが適用されたあとのシンボル名に
		置き換えらえる。
	*/
	#pragma work_around_begin:layout(std430,binding=0)buffer _{vec2 %s[];};layout(local_size_x=1)in;
	vec2 waveOutSamples[];
	#pragma work_around_end
#else
	layout(std430, binding = 0) buffer SoundOutput{ vec2 waveOutSamples[]; };
	layout(local_size_x = 1) in;
#endif


#define NUM_SAMPLES_PER_SEC 48000.
void main(){
	int offset = int(gl_GlobalInvocationID.x) + waveOutPosition;
	float sec = float(offset) / NUM_SAMPLES_PER_SEC;
	waveOutSamples[offset] = sin(vec2(sec * 440 * 4)) * exp(-sec);
}

