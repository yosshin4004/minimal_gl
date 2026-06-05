/* Copyright (C) 2026 Yosshin(@yosshin4004) */
#include "config.h"
#include "common.h"
#include "app.h"
#include "sound.h"
#include "wav_util.h"
#include "wasapi.h"
#include "user_uniform.h"
#include <process.h>
#include <winbase.h>


#define BUFFER_INDEX_FOR_SOUND_OUTPUT			(0)

static bool s_paused = false;
static GLuint s_soundShaderId = 0;
static GLuint s_soundOutputSsbo = 0;

#define NUM_SYNTH_RING_BUFFER_SAMPLES			(NUM_SOUND_BUFFER_SAMPLES)	/* ２のべき乗 */
#define NUM_SYNTH_RING_BUFFER_SIZE_IN_BYTES		(NUM_SYNTH_RING_BUFFER_SAMPLES * sizeof(SOUND_SAMPLE_TYPE) * NUM_SOUND_CHANNELS)
#define NUM_PRE_SYNTH_FRAMES					(16)						/* 先行フレーム数（60FPS 換算）*/
#define NUM_PRE_SYNTH_SAMPLES					(NUM_SOUND_SAMPLES_PER_SEC / 60 * NUM_PRE_SYNTH_FRAMES)
#define NUM_MAX_SYNTH_SAMPLES_PER_DISPATCH		(NUM_PRE_SYNTH_SAMPLES)		/* 先行波形は 1 dispatch ですべて生成可能 */
static struct {
	SOUND_SAMPLE_TYPE (*samples)[NUM_SOUND_CHANNELS];
	int32_t atomicReadPosition;
	int32_t atomicWritePosition;
	int32_t atomicWriteInprogressPosition;
	GLsync glSync;
} s_synthRingBuffer = {};

static struct {
	int32_t seekBasePosition;
	HANDLE hThread;
	int32_t atomicHasFinished;
	CRITICAL_SECTION criticalSection;
} s_waveOut = {
	/* seekBasePosition */ 0,
	/* hThread*/ 0,
	/* atomicHasFinished */ 0,
	/* criticalSection */ 0
};

/*=============================================================================
▼	サウンド合成関連
-----------------------------------------------------------------------------*/
static void SoundSync(){
	/* 直前の SoundSynthesize が完了していることを確認 */
	if (s_synthRingBuffer.glSync != 0) {
		glClientWaitSync(
			/* GLsync sync */		s_synthRingBuffer.glSync,
			/* GLbitfield flags */	GL_SYNC_FLUSH_COMMANDS_BIT,
			/* GLuint64 timeout */	GLuint64(1e9)		/* 1e9 = 1,000,000,000 ns = 1 sec */
		);
		glDeleteSync(s_synthRingBuffer.glSync);
		s_synthRingBuffer.glSync = 0;
	}
}

