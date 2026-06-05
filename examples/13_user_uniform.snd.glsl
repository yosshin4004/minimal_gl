#version 430	/* version ディレクティブが必要な場合は必ず 1 行目に書くこと */
/* Copyright (C) 2026 Yosshin(@yosshin4004) */

/*
	ユニフォーム変数の imgui および midi 入力バインドサンプル
*/
#define NUM_SAMPLES_PER_SEC 48000.
#define NUM_SOUND_BUFFER_SAMPLES 0x1000000
#define PI (3.14159265358979323846)

layout(location = 0) uniform int waveOutPosition;
layout(std430, binding = 0) buffer SoundOutput{ vec2 waveOutSamples[]; };
layout(local_size_x = 1) in;

layout(location = 25) uniform int algo;				// @ui slider (min = 0, max = 7, default=0)
/*
	KORG nanoKONTROL2 では、cc = 16～23 は上段ノブにアサインされる。
*/
layout(location = 26) uniform int midiInt0;			// @ui midi (cc = 16)
layout(location = 27) uniform int midiInt1;			// @ui midi (cc = 17)
layout(location = 28) uniform int midiInt2;			// @ui midi (cc = 18)
layout(location = 29) uniform int midiInt3;			// @ui midi (cc = 19)
layout(location = 30) uniform float midiFloat0;		// @ui midi (cc = 20)
layout(location = 31) uniform float midiFloat1;		// @ui midi (cc = 21)
layout(location = 32) uniform float midiFloat2;		// @ui midi (cc = 22)


float
EqualTemperament12(
	float key
){
	float freq = pow(2.0, key / 12.0);
	return freq;
}

/*
	double 精度の三角関数は次のように自前で定義する必要がある。
	double 精度の三角関数は OpenGL 4.x 系以降で利用可能だが、
	4.x 系の初期化コードはファイルサイズ肥大を起こすので、
	MinimalGL では非サポートである。
*/
float dsin(double x) {
	return sin(float(fract(x / (PI * 2))) * PI * 2);
}
float dcos(double x) {
	return cos(float(fract(x / (PI * 2))) * PI * 2);
}


void main(){
	int offset = int(gl_GlobalInvocationID.x) + waveOutPosition;
	double sec = double(offset) / NUM_SAMPLES_PER_SEC;

	double p1 = sec * PI * EqualTemperament12(midiInt0);
	double p2 = sec * PI * EqualTemperament12(midiInt1);
	double p3 = sec * PI * EqualTemperament12(midiInt2);
	double p4 = sec * PI * EqualTemperament12(midiInt3);
	double m1 = midiFloat0 * 10;
	double m2 = midiFloat1 * 10;
	double m3 = midiFloat2 * 10;

	/* 雑に実装した FM 音源 */
	float val = 0;
	switch (algo & 7) {
		case 0: {
			/*
				ALG0
				1→2→3→4
			*/
			float o1 = dsin(p1);
			float o2 = dsin(p2 + m1 * o1);
			float o3 = dsin(p3 + m2 * o2);
			float o4 = dsin(p4 + m3 * o3);
			val = o4;
		} break;
		case 1: {
			/*
				ALG 1
				(1+2)→3→4
			*/
			float o1 = dsin(p1);
			float o2 = dsin(p2);
			float o3 = dsin(p3 + m1 * (o1 + o2));
			float o4 = dsin(p4 + m2 * o3);
			val = o4;
		} break;
		case 2: {
			/*
				ALG 2
				1→(2+3)→4
			*/
			float o1 = dsin(p1);
			float o2 = dsin(p2 + m1 * o1);
			float o3 = dsin(p3 + m2 * o1);
			float o4 = dsin(p4 + m3 * (o2 + o3));
			val = o4;
		} break;
		case 3: {
			/*
				ALG 3
				1→2→4 と 3→4
			*/
			float o1 = dsin(p1);
			float o2 = dsin(p2 + m1 * o1);
			float o3 = dsin(p3);
			float o4 = dsin(p4 + m2 * o2 + m3 * o3);
			val = o4;
		} break;
		case 4: {
			/*
				ALG 4
				1→2 と 3→4 の並列
			*/
			float o1 = dsin(p1);
			float o2 = dsin(p2 + m1 * o1);
			float o3 = dsin(p3);
			float o4 = dsin(p4 + m2 * o3);
			val = o2 + o4;
		} break;
		case 5: {
			/*
				ALG 5
				1 が 2,3,4 を同時変調
			*/
			float o1 = dsin(p1);
			float o2 = dsin(p2 + m1 * o1);
			float o3 = dsin(p3 + m2 * o1);
			float o4 = dsin(p4 + m3 * o1);
			val = o2 + o3 + o4;
		} break;
		case 6: {
			/*
				ALG 6
				1→2 と 3 と 4
			*/
			float o1 = dsin(p1);
			float o2 = dsin(p2 + m1 * o1);
			float o3 = dsin(p3);
			float o4 = dsin(p4);
			val = o2 + o3 + o4;
		} break;
		case 7: {
			/*
				ALG 7
				全キャリア
			*/
			float o1 = dsin(p1);
			float o2 = dsin(p2);
			float o3 = dsin(p3);
			float o4 = dsin(p4);
			val = o1 + o2 + o3 + o4;
		} break;
	}

	waveOutSamples[offset % NUM_SOUND_BUFFER_SAMPLES] = vec2(val);
}

