#version 430	/* version ディレクティブが必要な場合は必ず 1 行目に書くこと */
/* Copyright (C) 2026 Yosshin(@yosshin4004) */

/*
	超基本サンプル

	このサンプルコードで利用される uniform 変数の意味は次のとおりである。
		サウンド再生位置: waveOutPosition
		現在時刻:         time
		画面解像度:       resolution

	エクスポートされた実行ファイル上では、現在時刻および、画面解像度は、
	uniform 変数では与えられなくなるので、次のようにおきかる必要がある。
		現在時刻: サウンド再生位置から求める。
		画面解像度: プリプロセッサ定義 SCREEN_XRESO SCREEN_YRESO を利用する。

	シェーダコードが現在エディットモードで使用されているかエクスポートされた
	実行ファイルから利用されているかは、プリプロセッサ定義 EXPORT_EXECUTABLE
	の有無で判別可能である。

	以上を踏まえ、煩雑になるが、シェーダ導入部で以下のような定型的な処理を
	記述することになる。
*/
#define NUM_SAMPLES_PER_SEC 48000.
layout(location = 0) uniform int waveOutPosition;
#if defined(EXPORT_EXECUTABLE)
	#define resolution vec2(SCREEN_XRESO, SCREEN_YRESO)
	float time = waveOutPosition / NUM_SAMPLES_PER_SEC;
#else
	layout(location = 2) uniform float time;
	layout(location = 3) uniform vec2 resolution;
#endif

out vec4 outColor;

void main(){
	vec2 uv = gl_FragCoord.xy / resolution;
	outColor = vec4(uv + sin(time), 0, 1);
}

