#version 430	/* version ディレクティブが必要な場合は必ず 1 行目に書くこと */
/* Copyright (C) 2020 Yosshin(@yosshin4004) */

/*
	テクスチャ利用サンプルコード。
	このサンプルコードを実行するには、11_user_texture.json を 
		File -> Import Project
	で読み込むか、minimal_gl のウィンドウ上にドラッグ＆ドロップすること。


	テクスチャの対応フォーマットは png または dds のみである。
	テクスチャはデバッグ目的での利用のみを想定しており、エクスポート
	された実行ファイルからは利用できない。

	アサインするテクスチャは
		File -> Load User Textures
	で選択できる。
	0 番テクスチャのみは、minimal_gl のウィンドウ上にファイルを
	ドラッグ＆ドロップすることでもアサインできる。

	テクスチャのアサイン情報はプロジェクトファイルとして保存できる。
	プロジェクトファイルを保存するには、
		File -> Export Project
	を選択する。
*/
layout(location = 2) uniform float time;
layout(location = 3) uniform vec2 resolution;
/* sampler2D を samplerCube にすればキューブマップも利用可能。*/
layout(binding =  8) uniform sampler2D userTexture0;

out vec4 outColor;

void main(){
	/*
		gl_FragCoord.xy を uv に変換する手順を順を追って解説する。


		OpenGL の gl_FragCoord.xy 値は、左下が原点である。
		左上、右上、左下、右下 それぞれの gl_FragCoord.xy 値は以下のようになる。

			(0,h) -------------- (w,h)
			  |                    |
			  |                    |
			  |                    |
			  |                    |
			  |                    |
			(0,0) -------------- (w,0)
			※ w=画面横解像度、h=画面横解像度


		OpenGL のテクスチャ uv 値は、画像の左上を原点とする。

			(0,0) -------------- (1,0)
			  |                    |
			  |                    |
			  |                    |
			  |                    |
			  |                    |
			(0,1) -------------- (1,1)


		以上を踏まえると、
		gl_FragCoord.xy を uv に変換するには、解像度で割ったのち、
		v 方向をフリップする必要がある。


		ちなみに ShaderToy では、v 方向をフリップする必要はない。
		この点において、minimalGL は ShaderToy と流儀が異なるので注意。
		（ShaderToy と同様、minimalGL でもテクスチャ load 時に v フリップ
		すれば互換になるが、dds の BC6H BC7 などの v フリップが困難な形式も
		考慮する必要があるため断念して現在の仕様としている。）
	*/
	vec2 uv = gl_FragCoord.xy / resolution;
	uv.y = 1 - uv.y;
	outColor = texture(userTexture0, uv, 0);
}

