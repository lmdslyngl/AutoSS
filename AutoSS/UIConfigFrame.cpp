
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
	
	this->pConf = pInitConf;
	RegisteringHotkey = false;
	
	// ホットキー登録用キーイベントフック
	Bind(wxEVT_CHAR_HOOK, &ConfigFrame::OnKeyDown, this);
	
	// ダイアログを閉じだときのイベント
	Bind(wxEVT_CLOSE_WINDOW, &ConfigFrame::OnClose, this);
	
	// 領域選択ウィンドウ
	pRgnSelWnd = new RegionSelectWindow();
	pRgnSelWnd->SetRegionFinishedCallback(
		[this]() { this->OnRegionSelectFinished(); });
	
	wxFlexGridSizer *pDialogSizer = new wxFlexGridSizer(1, wxSize(10, 10));
	SetSizer(pDialogSizer);
	
	// 各種UI初期化
	InitScreenShotPanel(this, pDialogSizer);
	InitHotKeyPanel(this, pDialogSizer);
	InitNotifSoundPanel(this, pDialogSizer);
	
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
	
	pDialogSizer->Add(pOKCancelSizer, wxSizerFlags().Right());
	
	Fit();
	UpdateRegionSelectionEnabling();
	UpdateNotifSoundEnabling();
	
}

void ConfigFrame::InitScreenShotPanel(wxWindow *pParent, wxSizer *pSizer) {
	wxPanel *pStaticBoxPanel = new wxPanel(pParent);
	
	wxStaticBoxSizer *pStaticBoxSizer = new wxStaticBoxSizer(
		wxHORIZONTAL, pStaticBoxPanel, L"連写設定");
	pStaticBoxPanel->SetSizer(pStaticBoxSizer);
	wxStaticBox *pStaticBox = pStaticBoxSizer->GetStaticBox();
	
	wxFlexGridSizer *pGridSizer = new wxFlexGridSizer(2, wxSize(5, 5));
	pStaticBoxSizer->Add(pGridSizer);
	
	InitSavePathUI(pStaticBox, pGridSizer);
	InitWaitTimeUI(pStaticBox, pGridSizer);
	InitCaptureMethodUI(pStaticBox, pGridSizer);
	InitCaptureRegionUI(pStaticBox, pGridSizer);
	InitMaxCaptureCountUI(pStaticBox, pGridSizer);
	
	pSizer->Add(pStaticBoxPanel);
	
}

void ConfigFrame::InitSavePathUI(wxWindow *pParent, wxSizer *pSizer) {
	wxStaticText *pSavePathLabel = new wxStaticText(pParent, wxID_ANY, L"SS保存先");
	pSaveDirSelector = new DirectorySelector(pParent, pConf->SavePath);
	pSaveDirSelector->GetPathTextCtrl()->SetToolTip(
		L"スクリーンショットの保存先を入力します\n"
		L"空欄にした場合はユーザのピクチャフォルダに保存されます");

	pSizer->Add(pSavePathLabel, wxSizerFlags().CenterVertical().Right());
	pSizer->Add(pSaveDirSelector, wxSizerFlags().Expand());

}

void ConfigFrame::InitWaitTimeUI(wxWindow *pParent, wxSizer *pSizer) {
	wxStaticText *pWaitTimeLabel = new wxStaticText(pParent, wxID_ANY, L"撮影間隔（ミリ秒）");

	wxIntegerValidator<unsigned int> intValid(nullptr);
	pWaitTimeText = new wxTextCtrl(pParent, wxID_ANY,
		std::to_string(pConf->WaitTime),
		wxDefaultPosition, wxSize(50 * pGlbDpiUtil->GetScalingX(), -1), 0, intValid);
	pWaitTimeText->SetToolTip(L"撮影間隔をミリ秒単位で設定します");

	pSizer->Add(pWaitTimeLabel, wxSizerFlags().CenterVertical().Right());
	pSizer->Add(pWaitTimeText, wxSizerFlags().CenterVertical());

}

