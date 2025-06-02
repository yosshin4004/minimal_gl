/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#include "config.h"
#include "common.h"
#include "app.h"
#include "graphics.h"
#include "png_util.h"
#include "tiny_vmath.h"
#include "record_image_sequence.h"
#include "dialog_confirm_over_write.h"
#include "resource/resource.h"
#include <process.h>
#include <winbase.h>


#define PROGRESS_BAR_MIN_VALUE 0
#define PROGRESS_BAR_MAX_VALUE 100

typedef enum {
	StateIdle,
	StateWorkInProgress,
	StateError,
	StateAborted,
	StateDone,
} State;
static volatile State s_state = StateIdle;

static LRESULT CALLBACK DialogFunc(
	HWND hDwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
){
	HWND hProg = GetDlgItem(hDwnd, IDC_PROGRESS_BAR);

	switch (uMsg) {
		/* ダイアログボックスの初期化 */
		case WM_INITDIALOG: {
			/* プログレスバーの最小値最大値を設定 */
			SendMessage(hProg, PBM_SETRANGE, (WPARAM)0, MAKELPARAM(PROGRESS_BAR_MIN_VALUE, PROGRESS_BAR_MAX_VALUE));

			/* メッセージは処理された */
			return 1;
		} break;

		/* UI 入力を検出 */
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				/* キャンセル */
				case IDCANCEL: {
					/* 中断するか？問い合わせ */
					if (AppYesNoMessageBox(APP_NAME, "Abort?")) {
						/* モードレスダイアログボックス終了 */
						DestroyWindow(hDwnd);

						/* 中断されたことをメインスレッドに伝える */
						s_state = StateAborted;
					}

					/* メッセージは処理された */
					return 1;
				} break;
			}
		} break;

		/* ユーザー定義のメッセージ : 進捗パーセンテージの設定 */
		case WM_APP: {
			int frameCount = (int)wParam;
			int numFrameCount = (int)lParam;
			float progress = 0.0f;
			if (numFrameCount != 0) {
				progress = (float)frameCount / (float)numFrameCount;
			}
			int pos = (int)(progress * 100);
			if (pos < PROGRESS_BAR_MIN_VALUE) pos = PROGRESS_BAR_MIN_VALUE;
			if (pos > PROGRESS_BAR_MAX_VALUE) pos = PROGRESS_BAR_MAX_VALUE;
			SendMessage(GetDlgItem(hDwnd, IDC_PROGRESS_BAR), PBM_SETPOS, pos, 0);

			char posAsString[0x100];
			snprintf(posAsString, sizeof(posAsString), "%d%% (%d / %d frames)", pos, frameCount, numFrameCount);
			SetWindowText(GetDlgItem(hDwnd, IDC_PROGRESS_BAR_CURRENT_STATUS), (LPCTSTR)posAsString);

			/* 連番画像生成が完了していたら終了 */
			if (frameCount == numFrameCount) {
				if (s_state == StateDone) {
					AppMessageBox(APP_NAME, "Completed.");
				} else
				if (s_state == StateError) {
					AppErrorMessageBox(APP_NAME, "Failed.");
				} else
				if (s_state == StateAborted) {
					AppErrorMessageBox(APP_NAME, "Aborted.");
				}

				/* モードレスダイアログボックス終了 */
				DestroyWindow(hDwnd);
			}

			/* メッセージは処理された */
			return 1;
		} break;
	}

	/* メッセージは処理されなかった */
	return 0;
}


struct Worker {
	HANDLE hThread;
};
struct Job {
	char fileName[MAX_PATH];
	void *image;
	const RecordImageSequenceSettings *settings;
};
static struct Queue {
	CRITICAL_SECTION criticalSection;
	HANDLE hSemaWritable;
	HANDLE hSemaReadable;
	int writeIndex;
	int readIndex;
	int numWorkers;
	Worker *workers;
	int numJobs;
	Job *jobs;
} s_queue;

static bool TryEnqueue(Job *job){
	DWORD result = WaitForSingleObject(s_queue.hSemaWritable, 0);			/* tryTake */
	if (result == WAIT_TIMEOUT) return false;
	EnterCriticalSection(&s_queue.criticalSection);
	{
		int index = s_queue.writeIndex++;
		index &= (s_queue.numJobs - 1);
		s_queue.jobs[index] = *job;
	}
	LeaveCriticalSection(&s_queue.criticalSection);
	ReleaseSemaphore(s_queue.hSemaReadable, 1, NULL);		/* post */
	return true;
}

static void Enqueue(Job *job){
	WaitForSingleObject(s_queue.hSemaWritable, INFINITE);	/* take */
	EnterCriticalSection(&s_queue.criticalSection);
	{
		int index = s_queue.writeIndex++;
		index &= (s_queue.numJobs - 1);
		s_queue.jobs[index] = *job;
	}
	LeaveCriticalSection(&s_queue.criticalSection);
	ReleaseSemaphore(s_queue.hSemaReadable, 1, NULL);		/* post */
}

