#pragma once

#include "CaptureBase.h"
#include <memory>
#include <string>
#include <vector>
#include <Windows.h>
#include "ImageWriterBase.h"
#include "TimerExec.h"

/*
 * ウィンドウ部分のトリミング方法を設定
*/
enum TRIMMING_MODE {
	TRIMMING_WINDOW_RECT,
	TRIMMING_WINDOW_RECT_DWM,
	TRIMMING_CLIENT_RECT
};

class ScreenShot2 {
public:
	
	ScreenShot2(
		std::shared_ptr<CaptureBase> &pCap,
		std::shared_ptr<ImageWriterBase> &pWriter,
		const std::string &savePathFormat,
		int waitTimeMillisec,
		TRIMMING_MODE trimMode);
	
	// 連写開始
	void Start() {
		SSCtr = 0;
		pTimer->Start();
	}
	
	// 連写終了
	void Stop() {
		pTimer->Stop();
	}
	
	// 画面キャプチャクラス設定
	void SetCapturer(std::shared_ptr<CaptureBase> &pCap) {
		this->pCap = pCap;
	}
	
	// 画面キャプチャクラス取得
	const std::shared_ptr<CaptureBase> &GetCapturer() const {
		return pCap;
	}
	
	// 画像書き出しクラス設定
	void SetWriter(std::shared_ptr<ImageWriterBase> &pWriter) {
		this->pWriter = pWriter;
	}
	
	// 画像書き出しクラス取得
	const std::shared_ptr<ImageWriterBase> &GetWriter() const {
		return pWriter;
	}
	
	// 画像保存パスフォーマット設定
	void SetSavePathFormat(const std::string &format) {
		this->SavePathFormat = format;
	}
	
	// 画像保存パスフォーマット取得
	const std::string &GetSavePathFormat() const {
		return SavePathFormat;
	}
	
	// 連写間隔設定
	void SetWaitTime(int waitMillisec) {
		pTimer->SetWaitTime(waitMillisec);
	}
	
	// 連写間隔取得
	int GetWaitTime() const {
		return pTimer->GetWaitTime();
	}
	
	// トリミング設定
	void SetTrimmingMode(TRIMMING_MODE mode) {
		this->TrimMode = mode;
	}
	
	// トリミング設定を取得
	TRIMMING_MODE GetTrimmingMode() const {
		return TrimMode;
	}
	
	// 撮影枚数を取得
	int GetTakenCount() const {
		return SSCtr;
	}
	
protected:
	
	// ウィンドウの大きさを取得
	void GetWindowSize(HWND hWindow, int *pOutWidth, int *pOutHeight) const;
	
	// トリミングするの領域を取得
	RECT GetWindowRegion(HWND hCaptureWindow) const;
	
	// 画面外の領域をクランプする
	RECT ClampOutOfRegion(const RECT &region) const;
	
	// スクリーンショット撮影
	void TakeSSFunc(void *ptr);
	
protected:
	int DesktopWidth, DesktopHeight;
	std::shared_ptr<CaptureBase> pCap;
	std::shared_ptr<ImageWriterBase> pWriter;
	TRIMMING_MODE TrimMode;
	std::unique_ptr<TimerExec> pTimer;
	std::string SavePathFormat;
	int SSCtr;
	std::vector<unsigned char> vecImgBuffer;
};

