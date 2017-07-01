
#include "CaptureRegion.h"

/*
 * 撮影範囲基底クラス
*/

CaptureRegionBase::CaptureRegionBase() {
	// デスクトップの大きさを取得
	HWND hDesktopWindow = GetDesktopWindow();
	RECT windowRect;
	GetWindowRect(hDesktopWindow, &windowRect);
	DesktopWidth = windowRect.right - windowRect.left;
	DesktopHeight = windowRect.bottom - windowRect.top;
}

// 画面外の領域をクランプする
void CaptureRegionBase::ClampOutOfRegion(RECT *pInOutRegion) const {
	if( pInOutRegion->left < 0 ) pInOutRegion->left = 0;
	if( DesktopWidth < pInOutRegion->right ) pInOutRegion->right = DesktopWidth;
	if( pInOutRegion->top < 0 ) pInOutRegion->top = 0;
	if( DesktopHeight < pInOutRegion->bottom ) pInOutRegion->bottom = DesktopHeight;
}


/*
 * 撮影範囲: 選択ウィンドウ
*/
void CaptureRegionSelectedWindow::GetCaptureRegionRect(RECT *pOutRegion) {
	if( IsIncludeBorder() ) {
		GetWindowRect(hCaptureWindow, pOutRegion);
		
	} else {
		RECT clientRect;
		GetClientRect(hCaptureWindow, &clientRect);
		
		POINT lt = { 0, 0 };
		ClientToScreen(hCaptureWindow, &lt);
		POINT rb = { clientRect.right, clientRect.bottom };
		ClientToScreen(hCaptureWindow, &rb);
		
		pOutRegion->left = lt.x;
		pOutRegion->top = lt.y;
		pOutRegion->right = rb.x;
		pOutRegion->bottom = rb.y;
		
	}
	
	ClampOutOfRegion(pOutRegion);
	
}


/*
 * 撮影範囲: アクティブウィンドウ
*/
void CaptureRegionActiveWindow::GetCaptureRegionRect(RECT *pOutRegion) {
	HWND hCaptureWindow = GetForegroundWindow();
	
	if( IsIncludeBorder() ) {
		GetWindowRect(hCaptureWindow, pOutRegion);
		
	} else {
		RECT clientRect;
		GetClientRect(hCaptureWindow, &clientRect);
		
		POINT lt = { 0, 0 };
		ClientToScreen(hCaptureWindow, &lt);
		POINT rb = { clientRect.right, clientRect.bottom };
		ClientToScreen(hCaptureWindow, &rb);
		
		pOutRegion->left = lt.x;
		pOutRegion->top = lt.y;
		pOutRegion->right = rb.x;
		pOutRegion->bottom = rb.y;
		
	}
	
	ClampOutOfRegion(pOutRegion);
	
}


/*
 * 撮影範囲: 選択範囲
*/
void CaptureRegionSelectedRegion::GetCaptureRegionRect(RECT *pOutRegion) {
	pOutRegion->left = x;
	pOutRegion->top = y;
	pOutRegion->right = x + width;
	pOutRegion->bottom = y + height;
	ClampOutOfRegion(pOutRegion);
}


/*
 * 撮影範囲: フルスクリーン
*/
void CaptureRegionFullscreen::GetCaptureRegionRect(RECT *pOutRegion) {
	pOutRegion->left = 0;
	pOutRegion->top = 0;
	pOutRegion->right = DesktopWidth;
	pOutRegion->bottom = DesktopHeight;
}