static void SoundSynthesize(
	int synthPosition,
	int numSamples
){
	if (s_soundShaderId == 0) return;

	/* シェーダをバインド */
	glUseProgram(s_soundShaderId);

	/* 出力先バッファのバインド */
	glBindBufferBase(
		/* GLenum target */	GL_SHADER_STORAGE_BUFFER,
		/* GLuint index */	BUFFER_INDEX_FOR_SOUND_OUTPUT,
		/* GLuint buffer */	s_soundOutputSsbo
	);

	/* ユーザーユニフォームパラメータを設定 */
	UserUniformApplyToShader(s_soundShaderId, UserUniformCategoryIndex_Sound);

	/*
		生成波形数がリングバッファ1周分を超える場合の対処。
		ラップアラウンドして多重に上書きされる部分をキャンセルする。
	*/
	if (numSamples > NUM_SYNTH_RING_BUFFER_SAMPLES) {
		uint32_t numSkipSamples = numSamples - NUM_SYNTH_RING_BUFFER_SAMPLES;
		synthPosition += numSkipSamples;
		numSamples = NUM_SYNTH_RING_BUFFER_SAMPLES;
	}

	/* ディスパッチサイズ上限の範囲内に分割しながらディスパッチ */
	while (numSamples > 0) {
		/* ディスパッチするサンプル数を決定 */
		int numDispatchSamples = numSamples;
		if (numDispatchSamples > NUM_MAX_SYNTH_SAMPLES_PER_DISPATCH) {
			numDispatchSamples = NUM_MAX_SYNTH_SAMPLES_PER_DISPATCH;
		}

		/* エラーチェック */
		CheckGlError("SoundUpdate : pre dispatch");
		/* サウンド生成 */
		{
			int wrapAroundedSynthPosition = synthPosition & (NUM_SYNTH_RING_BUFFER_SAMPLES - 1);

			/* サウンド出力先バッファは連続領域？ */
			if (wrapAroundedSynthPosition + numDispatchSamples <= NUM_SYNTH_RING_BUFFER_SAMPLES) {
				/* ユニフォームパラメータの設定 */
				if (ExistsShaderUniform(s_soundShaderId, UNIFORM_LOCATION_WAVE_OUT_POS, GL_INT)) {
					glUniform1i(
						UNIFORM_LOCATION_WAVE_OUT_POS,
						synthPosition
					);
				}

				/* コンピュートシェーダによるサウンド生成 */
				glDispatchCompute(numDispatchSamples, 1, 1);
			} else {
				/* ユニフォームパラメータの設定 */
				if (ExistsShaderUniform(s_soundShaderId, UNIFORM_LOCATION_WAVE_OUT_POS, GL_INT)) {
					glUniform1i(
						UNIFORM_LOCATION_WAVE_OUT_POS,
						synthPosition
					);
				}

				/* コンピュートシェーダによるサウンド生成 */
				int numPartialDispatchSamples = NUM_SYNTH_RING_BUFFER_SAMPLES - wrapAroundedSynthPosition;
				glDispatchCompute(numPartialDispatchSamples, 1, 1);

				/* ユニフォームパラメータの設定 */
				if (ExistsShaderUniform(s_soundShaderId, UNIFORM_LOCATION_WAVE_OUT_POS, GL_INT)) {
					glUniform1i(
						UNIFORM_LOCATION_WAVE_OUT_POS,
						synthPosition + numPartialDispatchSamples
					);
				}

				/* コンピュートシェーダによるサウンド生成 */
				glDispatchCompute(numDispatchSamples - numPartialDispatchSamples, 1, 1);
			}
		}

		/* エラーチェック */
		CheckGlError("SoundUpdate : post dispatch");

		/* numDispatchSamples 分が処理完了 */
		numSamples -= numDispatchSamples;
		synthPosition += numDispatchSamples;
	}

	/* 出力先バッファのアンバインド */
	glBindBufferBase(
		/* GLenum target */	GL_SHADER_STORAGE_BUFFER,
		/* GLuint index */	BUFFER_INDEX_FOR_SOUND_OUTPUT,
		/* GLuint buffer */	0	/* unbind */
	);

	/* シェーダをアンバインド */
	glUseProgram(NULL);
}

bool SoundDeleteShader(GLuint soundShaderId){
	if (soundShaderId == 0) return false;
	glFinish();
	glDeleteProgram(soundShaderId);
	return true;
}

bool SoundSetupShader(
	const char *shaderCode
){
	printf("setting up sound shader ...\n");

	const GLchar *(strings[]) = {
		SkipBomConst(shaderCode)
	};
	GLuint soundShaderId = 0;
	soundShaderId = CreateShader(GL_COMPUTE_SHADER, SIZE_OF_ARRAY(strings), strings);
	if (soundShaderId == 0) {
		printf("setting up sound shader ... failed.\n");
		return false;
	}
	SoundDeleteShader(s_soundShaderId);
	s_soundShaderId = soundShaderId;
	DumpShaderInterfaces(s_soundShaderId);

	printf("setting up sound shader ... done.\n");
	return true;
}

float SoundDetectDurationInSeconds(){
	/* サウンドの完全な更新（ブロッキング）*/
	SoundFullUpdate();

	/* 有効なサンプルの末端位置を求める */
	int numAvailableSamples = 0;
	{
		for (int iSample = NUM_SYNTH_RING_BUFFER_SAMPLES - 1; iSample >= 0; iSample--) {
			for (int iChannel = 0; iChannel < NUM_SOUND_CHANNELS; iChannel++) {
				if (s_synthRingBuffer.samples[iSample][iChannel] != 0) {
					numAvailableSamples = iSample + 1;
					break;
				}
			}
			if (numAvailableSamples != 0) break;
		}
	}

	/* 秒数に置き換える */
	return (float)numAvailableSamples / (float)NUM_SOUND_SAMPLES_PER_SEC;
}

