#version 430	/* version ディレクティブが必要な場合は必ず 1 行目に書くこと */
/* Copyright (C) 2020 Yosshin(@yosshin4004) */

/*
	キューブマップ利用サンプルコード。
	このサンプルコードを実行するには、12_user_cubemap.json を 
		File -> Import Project
	で読み込むか、minimal_gl のウィンドウ上にドラッグ＆ドロップすること。

	アサインするキューブマップは
		File -> Load User Textures
	で変更できる。
	0 番テクスチャのみは、minimal_gl のウィンドウ上にファイルを
	ドラッグ＆ドロップすることでもアサインできる。

	12_user_cubemap.dds は、07_camera_and_cubemap.gfx.glsl を利用して生成
	されている。
*/
layout(location = 2) uniform float time;
layout(location = 3) uniform vec2 resolution;
layout(location = 6) uniform float tanFovY;
layout(location = 7) uniform mat4 cameraInWorld;
layout(binding =  8) uniform samplerCube userTexture0;

out vec4 outColor;

void main(){
	vec3 cameraAxisX = cameraInWorld[0].xyz;
	vec3 cameraAxisY = cameraInWorld[1].xyz;
	vec3 cameraAxisZ = cameraInWorld[2].xyz;

	vec3 rayDir = mat3(cameraAxisX, cameraAxisY, cameraAxisZ)
				* normalize(vec3(((gl_FragCoord.xy * 2 - resolution) / resolution.y) * tanFovY, -1));

	outColor = texture(userTexture0, rayDir, 0);
}

