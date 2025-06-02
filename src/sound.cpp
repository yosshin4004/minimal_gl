/* Copyright (C) 2018 Yosshin(@yosshin4004) */
#include "config.h"
#include "common.h"
#include "app.h"
#include "sound.h"
#include "wav_util.h"


#define BUFFER_INDEX_FOR_SOUND_OUTPUT			(0)

#define NUM_SOUND_BUFFERS						(4)

/*
	原因は不明だが、再生開始時に冒頭 256 サンプルほどが正しく音声出力されない
	場合がある。この問題を回避するため、以下の定数で指定したサンプル数を
	スキップした位置からサウンド生成を行う。
	サウンド保存時には、このサンプルスキップはキャンセルされる。
*/
#define NUM_SOUND_MARGIN_SAMPLES				(0x100)

static bool s_paused = false;
static GLuint s_soundShaderId = 0;
static GLuint s_soundOutputSsbo = 0;
static SOUND_SAMPLE_TYPE *s_mappedSoundOutputSsbo = NULL;


static int s_soundCurrentPartitionIndex = 0;
static int s_soundSynthesizePartitionIndex = 0;
typedef enum {
	PartitionState_ZeroCleared,
	PartitionState_Copied,
	PartitionState_Synthesized,
} PartitionState;
static PartitionState s_soundBufferPartitionStates[NUM_SOUND_BUFFER_PARTITIONS] = {(PartitionState)0};
static uint32_t s_soundBufferPartitionSynthesizedFrameCount[NUM_SOUND_BUFFER_PARTITIONS] = {0};
static SOUND_SAMPLE_TYPE s_soundBuffer[(NUM_SOUND_BUFFER_SAMPLES + NUM_SOUND_MARGIN_SAMPLES) * NUM_SOUND_CHANNELS];
static HWAVEOUT s_waveOutHandle = 0;
static uint32_t s_waveOutOffset = 0;

/* waveout 関連 */
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

/* 再生位置取得用 */
static MMTIME s_mmTime = {
	TIME_SAMPLES,	/* win32 SDK で定義された定数 */
	{0}
};

/*=============================================================================
▼	サウンド合成関連
-----------------------------------------------------------------------------*/
static void SoundInvalidatePreSynthesizedCache(){
	s_soundSynthesizePartitionIndex = s_soundCurrentPartitionIndex;
}

static void SoundSynthesizePartition(
	int partitionIndex,
	uint32_t frameCount
){
	if (s_soundShaderId == 0) return;

	if (0 <= partitionIndex && partitionIndex < NUM_SOUND_BUFFER_PARTITIONS) {
		/* サウンド合成したフレームカウントの保存 */
		s_soundBufferPartitionSynthesizedFrameCount[partitionIndex] = frameCount;

		/* 指定のパーティションが ZeroCleared なら処理 */
		if (s_soundBufferPartitionStates[partitionIndex] == PartitionState_ZeroCleared) {
			s_soundBufferPartitionStates[partitionIndex] = PartitionState_Synthesized;
//			printf("SoundSynthesizePartition #%d\n", partitionIndex);

			/* シェーダをバインド */
			assert(s_soundShaderId != 0);
			glUseProgram(s_soundShaderId);

			/* 出力先バッファの指定 */
			glBindBufferBase(
				/* GLenum target */	GL_SHADER_STORAGE_BUFFER,
				/* GLuint index */	BUFFER_INDEX_FOR_SOUND_OUTPUT,
				/* GLuint buffer */	s_soundOutputSsbo
			);

			/* ユニフォームパラメータの設定 */
			if (ExistsShaderUniform(s_soundShaderId, UNIFORM_LOCATION_WAVE_OUT_POS, GL_INT)) {
				glUniform1i(
					UNIFORM_LOCATION_WAVE_OUT_POS,
					NUM_SOUND_BUFFER_SAMPLES_PER_DISPATCH * partitionIndex
				);
			}

			/* エラーチェック */
			CheckGlError("SoundUpdate : pre dispatch");

			/* コンピュートシェーダによるサウンド生成 */
			glDispatchCompute(NUM_SOUND_BUFFER_SAMPLES_PER_DISPATCH, 1, 1);

			/* エラーチェック */
			CheckGlError("SoundUpdate : post dispatch");

			/* アンバインド */
			glBindBufferBase(
				/* GLenum target */	GL_SHADER_STORAGE_BUFFER,
				/* GLuint index */	BUFFER_INDEX_FOR_SOUND_OUTPUT,
				/* GLuint buffer */	0	/* unbind */
			);

			/* シェーダをアンバインド */
			glUseProgram(NULL);
		}
	}
}

static void SoundGetSynthesizedPartitionResult(
	int partitionIndex,
	uint32_t frameCount
){
	if (0 <= partitionIndex && partitionIndex < NUM_SOUND_BUFFER_PARTITIONS) {
		if (s_soundBufferPartitionStates[partitionIndex] == PartitionState_Synthesized) {
			s_soundBufferPartitionStates[partitionIndex] = PartitionState_Copied;
//			printf("SoundCopyPartition #%d (synthesized %d frames ago.) \n", partitionIndex, frameCount - s_soundBufferPartitionSynthesizedFrameCount[partitionIndex]);
			size_t partitionSizeInBytes = NUM_SOUND_BUFFER_SAMPLES_PER_DISPATCH * sizeof(SOUND_SAMPLE_TYPE) * NUM_SOUND_CHANNELS;

			/* サウンド生成リクエストから十分なフレーム数が経過していないなら dispatch 完了待ち */
			if (s_soundBufferPartitionSynthesizedFrameCount[partitionIndex] + (NUM_SOUND_BUFFERS - 1) >= frameCount) {
				glFinish();
			}

			/*
				生成結果のコピー
				持続的 map を使うのでコピーしなくともそのまま waveout は可能だが、
				マージンを考慮するためコピーが必要
			*/
			memcpy(
				(void *)(
					(uintptr_t)s_soundBuffer + partitionSizeInBytes * partitionIndex
					/* マージンをスキップ */
				+	NUM_SOUND_MARGIN_SAMPLES * sizeof(SOUND_SAMPLE_TYPE) * NUM_SOUND_CHANNELS
				),
				(const void *)((uintptr_t)s_mappedSoundOutputSsbo + partitionSizeInBytes * partitionIndex),
				partitionSizeInBytes
			);
		}
	}
}

