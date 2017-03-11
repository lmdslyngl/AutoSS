
#include "ScreenShot2.h"
#include <sstream>
#include <fstream>
#include <dwmapi.h>

ScreenShot2::ScreenShot2(
	std::shared_ptr<CaptureBase> &pCap,
	std::shared_ptr<ImageWriterBase> &pWriter,
	const std::string &savePathFormat,
	int waitTimeMillisec,
	TRIMMING_MODE trimMode)
{
	
	this->pCap = pCap;
	this->pWriter = pWriter;
	this->SavePathFormat = savePathFormat;
	this->TrimMode = trimMode;
	
	HWND hDesktopWindow = GetDesktopWindow();
	GetWindowSize(hDesktopWindow, &DesktopWidth, &DesktopHeight);
	
	pTimer = std::make_unique<TimerExec>(
		[this](void *ptr) { this->TakeSSFunc(ptr); },
		waitTimeMillisec);
	
}


// ウィンドウの大きさを取得
void ScreenShot2::GetWindowSize(HWND hWindow, int *pOutWidth, int *pOutHeight) const {
	RECT windowRect;
	GetWindowRect(hWindow, &windowRect);
	
	if( pOutWidth ) *pOutWidth = windowRect.right - windowRect.left;
	if( pOutHeight ) *pOutHeight = windowRect.bottom - windowRect.top;
	
}

// トリミングするの領域を取得
RECT ScreenShot2::GetWindowRegion(HWND hCaptureWindow) const {
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

// 画面外に出た領域をクランプする
RECT ScreenShot2::ClampOutOfRegion(const RECT &region) const {
	RECT clampedRegion = region;
	
	// 範囲外の領域は無視
	if( region.left < 0 ) clampedRegion.left = 0;
	if( DesktopWidth < region.right ) clampedRegion.right = DesktopWidth;
	if( region.top < 0 ) clampedRegion.top = 0;
	if( DesktopHeight < region.bottom ) clampedRegion.bottom = DesktopHeight;
	
	return clampedRegion;
	
}


// スクリーンショット撮影
void ScreenShot2::TakeSSFunc(void *ptr) {
	HWND hCaptureWindow = GetForegroundWindow();
	RECT windowRect = GetWindowRegion(hCaptureWindow);
	windowRect = ClampOutOfRegion(windowRect);
	
	pCap->CaptureRegion(&windowRect);
	
	int sswidth, ssheight;
	pCap->GetImageSize(&sswidth, &ssheight);
	
	char savename[128];
	sprintf_s(savename, SavePathFormat.c_str(), SSCtr);
	
	pWriter->Write(savename, sswidth, ssheight,
		pCap->GetData(), pCap->GetDataLength());
	
	SSCtr++;
	
}


