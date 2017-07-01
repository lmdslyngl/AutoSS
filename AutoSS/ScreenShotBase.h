#pragma once

#include <Windows.h>
#include <memory>
#include <string>
#include <functional>

#include "CaptureBase.h"
#include "ImageWriterBase.h"
#include "CaptureRegion.h"

/*
 * スクリーンショット基底クラス
*/
class ScreenShotBase {
public:

	ScreenShotBase() {
		this->TakenCount = 0;
		this->CapturedFPS = 0.0;
	}
	virtual ~ScreenShotBase() = default;
	
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
	void SetSavePathFormat(const std::wstring &format) {
		this->SavePathFormat = format;
	}
	
	// 画像保存パスフォーマット取得
	const std::wstring &GetSavePathFormat() const {
		return SavePathFormat;
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
	
	// キャプチャ領域を設定
	void SetCaptureRegion(const std::shared_ptr<CaptureRegionBase> &pRegion) {
		this->pCapRegion = pRegion;
	}
	
	// キャプチャ領域を取得
	const std::shared_ptr<CaptureRegionBase> &GetCaptureRetion() const {
		return pCapRegion;
	}
	
protected:
	int DesktopWidth, DesktopHeight;
	std::shared_ptr<CaptureBase> pCap;
	std::shared_ptr<ImageWriterBase> pWriter;
	std::wstring SavePathFormat;
	int TakenCount;
	std::function<void()> OnFinishedFunc;
	double CapturedFPS;
	std::shared_ptr<CaptureRegionBase> pCapRegion;
};

