
#include "ScreenShotBase.h"
#include <dwmapi.h>

ScreenShotBase::ScreenShotBase() {
	this->TrimMode = TRIMMING_WINDOW_RECT;
	this->TakenCount = 0;
	HWND hDesktopWindow = GetDesktopWindow();
	GetWindowSize(hDesktopWindow, &DesktopWidth, &DesktopHeight);
}

ScreenShotBase::~ScreenShotBase() { }

// ウィンドウの大きさを取得
void ScreenShotBase::GetWindowSize(HWND hWindow, int *pOutWidth, int *pOutHeight) const {
	RECT windowRect;
	GetWindowRect(hWindow, &windowRect);
	
	if( pOutWidth ) *pOutWidth = windowRect.right - windowRect.left;
	if( pOutHeight ) *pOutHeight = windowRect.bottom - windowRect.top;
	
}

// トリミングするの領域を取得
RECT ScreenShotBase::GetWindowRegion(HWND hCaptureWindow) const {
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
RECT ScreenShotBase::ClampOutOfRegion(const RECT &region) const {
	RECT clampedRegion = region;
	
	// 範囲外の領域は無視
	if( region.left < 0 ) clampedRegion.left = 0;
	if( DesktopWidth < region.right ) clampedRegion.right = DesktopWidth;
	if( region.top < 0 ) clampedRegion.top = 0;
	if( DesktopHeight < region.bottom ) clampedRegion.bottom = DesktopHeight;
	
	return clampedRegion;
	
}


