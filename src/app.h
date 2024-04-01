/* Copyright (C) 2018 Yosshin(@yosshin4004) */


#include "graphics.h"
#include "export_executable.h"


#ifndef _APP_H_
#define _APP_H_


#define APP_NAME "MinimalGL"


/* 現在のアプリケーションインスタンスのハンドルを設定 */
void AppSetCurrentInstance(HINSTANCE hInstance);

/* 現在のアプリケーションインスタンスのハンドルを取得 */
HINSTANCE AppGetCurrentInstance();

/* 現在のメインウィンドウのハンドルを設定 */
void AppSetMainWindowHandle(HWND hWindow);

/* 現在のメインウィンドウのハンドルを取得 */
HWND AppGetMainWindowHandle();

/* メインウィンドウを最前面に出す */
void AppGetWindowFocus();

/* メインウィンドウのタイトルバーの表示を更新 */
void AppUpdateWindowTitleBar();


/* メッセージ BOX の表示 */
void AppMessageBox(const char *caption, const char *format, ...);

/* YES/NO メッセージ BOX の表示 */
bool AppYesNoMessageBox(const char *caption, const char *format, ...);

/* エラーメッセージ BOX の表示 */
void AppErrorMessageBox(const char *caption, const char *format, ...);

/* GetLastError() の結果をメッセージ BOX で表示 */
void AppLastErrorMessageBox(const char *caption);


/* マウス L ボタンが押された */
void AppMouseLButtonDown();

/* マウス M ボタンが押された */
void AppMouseMButtonDown();

/* マウス R ボタンが押された */
void AppMouseRButtonDown();

/* マウス L ボタンが放された */
void AppMouseLButtonUp();

/* マウス L ボタンが放された */
void AppMouseMButtonUp();

/* マウス L ボタンが放された */
void AppMouseRButtonUp();

/* マウスホイールの回転を検出した */
void AppSetMouseWheelDelta(int delta, int mButton);

/* マウス座標の設定 */
void AppSetMousePosition(int x, int y);

/* 画面解像度の設定 */
void AppSetResolution(int xReso, int yReso);

/* 画面解像度の取得 */
void AppGetResolution(int *xResoRet, int *yResoRet);

/* Camera -> World 変換行列の取得 */
void AppGetMat4x4CameraInWorld(float mat4x4CameraInWorld[4][4]);

/* 前回フレームの Camera -> World 変換行列の取得 */
void AppGetMat4x4PrevCameraInWorld(float mat4x4PrevCameraInWorld[4][4]);


/* ImGui 設定 : Current Status 表示フラグの設定 */
void AppImGuiSetDisplayCurrentStatusFlag(bool flag);

/* ImGui 設定 : Current Status 表示フラグの取得 */
bool AppImGuiGetDisplayCurrentStatusFlag();

/* ImGui 設定 : Camera Settings 表示フラグの設定 */
void AppImGuiSetDisplayCameraSettingsFlag(bool flag);

/* ImGui 設定 : Camera Settings 表示フラグの取得 */
bool AppImGuiGetDisplayCameraSettingsFlag();


/* プリファレンス設定 : グラフィクスシェーダ更新によるリスタート有効化フラグの設定 */
void AppPreferenceSettingsSetEnableAutoRestartByGraphicsShader(bool flag);

/* プリファレンス設定 : グラフィクスシェーダ更新によるリスタート有効化フラグの取得 */
bool AppPreferenceSettingsGetEnableAutoRestartByGraphicsShader();

/* プリファレンス設定 : サウンドシェーダ更新によるリスタート有効化フラグの設定 */
void AppPreferenceSettingsSetEnableAutoRestartBySoundShader(bool flag);

/* プリファレンス設定 : サウンドシェーダ更新によるリスタート有効化フラグの取得 */
bool AppPreferenceSettingsGetEnableAutoRestartBySoundShader();


/* レンダリング設定 : バックバッファ有効化フラグの設定 */
void AppRenderSettingsSetEnableBackBufferFlag(bool flag);

/* レンダリング設定 : バックバッファ有効化フラグの取得 */
bool AppRenderSettingsGetEnableBackBufferFlag();

/* レンダリング設定 : ミップマップ生成有効化フラグの設定 */
void AppRenderSettingsSetEnableMipmapGenerationFlag(bool flag);

/* レンダリング設定 : ミップマップ生成有効化フラグの取得 */
bool AppRenderSettingsGetEnableMipmapGenerationFlag();

/* レンダリング設定 : マルチレンダーターゲット有効化フラグの設定 */
void AppRenderSettingsSetEnableMultipleRenderTargetsFlag(bool flag);

