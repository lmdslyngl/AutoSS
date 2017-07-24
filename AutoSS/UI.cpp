﻿
#include "UI.h"
#include <wx/valnum.h>
#include <wx/appprogress.h>
#include "BitBltCapture.h"

/*
 * AutoSSウィンドウ
*/
AutoSSFrame::AutoSSFrame(const std::shared_ptr<Config> &pConf)
	: wxFrame(nullptr, wxID_ANY, L"AutoSS")
{
	
	this->pConf = pConf;
	pConfigFrame = nullptr;
	TakingSS = false;
	IsEnableCapture = true;
	
	wxSizer *pSizer = new wxBoxSizer(wxHORIZONTAL);
	wxPanel *pPanel = new wxPanel(this, wxID_ANY);
	pPanel->SetSizer(pSizer);
	
	pStartBtn = new wxButton(pPanel, wxID_ANY, L"Start");
	pStartBtn->Bind(wxEVT_BUTTON, &AutoSSFrame::OnStart, this);
	pSizer->Add(pStartBtn, wxSizerFlags(2).Expand());
	pSizer->AddSpacer(5);
	
	pBurstChk = new wxCheckBox(pPanel, wxID_ANY, L"BurstMode");
	pBurstChk->Bind(wxEVT_CHECKBOX, &AutoSSFrame::OnBurstCheck, this);
	pSizer->Add(pBurstChk, wxSizerFlags(1).Expand());
	pSizer->AddSpacer(5);
	
	wxButton *pConfBtn = new wxButton(pPanel, wxID_ANY, L"Config");
	pConfBtn->Bind(wxEVT_BUTTON, &AutoSSFrame::OnConf, this);
	pSizer->Add(pConfBtn, wxSizerFlags(1).Expand());
	
	SetSize(350, 100);
	
	CreateStatusBar(1);
	SetStatusText(L"Stopped");
	
	// ホットキー
	Bind(wxEVT_HOTKEY, &AutoSSFrame::OnHotkey, this);
	RegisterHotKey(HOTKEY_ID_START, pConf->HotkeyMod, pConf->HotkeyCodeRaw);
	
	// タスクバーインジケータ
	pProgressIndicator = new wxAppProgressIndicator(this);
	if( pProgressIndicator->IsAvailable() ) {
		pProgressIndicator->SetRange(1);
	} else {
		// 使えなかったときはnullptrにしておく
		delete pProgressIndicator;
		pProgressIndicator = nullptr;
	}
	
}

void AutoSSFrame::OnStart(wxCommandEvent &ev) {
	OnStartImpl();
}

void AutoSSFrame::OnConf(wxCommandEvent &ev) {
	pConfigFrame = new ConfigFrame(this, pConf);
	
	DisableCapture();
	UnregisterHotKey(HOTKEY_ID_START);	// 設定画面でホットキーが動作しないように
	pConfigFrame->ShowModal();
	EnableCapture();
	
	if( OnChangeConfFunc && pConfigFrame->GetCloseState() ) {
		auto pNewConfig = pConfigFrame->GetConfig();
		OnChangeConfFunc(pNewConfig);
		
		// ホットキー変更
		RegisterHotKey(HOTKEY_ID_START,
			pNewConfig->HotkeyMod, pNewConfig->HotkeyCodeRaw);
		
	} else {
		// 今までのホットキーを登録
		RegisterHotKey(HOTKEY_ID_START, pConf->HotkeyMod, pConf->HotkeyCodeRaw);
	}
	
	pConfigFrame->Destroy();
	pConfigFrame = nullptr;
	
}

void AutoSSFrame::OnHotkey(wxKeyEvent &ev) {
	if( IsEnableCapture ) {
		// 設定ダイアログが出ているときはホットキーを無視する
		OnStartImpl();
	}
}

void AutoSSFrame::OnStartImpl() {
	if( TakingSS ) {
		if( OnStopFunc ) OnStopFunc();
	} else {
		if( OnStartFunc ) OnStartFunc();
	}
}

void AutoSSFrame::Start() {
	TakingSS = true;
	pStartBtn->SetLabel(L"Stop");
	SetStatusText(L"Taking");
	if( pProgressIndicator ) pProgressIndicator->SetValue(1);
	pBurstChk->Disable();
}

void AutoSSFrame::Stop() {
	TakingSS = false;
	if( pBurstChk->GetValue() ) {
		pStartBtn->SetLabel(L"Burst Start");
	} else {
		pStartBtn->SetLabel(L"Start");
	}
	if( pProgressIndicator ) pProgressIndicator->SetValue(0);
	pBurstChk->Enable();
}


