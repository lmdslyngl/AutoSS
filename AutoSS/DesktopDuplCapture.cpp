
#include "DesktopDuplCapture.h"
#include <assert.h>

DesktopDuplCapture::DesktopDuplCapture() {
	pDevice = nullptr;
	pDeviceContext = nullptr;
	pDupl = nullptr;
	pTexStaging = nullptr;
}

DesktopDuplCapture::~DesktopDuplCapture() {
	if( pTexStaging ) pTexStaging->Release();
	if( pDupl ) pDupl->Release();
	if( pDeviceContext ) pDeviceContext->Release();
	if( pDevice ) pDevice->Release();
}

// 初期化
void DesktopDuplCapture::Setup() {
	HRESULT hr;
	
	hr = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		nullptr, 0,
		D3D11_SDK_VERSION,
		&pDevice,
		nullptr,
		&pDeviceContext
	);
	if( FAILED(hr) ) {
		throw std::exception("Failed to create d3d11 device");
	}
	
	IDXGIDevice *pDxgiDevice = nullptr;
	hr = pDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDxgiDevice);
	if( FAILED(hr) ) {
		throw std::exception("Failed to QueryInterface IDXGIDevice");
	}
	
	IDXGIAdapter *pDxgiAdapter = nullptr;
	hr = pDxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&pDxgiAdapter);
	if( FAILED(hr) ) {
		throw std::exception("Failed to QueryInterface IDXGIAdapter");
	}
	
	IDXGIOutput *pDxgiOutput = nullptr;
	hr = pDxgiAdapter->EnumOutputs(0, &pDxgiOutput);
	if( FAILED(hr) ) {
		throw std::exception("Failed to IDXGIAdapter->EnumOutputs");
	}
	
	IDXGIOutput1 *pDxgiOutput1 = nullptr;
	hr = pDxgiOutput->QueryInterface(__uuidof(IDXGIOutput1), (void**)&pDxgiOutput1);
	if( FAILED(hr) ) {
		throw std::exception("Failed to QueryInterface IDXGIOutput1");
	}
	
	pDupl = nullptr;
	hr = pDxgiOutput1->DuplicateOutput(pDevice, &pDupl);
	if( FAILED(hr) ) {
		throw std::exception("Failed to IDXGIOutput1->DuplicateOutput");
	}
	pDupl->GetDesc(&duplDesc);
	
	// ステージングテクスチャ作成
	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
	texDesc.Width = duplDesc.ModeDesc.Width;
	texDesc.Height = duplDesc.ModeDesc.Height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = duplDesc.ModeDesc.Format;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_STAGING;
	texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	hr = pDevice->CreateTexture2D(&texDesc, nullptr, &pTexStaging);
	if( FAILED(hr) ) {
		throw std::exception("Failed to create staging texture");
	}
	
	if( pDxgiOutput1 ) pDxgiOutput1->Release();
	if( pDxgiOutput ) pDxgiOutput->Release();
	if( pDxgiAdapter ) pDxgiAdapter->Release();
	if( pDxgiDevice ) pDxgiDevice->Release();
	
}

// 領域のキャプチャを行う
void DesktopDuplCapture::CaptureRegion(
	const RECT *region,
	unsigned char *pOutBuffer, unsigned int bufferLength,
	int *pOutCapturedWidth, int *pOutCapturedHeight)
{
	
	assert(region != nullptr);
	assert(pOutBuffer != nullptr);
	assert(pOutCapturedWidth != nullptr && pOutCapturedHeight != nullptr);
	
	int capturedWidth = region->right - region->left;
	int capturedHeight = region->bottom - region->top;
	unsigned int necessaryBufLen = capturedWidth * capturedHeight * 3;
	assert(necessaryBufLen <= bufferLength);
	
	IDXGIResource *pDeskRes = nullptr;
	DXGI_OUTDUPL_FRAME_INFO frameInfo;
	pDupl->AcquireNextFrame(500, &frameInfo, &pDeskRes);
	
	ID3D11Texture2D *pDeskTex = nullptr;
	pDeskRes->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&pDeskTex);
	
	// ステージングテクスチャにコピー
	pDeviceContext->CopyResource(pTexStaging, pDeskTex);
	
	// ステージングテクスチャからメモリにコピー
	D3D11_MAPPED_SUBRESOURCE mappedRes;
	pDeviceContext->Map(pTexStaging, 0, D3D11_MAP_READ, 0, &mappedRes);
	
	const unsigned char *src = (const unsigned char*)mappedRes.pData;
	unsigned char *dst = pOutBuffer;
	
	// BGRX -> BGR
	for( int y = 0; y < capturedHeight; y++ ) {
		const unsigned char *srcrow =
			&src[((region->top + y) * duplDesc.ModeDesc.Width + region->left) * 4];
		for( int x = 0; x < capturedWidth; x++ ) {
			*dst++ = *srcrow++;
			*dst++ = *srcrow++;
			*dst++ = *srcrow++;
			srcrow++;
		}
	}
	
	pDeviceContext->Unmap(pTexStaging, 0);
	
	pDupl->ReleaseFrame();
	
	*pOutCapturedWidth = capturedWidth;
	*pOutCapturedHeight = capturedHeight;
	
}

