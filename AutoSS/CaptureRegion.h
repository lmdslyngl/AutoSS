#pragma once

#include <Windows.h>

/*
 * 撮影範囲基底クラス
*/
class CaptureRegionBase {
public:
	
	CaptureRegionBase();
	virtual ~CaptureRegionBase() = default;
	virtual void GetCaptureRegionRect(RECT *pOutRegion) = 0;
	
protected:
	
	// 画面外の領域をクランプする
	void ClampOutOfRegion(RECT *pInOutRegion) const;
	
protected:
	int DesktopWidth, DesktopHeight;
	
};

/*
 * 撮影範囲: 選択ウィンドウ
*/
class CaptureRegionSelectedWindow : public CaptureRegionBase {
public:
	
	CaptureRegionSelectedWindow(HWND hCaptureWindow, bool includeBorder)
		: CaptureRegionBase()
	{
		this->hCaptureWindow = hCaptureWindow;
		this->IsIncludeBorderFlag = includeBorder;
	}
	
	virtual void GetCaptureRegionRect(RECT *pOutRegion);
	
	HWND GetCaptureWindow() const {
		return hCaptureWindow;
	}
	
	bool IsIncludeBorder() const {
		return IsIncludeBorderFlag;
	}
	
private:
	HWND hCaptureWindow;
	bool IsIncludeBorderFlag;
};


/*
 * 撮影範囲: アクティブウィンドウ
*/
class CaptureRegionActiveWindow : public CaptureRegionBase {
public:
	CaptureRegionActiveWindow(bool includeBorder) 
		: CaptureRegionBase()
	{
		this->IsIncludeBorderFlag = includeBorder;
		PrevRegion.left = 0;
		PrevRegion.top = 0;
		PrevRegion.right = DesktopWidth;
		PrevRegion.bottom = DesktopHeight;
	}
	
	virtual void GetCaptureRegionRect(RECT *pOutRegion);
	
	bool IsIncludeBorder() {
		return IsIncludeBorderFlag;
	}
	
private:
	bool IsIncludeBorderFlag;
	RECT PrevRegion;
};


/*
 * 撮影範囲: 選択範囲
*/
class CaptureRegionSelectedRegion : public CaptureRegionBase {
public:
	CaptureRegionSelectedRegion(int x, int y, int width, int height)
		: CaptureRegionBase()
	{
		this->x = x;
		this->y = y;
		this->width = width;
		this->height = height;
	}
	
	virtual void GetCaptureRegionRect(RECT *pOutRegion);
	
private:
	int x, y, width, height;
};


/*
 * 撮影範囲: フルスクリーン
*/
class CaptureRegionFullscreen : public CaptureRegionBase {
public:
	
	CaptureRegionFullscreen() : CaptureRegionBase() { }
	
	virtual void GetCaptureRegionRect(RECT *pOutRegion);
	
};


