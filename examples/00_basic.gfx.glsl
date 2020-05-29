#version 430	/* version ディレクティブが必要な場合は必ず 1 行目に書くこと */
/* Copyright (C) 2020 Yosshin(@yosshin4004) */

/*
	超基本サンプルコード。

	time は経過時間、resolution は画面の解像度である。

	このサンプルコードは実行ファイルエクスポートには対応していない。
*/
layout(location = 2) uniform float time;
layout(location = 3) uniform vec2 resolution;

void main(){
	vec2 uv = gl_FragCoord.xy / resolution;
	gl_FragColor = vec4(uv + sin(time), 0, 1);
}

