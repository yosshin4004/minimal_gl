/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#include "config.h"
#include "common.h"
#include "app.h"
#include "sound.h"
#include "glext_util.h"
#include "wav_util.h"


#define BUFFER_INDEX_FOR_SOUND_OUTPUT	(0)

#define NUM_SOUND_BUFFERS				(4)

static bool s_paused = false;
static GLuint s_soundProgramId = 0;
/*
	s_soundTempSsbos は、GPU と CPU で同一バッファをアクセスしないよう、複数
	バッファに分離したもの。使用位置は不連続である。
	s_soundOutputSsbo はグラフィクスシェーダに見せるためシンセサイズ結果を
	連続領域に作成するためのもの。
*/
static GLuint s_soundTempSsbos[NUM_SOUND_BUFFERS] = {0};
static GLuint s_soundOutputSsbo = 0;
static bool s_soundShaderSetupSucceeded = false;


static uint8_t s_soundBufferPartitionDirty[NUM_SOUND_BUFFER_PARTITIONS];
static SOUND_SAMPLE_TYPE s_soundBuffer[NUM_SOUND_BUFFER_SAMPLES * NUM_SOUND_CHANNELS];
static HWAVEOUT s_waveOutHandle = 0;
static uint32_t s_waveOutOffset = 0;

static const WAVEFORMATEX s_waveFormat = {
	/* WORD  wFormatTag */
#if SOUND_SAMPLE_TYPE_IS_FLOAT
								WAVE_FORMAT_IEEE_FLOAT,
#else
								WAVE_FORMAT_PCM,
#endif
	/* WORD  nChannels */		NUM_SOUND_CHANNELS,
	/* DWORD nSamplesPerSec */	NUM_SOUND_SAMPLES_PER_SEC,
	/* DWORD nAvgBytesPerSec */	NUM_SOUND_SAMPLES_PER_SEC * sizeof(SOUND_SAMPLE_TYPE) * NUM_SOUND_CHANNELS,
	/* WORD  nBlockAlign */		sizeof(SOUND_SAMPLE_TYPE) * NUM_SOUND_CHANNELS,
	/* WORD  wBitsPerSample */	sizeof(SOUND_SAMPLE_TYPE) * 8,
	/* WORD  cbSize */			0	/* extension not needed */
};

static WAVEHDR s_waveHeader = {
	/* LPSTR lpData */					(LPSTR)s_soundBuffer,
	/* DWORD dwBufferLength */			NUM_SOUND_BUFFER_SAMPLES * sizeof(SOUND_SAMPLE_TYPE) * NUM_SOUND_CHANNELS,
	/* DWORD dwBytesRecorded */			0,
	/* DWORD dwUser */					0,
	/* DWORD dwFlags */					0,
	/* DWORD dwLoops */					0,
	/* struct wavehdr_tag* lpNext */	NULL,
	/* DWORD reserved */				0
};

static MMTIME s_mmTime = {
	TIME_SAMPLES,	/* win32 SDK で定義された定数 */
	0
};

/*=============================================================================
▼	サウンド合成関連
-----------------------------------------------------------------------------*/
static void SoundSynthesizePartition(
	int partitionIndex
){
	if (s_soundProgramId == 0) return;

	if (0 <= partitionIndex && partitionIndex < NUM_SOUND_BUFFER_PARTITIONS) {
		/* 指定のパーティションが dirty なら処理 */
		if (s_soundBufferPartitionDirty[partitionIndex] != 0) {
			s_soundBufferPartitionDirty[partitionIndex] = 0;
//			printf("SoundSynthesizePartition #%d dirty\n", partitionIndex);

			/* シェーダをバインド */
			assert(s_soundProgramId != 0);
			glExtUseProgram(s_soundProgramId);

			/* 出力先バッファの指定 */
			glExtBindBufferBase(
				/* GLenum target */	GL_SHADER_STORAGE_BUFFER,
				/* GLuint index */	BUFFER_INDEX_FOR_SOUND_OUTPUT,
				/* GLuint buffer */	s_soundTempSsbos[partitionIndex % NUM_SOUND_BUFFERS]
			);

			/* ユニフォームパラメータの設定 */
			if (ExistsShaderUniform(s_soundProgramId, UNIFORM_LOCATION_WAVE_OUT_POS, GL_INT)) {
				glExtUniform1i(
					UNIFORM_LOCATION_WAVE_OUT_POS,
					NUM_SOUND_BUFFER_SAMPLES_PER_DISPATCH * partitionIndex
				);
			}

			/* エラーチェック */
			CheckGlError("SoundUpdate : pre dispatch");

			/* コンピュートシェーダによるサウンド生成 */
			glExtDispatchCompute(NUM_SOUND_BUFFER_SAMPLES_PER_DISPATCH, 1, 1);

			/* エラーチェック */
			CheckGlError("SoundUpdate : post dispatch");

			/* アンバインド */
			glExtBindBufferBase(
				/* GLenum target */	GL_SHADER_STORAGE_BUFFER,
				/* GLuint index */	BUFFER_INDEX_FOR_SOUND_OUTPUT,
				/* GLuint buffer */	0
			);

			/* シェーダをアンバインド */
			glExtUseProgram(NULL);
		}
	}
}

