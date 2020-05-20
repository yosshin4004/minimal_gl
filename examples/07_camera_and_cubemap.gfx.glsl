#version 430	/* version ディレクティブが必要な場合は必ず 1 行目に書くこと */
/* Copyright (C) 2020 Yosshin(@yosshin4004) */

/*
	カメラコントロール＆ CUBEMAP キャプチャ対応サンプルコード。

	デバッグ用途で、マウスによるカメラ移動機能が利用できる。
		左クリック＆ドラッグ：座標変更
		右クリック＆ドラッグ：角度変更
		ホイール：前進＆後退
		中央ボタンを押しながらホイール移動：画角変更
	現在のカメラの状態は、uniform 変数 cameraInWorld と tanFovY で取得できる。

	また、これらの uniform 変数を適切に利用していると、CUBEMAP キャプチャ
	機能が利用可能になる。
	CUBEMAP キャプチャを行うには、メニューから "Capture cubemap" を選択する。

	エクスポートされた実行ファイル上ではこれらの uniform 変数は利用できない。
*/

layout(location = 3) uniform vec2 resolution;
layout(location = 6) uniform float tanFovY;
layout(location = 7) uniform mat4 cameraInWorld;


float GetBoxDistance(
	vec3 position,
	vec3 extent
){
	vec3 absPosition = abs(position) - extent;
	return max(max(absPosition.x, absPosition.y), absPosition.z);
}

float GetBoxFieldDistance(
	vec3 position
){
	position = (fract(position * .5) - .5) * 2;
	return GetBoxDistance(position, vec3(.7));
}

float GetSceneDistance(
	vec3 position
){
	return GetBoxFieldDistance(position);
}

float GetSceneIntersect(
	vec3 rayOrg,
	vec3 rayDir
){
	float l = 0;
	for (int i = 0; i < 256; ++i) {
		vec3 position = rayOrg + rayDir * l;
		float dist = GetSceneDistance(position);
		if (abs(dist) < (l * 5. + 1.) * .0001) { return l; }
		l += dist;
	}
	return l;
}

vec3 EstimateSceneNormal(
	vec3 position
){
	vec3 eps = vec3(.001, 0, 0);
	return
		normalize(
			vec3(
				GetSceneDistance(position + eps.xyy) - GetSceneDistance(position - eps.xyy),
				GetSceneDistance(position + eps.yxy) - GetSceneDistance(position - eps.yxy),
				GetSceneDistance(position + eps.yyx) - GetSceneDistance(position - eps.yyx)
			)
		)
	;
}

vec3 Shading(
	vec3 position,
	vec3 normal
){
	return normal * .5 + .5;
}

void main(){
	vec3 cameraAxisX	= cameraInWorld[0].xyz;
	vec3 cameraAxisY	= cameraInWorld[1].xyz;
	vec3 cameraAxisZ	= cameraInWorld[2].xyz;
	vec3 cameraPosition	= cameraInWorld[3].xyz;

	vec3 rayOrg = cameraPosition;
	vec3 rayDir = mat3(cameraAxisX, cameraAxisY, cameraAxisZ)
				* normalize(vec3(((gl_FragCoord.xy * 2 - resolution) / resolution.y) * tanFovY, -1));
	vec3 color = vec3(0);

	float lIntersect = GetSceneIntersect(rayOrg, rayDir);
	if (lIntersect < 100) {
		vec3 intersectPosition = rayOrg + rayDir * lIntersect;
		vec3 normal = EstimateSceneNormal(rayOrg + rayDir * lIntersect);
		color = Shading(intersectPosition, normal);
	}

	gl_FragColor = vec4(color, 1);
}

