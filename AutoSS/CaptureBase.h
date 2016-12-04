#pragma once

#include <Windows.h>

/*
 * キャプチャ基底クラス
*/
class CaptureBase {
public:
	
	// 領域のキャプチャを行う
	virtual void CaptureRegion(const RECT *region) = 0;
	
	// 画像データを取得
	virtual const unsigned char *GetData() const = 0;
	
	// 画像データの長さを取得
	virtual unsigned int GetDataLength() const = 0;
	
	// 画像のサイズを取得
	virtual void GetImageSize(int *pOutWidth, int *pOutHeight) = 0;
	
};


