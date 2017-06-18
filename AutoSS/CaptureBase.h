#pragma once

#include <Windows.h>

/*
 * キャプチャ基底クラス
*/
class CaptureBase {
public:
	
	// 領域のキャプチャを行う
	virtual void CaptureRegion(
		const RECT *region,
		unsigned char *pOutBuffer, unsigned int bufferLength,
		int *pOutCapturedWidth, int *pOutCapturedHeight
	) = 0;
	
	// キャプチャに必要なバッファの大きさを計算
	virtual unsigned int CalcNecessaryBufferLength(const RECT *region) const;
	
};


