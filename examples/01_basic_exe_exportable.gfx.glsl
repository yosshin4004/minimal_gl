#version 430	/* version ディレクティブが必要な場合は必ず 1 行目に書くこと */
/* Copyright (C) 2020 Yosshin(@yosshin4004) */

/*
	超基本サンプル。

	エクスポートされた実行ファイル上では、time はサウンド再生位置を示す
	waveOutPosition から計算で求める必要がある。また、resolution は uniform
	変数として与えられなくなるので、解像度を知るにはプリプロセッサ定義の
	SCREEN_XRESO, SCREEN_YRESO を参照する必要がある。

	シェーダコードが現在エディットモードで使用されているかエクスポート
	された実行ファイルから利用されているかは、プリプロセッサ定義
	EXPORT_EXECUTABLE の有無で判別可能である。

	煩雑になるが、シェーダ導入部で以下のような定型的な処理を記述することになる。
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

