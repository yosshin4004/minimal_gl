/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <shellapi.h>

#include "config.h"
#include "dialog_camera_settings.h"
#include "dialog_export_executable.h"
#include "dialog_capture_screen_shot.h"
#include "dialog_capture_cubemap.h"
#include "dialog_capture_sound.h"
#include "dialog_record_image_sequence.h"
#include "dialog_preference_settings.h"
#include "dialog_render_settings.h"
#include "dialog_load_user_textures.h"
#include "dialog_gfx_uniforms.h"
#include "dialog_snd_uniforms.h"
#include "dialog_preprocessor_definitions.h"
#include "common.h"
#include "gl3w_work_around.h"
#include "app.h"

#include "resource/resource.h"
#define DEFAULT_ICON_NAME	"IDI_DEFAULT"
#define DEFAULT_ICON_ID		IDI_DEFAULT
#define SMALL_ICON_NAME		"IDI_SMALL"
#define SMALL_ICON_ID		IDI_SMALL
#define DEFAULT_MENU_NAME	"MYMENU"

/* GUI のスタイルをモダンに変更 */
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


/*=============================================================================
▼	メインウィンドウ関連
-----------------------------------------------------------------------------*/
static bool s_fullScreen = false;
static HACCEL s_hAccel = 0;
static HDC s_hDC = 0;
static HGLRC s_hRC = 0;
static const char s_wndClassName[] = APP_NAME "main window";
static const char s_windowName[] = APP_NAME;
static PIXELFORMATDESCRIPTOR s_pixelFormatDescriptor = {
	/* WORD  nSize */			sizeof(PIXELFORMATDESCRIPTOR),
	/* WORD  nVersion */		1,
	/* DWORD dwFlags */			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
	/* BYTE  iPixelType */		PFD_TYPE_RGBA,
	/* BYTE  cColorBits */		32,
	/* BYTE  cRedBits */		0,
	/* BYTE  cRedShift */		0,
	/* BYTE  cGreenBits */		0,
	/* BYTE  cGreenShift */		0,
	/* BYTE  cBlueBits */		0,
	/* BYTE  cBlueShift */		0,
	/* BYTE  cAlphaBits */		8,
	/* BYTE  cAlphaShift */		0,
	/* BYTE  cAccumBits */		0,
	/* BYTE  cAccumRedBits */	0,
	/* BYTE  cAccumGreenBits */	0,
	/* BYTE  cAccumBlueBits */	0,
	/* BYTE  cAccumAlphaBits */	0,
	/* BYTE  cDepthBits */		32,
	/* BYTE  cStencilBits */	0,
	/* BYTE  cAuxBuffers */		0,
	/* BYTE  iLayerType */		PFD_MAIN_PLANE,
	/* BYTE  bReserved */		0,
	/* DWORD dwLayerMask */		0,
	/* DWORD dwVisibleMask */	0,
	/* DWORD dwDamageMask */	0
};

