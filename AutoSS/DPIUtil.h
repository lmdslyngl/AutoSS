#pragma once

#include <memory>

/*
 * DPI関連のユーティリティクラス
*/
class DPIUtil {
public:
	
	DPIUtil();
	
	int GetDpiX() const {
		return DpiX;
	}
	int GetDpiY() const {
		return DpiY;
	}
	
	int GetScalingX() const {
		return ScalingX;
	}
	int GetScalingY() const {
		return ScalingY;
	}
	
private:
	int DpiX, DpiY;
	double ScalingX, ScalingY;
};

// GlobalDpiUtil
extern std::unique_ptr<DPIUtil> pGlbDpiUtil;

// GlobalDpiUtilの初期化
void SetupGlobalDpiUtil();

