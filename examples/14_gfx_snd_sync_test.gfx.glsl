#version 430	/* version ディレクティブが必要な場合は必ず 1 行目に書くこと */
/* Copyright (C) 2026 Yosshin(@yosshin4004) */

/*
	グラフィクス＆サウンド同期テスト

	グラフィクスは、描画完了後すぐにはモニタに表示されない。
	同様にサウンドも、シンセサイズ完了後すぐに音声出力されない。
	表示または出力されるまで、100ミリ秒程度の遅延が発生する。

	遅延特性は、グラフィクス・サウンドそれぞれで異なっている。
	そのため、両者の同期を厳密に取ることはとても難しい。

	遅延に影響を与える要因には、以下のようなものがある。
		・モニタの種類（CRT or 液晶 or ゲーミングモニタ）
		・オーディオ出力デバイス
		・画面モード（全画面 or ウィンドウ）
		・デバイスドライバ
		・機器の接続手段
		・OS の設定
		・OS の世代

	MinimalGL 上でのシェーダエディットは、通常ウィンドウモードで行う。
	一方、実行ファイルにエクスポート後は全画面モードで実行される。
	従って、遅延特性が変化する。そして、その変化量は、実行環境に
	依存して大きく異なる。グラフィクス・サウンドの厳密なシンクロが
	重要なデモの場合、これが問題になる。

	本サンプルコードは、waveOutPosition から単純に求めた時刻を用いて、
	周期的な映像と音声を出力する。これをテストパターンとして実測
	または目視確認することで、遅延特性の変化を把握し調整することが
	できる。
*/
layout(location = 0) uniform int waveOutPosition;
#if defined(EXPORT_EXECUTABLE)
	#define resolution vec2(SCREEN_XRESO, SCREEN_YRESO)
	#define NUM_SAMPLES_PER_SEC 48000.
	float time = waveOutPosition / NUM_SAMPLES_PER_SEC;
#else
	layout(location = 2) uniform float time;
	layout(location = 3) uniform vec2 resolution;
#endif

out vec4 outColor;

#define NUM_SAMPLES_PER_SEC 48000.
void main(){
	vec2 uv = gl_FragCoord.xy / resolution;

	if (uv.x < fract(time)) {
		outColor = vec4(1,1,1,1);
	} else {
		outColor = vec4(0,0,0,1);
	}
}