/* 全画面 or ウィンドウ切り替え */
void ToggleFullScreen(){
	DWORD windowStyle;

	if (s_fullScreen == false) {
		/* windowStyle を変更 */
		windowStyle = WS_MAXIMIZE | WS_POPUP | WS_VISIBLE;
		SetWindowLong(AppGetMainWindowHandle(), GWL_STYLE, windowStyle);

		/* メニューを消す */
		DestroyMenu(GetMenu(AppGetMainWindowHandle()));
		SetMenu(AppGetMainWindowHandle(), NULL);

		/* フルスクリーン化 */
		ChangeDisplaySettings(
			/* DEVMODEA *lpDevMode */	NULL,
			/* DWORD    dwFlags */		CDS_FULLSCREEN
		);

		/* ウィンドウサイズ変更 */
		SetWindowPos(
			/* HWND hWnd */				AppGetMainWindowHandle(),
			/* HWND hWndInsertAfter */	NULL,
			/* int X */					0,
			/* int Y */					0,
			/* int cx */				GetSystemMetrics(SM_CXSCREEN),
			/* int cy */				GetSystemMetrics(SM_CYSCREEN),
			/* UINT uFlags */			SWP_FRAMECHANGED
		);

		s_fullScreen = true;
	} else {
		/* windowStyle を変更 */
		windowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
		SetWindowLong(AppGetMainWindowHandle(), GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);

		/* メニューを復活 */
		SetMenu(AppGetMainWindowHandle(), LoadMenu(AppGetCurrentInstance(), DEFAULT_MENU_NAME));

		/* ウィンドウ化 */
		ChangeDisplaySettings(
			/* DEVMODEA *lpDevMode */	NULL,
			/* DWORD    dwFlags */		0
		);

		/* ウィンドウサイズ変更 */
		RECT rect = {0};
		rect.right = DEFAULT_SCREEN_XRESO;
		rect.bottom = DEFAULT_SCREEN_YRESO;
		AdjustWindowRectEx(
			&rect,
			GetWindowLong(AppGetMainWindowHandle(), GWL_STYLE),
			TRUE,		/* メニューを持つか？ */
			GetWindowLong(AppGetMainWindowHandle(), GWL_EXSTYLE)
		);
		SetWindowPos(
			/* HWND hWnd */				AppGetMainWindowHandle(),
			/* HWND hWndInsertAfter */	NULL,
			/* int X */					(GetSystemMetrics(SM_CXSCREEN) - rect.right + rect.left) >> 1,
			/* int Y */					(GetSystemMetrics(SM_CYSCREEN) - rect.bottom + rect.top) >> 1,
			/* int cx */				rect.right - rect.left,
			/* int cy */				rect.bottom - rect.top,
			/* UINT uFlags */			SWP_FRAMECHANGED
		);

		s_fullScreen = false;
	}
}

/* ImGui のウィンドウプロシージャ */
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

