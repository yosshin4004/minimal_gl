/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#include "config.h"
#include "common.h"
#include "app.h"
#include "sound.h"
#include "glext_util.h"
#include "wav_util.h"


#define BUFFER_INDEX_FOR_SOUND_OUTPUT	(0)

static bool s_paused = false;
static GLuint s_soundProgramId = 0;
static GLuint s_soundOutputSsbo = 0;
static bool s_soundShaderSetupSucceeded = false;


static uint8_t s_soundBufferPartitionDirty[NUM_SOUND_BUFFER_PARTITIONS];
static SAMPLE_TYPE *s_soundBuffer = NULL;
static HWAVEOUT s_waveOutHandle = 0;
static uint32_t s_waveOutOffset = 0;

static const WAVEFORMATEX s_waveFormat = {
	/* WORD  wFormatTag */
#if SAMPLE_TYPE_IS_FLOAT
								WAVE_FORMAT_IEEE_FLOAT,
#else
								WAVE_FORMAT_PCM,
#endif
	/* WORD  nChannels */		NUM_SOUND_CHANNELS,
	/* DWORD nSamplesPerSec */	NUM_SAMPLES_PER_SEC,
	/* DWORD nAvgBytesPerSec */	NUM_SAMPLES_PER_SEC * sizeof(SAMPLE_TYPE) * NUM_SOUND_CHANNELS,
	/* WORD  nBlockAlign */		sizeof(SAMPLE_TYPE) * NUM_SOUND_CHANNELS,
	/* WORD  wBitsPerSample */	sizeof(SAMPLE_TYPE) * 8,
	/* WORD  cbSize */			0	/* extension not needed */
};

static WAVEHDR s_waveHeader = {
	/* LPSTR lpData */					NULL,
	/* DWORD dwBufferLength */			NUM_SOUND_BUFFER_SAMPLES * sizeof(SAMPLE_TYPE) * NUM_SOUND_CHANNELS,
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
				/* GLuint buffer */	s_soundOutputSsbo
			);

			/* ユニフォームパラメータの設定 */
			if (ExistsShaderUniform(s_soundProgramId, UNIFORM_LOCATION_WAVE_OUT_POS, GL_INT)) {
				glExtUniform1i(
					UNIFORM_LOCATION_WAVE_OUT_POS,
					NUM_SAMPLES_PER_SOUND_BUFFER_PARTITION * partitionIndex
				);
			}

			/* エラーチェック */
			CheckGlError("SoundUpdate : pre dispatch");

			/* コンピュートシェーダによるサウンド生成 */
			glExtDispatchCompute(NUM_SAMPLES_PER_SOUND_BUFFER_PARTITION, 1, 1);

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
	return (float)numAvailableSamples / (float)NUM_SAMPLES_PER_SEC;
}

static bool SoundCreateSoundOutputBuffer(
){
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
		/* GLsizeiptr size */		NUM_SOUND_BUFFER_SAMPLES * NUM_SOUND_CHANNELS * sizeof(SAMPLE_TYPE),
		/* const GLvoid * data */	NULL,
		/* GLenum usage */			GL_DYNAMIC_COPY	/* DYNAMIC_COPY じゃないと GPU 上のデータが直接見えない */
	);
	s_soundBuffer = (SAMPLE_TYPE *)glExtMapBuffer(
		/* GLenum target */			GL_SHADER_STORAGE_BUFFER,
		/* GLenum access */			GL_READ_WRITE
	);
	glExtBindBuffer(
		/* GLenum target */			GL_SHADER_STORAGE_BUFFER,
		/* GLuint buffer */			0	/* unbind */
	);
	s_waveHeader.lpData = (LPSTR)s_soundBuffer;
	return true;
}

static bool SoundDeleteSoundOutputBuffer(
){
	if (s_soundOutputSsbo == 0) return false;
	glExtUnmapBuffer(
		/* GLenum target */		GL_SHADER_STORAGE_BUFFER
	);
	glExtDeleteBuffers(
		/* GLsizei n */			1,
		/* GLuint * buffers */	&s_soundOutputSsbo
	);
	s_soundOutputSsbo = 0;
	return true;
}

void SoundClearOutputBuffer(
){
	size_t sizeInBytes = NUM_SOUND_BUFFER_SAMPLES * sizeof(SAMPLE_TYPE) * NUM_SOUND_CHANNELS;
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
	int numSamples = (int)(settings->durationInSeconds * NUM_SAMPLES_PER_SEC);
	if (numSamples < 0) numSamples = 0;
	if (numSamples >= NUM_SOUND_BUFFER_SAMPLES) numSamples = NUM_SOUND_BUFFER_SAMPLES - 1;
	return SerializeAsWav(
		/* const char *fileName */			settings->fileName,
		/* const void *buffer */			s_soundBuffer,
		/* int numChannels */				NUM_SOUND_CHANNELS,
		/* int numSamples */				numSamples,
		/* int numSamplesPerSec */			NUM_SAMPLES_PER_SEC,
		/* uint16_t formatID */
#if SAMPLE_TYPE_IS_FLOAT
											WAVE_FORMAT_IEEE_FLOAT,
#else
											WAVE_FORMAT_PCM,
#endif
		/* int bitsPerSampleComponent */	sizeof(SAMPLE_TYPE) * 8
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

void SoundSeekWaveOut(uint32_t offset){
	s_waveOutOffset = offset;

	MMRESULT ret = waveOutReset(s_waveOutHandle);

	/* 現在と次のパーティションまでサウンド生成 */
	{
		int partitionIndex = offset / NUM_SAMPLES_PER_SOUND_BUFFER_PARTITION;
		SoundSynthesizePartition(partitionIndex);
		SoundSynthesizePartition((partitionIndex + 1) % NUM_SOUND_BUFFER_PARTITIONS);
	}

	if (offset > NUM_SOUND_BUFFER_SAMPLES) offset = NUM_SOUND_BUFFER_SAMPLES;
	s_waveHeader.lpData = (LPSTR)&s_soundBuffer[offset * NUM_SOUND_CHANNELS];
	s_waveHeader.dwBufferLength = (NUM_SOUND_BUFFER_SAMPLES - offset) * sizeof(SAMPLE_TYPE) * NUM_SOUND_CHANNELS;

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
	int partitionIndex = waveOutPos / NUM_SAMPLES_PER_SOUND_BUFFER_PARTITION;

	/* 現在と次のパーティションまでサウンド生成 */
	SoundSynthesizePartition(partitionIndex);
	SoundSynthesizePartition((partitionIndex + 1) % NUM_SOUND_BUFFER_PARTITIONS);
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
