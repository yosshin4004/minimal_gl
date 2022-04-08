#version 430	/* version ディレクティブが必要な場合は必ず 1 行目に書くこと */
/* Copyright (C) 2020 Yosshin(@yosshin4004) */


/*
	twigl (https://twigl.app/) グラフィクスシェーダ互換サンプルコード。
	geeker (300 es) 対応。

	twigl では #define マクロを利用した minify テクニックが多用されるが、
	MinimalGL の場合は #define マクロはすべてプリプロセッサの段階で展開される
	ため minify の手段としては利用できない。
*/

layout(binding = 0) uniform sampler2D b;
layout(location = 0) uniform int waveOutPosition;
#if defined(EXPORT_EXECUTABLE)
	#define NUM_SAMPLES_PER_SEC 48000.
	float t = waveOutPosition / NUM_SAMPLES_PER_SEC;
	vec2 r = {SCREEN_XRESO, SCREEN_YRESO};
	vec2 m = vec2(0);
#else
	layout(location = 2) uniform float t;
	layout(location = 3) uniform vec2 r;
	layout(location = 4) uniform vec2 m;
#endif

#if defined(EXPORT_EXECUTABLE)
	/*
		shader minifier が SSBO を認識できない問題を回避するためのハック。
		以下の記述はシェーダコードとしては正しくないが、shader minifier に認識
		され minify が適用されたのち、work_around_begin: 以降のコードに置換される。
		%s は、shader minifier によるリネームが適用されたあとのシンボル名に
		置き換えらえる。
	*/
	#pragma work_around_begin:layout(std430,binding=0)buffer _{vec2 %s[];};
	vec2 waveOutSamples[];
	#pragma work_around_end
#else
	layout(std430, binding = 0) buffer _{ vec2 waveOutSamples[]; };
#endif

out vec4 outColor;


/* s は仮対応である。twigl 互換ではない。*/
float s = waveOutSamples[waveOutPosition].x + waveOutSamples[waveOutPosition].y;
vec4 o = vec4(0);


/*
	ここに twigl の main 関数の中身を張り付ける。
*/
void geekerEs300Main(){
	vec2 p=(gl_FragCoord.xy*2.-r)/min(r.x,r.y)-m;
	for(int i=0;i<8;++i){p.xy=abs(p)/abs(dot(p,p))-vec2(.9+cos(t*.2)*.4);}
	o=vec4(p.xxy,1);
}

void main(){
	geekerEs300Main();
	outColor = o;
}

