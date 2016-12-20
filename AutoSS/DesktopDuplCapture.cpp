
#include "DesktopDuplCapture.h"

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
	pDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDxgiDevice);
	
	IDXGIAdapter *pDxgiAdapter = nullptr;
	pDxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&pDxgiAdapter);
	
	IDXGIOutput *pDxgiOutput = nullptr;
	pDxgiAdapter->EnumOutputs(0, &pDxgiOutput);
	
	IDXGIOutput1 *pDxgiOutput1 = nullptr;
	pDxgiOutput->QueryInterface(__uuidof(IDXGIOutput1), (void**)&pDxgiOutput1);
	
	pDupl = nullptr;
	pDxgiOutput1->DuplicateOutput(pDevice, &pDupl);
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
void DesktopDuplCapture::CaptureRegion(const RECT *region) {
	RegionWidth = region->right - region->left;
	RegionHeight = region->bottom - region->top;
	vecRegionImg.resize(RegionWidth * RegionHeight * 3);
	
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
	unsigned char *dst = vecRegionImg.data();
	
	// BGRX -> RGB
	for( int y = 0; y < RegionHeight; y++ ) {
		const unsigned char *srcrow =
			&src[((region->top + y) * duplDesc.ModeDesc.Width + region->left) * 4];
		for( int x = 0; x < RegionWidth; x++ ) {
			dst[0] = srcrow[2];
			dst[1] = srcrow[1];
			dst[2] = srcrow[0];
			dst += 3;
			srcrow += 4;
		}
	}
	
	pDeviceContext->Unmap(pTexStaging, 0);
	
	pDupl->ReleaseFrame();
	
}

// 画像データを取得
const unsigned char *DesktopDuplCapture::GetData() const {
	return vecRegionImg.data();
}

// 画像データの長さを取得
size_t DesktopDuplCapture::GetDataLength() const {
	return vecRegionImg.size();
}

// 画像のサイズを取得
void DesktopDuplCapture::GetImageSize(int *pOutWidth, int *pOutHeight) {
	if( pOutWidth ) *pOutWidth = RegionWidth;
	if( pOutHeight ) *pOutHeight = RegionHeight;
}

