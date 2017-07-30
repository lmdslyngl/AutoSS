
#include "BitBltCapture.h"
#include <assert.h>
#include "Logger.h"

/*
 * BitBltを用いたキャプチャ
*/

BitBltCapture::BitBltCapture() {
	hInstance = nullptr;
	hDesktop = nullptr;
	hDesktopDC = nullptr;
	hSSBitmap = nullptr;
	pSSBitmapPixels = nullptr;
	SSBitmapWidth = 0;
	SSBitmapHeight = 0;
	hSSBitmapMemDC = nullptr;
}

BitBltCapture::~BitBltCapture() {
	if( hSSBitmapMemDC ) DeleteDC(hSSBitmapMemDC);
	if( hSSBitmap ) DeleteObject(hSSBitmap);
	if( hDesktopDC ) ReleaseDC(hDesktop, hDesktopDC);
}

// 初期化
void BitBltCapture::Setup(HINSTANCE hInstance, std::shared_ptr<DummyWindow> pWindow) {
	this->hInstance = hInstance;
	hDesktop = GetDesktopWindow();
	hDesktopDC = GetDC(hDesktop);
	
	if( pWindow ) {
		pDummyWindow = pWindow;
	} else {
		// ダミーウィンドウが渡されなかったら自前で作成
		pDummyWindow = std::make_shared<DummyWindow>();
		if( !pDummyWindow->Setup(hInstance, nullptr) ) {
			throw std::exception("Failed to Setup dummywindow");
		}
	}
	HWND hWindowDummy = pDummyWindow->GetWindowHandle();
	
	// ビットマップの大きさを計算
	RECT desktopRect;
	GetWindowRect(hDesktop, &desktopRect);
	SSBitmapWidth = desktopRect.right - desktopRect.left;
	SSBitmapHeight = desktopRect.bottom - desktopRect.top;
	
	// スクリーンショット保管用のビットマップを作成
	BITMAPINFO bmi;
	ZeroMemory(&bmi, sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = SSBitmapWidth;
	bmi.bmiHeader.biHeight = -SSBitmapHeight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biCompression = BI_RGB;

	HDC hDC = GetDC(hWindowDummy);
	hSSBitmap = CreateDIBSection(
		hDC, &bmi, DIB_RGB_COLORS,
		(void**)&pSSBitmapPixels, nullptr, 0
	);
	hSSBitmapMemDC = CreateCompatibleDC(hDC);
	SelectObject(hSSBitmapMemDC, hSSBitmap);
	ReleaseDC(hWindowDummy, hDC);
	
}

// 領域のキャプチャを行う
void BitBltCapture::CaptureRegion(
	const RECT *region,
	unsigned char *pOutBuffer, unsigned int bufferLength,
	int *pOutCapturedWidth, int *pOutCapturedHeight)
{

	assert(region != nullptr);
	assert(pOutBuffer != nullptr);
	assert(pOutCapturedWidth != nullptr && pOutCapturedHeight != nullptr);
	
	auto timerStart = std::chrono::system_clock::now();
	
	int capturedWidth = region->right - region->left;
	int capturedHeight = region->bottom - region->top;
	unsigned int necessaryBufLen = capturedWidth * capturedHeight * 3;
	assert(necessaryBufLen <= bufferLength);
	
	BitBlt(hSSBitmapMemDC, 0, 0, capturedWidth, capturedHeight,
		hDesktopDC, region->left, region->top, SRCCOPY);
	
	const unsigned char *src = (unsigned char*)pSSBitmapPixels;
	unsigned char *dst = pOutBuffer;
	
	// BGRX -> BGR
	for( int y = 0; y < capturedHeight; y++ ) {
		memcpy(dst, src, capturedWidth * 3);
		src += SSBitmapWidth * 3;
		dst += capturedWidth * 3;
	}
	
	*pOutCapturedWidth = capturedWidth;
	*pOutCapturedHeight = capturedHeight;
	
	auto timerEnd = std::chrono::system_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(timerEnd - timerStart);
	GlbLog::GetLogger().Write(L"CaptureRegion: %d ms", duration.count());
	
}



