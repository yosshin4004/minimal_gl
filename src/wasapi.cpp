/* Copyright (C) 2026 Yosshin(@yosshin4004) */

#include "config.h"
#include "wasapi.h"
#include <windows.h>
#include <assert.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <stdio.h>

#pragma comment(lib, "ole32.lib")

static IAudioClient *s_pAudioClient = NULL;
static IAudioRenderClient *s_pRenderClient = NULL;
static IAudioClock *s_pAudioClock = NULL;
static HANDLE s_hEvent = NULL;
static int s_numChannels = 0;
static int s_numSamplesPerSec = 0;
static uint32_t s_bufferFrameCount = 0;

bool WasapiInitialize(int numSamplesPerSec, int numChannels) {
	HRESULT hr;
	IMMDeviceEnumerator *pEnumerator = NULL;
	IMMDevice *pDevice = NULL;
	WAVEFORMATEXTENSIBLE wfx;

	s_numChannels = numChannels;
	s_numSamplesPerSec = numSamplesPerSec;

	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
		return false;
	}

	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
	if (FAILED(hr)) return false;

	hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
	pEnumerator->Release();
	if (FAILED(hr)) return false;

	hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&s_pAudioClient);
	pDevice->Release();
	if (FAILED(hr)) return false;

	/* フォーマットの設定 */
	memset(&wfx, 0, sizeof(wfx));
	wfx.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	wfx.Format.nChannels = numChannels;
	wfx.Format.nSamplesPerSec = numSamplesPerSec;
	wfx.Format.nAvgBytesPerSec = numSamplesPerSec * numChannels * sizeof(SOUND_SAMPLE_TYPE);
	wfx.Format.nBlockAlign = (WORD)(numChannels * sizeof(SOUND_SAMPLE_TYPE));
	wfx.Format.wBitsPerSample = sizeof(SOUND_SAMPLE_TYPE) * 8;
	wfx.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
	wfx.Samples.wValidBitsPerSample = sizeof(SOUND_SAMPLE_TYPE) * 8;
	wfx.dwChannelMask = (numChannels == 2) ? (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT) : SPEAKER_FRONT_LEFT;
	assert(SOUND_SAMPLE_TYPE_IS_FLOAT);
	wfx.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;

	/* 共有モード、イベントドリブンで初期化 */
	REFERENCE_TIME hnsRequestedDuration = 1000000; /* 100ms */
//	REFERENCE_TIME hnsRequestedDuration = 0;
	hr = s_pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, hnsRequestedDuration, 0, &wfx.Format, NULL);
	if (FAILED(hr)) {
		s_pAudioClient->Release();
		s_pAudioClient = NULL;
	return false;
	}

	s_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	hr = s_pAudioClient->SetEventHandle(s_hEvent);
	if (FAILED(hr)) goto error;

	hr = s_pAudioClient->GetBufferSize(&s_bufferFrameCount);
	if (FAILED(hr)) goto error;

	hr = s_pAudioClient->GetService(__uuidof(IAudioRenderClient), (void**)&s_pRenderClient);
	if (FAILED(hr)) goto error;

	hr = s_pAudioClient->GetService(__uuidof(IAudioClock), (void**)&s_pAudioClock);
	if (FAILED(hr)) goto error;

	hr = s_pAudioClient->Start();
	if (FAILED(hr)) goto error;

	return true;

error:
	WasapiTerminate();
	return false;
}

void WasapiTerminate() {
	if (s_pAudioClient) {
		s_pAudioClient->Stop();
	}
	if (s_pAudioClock) {
		s_pAudioClock->Release();
		s_pAudioClock = NULL;
	}
	if (s_pRenderClient) {
		s_pRenderClient->Release();
		s_pRenderClient = NULL;
	}
	if (s_pAudioClient) {
		s_pAudioClient->Release();
		s_pAudioClient = NULL;
	}
	if (s_hEvent) {
		CloseHandle(s_hEvent);
		s_hEvent = NULL;
	}
	CoUninitialize();
}

void WasapiPause() {
	if (s_pAudioClient) s_pAudioClient->Stop();
}

void WasapiResume() {
	if (s_pAudioClient) s_pAudioClient->Start();
}

void WasapiReset() {
	if (s_pAudioClient) {
		s_pAudioClient->Stop();
		s_pAudioClient->Reset();
		s_pAudioClient->Start();
	}
}

uint32_t WasapiGetPosition() {
	if (!s_pAudioClock) return 0;
	UINT64 pos, freq;
	if (SUCCEEDED(s_pAudioClock->GetPosition(&pos, NULL)) && SUCCEEDED(s_pAudioClock->GetFrequency(&freq))) {
		return (uint32_t)(pos * s_numSamplesPerSec / freq);
	}
	return 0;
}

uint32_t WasapiGetAvailableFrames() {
	if (!s_pAudioClient) return 0;
	UINT32 padding;
	if (FAILED(s_pAudioClient->GetCurrentPadding(&padding))) return 0;
	return s_bufferFrameCount - padding;
}

bool WasapiWaitEvent(uint32_t timeoutMs) {
	if (!s_hEvent) return false;
	return WaitForSingleObject(s_hEvent, timeoutMs) == WAIT_OBJECT_0;
}

bool WasapiWrite(const SOUND_SAMPLE_TYPE *samples, uint32_t numFrames) {
	if (!s_pRenderClient) return false;
	if (numFrames == 0) return true;

	BYTE *pData;
	HRESULT hr = s_pRenderClient->GetBuffer(numFrames, &pData);
	if (FAILED(hr)) return false;

	memcpy(pData, samples, numFrames * s_numChannels * sizeof(SOUND_SAMPLE_TYPE));

	hr = s_pRenderClient->ReleaseBuffer(numFrames, 0);
	return SUCCEEDED(hr);
}

