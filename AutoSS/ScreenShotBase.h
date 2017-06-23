#pragma once

#include <Windows.h>
#include <memory>
#include <string>
#include <functional>

#include "CaptureBase.h"
#include "ImageWriterBase.h"

/*
 * ウィンドウ部分のトリミング方法を設定
*/
enum TRIMMING_MODE {
	TRIMMING_WINDOW_RECT,
	TRIMMING_WINDOW_RECT_DWM,
	TRIMMING_CLIENT_RECT
};

/*
 * スクリーンショット基底クラス
*/
class ScreenShotBase {
public:
	
	ScreenShotBase();
	virtual ~ScreenShotBase();
	
	// 撮影開始
	virtual void Start() = 0;
	
	// 撮影終了
	virtual void Stop() = 0;
	
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
		return TakenCount;
	}
	
	// 撮影終了コールバック関数を設定
	void SetOnFinishedFunc(std::function<void()> func) {
		this->OnFinishedFunc = func;
	}
	
	// 撮影FPSを取得
	double GetCapturedFPS() const {
		return CapturedFPS;
	}
	
protected:
	
	// ウィンドウの大きさを取得
	void GetWindowSize(HWND hWindow, int *pOutWidth, int *pOutHeight) const;
	
	// トリミングするの領域を取得
	RECT GetWindowRegion(HWND hCaptureWindow) const;
	
	// 画面外の領域をクランプする
	RECT ClampOutOfRegion(const RECT &region) const;
	
protected:
	int DesktopWidth, DesktopHeight;
	std::shared_ptr<CaptureBase> pCap;
	std::shared_ptr<ImageWriterBase> pWriter;
	std::string SavePathFormat;
	TRIMMING_MODE TrimMode;
	int TakenCount;
	std::function<void()> OnFinishedFunc;
	double CapturedFPS;
};