/* レンダリング設定 : マルチレンダーターゲット有効化フラグの取得 */
bool AppRenderSettingsGetEnableMultipleRenderTargetsFlag();

/* レンダリング設定 : 有効レンダーターゲット数の設定 */
void AppRenderSettingsSetNumEnabledRenderTargets(int numEnabledRenderTargets);

/* レンダリング設定 : 有効レンダーターゲット数の取得 */
int AppRenderSettingsGetNumEnabledRenderTargets();

/* レンダリング設定 : ピクセルフォーマットの設定 */
void AppRenderSettingsSetPixelFormat(PixelFormat format);

/* レンダリング設定 : ピクセルフォーマットの取得 */
PixelFormat AppRenderSettingsGetPixelFormat();

/* レンダリング設定 : テクスチャフィルタの設定 */
void AppRenderSettingsSetTextureFilter(TextureFilter filter);

/* レンダリング設定 : テクスチャフィルタの取得 */
TextureFilter AppRenderSettingsGetTextureFilter();

/* レンダリング設定 : テクスチャラップの設定 */
void AppRenderSettingsSetTextureWrap(TextureWrap wrap);

/* レンダリング設定 : テクスチャラップの取得 */
TextureWrap AppRenderSettingsGetTextureWrap();

/* レンダリング設定 : スワップインターバルコントロールフラグの設定 */
void AppRenderSettingsSetEnableSwapIntervalControlFlag(bool flag);

/* レンダリング設定 : スワップインターバルコントロールフラグの取得 */
bool AppRenderSettingsGetEnableSwapIntervalControlFlag();

/* レンダリング設定 : スワップインターバルの設定 */
void AppRenderSettingsSetSwapIntervalControl(SwapInterval interval);

/* レンダリング設定 : スワップインターバルの取得 */
SwapInterval AppRenderSettingsGetSwapIntervalControl();


/* ユーザーテクスチャ : テクスチャの読み込み */
bool AppUserTexturesLoad(int userTextureIndex, const char *fileName);

/* ユーザーテクスチャ : 現在のテクスチャファイル名の取得 */
const char *AppUserTexturesGetCurrentFileName(int userTextureIndex);

/* ユーザーテクスチャ : テクスチャの削除 */
bool AppUserTexturesDelete(int userTextureIndex);


/* カメラ設定 : 座標の設定 */
void AppCameraSettingsSetPosition(const float vec3Pos[3]);

/* カメラ設定 : 座標の取得 */
void AppCameraSettingsGetPosition(float vec3Pos[3]);

/* カメラ設定 : 角度の設定 */
void AppCameraSettingsSetAngleInRadians(const float vec3Ang[3]);

/* カメラ設定 : 角度の取得 */
void AppCameraSettingsGetAngleInRadians(float vec3Ang[3]);

/* カメラ設定 : 画角の設定 */
void AppCameraSettingsSetFovYInRadians(float rad);

/* カメラ設定 : 画角の取得 */
float AppCameraSettingsGetFovYInRadians();


/* スクリーンショットキャプチャ : 現在の出力ファイル名の設定 */
void AppCaptureScreenShotSetCurrentOutputFileName(const char *fileName);

/* スクリーンショットキャプチャ : 現在の出力ファイル名の取得 */
const char *AppCaptureScreenShotGetCurrentOutputFileName();

/* スクリーンショットキャプチャ : 解像度の設定 */
void AppCaptureScreenShotSetResolution(int xReso, int yReso);

/* スクリーンショットキャプチャ : 解像度の取得 */
void AppCaptureScreenShotGetResolution(int *xResoRet, int *yResoRet);

/* スクリーンショットキャプチャ : αチャンネル 1.0 強制置換フラグの設定 */
void AppCaptureScreenShotSetForceReplaceAlphaByOneFlag(bool flag);

/* スクリーンショットキャプチャ : αチャンネル 1.0 強制置換フラグの取得 */
bool AppCaptureScreenShotGetForceReplaceAlphaByOneFlag();

/* スクリーンショットキャプチャ */
void AppCaptureScreenShot();


/* キューブマップキャプチャ : 現在の出力ファイル名の設定 */
void AppCaptureCubemapSetCurrentOutputFileName(const char *fileName);

/* キューブマップキャプチャ : 現在の出力ファイル名の取得 */
const char *AppCaptureCubemapGetCurrentOutputFileName();

/* キューブマップキャプチャ : 解像度の設定 */
void AppCaptureCubemapSetResolution(int reso);

/* キューブマップキャプチャ : 解像度の取得 */
int AppCaptureCubemapGetResolution();

/* キューブマップキャプチャ */
void AppCaptureCubemap();


