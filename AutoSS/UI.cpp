
#include "UI.h"
#include <wx/valnum.h>

/*
 * AutoSSウィンドウ
*/
AutoSSFrame::AutoSSFrame() : wxFrame(nullptr, wxID_ANY, "AutoSS")
{
	
	pConfigFrame = nullptr;
	TakingSS = false;
	
	wxSizer *pSizer = new wxBoxSizer(wxHORIZONTAL);
	
	pStartBtn = new wxButton(this, wxID_ANY, "Start");
	pStartBtn->Bind(wxEVT_BUTTON, &AutoSSFrame::OnStart, this);
	pSizer->Add(pStartBtn, wxSizerFlags(2).Expand());
	
	wxButton *pConfBtn = new wxButton(this, wxID_ANY, "Config");
	pConfBtn->Bind(wxEVT_BUTTON, &AutoSSFrame::OnConf, this);
	pSizer->Add(pConfBtn, wxSizerFlags(1).Expand());
	
	SetSizer(pSizer);
	SetSize(250, 100);
	
	CreateStatusBar(1);
	SetStatusText("Stopped");
	
}

void AutoSSFrame::OnStart(wxCommandEvent &ev) {
	TakingSS = !TakingSS;
	if( TakingSS ) {
		if( OnStartFunc ) OnStartFunc();
		pStartBtn->SetLabel("Stop");
	} else {
		if( OnStopFunc ) OnStopFunc();
		pStartBtn->SetLabel("Start");
	}
}

void AutoSSFrame::OnConf(wxCommandEvent &ev) {
	if( pConfigFrame ) pConfigFrame->Destroy();
	pConfigFrame = new ConfigFrame(this);
	pConfigFrame->ShowModal();
	if( OnChangeConfFunc ) OnChangeConfFunc();
}