static Job Dequeue(){
	Job job;
	WaitForSingleObject(s_queue.hSemaReadable, INFINITE);	/* take */
	EnterCriticalSection(&s_queue.criticalSection);
	{
		int index = s_queue.readIndex++;
		index &= (s_queue.numJobs - 1);
		job = s_queue.jobs[index];
	}
	LeaveCriticalSection(&s_queue.criticalSection);
	ReleaseSemaphore(s_queue.hSemaWritable, 1, NULL);		/* post */
	return job;
}

static unsigned __stdcall WorkerThreadProc(
	void	*pWork_
){
	bool error = false;
	for (;;) {
		Job job = Dequeue();
		if (job.image == NULL) break;	/* end mark 検出 */
		if (error == false) {
			printf("generate %s.\n", job.fileName);
			bool ret = SerializeAsPng(
				/* const char *fileName */	job.fileName,
				/* const void *data */		job.image,
				/* int numChannels */		4,
				/* int width */				job.settings->xReso,
				/* int height */			job.settings->yReso,
				/* bool verticalFlip */		true
			);
			free(job.image);
			if (ret == false) {
				printf("failed.\n");
				error = true;
				s_state = StateError;
			}
		}
	}
	return 0;
}

static bool QueueInitialize(int numWorkers, int numJobs){
	InitializeCriticalSection(&s_queue.criticalSection);

	s_queue.writeIndex = 0;
	s_queue.readIndex = 0;

	s_queue.numWorkers = numWorkers;
	s_queue.workers = (Worker *)malloc(sizeof(Worker) * numWorkers);
	if (s_queue.workers == NULL) return false;

	s_queue.numJobs = numJobs;
	s_queue.jobs = (Job *)malloc(sizeof(Job) * numJobs);
	if (s_queue.jobs == NULL) return false;

	s_queue.hSemaWritable = CreateSemaphore(NULL, numJobs, 0x7FFFFFFF, "semaWritable");
	s_queue.hSemaReadable = CreateSemaphore(NULL,       0, 0x7FFFFFFF, "semaReadable");
	if (s_queue.hSemaWritable == NULL) return false;
	if (s_queue.hSemaReadable == NULL) return false;

	for (int i = 0; i < numWorkers; i++) {
		s_queue.workers[i].hThread = (HANDLE)_beginthreadex(NULL, 0, WorkerThreadProc, NULL, 0, NULL);
		if (s_queue.workers[i].hThread == NULL) return false;
	}

	return true;
}

static bool QueueTerminate(){
	for (int i = 0; i < s_queue.numWorkers; i++) {
		Job job = {{0}};
		Enqueue(&job);	/* end mark */
	}

	for (int i = 0; i < s_queue.numWorkers; i++) {
		if (WaitForSingleObject(s_queue.workers[i].hThread, INFINITE) != WAIT_OBJECT_0) return false;
		if (CloseHandle(s_queue.workers[i].hThread) == FALSE) return false;
	}

	if (CloseHandle(s_queue.hSemaWritable) == FALSE) return false;
	if (CloseHandle(s_queue.hSemaReadable) == FALSE) return false;

	free(s_queue.jobs);
	free(s_queue.workers);

	DeleteCriticalSection(&s_queue.criticalSection);

	return true;
}