static void SoundGetSynthesizePartitionResult(
	int partitionIndex
){
	if (0 <= partitionIndex && partitionIndex < NUM_SOUND_BUFFER_PARTITIONS) {
		size_t partitionSizeInBytes = NUM_SOUND_BUFFER_SAMPLES_PER_DISPATCH * sizeof(SOUND_SAMPLE_TYPE) * NUM_SOUND_CHANNELS;

		/* サウンド生成結果の取り出し */
		glExtBindBufferBase(
			/* GLenum target */		GL_SHADER_STORAGE_BUFFER,
			/* GLuint index */		BUFFER_INDEX_FOR_SOUND_OUTPUT,
			/* GLuint buffer */		s_soundTempSsbos[partitionIndex % NUM_SOUND_BUFFERS]
		);
		glExtGetBufferSubData(
			/* GLenum target */		GL_SHADER_STORAGE_BUFFER,
			/* GLintptr OFFSET */	partitionSizeInBytes * partitionIndex,
			/* GLsizeiptr size */	partitionSizeInBytes,
			/* GLvoid *data */		(void *)((uintptr_t)s_soundBuffer + partitionSizeInBytes * partitionIndex)
		);

		/* 生成結果のコピー */
		glExtCopyNamedBufferSubData(
			/* GLuint readBuffer */		s_soundTempSsbos[partitionIndex % NUM_SOUND_BUFFERS],
			/* GLuint writeBuffer */	s_soundOutputSsbo,
			/* GLintptr readOffset */	partitionSizeInBytes * partitionIndex,
			/* GLintptr writeOffset */	partitionSizeInBytes * partitionIndex,
			/* GLsizeiptr size */		partitionSizeInBytes
		);
	}
}

bool SoundCreateShader(
	const char *shaderCode
){
	printf("setup sound shader ...\n");
	const GLchar *(strings[]) = {
		SkipBomConst(shaderCode)
	};
	assert(s_soundProgramId == 0);
	s_soundProgramId = CreateShader(GL_COMPUTE_SHADER, SIZE_OF_ARRAY(strings), strings);
	if (s_soundProgramId == 0) {
		printf("setup sound shader ... fialed.\n");
		return false;
	}
	DumpShaderInterfaces(s_soundProgramId);
	printf("setup sound shader ... done.\n");
	SoundClearOutputBuffer();
	return true;
}

bool SoundDeleteShader(){
	if (s_soundProgramId == 0) return false;
	glExtDeleteProgram(s_soundProgramId);
	s_soundProgramId = 0;
	return true;
}

float SoundDetectDurationInSeconds(){
	/* 有効なサンプルの末端位置を求める */
	int numAvailableSamples = 0;
	{
		for (int iSample = 0; iSample < NUM_SOUND_BUFFER_SAMPLES; iSample++) {
			for (int iChannel = 0; iChannel < NUM_SOUND_CHANNELS; iChannel++) {
				if (s_soundBuffer[iSample * NUM_SOUND_CHANNELS + iChannel] != 0) {
					numAvailableSamples = iSample + 1;
				}
			}
		}
	}

	/* 秒数に置き換える */
	return (float)numAvailableSamples / (float)NUM_SOUND_SAMPLES_PER_SEC;
}