void AutoSSFrame::OnBurstCheck(wxCommandEvent &ev) {
	if( pBurstChk->GetValue() ) {
		pStartBtn->SetLabelText(L"Burst Start");
	} else {
		pStartBtn->SetLabelText(L"Start");
	}
	if( OnModeChangeFunc ) OnModeChangeFunc(pBurstChk->GetValue());
}

/*
 * 設定ダイアログ
*/
ConfigFrame::ConfigFrame(wxFrame *pParent, const std::shared_ptr<Config> &pInitConf)
	: wxDialog(pParent, wxID_ANY, L"AutoSS設定")
{
	
	RegisteringHotkey = false;
	
	// ホットキー登録用キーイベントフック
	Bind(wxEVT_CHAR_HOOK, &ConfigFrame::OnKeyDown, this);
	
	// ダイアログを閉じだときのイベント
	Bind(wxEVT_CLOSE_WINDOW, &ConfigFrame::OnClose, this);
	
	// 領域選択ウィンドウ
	pRgnSelWnd = new RegionSelectWindow();
	pRgnSelWnd->SetRegionFinishedCallback(
		[this]() { this->OnRegionSelectFinished(); });
	
	wxFlexGridSizer *pDialogSizer = new wxFlexGridSizer(2, wxSize(5, 5));
	pDialogSizer->AddSpacer(0);
	pDialogSizer->AddStretchSpacer();
	SetSizer(pDialogSizer);
	
	//
	// SS保存先
	//
	
	wxStaticText *pSavePathLabel = new wxStaticText(this, wxID_ANY, L"SS保存先");
	
	pSavePathText = new wxTextCtrl(this, wxID_ANY, pInitConf->SavePath);
	
	pSavePathRefButton = new wxButton(
		this, wxID_ANY, L"参照",
		wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	pSavePathRefButton->Bind(wxEVT_BUTTON, &ConfigFrame::OnSavePathRef, this);
	
	pDialogSizer->Add(pSavePathLabel, wxSizerFlags().CenterVertical().Right());
	
	wxBoxSizer *pSavePathSizer = new wxBoxSizer(wxHORIZONTAL);
	pSavePathSizer->Add(pSavePathText, wxSizerFlags(1).CenterVertical());
	pSavePathSizer->AddSpacer(5);
	pSavePathSizer->Add(pSavePathRefButton, wxSizerFlags().CenterVertical());
	pDialogSizer->Add(pSavePathSizer, wxSizerFlags().Expand());
	
	
	//
	// 撮影間隔
	//
	
	wxStaticText *pWaitTimeLabel = new wxStaticText(this, wxID_ANY, L"撮影間隔（ミリ秒）");
	
	wxIntegerValidator<unsigned int> intValid(nullptr);
	pWaitTimeText = new wxTextCtrl(this, wxID_ANY,
		std::to_string(pInitConf->WaitTime),
		wxDefaultPosition, wxSize(50, -1), 0, intValid);
	
	pDialogSizer->Add(pWaitTimeLabel, wxSizerFlags().CenterVertical().Right());
	pDialogSizer->Add(pWaitTimeText, wxSizerFlags().CenterVertical());
	
	
	//
	// キャプチャ方式
	//
	
	wxStaticText *pCaptureLabel = new wxStaticText(this, wxID_ANY, L"キャプチャ方式");
	
	pCaptureCombo = new wxComboBox(
		this, wxID_ANY, wxEmptyString,
		wxDefaultPosition, wxDefaultSize,
		0, nullptr, wxCB_READONLY);
	pCaptureCombo->Append(L"BitBlt");
	pCaptureCombo->Append(L"Desktop Duplication API");
	pCaptureCombo->Select(pInitConf->CaptureMethod);
	
	pDialogSizer->Add(pCaptureLabel, wxSizerFlags().CenterVertical().Right());
	pDialogSizer->Add(pCaptureCombo, wxSizerFlags().CenterVertical());
	
	
	//
	// 撮影領域
	//
	
	wxStaticText *pRegionLabel = new wxStaticText(this, wxID_ANY, L"撮影範囲");
	
	pRegionCombo = new wxComboBox(
		this, wxID_ANY, wxEmptyString,
		wxDefaultPosition, wxDefaultSize,
		0, nullptr, wxCB_READONLY);
	pRegionCombo->Bind(wxEVT_COMBOBOX, &ConfigFrame::OnRegionComboChanged, this);
	pRegionCombo->Append(L"アクティブウィンドウ");
	pRegionCombo->Append(L"選択範囲");
	pRegionCombo->Append(L"フルスクリーン");
	pRegionCombo->Select(pInitConf->RegionMode);
	
	pDialogSizer->Add(pRegionLabel, wxSizerFlags().CenterVertical().Right());
	pDialogSizer->Add(pRegionCombo, wxSizerFlags().CenterVertical());
	
	//
	// ウィンドウ枠パネル
	//
	
	pIncludeBorderCheck = new wxCheckBox(this, wxID_ANY, L"ウィンドウの枠を含める");
	pIncludeBorderCheck->SetValue(pInitConf->IncludeBorder);
	
	pDialogSizer->AddStretchSpacer();
	pDialogSizer->Add(pIncludeBorderCheck);
	
	
	//
	// 範囲選択パネル
	//
	pRegionSelectPanel = CreateRegionSelectPanel(pInitConf);
	pDialogSizer->AddStretchSpacer();
	pDialogSizer->Add(pRegionSelectPanel);
	
	
	//
	// 最大撮影枚数
	//
	wxStaticText *pMaxCapCountLabel = new wxStaticText(this, wxID_ANY, L"最大撮影枚数");
	pMaxCapCountText = new wxTextCtrl(this, wxID_ANY, 
		std::to_string(pInitConf->MaxCaptureCount),
		wxDefaultPosition, wxSize(50, -1), 0, intValid);
	
	pDialogSizer->Add(pMaxCapCountLabel, wxSizerFlags().CenterVertical().Right());
	pDialogSizer->Add(pMaxCapCountText, wxSizerFlags().CenterVertical());
	
	
	//
	// ホットキー
	//
	
	HotkeyMod = pInitConf->HotkeyMod;
	HotkeyCode = pInitConf->HotkeyCode;
	HotkeyCodeRaw = pInitConf->HotkeyCodeRaw;
	
	wxStaticText *pHotkeyLabel = new wxStaticText(this, wxID_ANY, L"ショートカット");
	
	pHotkeyText = new wxTextCtrl(this, wxID_ANY, L"",
		wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	pHotkeyText->SetValue(wxAcceleratorEntry(HotkeyMod, HotkeyCode).ToString());
	
	pHotkeyRegButton = new wxButton(this, wxID_ANY, L"登録開始");
	pHotkeyRegButton->Bind(wxEVT_BUTTON, &ConfigFrame::OnRegisterHotkey, this);
	
	pDialogSizer->Add(pHotkeyLabel, wxSizerFlags().CenterVertical().Right());
	
	wxBoxSizer *pHotkeySizer = new wxBoxSizer(wxHORIZONTAL);
	pHotkeySizer->Add(pHotkeyText, wxSizerFlags().CenterVertical());
	pHotkeySizer->Add(pHotkeyRegButton, wxSizerFlags().CenterVertical());
	pDialogSizer->Add(pHotkeySizer);
	
	//
	// OK，キャンセル
	//
	
	wxBoxSizer *pOKCancelSizer = new wxBoxSizer(wxHORIZONTAL);
	
	pOKButton = new wxButton(this, wxID_ANY, L"OK");
	pOKButton->Bind(wxEVT_BUTTON, &ConfigFrame::OnOK, this);
	pOKCancelSizer->Add(pOKButton);
	
	pCancelButton = new wxButton(this, wxID_ANY, L"キャンセル");
	pCancelButton->Bind(wxEVT_BUTTON, &ConfigFrame::OnCancel, this);
	pOKCancelSizer->Add(pCancelButton);
	
	pDialogSizer->AddStretchSpacer();
	pDialogSizer->Add(pOKCancelSizer, wxSizerFlags().Right());
	
	pDialogSizer->AddSpacer(5);
	pDialogSizer->AddStretchSpacer();
	
	Fit();
	UpdateRegionSelectionEnabling();
	
}

wxPanel *ConfigFrame::CreateRegionSelectPanel(
	const std::shared_ptr<Config> &pInitConf)
{
	
	wxPanel *pRegionSelectPanel = new wxPanel(this, wxID_ANY);
	wxIntegerValidator<int> regionValidator(nullptr);
	regionValidator.SetMin(0);
	
	// X
	wxStaticText *pRegionXLabel = new wxStaticText(
		pRegionSelectPanel, wxID_ANY, L"X");
	pRegionXText = new wxTextCtrl(
		pRegionSelectPanel, wxID_ANY,
		std::to_string(pInitConf->RegionX),
		wxDefaultPosition, wxSize(50, -1), 0, regionValidator);
	
	// Y
	wxStaticText *pRegionYLabel = new wxStaticText(
		pRegionSelectPanel, wxID_ANY, L"Y");
	pRegionYText = new wxTextCtrl(
		pRegionSelectPanel, wxID_ANY,
		std::to_string(pInitConf->RegionY),
		wxDefaultPosition, wxSize(50, -1), 0, regionValidator);
	
	// Width
	wxStaticText *pRegionWLabel = new wxStaticText(
		pRegionSelectPanel, wxID_ANY, L"Width");
	pRegionWidthText = new wxTextCtrl(
		pRegionSelectPanel, wxID_ANY,
		std::to_string(pInitConf->RegionWidth),
		wxDefaultPosition, wxSize(50, -1), 0, regionValidator);
	
	// Height
	wxStaticText *pRegionHLabel = new wxStaticText(
		pRegionSelectPanel, wxID_ANY, L"Height");
	pRegionHeightText = new wxTextCtrl(
		pRegionSelectPanel, wxID_ANY,
		std::to_string(pInitConf->RegionHeight),
		wxDefaultPosition, wxSize(50, -1), 0, regionValidator);
	
	// 範囲選択ボタン
	pRegionSelectBtn = new wxButton(
		pRegionSelectPanel, wxID_ANY, L"マウスで選択");
	pRegionSelectBtn->Bind(wxEVT_BUTTON, &ConfigFrame::OnRegionSelect, this);
	
	//
	// 配置
	//
	wxStaticBoxSizer *pPanelHSizer = new wxStaticBoxSizer(
		wxHORIZONTAL, pRegionSelectPanel, L"選択範囲");
	{
		wxBoxSizer *pVSizer = new wxBoxSizer(wxVERTICAL);
		pPanelHSizer->Add(pVSizer);
		{
			wxFlexGridSizer *pRegionSelectSizer = new wxFlexGridSizer(4, 5, 5);
			pVSizer->Add(pRegionSelectSizer);
			{
				pRegionSelectSizer->Add(pRegionXLabel,
					wxSizerFlags().CenterVertical().Right());
				pRegionSelectSizer->Add(pRegionXText);
				
				pRegionSelectSizer->Add(pRegionYLabel,
					wxSizerFlags().CenterVertical().Right());
				pRegionSelectSizer->Add(pRegionYText);
				
				pRegionSelectSizer->Add(pRegionWLabel,
					wxSizerFlags().CenterVertical().Right());
				pRegionSelectSizer->Add(pRegionWidthText);
				
				pRegionSelectSizer->Add(pRegionHLabel,
					wxSizerFlags().CenterVertical().Right());
				pRegionSelectSizer->Add(pRegionHeightText);
			}
			pVSizer->AddSpacer(5);
			pVSizer->Add(pRegionSelectBtn, wxSizerFlags().Right());
		}
	}
	
	pRegionSelectPanel->SetSizer(pPanelHSizer);
	
	return pRegionSelectPanel;
	
}

std::shared_ptr<Config> ConfigFrame::GetConfig() const {
	auto pConf = std::make_shared<Config>();
	pConf->SavePath = pSavePathText->GetValue();
	pConf->WaitTime = std::stoi(pWaitTimeText->GetValue().ToStdString());
	pConf->CaptureMethod = (CAPTURE_METHOD)pCaptureCombo->GetSelection();
	pConf->IncludeBorder = pIncludeBorderCheck->GetValue();
	pConf->HotkeyCode = HotkeyCode;
	pConf->HotkeyCodeRaw = HotkeyCodeRaw;
	pConf->HotkeyMod = HotkeyMod;
	pConf->ImageFormat = IMGFMT_BMP;
	pConf->RegionMode = (CAPTURE_REGION)pRegionCombo->GetSelection();
	pConf->RegionX = std::stoi(pRegionXText->GetValue().ToStdString());
	pConf->RegionY = std::stoi(pRegionYText->GetValue().ToStdString());
	pConf->RegionWidth = std::stoi(pRegionWidthText->GetValue().ToStdString());
	pConf->RegionHeight = std::stoi(pRegionHeightText->GetValue().ToStdString());
	pConf->MaxCaptureCount = std::stoi(pMaxCapCountText->GetValue().ToStdString());
	return pConf;
}


void ConfigFrame::OnSavePathRef(wxCommandEvent &ev) {
	wxDirDialog dlg(this,
		L"スクリーンショット保存先",
		L"", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
	dlg.ShowModal();
	
	wxString path = dlg.GetPath();
	if( !path.empty() ) {
		pSavePathText->SetValue(path);
	}
	
}

void ConfigFrame::OnOK(wxCommandEvent &ev) {
	Close();
	pRgnSelWnd->Close();
	CloseState = true;
}

void ConfigFrame::OnCancel(wxCommandEvent &ev) {
	Close();
	pRgnSelWnd->Close();
	CloseState = false;
}

void ConfigFrame::OnRegisterHotkey(wxCommandEvent &ev) {
	RegisteringHotkey = !RegisteringHotkey;
	if( RegisteringHotkey ) {
		pHotkeyRegButton->SetLabel(L"登録完了");
		pSavePathText->Disable();
		pSavePathRefButton->Disable();
		pWaitTimeText->Disable();
		pCaptureCombo->Disable();
		pRegionCombo->Disable();
		pIncludeBorderCheck->Disable();
		pRegionSelectPanel->Disable();
		pOKButton->Disable();
		pCancelButton->Disable();
	} else {
		pHotkeyRegButton->SetLabel(L"登録開始");
		pSavePathText->Enable();
		pSavePathRefButton->Enable();
		pWaitTimeText->Enable();
		pCaptureCombo->Enable();
		pRegionCombo->Enable();
		pIncludeBorderCheck->Enable();
		pRegionSelectPanel->Enable();
		pOKButton->Enable();
		pCancelButton->Enable();
	}
}

void ConfigFrame::OnKeyDown(wxKeyEvent &ev) {
	if( RegisteringHotkey ) {
		// ホットキー登録中
		int keycode = ev.GetKeyCode();
		if( keycode == WXK_COMMAND || keycode == WXK_CONTROL
			|| keycode == WXK_ALT || keycode == WXK_SHIFT )
		{
			// 装飾キーのみの場合は無視
		} else {
			HotkeyCode = ev.GetKeyCode();
			HotkeyCodeRaw = ev.GetRawKeyCode();
			HotkeyMod = ev.GetModifiers();
			pHotkeyText->SetValue(wxAcceleratorEntry(HotkeyMod, HotkeyCode).ToString());
		}
		
	} else {
		// ホットキー登録中じゃない
		ev.DoAllowNextEvent();
	}
	
}

void ConfigFrame::OnRegionComboChanged(wxCommandEvent &ev) {
	UpdateRegionSelectionEnabling();
}

// 撮影範囲コンボボックスによって選択範囲の有効/無効を更新する
void ConfigFrame::UpdateRegionSelectionEnabling() {
	CAPTURE_REGION region = (CAPTURE_REGION)pRegionCombo->GetSelection();
	
	if( region == CAPTURE_REGION_ACTIVE_WINDOW ) {
		pIncludeBorderCheck->Enable();
	} else {
		pIncludeBorderCheck->Disable();
	}
	
	if( region == CAPTURE_REGION_SELECTED_REGION ) {
		pRegionSelectPanel->Enable();
	} else {
		pRegionSelectPanel->Disable();
	}
	
}

void ConfigFrame::OnRegionSelect(wxCommandEvent &ev) {
	int startX, startY, endX, endY, width, height;
	startX = std::stoi(pRegionXText->GetValue().ToStdString());
	startY = std::stoi(pRegionYText->GetValue().ToStdString());
	width = std::stoi(pRegionWidthText->GetValue().ToStdString());
	height = std::stoi(pRegionHeightText->GetValue().ToStdString());
	endX = startX + width;
	endY = startY + height;
	pRgnSelWnd->SetRegion(startX, startY, endX, endY);
	
	pRgnSelWnd->UpdateScreenShot();
	pRgnSelWnd->Show();
	
}

void ConfigFrame::OnRegionSelectFinished() {
	if( !pRgnSelWnd->IsCanceled() ) {
		int startX, startY, endX, endY;
		pRgnSelWnd->GetRegion(&startX, &startY, &endX, &endY);
		
		if( endX < startX ) std::swap(startX, endX);
		if( endY < startY ) std::swap(startY, endY);
		
		pRegionXText->SetValue(std::to_string(startX));
		pRegionYText->SetValue(std::to_string(startY));
		pRegionWidthText->SetValue(std::to_string(endX - startX));
		pRegionHeightText->SetValue(std::to_string(endY - startY));
		
	}
}

void ConfigFrame::OnClose(wxCloseEvent &ev) {
	CloseState = false;
	pRgnSelWnd->Destroy();
	Hide();
}


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
		pBitmapBuffer = (unsigned char*)malloc(BitmapBufferLen);
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

