# MinimalGL


# 解説

PC Intro 作成ツールです。
GLSL に対応しています。

PC Intro とは、数キロバイト程度の小さな実行ファイルで映像と音を生成し、
その内容で優劣を競うメガデモ（デモシーン）のカテゴリです。  
PC Intro の実行ファイルは、単体で動作することが要求されます。
外部ファイルやネットワーク上のデータを参照することは禁止されています（例外として OS に標準装備されている dll などの利用は許可されています）。
映像と音いずれも実行時に何らかのアルゴリズムにより生成する必要があります。
そして、いかにファイルサイズを小さくするかが重要です。  

PC Intro には、
ファイルサイズ 1024 バイト未満の PC 1K Intro、
4096 バイト未満の PC 4K Intro、
8192 バイト未満の PC 8K Intro、
65536 バイト未満の PC 64K Intro
などのサブカテゴリがあります。
MinimalGL は、このうち PC 4K Intro の作成に特化したツールです。


# スクリーンショット
![screen_shot_gfx](https://user-images.githubusercontent.com/11882108/82467562-c1ad0800-9afc-11ea-8582-5ef5dbdf2e0f.png)
![screen_shot_snd](https://user-images.githubusercontent.com/11882108/82468262-8fe87100-9afd-11ea-8f94-ebf531cb53be.png)


# 準備

MinimalGL を利用するには、
事前に以下のツールをパスの通ったディレクトリにインストールする必要があります。

1. shader_minifier.exe  
	https://github.com/laurentlb/Shader_Minifier/releases


2. crinkler.exe  
	http://www.crinkler.net/


# 簡易チュートリアル

MinimalGL を使った PC 4K Intro 作成の簡単な流れは以下のようになります。

1. シェーダファイルを用意する  
	example/ 以下の適当な *.gfx.glsl *.snd.glsl ファイルからコピーしてください。

2. グラフィクスシェーダファイルを開く  
	メインウィンドウに \*.gfx.glsl ファイルをドラッグ＆ドロップするか、
	メニューから [File]→[Open Graphics Shader] を選択し \*.gfx.glsl ファイルを読み込みます。

3. サウンドシェーダファイルを開く  
	メインウィンドウに \*.snd.glsl ファイルをドラッグ＆ドロップするか、
	メニューから [File]→[Open Sound Shader] を選択し \*.snd.glsl ファイルを読み込みます。

4. シェーダのエディット  
	MinimalGL は、シェーダのエディット機能を持ちません。
	シェーダのエディットは、ユーザーお手持ちのテキストエディタ等で行ってください。
	現在描画中（再生中）の glsl ファイルのタイムスタンプが更新されると、
	直ちにリコンパイルされ描画結果（再生結果）に反映されます。

5. 実行ファイルにエクスポート  
	メニューから [File]→[Export Executable] を選択します。
	Output file に生成する実行ファイル名を指定し、OK をクリックします。
	エクスポートに成功すると、ファイルサイズがメッセージボックスで表示されます。

6. minify  
	ファイルサイズが 4096 バイト未満になっていない場合、ファイルサイズ削減作業（minify）を行います。
	実行ファイルエクスポート時に同時に生成される minify されたシェーダコード（\*.inl ファイル）、
	圧縮結果のレポート（\*.crinkler_report.html）を参考にしつつ、シェーダコードを短くしていきます。
	実行ファイルエクスポート時の設定を調整したり、
	描画設定（[Setup]→[Render Settings]）から無駄な機能を off にすることでもファイルサイズを削減できます。

7. 完成  


# 機能一覧

- シェーダよるグラフィクス描画  
	画面全体に一枚の四角形ポリゴンを表示し、フラグメントシェーダにより描画を行います。

- シェーダよるサウンド生成  
	コンピュートシェーダによるサウンド生成を行います。

- シェーダホットリロード  
	シェーダファイルが更新されると直ちに自動リロードを行います。
	現在時刻（uniform float time）をリセットせずにリロードする機能も提供されています（ライブコーディング用途を想定）。

- 実行ファイルエクスポート  
	現在のグラフィクス及びサウンドの内容を実行ファイルにエクスポートします。
	shader_minifier (https://github.com/laurentlb/Shader_Minifier) によるシェーダコード短縮化、
	および crinkler (http://www.crinkler.net/) による実行ファイル圧縮が適用されます。

- プロジェクトファイルエクスポート/インポート  
	現在の状態（現在のシェーダファイル名、カメラの位置、描画設定、エクスポート設定等々）をプロジェクトファイルにエクスポートします。
	プロジェクトファイルをインポートすることで状態を復元できます。

- スクリーンショットキャプチャ  
	Unorm8 RGBA フォーマットの png ファイルとしてスクリーンショットをキャプチャします。

- カメラコントロール  
	マウスによるカメラ操作機能を提供します（利用するかはオプショナル）。

- キューブマップキャプチャ  
	カメラコントロールを利用している場合、
	現在のカメラ位置から見える全方位の状態をキューブマップとして FP32 RGBA フォーマットの dds ファイルにキャプチャできます。

- サウンドキャプチャ  
	サウンド生成結果を float 2ch 形式の wav ファイルに保存します。

- 連番画像保存  
	グラフィクス生成結果を Unorm8 RGBA フォーマットの連番 png ファイルとして保存します。

- 一時停止、スロー再生/スロー巻き戻し、早送り/巻き戻し  


# グラフィクス周り機能一覧

- バックバッファテクスチャの利用  
	前回フレームの描画結果をテクスチャとして利用できます。

- マルチプルレンダーターゲット (MRT)  
	MRT4 まで対応します。バックバッファテクスチャも MRT 枚数分利用できます。

- ミップマップ生成  
	バックバッファテクスチャをミップマップ化します。

- LDR/HDR レンダリング  
	LDR (Unorm8 RGBA) および HDR (FP16 FP32 RGBA) でのレンダリングに対応します。


# 制限事項

MinimalGL には以下の制限事項があります。

- windows exe 専用です。

- バーテクスシェーダは利用できません。

- サウンドはコンピュートシェーダによる生成のみに対応します。


# ライセンス

- 外部由来のもの

	- src/external/cJSON 以下

		cJSON のライセンスに従います。

	- src/external/stb 以下

		stb のライセンスに従います。

	- src/GL src/KHR 以下

		ソースコード中に書かれたライセンスに従います。

	- examples/ 以下

		twigl 互換サンプルコードに含まれる main 関数は twigl のデフォルトシェーダを流用しています。


- 上記以外

	(C) Yosshin (aka 0x4015)  
	Apache License Version 2.0 が適用されます。  
	This software includes the work that is distributed in the Apache License 2.0


# 謝辞 Special Thanks

- Mentor/TBC and Blueberry/Loonies

	The creators of Crinkler, a great compression tool.


- LLB/Ctrl-Alt-Test

	The creator of Shader Minifier, a great minify tool.

