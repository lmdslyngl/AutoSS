
#include "BitBltCapture.h"

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
	CapturedWidth = 0;
	CapturedHeight = 0;
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
	bmi.bmiHeader.biBitCount = 32;
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
void BitBltCapture::CaptureRegion(const RECT *region) {
	BitBlt(hSSBitmapMemDC, 0, 0, SSBitmapWidth, SSBitmapHeight,
		hDesktopDC, 0, 0, SRCCOPY);
	
	CapturedWidth = region->right - region->left;
	CapturedHeight = region->bottom - region->top;
	
	CapturedImageRGB.resize(CapturedWidth * CapturedHeight * 3);
	const unsigned char *src = (unsigned char*)pSSBitmapPixels;
	unsigned char *dst = CapturedImageRGB.data();
	
	// BGRX -> RGB
	for( int y = 0; y < CapturedHeight; y++ ) {
		const unsigned char *srcrow =
			&src[((region->top + y) * SSBitmapWidth + region->left) * 4];
		for( int x = 0; x < CapturedWidth; x++ ) {
			dst[0] = srcrow[2];
			dst[1] = srcrow[1];
			dst[2] = srcrow[0];
			dst += 3;
			srcrow += 4;
		}
	}
	
}

// 画像データを取得
const unsigned char *BitBltCapture::GetData() const {
	return CapturedImageRGB.data();
}

// 画像データの長さを取得
unsigned int BitBltCapture::BitBltCapture::GetDataLength() const {
	return CapturedImageRGB.size();
}

// 画像のサイズを取得
void BitBltCapture::GetImageSize(int *pOutWidth, int *pOutHeight) {
	if( pOutWidth ) *pOutWidth = CapturedWidth;
	if( pOutHeight ) *pOutHeight = CapturedHeight;
}


