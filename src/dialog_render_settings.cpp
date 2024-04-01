/* Copyright (C) 2018 Yosshin(@yosshin4004) */

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "config.h"
#include "common.h"
#include "app.h"
#include "dialog_render_settings.h"
#include "resource/resource.h"


static void UpdateButtonState(HWND hDwnd){
	bool enableMultipleRenderTargets = GetDlgItemCheck(
		hDwnd, IDD_RENDER_SETTINGS_ENABLE_MULTIPLE_RENDER_TARGETS
	);
	EnableWindow(
		GetDlgItem(hDwnd, IDC_RENDER_SETTINGS_NUM_ENABLED_RENDER_TARGETS),
		enableMultipleRenderTargets
	);

	bool enableBackBuffer = GetDlgItemCheck(
		hDwnd, IDD_RENDER_SETTINGS_ENABLE_BACK_BUFFER
	);
	EnableWindow(
		GetDlgItem(hDwnd, IDD_RENDER_SETTINGS_ENABLE_MIPMAP_GENERATION),
		enableBackBuffer
	);
	EnableWindow(
		GetDlgItem(hDwnd, IDR_RENDER_SETTINGS_TEXTURE_FILTER_NEAREST),
		enableBackBuffer
	);
	EnableWindow(
		GetDlgItem(hDwnd, IDR_RENDER_SETTINGS_TEXTURE_FILTER_LINEAR),
		enableBackBuffer
	);
	EnableWindow(
		GetDlgItem(hDwnd, IDR_RENDER_SETTINGS_TEXTURE_WRAP_REPEAT),
		enableBackBuffer
	);
	EnableWindow(
		GetDlgItem(hDwnd, IDR_RENDER_SETTINGS_TEXTURE_WRAP_CLAMP_TO_EDGE),
		enableBackBuffer
	);
	EnableWindow(
		GetDlgItem(hDwnd, IDR_RENDER_SETTINGS_TEXTURE_WRAP_MIRRORED_REPEAT),
		enableBackBuffer
	);

	bool enableSwapIntervalControl = GetDlgItemCheck(
		hDwnd, IDC_RENDER_SETTINGS_ENABLE_SWAP_INTERVAL_CONTROL
	);
	EnableWindow(
		GetDlgItem(hDwnd, IDR_RENDER_SETTINGS_SWAP_INTERVAL_ALLOW_TEARING),
		enableSwapIntervalControl
	);
	EnableWindow(
		GetDlgItem(hDwnd, IDR_RENDER_SETTINGS_SWAP_INTERVAL_HSYNC),
		enableSwapIntervalControl
	);
	EnableWindow(
		GetDlgItem(hDwnd, IDR_RENDER_SETTINGS_SWAP_INTERVAL_VSYNC),
		enableSwapIntervalControl
	);
}

