#pragma once

#define WXUSINGDLL
#include <wx/wx.h>

#include <functional>

class ConfigFrame;

/*
 * AutoSSウィンドウ
*/
class AutoSSFrame : public wxFrame {
public:
	
	AutoSSFrame();
	
	void SetOnStartFunc(std::function<void()> func) {
		this->OnStartFunc = func;
	}
	void SetOnStopFunc(std::function<void()> func) {
		this->OnStopFunc = func;
	}
	void SetOnChangeConfFunc(std::function<void()> func) {
		this->OnChangeConfFunc = func;
	}
	
private:
	void OnStart(wxCommandEvent &ev);
	void OnConf(wxCommandEvent &ev);
	
private:
	ConfigFrame *pConfigFrame;
	bool TakingSS;
	
	wxButton *pStartBtn;
	
	std::function<void()> OnStartFunc;
	std::function<void()> OnStopFunc;
	std::function<void()> OnChangeConfFunc;
	
};


/*
 * 設定ダイアログ
*/
class ConfigFrame : public wxDialog {
public:
	
	struct CONFIG {
		std::string SavePath;
		unsigned int WaitTime;
		int CaptureMethod;
		bool IncludeBorder;
		int HotkeyMod;
		int HotkeyCode;
	};
	
public:
	
	ConfigFrame(wxFrame *pParent);
	
	// 設定ダイアログが閉じられた状態を取得
	// true: OK
	// false: Cancel
	bool GetCloseState() const {
		return CloseState;
	}
	
	CONFIG GetConfig() const;
	
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
	unsigned int WaitTime;
	wxComboBox *pCaptureCombo;
	wxCheckBox *pIncludeBorderCheck;
	wxButton *pHotkeyRegButton;
	wxTextCtrl *pHotkeyText;
	bool RegisteringHotkey;
	int HotkeyCode, HotkeyMod;
	wxButton *pOKButton, *pCancelButton;
	bool CloseState;
};


