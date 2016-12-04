
#include "ScreenShot2.h"
#include <sstream>
#include <fstream>
#include <dwmapi.h>

ScreenShot2::ScreenShot2(std::shared_ptr<CaptureBase> &pCap) {
	this->pCap = pCap;
	HWND hDesktopWindow = GetDesktopWindow();
	GetWindowSize(hDesktopWindow, &DesktopWidth, &DesktopHeight);
}

// スクリーンショット撮影
void ScreenShot2::TakeScreenShot(HWND hCaptureWindow) {
	if( !hCaptureWindow ) return;
	
	RECT windowRect = GetWindowRegion(hCaptureWindow);
	
	// 範囲外の領域は無視
	if( windowRect.left < 0 ) windowRect.left = 0;
	if( DesktopWidth < windowRect.right ) windowRect.right = DesktopWidth;
	if( windowRect.top < 0 ) windowRect.top = 0;
	if( DesktopHeight < windowRect.bottom ) windowRect.bottom = DesktopHeight;
	
	pCap->CaptureRegion(&windowRect);
	
}

// PPM書き出し
void ScreenShot2::WriteToPPM(const std::string &filename) {
	int width, height;
	pCap->GetImageSize(&width, &height);
	
	std::ofstream ofs(filename, std::ios::binary | std::ios::trunc);
	
	std::stringstream ss;
	ss << "P6\n"
		<< width << " " << height << "\n"
		<< 255 << "\n";
	std::string header = ss.str();
	ofs.write(header.c_str(), header.size());
	ofs.write((const char*)pCap->GetData(), pCap->GetDataLength());
	
	ofs.close();
	
}

// PNG書き出し
void ScreenShot2::WriteToPNG(const std::string &filename) {
	
}

// ウィンドウの大きさを取得
void ScreenShot2::GetWindowSize(HWND hWindow, int *pOutWidth, int *pOutHeight) {
	RECT windowRect;
	GetWindowRect(hWindow, &windowRect);
	
	if( pOutWidth ) *pOutWidth = windowRect.right - windowRect.left;
	if( pOutHeight ) *pOutHeight = windowRect.bottom - windowRect.top;
	
}

// トリミングするの領域を取得
RECT ScreenShot2::GetWindowRegion(HWND hCaptureWindow) {
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



