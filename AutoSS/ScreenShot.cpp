
#include <fstream>
#include <sstream>
#include <png.h>
#include "ScreenShot.h"
#include <chrono>
#include <iostream>
#include <dwmapi.h>

/**
 * スクリーンショットクラス
*/
ScreenShot::~ScreenShot() {
	DeleteDC(hSSBitmapMemDC);
	DeleteObject(hSSBitmap);
	ReleaseDC(hDesktop, hDesktopDC);
}

// 初期化
bool ScreenShot::Setup(HINSTANCE hInstance, std::shared_ptr<DummyWindow> pWindow) {
	this->hInstance = hInstance;
	hDesktop = GetDesktopWindow();
	hDesktopDC = GetDC(hDesktop);
	
	if( pWindow ) {
		pDummyWindow = pWindow;
	} else {
		pDummyWindow = std::make_shared<DummyWindow>();
		if( !pDummyWindow->Setup(hInstance, nullptr) ) return false;
	}
	hWindowDummy = pDummyWindow->GetWindowHandle();
	
	if( !CreateSSBitmap() ) return false;
	
	SetTrimmingMode(TRIMMING_WINDOW_RECT);
	
	return true;
	
}

// スクリーンショット撮影
void ScreenShot::TakeScreenShot(HWND hCaptureWindow) {
	if( !hCaptureWindow ) return;
	
	RECT windowRect = GetWindowRegion(hCaptureWindow);
	
	// 範囲外の領域は無視
	if( windowRect.left < 0 ) windowRect.left = 0;
	if( SSBitmapWidth < windowRect.right ) windowRect.right = SSBitmapWidth;
	if( windowRect.top < 0 ) windowRect.top = 0;
	if( SSBitmapHeight < windowRect.bottom ) windowRect.bottom = SSBitmapHeight;
	
	CapturedWidth = windowRect.right - windowRect.left;
	CapturedHeight = windowRect.bottom - windowRect.top;
	
	auto start = std::chrono::system_clock::now();
	BitBlt(hSSBitmapMemDC, 0, 0, SSBitmapWidth, SSBitmapHeight,
		hDesktopDC, 0, 0, SRCCOPY);
	auto end = std::chrono::system_clock::now();
	
	unsigned char *pixels = (unsigned char*)pSSBitmapPixels;
	CapturedImage.resize(CapturedWidth * CapturedHeight * 3);
	
	unsigned char *incursor =
		&pixels[(windowRect.top * SSBitmapWidth + windowRect.left) * 4];
	unsigned char *outcursor = &CapturedImage[0];
	
	// BGRX -> RGB
	for( int y = 0; y < CapturedHeight; y++ ) {
		incursor = &pixels[((windowRect.top + y) * SSBitmapWidth + windowRect.left) * 4];
		for( int x = 0; x < CapturedWidth; x++ ) {
			outcursor[0] = incursor[2];
			outcursor[1] = incursor[1];
			outcursor[2] = incursor[0];
			outcursor += 3;
			incursor += 4;
		}
	}
	
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << elapsed.count() << " ms\n";
	
}

// PPM書き出し
void ScreenShot::WriteToPPM(const std::string &filename) {
	std::ofstream ofs(filename, std::ios::binary | std::ios::trunc);
	
	std::stringstream ss;
	ss << "P6\n"
		<< CapturedWidth << " " << CapturedHeight << "\n"
		<< 255 << "\n";
	std::string header = ss.str();
	ofs.write(header.c_str(), header.size());
	
	ofs.write((const char*)&CapturedImage[0], CapturedImage.size());
	
	ofs.close();
	
}

// PNG書き出し
bool ScreenShot::WriteToPNG(const std::string &filename) {
	FILE *fp = nullptr;
	fopen_s(&fp, filename.c_str(), "wb");
	if( !fp ) return false;
	
	// init libpng
	png_structp pStruct = png_create_write_struct(
		PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if( !pStruct ) return false;
	
	png_infop pInfo = png_create_info_struct(pStruct);
	if( !pInfo ) return false;
	
	png_init_io(pStruct, fp);
	
	// header
	png_set_IHDR(pStruct, pInfo,
		CapturedWidth, CapturedHeight,
		8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	
	png_write_info(pStruct, pInfo);
	
	// write image
	unsigned char *row = &CapturedImage[0];
	for( int i = 0; i < CapturedHeight; i++ ) {
		png_write_row(pStruct, row);
		row += CapturedWidth * 3;
	}
	
	png_write_end(pStruct, nullptr);
	
	// cleanup
	png_destroy_write_struct(&pStruct, &pInfo);
	fclose(fp);
	
	return true;
	
}

// スクリーンショット保管用のビットマップを作成
bool ScreenShot::CreateSSBitmap() {
	GetWindowSize(hDesktop, &SSBitmapWidth, &SSBitmapHeight);
	
	BITMAPINFO bmi;
	ZeroMemory(&bmi, sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = SSBitmapWidth;
	bmi.bmiHeader.biHeight = -SSBitmapHeight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	
	HDC hDC = GetDC(hWindowDummy);
	hSSBitmap  = CreateDIBSection(
		hDC, &bmi, DIB_RGB_COLORS,
		(void**)&pSSBitmapPixels, nullptr, 0
	);
	hSSBitmapMemDC = CreateCompatibleDC(hDC);
	SelectObject(hSSBitmapMemDC, hSSBitmap);
	ReleaseDC(hWindowDummy, hDC);
	
	return true;
	
}

// ウィンドウの大きさを取得
void ScreenShot::GetWindowSize(HWND hWindow, int *pOutWidth, int *pOutHeight) {
	RECT windowRect;
	GetWindowRect(hWindow, &windowRect);
	
	if( pOutWidth ) *pOutWidth = windowRect.right - windowRect.left;
	if( pOutHeight ) *pOutHeight = windowRect.bottom - windowRect.top;
	
}

// トリミングするの領域を取得
RECT ScreenShot::GetWindowRegion(HWND hCaptureWindow) {
	RECT region;
	
	switch( TrimMode ) {
	case TRIMMING_WINDOW_RECT:
		GetWindowRect(hCaptureWindow, &region);
		break;
		
	case TRIMMING_WINDOW_RECT_DWM:
		DwmGetWindowAttribute(
			hCaptureWindow, DWMWA_EXTENDED_FRAME_BOUNDS,
			&region, sizeof(RECT));
		break;
		
	case TRIMMING_CLIENT_RECT:
		RECT clientRect;
		GetClientRect(hCaptureWindow, &clientRect);
		
		POINT lt = { 0, 0 };
		ClientToScreen(hCaptureWindow, &lt);
		POINT rb = { clientRect.right, clientRect.bottom };
		ClientToScreen(hCaptureWindow, &rb);
		
		region.left = lt.x;
		region.top = lt.y;
		region.right = rb.x;
		region.bottom = rb.y;
		
		break;
		
	}
	
	return region;
	
}