bool SoundCreateShader(
	const char *shaderCode
){
	printf("setup the sound shader ...\n");
	const GLchar *(strings[]) = {
		SkipBomConst(shaderCode)
	};
	assert(s_soundShaderId == 0);
	s_soundShaderId = CreateShader(GL_COMPUTE_SHADER, SIZE_OF_ARRAY(strings), strings);
	if (s_soundShaderId == 0) {
		printf("setup the sound shader ... fialed.\n");
		return false;
	}
	DumpShaderInterfaces(s_soundShaderId);
	printf("setup the sound shader ... done.\n");
	return true;
}

bool SoundDeleteShader(){
	if (s_soundShaderId == 0) return false;
	glFinish();
	glDeleteProgram(s_soundShaderId);
	s_soundShaderId = 0;
	return true;
}

float SoundDetectDurationInSeconds(){
	/* 有効なサンプルの末端位置を求める */
	int numAvailableSamples = 0;
	{
		for (int iSample = 0; iSample < NUM_SOUND_BUFFER_SAMPLES; iSample++) {
			for (int iChannel = 0; iChannel < NUM_SOUND_CHANNELS; iChannel++) {
				if (s_soundBuffer[(iSample + NUM_SOUND_MARGIN_SAMPLES) * NUM_SOUND_CHANNELS + iChannel] != 0) {
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
		/* GLsizeiptr size */		bufferSizeInBytes,
		/* const void * data */		NULL,
		/* GLbitfield flags */		GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT
	);
	s_mappedSoundOutputSsbo = (SOUND_SAMPLE_TYPE *)glMapBuffer(
		/* GLenum target */			GL_SHADER_STORAGE_BUFFER,
		/* GLenum access */			GL_READ_WRITE
	);
	assert(s_mappedSoundOutputSsbo != NULL);
	glBindBuffer(
		/* GLenum target */			GL_SHADER_STORAGE_BUFFER,
		/* GLuint buffer */			0	/* unbind */
	);
	return true;
}

static bool SoundDeleteSoundOutputBuffer(
){
	glDeleteBuffers(
		/* GLsizei n */			1,
		/* GLuint * buffers */	&s_soundOutputSsbo
	);
	s_soundOutputSsbo = 0;

	return true;
}

void SoundClearOutputBuffer(
){
	/*
		サウンドシェーダ更新によるリスタートが無効の場合は、サウンドシェーダ更新
		に伴うプチノイズ回避のため、出力バッファをクリアしない。
		副作用として、メッセージループ停止時（ウィドウドラッグ移動中）や、
		サウンド生成が間に合わない場合に、バッファ上の古いサウンドが再生されてしまう。
	*/
	if (AppPreferenceSettingsGetEnableAutoRestartBySoundShader()) {
		memset(s_soundBuffer, 0, sizeof(s_soundBuffer));
	}

	for (int i = 0; i < NUM_SOUND_BUFFER_PARTITIONS; i++) {
		s_soundBufferPartitionStates[i] = PartitionState_ZeroCleared;
	}
	SoundInvalidatePreSynthesizedCache();
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
		/* const void *buffer */			(const void *)(
												(uintptr_t)s_soundBuffer
												/* マージンをスキップ */
											+	NUM_SOUND_MARGIN_SAMPLES * sizeof(SOUND_SAMPLE_TYPE) * NUM_SOUND_CHANNELS
											),
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

void SoundSeekWaveOut(uint32_t offset){
	s_waveOutOffset = offset;

	MMRESULT ret = waveOutReset(s_waveOutHandle);

	s_soundCurrentPartitionIndex = offset / NUM_SOUND_BUFFER_SAMPLES_PER_DISPATCH;
	SoundInvalidatePreSynthesizedCache();

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
	uint32_t frameCount
){
	int waveOutPos = SoundGetWaveOutPos();
	s_soundCurrentPartitionIndex = waveOutPos / NUM_SOUND_BUFFER_SAMPLES_PER_DISPATCH;

	/* 先行してシンセサイズするパーティションの終点 */
	int preSynthesizeEndPartitionIndex =
		(s_soundCurrentPartitionIndex + (NUM_SOUND_BUFFERS - 1)) % NUM_SOUND_BUFFER_PARTITIONS;

	/* 先行してシンセサイズするパーティションの終点までサウンド生成をリクエスト */
	while (s_soundSynthesizePartitionIndex != preSynthesizeEndPartitionIndex) {
		SoundSynthesizePartition(s_soundSynthesizePartitionIndex, frameCount);
		s_soundSynthesizePartitionIndex++;
		s_soundSynthesizePartitionIndex %= NUM_SOUND_BUFFER_PARTITIONS;
	}

	/* 現在と次のパーティションのサウンド生成結果を取り出す */
	SoundGetSynthesizedPartitionResult(s_soundCurrentPartitionIndex, frameCount);
	SoundGetSynthesizedPartitionResult((s_soundCurrentPartitionIndex + 1) % NUM_SOUND_BUFFER_PARTITIONS, frameCount);
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
