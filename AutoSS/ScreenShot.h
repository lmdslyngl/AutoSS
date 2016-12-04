#pragma once

#include <Windows.h>
#include <vector>
#include <string>
#include <memory>
#include "DummyWindow.h"
#include "CaptureBase.h"

/*
 * ウィンドウ部分のトリミング方法を設定
*/
enum TRIMMING_MODE {
	TRIMMING_WINDOW_RECT,
	TRIMMING_WINDOW_RECT_DWM,
	TRIMMING_CLIENT_RECT
};

/**
 * スクリーンショットクラス
*/
class ScreenShot {
public:
	
	ScreenShot() = default;
	~ScreenShot();
	
	// 初期化
	bool Setup(HINSTANCE hInstance, std::shared_ptr<DummyWindow> pWindow);
	
	// トリミング設定
	void SetTrimmingMode(TRIMMING_MODE mode) {
		this->TrimMode = mode;
	}
	
	// トリミング設定を取得
	TRIMMING_MODE GetTrimmingMode() const {
		return TrimMode;
	}
	
	// スクリーンショット撮影
	void TakeScreenShot(HWND hCaptureWindow);
	
	// PPM書き出し
	void WriteToPPM(const std::string &filename);
	
	// PNG書き出し
	bool WriteToPNG(const std::string &filename);
	
protected:
	
	// スクリーンショット保管用のビットマップを作成
	bool CreateSSBitmap();
	
	// ウィンドウの大きさを取得
	void GetWindowSize(HWND hWindow, int *pOutWidth, int *pOutHeight);
	
	// トリミングするの領域を取得
	RECT GetWindowRegion(HWND hCaptureWindow);
	
protected:
	HINSTANCE hInstance;
	std::shared_ptr<DummyWindow> pDummyWindow;
	HWND hWindowDummy;
	HWND hDesktop;
	HDC hDesktopDC;
	HBITMAP hSSBitmap;
	LPDWORD pSSBitmapPixels;
	int SSBitmapWidth, SSBitmapHeight;
	HDC hSSBitmapMemDC;
	
	std::vector<unsigned char> CapturedImage;
	int CapturedWidth, CapturedHeight;
	
	TRIMMING_MODE TrimMode;
	
};


