#version 430	/* version ディレクティブが必要な場合は必ず 1 行目に書くこと */
/* Copyright (C) 2026 Yosshin(@yosshin4004) */

/*
*/
layout(location = 2) uniform float time;
layout(location = 3) uniform vec2 resolution;

/*
	ユニフォーム変数の imgui および midi 入力バインドサンプル

	location=16 以降はユーザーユニフォームとして利用できる。
	ユーザーユニフォーム定義行の末尾に、このサンプルコードで示したようなコメントを
	記述することで、imgui ウィジェットまたは midi 入力を自動でバインドすることができ、
	動的に変更したパラメータをリアルタイムに出力結果に反映することができる。

	midi 入力をアサインする場合は、cc（Control Change Number）を指定する必要がある。
	cc は、midi 機器を接続した状態で、次の web サイトにアクセスすることで確認可能。
		https://www.midimonitor.com/
	midi 入力に接続したユニフォームは、値の変更は midi デバイスで行い、imgui 上では
	表示のみが行われる。
*/
layout(location = 16) uniform bool	myCheckbox;		// @ui checkbox (default=false)
layout(location = 17) uniform int	myInputInt;		// @ui input (default=123)
layout(location = 18) uniform float	myInputFloat;	// @ui input (default=123)
layout(location = 19) uniform int	mySliderInt;	// @ui slider (min=0, max=10, default=5)
layout(location = 20) uniform float	mySliderFloat;	// @ui slider (min=0, max=10, default=5)
layout(location = 21) uniform vec3	myColor3;		// @ui color (default={1,1,1})
layout(location = 22) uniform vec4	myColor4;		// @ui color (default={1,1,1,0.5})
layout(location = 23) uniform int	myMidiInInt;	// @ui midi (cc=0)
layout(location = 24) uniform float	myMidiInFloat;	// @ui midi (cc=1)

out vec4 outColor;

void main(){
	vec2 uv = gl_FragCoord.xy / resolution;
	uv.y = 1 - uv.y;

	outColor = vec4(0,0,0,1);

	if (uv.y < 0.1) {
		outColor.rgb = myCheckbox? vec3(1): vec3(0);
	}

	if (0.1 <= uv.y && uv.y < 0.2) {
		outColor.rgb = fract(myInputInt * vec3(0.1, 0.2, 0.3));
	}

	if (0.2 <= uv.y && uv.y < 0.3) {
		outColor.rgb = fract(myInputFloat * vec3(0.1, 0.2, 0.3));
	}

	if (0.4 <= uv.y && uv.y < 0.5) {
		outColor.rgb = fract(mySliderInt * vec3(0.1, 0.2, 0.3));
	}

	if (0.5 <= uv.y && uv.y < 0.6) {
		outColor.rgb = fract(mySliderFloat * vec3(0.1, 0.2, 0.3));
	}

	if (0.6 <= uv.y && uv.y < 0.7) {
		outColor.rgb = myColor3;
	}

	if (0.7 <= uv.y && uv.y < 0.8) {
		vec3 checkPattern = vec3(((int(gl_FragCoord.x) ^ int(gl_FragCoord.y)) & 16) == 0? 0.75: 0.4);
		outColor.rgb = mix(checkPattern, myColor4.rgb, myColor4.a);
	}

	if (0.8 <= uv.y && uv.y < 0.9) {
		outColor.rgb = fract(myMidiInInt * vec3(0.1, 0.2, 0.3));
	}

	if (0.9 <= uv.y && uv.y < 1.0) {
		outColor.rgb = fract(myMidiInFloat * vec3(0.1, 0.2, 0.3));
	}
}