void ConfigFrame::InitCaptureMethodUI(wxWindow *pParent, wxSizer *pSizer) {
	wxStaticText *pCaptureLabel = new wxStaticText(pParent, wxID_ANY, L"キャプチャ方式");

	pCaptureCombo = new wxComboBox(
		pParent, wxID_ANY, wxEmptyString,
		wxDefaultPosition, wxDefaultSize,
		0, nullptr, wxCB_READONLY);
	pCaptureCombo->Append(L"BitBlt");
	pCaptureCombo->Append(L"Desktop Duplication API");
	pCaptureCombo->Select(pConf->CaptureMethod);
	pCaptureCombo->SetToolTip(
		L"撮影方式を設定します\n"
		L"通常はBitBlt形式が推奨されます");

	pSizer->Add(pCaptureLabel, wxSizerFlags().CenterVertical().Right());
	pSizer->Add(pCaptureCombo, wxSizerFlags().CenterVertical());
	
}

void ConfigFrame::InitCaptureRegionUI(wxWindow *pParent, wxSizer *pSizer) {
	// 撮影領域
	wxStaticText *pRegionLabel = new wxStaticText(pParent, wxID_ANY, L"撮影範囲");

	pRegionCombo = new wxComboBox(
		pParent, wxID_ANY, wxEmptyString,
		wxDefaultPosition, wxDefaultSize,
		0, nullptr, wxCB_READONLY);
	pRegionCombo->Bind(wxEVT_COMBOBOX, &ConfigFrame::OnRegionComboChanged, this);
	pRegionCombo->Append(L"アクティブウィンドウ");
	pRegionCombo->Append(L"選択範囲");
	pRegionCombo->Append(L"フルスクリーン");
	pRegionCombo->Select(pConf->RegionMode);
	pRegionCombo->SetToolTip(
		L"撮影領域を設定します\n"
		L"アクティブウィンドウ: 最前面のウィンドウを撮影します\n"
		L"選択範囲: 座標で領域を設定して撮影します\n"
		L"フルスクリーン: 画面全体を撮影します");

	pSizer->Add(pRegionLabel, wxSizerFlags().CenterVertical().Right());
	pSizer->Add(pRegionCombo, wxSizerFlags().CenterVertical());

	// ウィンドウ枠を含めるかどうか
	pIncludeBorderCheck = new wxCheckBox(pParent, wxID_ANY, L"ウィンドウの枠を含める");
	pIncludeBorderCheck->SetValue(pConf->IncludeBorder);

	pSizer->AddStretchSpacer();
	pSizer->Add(pIncludeBorderCheck);

	// 範囲選択パネル
	wxPanel *pRegionPanelParent = new wxPanel(pParent);

	pRegionSelectPanel = new RegionSelectPanel(
		pRegionPanelParent,
		pConf->RegionX, pConf->RegionY,
		pConf->RegionWidth, pConf->RegionHeight,
		[this](RegionSelectPanel *pPanel) { this->OnRegionSelect(); });

	wxStaticBoxSizer *pRegionPanelSizer = new wxStaticBoxSizer(
		wxHORIZONTAL, pRegionPanelParent, L"選択範囲");
	pRegionPanelSizer->Add(pRegionSelectPanel);
	pRegionPanelParent->SetSizer(pRegionPanelSizer);

	pSizer->AddStretchSpacer();
	pSizer->Add(pRegionPanelParent);
	
}

