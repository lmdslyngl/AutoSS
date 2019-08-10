#pragma once

#include <wx/wx.h>
#include <memory>
#include <functional>
#include "Config.h"


/*
 * 範囲選択ウィンドウ
*/
class RegionSelectWindow : public wxFrame {
public:

	RegionSelectWindow();
	virtual ~RegionSelectWindow();

	void SetRegion(int startX, int startY, int endX, int endY) {
		RegionStartX = startX;
		RegionStartY = startY;
		RegionEndX = endX;
		RegionEndY = endY;
	}

	void GetRegion(int *startX, int *startY, int *endX, int *endY) {
		*startX = RegionStartX;
		*startY = RegionStartY;
		*endX = RegionEndX;
		*endY = RegionEndY;
	}

	// 領域が決定したときのコールバック
	void SetRegionFinishedCallback(std::function<void()> func) {
		this->RegionFinishedCallbackFunc = func;
	}

	// デスクトップの画像を更新
	void UpdateScreenShot();

	// キャンセルされたかどうか
	bool IsCanceled() const {
		return IsCanceledFlag;
	}

private:

	void OnPaint(wxPaintEvent &ev);

	void OnMousePressed(wxMouseEvent &ev);
	void OnMouseReleased(wxMouseEvent &ev);
	void OnMouseMoved(wxMouseEvent &ev);
	void OnMouseCanceled(wxMouseEvent &ev);
	void OnFocusLoad(wxFocusEvent &ev);

	// 決定
	void Decide();

	// キャンセル
	void Cancel();

	// 選択範囲の説明を描画
	void DrawInstruction(wxDC &dc);

	// 選択範囲を描画
	void DrawRegion(wxDC &dc);

	// デスクトップの画像を取得
	void CaptureDesktop(wxImage *pOutImage);

private:
	std::unique_ptr<wxBitmap> pDesktopBitmap;
	unsigned char *pBitmapBuffer;
	size_t BitmapBufferLen;
	std::unique_ptr<wxBitmap> pBackbufferBitmap;

	int RegionStartX, RegionStartY;
	int RegionEndX, RegionEndY;
	bool IsMousePressingFlag;

	int DesktopWidth, DesktopHeight;

	std::function<void()> RegionFinishedCallbackFunc;
	bool IsCanceledFlag;

};

