#version 430	/* version ディレクティブが必要な場合は必ず 1 行目に書くこと */
/* Copyright (C) 2020 Yosshin(@yosshin4004) */

/*
	超基本サンプル。

	デバッグ用途で、マウス座標とボタンの状態が取得できる。

	uniform 変数 mouse が正規化した画面座標。
	uniform 変数 mouseButtons がボタンの状態を示す。

	エクスポートされた実行ファイル上ではこれらの uniform 変数は利用できない。
*/
layout(location = 4) uniform vec2 mouse;
layout(location = 5) uniform ivec3 mouseButtons;

void main(){
	vec3 color = vec3(mouse * .5, 0);
	if (mouseButtons.x != 0) color.r += .5;
	if (mouseButtons.y != 0) color.g += .5;
	if (mouseButtons.z != 0) color.b += .5;
	gl_FragColor = vec4(color, 1);
}