/*
 * 設定ダイアログ
*/
ConfigFrame::ConfigFrame(wxFrame *pParent)
	: wxDialog(pParent, wxID_ANY, "AutoSS設定")
{
	
	WaitTime = 10;
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
	
	wxStaticText *pSavePathLabel = new wxStaticText(this, wxID_ANY, "SS保存先");
	pSavePathSizer->Add(pSavePathLabel, wxSizerFlags().CenterVertical());
	pSavePathSizer->AddSpacer(5);
	
	pSavePathText = new wxTextCtrl(this, wxID_ANY);
	pSavePathSizer->Add(pSavePathText, wxSizerFlags(1).CenterVertical());
	pSavePathSizer->AddSpacer(5);
	
	pSavePathRefButton = new wxButton(
		this, wxID_ANY, "参照",
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
	
	wxStaticText *pWaitTimeLabel = new wxStaticText(this, wxID_ANY, "撮影間隔（ミリ秒）");
	pWaitTimeSizer->Add(pWaitTimeLabel, wxSizerFlags().CenterVertical());
	pWaitTimeSizer->AddSpacer(5);
	
	wxIntegerValidator<unsigned int> intValid(&WaitTime);
	pWaitTimeText = new wxTextCtrl(this, wxID_ANY,
		wxEmptyString, wxDefaultPosition, wxSize(50, -1),
		0, intValid);
	pWaitTimeSizer->Add(pWaitTimeText, wxSizerFlags().CenterVertical());
	
	pWaitTimeSizer->AddSpacer(10);
	
	pDialogSizer->Add(pWaitTimeSizer);
	pDialogSizer->AddSpacer(10);
	
	
	//
	// キャプチャ方式
	//
	
	wxBoxSizer *pCaptureSizer = new wxBoxSizer(wxHORIZONTAL);
	pCaptureSizer->AddSpacer(10);
	
	wxStaticText *pCaptureLabel = new wxStaticText(this, wxID_ANY, "キャプチャ方式");
	pCaptureSizer->Add(pCaptureLabel, wxSizerFlags().CenterVertical());
	pCaptureSizer->AddSpacer(5);
	
	pCaptureCombo = new wxComboBox(
		this, wxID_ANY, wxEmptyString,
		wxDefaultPosition, wxDefaultSize,
		0, nullptr, wxCB_READONLY);
	pCaptureCombo->Append("BitBlt");
	pCaptureCombo->Append("Desktop Duplication API");
	pCaptureCombo->Select(0);
	pCaptureSizer->Add(pCaptureCombo, wxSizerFlags().CenterVertical());
	
	pCaptureSizer->AddSpacer(10);
	
	pDialogSizer->Add(pCaptureSizer);
	pDialogSizer->AddSpacer(10);
	
	
	//
	// ウィンドウの枠を含める
	//
	
	wxBoxSizer *pBorderSizer = new wxBoxSizer(wxHORIZONTAL);
	pBorderSizer->AddSpacer(10);
	
	pIncludeBorderCheck = new wxCheckBox(this, wxID_ANY, "ウィンドウの枠を含める");
	pBorderSizer->Add(pIncludeBorderCheck);
	
	pDialogSizer->Add(pBorderSizer);
	pDialogSizer->AddSpacer(10);
	
	
	//
	// ホットキー
	//
	
	wxBoxSizer *pHotkeySizer = new wxBoxSizer(wxHORIZONTAL);
	pHotkeySizer->AddSpacer(10);
	
	wxStaticText *pHotkeyLabel = new wxStaticText(this, wxID_ANY, "ショートカット");
	pHotkeySizer->Add(pHotkeyLabel, wxSizerFlags().CenterVertical());
	pHotkeySizer->AddSpacer(5);
	
	pHotkeyText = new wxTextCtrl(this, wxID_ANY, "",
		wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	pHotkeySizer->Add(pHotkeyText, wxSizerFlags(1).CenterVertical());
	pHotkeySizer->AddSpacer(5);
	
	pHotkeyRegButton = new wxButton(this, wxID_ANY, "登録開始");
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
	
	pOKButton = new wxButton(this, wxID_ANY, "OK");
	pOKButton->Bind(wxEVT_BUTTON, &ConfigFrame::OnOK, this);
	pOKCancelSizer->Add(pOKButton);
	
	pCancelButton = new wxButton(this, wxID_ANY, "キャンセル");
	pCancelButton->Bind(wxEVT_BUTTON, &ConfigFrame::OnCancel, this);
	pOKCancelSizer->Add(pCancelButton);
	pOKCancelSizer->AddSpacer(10);
	
	pDialogSizer->Add(pOKCancelSizer, wxSizerFlags().Right());
	pDialogSizer->AddSpacer(10);
	
	Fit();
	
}

ConfigFrame::CONFIG ConfigFrame::GetConfig() const {
	CONFIG conf;
	conf.SavePath = pSavePathText->GetValue();
	conf.WaitTime = WaitTime;
	conf.CaptureMethod = pCaptureCombo->GetSelection();
	conf.IncludeBorder = pIncludeBorderCheck->GetValue();
	conf.HotkeyCode = HotkeyCode;
	conf.HotkeyMod = HotkeyMod;
	return conf;
}


void ConfigFrame::OnSavePathRef(wxCommandEvent &ev) {
	wxDirDialog dlg(this,
		"スクリーンショット保存先",
		"", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
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
		pHotkeyRegButton->SetLabel("登録完了");
		pSavePathText->Disable();
		pSavePathRefButton->Disable();
		pWaitTimeText->Disable();
		pCaptureCombo->Disable();
		pIncludeBorderCheck->Disable();
		pOKButton->Disable();
		pCancelButton->Disable();
	} else {
		pHotkeyRegButton->SetLabel("登録開始");
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
			HotkeyMod = ev.GetModifiers();
			pHotkeyText->SetValue(wxAcceleratorEntry(HotkeyMod, HotkeyCode).ToString());
		}
		
	} else {
		// ホットキー登録中じゃない
		ev.DoAllowNextEvent();
	}
	
}


