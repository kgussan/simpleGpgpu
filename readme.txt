日本語解説付きシンプルなGPGPU (hlslシェーダ) サンプル
kgussan@gmail.com
================================================================
2014-09 初版
2020-11 2版　Windows 10 とVS2019 対応更新　@CTsuchinokoさんによるVS2019対応

コンスタントバッファを使用するシンプルなGPGPUサンプル

●概要
	CPUとGPUで加算、乗算を行う。
	計算終了後、CPU計算結果とGPU計算結果を比較する。
	結果をCPUで取得してテキストで結果を出力。

	指定したindex 以降は、
	入力したindexに定数バッファで指定した数を加算して出力。

●動機
	これがあれば結構楽にGPGPUが書けるかもしれないと思い公開します。
	DirectX11が動作するPCであれば動きます。
	おそらくDirectX SDK とWidows SDKはインストールしておいたほうがいいかと思います。
	レイトレ合宿でできた副産物。

----------------------------------------------------------------
注意事項

●MITライセンス
	制限の緩いライセンスのつもりで選択。

●お断り
	プログラムを動作させたことによる一切の責任を負いません。

----------------------------------------------------------------
参考

●以下のソースを参考にした。
	Asif BahrainwalaさんのCODE Project の記事から
	http://www.codeproject.com/Articles/42612/DirectX-Compute-Shaders
	このソースのライセンスは氏のものに準じる。要約すると商用利用OK。コード保証なし。
	http://www.codeproject.com/info/cpol10.aspx

●確認環境
・初版
	1)
	Windows7
	Visual Studio 2010
	Microsoft DirectX SDK (June 2010)
	2)
	Windows8.1
	Visual Studio 2010
	Microsoft DirectX SDK (June 2010)
	Windows SDK 8.1 ※注意　windows 8.1で動作に必須
・第2版
	Windows 10 
	Visual Studio 2019

●HLSLシェーダコンパイル
	コンピュートシェーダは実行速度向上のため、従来の互換性の取れる形にするため、
	オフラインコンパイルすることもできる。
	fxc コンパイラで次のようにしてバイナリを作る
	> $(DXSDK_DIR)/Utilities/bin/x64/fxc /T cs_5_0 /Fo simpleGpgpu.cfx simpleGpgpu.hlsl 

	$(DXSDK_DIR)の例
		c:\Program Files (x86)\Microsoft DirectX SDK (June 2010)
	上記bin パスを環境変数に指定すると便利。
	参考：	http://marupeke296.com/DXPS_No10_CreatePricompileFXFile.html

●dispatch単位
	AMDは実行スレッドの管理単位が64スレッド(2016年当時、2020年のAMD GPUは32のはず)
	NVIDIAは32スレッド。
	両社の最小公倍数の64単位でスレッドを起動している。
	あまり小さい単位でスレッドを呼ぶと十分にGPUパイプを充填できず、
	稼働率が下がってしまうため。
	INTELのブランチ粒度は不明。GPU branche granularity などで検索すると詳細が出ると思います。