static bool SoundCreateSoundOutputBuffer(
){
	size_t bufferSizeInBytes = NUM_SOUND_BUFFER_SAMPLES * sizeof(SOUND_SAMPLE_TYPE) * NUM_SOUND_CHANNELS;
	for (int i = 0; i < NUM_SOUND_BUFFERS; i++) {
		glExtGenBuffers(
			/* GLsizei n */				1,
			/* GLuint * buffers */		&s_soundTempSsbos[i]
		);
		glExtBindBuffer(
			/* GLenum target */			GL_SHADER_STORAGE_BUFFER,
			/* GLuint buffer */			s_soundTempSsbos[i]
		);
		glExtBufferData(
			/* GLenum target */			GL_SHADER_STORAGE_BUFFER,
			/* GLsizeiptr size */		bufferSizeInBytes,
			/* const GLvoid * data */	NULL,
			/* GLenum usage */			GL_DYNAMIC_COPY
		);
		glExtBindBuffer(
			/* GLenum target */			GL_SHADER_STORAGE_BUFFER,
			/* GLuint buffer */			0	/* unbind */
		);
	}
	{
		glExtGenBuffers(
			/* GLsizei n */				1,
			/* GLuint * buffers */		&s_soundOutputSsbo
		);
		glExtBindBuffer(
			/* GLenum target */			GL_SHADER_STORAGE_BUFFER,
			/* GLuint buffer */			s_soundOutputSsbo
		);
		glExtBufferData(
			/* GLenum target */			GL_SHADER_STORAGE_BUFFER,
			/* GLsizeiptr size */		bufferSizeInBytes,
			/* const GLvoid * data */	NULL,
			/* GLenum usage */			GL_DYNAMIC_COPY
		);
		glExtBindBuffer(
			/* GLenum target */			GL_SHADER_STORAGE_BUFFER,
			/* GLuint buffer */			0	/* unbind */
		);
	}
	return true;
}

static bool SoundDeleteSoundOutputBuffer(
){
	glExtDeleteBuffers(
		/* GLsizei n */			1,
		/* GLuint * buffers */	&s_soundOutputSsbo
	);
	s_soundOutputSsbo = 0;
	for (int i = 0; i < NUM_SOUND_BUFFERS; i++) {
		glExtDeleteBuffers(
			/* GLsizei n */			1,
			/* GLuint * buffers */	&s_soundTempSsbos[i]
		);
		s_soundTempSsbos[i] = 0;
	}
	return true;
}

void SoundClearOutputBuffer(
){
	size_t sizeInBytes = NUM_SOUND_BUFFER_SAMPLES * sizeof(SOUND_SAMPLE_TYPE) * NUM_SOUND_CHANNELS;
	memset(s_soundBuffer, 0, sizeInBytes);
	memset(s_soundBufferPartitionDirty, 0xFF, sizeof(s_soundBufferPartitionDirty));
}

GLuint SoundGetOutputSsbo(
){
	return s_soundOutputSsbo;
}