/* ウィンドウプロシージャ */
static LRESULT CALLBACK MainWndProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
){
	/* ImGui 関連 */
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) return true;

	switch (uMsg) {
		case WM_SYSCOMMAND: {
			/* スクリーンセーバー抑制 */
			if ((wParam == SC_SCREENSAVE || wParam == SC_MONITORPOWER)) {
				return 0;
			}
		} break;

		/* [x] で閉じる */
		case WM_CLOSE:
		case WM_DESTROY: {
			PostQuitMessage(0);
			return 0;
		} break;

		/* マウスボタンの押下と開放を検出 */
		case WM_LBUTTONDOWN: {
			if (ImGui::GetIO().WantCaptureMouse == false) AppMouseLButtonDown();
			return 0;
		} break;
		case WM_MBUTTONDOWN: {
			if (ImGui::GetIO().WantCaptureMouse == false) AppMouseMButtonDown();
			return 0;
		} break;
		case WM_RBUTTONDOWN: {
			if (ImGui::GetIO().WantCaptureMouse == false) AppMouseRButtonDown();
			return 0;
		} break;
		case WM_LBUTTONUP: {
			if (ImGui::GetIO().WantCaptureMouse == false) AppMouseLButtonUp();
			return 0;
		} break;
		case WM_MBUTTONUP: {
			if (ImGui::GetIO().WantCaptureMouse == false) AppMouseMButtonUp();
			return 0;
		} break;
		case WM_RBUTTONUP: {
			if (ImGui::GetIO().WantCaptureMouse == false) AppMouseRButtonUp();
			return 0;
		} break;

		/* マウスホイールの回転を検出 */
		case WM_MOUSEWHEEL: {
			if (ImGui::GetIO().WantCaptureMouse == false) {
				AppSetMouseWheelDelta(GET_WHEEL_DELTA_WPARAM(wParam), GetAsyncKeyState(VK_MBUTTON));
			}
			return 0;
		} break;

		/* マウス移動を検出 */
		case WM_MOUSEMOVE: {
			if (ImGui::GetIO().WantCaptureMouse == false) {
				POINT pos;
				GetCursorPos(&pos);
				ScreenToClient(hWnd, &pos);
				AppSetMousePosition(pos.x, pos.y);
			}
			return 0;
		} break;

		/* ウィンドウサイズの変更を検出 */
		case WM_SIZE: {
			AppSetResolution(lParam & 65535, (int)(lParam >> 16));
			return 0;
		} break;

		/* キーの押下を検出 */
		case WM_KEYDOWN: {
		} break;

		/* ファイルのドロップを検出 */
		case WM_DROPFILES: {
			static char fileName[MAX_PATH] = {0};
			/* UINT */ DragQueryFile(
				/* HDROP hDrop */		/* ファイル名構造体のハンドル */
										(HDROP)wParam,
				/* UINT iFile */		/* 取得するファイルのインデックス番号 */
										0,
				/* LPTSTR lpszFile */	/* 取得したファイル名を格納するバッファ */
										fileName,
				/* UINT cch */			/* 取得したファイル名を格納するバッファのサイズ */
										sizeof(fileName)
			);
			AppGetWindowFocus();
			AppOpenDragAndDroppedFile(fileName);
			return 0;
		} break;

		/* UI 入力を検出 */
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				/* グラフィクスシェーダソースのオープン */
				case IDM_OPEN_GRAPHICS_SHADER: {
					if (s_fullScreen) {
						ToggleFullScreen();
					} else {
						char fileName[MAX_PATH] = {0};
						OPENFILENAME ofn = {0};
						ofn.lStructSize = sizeof(OPENFILENAME);
						ofn.hwndOwner = NULL;
						ofn.lpstrFilter =
							"Graphics shader file (*.gfx.glsl)\0*.gfx.glsl\0"
							"All files (*.*)\0*.*\0"
							"\0";
						ofn.lpstrFile = fileName;
						ofn.nMaxFile = sizeof(fileName);
						ofn.lpstrTitle = (LPSTR)"Open graphics shader file";

						if (GetOpenFileName(&ofn)) {
							AppOpenGraphicsShaderFile(fileName);
						}
					}
					return 0;
				} break;

				/* サウンドシェーダソースのオープン */
				case IDM_OPEN_SOUND_SHADER: {
					if (s_fullScreen) {
						ToggleFullScreen();
					} else {
						char fileName[MAX_PATH] = {0};
						OPENFILENAME ofn = {0};
						ofn.lStructSize = sizeof(OPENFILENAME);
						ofn.hwndOwner = NULL;
						ofn.lpstrFilter =
							"Sound shader file (*.snd.glsl)\0*.snd.glsl\0"
							"All files (*.*)\0*.*\0"
							"\0";
						ofn.lpstrFile = fileName;
						ofn.nMaxFile = sizeof(fileName);
						ofn.lpstrTitle = (LPSTR)"Open sound shader file";

						if (GetOpenFileName(&ofn)) {
							AppOpenSoundShaderFile(fileName);
						}
					}
					return 0;
				} break;

				/* スクリーンショット保存 */
				case IDM_CAPTURE_SCREEN_SHOT: {
					if (s_fullScreen) {
						ToggleFullScreen();
					} else {
						if (DialogCaptureScreenShot() == DialogCaptureScreenShotResult_Ok) {
							AppCaptureScreenShot();
						}
					}
					return 0;
				} break;

				/* キューブマップ dds として保存 */
				case IDM_CAPTURE_CUBEMAP: {
					if (s_fullScreen) {
						ToggleFullScreen();
					} else {
						if (DialogCaptureCubemap() == DialogCaptureCubemapResult_Ok) {
							AppCaptureCubemap();
						}
					}
					return 0;
				} break;

				/* サウンドを wav として保存 */
				case IDM_CAPTURE_SOUND: {
					if (s_fullScreen) {
						ToggleFullScreen();
					} else {
						if (DialogCaptureSound() == DialogCaptureSoundResult_Ok) {
							AppCaptureSound();
						}
					}
					return 0;
				} break;

				/* exe としてエクスポート */
				case IDM_EXPORT_EXECUTABLE: {
					if (s_fullScreen) {
						ToggleFullScreen();
					} else {
						if (DialogExportExecutable() == DialogExportExecutableResult_Ok) {
							AppExportExecutable();
						}
					}
					return 0;
				} break;

				/* 連番画像の保存 */
				case IDM_RECORD_IMAGE_SEQUENCE: {
					if (s_fullScreen) {
						ToggleFullScreen();
					} else {
						if (DialogRecordImageSequence() == DialogRecordImageSequenceResult_Ok) {
							AppRecordImageSequence();
						}
					}
					return 0;
				} break;

				/* プロジェクトファイルのインポート */
				case IDM_IMPORT_PROJECT: {
					if (s_fullScreen) {
						ToggleFullScreen();
					} else {
						AppProjectAutoExport(true);

						char fileName[MAX_PATH] = {0};
						strcpy_s(fileName, sizeof(fileName), AppProjectGetCurrentFileName());
						OPENFILENAME ofn = {0};
						ofn.lStructSize = sizeof(OPENFILENAME);
						ofn.hwndOwner = NULL;
						ofn.lpstrFilter =
							"Project json file (*.json)\0*.json\0"
							"\0";
						ofn.lpstrFile = fileName;
						ofn.nMaxFile = sizeof(fileName);
						ofn.lpstrTitle = (LPSTR)"Import project from json file";

						if (GetOpenFileName(&ofn)) {
							AppProjectImport(fileName);
						}
					}
					return 0;
				} break;

				/* プロジェクトファイルのエクスポート */
				case IDM_EXPORT_PROJECT: {
					if (s_fullScreen) {
						ToggleFullScreen();
					} else {
						char fileName[MAX_PATH] = {0};
						strcpy_s(fileName, sizeof(fileName), AppProjectGetCurrentFileName());
						OPENFILENAME ofn = {0};
						ofn.lStructSize = sizeof(OPENFILENAME);
						ofn.hwndOwner = NULL;
						ofn.lpstrFilter =
							"Project json file (*.json)\0*.json\0"
							"\0";
						ofn.lpstrFile = fileName;
						ofn.nMaxFile = sizeof(fileName);
						ofn.lpstrTitle = (LPSTR)"Export project to json file";

						if (GetSaveFileName(&ofn)) {
							AppProjectExport(fileName);
						}
					}
					return 0;
				} break;

				/* ユーザーテクスチャ読み込み */
				case IDM_LOAD_USER_TEXTURES: {
					if (s_fullScreen) {
						ToggleFullScreen();
					} else {
						DialogLoadUserTextures();
					}
					return 0;
				} break;

				/* プリファレンス設定 */
				case IDM_PREFERENCE_SETTINGS: {
					if (s_fullScreen) {
						ToggleFullScreen();
					} else {
						DialogPreferenceSettings();
					}
					return 0;
				} break;

				/* レンダリング設定 */
				case IDM_RENDER_SETTINGS: {
					if (s_fullScreen) {
						ToggleFullScreen();
					} else {
						DialogRenderSettings();
					}
					return 0;
				} break;

				/* 全レンダーターゲットをクリア */
				case IDM_CLEAR_ALL_RENDER_TARGETS: {
					AppClearAllRenderTargets();
				} break;

				/* カメラをリセット */
				case IDM_RESET_CAMERA: {
					AppResetCamera();
					return 0;
				} break;

				/* カメラ設定 */
				case IDM_CAMERA_SETTINGS: {
					if (s_fullScreen) {
						ToggleFullScreen();
					} else {
						DialogCameraSettings();
					}
					return 0;
				} break;

				/* リスタート */
				case IDM_RESTART: {
					AppRestart();
					return 0;
				} break;

				/* 一時停止 / 再開 */
				case IDM_PAUSE_AND_RESUME: {
					AppTogglePauseAndResume();
					return 0;
				} break;

				/* スロー送り */
				case IDM_SLOW_FORWARD: {
					AppSlowForward();
					return 0;
				} break;

				/* スロー巻き戻し */
				case IDM_SLOW_BACKWARD: {
					AppSlowBackward();
					return 0;
				} break;

				/* 早送り */
				case IDM_FAST_FORWARD: {
					AppFastForward();
					return 0;
				} break;

				/* 巻き戻し */
				case IDM_FAST_BACKWARD: {
					AppFastBackward();
					return 0;
				} break;

				/* デフォルトグラフィクスシェーダの読み込み */
				case IDM_LOAD_DEFAULT_GRAPHICS_SHADER: {
					AppOpenDefaultGraphicsShader();
				} break;

				/* デフォルトサウンドシェーダの読み込み */
				case IDM_LOAD_DEFAULT_SOUND_SHADER: {
					AppOpenDefaultSoundShader();
				} break;

				/* 全画面 */
				case IDM_TOGGLE_FULL_SCREEN: {
					ToggleFullScreen();
					return 0;
				} break;

				/* ImGui Current Status の表示 */
				case IDM_TOGGLE_DISPLAY_CURRENT_STATUS: {
					bool flag = ToggleMenuItemCheck(
						GetMenu(AppGetMainWindowHandle()),
						IDM_TOGGLE_DISPLAY_CURRENT_STATUS
					);
					AppImGuiSetDisplayCurrentStatusFlag(flag);
				} break;

				/* ImGui Camera Settings の表示 */
				case IDM_TOGGLE_DISPLAY_CAMERA_SETTINGS: {
					bool flag = ToggleMenuItemCheck(
						GetMenu(AppGetMainWindowHandle()),
						IDM_TOGGLE_DISPLAY_CAMERA_SETTINGS
					);
					AppImGuiSetDisplayCameraSettingsFlag(flag);
				} break;

				/* グラフィクスシェーダ用ユニフォーム一覧 */
				case IDM_HELP_GRAPHICS_SHADER_UNIFORMS: {
					if (s_fullScreen) {
						ToggleFullScreen();
					} else {
						DialogGraphicsShaderUniforms();
					}
					return 0;
				} break;

				/* サウンドシェーダ用ユニフォーム一覧 */
				case IDM_HELP_SOUND_SHADER_UNIFORMS: {
					if (s_fullScreen) {
						ToggleFullScreen();
					} else {
						DialogSoundShaderUniforms();
					}
					return 0;
				} break;

				/* プリプロセッサ定義一覧 */
				case IDM_HELP_PREPROCESSOR_DEFINITIONS: {
					if (s_fullScreen) {
						ToggleFullScreen();
					} else {
						DialogPreprocessorDefinitions();
					}
					return 0;
				} break;

				/* このアプリケーションについて */
				case IDM_HELP_ABOUT: {
					if (s_fullScreen) {
						ToggleFullScreen();
					} else {
						AppHelpAbout();
					}
					return 0;
				} break;

				/* アプリケーションの終了 */
				case IDM_QUIT: {
					if (s_fullScreen) {
						ToggleFullScreen();
					} else {
						if (AppYesNoMessageBox(APP_NAME, "Quit?") == true) {
							AppProjectAutoExport(true);
							PostQuitMessage(0);
						}
					}
					return 0;
				} break;
			}
		} break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/* ウィンドウ終了処理 */
