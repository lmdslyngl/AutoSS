#pragma once

#include "CaptureBase.h"
#include <d3d11.h>
#include <dxgi1_2.h>
#include <vector>

/*
 * DesktopDuplicationAPIを使ったキャプチャ
*/
class DesktopDuplCapture : public CaptureBase {
public:
	
	DesktopDuplCapture();
	virtual ~DesktopDuplCapture();
	
	// 初期化
	void Setup();
	
	// 領域のキャプチャを行う
	virtual void CaptureRegion(const RECT *region);
	
	// 画像データを取得
	virtual const unsigned char *GetData() const;
	
	// 画像データの長さを取得
	virtual unsigned int GetDataLength() const;
	
	// 画像のサイズを取得
	virtual void GetImageSize(int *pOutWidth, int *pOutHeight);
	
protected:
	ID3D11Device *pDevice;
	ID3D11DeviceContext *pDeviceContext;
	IDXGIOutputDuplication *pDupl;
	ID3D11Texture2D *pTexStaging;
	DXGI_OUTDUPL_DESC duplDesc;
	std::vector<unsigned char> vecRegionImg;
	int RegionWidth, RegionHeight;
};