static LRESULT CALLBACK DialogFunc(
	HWND hDwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
){
	switch (uMsg) {
		/* ダイアログボックスの初期化 */
		case WM_INITDIALOG: {
			/* 解像度をエディットボックスに設定 */
			{
				int xReso = 0;
				int yReso = 0;
				AppGetResolution(&xReso, &yReso);
				SetDlgItemInt(
					hDwnd, IDD_RENDER_SETTINGS_XRESO,
					xReso, false
				);
				SetDlgItemInt(
					hDwnd, IDD_RENDER_SETTINGS_YRESO,
					yReso, false
				);
			}

			/* ピクセルフォーマットをラジオボタンに設定 */
			{
				int nIDDlgItem = 0;
				switch (AppRenderSettingsGetPixelFormat()) {
					case PixelFormatUnorm8Rgba: {
						nIDDlgItem = IDR_RENDER_SETTINGS_PIXEL_FORMAT_UNORM8_RGBA;
					} break;
					case PixelFormatFp16Rgba: {
						nIDDlgItem = IDR_RENDER_SETTINGS_PIXEL_FORMAT_FP16_RGBA;
					} break;
					case PixelFormatFp32Rgba: {
						nIDDlgItem = IDR_RENDER_SETTINGS_PIXEL_FORMAT_FP32_RGBA;
					} break;
					default: {
						assert(false);
					} break;
				}
				SetDlgItemCheck(hDwnd, nIDDlgItem, true);
			}

			/* マルチレンダーターゲット有効化フラグをチェックボックスに設定 */
			SetDlgItemCheck(
				hDwnd, IDD_RENDER_SETTINGS_ENABLE_MULTIPLE_RENDER_TARGETS,
				AppRenderSettingsGetEnableMultipleRenderTargetsFlag()
			);

			/* 有効レンダーターゲット数をコンボボックスに設定 */
			{
				HWND dlgItem = GetDlgItem(hDwnd, IDC_RENDER_SETTINGS_NUM_ENABLED_RENDER_TARGETS);

				/* コンボボックスに項目を送信 */
				for (int numRenderTargets = 2; numRenderTargets <= NUM_RENDER_TARGETS; numRenderTargets++) {
					char string[0x100];
					snprintf(string, sizeof(string), "MRT%d", numRenderTargets);
					SendMessage(
						dlgItem, CB_INSERTSTRING,
						numRenderTargets - 2 /* 0 でなく 2 から開始するので */,
						(LPARAM)string);
				}

				/* 初期状態で選択されている項目の指定 */
				SendMessage(
					dlgItem, CB_SETCURSEL,
					AppRenderSettingsGetNumEnabledRenderTargets() - 2 /* 0 でなく 2 から開始するので */,
					(LPARAM)0
				);
			}

			/* バックバッファ有効化フラグをチェックボックスに設定 */
			SetDlgItemCheck(
				hDwnd, IDD_RENDER_SETTINGS_ENABLE_BACK_BUFFER,
				AppRenderSettingsGetEnableBackBufferFlag()
			);

			/* ミップマップ生成有効化フラグをチェックボックスに設定 */
			SetDlgItemCheck(
				hDwnd, IDD_RENDER_SETTINGS_ENABLE_MIPMAP_GENERATION,
				AppRenderSettingsGetEnableMipmapGenerationFlag()
			);

			/* テクスチャフィルタをラジオボタンに設定 */
			{
				int nIDDlgItem = 0;
				switch (AppRenderSettingsGetTextureFilter()) {
					case TextureFilterNearest: {
						nIDDlgItem = IDR_RENDER_SETTINGS_TEXTURE_FILTER_NEAREST;
					} break;
					case TextureFilterLinear: {
						nIDDlgItem = IDR_RENDER_SETTINGS_TEXTURE_FILTER_LINEAR;
					} break;
					default: {
						assert(false);
					} break;
				}
				SetDlgItemCheck(hDwnd, nIDDlgItem, true);
			}

			/* テクスチャラップをラジオボタンに設定 */
			{
				int nIDDlgItem = 0;
				switch (AppRenderSettingsGetTextureWrap()) {
					case TextureWrapRepeat: {
						nIDDlgItem = IDR_RENDER_SETTINGS_TEXTURE_WRAP_REPEAT;
					} break;
					case TextureWrapClampToEdge: {
						nIDDlgItem = IDR_RENDER_SETTINGS_TEXTURE_WRAP_CLAMP_TO_EDGE;
					} break;
					case TextureWrapMirroredRepeat: {
						nIDDlgItem = IDR_RENDER_SETTINGS_TEXTURE_WRAP_MIRRORED_REPEAT;
					} break;
					default: {
						assert(false);
					} break;
				}
				SetDlgItemCheck(hDwnd, nIDDlgItem, true);
			}

			/* スワップインターバルコントロール有効化フラグをチェックボックスに設定 */
			SetDlgItemCheck(
				hDwnd, IDC_RENDER_SETTINGS_ENABLE_SWAP_INTERVAL_CONTROL,
				AppRenderSettingsGetEnableSwapIntervalControlFlag()
			);

			/* スワップインターバルをラジオボタンに設定 */
			{
				int nIDDlgItem = 0;
				switch (AppRenderSettingsGetSwapIntervalControl()) {
					case SwapIntervalAllowTearing :{
						nIDDlgItem = IDR_RENDER_SETTINGS_SWAP_INTERVAL_ALLOW_TEARING;
					} break;
					case SwapIntervalHsync :{
						nIDDlgItem = IDR_RENDER_SETTINGS_SWAP_INTERVAL_HSYNC;
					} break;
					case SwapIntervalVsync :{
						nIDDlgItem = IDR_RENDER_SETTINGS_SWAP_INTERVAL_VSYNC;
					} break;
					default: {
						assert(false);
					} break;
				}
				SetDlgItemCheck(hDwnd, nIDDlgItem, true);
			}

			/* ボタン有効/無効ステート更新 */
			UpdateButtonState(hDwnd);

			/* メッセージは処理された */
			return 1;
		} break;

		/* UI 入力を検出 */
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				/* OK */
				case IDOK: {
					/* 解像度をエディットボックスから取得 */
					BOOL xResoTranslated = FALSE;
					int xReso = GetDlgItemInt(
						hDwnd, IDD_RENDER_SETTINGS_XRESO,
						&xResoTranslated, false
					);
					BOOL yResoTranslated = FALSE;
					int yReso = GetDlgItemInt(
						hDwnd, IDD_RENDER_SETTINGS_YRESO,
						&yResoTranslated, false
					);
					if (xResoTranslated == FALSE) {
						AppErrorMessageBox(APP_NAME, "Invalid X resolution");
						return 0;	/* メッセージは処理されなかった */
					}
					if (yResoTranslated == FALSE) {
						AppErrorMessageBox(APP_NAME, "Invalid Y resolution");
						return 0;	/* メッセージは処理されなかった */
					}
					if (xReso > MAX_RESO || yReso > MAX_RESO) {
						AppErrorMessageBox(APP_NAME, "Invalid resolution (mast be <= %d)", MAX_RESO);
						return 0;	/* メッセージは処理されなかった */
					}

					/* ピクセルフォーマットをラジオボタンから取得 */
					PixelFormat pixelFormat = PixelFormatUnorm8Rgba;
					{
						if (GetDlgItemCheck(hDwnd, IDR_RENDER_SETTINGS_PIXEL_FORMAT_UNORM8_RGBA)) {
							pixelFormat = PixelFormatUnorm8Rgba;
						} else
						if (GetDlgItemCheck(hDwnd, IDR_RENDER_SETTINGS_PIXEL_FORMAT_FP16_RGBA)) {
							pixelFormat = PixelFormatFp16Rgba;
						} else
						if (GetDlgItemCheck(hDwnd, IDR_RENDER_SETTINGS_PIXEL_FORMAT_FP32_RGBA)) {
							pixelFormat = PixelFormatFp32Rgba;
						} else {
							AppErrorMessageBox(APP_NAME, "Invalid frame buffer.");
						}
					}

					/* マルチレンダーターゲット有効化フラグをチェックボックスから取得 */
					bool enableMultipleRenderTargets = GetDlgItemCheck(
						hDwnd, IDD_RENDER_SETTINGS_ENABLE_MULTIPLE_RENDER_TARGETS
					);

					/* 有効レンダーターゲット数をコンボボックスから取得 */
					int numRenderTargets = (int)SendMessage(
						GetDlgItem(hDwnd, IDC_RENDER_SETTINGS_NUM_ENABLED_RENDER_TARGETS),
						CB_GETCURSEL, 0, (LPARAM)0
					) + 2 /* 0 でなく 2 から開始するので */;

					/* バックバッファ有効化フラグをチェックボックスから取得 */
					bool enableBackBuffer = GetDlgItemCheck(
						hDwnd, IDD_RENDER_SETTINGS_ENABLE_BACK_BUFFER
					);

					/* ミップマップ生成フラグをチェックボックスから取得 */
					bool enableMipmapGeneration = GetDlgItemCheck(
						hDwnd, IDD_RENDER_SETTINGS_ENABLE_MIPMAP_GENERATION
					);

					/* テクスチャフィルタをラジオボタンから取得 */
					TextureFilter textureFilter = TextureFilterNearest;
					{
						if (GetDlgItemCheck(hDwnd, IDR_RENDER_SETTINGS_TEXTURE_FILTER_NEAREST)) {
							textureFilter = TextureFilterNearest;
						} else
						if (GetDlgItemCheck(hDwnd, IDR_RENDER_SETTINGS_TEXTURE_FILTER_LINEAR)) {
							textureFilter = TextureFilterLinear;
						} else {
							AppErrorMessageBox(APP_NAME, "Invalid texture filter.");
						}
					}

					/* テクスチャラップをラジオボタンから取得 */
					TextureWrap textureWrap = TextureWrapRepeat;
					{
						if (GetDlgItemCheck(hDwnd, IDR_RENDER_SETTINGS_TEXTURE_WRAP_REPEAT)) {
							textureWrap = TextureWrapRepeat;
						} else
						if (GetDlgItemCheck(hDwnd, IDR_RENDER_SETTINGS_TEXTURE_WRAP_CLAMP_TO_EDGE)) {
							textureWrap = TextureWrapClampToEdge;
						} else
						if (GetDlgItemCheck(hDwnd, IDR_RENDER_SETTINGS_TEXTURE_WRAP_MIRRORED_REPEAT)) {
							textureWrap = TextureWrapMirroredRepeat;
						} else {
							AppErrorMessageBox(APP_NAME, "Invalid texture wrap.");
						}
					}

					/* スワップインターバルコントロール有効化フラグをチェックボックスから取得 */
					bool enableSwapIntervalControl = GetDlgItemCheck(
						hDwnd, IDC_RENDER_SETTINGS_ENABLE_SWAP_INTERVAL_CONTROL
					);

					/* スワップインターバルをラジオボタンから取得 */
					SwapInterval swapInterval = SwapIntervalAllowTearing;
					{
						if (GetDlgItemCheck(hDwnd, IDR_RENDER_SETTINGS_SWAP_INTERVAL_ALLOW_TEARING)) {
							swapInterval = SwapIntervalAllowTearing;
						} else
						if (GetDlgItemCheck(hDwnd, IDR_RENDER_SETTINGS_SWAP_INTERVAL_HSYNC)) {
							swapInterval = SwapIntervalHsync;
						} else
						if (GetDlgItemCheck(hDwnd, IDR_RENDER_SETTINGS_SWAP_INTERVAL_VSYNC)) {
							swapInterval = SwapIntervalVsync;
						} else {
							AppErrorMessageBox(APP_NAME, "Invalid swap interval.");
						}
					}

					/* App に通知 */
					AppSetResolution(xReso, yReso);
					AppRenderSettingsSetPixelFormat(pixelFormat);
					AppRenderSettingsSetEnableMultipleRenderTargetsFlag(enableMultipleRenderTargets);
					AppRenderSettingsSetNumEnabledRenderTargets(numRenderTargets);
					AppRenderSettingsSetEnableBackBufferFlag(enableBackBuffer);
					AppRenderSettingsSetEnableMipmapGenerationFlag(enableMipmapGeneration);
					AppRenderSettingsSetTextureFilter(textureFilter);
					AppRenderSettingsSetTextureWrap(textureWrap);
					AppRenderSettingsSetEnableSwapIntervalControlFlag(enableSwapIntervalControl);
					AppRenderSettingsSetSwapIntervalControl(swapInterval);

					/* ダイアログボックス終了 */
					EndDialog(hDwnd, DialogRenderSettingsResult_Ok);

					/* メッセージは処理された */
					return 1;
				} break;

				/* キャンセル */
				case IDCANCEL: {
					/* ダイアログボックス終了 */
					EndDialog(hDwnd, DialogRenderSettingsResult_Canceled);

					/* メッセージは処理された */
					return 1;
				} break;

				/* Enable multiple render targets チェックボックスの更新  */
				case IDD_RENDER_SETTINGS_ENABLE_MULTIPLE_RENDER_TARGETS: {
					/* ボタン有効/無効ステート更新 */
					UpdateButtonState(hDwnd);
				} break;

				/* Enable back buffer チェックボックスの更新  */
				case IDD_RENDER_SETTINGS_ENABLE_BACK_BUFFER: {
					/* ボタン有効/無効ステート更新 */
					UpdateButtonState(hDwnd);
				} break;

				/* Enable swap interval control チェックボックスの更新  */
				case IDC_RENDER_SETTINGS_ENABLE_SWAP_INTERVAL_CONTROL: {
					/* ボタン有効/無効ステート更新 */
					UpdateButtonState(hDwnd);
				} break;
			}
		} break;
	}

	/* メッセージは処理されなかった */
	return 0;
}


DialogRenderSettingsResult
DialogRenderSettings()
{
	return (DialogRenderSettingsResult)DialogBox(
		AppGetCurrentInstance(),
		"RENDER_SETTINGS",
		AppGetMainWindowHandle(),
		DialogFunc
	);
}