static bool SoundCreateSoundOutputBuffer(){
	glGenBuffers(
		/* GLsizei n */				1,
		/* GLuint * buffers */		&s_soundOutputSsbo
	);
	glBindBuffer(
		/* GLenum target */			GL_SHADER_STORAGE_BUFFER,
		/* GLuint buffer */			s_soundOutputSsbo
	);
	/*
		AMD 環境では、GL_MAP_PERSISTENT_BIT を指定しないバッファは、
		持続的な MAP 状態にできない。GL_MAP_PERSISTENT_BIT を指定するには、
		glBufferData でなく glBufferStorage を利用する必要がある。
	*/
	glBufferStorage(
		/* GLenum target */			GL_SHADER_STORAGE_BUFFER,
		/* GLsizeiptr size */		NUM_SYNTH_RING_BUFFER_SIZE_IN_BYTES,
		/* const void * data */		NULL,
		/* GLbitfield flags */		GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT /* = CPU が読める */
	);
	s_synthRingBuffer.samples = (SOUND_SAMPLE_TYPE (*)[NUM_SOUND_CHANNELS])glMapBufferRange(
		/* GLenum target */			GL_SHADER_STORAGE_BUFFER,
		/* GLintptr offset */		0,
		/* GLsizeiptr length */		NUM_SYNTH_RING_BUFFER_SIZE_IN_BYTES,
		/* GLbitfield access */		GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT /* = CPU が読める */
	);
	assert(s_synthRingBuffer.samples != NULL);
	glBindBuffer(
		/* GLenum target */			GL_SHADER_STORAGE_BUFFER,
		/* GLuint buffer */			0	/* unbind */
	);
	return true;
}

static bool SoundDeleteSoundOutputBuffer(){
	glBindBuffer(
		/* GLenum target */		GL_SHADER_STORAGE_BUFFER,
		/* GLuint buffer */		s_soundOutputSsbo
	);
	glUnmapBuffer(
		/* GLenum target */		GL_SHADER_STORAGE_BUFFER
	);
	glDeleteBuffers(
		/* GLsizei n */			1,
		/* GLuint * buffers */	&s_soundOutputSsbo
	);
	s_soundOutputSsbo = 0;

	return true;
}

void SoundClearOutputBuffer(){
	memset(s_synthRingBuffer.samples, 0, NUM_SYNTH_RING_BUFFER_SIZE_IN_BYTES);
}

GLuint SoundGetOutputSsbo(){
	return s_soundOutputSsbo;
}

/*=============================================================================
▼	サウンドキャプチャ関連
-----------------------------------------------------------------------------*/
bool SoundCaptureSound(
	const CaptureSoundSettings *settings
){
	/* サウンドの完全な更新（ブロッキング）*/
	SoundFullUpdate();

	/* サウンドの持続時間をサンプル数に変換 */
	int numSamples = (int)(settings->durationInSeconds * NUM_SOUND_SAMPLES_PER_SEC);
	if (numSamples < 0) numSamples = 0;
	if (numSamples > NUM_SOUND_BUFFER_SAMPLES) { numSamples = NUM_SOUND_BUFFER_SAMPLES; }

	/* wav ファイルに保存 */
	return SerializeAsWav(
		/* const char *fileName */			settings->fileName,
		/* const void *buffer */			s_synthRingBuffer.samples,
		/* int numChannels */				NUM_SOUND_CHANNELS,
		/* int numSamples */				numSamples,
		/* int numSamplesPerSec */			NUM_SOUND_SAMPLES_PER_SEC,
		/* uint16_t formatID */
#if SOUND_SAMPLE_TYPE_IS_FLOAT
											WAVE_FORMAT_IEEE_FLOAT,
#else
											WAVE_FORMAT_PCM,
#endif
		/* int bitsPerSampleComponent */	sizeof(SOUND_SAMPLE_TYPE) * 8
	);
}

