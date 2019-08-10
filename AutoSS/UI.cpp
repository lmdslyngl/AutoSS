
#include "UI.h"
#include <wx/valnum.h>
#include <wx/appprogress.h>
#include <wx/sound.h>
#include "BitBltCapture.h"
#include "DPIUtil.h"

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
	
	pConfBtn = new wxButton(pPanel, wxID_ANY, L"Config");
	pConfBtn->Bind(wxEVT_BUTTON, &AutoSSFrame::OnConf, this);
	pSizer->Add(pConfBtn, wxSizerFlags(1).Expand());
	
	SetSize(250 * pGlbDpiUtil->GetScalingX(),
		100 * pGlbDpiUtil->GetScalingY());
	
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
	
	if( pConfigFrame->GetCloseState() ) {
		// 設定ダイアログでOKボタンが押されていた場合
		pConf = pConfigFrame->GetConfig();
		if( OnChangeConfFunc ) OnChangeConfFunc(pConf);
	}
	
	EnableCapture();
	RegisterHotKey(HOTKEY_ID_START, pConf->HotkeyMod, pConf->HotkeyCodeRaw);
	
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
		pConfBtn->Enable();
		if( OnStopFunc ) OnStopFunc();
	} else {
		pConfBtn->Disable();
		if( OnStartFunc ) OnStartFunc();
	}
}

void AutoSSFrame::Start() {
	TakingSS = true;
	pStartBtn->SetLabel(L"Stop");
	SetStatusText(L"Taking");
	if( pProgressIndicator ) pProgressIndicator->SetValue(1);
}

void AutoSSFrame::Stop() {
	TakingSS = false;
	pStartBtn->SetLabel(L"Start");
	if( pProgressIndicator ) pProgressIndicator->SetValue(0);
}

