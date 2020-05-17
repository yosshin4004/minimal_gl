#version 430	/* version ディレクティブが必要な場合は必ず 1 行目に書くこと */
/* Copyright (C) 2020 Yosshin(@yosshin4004) */

/*
	超基本サンプル。

	実行ファイル上では、time はサウンド再生位置を示す waveOutPosition から
	計算で求める必要がある。また、resolution は uniform 変数として与えられず、
	プリプロセッサ定義の SCREEN_XRESO, SCREEN_YRESO を利用する必要がある。

	実行ファイル向けのシェーダかどうかは、プリプロセッサ定義 EXPORT_EXECUTABLE
	で判別可能である。煩雑になるが、以下のように記述する。
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

void main(){
	vec2 uv = gl_FragCoord.xy / resolution;
	gl_FragColor = vec4(uv + sin(time), 0, 1);
}

