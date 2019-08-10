
#include "UIRegionSelectWindow.h"
#include "BitBltCapture.h"

/*
 * 範囲選択ウィンドウ
*/
RegionSelectWindow::RegionSelectWindow()
	: wxFrame(nullptr, wxID_ANY, L"",
		wxDefaultPosition, wxDefaultSize, wxBORDER_NONE)
{

	pBitmapBuffer = nullptr;
	BitmapBufferLen = 0;
	RegionStartX = 0;
	RegionStartY = 0;
	RegionEndX = 0;
	RegionEndY = 0;
	IsMousePressingFlag = false;
	IsCanceledFlag = false;

	// デスクトップのサイズ
	DesktopWidth = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
	DesktopHeight = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);

	// バックバッファ
	pBackbufferBitmap = std::make_unique<wxBitmap>(
		DesktopWidth, DesktopHeight);

	// イベントハンドラ
	Bind(wxEVT_PAINT, &RegionSelectWindow::OnPaint, this);
	Bind(wxEVT_LEFT_DOWN, &RegionSelectWindow::OnMousePressed, this);
	Bind(wxEVT_LEFT_UP, &RegionSelectWindow::OnMouseReleased, this);
	Bind(wxEVT_MOTION, &RegionSelectWindow::OnMouseMoved, this);
	Bind(wxEVT_RIGHT_DOWN, &RegionSelectWindow::OnMouseCanceled, this);
	Bind(wxEVT_KILL_FOCUS, &RegionSelectWindow::OnFocusLoad, this);

	SetSize(wxSize(DesktopWidth, DesktopHeight));

}

RegionSelectWindow::~RegionSelectWindow() {
	if( pBitmapBuffer != nullptr ) {
		free(pBitmapBuffer);
		pBitmapBuffer = nullptr;
		BitmapBufferLen = 0;
	}
}

void RegionSelectWindow::UpdateScreenShot() {
	// デスクトップの画像のビットマップを作成
	wxImage image(DesktopWidth, DesktopHeight);
	CaptureDesktop(&image);
	pDesktopBitmap = std::make_unique<wxBitmap>(image);
}

void RegionSelectWindow::OnPaint(wxPaintEvent &ev) {
	DrawRegion(wxPaintDC(this));
}

void RegionSelectWindow::OnMousePressed(wxMouseEvent &ev) {
	IsMousePressingFlag = true;
	RegionStartX = ev.GetX();
	RegionStartY = ev.GetY();
	RegionEndX = RegionStartX;
	RegionEndY = RegionStartY;
	DrawRegion(wxClientDC(this));
}

void RegionSelectWindow::OnMouseReleased(wxMouseEvent &ev) {
	IsMousePressingFlag = false;
	RegionEndX = ev.GetX();
	RegionEndY = ev.GetY();
	DrawRegion(wxClientDC(this));
	Decide();
}

void RegionSelectWindow::OnMouseMoved(wxMouseEvent &ev) {
	if( IsMousePressingFlag ) {
		RegionEndX = ev.GetX();
		RegionEndY = ev.GetY();
		DrawRegion(wxClientDC(this));
	}
}

void RegionSelectWindow::OnMouseCanceled(wxMouseEvent &ev) {
	IsMousePressingFlag = false;
	Cancel();
}

void RegionSelectWindow::OnFocusLoad(wxFocusEvent &ev) {
	IsMousePressingFlag = false;
	Cancel();
}

// 決定
void RegionSelectWindow::Decide() {
	IsCanceledFlag = false;
	if( RegionFinishedCallbackFunc ) RegionFinishedCallbackFunc();
	Hide();
}

// キャンセル
void RegionSelectWindow::Cancel() {
	IsCanceledFlag = true;
	if( RegionFinishedCallbackFunc ) RegionFinishedCallbackFunc();
	Hide();
}

// 選択範囲を描画
void RegionSelectWindow::DrawRegion(wxDC &dc) {
	wxMemoryDC backDC(*pBackbufferBitmap);
	backDC.DrawBitmap(*pDesktopBitmap, 0, 0);
	backDC.SetPen(wxPen(wxColour(255, 128, 0), 4));
	backDC.SetBrush(wxBrush(wxColour(0, 0, 0), wxBRUSHSTYLE_TRANSPARENT));
	backDC.DrawRectangle(RegionStartX, RegionStartY,
		RegionEndX - RegionStartX,
		RegionEndY - RegionStartY);
	if( !IsMousePressingFlag ) DrawInstruction(backDC);
	dc.Blit(
		wxPoint(0, 0), wxSize(DesktopWidth, DesktopHeight),
		&backDC, wxPoint(0, 0));
}

// 選択範囲の説明を描画
void RegionSelectWindow::DrawInstruction(wxDC &dc) {
	wxString instMsg = L"マウスでドラッグして範囲選択します";

	dc.SetFont(wxFont(50, wxFONTFAMILY_DEFAULT,
		wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
	dc.SetTextForeground(wxColour(255, 255, 255));
	wxSize textExtent = dc.GetTextExtent(instMsg);

	dc.SetPen(wxPen(wxColour(10, 10, 10)));
	dc.SetBrush(wxBrush(wxColour(10, 10, 10)));
	dc.DrawRectangle(wxRect(
		(DesktopWidth - (textExtent.GetWidth() + 50)) / 2,
		(DesktopHeight - (textExtent.GetHeight() + 50)) / 2,
		textExtent.GetWidth() + 50, textExtent.GetHeight() + 50));

	wxRect rc(0, 0, DesktopWidth, DesktopHeight);
	dc.DrawLabel(instMsg, rc, wxALIGN_CENTER);

}

void RegionSelectWindow::CaptureDesktop(wxImage *pOutImage) {
	HINSTANCE hInstance = GetModuleHandle(nullptr);
	auto pCap = std::make_unique<BitBltCapture>();
	pCap->Setup(hInstance, nullptr);

	RECT desktopRegion = {
		0, 0,
		wxSystemSettings::GetMetric(wxSYS_SCREEN_X),
		wxSystemSettings::GetMetric(wxSYS_SCREEN_Y)
	};

	BitmapBufferLen = pCap->CalcNecessaryBufferLength(&desktopRegion);
	if( pBitmapBuffer == nullptr ) {
		pBitmapBuffer = (unsigned char *)malloc(BitmapBufferLen);
	}

	int capturedWidth, capturedHeight;
	pCap->CaptureRegion(&desktopRegion,
		pBitmapBuffer, BitmapBufferLen,
		&capturedWidth, &capturedHeight);

	// BGR -> RGB
	unsigned char *cursor = pBitmapBuffer;
	unsigned char *end = pBitmapBuffer + BitmapBufferLen;
	while( cursor != end ) {
		std::swap(cursor[0], cursor[2]);
		cursor += 3;
	}

	pOutImage->SetData(pBitmapBuffer, true);

}

