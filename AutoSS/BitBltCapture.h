#pragma once

#include "CaptureBase.h"
#include <memory>
#include <vector>
#include <Windows.h>
#include <chrono>
#include "DummyWindow.h"

/*
 * BitBltを用いたキャプチャ
*/
class BitBltCapture : public CaptureBase {
public:
	
	BitBltCapture();
	virtual ~BitBltCapture();
	
	// 初期化
	void Setup(HINSTANCE hInstance, std::shared_ptr<DummyWindow> pWindow);
	
	// 領域のキャプチャを行う
	virtual void CaptureRegion(
		const RECT *region,
		unsigned char *pOutBuffer, unsigned int bufferLength,
		int *pOutCapturedWidth, int *pOutCapturedHeight);
	
protected:
	HINSTANCE hInstance;
	std::shared_ptr<DummyWindow> pDummyWindow;
	HWND hDesktop;
	HDC hDesktopDC;
	HBITMAP hSSBitmap;
	LPDWORD pSSBitmapPixels;
	int SSBitmapWidth, SSBitmapHeight;
	HDC hSSBitmapMemDC;
};