/*=============================================================================
▼	サウンド出力関連
-----------------------------------------------------------------------------*/
static bool SoundOutput(){
	EnterCriticalSection(&s_waveOut.criticalSection);

	/* アトミック変数読み取り */
	int32_t synthRingBufferReadPosition  = AtomicGet32(&s_synthRingBuffer.atomicReadPosition);
	int32_t synthRingBufferWritePosition = AtomicGet32(&s_synthRingBuffer.atomicWritePosition);

	#define NUM_MAX_BURST_SAMPLES	(0x4000)
	static SOUND_SAMPLE_TYPE s_samples[NUM_MAX_BURST_SAMPLES][NUM_SOUND_CHANNELS];

	/* 出力サンプル数 */
	int numOutputSamples = synthRingBufferWritePosition - synthRingBufferReadPosition;
	if (numOutputSamples < 0) { numOutputSamples = 0; }

	/* 出力サンプル数全体を処理するまで繰り返す */
	while (numOutputSamples > 0) {
		/* 一括転送サンプル数 */
		int numBurstSamples = numOutputSamples;
		if (numBurstSamples > NUM_MAX_BURST_SAMPLES) {
			numBurstSamples = NUM_MAX_BURST_SAMPLES;
		}

		/* WASAPI の出力可能サンプル数 */
		int numWasapiAvailableSamples = WasapiGetAvailableFrames();
		if (numBurstSamples > numWasapiAvailableSamples) {
			numBurstSamples = numWasapiAvailableSamples;
		}

		/* 一括転送サンプル数が 0 ならループを抜ける */
		if (numBurstSamples == 0) {
			break;
		}

		/* 一括転送 */
		{
//printf("numBurstSamples %d\n", numBurstSamples);
			/* 出力バイト数 */
			size_t burstSizeInBytes = numBurstSamples * NUM_SOUND_CHANNELS * sizeof(SOUND_SAMPLE_TYPE);

			/* ラップアラウンドした読み取り位置 */
			int wrapAroundedSynthRingBufferReadPosition = synthRingBufferReadPosition & (NUM_SYNTH_RING_BUFFER_SAMPLES - 1);

			/* サンプル出力 */
			if (wrapAroundedSynthRingBufferReadPosition + (int)numBurstSamples <= NUM_SYNTH_RING_BUFFER_SAMPLES) {
				/* リングバッファ終点をまたがない場合 */
				memcpy(
					s_samples,
					&s_synthRingBuffer.samples[wrapAroundedSynthRingBufferReadPosition][0],
					burstSizeInBytes
				);
			} else {
				/* リングバッファ終点をまたぐ場合 */
				size_t firstPartSizeInBytes  = (NUM_SYNTH_RING_BUFFER_SAMPLES - wrapAroundedSynthRingBufferReadPosition) * NUM_SOUND_CHANNELS * sizeof(SOUND_SAMPLE_TYPE);
				size_t secondPartSizeInBytes = burstSizeInBytes - firstPartSizeInBytes;
				memcpy(
					s_samples,
					(const void *)&s_synthRingBuffer.samples[wrapAroundedSynthRingBufferReadPosition][0],
					firstPartSizeInBytes
				);
				memcpy(
					(void *)(((uintptr_t)s_samples) + firstPartSizeInBytes),
					&s_synthRingBuffer.samples[0][0],
					secondPartSizeInBytes
				);
			}

			/* 音声データの書き込み */
			WasapiWrite(&s_samples[0][0], numBurstSamples);

			/* numOutputSamples 分が処理完了 */
			synthRingBufferReadPosition += numBurstSamples;
			numOutputSamples -= numBurstSamples;

			/* アトミック変数書き戻し */
			AtomicSet32(&s_synthRingBuffer.atomicReadPosition, synthRingBufferReadPosition);
		}
	}

	/* アトミック変数書き戻し */
	AtomicSet32(&s_synthRingBuffer.atomicReadPosition, synthRingBufferReadPosition);

	LeaveCriticalSection(&s_waveOut.criticalSection);
	return true;
}

static unsigned __stdcall SoundStreamingThreadProc(
	void	*pArg
) {
	while (AtomicGet32(&s_waveOut.atomicHasFinished) == 0) {
		SoundOutput();
		Sleep(10);
	}

	return 0;
}

static void SoundPrepare(){
	WasapiInitialize(NUM_SOUND_SAMPLES_PER_SEC, NUM_SOUND_CHANNELS);
}
static void SoundUnprepare(){
	WasapiTerminate();
}

void SoundPauseWaveOut(){
	WasapiPause();
	s_paused = true;
}

void SoundResumeWaveOut(){
	WasapiResume();
	s_paused = false;
}

