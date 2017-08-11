
#include "DPIUtil.h"
#include <Windows.h>

/*
 * DPI関連のユーティリティクラス
*/

DPIUtil::DPIUtil() {
	SetProcessDPIAware();
	
	HDC hDC = GetDC(nullptr);
	DpiX = GetDeviceCaps(hDC, LOGPIXELSX);
	DpiY = GetDeviceCaps(hDC, LOGPIXELSY);
	ReleaseDC(nullptr, hDC);
	
	ScalingX = DpiX / 96.0;
	ScalingY = DpiY / 96.0;
	
}

// GlobalDpiUtil
std::unique_ptr<DPIUtil> pGlbDpiUtil = nullptr;

// GlobalDpiUtilの初期化
void SetupGlobalDpiUtil() {
	pGlbDpiUtil = std::make_unique<DPIUtil>();
}