/*=============================================================================
▼	サウンドキャプチャ関連
-----------------------------------------------------------------------------*/
bool SoundCaptureSound(
	const CaptureSoundSettings *settings
){
	int numSamples = (int)(settings->durationInSeconds * NUM_SOUND_SAMPLES_PER_SEC);
	if (numSamples < 0) numSamples = 0;
	if (numSamples >= NUM_SOUND_BUFFER_SAMPLES) numSamples = NUM_SOUND_BUFFER_SAMPLES - 1;
	return SerializeAsWav(
		/* const char *fileName */			settings->fileName,
		/* const void *buffer */			s_soundBuffer,
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
void SoundPauseWaveOut(){
	waveOutPause(s_waveOutHandle);
	s_paused = true;
}

void SoundResumeWaveOut(){
	waveOutRestart(s_waveOutHandle);
	s_paused = false;
}

void SoundRestartWaveOut(){
	s_paused = false;
	SoundSeekWaveOut(0);
}

void SoundDisposePreSynthesizedCache(){
	uint32_t offset = SoundGetWaveOutPos();

	/* 再生位置のサウンド生成（2 パーティション先まで）*/
	{
		int partitionIndex = offset / NUM_SOUND_BUFFER_SAMPLES_PER_DISPATCH;
		SoundSynthesizePartition(partitionIndex);
		SoundSynthesizePartition((partitionIndex + 1) % NUM_SOUND_BUFFER_PARTITIONS);
		SoundSynthesizePartition((partitionIndex + 2) % NUM_SOUND_BUFFER_PARTITIONS);
		glFinish();
		SoundGetSynthesizePartitionResult(partitionIndex);
		SoundGetSynthesizePartitionResult((partitionIndex + 1) % NUM_SOUND_BUFFER_PARTITIONS);
	}
}

void SoundSeekWaveOut(uint32_t offset){
	s_waveOutOffset = offset;

	MMRESULT ret = waveOutReset(s_waveOutHandle);

	SoundDisposePreSynthesizedCache();

	if (offset > NUM_SOUND_BUFFER_SAMPLES) offset = NUM_SOUND_BUFFER_SAMPLES;
	s_waveHeader.lpData = (LPSTR)&s_soundBuffer[offset * NUM_SOUND_CHANNELS];
	s_waveHeader.dwBufferLength = (NUM_SOUND_BUFFER_SAMPLES - offset) * sizeof(SOUND_SAMPLE_TYPE) * NUM_SOUND_CHANNELS;

	assert(ret == MMSYSERR_NOERROR);
	ret = waveOutPrepareHeader(
		/* HWAVEOUT hwo */	s_waveOutHandle,
		/* LPWAVEHDR pwh */	&s_waveHeader,
		/* UINT cbwh */		sizeof(s_waveHeader)
	);
	assert(ret == MMSYSERR_NOERROR);
	ret = waveOutWrite(
		/* HWAVEOUT hwo */	s_waveOutHandle,
		/* LPWAVEHDR pwh */	&s_waveHeader,
		/* UINT cbwh */		sizeof(s_waveHeader)
	);
	assert(ret == MMSYSERR_NOERROR);
	if (s_paused) {
		waveOutPause(s_waveOutHandle);
	} else {
		ret = waveOutRestart(s_waveOutHandle);
		assert(ret == MMSYSERR_NOERROR);
	}
}

int SoundGetWaveOutPos(
){
	MMRESULT ret = waveOutGetPosition(s_waveOutHandle, &s_mmTime, sizeof(MMTIME));
	if (ret != MMSYSERR_NOERROR) {
		AppErrorMessageBox(APP_NAME, "waveOutGetPosition() failed. ret = %08X", ret);
		return 0;
	}
	return s_mmTime.u.sample + s_waveOutOffset;
}


/*=============================================================================
▼	サウンド関連
-----------------------------------------------------------------------------*/
void SoundUpdate(
){
	int waveOutPos = SoundGetWaveOutPos();
	int partitionIndex = waveOutPos / NUM_SOUND_BUFFER_SAMPLES_PER_DISPATCH;

	/* 3 パーティション先のサウンド生成をリクエスト */
	SoundSynthesizePartition((partitionIndex + 3) % NUM_SOUND_BUFFER_PARTITIONS);

	/* 現在と次のパーティションのサウンド生成結果を取り出す */
	SoundGetSynthesizePartitionResult(partitionIndex);
	SoundGetSynthesizePartitionResult((partitionIndex + 1) % NUM_SOUND_BUFFER_PARTITIONS);
}

bool SoundInitialize(
){
	SoundCreateSoundOutputBuffer();
	SoundClearOutputBuffer();

	MMRESULT ret = waveOutOpen(
		/* LPHWAVEOUT phwo */			&s_waveOutHandle,
		/* UINT uDeviceID */			WAVE_MAPPER,
		/* LPWAVEFORMATEX pwfx */		&s_waveFormat,
		/* DWORD dwCallback */			NULL,
		/* DWORD dwCallbackInstance */	0,
		/* DWORD fdwOpen */				CALLBACK_NULL
	);
	if (ret != MMSYSERR_NOERROR) return false;

	return true;
}

bool SoundTerminate(
){
	waveOutReset(s_waveOutHandle);
	SoundDeleteSoundOutputBuffer();
	SoundDeleteShader();

	MMRESULT ret = waveOutClose(s_waveOutHandle);
	if (ret != MMSYSERR_NOERROR) return false;

	return true;
}
