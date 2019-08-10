
#include "UIConfigFrame.h"
#include <wx/valnum.h>
#include <wx/sound.h>
#include "UIRegionSelectWindow.h"
#include "DPIUtil.h"

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
	pSavePathText->SetToolTip(
		L"スクリーンショットの保存先を入力します\n"
		L"空欄にした場合はユーザのピクチャフォルダに保存されます");
	
	pSavePathRefButton = new wxButton(
		this, wxID_ANY, L"参照",
		wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	pSavePathRefButton->Bind(wxEVT_BUTTON, &ConfigFrame::OnSavePathRef, this);
	pSavePathRefButton->SetToolTip(L"スクリーンショットの保存先を選択します");
	
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
		wxDefaultPosition, wxSize(50 * pGlbDpiUtil->GetScalingX(), -1), 0, intValid);
	pWaitTimeText->SetToolTip(L"撮影間隔をミリ秒単位で設定します");
	
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
	pCaptureCombo->SetToolTip(
		L"撮影方式を設定します\n"
		L"通常はBitBlt形式が推奨されます");
	
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
	pRegionCombo->SetToolTip(
		L"撮影領域を設定します\n"
		L"アクティブウィンドウ: 最前面のウィンドウを撮影します\n"
		L"選択範囲: 座標で領域を設定して撮影します\n"
		L"フルスクリーン: 画面全体を撮影します");
	
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
		wxDefaultPosition, wxSize(50 * pGlbDpiUtil->GetScalingX(), -1), 0, intValid);
	pMaxCapCountText->SetToolTip(
		L"最大撮影枚数を設定します。最大撮影枚数に達したときに自動的に連写が停止します\n"
		L"0に設定すると、無制限に連写を行います");
	
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
	pHotkeyRegButton->SetToolTip(
		L"連写の開始/停止を行うショートカットキーを設定します");
	
	pDialogSizer->Add(pHotkeyLabel, wxSizerFlags().CenterVertical().Right());
	
	wxBoxSizer *pHotkeySizer = new wxBoxSizer(wxHORIZONTAL);
	pHotkeySizer->Add(pHotkeyText, wxSizerFlags().CenterVertical());
	pHotkeySizer->Add(pHotkeyRegButton, wxSizerFlags().CenterVertical());
	pDialogSizer->Add(pHotkeySizer);
	
	//
	// 通知音再生設定
	//
	wxStaticText *pPlayNotifSoundLabel = new wxStaticText(this, wxID_ANY, L"通知音を再生する");
	
	pPlayNotifSoundCheck = new wxCheckBox(this, wxID_ANY, L"");
	pPlayNotifSoundCheck->SetValue(pInitConf->PlayNotificationSound);
	pPlayNotifSoundCheck->SetToolTip(L"連続撮影開始時と終了時に通知音を再生します");
	pPlayNotifSoundCheck->Bind(wxEVT_CHECKBOX, &ConfigFrame::OnPlayNotifChanged, this);
	
	pDialogSizer->Add(pPlayNotifSoundLabel, wxSizerFlags().CenterVertical().Right());
	pDialogSizer->Add(pPlayNotifSoundCheck, wxSizerFlags().CenterVertical());
	
	// 連写開始時通知音
	wxStaticText *pStartNotifSoundLabel = new wxStaticText(this, wxID_ANY, L"連写開始時");
	
	pStartNotifSoundText = new wxTextCtrl(
		this, wxID_ANY, pInitConf->StartNotificationSoundPath);
	pStartNotifSoundRefButton = new wxButton(
		this, wxID_ANY, L"...",
		wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	pStartNotifSoundRefButton->Bind(wxEVT_BUTTON, &ConfigFrame::OnStartSoundRef, this);
	pStartNotifSoundPlayButton = new wxButton(
		this, wxID_ANY, L"再生",
		wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	pStartNotifSoundPlayButton->Bind(wxEVT_BUTTON, &ConfigFrame::OnStartSoundPlay, this);
	
	wxBoxSizer *pStartNotifSoundSizer = new wxBoxSizer(wxHORIZONTAL);
	pStartNotifSoundSizer->Add(pStartNotifSoundText, wxSizerFlags(1).CenterVertical());
	pStartNotifSoundSizer->Add(pStartNotifSoundRefButton);
	pStartNotifSoundSizer->Add(pStartNotifSoundPlayButton);
	
	pDialogSizer->Add(pStartNotifSoundLabel, wxSizerFlags().CenterVertical().Right());
	pDialogSizer->Add(pStartNotifSoundSizer, wxSizerFlags().Expand());
	
	// 連写停止時通知音
	wxStaticText *pStopNotifSoundLabel = new wxStaticText(this, wxID_ANY, L"連写停止時");
	
	pStopNotifSoundText = new wxTextCtrl(
		this, wxID_ANY, pInitConf->StopNotificationSoundPath);
	pStopNotifSoundRefButton = new wxButton(
		this, wxID_ANY, L"...",
		wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	pStopNotifSoundRefButton->Bind(wxEVT_BUTTON, &ConfigFrame::OnStopSoundRef, this);
	pStopNotifSoundPlayButton = new wxButton(
		this, wxID_ANY, L"再生",
		wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	pStopNotifSoundPlayButton->Bind(wxEVT_BUTTON, &ConfigFrame::OnStopSoundPlay, this);
	
	wxBoxSizer *pStopNotifSoundSizer = new wxBoxSizer(wxHORIZONTAL);
	pStopNotifSoundSizer->Add(pStopNotifSoundText, wxSizerFlags(1).CenterVertical());
	pStopNotifSoundSizer->Add(pStopNotifSoundRefButton);
	pStopNotifSoundSizer->Add(pStopNotifSoundPlayButton);
	
	pDialogSizer->Add(pStopNotifSoundLabel, wxSizerFlags().CenterVertical().Right());
	pDialogSizer->Add(pStopNotifSoundSizer, wxSizerFlags().Expand());
	
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
	UpdateNotifSoundEnabling();
	
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
		wxDefaultPosition, wxSize(50 * pGlbDpiUtil->GetScalingX(), -1),
		0, regionValidator);
	
	// Y
	wxStaticText *pRegionYLabel = new wxStaticText(
		pRegionSelectPanel, wxID_ANY, L"Y");
	pRegionYText = new wxTextCtrl(
		pRegionSelectPanel, wxID_ANY,
		std::to_string(pInitConf->RegionY),
		wxDefaultPosition, wxSize(50 * pGlbDpiUtil->GetScalingX(), -1),
		0, regionValidator);
	
	// Width
	wxStaticText *pRegionWLabel = new wxStaticText(
		pRegionSelectPanel, wxID_ANY, L"Width");
	pRegionWidthText = new wxTextCtrl(
		pRegionSelectPanel, wxID_ANY,
		std::to_string(pInitConf->RegionWidth),
		wxDefaultPosition, wxSize(50 * pGlbDpiUtil->GetScalingX(), -1),
		0, regionValidator);
	
	// Height
	wxStaticText *pRegionHLabel = new wxStaticText(
		pRegionSelectPanel, wxID_ANY, L"Height");
	pRegionHeightText = new wxTextCtrl(
		pRegionSelectPanel, wxID_ANY,
		std::to_string(pInitConf->RegionHeight),
		wxDefaultPosition, wxSize(50 * pGlbDpiUtil->GetScalingX(), -1),
		0, regionValidator);
	
	// 範囲選択ボタン
	pRegionSelectBtn = new wxButton(
		pRegionSelectPanel, wxID_ANY, L"マウスで選択");
	pRegionSelectBtn->Bind(wxEVT_BUTTON, &ConfigFrame::OnRegionSelect, this);
	pRegionSelectBtn->SetToolTip(L"マウスを用いて撮影範囲を設定します");
	
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
	pConf->PlayNotificationSound = pPlayNotifSoundCheck->GetValue();
	pConf->StartNotificationSoundPath = pStartNotifSoundText->GetValue();
	pConf->StopNotificationSoundPath = pStopNotifSoundText->GetValue();
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

void ConfigFrame::OnPlayNotifChanged(wxCommandEvent &ex) {
	UpdateNotifSoundEnabling();
}

// 通知音チェックボックスによって通知村選択の有効/無効を更新する
void ConfigFrame::UpdateNotifSoundEnabling() {
	if( pPlayNotifSoundCheck->GetValue() ) {
		pStartNotifSoundText->Enable();
		pStartNotifSoundRefButton->Enable();
		pStartNotifSoundPlayButton->Enable();
		pStopNotifSoundText->Enable();
		pStopNotifSoundRefButton->Enable();
		pStopNotifSoundPlayButton->Enable();
	} else {
		pStartNotifSoundText->Disable();
		pStartNotifSoundRefButton->Disable();
		pStartNotifSoundPlayButton->Disable();
		pStopNotifSoundText->Disable();
		pStopNotifSoundRefButton->Disable();
		pStopNotifSoundPlayButton->Disable();
	}
}


void ConfigFrame::OnStartSoundRef(wxCommandEvent &ev) {
	wxFileDialog dlg(
		this, L"連写開始時通知音",
		wxEmptyString,
		pStartNotifSoundText->GetValue(),
		L"WAVEファイル (*.wav)|*.wav");
	dlg.ShowModal();
	wxString path = dlg.GetPath();
	if( !path.empty() ) {
		pStartNotifSoundText->SetValue(path);
	}
}

void ConfigFrame::OnStartSoundPlay(wxCommandEvent &ev) {
	wxString soundFilePath = pStartNotifSoundText->GetValue();
	if( !wxFileExists(soundFilePath) ) {
		wxMessageBox(
			L"音声ファイルが見つかりません: " + soundFilePath,
			L"AutoSS",
			wxOK | wxICON_ERROR);
	}
	wxSound::Play(soundFilePath);
}

void ConfigFrame::OnStopSoundRef(wxCommandEvent &ev) {
	wxFileDialog dlg(
		this, L"連写停止時通知音",
		wxEmptyString,
		pStopNotifSoundText->GetValue(),
		L"WAVEファイル (*.wav)|*.wav");
	dlg.ShowModal();
	wxString path = dlg.GetPath();
	if( !path.empty() ) {
		pStopNotifSoundText->SetValue(path);
	}
}

void ConfigFrame::OnStopSoundPlay(wxCommandEvent &ev) {
	wxString soundFilePath = pStopNotifSoundText->GetValue();
	if( !wxFileExists(soundFilePath) ) {
		wxMessageBox(
			L"音声ファイルが見つかりません: " + soundFilePath,
			L"AutoSS",
			wxOK | wxICON_ERROR);
	}
	wxSound::Play(soundFilePath);
}

void ConfigFrame::OnOK(wxCommandEvent &ev) {
	if( !ValidateConfig() ) return;
	Close();
	pRgnSelWnd->Close();
	CloseState = true;
}

bool ConfigFrame::ValidateConfig() {
	// SS保存先存在チェック
	wxString path = pSavePathText->GetValue();
	if( !path.empty() && !wxDirExists(path) ) {
		wxMessageBox(
			L"存在しないSS保存先が指定されています: " + path,
			L"AutoSS",
			wxOK | wxICON_ERROR);
		return false;
	}
	
	// 通知音存在チェック
	if( pPlayNotifSoundCheck->GetValue() ) {
		path = pStartNotifSoundText->GetValue();
		if( !path.empty() && !wxFileExists(path) ) {
			wxMessageBox(
				L"連写開始時通知音として指定された音声ファイルが見つかりません: " + path,
				L"AutoSS",
				wxOK | wxICON_ERROR);
			return false;
		}
		
		path = pStopNotifSoundText->GetValue();
		if( !path.empty() && !wxFileExists(path) ) {
			wxMessageBox(
				L"連写停止時通知音として指定された音声ファイルが見つかりません: " + path,
				L"AutoSS",
				wxOK | wxICON_ERROR);
			return false;
		}
	}
	
	return true;
	
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

