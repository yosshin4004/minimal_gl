/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#ifndef _CONFIG_H_
#define _CONFIG_H_


/* スクリーン解像度 */
#define SCREEN_XRESO							(1280)
#define SCREEN_YRESO							(720)

/* キューブマップの初期解像度 */
#define CUBEMAP_RESO							(512)

/* 解像度の上限 */
#define MAX_RESO								(8192)

/* uniform の location */
#define UNIFORM_LOCATION_WAVE_OUT_POS			0
#define UNIFORM_LOCATION_FRAME_COUNT			1
#define UNIFORM_LOCATION_TIME					2
#define UNIFORM_LOCATION_RESO					3
#define UNIFORM_LOCATION_MOUSE_POS				4
#define UNIFORM_LOCATION_MOUSE_BUTTONS			5
#define UNIFORM_LOCATION_TAN_FOVY				6
#define UNIFORM_LOCATION_CAMERA_COORD			7

/* レンダーターゲット数 */
#define NUM_RENDER_TARGETS						(4)

/* ユーザーテクスチャ数 */
#define NUM_USER_TEXTURES						(4)

/* サウンドのサンプルの型 */
#define SOUND_SAMPLE_TYPE						float

/* サウンドのサンプルの型は float か？ */
#define SOUND_SAMPLE_TYPE_IS_FLOAT				1

/* サウンド再生デバイスの周波数 */
#define NUM_SOUND_SAMPLES_PER_SEC				48000

/* サウンド再生デバイスのチャンネル数 */
#define NUM_SOUND_CHANNELS						2

/* サウンドバッファサンプル数 */
#define NUM_SOUND_BUFFER_SAMPLES				0x1000000

/* 1 dispatch で生成するサウンドサンプル数 */
#define NUM_SOUND_BUFFER_SAMPLES_PER_DISPATCH	0x8000

/* サウンドバッファパーティション数 */
#define NUM_SOUND_BUFFER_PARTITIONS				(NUM_SOUND_BUFFER_SAMPLES / NUM_SOUND_BUFFER_SAMPLES_PER_DISPATCH)

/* 文字列リテラルに変換するマクロ */
#define TO_STRING_SUB(token) #token
#define TO_STRING(token) TO_STRING_SUB(token)


#endif