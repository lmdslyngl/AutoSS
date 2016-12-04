#pragma once

#include "CaptureBase.h"
#include <memory>
#include <string>
#include <vector>
#include <Windows.h>


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
	
	ScreenShot2(std::shared_ptr<CaptureBase> &pCap);
	
	// スクリーンショット撮影
	void TakeScreenShot(HWND hCaptureWindow);
	
	// PPM書き出し
	void WriteToPPM(const std::string &filename);

	// PNG書き出し
	void WriteToPNG(const std::string &filename);
	
	// トリミング設定
	void SetTrimmingMode(TRIMMING_MODE mode) {
		this->TrimMode = mode;
	}
	
	// トリミング設定を取得
	TRIMMING_MODE GetTrimmingMode() const {
		return TrimMode;
	}
	
protected:
	
	// ウィンドウの大きさを取得
	void GetWindowSize(HWND hWindow, int *pOutWidth, int *pOutHeight);
	
	// トリミングするの領域を取得
	RECT GetWindowRegion(HWND hCaptureWindow);
	
protected:
	int DesktopWidth, DesktopHeight;
	std::shared_ptr<CaptureBase> pCap;
	TRIMMING_MODE TrimMode;
};

