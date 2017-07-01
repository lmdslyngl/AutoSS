﻿
#include "UI.h"
#include <wx/valnum.h>
#include <wx/appprogress.h>

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
	if( pConfigFrame ) pConfigFrame->Destroy();
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
	
	wxBoxSizer *pDialogSizer = new wxBoxSizer(wxVERTICAL);
	pDialogSizer->AddSpacer(10);
	SetSizer(pDialogSizer);
	
	//
	// SS保存先
	//
	
	wxBoxSizer *pSavePathSizer = new wxBoxSizer(wxHORIZONTAL);
	pSavePathSizer->AddSpacer(10);
	
	wxStaticText *pSavePathLabel = new wxStaticText(this, wxID_ANY, L"SS保存先");
	pSavePathSizer->Add(pSavePathLabel, wxSizerFlags().CenterVertical());
	pSavePathSizer->AddSpacer(5);
	
	pSavePathText = new wxTextCtrl(this, wxID_ANY, pInitConf->SavePath);
	pSavePathSizer->Add(pSavePathText, wxSizerFlags(1).CenterVertical());
	pSavePathSizer->AddSpacer(5);
	
	pSavePathRefButton = new wxButton(
		this, wxID_ANY, L"参照",
		wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	pSavePathRefButton->Bind(wxEVT_BUTTON, &ConfigFrame::OnSavePathRef, this);
	pSavePathSizer->Add(pSavePathRefButton, wxSizerFlags().CenterVertical());
	
	pSavePathSizer->AddSpacer(10);
	
	pDialogSizer->Add(pSavePathSizer, wxSizerFlags().Expand());
	pDialogSizer->AddSpacer(10);
	
	
	//
	// 撮影間隔
	//
	
	wxBoxSizer *pWaitTimeSizer = new wxBoxSizer(wxHORIZONTAL);
	pWaitTimeSizer->AddSpacer(10);
	
	wxStaticText *pWaitTimeLabel = new wxStaticText(this, wxID_ANY, L"撮影間隔（ミリ秒）");
	pWaitTimeSizer->Add(pWaitTimeLabel, wxSizerFlags().CenterVertical());
	pWaitTimeSizer->AddSpacer(5);
	
	wxIntegerValidator<unsigned int> intValid(nullptr);
	pWaitTimeText = new wxTextCtrl(this, wxID_ANY,
		std::to_string(pInitConf->WaitTime),
		wxDefaultPosition, wxSize(50, -1), 0, intValid);
	pWaitTimeSizer->Add(pWaitTimeText, wxSizerFlags().CenterVertical());
	
	pWaitTimeSizer->AddSpacer(10);
	
	pDialogSizer->Add(pWaitTimeSizer);
	pDialogSizer->AddSpacer(10);
	
	
	//
	// キャプチャ方式
	//
	
	wxBoxSizer *pCaptureSizer = new wxBoxSizer(wxHORIZONTAL);
	pCaptureSizer->AddSpacer(10);
	
	wxStaticText *pCaptureLabel = new wxStaticText(this, wxID_ANY, L"キャプチャ方式");
	pCaptureSizer->Add(pCaptureLabel, wxSizerFlags().CenterVertical());
	pCaptureSizer->AddSpacer(5);
	
	pCaptureCombo = new wxComboBox(
		this, wxID_ANY, wxEmptyString,
		wxDefaultPosition, wxDefaultSize,
		0, nullptr, wxCB_READONLY);
	pCaptureCombo->Append(L"BitBlt");
	pCaptureCombo->Append(L"Desktop Duplication API");
	pCaptureCombo->Select(pInitConf->CaptureMethod);
	pCaptureSizer->Add(pCaptureCombo, wxSizerFlags().CenterVertical());
	
	pCaptureSizer->AddSpacer(10);
	
	pDialogSizer->Add(pCaptureSizer);
	pDialogSizer->AddSpacer(10);
	
	//
	// 撮影領域
	//
	
	wxBoxSizer *pRegionComboSizer = new wxBoxSizer(wxHORIZONTAL);
	pRegionComboSizer->AddSpacer(10);
	
	wxStaticText *pRegionLabel = new wxStaticText(this, wxID_ANY, L"撮影範囲");
	pRegionComboSizer->Add(pRegionLabel, wxSizerFlags().CenterVertical());
	pRegionComboSizer->AddSpacer(5);
	
	pRegionCombo = new wxComboBox(
		this, wxID_ANY, wxEmptyString,
		wxDefaultPosition, wxDefaultSize,
		0, nullptr, wxCB_READONLY);
	pRegionCombo->Bind(wxEVT_COMBOBOX, &ConfigFrame::OnRegionComboChanged, this);
	pRegionCombo->Append(L"アクティブウィンドウ");
	pRegionCombo->Append(L"指定ウィンドウ");
	pRegionCombo->Append(L"選択範囲");
	pRegionCombo->Append(L"フルスクリーン");
	pRegionCombo->Select(0);
	pRegionComboSizer->Add(pRegionCombo, wxSizerFlags().CenterVertical());
	pRegionComboSizer->AddSpacer(10);
	
	pDialogSizer->Add(pRegionComboSizer);
	pDialogSizer->AddSpacer(10);
	
	//
	// ウィンドウ枠パネル
	//
	
	pWindowBorderPanel = new wxPanel(this, wxID_ANY);
	
	wxBoxSizer *pBorderSizer = new wxBoxSizer(wxHORIZONTAL);
	pWindowBorderPanel->SetSizer(pBorderSizer);
	
	pBorderSizer->AddSpacer(10);
	
	pIncludeBorderCheck = new wxCheckBox(pWindowBorderPanel, wxID_ANY, L"ウィンドウの枠を含める");
	pIncludeBorderCheck->SetValue(pInitConf->IncludeBorder);
	pBorderSizer->Add(pIncludeBorderCheck);
	
	
	pDialogSizer->Add(pWindowBorderPanel);
	pDialogSizer->AddSpacer(10);
	
	
	//
	// 範囲選択パネル
	//
	pRegionSelectPanel = CreateRegionSelectPanel(pInitConf);
	pDialogSizer->Add(pRegionSelectPanel);
	
	//
	// ホットキー
	//
	
	HotkeyMod = pInitConf->HotkeyMod;
	HotkeyCode = pInitConf->HotkeyCode;
	HotkeyCodeRaw = pInitConf->HotkeyCodeRaw;
	
	wxBoxSizer *pHotkeySizer = new wxBoxSizer(wxHORIZONTAL);
	pHotkeySizer->AddSpacer(10);
	
	wxStaticText *pHotkeyLabel = new wxStaticText(this, wxID_ANY, L"ショートカット");
	pHotkeySizer->Add(pHotkeyLabel, wxSizerFlags().CenterVertical());
	pHotkeySizer->AddSpacer(5);
	
	pHotkeyText = new wxTextCtrl(this, wxID_ANY, L"",
		wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	pHotkeyText->SetValue(wxAcceleratorEntry(HotkeyMod, HotkeyCode).ToString());
	pHotkeySizer->Add(pHotkeyText, wxSizerFlags(1).CenterVertical());
	pHotkeySizer->AddSpacer(5);
	
	pHotkeyRegButton = new wxButton(this, wxID_ANY, L"登録開始");
	pHotkeyRegButton->Bind(wxEVT_BUTTON, &ConfigFrame::OnRegisterHotkey, this);
	pHotkeySizer->Add(pHotkeyRegButton, wxSizerFlags().CenterVertical());
	pHotkeySizer->AddSpacer(10);
	
	pDialogSizer->Add(pHotkeySizer);
	pDialogSizer->AddSpacer(10);
	
	//
	// OK，キャンセル
	//
	
	wxBoxSizer *pOKCancelSizer = new wxBoxSizer(wxHORIZONTAL);
	pOKCancelSizer->AddSpacer(10);
	
	pOKButton = new wxButton(this, wxID_ANY, L"OK");
	pOKButton->Bind(wxEVT_BUTTON, &ConfigFrame::OnOK, this);
	pOKCancelSizer->Add(pOKButton);
	
	pCancelButton = new wxButton(this, wxID_ANY, L"キャンセル");
	pCancelButton->Bind(wxEVT_BUTTON, &ConfigFrame::OnCancel, this);
	pOKCancelSizer->Add(pCancelButton);
	pOKCancelSizer->AddSpacer(10);
	
	pDialogSizer->Add(pOKCancelSizer, wxSizerFlags().Right());
	pDialogSizer->AddSpacer(10);
	
	Fit();
	
}

wxPanel *ConfigFrame::CreateRegionSelectPanel(
	const std::shared_ptr<Config> &pInitConf)
{
	
	wxPanel *pRegionSelectPanel = new wxPanel(this, wxID_ANY);
	
	// X
	wxStaticText *pRegionXLabel = new wxStaticText(
		pRegionSelectPanel, wxID_ANY, L"X");
	pRegionXText = new wxTextCtrl(
		pRegionSelectPanel, wxID_ANY,
		std::to_string(pInitConf->RegionX),
		wxDefaultPosition, wxSize(50, -1));
	
	// Y
	wxStaticText *pRegionYLabel = new wxStaticText(
		pRegionSelectPanel, wxID_ANY, L"Y");
	pRegionYText = new wxTextCtrl(
		pRegionSelectPanel, wxID_ANY,
		std::to_string(pInitConf->RegionY),
		wxDefaultPosition, wxSize(50, -1));
	
	// Width
	wxStaticText *pRegionWLabel = new wxStaticText(
		pRegionSelectPanel, wxID_ANY, L"Width");
	pRegionWidthText = new wxTextCtrl(
		pRegionSelectPanel, wxID_ANY,
		std::to_string(pInitConf->RegionWidth),
		wxDefaultPosition, wxSize(50, -1));
	
	// Height
	wxStaticText *pRegionHLabel = new wxStaticText(
		pRegionSelectPanel, wxID_ANY, L"Height");
	pRegionHeightText = new wxTextCtrl(
		pRegionSelectPanel, wxID_ANY,
		std::to_string(pInitConf->RegionHeight),
		wxDefaultPosition, wxSize(50, -1));
	
	// 範囲選択ボタン
	pRegionSelectBtn = new wxButton(
		pRegionSelectPanel, wxID_ANY, L"マウスで選択");
	
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
	CloseState = true;
}

void ConfigFrame::OnCancel(wxCommandEvent &ev) {
	Close();
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
		pIncludeBorderCheck->Disable();
		pOKButton->Disable();
		pCancelButton->Disable();
	} else {
		pHotkeyRegButton->SetLabel(L"登録開始");
		pSavePathText->Enable();
		pSavePathRefButton->Enable();
		pWaitTimeText->Enable();
		pCaptureCombo->Enable();
		pIncludeBorderCheck->Enable();
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
	CAPTURE_REGION region = (CAPTURE_REGION)pRegionCombo->GetSelection();
	
	if( region == CAPTURE_REGION_ACTIVE_WINDOW || region == CAPTURE_REGION_SELECTED_WINDOW ) {
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


