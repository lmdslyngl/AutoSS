#pragma once

#define WXUSINGDLL
#include <wx/wx.h>

#include <functional>
#include <memory>
#include "Config.h"

class ConfigFrame;

/*
 * AutoSSウィンドウ
*/
class AutoSSFrame : public wxFrame {
public:
	
	AutoSSFrame(const std::shared_ptr<Config> &pConf);
	
	void SetOnStartFunc(std::function<void()> func) {
		this->OnStartFunc = func;
	}
	void SetOnStopFunc(std::function<void()> func) {
		this->OnStopFunc = func;
	}
	void SetOnChangeConfFunc(std::function<void(const std::shared_ptr<Config>&)> func) {
		this->OnChangeConfFunc = func;
	}
	
	void EnableCapture() {
		IsEnableCapture = true;
		pStartBtn->Enable();
	}
	
	void DisableCapture() {
		IsEnableCapture = false;
		pStartBtn->Disable();
	}
	
private:
	void OnConf(wxCommandEvent &ev);
	
	void OnHotkey(wxKeyEvent &ev);
	void OnStart(wxCommandEvent &ev);
	void OnStartImpl();
	
private:
	ConfigFrame *pConfigFrame;
	bool TakingSS;
	
	wxAppProgressIndicator *pProgressIndicator;
	wxButton *pStartBtn;
	bool IsEnableCapture;
	
	std::function<void()> OnStartFunc;
	std::function<void()> OnStopFunc;
	std::function<void(const std::shared_ptr<Config>&)> OnChangeConfFunc;
	
	std::shared_ptr<Config> pConf;
	
	enum {
		HOTKEY_ID_START
	};
	
};


/*
 * 設定ダイアログ
*/
class ConfigFrame : public wxDialog {
public:
	
	ConfigFrame(wxFrame *pParent, const std::shared_ptr<Config> &pInitConf);
	
	// 設定ダイアログが閉じられた状態を取得
	// true: OK
	// false: Cancel
	bool GetCloseState() const {
		return CloseState;
	}
	
	std::shared_ptr<Config> GetConfig() const;
	
private:
	void OnSavePathRef(wxCommandEvent &ev);
	void OnOK(wxCommandEvent &ev);
	void OnCancel(wxCommandEvent &ev);
	
	void OnRegisterHotkey(wxCommandEvent &ev);
	void OnKeyDown(wxKeyEvent &ev);
	
private:
	wxTextCtrl *pSavePathText;
	wxButton *pSavePathRefButton;
	wxTextCtrl *pWaitTimeText;
	wxComboBox *pCaptureCombo;
	wxCheckBox *pIncludeBorderCheck;
	wxButton *pHotkeyRegButton;
	wxTextCtrl *pHotkeyText;
	bool RegisteringHotkey;
	int HotkeyCode, HotkeyCodeRaw, HotkeyMod;
	wxButton *pOKButton, *pCancelButton;
	bool CloseState;
};


