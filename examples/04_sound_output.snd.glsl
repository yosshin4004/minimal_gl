#version 430	/* version ディレクティブが必要な場合は必ず 1 行目に書くこと */
/* Copyright (C) 2026 Yosshin(@yosshin4004) */

/*
	サウンド出力サンプルコード
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
	/*
		サウンドバッファは、およそ５分程度の固定長のバッファになっている。
		サウンド生成位置を offset % NUM_SOUND_BUFFER_SAMPLES のように
		ループさせることで、５分以上のサウンドを生成することができる。
		ただし、ループ再生はエクスポートされた実行ファイル上では非サポート
		である。
	*/
	waveOutSamples[
#if defined(EXPORT_EXECUTABLE)
		offset
#else
		offset % NUM_SOUND_BUFFER_SAMPLES
#endif
	] = vec2(dsin(sec * PI * 2 * 440)) / exp(float(fract(sec)) * 10);
}

