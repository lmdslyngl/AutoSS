#pragma once

#include "CaptureBase.h"
#include <memory>
#include <vector>
#include <Windows.h>
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
	virtual void CaptureRegion(const RECT *region);
	
	// 画像データを取得
	virtual const unsigned char *GetData() const;
	
	// 画像データの長さを取得
	virtual size_t GetDataLength() const;
	
	// 画像のサイズを取得
	virtual void GetImageSize(int *pOutWidth, int *pOutHeight);
	
protected:
	HINSTANCE hInstance;
	std::shared_ptr<DummyWindow> pDummyWindow;
	HWND hDesktop;
	HDC hDesktopDC;
	HBITMAP hSSBitmap;
	LPDWORD pSSBitmapPixels;
	int SSBitmapWidth, SSBitmapHeight;
	HDC hSSBitmapMemDC;
	std::vector<unsigned char> CapturedImageBGR;
	int CapturedWidth, CapturedHeight;
};

