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
	virtual void CaptureRegion(
		const RECT *region,
		unsigned char *pOutBuffer, unsigned int bufferLength,
		int *pOutCapturedWidth, int *pOutCapturedHeight);
	
private:
	
	// デバイスを開放
	void Release();
	
protected:
	ID3D11Device *pDevice;
	ID3D11DeviceContext *pDeviceContext;
	IDXGIOutputDuplication *pDupl;
	ID3D11Texture2D *pTexStaging;
	DXGI_OUTDUPL_DESC duplDesc;
};

