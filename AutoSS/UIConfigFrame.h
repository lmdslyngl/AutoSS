#pragma once

#include <wx/wx.h>
#include <memory>
#include <functional>
#include "Config.h"

/*
 * 範囲選択ウィンドウプロトタイプ
*/
class RegionSelectWindow;


/*
 * ディレクトリ選択コントロール
*/
class DirectorySelector : public wxPanel {
public:
	DirectorySelector(wxWindow *pParent, const std::wstring &defaultPath);
	
	void SetPath(const std::wstring &path) {
		pPathText->SetValue(path);
	}
	
	std::wstring GetPath() const {
		return pPathText->GetValue().ToStdWstring();
	}
	
	wxTextCtrl *GetPathTextCtrl() const {
		return pPathText;
	}
	
private:

	void OnSelectPath(wxCommandEvent &ev);
	
private:
	wxTextCtrl *pPathText;
	wxButton *pPathSelectBtn;
};


/*
 * 選択範囲数値入力パネル
*/
class RegionSelectPanel : public wxPanel {
public:
	RegionSelectPanel(
		wxWindow *pParent,
		int x, int y, int width, int height,
		std::function<void(RegionSelectPanel*)> onRegionSelectFunc);

	void SetX(int x) {
		pRegionXText->SetValue(std::to_wstring(x));
	}
	int GetX() const {
		return std::stoi(pRegionXText->GetValue().ToStdString());
	}
	
	void SetY(int y) {
		pRegionYText->SetValue(std::to_wstring(y));
	}
	int GetY() const {
		return std::stoi(pRegionYText->GetValue().ToStdString());
	}

	void SetWidth(int width) {
		pRegionWidthText->SetValue(std::to_wstring(width));
	}
	int GetWidth() const {
		return std::stoi(pRegionWidthText->GetValue().ToStdString());
	}

	void SetHeight(int height) {
		pRegionHeightText->SetValue(std::to_wstring(height));
	}
	int GetHeight() const {
		return std::stoi(pRegionHeightText->GetValue().ToStdString());
	}

private:
	void OnRegionSelect(wxCommandEvent &ev);
	
private:
	wxTextCtrl *pRegionXText;
	wxTextCtrl *pRegionYText;
	wxTextCtrl *pRegionWidthText;
	wxTextCtrl *pRegionHeightText;
	wxButton *pRegionSelectBtn;
	std::function<void(RegionSelectPanel*)> OnRegionSelectFunc;
};


/*
 * 音声ファイル選択コントロール
*/
class SoundFileSelector : public wxPanel {
public:
	SoundFileSelector(wxWindow *pParent, const std::wstring &defaultPath);
	
	void SetPath(const std::wstring &path) {
		pPathText->SetValue(path);
	}

	std::wstring GetPath() const {
		return pPathText->GetValue().ToStdWstring();
	}

	wxTextCtrl *GetPathTextCtrl() const {
		return pPathText;
	}
	
private:

	void OnSelectPath(wxCommandEvent &ev);
	void OnPlaySound(wxCommandEvent &ev);
	
private:
	wxTextCtrl *pPathText;
	wxButton *pPathSelectBtn;
	wxButton *pSoundPlayBtn;
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
	// UI初期化
	// スクリーンショット系設定パネル
	void InitScreenShotPanel(wxWindow *pParent, wxSizer *pSizer);
	void InitSavePathUI(wxWindow *pParent, wxSizer *pSizer);
	void InitWaitTimeUI(wxWindow *pParent, wxSizer *pSizer);
	void InitCaptureMethodUI(wxWindow *pParent, wxSizer *pSizer);
	void InitCaptureRegionUI(wxWindow *pParent, wxSizer *pSizer);
	void InitMaxCaptureCountUI(wxWindow *pParent, wxSizer *pSizer);
	
	// ホットキー系設定パネル
	void InitHotKeyPanel(wxWindow *pParent, wxSizer *pSizer);
	void InitHotKeyUI(wxWindow *pParent, wxSizer *pSizer);
	
	// 通知音系設定パネル
	void InitNotifSoundPanel(wxWindow *pParent, wxSizer *pSizer);
	void InitNotifSoundUI(wxWindow *pParent, wxSizer *pSizer);
	
	void OnPlayNotifChanged(wxCommandEvent &ex);
	
	void OnOK(wxCommandEvent &ev);
	void OnCancel(wxCommandEvent &ev);

	void OnRegisterHotkey(wxCommandEvent &ev);
	void OnKeyDown(wxKeyEvent &ev);

	void OnRegionComboChanged(wxCommandEvent &ev);
	void OnRegionSelect();
	void OnRegionSelectFinished();

	void OnClose(wxCloseEvent &ev);

	// 撮影範囲コンボボックスによって選択範囲の有効/無効を更新する
	void UpdateRegionSelectionEnabling();

	// 通知音チェックボックスによって通知村選択の有効/無効を更新する
	void UpdateNotifSoundEnabling();

	// 設定の検証
	bool ValidateConfig();

private:
	std::shared_ptr<Config> pConf;
	
	DirectorySelector *pSaveDirSelector;
	wxTextCtrl *pWaitTimeText;
	wxComboBox *pCaptureCombo;
	wxComboBox *pRegionCombo;
	wxCheckBox *pIncludeBorderCheck;
	RegionSelectPanel *pRegionSelectPanel;
	wxTextCtrl *pMaxCapCountText;
	wxButton *pHotkeyRegButton;
	wxTextCtrl *pHotkeyText;
	SoundFileSelector *pStartSoundSelector;
	SoundFileSelector *pStopSoundSelector;
	bool RegisteringHotkey;
	int HotkeyCode, HotkeyCodeRaw, HotkeyMod;
	wxCheckBox *pPlayNotifSoundCheck;
	
	wxButton *pOKButton, *pCancelButton;
	bool CloseState;
	RegionSelectWindow *pRgnSelWnd;
};