static bool WindowTerminate(
){
	/* ImGui 関連 */
	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();
	ImGui_ImplWin32_Shutdown();

	/* GL コンテキストの削除 */
	if (s_hRC) {
		wglMakeCurrent(0, 0);
		wglDeleteContext(s_hRC);
	}

	/* デバイスコンテキスト解放 */
	if (s_hDC) ReleaseDC(AppGetMainWindowHandle(), s_hDC);

	/* ウィンドウ削除 */
	if (AppGetMainWindowHandle()) DestroyWindow(AppGetMainWindowHandle());

	/* ウィンドウクラスの登録解除 */
	UnregisterClass(s_wndClassName, AppGetCurrentInstance());

	return true;
}


/* ウィンドウ初期化 */
static bool WindowInitialize(
){
	/* ウィンドウクラスの登録 */
	{
		WNDCLASSEX wndClass;
		wndClass.cbSize			= sizeof(wndClass);
		wndClass.style			= CS_HREDRAW | CS_VREDRAW;
		wndClass.lpfnWndProc	= (WNDPROC)MainWndProc;
		wndClass.cbClsExtra		= 0;
		wndClass.cbWndExtra		= 0;
		wndClass.hInstance		= AppGetCurrentInstance();
		wndClass.hIcon			= LoadIcon(AppGetCurrentInstance(), MAKEINTRESOURCE(IDI_DEFAULT));
		wndClass.hCursor		= LoadCursor(NULL, IDC_ARROW);
		wndClass.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH);
		wndClass.lpszMenuName	= DEFAULT_MENU_NAME;
		wndClass.lpszClassName	= s_wndClassName;
		wndClass.hIconSm		= LoadIcon(AppGetCurrentInstance(), MAKEINTRESOURCE(IDI_SMALL));
		if (wndClass.hIcon == 0) {
			printf("LoadIcon " DEFAULT_ICON_NAME " failed.\n");
			return false;
		}
		if (wndClass.hIconSm == 0) {
			printf("LoadIcon " SMALL_ICON_NAME " failed.\n");
			return false;
		}
		if (!RegisterClassEx(&wndClass)) {
			printf("RegisterClassEx failed.\n");
			return false;
		}
	}

	/* ウィンドウ作成 */
	{
		DWORD windowExStyle = 0;
		DWORD windowStyle = (
			WS_BORDER * 0
		|	WS_CAPTION * 0
		|	WS_CHILD * 0
		|	WS_CLIPCHILDREN * 0
		|	WS_CLIPSIBLINGS * 0
		|	WS_DISABLED * 0
		|	WS_DLGFRAME * 0
		|	WS_GROUP * 0
		|	WS_HSCROLL * 0
		|	WS_MAXIMIZE * 0
		|	WS_MAXIMIZEBOX * 0
		|	WS_MINIMIZE * 0
		|	WS_MINIMIZEBOX * 0
		|	WS_OVERLAPPED * 0
		|	WS_OVERLAPPEDWINDOW * 1
		|	WS_POPUP * 0
		|	WS_POPUPWINDOW * 0
		|	WS_SYSMENU * 0
		|	WS_TABSTOP * 0
		|	WS_THICKFRAME * 0
		|	WS_VISIBLE * 1
		|	WS_VSCROLL * 0
		);

		RECT rect = {0};
		rect.right = DEFAULT_SCREEN_XRESO;
		rect.bottom = DEFAULT_SCREEN_YRESO;
		AdjustWindowRectEx(
			&rect,
			windowStyle,
			TRUE,		/* メニューを持つか？ */
			windowExStyle
		);

		HWND hWnd = CreateWindowEx(
			windowExStyle,
			s_wndClassName,
			s_windowName,
			windowStyle,
			(GetSystemMetrics(SM_CXSCREEN) - rect.right + rect.left) >> 1,
			(GetSystemMetrics(SM_CYSCREEN) - rect.bottom + rect.top) >> 1,
			rect.right - rect.left,
			rect.bottom - rect.top,
			(HWND)NULL,
			(HMENU)NULL,
			AppGetCurrentInstance(),
			NULL
		);
		if (hWnd == NULL) {
			printf("CreateWindowEx failed.\n");
			return false;
		}

		/* 現在のメインウィンドウのハンドルを設定 */
		AppSetMainWindowHandle(hWnd);
	}

	/* アクセラレータテーブルをロード */
	s_hAccel = LoadAccelerators(AppGetCurrentInstance(), DEFAULT_MENU_NAME);

	/* デバイスコンテキストのハンドルを取得 */
	s_hDC = GetDC(AppGetMainWindowHandle());

	/* デバイスコンテキストにピクセルフォーマットを設定 */
	SetPixelFormat(
		s_hDC,
		ChoosePixelFormat(s_hDC, &s_pixelFormatDescriptor),
		&s_pixelFormatDescriptor
	);

	/* GL コンテキストを作成して、カレントに設定 */
	s_hRC = wglCreateContext(s_hDC);
	wglMakeCurrent(s_hDC, s_hRC);

	/* ドラッグアンドドロップを受け入れる */
	DragAcceptFiles(AppGetMainWindowHandle(), TRUE);

	/* ImGui 関連 */
	{
		/* GL 拡張 API の取得 */
		CallGl3wInit();

		/* ImGui バインディングの設定 */
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO &io = ImGui::GetIO(); (void)io;

		/* Win32 用の初期化 */
		ImGui_ImplWin32_Init(AppGetMainWindowHandle());

		/* OpenGL 実装の初期化（GL3.0 & GLSL130）*/
		const char *glslVersion = "#version 130";
		ImGui_ImplOpenGL3_Init(glslVersion);

		/* スタイルの設定 */
		ImGui::StyleColorsClassic();
	}

	return true;
}