void ConfigFrame::InitMaxCaptureCountUI(wxWindow *pParent, wxSizer *pSizer) {
	wxStaticText *pMaxCapCountLabel = new wxStaticText(pParent, wxID_ANY, L"最大撮影枚数");
	
	wxIntegerValidator<unsigned int> intValid(nullptr);
	pMaxCapCountText = new wxTextCtrl(pParent, wxID_ANY,
		std::to_string(pConf->MaxCaptureCount),
		wxDefaultPosition, wxSize(50 * pGlbDpiUtil->GetScalingX(), -1), 0, intValid);
	pMaxCapCountText->SetToolTip(
		L"最大撮影枚数を設定します。最大撮影枚数に達したときに自動的に連写が停止します\n"
		L"0に設定すると、無制限に連写を行います");

	pSizer->Add(pMaxCapCountLabel, wxSizerFlags().CenterVertical().Right());
	pSizer->Add(pMaxCapCountText, wxSizerFlags().CenterVertical());
	
}

void ConfigFrame::InitHotKeyPanel(wxWindow *pParent, wxSizer *pSizer) {
	wxPanel *pStaticBoxPanel = new wxPanel(pParent);
	
	wxStaticBoxSizer *pStaticBoxSizer = new wxStaticBoxSizer(
		wxHORIZONTAL, pStaticBoxPanel, L"ホットキー設定");
	pStaticBoxPanel->SetSizer(pStaticBoxSizer);
	wxStaticBox *pStaticBox = pStaticBoxSizer->GetStaticBox();
	
	wxFlexGridSizer *pGridSizer = new wxFlexGridSizer(2, wxSize(5, 5));
	pGridSizer->AddGrowableCol(1);
	pStaticBoxSizer->Add(pGridSizer, wxSizerFlags(1).Expand());
	
	InitHotKeyUI(pStaticBox, pGridSizer);
	
	pSizer->Add(pStaticBoxPanel, wxSizerFlags(1).Expand());
	
}