/* サウンドキャプチャ : 現在の出力ファイル名の設定 */
void AppCaptureSoundSetCurrentOutputFileName(const char *fileName);

/* サウンドキャプチャ : 現在の出力ファイル名の取得 */
const char *AppCaptureSoundGetCurrentOutputFileName();

/* サウンドキャプチャ : 継続時間（秒）の設定 */
void AppCaptureSoundSetDurationInSeconds(float durationInSeconds);

/* サウンドキャプチャ : 継続時間（秒）の取得 */
float AppCaptureSoundGetDurationInSeconds();

/* サウンドキャプチャ */
void AppCaptureSound();


/* exe エクスポート : 現在の出力ファイル名の設定 */
void AppExportExecutableSetCurrentOutputFileName(const char *fileName);

/* exe エクスポート : 現在の出力ファイル名の取得 */
const char *AppExportExecutableGetCurrentOutputFileName();

/* exe エクスポート : 解像度の設定 */
void AppExportExecutableSetResolution(int xReso, int yReso);

/* exe エクスポート : 解像度の取得 */
void AppExportExecutableGetResolution(int *xResoRet, int *yResoRet);

/* exe エクスポート : 継続時間（秒）の設定 */
void AppExportExecutableSetDurationInSeconds(float durationInSeconds);

/* exe エクスポート : 継続時間（秒）の取得 */
float AppExportExecutableGetDurationInSeconds();

/* exe エクスポート : frameCount uniform 有効化フラグの設定 */
void AppExportExecutableSetEnableFrameCountUniformFlag(bool flag);

/* exe エクスポート : frameCount uniform 有効化フラグの取得 */
bool AppExportExecutableGetEnableFrameCountUniformFlag();

/* exe エクスポート : サウンドディスパッチのウェイト実行フラグの設定 */
void AppExportExecutableSetEnableSoundDispatchWaitFlag(bool flag);

/* exe エクスポート : サウンドディスパッチのウェイト実行フラグの取得 */
bool AppExportExecutableGetEnableSoundDispatchWaitFlag();

/* exe エクスポート : ShaderMinifier の no-renaming フラグの設定 */
void AppExportExecutableSetShaderMinifierOptionsNoRenaming(bool flag);

/* exe エクスポート : ShaderMinifier の no-renaming フラグの取得 */
bool AppExportExecutableGetShaderMinifierOptionsNoRenaming();

/* exe エクスポート : ShaderMinifier の no-renaming-list 有効化フラグの設定 */
void AppExportExecutableSetShaderMinifierOptionsEnableNoRenamingList(bool flag);

/* exe エクスポート : ShaderMinifier の no-renaming-list 有効化フラグの取得 */
bool AppExportExecutableGetShaderMinifierOptionsEnableNoRenamingList();

/* exe エクスポート : ShaderMinifier の no-renaming-list の設定 */
void AppExportExecutableSetShaderMinifierOptionsNoRenamingList(const char *noRenamingList);

/* exe エクスポート : ShaderMinifier の no-renaming-list の取得 */
const char *AppExportExecutableGetShaderMinifierOptionsNoRenamingList();

/* exe エクスポート : ShaderMinifier の no-sequence フラグの設定 */
void AppExportExecutableSetShaderMinifierOptionsNoSequence(bool flag);

/* exe エクスポート : ShaderMinifier の no-renaming フラグの取得 */
bool AppExportExecutableGetShaderMinifierOptionsNoSequence();

/* exe エクスポート : ShaderMinifier の smoothstep フラグの設定 */
void AppExportExecutableSetShaderMinifierOptionsSmoothstep(bool flag);

/* exe エクスポート : ShaderMinifier の smoothstep フラグの取得 */
bool AppExportExecutableGetShaderMinifierOptionsSmoothstep();

/* exe エクスポート : Crinkler の CompMode の設定 */
void AppExportExecutableSetCrinklerOptionsCompMode(CrinklerCompMode mode);

/* exe エクスポート : Crinkler の CompMode の取得 */
CrinklerCompMode AppExportExecutableGetCrinklerOptionsCompMode();

/* exe エクスポート : Crinkler の TinyHeader 使用フラグの設定 */
void AppExportExecutableSetCrinklerOptionsUseTinyHeader(bool flag);

/* exe エクスポート : Crinkler の TinyHeader 使用フラグの取得 */
bool AppExportExecutableGetCrinklerOptionsUseTinyHeader();

/* exe エクスポート : Crinkler の TinyImport 使用フラグの設定 */
void AppExportExecutableSetCrinklerOptionsUseTinyImport(bool flag);