void SoundRestartWaveOut(){
	s_paused = false;
	SoundSeekWaveOut(0);
}

void SoundSeekWaveOut(uint32_t offset){
	EnterCriticalSection(&s_waveOut.criticalSection);
	s_waveOut.seekBasePosition = offset;
	AtomicSet32(&s_synthRingBuffer.atomicReadPosition, offset);
	AtomicSet32(&s_synthRingBuffer.atomicWritePosition, offset);
	AtomicSet32(&s_synthRingBuffer.atomicWriteInprogressPosition, offset);
	WasapiReset();
	if (s_paused) {
		SoundPauseWaveOut();
	}
	SoundUpdate();
	SoundSync();
	LeaveCriticalSection(&s_waveOut.criticalSection);
}

int SoundGetWaveOutPos(){
	return
		WasapiGetPosition()
	+	s_waveOut.seekBasePosition;
}

/*=============================================================================
▼	サウンド関連
-----------------------------------------------------------------------------*/
void SoundUpdate(){
	/*
		前回の dispatch の完了を待つ。
		つまり s_synthRingBuffer.atomicWriteInprogressPosition の位置まで、
		サウンド関連が完了するのを待つ。
	*/
	SoundSync();

	{
		/* アトミック変数読み取り */
		int32_t synthRingBufferWriteInprogressPosition = AtomicGet32(&s_synthRingBuffer.atomicWriteInprogressPosition);
		int32_t synthRingBufferWritePosition           = synthRingBufferWriteInprogressPosition;

		/* 現在の waveOutPos よりも NUM_PRE_SYNTH_SAMPLES 先行した位置まで波形生成する */
		int numSynthSamples = SoundGetWaveOutPos() + NUM_PRE_SYNTH_SAMPLES - synthRingBufferWritePosition;
		if (numSynthSamples > 0) {
//printf("numSynthSamples %d\n", numSynthSamples);
			SoundSynthesize(synthRingBufferWritePosition, numSynthSamples);
			synthRingBufferWritePosition += numSynthSamples;
		}

		/* アトミック変数書き戻し */
		AtomicSet32(&s_synthRingBuffer.atomicWriteInprogressPosition, synthRingBufferWritePosition);
		AtomicSet32(&s_synthRingBuffer.atomicWritePosition, synthRingBufferWriteInprogressPosition);
	}

	/* この時点までの完了 fence */
	s_synthRingBuffer.glSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

void SoundFullUpdate(){
	SoundClearOutputBuffer();
	SoundSynthesize(0, NUM_SYNTH_RING_BUFFER_SAMPLES);
	glFinish();
}


bool SoundInitialize(){
	InitializeCriticalSection(&s_waveOut.criticalSection);

	if (SoundCreateSoundOutputBuffer() == false) {
		printf("SoundInitialize: SoundCreateSoundOutputBuffer failed.\n");
		DeleteCriticalSection(&s_waveOut.criticalSection);
		return false;
	}
	SoundClearOutputBuffer();

	SoundRestartWaveOut();
	SoundPrepare();

	AtomicSet32(&s_waveOut.atomicHasFinished, 0);
	s_waveOut.hThread = (HANDLE)_beginthreadex(NULL, 0, SoundStreamingThreadProc, NULL, 0, NULL);
	if (s_waveOut.hThread == NULL) {
		SoundUnprepare();
		SoundDeleteSoundOutputBuffer();
		DeleteCriticalSection(&s_waveOut.criticalSection);
		return false;
	}
	SetThreadPriority(s_waveOut.hThread, THREAD_PRIORITY_TIME_CRITICAL);

	return true;
}

bool SoundTerminate(){
	AtomicSet32(&s_waveOut.atomicHasFinished, 1);
	if (WaitForSingleObject(s_waveOut.hThread, INFINITE) != WAIT_OBJECT_0) {
		printf("SoundInitialize: WaitForSingleObject failed.\n");
	}

	SoundUnprepare();
	if (s_synthRingBuffer.glSync != 0) {
		glDeleteSync(s_synthRingBuffer.glSync);
	}
	SoundDeleteSoundOutputBuffer();
	SoundDeleteShader(s_soundShaderId);
	WasapiTerminate();

	DeleteCriticalSection(&s_waveOut.criticalSection);

	return true;
}
