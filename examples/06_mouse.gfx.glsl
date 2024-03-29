﻿#version 430	/* version ディレクティブが必要な場合は必ず 1 行目に書くこと */
/* Copyright (C) 2020 Yosshin(@yosshin4004) */

/*
	マウス入力取得サンプルコード。

	デバッグ用途で、マウス座標とボタンの状態が取得できる。

	uniform 変数 mouse が、正規化した画面座標。
	uniform 変数 mouseButtons が、ボタンの状態を示す。

	エクスポートされた実行ファイル上ではこれらの uniform 変数は利用できない。
*/
layout(location = 4) uniform vec2 mouse;
layout(location = 5) uniform ivec3 mouseButtons;

out vec4 outColor;

void main(){
	vec3 color = vec3(mouse * .5, 0);
	if (mouseButtons.x != 0) color.r += .5;
	if (mouseButtons.y != 0) color.g += .5;
	if (mouseButtons.z != 0) color.b += .5;
	outColor = vec4(color, 1);
}