void ConfigFrame::InitHotKeyUI(wxWindow *pParent, wxSizer *pSizer) {
	HotkeyMod = pConf->HotkeyMod;
	HotkeyCode = pConf->HotkeyCode;
	HotkeyCodeRaw = pConf->HotkeyCodeRaw;

	wxStaticText *pHotkeyLabel = new wxStaticText(pParent, wxID_ANY, L"ショートカット");

	pHotkeyText = new wxTextCtrl(pParent, wxID_ANY, L"",
		wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	pHotkeyText->SetValue(wxAcceleratorEntry(HotkeyMod, HotkeyCode).ToString());

	pHotkeyRegButton = new wxButton(
		pParent, wxID_ANY, L"登録開始",
		wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	pHotkeyRegButton->Bind(wxEVT_BUTTON, &ConfigFrame::OnRegisterHotkey, this);
	pHotkeyRegButton->SetToolTip(
		L"連写の開始/停止を行うショートカットキーを設定します");

	pSizer->Add(pHotkeyLabel, wxSizerFlags().CenterVertical().Right());

	wxBoxSizer *pHotkeySizer = new wxBoxSizer(wxHORIZONTAL);
	pHotkeySizer->Add(pHotkeyText, wxSizerFlags(1).Expand());
	pHotkeySizer->Add(pHotkeyRegButton);
	pSizer->Add(pHotkeySizer, wxSizerFlags(1).Expand());
	
}

void ConfigFrame::InitNotifSoundPanel(wxWindow *pParent, wxSizer *pSizer) {
	wxPanel *pStaticBoxPanel = new wxPanel(pParent);

	wxStaticBoxSizer *pStaticBoxSizer = new wxStaticBoxSizer(
		wxHORIZONTAL, pStaticBoxPanel, L"通知音設定");
	pStaticBoxPanel->SetSizer(pStaticBoxSizer);
	wxStaticBox *pStaticBox = pStaticBoxSizer->GetStaticBox();

	wxFlexGridSizer *pGridSizer = new wxFlexGridSizer(2, wxSize(5, 5));
	pGridSizer->AddGrowableCol(1);
	pStaticBoxSizer->Add(pGridSizer, wxSizerFlags(1).Expand());

	InitNotifSoundUI(pStaticBox, pGridSizer);

	pSizer->Add(pStaticBoxPanel, wxSizerFlags(1).Expand());
	
}

void ConfigFrame::InitNotifSoundUI(wxWindow *pParent, wxSizer *pSizer) {
	wxStaticText *pPlayNotifSoundLabel = new wxStaticText(pParent, wxID_ANY, L"通知音を再生する");

	pPlayNotifSoundCheck = new wxCheckBox(pParent, wxID_ANY, L"");
	pPlayNotifSoundCheck->SetValue(pConf->PlayNotificationSound);
	pPlayNotifSoundCheck->SetToolTip(L"連続撮影開始時と終了時に通知音を再生します");
	pPlayNotifSoundCheck->Bind(wxEVT_CHECKBOX, &ConfigFrame::OnPlayNotifChanged, this);

	pSizer->Add(pPlayNotifSoundLabel, wxSizerFlags().CenterVertical().Right());
	pSizer->Add(pPlayNotifSoundCheck, wxSizerFlags().CenterVertical());

	// 連写開始時通知音
	wxStaticText *pStartNotifSoundLabel = new wxStaticText(pParent, wxID_ANY, L"連写開始時");
	pStartSoundSelector = new SoundFileSelector(
		pParent, pConf->StartNotificationSoundPath);

	pSizer->Add(pStartNotifSoundLabel, wxSizerFlags().CenterVertical().Right());
	pSizer->Add(pStartSoundSelector, wxSizerFlags(1).Expand());

	// 連写停止時通知音
	wxStaticText *pStopNotifSoundLabel = new wxStaticText(pParent, wxID_ANY, L"連写停止時");
	pStopSoundSelector = new SoundFileSelector(
		pParent, pConf->StopNotificationSoundPath);

	pSizer->Add(pStopNotifSoundLabel, wxSizerFlags().CenterVertical().Right());
	pSizer->Add(pStopSoundSelector, wxSizerFlags(1).Expand());
	
}

std::shared_ptr<Config> ConfigFrame::GetConfig() const {
	auto pConf = std::make_shared<Config>();
	pConf->SavePath = pSaveDirSelector->GetPath();
	pConf->WaitTime = std::stoi(pWaitTimeText->GetValue().ToStdString());
	pConf->CaptureMethod = (CAPTURE_METHOD)pCaptureCombo->GetSelection();
	pConf->IncludeBorder = pIncludeBorderCheck->GetValue();
	pConf->HotkeyCode = HotkeyCode;
	pConf->HotkeyCodeRaw = HotkeyCodeRaw;
	pConf->HotkeyMod = HotkeyMod;
	pConf->ImageFormat = IMGFMT_BMP;
	pConf->RegionMode = (CAPTURE_REGION)pRegionCombo->GetSelection();
	pConf->RegionX = pRegionSelectPanel->GetX();
	pConf->RegionY = pRegionSelectPanel->GetY();
	pConf->RegionWidth = pRegionSelectPanel->GetWidth();
	pConf->RegionHeight = pRegionSelectPanel->GetHeight();
	pConf->MaxCaptureCount = std::stoi(pMaxCapCountText->GetValue().ToStdString());
	pConf->PlayNotificationSound = pPlayNotifSoundCheck->GetValue();
	pConf->StartNotificationSoundPath = pStartSoundSelector->GetPath();
	pConf->StopNotificationSoundPath = pStopSoundSelector->GetPath();
	return pConf;
}

void ConfigFrame::OnPlayNotifChanged(wxCommandEvent &ex) {
	UpdateNotifSoundEnabling();
}

// 通知音チェックボックスによって通知村選択の有効/無効を更新する
void ConfigFrame::UpdateNotifSoundEnabling() {
	if( pPlayNotifSoundCheck->GetValue() ) {
		pStartSoundSelector->Enable();
		pStopSoundSelector->Enable();
	} else {
		pStartSoundSelector->Disable();
		pStopSoundSelector->Disable();
	}
}

void ConfigFrame::OnOK(wxCommandEvent &ev) {
	if( !ValidateConfig() ) return;
	Close();
	pRgnSelWnd->Close();
	CloseState = true;
}

bool ConfigFrame::ValidateConfig() {
	// SS保存先存在チェック
	wxString path = pSaveDirSelector->GetPath();
	if( !path.empty() && !wxDirExists(path) ) {
		wxMessageBox(
			L"存在しないSS保存先が指定されています: " + path,
			L"AutoSS",
			wxOK | wxICON_ERROR);
		return false;
	}
	
	// 通知音存在チェック
	if( pPlayNotifSoundCheck->GetValue() ) {
		path = pStartSoundSelector->GetPath();
		if( !path.empty() && !wxFileExists(path) ) {
			wxMessageBox(
				L"連写開始時通知音として指定された音声ファイルが見つかりません: " + path,
				L"AutoSS",
				wxOK | wxICON_ERROR);
			return false;
		}
		
		path = pStopSoundSelector->GetPath();
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
		pSaveDirSelector->Disable();
		pWaitTimeText->Disable();
		pCaptureCombo->Disable();
		pRegionCombo->Disable();
		pIncludeBorderCheck->Disable();
		pRegionSelectPanel->Disable();
		pOKButton->Disable();
		pCancelButton->Disable();
	} else {
		pHotkeyRegButton->SetLabel(L"登録開始");
		pSaveDirSelector->Enable();
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

void ConfigFrame::OnRegionSelect() {
	int startX, startY, endX, endY, width, height;
	startX = pRegionSelectPanel->GetX();
	startY = pRegionSelectPanel->GetY();
	width = pRegionSelectPanel->GetWidth();
	height = pRegionSelectPanel->GetHeight();
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
		
		pRegionSelectPanel->SetX(startX);
		pRegionSelectPanel->SetY(startY);
		pRegionSelectPanel->SetWidth(endX - startX);
		pRegionSelectPanel->SetHeight(endY - startY);
		
	}
}

void ConfigFrame::OnClose(wxCloseEvent &ev) {
	CloseState = false;
	pRgnSelWnd->Destroy();
	Hide();
}


/*
 * パス選択コントロール
*/
DirectorySelector::DirectorySelector(wxWindow *pParent, const std::wstring &defaultPath)
	: wxPanel(pParent)
{
	
	pPathText = new wxTextCtrl(this, wxID_ANY, defaultPath);
	
	pPathSelectBtn = new wxButton(
		this, wxID_ANY, L"...",
		wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	pPathSelectBtn->Bind(wxEVT_BUTTON, &DirectorySelector::OnSelectPath, this);
	
	wxBoxSizer *pSizer = new wxBoxSizer(wxHORIZONTAL);
	pSizer->Add(pPathText, wxSizerFlags(1).CenterVertical());
	pSizer->Add(pPathSelectBtn, wxSizerFlags().CenterVertical());
	
	SetSizer(pSizer);
	
}

void DirectorySelector::OnSelectPath(wxCommandEvent &ev) {
	wxDirDialog dlg(this,
		L"ディレクトリ選択",
		L"", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
	dlg.ShowModal();

	wxString path = dlg.GetPath();
	if( !path.empty() ) {
		pPathText->SetValue(path);
	}
}


/*
 * 選択範囲数値入力パネル
*/
RegionSelectPanel::RegionSelectPanel(
	wxWindow *pParent,
	int x, int y, int width, int height,
	std::function<void(RegionSelectPanel *)> onRegionSelectFunc)
	: wxPanel(pParent)
{
	
	this->OnRegionSelectFunc = onRegionSelectFunc;
	
	wxIntegerValidator<int> regionValidator(nullptr);
	regionValidator.SetMin(0);

	// X
	wxStaticText *pRegionXLabel = new wxStaticText(this, wxID_ANY, L"X");
	pRegionXText = new wxTextCtrl(
		this, wxID_ANY,
		std::to_string(x),
		wxDefaultPosition, wxSize(50 * pGlbDpiUtil->GetScalingX(), -1),
		0, regionValidator);

	// Y
	wxStaticText *pRegionYLabel = new wxStaticText(this, wxID_ANY, L"Y");
	pRegionYText = new wxTextCtrl(
		this, wxID_ANY,
		std::to_string(y),
		wxDefaultPosition, wxSize(50 * pGlbDpiUtil->GetScalingX(), -1),
		0, regionValidator);

	// Width
	wxStaticText *pRegionWLabel = new wxStaticText(
		this, wxID_ANY, L"Width");
	pRegionWidthText = new wxTextCtrl(
		this, wxID_ANY,
		std::to_string(width),
		wxDefaultPosition, wxSize(50 * pGlbDpiUtil->GetScalingX(), -1),
		0, regionValidator);

	// Height
	wxStaticText *pRegionHLabel = new wxStaticText(
		this, wxID_ANY, L"Height");
	pRegionHeightText = new wxTextCtrl(
		this, wxID_ANY,
		std::to_string(height),
		wxDefaultPosition, wxSize(50 * pGlbDpiUtil->GetScalingX(), -1),
		0, regionValidator);

	// 範囲選択ボタン
	pRegionSelectBtn = new wxButton(
		this, wxID_ANY, L"マウスで選択");
	pRegionSelectBtn->Bind(wxEVT_BUTTON, &RegionSelectPanel::OnRegionSelect, this);
	pRegionSelectBtn->SetToolTip(L"マウスを用いて撮影範囲を設定します");
	
	wxBoxSizer *pVSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(pVSizer);
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

void RegionSelectPanel::OnRegionSelect(wxCommandEvent &ev) {
	if( OnRegionSelectFunc ) OnRegionSelectFunc(this);
}


/*
 * 音声ファイル選択コントロール
*/
SoundFileSelector::SoundFileSelector(wxWindow *pParent, const std::wstring &defaultPath)
	: wxPanel(pParent)
{
	
	pPathText = new wxTextCtrl(this, wxID_ANY, defaultPath);

	pPathSelectBtn = new wxButton(
		this, wxID_ANY, L"...",
		wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	pPathSelectBtn->Bind(wxEVT_BUTTON, &SoundFileSelector::OnSelectPath, this);

	pSoundPlayBtn = new wxButton(
		this, wxID_ANY, L"再生",
		wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	pSoundPlayBtn->Bind(wxEVT_BUTTON, &SoundFileSelector::OnPlaySound, this);
	
	wxBoxSizer *pSizer = new wxBoxSizer(wxHORIZONTAL);
	pSizer->Add(pPathText, wxSizerFlags(1));
	pSizer->Add(pPathSelectBtn, wxSizerFlags(0));
	pSizer->Add(pSoundPlayBtn, wxSizerFlags(0));
	
	SetSizer(pSizer);
	
}

void SoundFileSelector::OnSelectPath(wxCommandEvent &ev) {
	wxFileDialog dlg(
		this, L"音声ファイル選択",
		wxEmptyString,
		pPathText->GetValue(),
		L"WAVEファイル (*.wav)|*.wav");
	dlg.ShowModal();
	
	wxString path = dlg.GetPath();
	if( !path.empty() ) {
		pPathText->SetValue(path);
	}
}

void SoundFileSelector::OnPlaySound(wxCommandEvent &ev) {
	wxString path = pPathText->GetValue();
	if( !wxFileExists(path) ) {
		wxMessageBox(
			L"音声ファイルが見つかりません: " + path,
			L"AutoSS",
			wxOK | wxICON_ERROR);
	}
	wxSound::Play(path);
}