/* exe エクスポート : Crinkler の TinyImport 使用フラグの取得 */
bool AppExportExecutableGetCrinklerOptionsUseTinyImport();

/* exe ファイルとしてエクスポート */
void AppExportExecutable();


/* 連番画像保存 : 現在の出力先ディレクトリの設定 */
void AppRecordImageSequenceSetCurrentOutputDirectoryName(const char *directoryName);

/* 連番画像保存 : 現在の出力先ディレクトリの取得 */
const char *AppRecordImageSequenceGetCurrentOutputDirectoryName();

/* 連番画像保存 : 解像度の設定 */
void AppRecordImageSequenceSetResolution(int xReso, int yReso);

/* 連番画像保存 : 解像度の取得 */
void AppRecordImageSequenceGetResolution(int *xResoRet, int *yResoRet);

/* 連番画像保存 : 開始位置（秒）の設定 */
void AppRecordImageSequenceSetStartTimeInSeconds(float startTimeInSeconds);

/* 連番画像保存 : 開始位置（秒）の取得 */
float AppRecordImageSequenceGetStartTimeInSeconds();

/* 連番画像保存 : 継続時間（秒）の設定 */
void AppRecordImageSequenceSetDurationInSeconds(float durationInSeconds);

/* 連番画像保存 : 継続時間（秒）の取得 */
float AppRecordImageSequenceGetDurationInSeconds();

/* 連番画像保存 : FPS の設定 */
void AppRecordImageSequenceSetFramesPerSecond(float framesPerSecond);

/* 連番画像保存 : FPS の取得 */
float AppRecordImageSequenceGetFramesPerSecond();

/* 連番画像保存 : αチャンネル 1.0 強制置換フラグの設定 */
void AppRecordImageSequenceSetForceReplaceAlphaByOneFlag(bool flag);

/* 連番画像保存 : αチャンネル 1.0 強制置換フラグの取得 */
bool AppRecordImageSequenceGetForceReplaceAlphaByOneFlag();

/* 連番画像の保存 */
void AppRecordImageSequence();


/* プロジェクト管理 : 現在のプロジェクトファイル名の取得 */
const char *AppProjectGetCurrentFileName();

/* プロジェクト管理 : インポート */
bool AppProjectImport(const char *fileName);

/* プロジェクト管理 : エクスポート */
bool AppProjectExport(const char *fileName);

/* プロジェクト管理 : 自動エクスポート */
bool AppProjectAutoExport(bool confirm);


/* 強制上書きフラグを設定 */
void AppSetForceOverWriteFlag(bool flag);

/* 強制上書きフラグを取得 */
bool AppGetForceOverWriteFlag();


/* デフォルトディレクトリの取得 */
void AppGetDefaultDirectoryName(char *directoryName, size_t directoryNameSizeInBytes);

/* デフォルトグラフィクスシェーダファイルを開く */
bool AppOpenDefaultGraphicsShader();

/* デフォルトサウンドシェーダファイルを開く */
bool AppOpenDefaultSoundShader();

/* グラフィクスシェーダファイルを開く */
bool AppOpenGraphicsShaderFile(const char *fileName);

/* サウンドシェーダファイルを開く */
bool AppOpenSoundShaderFile(const char *fileName);

/* ドラッグ＆ドロップされたファイルを開く */
bool AppOpenDragAndDroppedFile(const char *fileName);

/* 現在開かれているグラフィクスシェーダファイル名の取得 */
const char *AppGetCurrentGraphicsShaderFileName();

/* 現在開かれているサウンドシェーダファイル名の取得 */
const char *AppGetCurrentSoundShaderFileName();

/* 現在のグラフィクスシェーダコードのポインタを取得 */
const char *AppGetCurrentGraphicsShaderCode();

/* 現在のサウンドシェーダコードのポインタを取得 */
const char *AppGetCurrentSoundShaderCode();

/* 全レンダーターゲットをクリア */
void AppClearAllRenderTargets();

/* リスタート */
void AppRestart();

/* カメラをリセット */
void AppResetCamera();

/* 一時停止 / 再開 */
void AppTogglePauseAndResume();

/* 一時停止 */
void AppPause();

/* 再開 */
void AppResume();

/* スロー送り */
void AppSlowForward();

/* スロー戻し */
void AppSlowBackward();

/* 早送り */
void AppFastForward();

/* 巻き戻し */
void AppFastBackward();

/* アプリケーションの更新（毎フレーム実行）*/
bool AppUpdate();

/* アプリケーションの説明 */
bool AppHelpAbout();

/* アプリケーション初期化 */
bool AppInitialize();

/* アプリケーション終了処理 */
bool AppTerminate();


#endif
