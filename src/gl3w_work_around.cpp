/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#include <GL/gl3w.h>
#include "gl3w_work_around.h"

/*
	gl3w はデフォルトの gl.h を無効化してしまう。
	そのため、デフォルトの gl.h で定義される API を呼び出したいコード上で、
	gl3w.h を include することができない。

	デフォルトの gl.h で定義されている OpenGL API はレガシーなので、これを使わず
	gl3w.h のみで完結するという選択もある。しかし PC 4K Intro ではレガシーか
	どうかに関わらず使える API は何でも使わなければならない。

	やむを得ず、gl3w 関連の API 呼び出しはここに隔離して実行する。
*/
bool CallGl3wInit(){
	return gl3wInit();
}

