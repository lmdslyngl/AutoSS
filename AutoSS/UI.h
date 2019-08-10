#pragma once

#include <wx/wx.h>
#include <wx/graphics.h>
#include <functional>
#include <memory>
#include "Config.h"
#include "UIConfigFrame.h"

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
	
	void Start();
	void Stop();
	
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
	wxButton *pConfBtn;
	bool IsEnableCapture;
	
	std::function<void()> OnStartFunc;
	std::function<void()> OnStopFunc;
	std::function<void(const std::shared_ptr<Config>&)> OnChangeConfFunc;
	
	std::shared_ptr<Config> pConf;
	
	enum {
		HOTKEY_ID_START
	};
	
};