bool RecordImageSequence(
	const RenderSettings *renderSettings,
	const RecordImageSequenceSettings *recordImageSequenceSettings
){
	s_state = StateWorkInProgress;

	/* ディスクを大量に消費することを示唆し続行するか確認 */
	if (
		AppYesNoMessageBox(
			APP_NAME,
			"This process will consume a large amount of free disk space.\n"
			"Do you wish to continue?"
		) == false
	) {
		/* ユーザーの同意があるので正常終了扱い */
		return true;
	}

	/* プログレスバー表示 */
	HWND hDwnd = CreateDialog(
		AppGetCurrentInstance(),
		"PROGRESS_BAR",
		AppGetMainWindowHandle(),
		DialogFunc
	);

	/* 先だって全レンダーターゲットをクリア */
	AppClearAllRenderTargets();

	/* 連番画像の保存 */
	{
		/* キューの初期化 */
		{
			/* ワーカースレッド数は論理コア数の半分とする */
			SYSTEM_INFO systemInfo;
			GetSystemInfo(&systemInfo);
			int numWorkers = (systemInfo.dwNumberOfProcessors + 1) / 2;
			if (numWorkers == 0) numWorkers = 1;

			int numJobs = Pow2CeilAlign(numWorkers * 16);
			bool ret = QueueInitialize(numWorkers, numJobs);
			if (ret == false) {
				AppErrorMessageBox(APP_NAME, "QueueInitialize failed.");
			}
		}

		float startTime = AppRecordImageSequenceGetStartTimeInSeconds();
		float duration = AppRecordImageSequenceGetDurationInSeconds();
		float framesPerSecond = AppRecordImageSequenceGetFramesPerSecond();

		float fovYInRadians = AppCameraSettingsGetFovYInRadians();
		float mat4x4CameraInWorld[4][4];
		AppGetMat4x4CameraInWorld(mat4x4CameraInWorld);

		int numFrameCount = (int)(framesPerSecond * duration);
		for (int frameCount = 0; frameCount < numFrameCount && s_state == StateWorkInProgress; ++frameCount) {
			/* 進捗をダイアログボックスに送信 */
			SendMessage(hDwnd, WM_APP, frameCount, numFrameCount);
			UpdateWindow(hDwnd);

			/* 経過時間 */
			float time = startTime + (float)frameCount / framesPerSecond;

			/* サウンド再生位置 */
			int waveOutPos = (int)(time * NUM_SOUND_SAMPLES_PER_SEC);

			/* ジョブ作成 */
			Job job;
			{
				/* 設定 */
				job.settings = recordImageSequenceSettings;
				snprintf(
					job.fileName,
					sizeof(job.fileName),
					"%s\\%08d.png",
					recordImageSequenceSettings->directoryName,
					frameCount
				);

				/* 画像をキャプチャ */
				RenderSettings renderSettingsForceUnorm8 = *renderSettings;
				renderSettingsForceUnorm8.pixelFormat = PixelFormatUnorm8Rgba;
				CaptureScreenShotSettings captureSettings = {
					/* char fileName[MAX_PATH]; */	{0},
					/* int xReso; */				recordImageSequenceSettings->xReso,
					/* int yReso; */				recordImageSequenceSettings->yReso,
					/* bool replaceAlphaByOne; */	recordImageSequenceSettings->replaceAlphaByOne,
				};
				snprintf(captureSettings.fileName, sizeof(captureSettings.fileName), "%s", job.fileName);
				size_t numBitsPerPixel = PixelFormatToGlPixelFormatInfo(renderSettingsForceUnorm8.pixelFormat).numBitsPerPixel;
				size_t imageBufferSizeInBytes = (size_t)(recordImageSequenceSettings->xReso * recordImageSequenceSettings->yReso) * numBitsPerPixel / 8;
				job.image = malloc(imageBufferSizeInBytes);
				if (job.image == NULL) return false;

				CurrentFrameParams params = {0};
				params.waveOutPos				= waveOutPos;
				params.frameCount				= frameCount;
				params.time						= time;
				params.xMouse					= 0;
				params.yMouse					= 0;
				params.mouseLButtonPressed		= 0;
				params.mouseMButtonPressed		= 0;
				params.mouseRButtonPressed		= 0;
				params.xReso					= recordImageSequenceSettings->xReso;
				params.yReso					= recordImageSequenceSettings->yReso;
				params.fovYInRadians			= fovYInRadians;
				Mat4x4Copy(params.mat4x4CameraInWorld,		mat4x4CameraInWorld);
				Mat4x4Copy(params.mat4x4PrevCameraInWorld,	mat4x4CameraInWorld);
				GraphicsCaptureScreenShotOnMemory(
					job.image, imageBufferSizeInBytes,
					&params, &renderSettingsForceUnorm8, &captureSettings
				);
			}

			/* 上書き確認 */
			if (DialogConfirmOverWrite(job.fileName) == DialogConfirmOverWriteResult_Canceled) {
				s_state = StateAborted;
				break;
			}

			/* ジョブ投入に成功するまでリトライ */
			for (;;) {
				/*
					ダイアログボックスのメッセージ処理だけ行う。
					メインウィンドウは操作不能になる。
				*/
				MSG	msg;
				while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
					/*
						IsDialogMessage は、そのメッセージがダイアログ向けか確認し、
						もしそうなら、そのメッセージを処理する関数である。
						関数名からは、何らかの処理を行うようには見えないが、
						実際にはメッセージ処理が行われることに注意。
					*/
					IsDialogMessage(hDwnd, &msg);
				}

				/* ジョブ投入（イメージバッファはワーカースレッドが解放）*/
				bool ret = TryEnqueue(&job);
				if (ret) break;

				/* 他のスレッドに処理権を与える */
				Sleep(10);
			}
		}

		/* 正常終了なら StateDone に変更 */
		if (s_state == StateWorkInProgress) {
			s_state = StateDone;
		}

		/* キューの終了処理 */
		{
			bool ret = QueueTerminate();
			if (ret == false) {
				AppErrorMessageBox(APP_NAME, "QueueTerminate failed.");
			}
		}

		/* 進捗 100%（プログレスバー終了）*/
		SendMessage(hDwnd, WM_APP, numFrameCount, numFrameCount);
	}

	return true;
}