/*=============================================================================
▼	メイン処理
-----------------------------------------------------------------------------*/
int WINAPI WinMain(
	HINSTANCE hCurrentInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
){
	MSG	msg;
	int	done = 0;

	/* 高 DPI モニタ環境で、ウィンドウが拡大されるのを抑制 */
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	/* 現在のアプリケーションインスタンスのハンドルを設定 */
	AppSetCurrentInstance(hCurrentInstance);

	/* TTY 出力確認用に dos 窓を開く */
	if (1) {
		COORD coord;
		coord.X = 80;
		coord.Y = 4095;
		AllocConsole();
		SetConsoleScreenBufferSize(
			GetStdHandle(STD_OUTPUT_HANDLE),
			coord
		);
		freopen("conin$", "r", stdin);
		freopen("conout$", "w", stdout);
		freopen("conout$", "w", stderr);
	}

	/* コマンドライン文字列のコピーを作成 */
	size_t cmdLineLength = strlen(lpCmdLine) + 1 /* 末端 \0 分 */;
	char *cmdLineCopy = (char *)malloc(cmdLineLength);
	if (cmdLineCopy == NULL) return 0;
	memcpy(cmdLineCopy, lpCmdLine, cmdLineLength);

	/* コマンドライン引数の解析 */
	#define ARGC_MAX	(256)
	char *(argv[ARGC_MAX]) = {0};
	int argc = 1;
	{
		/* 実行ファイル名をフルパスで取得 */
		static char s_appFullPathFileName[MAX_PATH] = {0};
		GetModuleFileName(
			/* HMODULE hModule    モジュールのハンドル */	NULL,
			/* LPTSTR lpFileName  モジュールのファイル名 */	s_appFullPathFileName,
			/* DWORD nSize        バッファのサイズ */		sizeof(s_appFullPathFileName)
		);

		/* コマンドライン引数のパース */
		argv[0] = s_appFullPathFileName;
		{
			bool inQuote = false;
			bool isBlank = true;
			char *p;
			for (p = cmdLineCopy; p < cmdLineCopy + cmdLineLength; p++) {
				switch (*p) {
					case '\0': {
						/* 終点検出 */
					} break;

					case ' ':
					case '\t': {
						if (inQuote == false) {
							*p = '\0';
							isBlank = true;
						}
					} break;

					case '\"': {
						*p = '\0';
						if (inQuote) {
							inQuote = false;
						} else {
							inQuote = true;
						}
					} break;

					default: {
						if (isBlank) {
							isBlank = false;

							/* 項目の開始位置と見なす */
							if (argc < ARGC_MAX) {
								argv[argc] = p;
								argc++;
							}
						}
					} break;
				}
			}
		}
	}

	/* ウィンドウ初期化 */
	if (WindowInitialize() == false) {
		AppErrorMessageBox(APP_NAME, "WindowInitialize() failed.");
		return 0;
	}

	/* アプリケーション初期化 */
	if (AppInitialize(argc, argv) == false) {
		AppErrorMessageBox(APP_NAME, "AppInitialize() failed.");
		return 0;
	}

	/* メインループ */
	HWND hWnd = AppGetMainWindowHandle();
	while (!done) {
		/* メッセージ監視 */
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) done = 1;
			if (!TranslateAccelerator(hWnd, s_hAccel, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		/* ImGui フレームの開始 */
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		/* アプリケーション更新 */
		if (AppUpdate() == false) {
			AppErrorMessageBox(APP_NAME, "AppUpdate() failed.");
			break;
		}

		/* メニューのチェック状態を更新 */
		{
			HMENU hMenu = GetMenu(AppGetMainWindowHandle());
			SetMenuItemCheck(
				hMenu,
				IDM_TOGGLE_DISPLAY_CURRENT_STATUS,
				AppImGuiGetDisplayCurrentStatusFlag()
			);
			SetMenuItemCheck(
				hMenu,
				IDM_TOGGLE_DISPLAY_CAMERA_SETTINGS,
				AppImGuiGetDisplayCameraSettingsFlag()
			);
		}

		/* アプリケーション解像度取得 */
		int xReso, yReso;
		AppGetResolution(&xReso, &yReso);

		/* アプリケーション側で解像度変更があったらメインウィンドウをリサイズ */
		{
			int wClient, hClient;
			{
				RECT clientRect;
				GetClientRect(hWnd, &clientRect);
				wClient = clientRect.right - clientRect.left;
				hClient = clientRect.bottom - clientRect.top;
			}
			if (wClient != xReso || hClient != yReso) {
				printf(
					"\n"
					"clientSize %d %d != currentReso %d %d\n",
					wClient, hClient,
					xReso, yReso
				);
				int xWindow, yWindow;
				{
					RECT windowRect;
					GetWindowRect(hWnd, &windowRect);
					xWindow = windowRect.left;
					yWindow = windowRect.top;
				}
				int wWindow, hWindow;
				{
					RECT windowRect = {0, 0, xReso, yReso};
					AdjustWindowRectEx(
						&windowRect,
						GetWindowLong(hWnd, GWL_STYLE),
						TRUE,		/* メニューを持つか？ */
						GetWindowLong(hWnd, GWL_EXSTYLE)
					);
					wWindow = windowRect.right - windowRect.left;
					hWindow = windowRect.bottom - windowRect.top;
				}
				SetWindowPos(
					/* HWND hWnd */				hWnd,
					/* HWND hWndInsertAfter */	NULL,
					/* int X */					xWindow,
					/* int Y */					yWindow,
					/* int cx */				wWindow,
					/* int cy */				hWindow,
					/* UINT uFlags */			SWP_FRAMECHANGED
				);
			}
		}

		/* デモウィンドウの表示 */
//		ImGui::ShowDemoWindow();

		/* ImGui レンダリング */
		ImGui::Render();
		glViewport(0, 0, xReso, yReso);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		/* フリップ */
		SwapBuffers(s_hDC);

		/* 他プロセスに処理時間を与える */
		Sleep(0);
	}

	/* 終了処理 */
	if (AppTerminate() == false) {
		AppErrorMessageBox(APP_NAME, "AppTerminate() failed.");
		return 0;
	}
	if (WindowTerminate() == false) {
		AppErrorMessageBox(APP_NAME, "WindowTerminate() failed.");
		return 0;
	}

	/* コマンドライン文字列のコピーを破棄 */
	free(cmdLineCopy);

    return 0;
}


